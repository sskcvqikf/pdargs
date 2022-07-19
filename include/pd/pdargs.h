#ifndef PDARGS_PDARGS_H_
#define PDARGS_PDARGS_H_
#pragma once

#include <algorithm>
#include <exception>
#include <limits>
#include <optional>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <vector>

namespace pd {

struct pdargs;

namespace detail {
inline void ltrim(std::string& str) {
  str.erase(str.begin(), std::find_if(str.cbegin(), str.cend(), [](auto e) { return e != '-'; }));
}

inline std::pair<std::string, std::string> lpartition(std::string str, char delim) {
  auto delim_pos = str.find(delim);

  if (delim_pos == std::string::npos) {
    return {std::string(str), {}};
  }

  std::string rhs = std::string(str, delim_pos + 1);
  str.erase(delim_pos);

  return {std::move(str), std::move(rhs)};
}

inline bool is_opt(std::string_view opt) { return opt[0] == '-'; }

inline bool is_short_opt(std::string_view opt) { return is_opt(opt); }

inline bool is_long_opt(std::string_view opt) { return is_opt(opt) and opt[1] == '-'; }

inline bool is_digit(char c) noexcept { return c >= '0' and c <= '9'; }

inline bool is_looks_like_number(std::string_view str) {
  auto const is_positive_number = is_digit(str[0]);
  auto const is_negative_number = str[0] == '-' and is_digit(str[1]);
  return is_negative_number or is_positive_number;
}

inline bool erase_bool_flag(std::vector<std::string>& arguments, char flag) {
  bool erased = false;
  auto find_flag_in_string = [flag, &erased](char ch) {
    if (ch == flag) {
      if (erased) {
        throw std::invalid_argument(
            "Short options for bool presented in "
            "multiple variants.\n");
      }
      erased = true;
      return true;
    }
    return false;
  };

  auto erase_string = [&find_flag_in_string](std::string& str) {
    str.erase(std::remove_if(str.begin(), str.end(), find_flag_in_string), str.end());
    return str.empty();
  };

  arguments.erase(std::remove_if(arguments.begin(), arguments.end(), erase_string), arguments.end());

  return erased;
}

template <typename T>
struct get_return_type {
  using type = std::optional<T>;
};

template <>
struct get_return_type<bool> {
  using type = bool;
};

template <typename T>
using get_return_type_t = typename get_return_type<T>::type;

template <typename T, typename U>
constexpr bool is_allowed_get_or_v = std::is_convertible_v<U, T> and not std::is_same_v<T, bool>;

template <typename T, typename U>
using enable_get_or_t = std::enable_if_t<is_allowed_get_or_v<T, U>, T>;

}  // namespace detail

template <typename T>
T string_to_T(const std::string& str);

struct pdargs {
  explicit pdargs(int argc, char** argv);

  template <typename T>
  detail::get_return_type_t<T> get(const std::pair<std::string, char>& key);

  template <typename T, typename U = T>
  detail::enable_get_or_t<T, U> get_or(const std::pair<std::string, char>& key, U&&);

  const std::vector<std::string>& get_unnamed_args() const& { return no_interest_options_; }

  std::vector<std::string>& get_unnamed_args() & { return no_interest_options_; }

  std::vector<std::string>&& get_unnamed_args() && { return std::move(no_interest_options_); }

 private:
  void add_long_arg(std::string key);
  void add_long_arg(std::string key, std::string value);

  void add_short_arg(std::string key);
  void add_short_arg(std::string key, std::string value);

 private:
  void handle_long_opt(int& i, int argc, char** argv);

  void handle_short_opt(int& i, int argc, char** argv);

 private:
  std::unordered_map<std::string, std::string> long_options_;
  std::unordered_map<char, std::string> short_split_options_;
  std::vector<std::string> short_options_;
  std::vector<std::string> no_interest_options_;
};

inline pdargs::pdargs(int argc, char** argv) {
  for (int i = 1; i != argc; ++i) {
    auto arg = argv[i];

    if (detail::is_long_opt(arg)) {
      handle_long_opt(i, argc, argv);

    } else if (detail::is_short_opt(arg)) {
      handle_short_opt(i, argc, argv);

    } else {
      no_interest_options_.emplace_back(arg);
    }
  }
}

template <typename T>
detail::get_return_type_t<T> pdargs::get(const std::pair<std::string, char>& key) {
  auto long_arg = long_options_.extract(key.first);
  auto short_long_arg = short_split_options_.extract(key.second);
  auto short_arg = std::find_if(short_options_.cbegin(), short_options_.cend(),
                                [&key](const auto& str) { return str[0] == key.second; });

  if (long_arg.empty() and short_arg == short_options_.end() and short_long_arg.empty()) {
    return std::nullopt;
  }

  if (!long_arg.empty() and (short_arg != short_options_.end() or not short_long_arg.empty())) {
    throw std::runtime_error("Option is presented both in long and short variants.\n");
  }

  if (!long_arg.empty()) {
    return string_to_T<T>(std::move(long_arg.mapped()));
  }

  if (!short_long_arg.empty()) {
    return string_to_T<T>(std::move(short_long_arg.mapped()));
  }

  auto ret = string_to_T<T>(std::string(*short_arg, 1));
  short_options_.erase(short_arg);
  return ret;
}

template <>
inline detail::get_return_type_t<bool> pdargs::get<bool>(const std::pair<std::string, char>& key) {
  auto long_arg = long_options_.extract(key.first);
  auto short_long_arg = short_split_options_.extract(key.second);
  auto short_flag = detail::erase_bool_flag(short_options_, key.second);

  if (long_arg.empty() and not short_flag and short_long_arg.empty()) {
    return false;
  }

  if (not long_arg.empty() and (short_flag or not short_long_arg.empty())) {
    throw std::runtime_error("Option is presented both in long and short variants.\n");
  }
  return true;
}

template <typename T, typename U>
detail::enable_get_or_t<T, U> pdargs::get_or(const std::pair<std::string, char>& key, U&& args) {
  auto result = get<T>(key);
  if (result) {
    return *std::move(result);
  }
  return T{std::forward<U>(args)};
}

inline void pdargs::handle_long_opt(int& i, int argc, char** argv) {
  auto&& arg = argv[i];

  if (i == argc - 1) {  // NOLINT: order of conditions matter
    add_long_arg(arg);

  } else if (not detail::is_opt(argv[i + 1]) or detail::is_looks_like_number(argv[i + 1])) {
    add_long_arg(arg, argv[++i]);

  } else {
    add_long_arg(arg);
  }
}

inline void pdargs::handle_short_opt(int& i, int argc, char** argv) {
  auto&& arg = argv[i];

  if (i == argc - 1) {  // NOLINT: order of conditions matter
    add_short_arg(arg);

  } else if (not detail::is_opt(argv[i + 1]) or detail::is_looks_like_number(argv[i + 1])) {
    add_short_arg(arg, argv[++i]);

  } else {
    add_short_arg(arg);
  }
}

inline void pdargs::add_short_arg(std::string key) {
  detail::ltrim(key);

  auto delim_idx = key.find('=');
  if (delim_idx == std::string::npos) {
    short_options_.push_back(std::move(key));
    return;
  }

  auto&& [opt, val] = detail::lpartition(key, '=');
  short_split_options_.emplace(opt[0], std::move(val));
}

inline void pdargs::add_short_arg(std::string key, std::string value) {
  detail::ltrim(key);
  short_split_options_.emplace(key[0], std::move(value));
}

inline void pdargs::add_long_arg(std::string key) {
  detail::ltrim(key);
  auto&& [opt, val] = detail::lpartition(key, '=');
  long_options_.emplace(std::move(opt), std::move(val));
}

inline void pdargs::add_long_arg(std::string key, std::string value) {
  detail::ltrim(key);
  long_options_.emplace(std::move(key), std::move(value));
}

template <>
inline int string_to_T<int>(const std::string& str) {
  return std::stoi(str);
}

template <>
inline long string_to_T<long>(const std::string& str) {
  return std::stol(str);
}

template <>
inline long long string_to_T<long long>(const std::string& str) {
  return std::stoll(str);
}

template <>
inline unsigned string_to_T<unsigned>(const std::string& str) {
  unsigned long result = std::stoul(str);
  if (result > std::numeric_limits<unsigned>::max()) throw std::out_of_range("stou");
  return result;
}

template <>
inline unsigned long string_to_T<unsigned long>(const std::string& str) {
  return std::stoul(str);
}

template <>
inline unsigned long long string_to_T<unsigned long long>(const std::string& str) {
  return std::stoull(str);
}

template <>
inline float string_to_T<float>(const std::string& str) {
  return std::stof(str);
}

template <>
inline double string_to_T<double>(const std::string& str) {
  return std::stod(str);
}

template <>
inline long double string_to_T<long double>(const std::string& str) {
  return std::stold(str);
}

template <>
inline std::string string_to_T<std::string>(const std::string& str) {
  return str;
}

}  // namespace pd

#endif  // PDARGS_PDARGS_H_

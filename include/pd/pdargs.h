#ifndef PDARGS_PDARGS_HH_
#define PDARGS_PDARGS_HH_
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
  str.erase(str.begin(), std::find_if(str.cbegin(), str.cend(),
                                      [](auto e) { return e != '-'; }));
}

inline std::pair<std::string, std::string> lpartition(const std::string& str,
                                                      char delim) {
  auto delim_pos = str.find(delim);

  if (delim_pos == std::string::npos) {
    return {str, {}};
  }

  std::string lhs = std::string(str, 0, delim_pos);
  std::string rhs = std::string(str, delim_pos + 1);

  return {lhs, rhs};
}

inline bool is_opt(const char* opt) { return opt != nullptr and opt[0] == '-'; }

inline bool is_short_opt(const char* opt) {
  return is_opt(opt) and opt[0] != '\0';
}

inline bool is_long_opt(const char* opt) {
  return is_opt(opt) and opt[1] == '-';
}

inline bool is_digit(char c) noexcept { return c >= '0' and c <= '9'; }

inline bool is_looks_like_number(const char* str) {
  if (str == nullptr) {
    return false;
  }
  auto const is_positive_number = is_digit(str[0]);
  auto const is_negative_number = str[0] == '-' and is_digit(str[1]);
  return is_negative_number or is_positive_number;
}

template <typename T>
T string_to_T(const std::string&);

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
  if (result > std::numeric_limits<unsigned>::max())
    throw std::out_of_range("stou");
  return result;
}

template <>
inline unsigned long string_to_T<unsigned long>(const std::string& str) {
  return std::stoul(str);
}

template <>
inline unsigned long long string_to_T<unsigned long long>(
    const std::string& str) {
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

template <typename T>
struct get_return_type {
  using type = std::optional<T>;
};

template <>
struct get_return_type<bool> {
  using type = bool;
};

template <typename T>
using get_return_t = typename get_return_type<T>::type;

template <typename T>
struct get_or_impl {
  template <typename U>
  static T invoke(std::pair<std::string, char>, U&&,
                  std::unordered_map<std::string, std::string>& longs,
                  std::unordered_map<char, std::string>& short_longs,
                  std::vector<std::string>& shorts);
};

template <>
struct get_or_impl<bool> {
  template <typename U>
  static bool invoke(std::pair<std::string, char>, U&&,
                     std::unordered_map<std::string, std::string>& longs,
                     std::unordered_map<char, std::string>& short_longs,
                     std::vector<std::string>& shorts) = delete;
};

template <typename T>
template <typename U>
T get_or_impl<T>::invoke(std::pair<std::string, char> arg, U&& val,
                         std::unordered_map<std::string, std::string>& longs,
                         std::unordered_map<char, std::string>& short_longs,
                         std::vector<std::string>& shorts) {
  static_assert(std::is_convertible_v<U&&, T> and not std::is_same_v<T, bool>,
                "T must be convertible from U&& and T must not be bool.\n");
  
  auto long_arg = longs.extract(arg.first);
  auto short_long = short_longs.extract(arg.second);
  auto short_arg = std::find_if(shorts.cbegin(), shorts.cend(),
                                [&arg](const auto& str) { return str[0] == arg.second; });
  
  if (long_arg.empty() and short_long.empty() and short_arg == shorts.end())
    return static_cast<T>(std::forward<U>(val));
  
  if (not long_arg.empty() and (not short_long.empty() or short_arg != shorts.end()))
    throw std::runtime_error(
        "Option is presented both in long and short variants.\n");
  
  if (not long_arg.empty()) {
    return detail::string_to_T<T>(std::move(long_arg.mapped()));
  }
  
  if (not short_long.empty()) {
    return detail::string_to_T<T>(std::move(short_long.mapped()));
  }
  
  auto ret = detail::string_to_T<T>(std::string(*short_arg, 1));
  shorts.erase(short_arg);
  return ret;
}
}  // namespace detail

struct pdargs {
  explicit pdargs(int argc, char** argv);

  template <typename T>
  detail::get_return_t<T> get(std::pair<std::string, char>);

  template <typename T, typename U = T>
  T get_or(std::pair<std::string, char>, U&&);

 private:
  void add_long_arg(std::string);
  void add_long_arg(std::string, std::string);

  void add_short_arg(std::string);
  void add_short_arg(std::string, std::string);

 private:
  void handle_long_opt(int& i, int argc, char** argv);

  void handle_short_opt(int& i, int argc, char** argv);

 private:
  std::unordered_map<std::string, std::string> long_opts_;
  std::unordered_map<char, std::string> short_long_opts_;
  std::vector<std::string> short_opts_;
  std::vector<std::string> no_interest_opts_;
};

inline pdargs::pdargs(int argc, char** argv) {
  for (int i = 1; i != argc; ++i) {
    auto arg = argv[i];

    if (detail::is_long_opt(arg)) {
      handle_long_opt(i, argc, argv);

    } else if (detail::is_short_opt(arg)) {
      handle_short_opt(i, argc, argv);

    } else {
      no_interest_opts_.emplace_back(arg);
    }
  }
}

template <typename T>
detail::get_return_t<T> pdargs::get(std::pair<std::string, char> arg) {
  auto long_arg = long_opts_.extract(arg.first);
  auto short_long_arg = short_long_opts_.extract(arg.second);
  auto short_arg = std::find_if(short_opts_.cbegin(), short_opts_.cend(),
                                [&arg](const auto& str) { return str[0] == arg.second; });

  if (long_arg.empty() and short_arg == short_opts_.end() and short_long_arg.empty()) {
    return std::nullopt;
  }

  if (!long_arg.empty() and (short_arg != short_opts_.end() or not short_long_arg.empty())) {
    throw std::runtime_error(
        "Option is presented both in long and short variants.\n");
  }
  
  if (!long_arg.empty()) {
    return detail::string_to_T<T>(std::move(long_arg.mapped()));
  }

  if (!short_long_arg.empty()) {
    return detail::string_to_T<T>(std::move(short_long_arg.mapped()));
  }
  
  auto ret = detail::string_to_T<T>(std::string(*short_arg, 1));
  short_opts_.erase(short_arg);
  return ret;
}

bool erase_bool_flag(std::vector<std::string>& arguments, char flag)
{
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
  
  auto erase_string = [&find_flag_in_string](std::string& str){
    str.erase(std::remove_if(str.begin(), str.end(), find_flag_in_string), str.end());
    return str.empty();
  };
  
  arguments.erase(std::remove_if(arguments.begin(), arguments.end(), erase_string), arguments.end());
  
  return erased;
}

template <>
inline detail::get_return_t<bool> pdargs::get<bool>(
    std::pair<std::string, char> arg) {
  auto long_arg = long_opts_.extract(arg.first);
  auto short_long_arg = short_long_opts_.extract(arg.second);
  auto short_flag = erase_bool_flag(short_opts_, arg.second);
  
  if (long_arg.empty() and not short_flag and short_long_arg.empty())
  {
    return false;
  }
  
  if (not long_arg.empty() and (short_flag or not short_long_arg.empty()))
  {
    throw std::runtime_error(
        "Option is presented both in long and short variants.\n");
  }
  return true;
}

template <typename T, typename U>
T pdargs::get_or(std::pair<std::string, char> arg, U&& val) {
  return detail::get_or_impl<T>::template invoke<U>(
      std::move(arg), std::forward<U>(val), long_opts_, short_long_opts_, short_opts_);
}

inline void pdargs::handle_long_opt(int& i, int argc, char** argv) {
  auto&& arg = argv[i];

  if (i == argc - 1) {
    add_long_arg(arg);
    
  } else if (not detail::is_opt(argv[i + 1]) or
             detail::is_looks_like_number(argv[i + 1])) {
    add_long_arg(arg, argv[++i]);
    
  } else {
    add_long_arg(arg);
    
  }
}

inline void pdargs::handle_short_opt(int& i, int argc, char** argv) {
  auto&& arg = argv[i];

  if (i == argc - 1) {
    add_short_arg(arg);
   
  } else if (not detail::is_opt(argv[i + 1]) or
             detail::is_looks_like_number(argv[i + 1])) {
    add_short_arg(arg, argv[++i]);
    
  } else {
    add_short_arg(arg);
  }
}

inline void pdargs::add_short_arg(std::string arg) {
  detail::ltrim(arg);

  auto delim_idx = arg.find('=');
  if (delim_idx == std::string::npos) {
    short_opts_.push_back(std::move(arg));
    return;
  }

  auto&& [opt, val] = detail::lpartition(arg, '=');
  short_long_opts_.emplace(opt[0], std::move(val));
}

inline void pdargs::add_short_arg(std::string opt, std::string val) {
  detail::ltrim(opt);
  short_long_opts_.emplace(opt[0], std::move(val));
}

inline void pdargs::add_long_arg(std::string arg) {
  detail::ltrim(arg);
  auto&& [opt, val] = detail::lpartition(arg, '=');
  long_opts_.emplace(std::move(opt), std::move(val));
}

inline void pdargs::add_long_arg(std::string opt, std::string val) {
  detail::ltrim(opt);
  long_opts_.emplace(std::move(opt), std::move(val));
}

}  // namespace pd

#endif  // PDARGS_PDARGS_HH_

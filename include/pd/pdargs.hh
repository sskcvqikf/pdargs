#ifndef PDARGS_PDARGS_HH_
#define PDARGS_PDARGS_HH_ 
#pragma once

#include <string>
#include <vector>
#include <string_view>
#include <optional>
#include <exception>
#include <algorithm>
#include <iostream>
#include <limits>

namespace pd
{

struct pdargs;

namespace detail
{
    inline void ltrim(std::string& str)
    {
        str.erase(str.begin(),
                std::find_if(str.cbegin(), str.cend(),
                    [] (auto e)
                    {
                        return e != '-';
                    }));
    }

    inline bool is_long_opt(const char* opt)
    {
        return opt != nullptr &&
               opt[1] != '\0' &&
               opt[0] == '-' && opt[1] == '-';
    }

    inline bool is_short_opt(const char* opt)
    {
        return opt != nullptr &&
               opt[0] == '-';
    }

    template<typename T>
    T string_to_T(const std::string&);

    template<>
    int string_to_T<int>(const std::string &str)
    {
        return std::stoi(str);
    }
    template<>
    long string_to_T<long>(const std::string &str)
    {
        return std::stol(str);
    }
    template<>
    long long string_to_T<long long>(const std::string &str)
    {
        return std::stoll(str);
    }
    template<>
    unsigned string_to_T<unsigned>(const std::string &str)
    {
        unsigned long result = std::stoul(str);
        if (result > std::numeric_limits<unsigned>::max())
            throw std::out_of_range("stou");
        return result;
    }
    template<>
    unsigned long string_to_T<unsigned long>(const std::string &str)
    {
        return std::stoul(str);
    }
    template<>
    unsigned long long string_to_T<unsigned long long>(const std::string &str)
    {
        return std::stoull(str);
    }
    template<>
    float string_to_T<float>(const std::string &str)
    {
        return std::stof(str);
    }
    template<>
    double string_to_T<double>(const std::string &str)
    {
        return std::stod(str);
    }
    template<>
    long double string_to_T<long double>(const std::string &str)
    {
        return std::stold(str);
    }

    bool is_digit(char c)
    {
        return c >= '0' && c <= '9';
    }
} // namespace detail

struct pdargs
{
    explicit pdargs(int argc, char** argv); 

    void print()
    {
        std::cout << "Longs:\n";
        for(const auto& i: longs_)
            std::cout << i.first << " " << i.second << '\n';
        std::cout << "Shorts:\n";
        for(const auto& i: shorts_)
            std::cout << i << '\n';
    }

    template<typename T>
    std::optional<T> get(std::pair<std::string, char>);

private:
    void add_long_arg(std::string);
    void add_long_arg(std::string, std::string);
    void add_short_arg(std::string);
    void add_short_arg(std::string, std::string);

    std::unordered_map<std::string, std::string> longs_;
    std::vector<std::string> shorts_;
};

void pdargs::add_short_arg(std::string arg)
{
    auto delim_idx = arg.find('=');
    if (delim_idx != std::string::npos)
        arg.erase(delim_idx);
    detail::ltrim(arg);
    shorts_.push_back(std::move(arg));
}

void pdargs::add_short_arg(std::string arg, std::string param)
{
    detail::ltrim(arg);
    if (arg.size() > 2)
        throw std::invalid_argument("Short option must be exactly one character length if separated with space");
    shorts_.push_back(arg + param);
}

void pdargs::add_long_arg(std::string arg)
{
    detail::ltrim(arg);
    auto delim_idx = arg.find('=');
    if (delim_idx == std::string::npos)
        longs_[std::move(arg)] = {};
    longs_[std::string(arg, 0, delim_idx)] = std::string(arg, delim_idx+1);

}

void pdargs::add_long_arg(std::string arg, std::string param)
{
    detail::ltrim(arg);
    longs_[std::move(arg)] = std::move(param);
}

pdargs::pdargs(int argc, char** argv)
{
    for (int i = 0; i < argc; ++i)
    {
        auto arg = argv[i];
        if (detail::is_long_opt(arg))
        {
            if (i == argc - 1)
                add_long_arg(arg);
            else if (argv[i+1][0] != '-' ||
                    (argv[i+1][0] == '-' && detail::is_digit(argv[i+1][1])))
            {
                add_long_arg(arg, argv[++i]);
            }
            else
                add_long_arg(arg);
        }
        else if (detail::is_short_opt(arg))
        {
            if (i == argc - 1)
                add_short_arg(arg);
            else if (argv[i+1][0] != '-' ||
                    (argv[i+1][0] == '-' && detail::is_digit(argv[i+1][1])))
                add_short_arg(arg, argv[++i]);
            else
                add_short_arg(arg);
        }
    }
}

template<typename T>
std::optional<T> pdargs::get(std::pair<std::string, char> arg)
{
    auto long_arg = longs_.extract(arg.first);
    auto short_arg = std::find_if(shorts_.cbegin(), shorts_.cend(),
            [&arg] (const auto& str)
            {
                return str[0] == arg.second;
            });
    if (long_arg.empty() && short_arg == shorts_.end())
        return std::nullopt;
    if (!long_arg.empty() && short_arg != shorts_.end())
        throw std::runtime_error("Option is presented both in long and short variants.\n");
    if (!long_arg.empty())
    {
        return detail::string_to_T<T>(long_arg.mapped());
    }
    auto ret = detail::string_to_T<T>(std::string(*short_arg, 1));
    shorts_.erase(short_arg);
    return ret;
}

template<>
std::optional<bool> pdargs::get<bool>(std::pair<std::string, char> arg)
{
    auto long_arg = longs_.extract(arg.first);
    
    bool short_flag{false};
    shorts_.erase(std::remove_if(shorts_.begin(), shorts_.end(),
        [&arg, &short_flag] (auto& str)
        {
            str.erase(std::remove_if(str.begin(), str.end(),
                [&arg, &short_flag](auto c)
                {
                    if (c == arg.second)
                    {
                        if (short_flag)
                            throw std::invalid_argument("Short options for bool presented in multiple variants.\n");
                        else
                            short_flag = true;
                        return true;
                    }
                    return false;
                }), str.end());
            return str.empty();
        }), shorts_.end());
    if (long_arg.empty() && !short_flag)
        return std::nullopt;
    if (!long_arg.empty() && short_flag)
        throw std::runtime_error("Option is presented both in long and short variants.\n");
    return true;
}

} // namespace pd
#endif // PDARGS_PDARGS_HH_

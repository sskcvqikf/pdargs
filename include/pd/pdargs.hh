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

private:

    void add_long_arg(std::string);
    void add_long_arg(std::string, std::string);
    void add_short_arg(std::string);
    void add_short_arg(std::string, std::string);

    std::vector<std::pair<std::string, std::string>> longs_;
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
    shorts_.push_back(arg + param);
}

void pdargs::add_long_arg(std::string arg)
{
    detail::ltrim(arg);
    auto delim_idx = arg.find('=');
    if (delim_idx == std::string::npos)
        throw std::invalid_argument("Long option must have an argument separated with '='\n");
    longs_.push_back(std::make_pair(
                std::string(arg, 0, delim_idx),
                std::string(arg, delim_idx+1)));

}

void pdargs::add_long_arg(std::string arg, std::string param)
{
    detail::ltrim(arg);
    longs_.push_back(std::make_pair(
                std::move(arg), std::move(param)));
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
            else if (argv[i+1][0] != '-')
            {
                add_long_arg(arg, argv[i+1]);
                ++i;
            }
            else
                add_long_arg(arg);
        }
        else if (detail::is_short_opt(arg))
        {
            if (i == argc - 1)
                add_short_arg(arg);
            else if (argv[i+1][0] != '-')
            {
                add_short_arg(arg, argv[i+1]);
                ++i;
            }
            else
                add_short_arg(arg);
        }
    }
}

} // namespace pd
#endif // PDARGS_PDARGS_HH_

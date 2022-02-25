
#pragma once

#include <string>
#include <string_view>
#include <cctype>           // std::isspace


class parsed
{

std::string_view str;

public:
    parsed(std::string_view str) : str(str)
    { }

    std::string until(char end)
    {
        auto result = std::string{};

        while (!str.empty())
        {
            char ch = str.front();
            str.remove_prefix(1);

            if (ch == end)
                break;

            result += ch;
        }
        return result;
    }


    void whitespace()
    {
        auto isspace = [&](char ch) -> bool
        {
            return std::isspace(static_cast<unsigned char>(ch));
        };

        while (!str.empty() && isspace(str.front()))
        {
            str.remove_prefix(1);
        }
    }


    bool symbol(char ch)
    {
        if (str.empty())
            return false;

        if (str.front() != ch)
            return false;
        
        str.remove_prefix(1);
        return true;
    }


    bool next_one_of(std::string_view options)
    {
        if (str.empty())
            return false;

        return options.find(str.front()) != options.npos;
    }


    std::string parse(std::string_view allowed)
    {
        auto result = std::string{};
        while (!str.empty() && allowed.find(str.front()) != allowed.npos)
        {
            result += str.front();
            str.remove_prefix(1);
        }
        return result;
    }
};


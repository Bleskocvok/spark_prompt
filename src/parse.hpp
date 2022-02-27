
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


    bool empty() const
    {
        return str.empty();
    }


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


    std::string until_one_of(std::string_view options)
    {
        auto result = std::string{};

        while (!str.empty())
        {
            char ch = str.front();

            if (options.find(ch) != options.npos)
                break;

            str.remove_prefix(1);
            result += ch;
        }
        return result;
    }


    bool whitespace()
    {
        auto isspace = [&](unsigned char ch) -> bool
        {
            return std::isspace(ch);
        };

        bool found = false;
        while (!str.empty() && isspace(str.front()))
        {
            found = true;
            str.remove_prefix(1);
        }
        return found;
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


    bool next(char ch) const
    {
        return !str.empty() && str.front() == ch;
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


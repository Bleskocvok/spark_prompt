#pragma once

// custom
#include "exception.hpp"

// c
#include <cctype>       // isspace

// cpp
#include <string>       // string
#include <string_view>  // string_view


class parsed
{
    std::string_view str;
    std::string_view start;

public:
    parsed(std::string_view str) : str(str), start(str)
    { }


    bool empty() const { return str.empty(); }

    size_t read_bytes() const { return str.data() - start.data(); }


    bool is_next(char ch) const noexcept
    {
        return !str.empty() && str.front() == ch;
    }


    bool is_next_one_of(std::string_view options) const noexcept
    {
        if (str.empty())
            return false;

        return options.find(str.front()) != options.npos;
    }


    bool try_whitespace() noexcept
    {
        auto isspace = [](unsigned char ch) { return std::isspace(ch); };

        bool found = false;
        while (!str.empty() && isspace(str.front()))
        {
            found = true;
            str.remove_prefix(1);
        }
        return found;
    }


    char peek() const noexcept(false)
    {
        if (str.empty())
            throw parse_exception("unexpected eof reached");

        return str.front();
    }


    std::string try_until(std::string_view options) noexcept
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


    char symbol(char ch) noexcept(false)
    {
        if (peek() != ch)
            throw parse_exception("unexpected '", str.front(), "'"
                                  ", expected '", ch,          "'");

        str.remove_prefix(1);
        return ch;
    }


    char symbol(std::string_view allowed) noexcept(false)
    {
        auto it = allowed.find(peek());
        if (it == allowed.npos)
            throw parse_exception("unexpected '",        peek(),  "'"
                                  ", expected one of '", allowed, "'");

        str.remove_prefix(1);
        return allowed[it];
    }


    bool try_symbol(char c) noexcept
    {
        return try_symbol(std::string_view(&c, 1));
    }

    bool try_symbol(std::string_view allowed) noexcept
    {
        if (str.empty() || allowed.find(str.front()) != allowed.npos)
            return false;

        str.remove_prefix(1);
        return true;
    }


    std::string string(std::string_view allowed) noexcept(false)
    {
        auto result = std::string{};
        result += symbol(allowed);

        while (is_next_one_of(allowed))
            result += symbol(allowed);

        return result;
    }


    std::string try_string(std::string_view allowed) noexcept
    {
        auto result = std::string{};

        while (is_next_one_of(allowed))
            result += symbol(allowed);

        return result;
    }
};


#pragma once

// cpp
#include <string>       // string
#include <exception>    // exception
#include <utility>      // move, forward
#include <ostream>      // ostream
#include <sstream>      // stringstream


template<typename Fst, typename ... Args>
void concat(std::stringstream& str, Fst&& fst, Args&& ... args)
{
    str << fst;

    if constexpr (sizeof...(Args) > 0)
    {
        concat(str, std::forward<Args>(args)...);
    }
}


struct parse_exception : std::exception
{
    std::string msg;

    parse_exception(std::string what) : msg(std::move(what)) { }

    template<typename ... Args>
    parse_exception(Args&& ... args)
    {
        auto str = std::stringstream{};
        concat(str, std::forward<Args>(args)...);
        msg = str.str();
    }

    const char* what() const noexcept override { return msg.c_str(); }

    friend std::ostream& operator<<(std::ostream& out,
                                    const parse_exception& err)
    {
        return out << err.what();
    }
};

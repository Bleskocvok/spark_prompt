#pragma once

#include "eval/evaluate.hpp"

#include <string>       // string
#include <cstdlib>      // getenv
#include <cstdint>      // uint8_t
#include <utility>      // move
#include <memory>       // unique_ptr

// POSIX
#include <unistd.h>     // posix, geteuid, gethostname, getcwd
#include <sys/types.h>  // geteuid, getpwuid
#include <pwd.h>        // getpwuid


inline void add_builtin(evaluator&, int);


struct username_t : builtin_func<>
{
    evaluated perform() override
    {
        struct passwd* user_info = ::getpwuid(::geteuid());
        return std::string{ user_info->pw_name };
    }

    const char* name() const override { return "user"; }
};

struct hostname_t : builtin_func<>
{
    evaluated perform() override
    {
        char buffer[256] = { 0 };
        ::gethostname(buffer, sizeof(buffer));
        return std::string{ buffer };
    }

    const char* name() const override { return "host"; }
};

inline bool replace_home(std::string& str)
{
    struct passwd* user_info = ::getpwuid(::geteuid());
    std::string home = user_info->pw_dir;

    if (str.substr(0, home.size()) == home)
    {
        str.erase(1, home.size() - 1);
        str.front() = '~';
        return true;
    }
    return false;
}

struct pwd_t : builtin_func<>
{
    evaluated perform() override
    {
        char* buffer = std::getenv("PWD");
        if (buffer == nullptr)
            return std::string{};

        std::string result = buffer;

        replace_home(result);

        return result;
    }

    const char* name() const override { return "pwd"; }
};

struct pwd_limited_t : builtin_func<typ::integer>
{
    evaluated perform(unsigned max_size) override
    {
        char* buffer = std::getenv("PWD");
        if (buffer == nullptr)
            return std::string{};

        std::string result = buffer;

        struct passwd* user_info = ::getpwuid(::geteuid());
        std::string home = user_info->pw_dir;

        replace_home(result);

        if (result.size() > max_size)
        {
            result.erase(0, result.size() - max_size);
            result.insert(0, "…");
        }
        return result;
    }

    const char* name() const override { return "pwd_limited"; }
};

struct rgb_t : builtin_func<typ::integer, typ::integer, typ::integer>
{
    evaluated perform(unsigned r, unsigned g, unsigned b) override
    {
        if (r > 255) return fail("r > 255");
        if (g > 255) return fail("g > 255");
        if (b > 255) return fail("b > 255");

        return rgb{ std::uint8_t(r),
                    std::uint8_t(g),
                    std::uint8_t(b) };
    }

    const char* name() const override { return "rgb"; }
};

struct exit_t : builtin_func<>
{
    int value = 0;

    exit_t(int value) : value(value)
    { }

    evaluated perform() override
    {
        // TODO: return the actual integer value
        return bool(value == 0);
    }

    const char* name() const override { return "exit"; }
};

struct if_then_else_t : builtin_func<typ::boolean, typ::any, typ::any>
{
    evaluated perform(bool b, evaluated t, evaluated f) override
    {
        return b ? std::move(t) : std::move(f);
    }

    const char* name() const override { return "if"; }
};

struct append_t : builtin_func<typ::string, typ::string>
{
    evaluated perform(std::string a, std::string b) override
    {
        return std::move(a) += b;
    }

    const char* name() const override { return "append"; }
};

struct fmt_t : builtin_func<typ::string, typ::string, typ::string>
{
    evaluated perform(std::string a, std::string b, std::string c) override
    {
        return (std::move(a) += b) += c;
    }

    const char* name() const override { return "fmt"; }
};


inline void add_builtin(evaluator& eval, int exit_code)
{
    eval.add_func("exit", std::make_unique<exit_t>(exit_code));
    eval.add_func("if", std::make_unique<if_then_else_t>());
    eval.add_func("pwd", std::make_unique<pwd_t>());
    eval.add_func("pwd_limited", std::make_unique<pwd_limited_t>());
    eval.add_func("user", std::make_unique<username_t>());
    eval.add_func("host", std::make_unique<hostname_t>());
    eval.add_func("rgb", std::make_unique<rgb_t>());
    eval.add_func("append", std::make_unique<append_t>());
    eval.add_func("fmt", std::make_unique<fmt_t>());
}

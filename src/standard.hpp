
#pragma once

#include "function.hpp"
#include "style.hpp"


class username_t : public func
{
    std::variant<std::string, f_err> perform(segment&) override;
};


class hostname_t : public func
{
    std::variant<std::string, f_err> perform(segment&) override;
};


class exit_t : public func
{
    std::variant<std::string, f_err> perform(segment& seg) override;
};


class pwd_t : public func
{
    std::variant<std::string, f_err> perform(segment&) override;
};


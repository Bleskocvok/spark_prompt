#pragma once

// custom
#include "segment.hpp"

// cpp
#include <map>          // map
#include <string>       // stoi
#include <vector>       // vector
#include <variant>      // variant
#include <utility>      // move
#include <memory>       // unique_ptr
#include <tuple>        // ignore


class func
{

int code = 1;
std::vector<std::string> parameters;

public:
    struct f_err { std::string msg; };

    virtual ~func() = default;

    void exit_code(int val) { code = val; }
    int exit_code() const { return code; }

    std::variant<std::string, f_err> call(segment& segment,
                                          std::vector<std::string> params)
    {
        parameters = std::move(params);

        // TODO: this is a temporary workaround/hack
        try
        {
            auto r = perform(segment);
            parameters.clear();
            return r;
        }
        catch (std::exception& ex)
        {
            return f_err{ ex.what() };
        }
    }

protected:
    virtual std::variant<std::string, f_err> perform(segment& segment) = 0;

    const std::string& get(size_t idx) { return parameters.at(idx); }

    int get_int(size_t idx) { return std::stoi(get(idx)); }
};


class functions
{
    std::map<std::string, std::unique_ptr<func>> data;

public:

    std::variant<std::string, func::f_err> call(const std::string& name,
                                                segment& segment,
                                                std::vector<std::string> params)
    {
        using namespace std::literals;

        auto found = data.find(name);
        if (found == data.end())
            return func::f_err{ "invalid function `"s + name + "`" };

        auto& func = *found->second;
        return func.call(segment, std::move(params));
    }

    template<typename T, typename ... Args>
    bool add(std::string name, Args&& ... args)
    {
        auto r = data.emplace(std::move(name),
                              std::make_unique<T>(std::forward<Args>(args)...))
                    .second;
        return r;
    }

    template<typename Func>
    void for_each(Func func)
    {
        for (auto& [name, elem] : data)
        {
            std::ignore = name;
            func(*elem);
        }
    }
};


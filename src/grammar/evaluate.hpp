
#pragma once

#include "grammar.hpp"

#include <utility>      // move, pair
#include <variant>      // visit
#include <stdexcept>    // runtime_error
#include <cstdint>      // size_t
#include <optional>     // optional, nullopt
#include <map>          // map
#include <memory>       // unique_ptr



struct color {};
struct segment {};

using evaluated = std::variant<fail,
                               bool,
                               unsigned,
                               std::string,
                               color,
                               sep,
                               segment>;


enum class type : unsigned
{
    boolean  = 1,
    integer  = 2,
    string   = 3,
    color    = 4,
    sep      = 5,
    segment  = 6,
    // variadic = 16,
    any      = 64,
};


// (def  func  x:int  y:bool|int  z:str  =  (if $y $x $z))

struct func
{
    virtual ~func() = default;

    using eval_vec = std::vector<evaluated>;

    std::vector<type> expected;

    virtual evaluated operator()()
    {
        return fail("operator() not implemented");
    }

    template<type Idx>
    auto get(const eval_vec& vec, int i)
    {
        const auto& val = vec[i];

        if constexpr (Idx == type::any)
        {
            return std::get<evaluated>(val);
        }
        else if constexpr (Idx == type::integer)
        {
            return std::get<unsigned>(val);
        }
        else if constexpr (Idx == type::string)
        {
            return std::get<std::string>(val);
        }
        else if constexpr (Idx == type::boolean)
        {
            return std::get<bool>(val);
        }
        else
        {
            return std::get<evaluated>(val);
        }
    }

    auto check_types(const eval_vec& args)
        -> std::optional<std::pair<int, fail>>
    {
        for (std::size_t i = 0; i < expected.size(); ++i)
        {
            if (i >= args.size())
                return std::pair{ i, fail("expected more arguments") };

            if (expected[i] != type::any
                    && args[i].index() != unsigned(expected[i]))
                return std::pair{ i, fail("type mismatch") };
        }

        if (args.size() > expected.size())
            return std::pair{ -1, fail("too many arguments") };

        return std::nullopt;
    }
};


struct evaluator
{
    using functions_t = std::map<std::string, std::unique_ptr<func>>;

    auto operator()(const std::vector<node_ptr>& nodes)
        -> std::vector<evaluated>
    {
        auto result = std::vector<evaluated>{};
        result.reserve(nodes.size());

        for (const auto& ptr : nodes)
            result.push_back(eval(ptr));

        return result;
    }

    auto eval(const node_ptr& ptr) -> evaluated
    {
        functions_t funcs = {};

        auto performer = visitor(funcs);
        return std::visit(performer, *ptr);
    }

    struct visitor
    {
        const functions_t& functions;

        visitor(const functions_t& functions) : functions(functions) {}

        evaluated operator()(const composite_color& com)
        {
            for (auto& ptr : com.args)
                std::visit(*this, *ptr);
            return fail();
        }

        evaluated operator()(const composite_segment& com)
        {
            for (auto& ptr : com.args)
                std::visit(*this, *ptr);
            return fail();
        }

        evaluated operator()(const call& com)
        {
            for (auto& ptr : com.args)
                std::visit(*this, *ptr);
            return fail();
        }

        evaluated operator()(const literal_string& l) { return l.data; }

        evaluated operator()(const literal_bool& l) { return l.value; }

        evaluated operator()(const literal_effect& l) { return std::string{}; }

        evaluated operator()(const literal_separator& l) { return sep::powerline; }

        evaluated operator()(const literal_color& l) { return color{}; }
    };
};

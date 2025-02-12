#pragma once

#include "grammar/grammar.hpp"
#include "style/style.hpp"
#include "func.hpp"

#include <utility>      // move, pair, forward
#include <variant>      // get, visit, variant_alternative_t, variant_size_v
#include <optional>     // optional, nullopt
#include <map>          // map
#include <memory>       // unique_ptr

// (def  func  x:int  y:bool|int  z:str  =  (if $y $x $z))

struct mk_theme : builtin_func<typ::color, typ::color, typ::any>
{
    evaluated perform(color fg, color bg, evaluated) override
    {
        return theme{ fg, bg };
    }

    const char* name() const override { return "mk_theme"; }
};


struct mk_segment : builtin_func<typ::theme, typ::string, typ::sep>
{
    evaluated perform(theme th, std::string str, sep s) override
    {
        return segment{ std::move(str), th, s };
    }

    const char* name() const override { return "mk_segment"; }
};


struct evaluator
{
    using functions_t = std::map<std::string, std::unique_ptr<func>>;

    functions_t funcs;

    evaluator() = default;

    evaluator(functions_t funcs) : funcs(std::move(funcs)) {}

    bool add_func(std::string name, std::unique_ptr<func> f)
    {
        return funcs.emplace(std::move(name), std::move(f)).second;
    }

    template<typename T, typename... Args>
    bool emplace_func(std::string name, Args&&... args)
    {
        return funcs.emplace(
                    std::move(name),
                    std::make_unique<T>(std::forward<Args>(args)...)).second;
    }

    auto operator()(std::vector<node_ptr> nodes) -> maybe<style>
    {
        auto segments = std::vector<segment>{};
        segments.reserve(nodes.size());

        for (auto& ptr : nodes)
        {
            auto res = eval(std::move(ptr));

            if (!is_segment(res))
            {
                if (is_fail(res))
                    return std::get<fail>(res);

                return fail("type mismatch: needs to be ‹segment›, "
                            "but is type ‹", typ_to_str(typ_of(res)), "›");
            }

            segments.push_back(std::move(std::get<segment>(res)));
        }

        return style{ std::move(segments) };
    }

    evaluated eval(node_ptr ptr)
    {
        auto performer = visitor(funcs);
        return std::visit(performer, std::move(*ptr));
    }

    struct visitor
    {
        functions_t& functions;

        // eval_vec buf;

        visitor(functions_t& functions) : functions(functions) {}

        eval_vec operator()(std::vector<node_ptr> args)
        {
            // TODO: reuse this buffer in all visits of this visitor
            // (turn this into an attribute)
            eval_vec buf;
            // buf.clear();

            buf.reserve(args.size());

            for (auto& ptr : args)
                buf.push_back(std::visit(*this, std::move(*ptr)));

            return buf;
        }

        evaluated operator()(composite_color com)
        {
            auto args = (*this)(std::move(com.args));
            auto mk = mk_theme{};
            auto err = mk.check_types(args);
            if (err)
                return err->second;
            return mk(std::move(args));
        }

        evaluated operator()(composite_segment com)
        {
            auto args = (*this)(std::move(com.args));
            auto mk = mk_segment{};
            auto err = mk.check_types(args);
            if (err)
                return err->second;
            return mk(std::move(args));
        }

        evaluated operator()(call com)
        {
            auto it = functions.find(com.name);

            if (it == functions.end())
                return fail("function with name '", com.name, "' not found");

            auto args = (*this)(std::move(com.args));

            func& f = *(it->second);

            auto err = f.check_types(args);
            if (err)
                return err->second;

            return f(std::move(args));
        }

        evaluated operator()(literal_string l) { return std::move(l.data); }

        evaluated operator()(literal_bool l) { return l.value; }

        // TODO
        evaluated operator()(literal_effect) { return std::string{}; }

        evaluated operator()(literal_separator l) { return l.data; }

        evaluated operator()(literal_color l) { return color{ l.data }; }

        evaluated operator()(literal_number n) { return unsigned(n.value); }
    };
};

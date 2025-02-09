
#pragma once

#include "grammar/grammar.hpp"
#include "style/style.hpp"

#include <utility>      // move, pair, forward
#include <variant>      // visit, variant_alternative_t, variant_size_v
#include <stdexcept>    // runtime_error
#include <cstdint>      // size_t
#include <optional>     // optional, nullopt
#include <map>          // map
#include <memory>       // unique_ptr
#include <tuple>        // get, apply
#include <algorithm>    // min


using evaluated = std::variant<fail,
                               bool,
                               unsigned,
                               std::string,
                               color,
                               sep,
                               segment,
                               theme>;


inline bool is_segment(const evaluated& elem)
{
    return std::holds_alternative<segment>(elem);
}

inline bool is_fail(const evaluated& elem)
{
    return std::holds_alternative<fail>(elem);
}


enum class typ : unsigned
{
    fail     = 0,
    boolean  = 1,
    integer  = 2,
    string   = 3,
    color    = 4,
    sep      = 5,
    segment  = 6,
    theme    = 7,
    // variadic = 16,
    any      = 64,
};


inline auto typ_to_str(typ t) -> const char*
{
    switch (t)
    {
        case typ::fail: return "fail";
        case typ::boolean: return "bool";
        case typ::integer: return "int";
        case typ::string: return "str";
        case typ::color: return "color";
        case typ::sep: return "sep";
        case typ::segment: return "segment";
        case typ::theme: return "theme";
        case typ::any: return "any";
        default: return "unknown";
    }
}


inline typ typ_of(const evaluated& val)
{
    return static_cast<typ>(val.index());
}


template<typ Type>
struct get_type
{
    using type = std::conditional_t<Type == typ::any,
                                    evaluated,
                                    std::variant_alternative_t<
                                            std::min<unsigned>(
                                                unsigned(Type),
                                                std::variant_size_v<evaluated> - 1),
                                            evaluated>>;
};

template<typ Type>
using get_type_t = typename get_type<Type>::type;


// (def  func  x:int  y:bool|int  z:str  =  (if $y $x $z))


using eval_vec = std::vector<evaluated>;


struct func
{
    virtual ~func() = default;

    std::vector<typ> expected;

    func() = default;

    explicit func(std::vector<typ> expected) : expected(std::move(expected))
    { }

    virtual evaluated operator()(eval_vec) // NOLINT
    {
        return fail("operator() not implemented");
    }

    virtual const char* name() const = 0;

    template<typ Idx>
    auto get(const eval_vec& vec, int i) -> get_type_t<Idx>
    {
        const evaluated& val = vec[i];

        if constexpr (Idx == typ::any)
            return val;
        else
            return std::get<get_type_t<Idx>>(val);
    }

    template<typ Idx>
    auto get(eval_vec&& vec, int i) -> get_type_t<Idx>
    {
        if constexpr (Idx == typ::any)
            return std::move(vec[i]);
        else
            return std::move(std::get<get_type_t<Idx>>(vec[i]));
    }

    auto check_types(const eval_vec& args)
        -> std::optional<std::pair<int, fail>>
    {
        for (std::size_t i = 0; i < expected.size(); ++i)
        {
            if (i >= args.size())
                return std::pair{ i, fail(name(), ": expected more arguments") };

            if (expected[i] != typ::any
                    && args[i].index() != unsigned(expected[i]))
            {
                // TODO: Why was this here? Investigate.
                // if (args[i].index() == 0)
                //     std::cerr << std::get<fail>(args[i]) << std::endl;

                return std::pair{ i, fail(name(), ": type mismatch: expected ‹",
                                          typ_to_str(expected[i]),
                                          "›, got ‹",
                                          typ_to_str(typ(args[i].index())),
                                          "›") };
            }
        }

        if (args.size() > expected.size())
            return std::pair{ -1, fail(name(), ": too many arguments ", args.size(),
                              ", expected ", expected.size()) };

        return std::nullopt;
    }

    std::string arguments_description() const
    {
        auto result = std::string{};
        bool fst = true;
        for (const auto& t : expected)
        {
            if (!fst)
                result += " ";
            fst = false;

            result += typ_to_str(t);
        }
        return result;
    }

private:
    template<typename... Args>
    std::string desc_rec(typ t, Args... args) const
    {
        if constexpr (sizeof...(Args) > 0)
            return typ_to_str(t) + desc_rec(args...);
        else
            return typ_to_str(t);
    }
};


template<typ... Types>
struct builtin_func : func
{
    builtin_func() : func(std::vector<typ>{ Types... }) // NOLINT
    { }

    evaluated operator()(eval_vec args) override
    {
        if constexpr (sizeof...(Types) == 0)
        {
            return perform();
        }
        else
        {
            auto pack = std::tuple<get_type_t<Types>...>();
            fill_tuple<0, decltype(pack), Types...>(pack, args);

            return std::apply([&](auto... params)
            {
                return perform(std::move(params)...);

            }, std::move(pack));
        }
    }

    virtual evaluated perform([[maybe_unused]] get_type_t<Types>... args)
    {
        return fail("not implemented");
    }

    template<int N = 0, typename Tuple, typ Typ, typ... Nxt>
    void fill_tuple(Tuple& out, eval_vec& args)
    {
        assert(N < args.size());
        std::get<N>(out) = std::move(func::get<Typ>(args, N));

        if constexpr (sizeof...(Nxt) > 0)
            fill_tuple<N + 1, Tuple, Nxt...>(out, args);
    }
};


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

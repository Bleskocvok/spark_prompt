#pragma once

#include "typ.hpp"

#include <utility>      // move, pair, forward
#include <cstddef>      // size_t
#include <optional>     // optional, nullopt
#include <tuple>        // get, apply
#include <algorithm>    // min
#include <variant>      // holds_alternative


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

    // template<typ Idx>
    // auto get(eval_vec&& vec, int i) -> get_type_t<Idx>
    // {
    //     // TODO: This thing is never run.
    //     throw 0;
    //     if constexpr (Idx == typ::any)
    //         return std::move(vec[i]);
    //     else
    //         return std::move(std::get<get_type_t<Idx>>(vec[i]));
    // }

    auto check_types(const eval_vec& args)
        -> std::optional<std::pair<int, fail>>
    {
        for (std::size_t i = 0; i < expected.size(); ++i)
        {
            if (i >= args.size())
                return std::pair{ i, fail(name(), ": expected more arguments") };

            if (expected[i] != typ::any
                    // && expected[i] != typ::optional_any
                    && args[i].index() != unsigned(expected[i]))
            {
                // TODO: Why was this here? Investigate.
                // if (args[i].index() == 0)
                //     std::cerr << std::get<fail>(args[i]) << std::endl;

                // if (args[i].index() != unsigned(from_optional(expected[i])))
                //     continue;

                auto f = fail(name(), ": type mismatch: expected ‹",
                                          typ_to_str(expected[i]),
                                          "›, got ‹",
                                          typ_to_str(typ(args[i].index())),
                                          "›");

                if (std::holds_alternative<fail>(args[i]))
                    f += std::get<fail>(args[i]);

                return std::pair{ i, std::move(f) };
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


#include <memory>       // shared_ptr
#include <variant>      // variant, get_if, get, holds_alternative
#include <vector>       // vector
#include <string>       // string
#include <utility>      // declval, move, forward
#include <functional>   // invoke, forward, ref
#include <type_traits>  // remove_cv, remove_reference, invoke_result,
                        // is_invocable_v, is_constructible
#include <cstdint>      // uint8_t
#include <string_view>  // string_view
#include <set>          // set
#include <sstream>      // stringstream
#include <ostream>      // ostream
#include <tuple>        // tuple
#include <stdexcept>    // runtime_error



struct rgb
{
    std::uint8_t r, g, b;
};

enum class effect : std::uint8_t
{
    bold, blink
};

enum class sep
{
    empty,
    space,
    powerline,
    powerline_space,
    powerline_pseudo,
    newline,
    fill,

    rpowerline,
    rpowerline_space,
    rpowerline_pseudo,
};


struct literal_string;
struct literal_color;
struct literal_effect;
struct literal_separator;
struct literal_bool;
struct composite_color;
struct call;

using node = std::variant<literal_string, literal_color, literal_effect,
                          literal_separator, literal_bool, composite_color,
                          call>;
using node_ptr = std::shared_ptr<node>;


struct literal_string   { std::string data; };

struct literal_color    { rgb data; };

struct literal_effect   { effect data; };

struct literal_separator{ sep data; };

struct literal_bool     { bool value; };

struct composite_color  { std::vector<node_ptr> arguments; };

struct call             { std::string name;
                          std::vector<node_ptr> arguments; };


class tree
{
    node_ptr root;
public:

};



struct fail
{
    std::string msg;

    template<typename ... Args>
    fail(Args&& ... args)
    {
        auto ss = std::stringstream{};
        ( (ss << args), ... );
        msg = ss.str();
    }

    friend std::ostream& operator<<(std::ostream& out, const fail& f)
    {
        return out << f.msg;
    }
};


template<class T>
struct maybe
{
    using value_type = T;

    template<class F, class A>
    using result_t = std::remove_reference_t<std::remove_cv_t
                                            <std::invoke_result_t
                                            <F, A&>>>;

    std::variant<fail, T> value;

    maybe() : value(fail("<empty>")) {}
    maybe(T value) : value(std::move(value)) {}
    maybe(fail er) : value(std::move(er)) {}

    const T& get() const { return std::get<T>(value); }
          T& get()       { return std::get<T>(value); }

    const fail& get_fail() const { return std::get<fail>(value); }
          fail& get_fail()       { return std::get<fail>(value); }

    bool is_failed() const { return std::holds_alternative<fail>(value); }
    bool good()      const { return !is_failed(); }

    operator bool() const { return good(); }

    template<class F>
    auto and_then(F&& f)
    {
        if (T* ptr = std::get_if<T>(&value))
        {
            return std::invoke(std::forward<F>(f), *ptr);
        }
        return result_t<F, T>( std::get<fail>(value) );
    }

    template<class F>
    auto fmap(F&& f)
    {
        using B = result_t<F, T>;

        if (T* ptr = std::get_if<T>(&value))
        {
            return maybe<B>( std::invoke(std::forward<F>(f), *ptr) );
        }
        return maybe<B>( std::get<fail>(value) );
    }
};




class input
{
    std::string_view str;
    std::string_view start;

public:
    input(std::string_view str) : str(str), start(str) {}

    bool end()  const { return str.empty(); }
    bool good() const { return !end(); }

    operator bool() const { return good(); }

    char peek() const { return str.front(); }

    char eat()
    {
        char c = str.front();
        str.remove_prefix(1);
        return c;
    }
};


// struct literal_string;
// struct literal_color;
// struct literal_effect;
// struct literal_separator;
// struct literal_bool;
// struct composite_color;
// struct call;


struct p_char
{
    char c = 0;

    using value_type = char;
    maybe<char> operator()(input& in)
    {
        if (in.good() && in.peek() == c)
        {
            in.eat();
            return c;
        }

        return fail("'", c, "' expected");
    }
};


struct p_num
{
    using value_type = int;
    maybe<int> operator()(input&)
    {
        return 0;
    }
};



struct p_one_of
{
    std::set<char> allowed;

    using value_type = char;
    maybe<char> operator()(input& in)
    {
        if (in.good() && allowed.count(in.peek()))
            return in.eat();

        return fail("expected a symbol");
    }
};


struct p_string
{
    std::set<char> allowed;
    unsigned min_len = 0;

    using value_type = std::string;
    maybe<std::string> operator()(input& in)
    {
        auto result = std::string{};

        while (!in.end() && allowed.count(in.peek()))
            result += in.eat();

        if (result.length() < min_len)
            return fail("string is not of minimum length");

        return result;
    }
};


template<typename B, typename T, typename E>
struct p_between
{
    B begin;
    T thing;
    E end;

    using value_type = typename T::value_type;
    maybe<value_type> operator()(input& in)
    {
        return begin(in)
            .and_then([&](const auto&) { return thing(in); })
            .and_then([&](const auto& t)
            {
                if (auto e = end(in); !e)
                    return e.get_fail();

                return t;
            });
    }
};











// // good source about c++ curry: https://youtu.be/15U4qutsPGk
// // inspired from ^

// template<typename Fn, typename... Args>
// auto curry(Fn&& fn, Args&& ... args)
// {
//     if constexpr (std::is_invocable_v<Fn, Args...>)
//     {
//         return fn(std::forward<Args>(args)...);
//     }
//     else
//     {
//         return [=](auto&&... next)
//         {
//             return curry(fn, args..., next...);
//         };
//     }
// }


// template<int N, typename Fn, typename... Args>
// auto curry_tuple_rec(Fn&& fn, std::tuple<Args...> tuple)
// {
//     if constexpr (N < sizeof...(Args))
//     {
//         auto f_next = curry(fn, std::move(std::get<N>(tuple)));
//         return curry_tuple_rec<N + 1>(f_next, std::move(tuple));
//     }
//     else
//     {
//         return fn;
//     }
// }


// template<typename Fn, typename... Args>
// auto curry_tuple(Fn&& fn, std::tuple<Args...> tuple)
// {
//     // THIS CANNOT BE UNCOMMENTED!
//     // static_assert(std::is_invocable_v<Fn, Args...>,
//     //               "Fn must be invocable with Args...");

//     return curry_tuple_rec<0>(fn, std::move(tuple));
// }



template<typename ... Args>
using tuple_of_maybe = std::tuple<maybe<typename Args::value_type> ...>;


template<int N = 0, typename ... Args>
void unmaybe_tuple_rec(std::tuple<Args...>& out,
                       const std::tuple<maybe<Args>...>& tuple)
{
    if constexpr (N < sizeof...(Args))
    {
        if (!std::get<N>(tuple))
            throw std::runtime_error("maybe holds a ‹fail› value");

        std::get<N>(out) = std::get<N>(tuple).get();
        unmaybe_tuple_rec<N + 1>(out, tuple);
    }
}


template<typename ... Args>
auto unmaybe_tuple(std::tuple<maybe<Args>...> tuple) -> std::tuple<Args...>
{
    auto result = std::tuple<Args...>();
    unmaybe_tuple_rec<0>(result, tuple);
    return result;
}




template<typename Result, int N, typename Tuple, typename ... Args>
static bool parse_from_nth(input& in,
                      Tuple& out,
                      std::tuple<Args...> parsers)
{
    if constexpr (N >= std::tuple_size_v<decltype(parsers)>)
    {
        return true;
    }
    else
    {
        std::get<N>(out) = std::get<N>(parsers)(in);
        if (!std::get<N>(out))
            return false;

        return parse_from_nth<Result, N + 1>(in, out, std::move(parsers));
    }
}


template<typename T, typename ... Args>
static T make(Args&&... args)
{
    // static_assert(std::is_constructible_v<T, Args...>,
    //              "T must be constructible from Args");

    return T(std::forward<Args>(args)...);
}


template<typename Result, typename ... Args>
static maybe<Result> parse(input& in, std::tuple<Args...> parsers)
{
    auto parse_results = tuple_of_maybe<Args ...>();

    bool ok = parse_from_nth<Result, 0>(in, parse_results, parsers);

    if (!ok)
        return fail("p_parse failed");

    auto values = unmaybe_tuple<typename Args::value_type...>(parse_results);

    // static_assert(std::is_constructible_v<Result, typename Args::value_type...>,
    //               "sanity check");

    // return curry_tuple(make<Result, typename Args::value_type...>, values);
    // return curry_tuple(Result::make, values);
    return std::make_from_tuple<Result>(std::move(values));
}



template<typename Result, typename ... Parsers>
struct p_build
{
    std::tuple<Parsers...> parsers;

    p_build(Parsers&&... parsers)
        : parsers(std::forward<Parsers>(parsers)...)
    { }

    using value_type = Result;
    maybe<Result> operator()(input& in)
    {
        static_assert(std::is_constructible_v<Result,
                                              typename Parsers::value_type...>,
                      "Result must be constructible from parser results");

        // auto lam = [&](auto... next) -> maybe<Result>
        // {
        //     return parse<Result>(in, next...);
        // };
        // return curry_tuple(lam, parsers);
        // return curry_tuple(curry(parse<Result>, in), parsers);
        return parse<Result>(in, parsers);
    }
};



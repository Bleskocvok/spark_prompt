
#pragma once

#include <variant>      // variant, get_if, get, holds_alternative

#include <string>       // string
#include <vector>       // vector
#include <functional>   // invoke, forward, ref
#include <type_traits>  // remove_cv, remove_reference, invoke_result,
                        // is_invocable_v, is_constructible
#include <string_view>  // string_view
#include <set>          // set
#include <sstream>      // stringstream
#include <ostream>      // ostream
#include <utility>      // declval, move, forward, pair
#include <tuple>        // tuple, tuple_size_v, get
#include <stdexcept>    // runtime_error
#include <cctype>       // isdigit, isspace
#include <cassert>      // assert



struct fail;
struct input;

template<typename T>
struct maybe;

template<typename F2, typename A, typename B>
auto lift2(F2&& f2, A&& a, B&& b) -> maybe<decltype(f2(a.get(), b.get()))>;


//
// parser objects
//
template<typename T>
struct p_parser;

struct p_char;
struct p_string;
struct p_alpha_str;
struct p_quoted;
struct p_one_of;
struct p_unsigned;
struct p_space;
struct p_space1;

template<typename Pred>
struct p_one_pred;

template<typename B, typename T, typename E>
struct p_between;   // B  *>  T  <*  E

template<typename A, typename B>
struct p_after;

template<typename B, typename T>
struct p_prefixed;  // B  *>  T

template<typename T, typename B>
struct p_suffixed;  // T  <*  B

template<typename P>
struct p_many;

template<typename Result, typename ... Parsers>
struct p_build;

template<typename Result, typename ... Parsers>
struct p_try_seq;



// -----------------------------------------------------------------------------
// IMPLEMENTATION
// -----------------------------------------------------------------------------



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

    std::size_t consumed() const
    {
        return start.length() - str.length();
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

    const T& get() const { assert(good()); return std::get<T>(value); }
          T& get()       { assert(good()); return std::get<T>(value); }

    const fail& get_fail() const { assert(!good()); return std::get<fail>(value); }
          fail& get_fail()       { assert(!good()); return std::get<fail>(value); }

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

    template<typename FVal, typename FFail>
    auto visit(FVal&& f_val, FFail&& f_fail)
    {
        if (good())
            return std::invoke(std::forward<FVal>(f_val), get());

        return std::invoke(std::forward<FFail>(f_fail), get_fail());
    }
};




// TODO: enable_if only if A, B are maybe types
template<typename F2, typename A, typename B>
auto lift2(F2&& f2, A&& a, B&& b) -> maybe<decltype(f2(a.get(), b.get()))>
{
    if (!a)
        return a.get_fail();

    if (!b)
        return b.get_fail();

    return std::invoke(std::forward<A::value_type>(a.get()),
                       std::forward<B::value_type>(b.get()));
}



// -----------------------------------------------------------------------------
// PREDICATES
// -----------------------------------------------------------------------------



struct is_space
{
    bool operator()(unsigned char c) const { return std::isspace(c); }
};


struct is_digit
{
    bool operator()(unsigned char c) const { return std::isdigit(c); }
};


struct is_alpha
{
    bool operator()(unsigned char c) const { return (c >= 'a' && c <= 'z')
                                                 || (c >= 'A' && c <= 'Z'); }
};


// -----------------------------------------------------------------------------
// PARSERS
// -----------------------------------------------------------------------------



template<typename T>
struct p_parser
{
    // typedef  T  value_type;
    using value_type = T;

protected:
    ~p_parser() = default;
};



struct p_char : p_parser<char>
{
    char c = 0;
    p_char(char c) : c(c) {}

    maybe<char> operator()(input& in)
    {
        if (in.good() && in.peek() == c)
        {
            in.eat();
            return c;
        }

        auto got = !in.good() ? std::string("EOF")
                              : std::string() += in.peek();

        return fail("'", c, "' expected, got '", got, "'");
    }
};



struct p_unsigned : p_parser<unsigned>
{
    unsigned min = 0;
    unsigned max = -1;

    p_unsigned(int min, int max) : min(min), max(max) {}
    p_unsigned() = default;

    maybe<unsigned> operator()(input& in)
    {
        if (in.end())
            return fail("EOF unexpected");

        if (!is_digit{}(in.peek()))
            return fail("digit expected, got '", in.peek(), "'");

        unsigned res = 0;
        do
        {
            char c = in.eat();
            res = res * 10 + (c - '0');

        } while (!in.end() && is_digit{}(in.peek()));

        if (res < min || res > max)
            return fail("num ", res, " outside bounds <", min, ",", max, ">");

        return res;
    }
};



template<typename Pred>
struct p_one_pred : p_parser<char>
{
    Pred pred;

    p_one_pred() = default;

    p_one_pred(Pred&& pred) : pred(std::forward<Pred>(pred))
    { }

    maybe<char> operator()(input& in)
    {
        if (in.good() && pred(in.peek()))
            return in.eat();

        auto got = !in.good() ? std::string("EOF")
                              : std::string() += in.peek();

        // TODO: better expected info
        return fail("unexpected '", got, "'");
    }
};



struct p_one_of : p_parser<char>
{
    std::set<char> allowed;

    p_one_of(std::set<char> allowed) : allowed(std::move(allowed))
    { }

    maybe<char> operator()(input& in)
    {
        if (in.good() && allowed.count(in.peek()))
            return in.eat();

        return fail("expected a symbol");
    }
};



struct p_string : p_parser<std::string>
{
    std::set<char> allowed;
    unsigned min_len = 0;

    p_string(std::set<char> allowed, unsigned min_len = 0)
        : allowed(std::move(allowed)), min_len(min_len)
    { }

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
struct p_between : p_parser<typename T::value_type>
{
    using value_type = typename T::value_type;

    B begin;
    T thing;
    E end;

    p_between(B b, T t, E e)
        : begin(std::move(b)), thing(std::move(t)), end(std::move(e))
    { }

    maybe<value_type> operator()(input& in)
    {
        return begin(in)
            .and_then([&](const auto&) { return thing(in); })
            .and_then([&](const auto& t) -> maybe<value_type>
            {
                if (auto e = end(in); !e)
                    return e.get_fail();

                return t;
            });
    }
};



template<typename P>
struct p_many : p_parser<std::vector<typename P::value_type>>
{
    using chunk = typename P::value_type;
    using value_type = std::vector<chunk>;

    P thing;
    unsigned min_count = 0;

    p_many() = default;

    p_many(unsigned min_count) : thing(), min_count(min_count) {}

    p_many(P p, unsigned min_count = 0)
        : thing(std::move(p)), min_count(min_count)
    { }

    maybe<value_type> operator()(input& in)
    {
        auto result = std::vector<chunk>{};
        auto prev_read = in.consumed();

        while (!in.end())
        {
            auto parsed = thing(in);

            auto read = in.consumed();

            if (parsed.is_failed())
            {
                // thing consumed input
                if (read - prev_read > 0)
                    return parsed.get_fail();

                // otherwise
                return result;
            }

            result.push_back(parsed.get());

            // thing consumed input
            if (read == prev_read)
                return result;

            prev_read = read;
        }

        if (result.size() < min_count)
            return fail("p_many parsed only ", result.size(),
                        " while expecting at least ", min_count);

        return result;
    }
};



template<typename Result, typename ... Parsers>
struct p_build : p_parser<Result>
{
    std::tuple<Parsers...> parsers;

    p_build(Parsers&&... parsers)
        : parsers(std::forward<Parsers>(parsers)...)
    { }

    maybe<Result> operator()(input& in)
    {
        static_assert(std::is_constructible_v<Result,
                                              typename Parsers::value_type...>,
                      "Result must be constructible from parser results");

        return parse<Result>(in, parsers);
    }

private:
    template<typename ... Args>
    using tuple_of_maybe = std::tuple<maybe<typename Args::value_type> ...>;


    template<int N = 0, typename ... Args>
    static void extract_tuple_rec(std::tuple<Args...>& out,
                                  const std::tuple<maybe<Args>...>& tuple)
    {
        if constexpr (N < sizeof...(Args))
        {
            if (!std::get<N>(tuple))
                throw std::runtime_error("maybe holds a ‹fail› value");

            std::get<N>(out) = std::get<N>(tuple).get();
            extract_tuple_rec<N + 1>(out, tuple);
        }
    }


    // tuple<maybe<A>, ..., maybe<N>>  ->  tuple<A, ..., N>
    template<typename ... Args>
    static auto extract_tuple(std::tuple<maybe<Args>...> tuple)
        -> std::tuple<Args...>
    {
        auto result = std::tuple<Args...>();
        extract_tuple_rec<0>(result, tuple);
        return result;
    }


    template<int N, typename Tuple, typename ... Args>
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


    template<typename ... Args>
    static maybe<Result> parse(input& in, std::tuple<Args...> parsers)
    {
        auto parse_results = tuple_of_maybe<Args ...>();

        bool ok = parse_from_nth<Result, 0>(in, parse_results, parsers);

        // TODO: print the error from the actual failed parser
        if (!ok)
            return fail("p_build failed");

        auto values = extract_tuple(std::move(parse_results));

        return std::make_from_tuple<Result>(std::move(values));
    }
};



struct p_space : p_parser<std::vector<char>>
{
    p_many<p_one_pred<is_space>> parser;

    auto operator()(input& in)
    {
        return parser(in);
    }
};



struct p_space1 : p_parser<std::vector<char>>
{
    p_many<p_one_pred<is_space>> parser;

    maybe<value_type> operator()(input& in)
    {
        auto r = parser(in);

        if (r.good() && r.get().empty())
            return fail("expected whitespace");

        return r;
    }
};



template<typename A, typename B>
struct p_after : p_parser<std::pair<typename A::value_type,
                                    typename B::value_type>>
{
    A first;
    B second;

    auto operator()(input& in)
    {
        auto a = first(in);
        auto b = second(in);
        return lift2(std::make_pair, std::move(a), std::move(b));
    }
};



// B  *>  T
template<typename B, typename T>
struct p_prefixed : p_parser<typename T::value_type>
{
    p_after<B, T> parser;

    auto operator()(input& in) -> maybe<typename T::value_type>
    {
        return parser(in)
            .fmap([](const auto& pair){ return pair.second; });
    }
};



// T  <*  B
template<typename T, typename B>
struct p_suffixed : p_parser<typename T::value_type>
{
    p_after<T, B> parser;

    auto operator()(input& in) -> maybe<typename T::value_type>
    {
        return parser(in)
            .fmap([](const auto& pair){ return pair.first; });
    }
};



struct p_alpha_str : p_parser<std::string>
{
    p_many<p_one_pred<is_alpha>> parser;

    p_alpha_str(unsigned min_len = 0) : parser(min_len)
    { }

    maybe<std::string> operator()(input& in)
    {
        return parser(in).fmap([](auto vec)
        {
            return std::string(vec.begin(), vec.end());
        });
    }
};



template<typename Result, typename ... Parsers>
struct p_try_seq : p_parser<Result>
{
    std::tuple<Parsers...> parsers;

    p_try_seq() : parsers() {}

    p_try_seq(Parsers&&... parsers)
        : parsers(std::forward<Parsers>(parsers)...) {}

    template<int N = 0>
    static maybe<Result> try_from_nth(input& in,
                                      std::tuple<Parsers...>& parsers)
    {
        if constexpr (N < sizeof...(Parsers))
        {
            auto prev = in.consumed();
            auto res = std::get<N>(parsers)(in);
            if (res)
                return res.get();

            // this parser consumed some input
            if (in.consumed() > prev)
                return res.get_fail();

            return try_from_nth<N + 1>(in, parsers);
        }

        // TODO: improve this error message
        return fail("expected one of [...], but none succeeded");
    }

    maybe<Result> operator()(input& in)
    {
        return try_from_nth(in, parsers);
    }
};



struct p_quoted : p_parser<std::string>
{
    char quote = '"';
    char escape = '\\';

    p_quoted() = default;
    p_quoted(char quote, char escape) : quote(quote), escape(escape) {}

    maybe<std::string> operator()(input& in)
    {
        auto get_got = [&]()
        {
            return !in.good() ? std::string("EOF")
                              : (std::string("'") += in.peek()) += "'";
        };

        if (in.end() || in.peek() != quote)
            return fail("expected opening '", quote, "', got ", get_got());

        // eat opening quote
        in.eat();

        auto result = std::string{};
        bool escaped = false;

        while (in.good())
        {
            if (!escaped && in.peek() == quote)
                break;

            if (in.peek() == escape)
            {
                if (escaped)
                    result += escape;

                escaped = !escaped;
                in.eat();
            }
            else
            {
                result += in.eat();
                escaped = false;
            }
        }

        if (in.end() || in.peek() != quote)
            return fail("expected closing '", quote, "', got ", get_got());

        // eat closing quote
        in.eat();

        return result;
    }
};


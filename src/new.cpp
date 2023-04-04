
#include "grammar/tree.hpp"


#include <iostream>



int func(int& a, int b, int c)
{
    a = 10;
    return a + b * c;
}

// struct neco_t
// {
//     char a;
//     int b;
//     neco_t(char a, int b) : a(a), b(b) {}
// };


struct neco_t
{
    char a, b, c;
    neco_t(char a, char b, char c) : a(a), b(b), c(c) {}
};


// struct neco_t
// {
//     char a;
//     neco_t(char a) : a(a) {}
// };


int main()
{
    // auto a = maybe<int>(5);
    // auto b = a
    //         .and_then([](auto i) -> maybe<std::string>
    //                 {
    //                     if (i == 4)
    //                         return { fail{ "ahoj" } };
    //                     return { std::string{ "ahoj" } };
    //                 })
    //         .fmap([](const auto&)
    //                 {
    //                     return 8;
    //                 });

    // std::cout << b.get() << std::endl;

    // int n = 0;
    // auto lam = curry(func, n, 2, 3);
    // std::cout << lam << std::endl;
    // std::cout << n << std::endl;

    // {
    //     auto tup = std::make_tuple(1, 2, 3);
    //     auto res = curry_tuple(func, tup);

    //     // res.sdadsad();

    //     // std::cout << res << std::endl;
    // }

    {
        auto borek = p_build<neco_t, p_char, p_char, p_char>( p_char{ 'a' },
                                                              p_char{ 'b' },
                                                              p_char{ 'c' } );
        // auto borek = p_build<neco_t, p_char>( p_char{ 'a' } );
        auto in = input("abc");
        auto mby = borek(in);
        std::cout << mby.good() << std::endl;
    }

    // {
    //     auto tup = std::make_tuple('c', 1);
    //     // auto res = curry_tuple(neco_t::make, tup);
    //     auto dummy = std::make_tuple(1);
    //     neco_t res = std::pair<neco_t, int>(std::piecewise_construct, tup, dummy).first;
    // }

    return 0;
}

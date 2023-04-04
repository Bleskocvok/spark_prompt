
#include "grammar/parsing.hpp"
#include "grammar/grammar.hpp"


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
    {
        auto borek = p_many
        {
            p_one_of{ { 'a', 'b', 'c' } },
        };
        // auto borek = p_build<neco_t, p_char>( p_char{ 'a' } );
        auto in = input("");
        auto mby = borek(in);

        auto print = [](const auto& thing)
        {
            std::cout << std::boolalpha << thing << std::endl;
        };

        mby.visit(print, print);
    }

    // {
    //     auto tup = std::make_tuple('c', 1);
    //     // auto res = curry_tuple(neco_t::make, tup);
    //     auto dummy = std::make_tuple(1);
    //     neco_t res = std::pair<neco_t, int>(std::piecewise_construct, tup, dummy).first;
    // }

    std::cout << literal_color(rgb{ 255, 128, 0 }) << std::endl;

    return 0;
}

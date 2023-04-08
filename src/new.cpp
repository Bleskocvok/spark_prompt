
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


// struct neco_t
// {
//     char a, b, c;
//     neco_t(char a, char b, char c) : a(a), b(b), c(c) {}
// };


// struct neco_t
// {
//     char a;
//     neco_t(char a) : a(a) {}
// };



int main()
{
    {
        auto borek = p_node{};
        auto in = input("(ahoj #ffffff \n (asdhasdaskdk) 'ahoj' \"lolololo\")");
        auto mby = borek(in);

        auto print = [](const auto& thing)
        {
            std::cout << std::boolalpha << thing << std::endl;
        };

        mby.visit(print, print);
    }

    std::cout << literal_color(rgb{ 255, 128, 0 }) << std::endl;

    return 0;
}

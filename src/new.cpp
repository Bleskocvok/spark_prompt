
#include "grammar/parsing.hpp"
#include "grammar/grammar.hpp"
#include "grammar/evaluate.hpp"


#include <iostream>



int main()
{
    {
        auto borek = p_sequence{};
        auto in = input("  [ (ahoj #ffffff \n (asdhasdaskdk true) "
                        "{ #ff00ff true } ( a) true false 'ahoj' \"lolololo\") >> ]   ");
        // auto in = input("( ahoj #ffffff    (a ) (bbbbb (bbb))\n )");
        auto mby = borek(in);

        auto print = [](const auto& thing)
        {
            std::cout << std::boolalpha << thing << std::endl;
        };

        mby.visit(print, print);

        if (mby)
        {
            auto node = mby.get();
            auto eval = evaluator{};
            auto result = eval(node);

            for (unsigned i = 0; i < result.size(); ++i)
                std::cout << "[" << i << "].index="
                          << result[i].index()
                          << std::endl;
        }
    }

    std::cout << literal_color(rgb{ 255, 128, 0 }) << std::endl;

    return 0;
}

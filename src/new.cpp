
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
        maybe<std::vector<node_ptr>> parsed = borek(in);

        auto print = [](const auto& thing)
        {
            std::cout << std::boolalpha << thing << std::endl;
        };

        parsed.visit(print, print);

        if (parsed)
        {
            auto node = parsed.get();
            auto eval = evaluator{};
            maybe<style> result = eval(node);

            result.visit([](const style& st)
            {
                std::cout << "style" << std::endl;
                st.render(std::cout);
            },
            [](const fail& f)
            {
                std::cout << "fail: " << f << std::endl;
            });
        }
    }

    std::cout << literal_color(rgb{ 255, 128, 0 }) << std::endl;

    return 0;
}

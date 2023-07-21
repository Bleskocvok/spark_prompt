
#include "grammar/parsing.hpp"
#include "grammar/grammar.hpp"
#include "grammar/evaluate.hpp"


#include <iostream>



struct lolol : builtin_func<>
{
    evaluated perform() override
    {
        return std::string{ "lolol" };
    }
};


struct if_then_else : builtin_func<typ::boolean, typ::any, typ::any>
{
    evaluated perform(bool b, evaluated t, evaluated f) override
    {
        return b ? t : f;
    }
};



int main()
{
    {
        auto borek = p_sequence{};
        // auto in = input("  [ (ahoj #ffffff \n (asdhasdaskdk true) "
        //                 "{ #ff00ff true } ( a) true false 'ahoj' \"lolololo\") >> ]   ");
        auto in = input("  [ { #ff00ff #0000ff #0000ff } (if true 'its tru' (lolol)) >> ]   ");
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

            eval.add_func("lolol", std::make_unique<lolol>());
            eval.add_func("if", std::make_unique<if_then_else>());

            maybe<style> result = eval(node);

            result.visit([](const style& st)
            {
                std::cout << "style" << std::endl;
                st.render(std::cout);
                std::cout << "\n";
            },
            [](const fail& f)
            {
                std::cout << "fail: " << f << std::endl;
            });
        }
    }

    // std::cout << literal_color(rgb{ 255, 128, 0 }) << std::endl;

    // std::unique_ptr<func> mk = std::make_unique<mk_theme>();
    // auto params = std::vector<evaluated>{ color{}, fail{}, color{} };

    // auto check = mk->check_types(params);
    // if (check)
    // {
    //     auto[i, err] = *check;
    //     std::cout << "check fail: " << i << "; err: " << err << std::endl;
    // }

    // auto result = (*mk)(params);
    // std::cout << "result.index(): " << result.index() << std::endl;

    // if (result.index() == 0)
    // {
    //     std::cout << "result: " << std::get<fail>(result) << std::endl;
    // }

    return 0;
}

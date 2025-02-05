#include "grammar.hpp"

#include "parsing.hpp"

#include <utility>      // move


maybe<node_ptr> p_call::operator()(input& in)
{
    using p_name = p_str_pred<pred_or<is_char<'_'>, is_alpha>, 1>;
    // using p_name = p_str_pred<is_alpha, 1>;

    auto parser = p_between<p_spaces_after<p_char<'('>>,
                            p_after<p_spaces_after<p_name>,
                                    p_many<p_node>>,
                            p_char<')'>>{};

    return parser(in)
        .fmap([](auto pair)
        {
            return make_node<call>(std::move(pair.first),
                                   std::move(pair.second));
        });
}


maybe<node_ptr> p_composite_color::operator()(input& in)
{
    auto parser = p_between<p_char<'{'>,
                            p_many<p_node>,
                            p_char<'}'>>{};

    return parser(in)
        .fmap([](auto nodes)
        {
            return make_node<composite_color>(std::move(nodes));
        });
}

maybe<node_ptr> p_composite_segment::operator()(input& in)
{
    auto parser = p_between<p_char<'['>,
                            p_many<p_node>,
                            p_char<']'>>{};

    return parser(in)
        .fmap([](auto nodes)
        {
            return make_node<composite_segment>(std::move(nodes));
        });
}

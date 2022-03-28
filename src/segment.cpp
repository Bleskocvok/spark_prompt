
#include "segment.hpp"

#include "unicode.hpp"

#include <iostream>     // cout
#include <utility>      // pair, move
#include <unordered_map>
#include <cctype>
#include <stdexcept>


using namespace std::literals;
using namespace std::string_literals;


size_t sep_len(sep s)
{
    switch (s)
    {
        case sep::empty:                return 0;

        case sep::space:                // [[fallthrough]]
        case sep::newline:              return 1;

        case sep::powerline:            // [[fallthrough]]
        case sep::rpowerline:           // [[fallthrough]]
        case sep::powerline_pseudo:     // [[fallthrough]]
        case sep::rpowerline_pseudo:    return 1;

        case sep::powerline_space:      // [[fallthrough]]
        case sep::rpowerline_space:     return 2;
    }

    throw std::logic_error{ "invalid separator" };
}


void render_sep(std::ostream& out, sep s, color fg, color bg,
                     color next_bg)
{
    static const auto arrow = "\uE0B0"s;
    static const auto pseudo = "▶"s;
    static const auto rarrow = "\uE0B2"s;
    static const auto rpseudo = "◀"s;

    switch (s)
    {
        case sep::empty:
            out << fg_color_str(bit3::reset);
            break;

        case sep::space:
            out << fg_color_str(bit3::reset)
                << " ";
            break;

        case sep::powerline:
            out << fg_color_str(bit3::reset)  // needed to cancel font effect
                << bg_color_str(next_bg)
                << fg_color_str(bg)
                << arrow;
            break;

        case sep::powerline_space:
        {
            // TODO: add option to change color of “thick”
            color thick = rgb{ 0, 0, 0 };
            out << bg_color_str(thick)
                << fg_color_str(bg)
                << arrow
                << fg_color_str(thick)
                << bg_color_str(next_bg)
                << arrow;
            break;
        }

        case sep::powerline_pseudo:
            out << bg_color_str(next_bg)
                << fg_color_str(bg)
                << pseudo;
            break;

        case sep::newline:
            out << bg_color_str(bit3::reset)
                << "\n";
            break;

        case sep::rpowerline:
            out << fg_color_str(bit3::reset)  // needed to cancel font effect
                << bg_color_str(bg)
                << fg_color_str(next_bg)
                << rarrow;
            break;

        case sep::rpowerline_space:
        {
            // TODO: add option to change color of “thick”
            color thick = rgb{ 0, 0, 0 };
            out << bg_color_str(bg)
                << fg_color_str(thick)
                << rarrow
                << fg_color_str(next_bg)
                << bg_color_str(thick)
                << rarrow;
            break;
        }

        case sep::rpowerline_pseudo:
            out << bg_color_str(bg)
                << fg_color_str(next_bg)
                << rpseudo;
            break;
    }
}


size_t segment::width() const
{
    return unicode_size(str) + sep_len(end);
}

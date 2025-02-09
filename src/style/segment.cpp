#include "segment.hpp"

// custom
#include "../unicode.hpp"

// cpp
#include <stdexcept>    // logic_error
#include <string>       // ""s
#include <string_view>  // ""sv


using namespace std::literals;
using namespace std::string_literals;


size_t segment::width() const
{
    return unicode_size(str) + sep_len(end);
}


size_t sep_len(sep s)
{
    switch (s)
    {
        case sep::newline:              // [[fallthrough]]
        case sep::fill:                 // [[fallthrough]]
        case sep::empty:                return 0;

        case sep::space:                // [[fallthrough]]
        case sep::powerline:            // [[fallthrough]]
        case sep::rpowerline:           // [[fallthrough]]
        case sep::powerline_pseudo:     // [[fallthrough]]
        case sep::slope:                // [[fallthrough]]
        case sep::rslope:               // [[fallthrough]]
        case sep::rpowerline_pseudo:    return 1;

        case sep::slope_space:          // [[fallthrough]]
        case sep::rslope_space:         // [[fallthrough]]
        case sep::powerline_space:      // [[fallthrough]]
        case sep::rpowerline_space:     return 2;
    }

    throw std::logic_error{ "invalid separator" };
}


static void space_thing(std::ostream& out, color bg, color next_bg,
                        const std::string& symbol, bool reversed = false)
{
    // TODO: add option to change color of “thick”
    color thick = rgb{ 0, 0, 0 };
    out << bg_color_str(reversed ? bg    : thick)
        << fg_color_str(reversed ? thick : bg)
        << symbol
        << fg_color_str(reversed ? next_bg : thick)
        << bg_color_str(reversed ? thick   : next_bg)
        << symbol;
}


static void arrow_thing(std::ostream& out, color bg, color next_bg,
                        const std::string& symbol, bool reversed = false)
{
    out << fg_color_str(bit3::reset)  // needed to cancel font effect
        << (reversed ? bg_color_str(bg)      : bg_color_str(next_bg))
        << (reversed ? fg_color_str(next_bg) : fg_color_str(bg))
        << symbol;
}


void render_sep(std::ostream& out, const segment& seg, color next_bg)
{
    static const auto arrow = "\uE0B0"s;
    static const auto pseudo = "▶"s;
    static const auto rarrow = "\uE0B2"s;
    static const auto rpseudo = "◀"s;

    static const auto slope = "\ue0b8"s;
    static const auto rslope = "\ue0ba"s;

    auto s = seg.end;
    auto bg = seg.th.bg;

    switch (s)
    {
        case sep::fill:
        case sep::empty:    out << fg_color_str(bit3::reset);         break;
        case sep::space:    out << fg_color_str(bit3::reset) << " ";  break;
        case sep::newline:  out << bg_color_str(bit3::reset) << "\n"; break;

        case sep::powerline_pseudo:     arrow_thing(out, bg, next_bg, pseudo); break;
        case sep::powerline:            arrow_thing(out, bg, next_bg, arrow); break;
        case sep::slope:                arrow_thing(out, bg, next_bg, slope); break;
        case sep::rpowerline_pseudo:    arrow_thing(out, bg, next_bg, rpseudo, true); break;
        case sep::rpowerline:           arrow_thing(out, bg, next_bg, rarrow, true); break;
        case sep::rslope:               arrow_thing(out, bg, next_bg, rslope, true); break;

        case sep::slope_space:          space_thing(out, bg, next_bg, slope); break;
        case sep::powerline_space:      space_thing(out, bg, next_bg, arrow); break;
        case sep::rslope_space:         space_thing(out, bg, next_bg, rslope, true); break;
        case sep::rpowerline_space:     space_thing(out, bg, next_bg, rarrow, true); break;
    }
}

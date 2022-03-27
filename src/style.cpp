
#include "style.hpp"

#include <iostream>     // cout
#include <utility>      // pair, move
#include <unordered_map>
#include <cctype>

#include <sys/ioctl.h>


using namespace std::literals;
using namespace std::string_literals;


void style::render(std::ostream& out) const
{
    static const auto arrow = "\uE0B0"s;
    static const auto pseudo = "▶"s;

    for (size_t i = 0; i < segments.size(); i++)
    {
        out << bg_color_str(segments[i].th.bg)
            << fg_color_str(segments[i].th.fg, segments[i].th.ef);

        out << segments[i].str;

        color next = i != segments.size() - 1 ? segments[i + 1].th.bg
                                              : bit3::reset;

        switch (segments[i].end)
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
                    << bg_color_str(next)
                    << fg_color_str(segments[i].th.bg)
                    << arrow;
                break;

            case sep::powerline_space:
            {
                // TODO: add option to change color of “thick”
                color thick = rgb{ 0, 0, 0 };
                out << bg_color_str(thick)
                    << fg_color_str(segments[i].th.bg)
                    << arrow
                    << fg_color_str(thick)
                    << bg_color_str(next)
                    << arrow;
                break;
            }

            case sep::powerline_pseudo:
                out << bg_color_str(next)
                    << fg_color_str(segments[i].th.bg)
                    << pseudo;
                break;

            case sep::newline:
                out << bg_color_str(bit3::reset)
                    << "\n";
                break;

            case sep::rpowerline:
                out << fg_color_str(bit3::reset)  // needed to cancel font effect
                    << bg_color_str(segments[i].th.bg)
                    << fg_color_str(next)
                    << rarrow;
                break;

            case sep::rpowerline_space:
            {
                // TODO: add option to change color of “thick”
                color thick = rgb{ 0, 0, 0 };
                out << bg_color_str(segments[i].th.bg)
                    << fg_color_str(thick)
                    << rarrow
                    << fg_color_str(next)
                    << bg_color_str(thick)
                    << rarrow;
                break;
            }

            case sep::rpowerline_pseudo:
                out << bg_color_str(segments[i].th.bg)
                    << fg_color_str(next)
                    << rpseudo;
                break;
        }
    }
}


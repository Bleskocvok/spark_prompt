
#include "style.hpp"

#include <iostream>     // cout
#include <utility>      // pair, move
#include <unordered_map>
#include <cctype>

#include <sys/ioctl.h>


template<typename It>
static size_t total_width(It begin, It end)
{
    size_t res = 0;
    for (; begin != end; ++begin)
    {
        res += begin->width();
    }
    return res;
}


static void spaces(std::ostream& out, int i)
{
    for (int j = 0; j < i; j++)
        out << " ";
}


void style::render(std::ostream& out) const
{
    struct winsize win;
    ioctl(1, TIOCGWINSZ, &win);
    const int twidth = win.ws_col;

    int w = 0;
    for (size_t i = 0; i < segments.size(); i++)
    {
        out << bg_color_str(segments[i].th.bg)
            << fg_color_str(segments[i].th.fg, segments[i].th.ef);

        out << segments[i].str;

        color next = i != segments.size() - 1 ? segments[i + 1].th.bg
                                              : bit3::reset;

        render_sep(out, segments[i].end, segments[i].th.fg,
                   segments[i].th.bg, next);

        w += segments[i].width();

        if (segments[i].h_space)
        {
            // size_t rest = total_width(segments.begin() + i + 1, segments.end());
            // spaces(out, twidth - (rest + w));
        }
    }
}


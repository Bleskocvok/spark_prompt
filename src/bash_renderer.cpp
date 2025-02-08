#include "bash_renderer.hpp"

// cpp
#include <ostream>      // ostream

// linux
#include <sys/ioctl.h>  // ioctl

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

void bash_renderer::render(const style& stl, std::ostream& out) const
{
    struct winsize win;
    ::ioctl(2, TIOCGWINSZ, &win);
    const int twidth = win.ws_col - 2;

    int w = 0;
    for (size_t i = 0; i < stl.segments.size(); i++)
    {
        out << bg_color_str(stl.segments[i].th.bg)
            << fg_color_str(stl.segments[i].th.fg, stl.segments[i].th.ef);

        out << stl.segments[i].str;

        color next = i != stl.segments.size() - 1 ? stl.segments[i + 1].th.bg
                                              : bit3::reset;

        render_sep(out, stl.segments[i], next);

        w += stl.segments[i].width();

        // if (stl.segments[i].h_space)
        if (stl.segments[i].end == sep::space)
        {
            size_t rest = total_width(stl.segments.begin() + i + 1, stl.segments.end());
            spaces(out, twidth - (rest + w));
        }
    }
}

#include "bash_renderer.hpp"
#include "style/style.hpp"
#include "style/color.hpp"
#include "style/segment.hpp"

// cpp
#include <cstring>
#include <ostream>      // ostream

// linux
#include <sys/ioctl.h>  // ioctl

namespace {

template<typename It>
size_t total_width(It begin, It end)
{
    size_t res = 0;
    for (; begin != end; ++begin)
    {
        res += begin->width();
    }
    return res;
}

void spaces(std::ostream& out, int i)
{
    for (int j = 0; j < i; j++)
        out << " ";
}

} // namespace

void bash_renderer::render(const style& stl, std::ostream& out) const
{
    struct winsize win;
    std::memset(&win, 0, sizeof win);
    // NOLINTNEXTLINE(misc-include-cleaner)
    ::ioctl(2, TIOCGWINSZ, &win);
    const int twidth = win.ws_col - 2;

    int w = 0;
    for (size_t i = 0; i < stl.segments.size(); i++)
    {
        out << bg_color_str(stl.segments[i].th.bg)
            << fg_color_str(stl.segments[i].th.fg, stl.segments[i].th.ef);

        out << stl.segments[i].str;

        color next = i != stl.segments.size() - 1 ? stl.segments[i + 1].th.bg
                                              : bit3::black;

        render_sep(out, stl.segments[i], next);

        w += stl.segments[i].width();

        // TODO: Account for width continuously and reset on newline.
        // if (stl.segments[i].h_space)
        if (stl.segments[i].end == sep::fill)
        {
            size_t rest = total_width(stl.segments.begin() + i + 1, stl.segments.end());
            spaces(out, twidth - (rest + w));
        }
    }
}

#include "common.hpp"

#include "format_sup.hpp"

bool load_sup(std::istream* in, std::list<Subtitle> subs)
{
    unsigned int count = 0;
    while (!in->eof() && !in->bad())
    {
        Subtitle subtitle;
        // if (!read_section(in, subtitle))
        // {
        //     return false;
        // }
    }
    return count > 0 && !in->bad();
}

bool save_sup(std::ostream* out, std::list<Subtitle> subs)
{
    return false;
}

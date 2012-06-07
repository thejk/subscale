#ifndef SUBS_HPP
#define SUBS_HPP

#if HAVE_CSTDINT
# include <cstdint>
#else
extern "C" {
# include <stdint.h>
};
#endif

#include <list>

class Sub
{
public:
    unsigned long start_s, start_ms;
    unsigned long duration_ms;

    unsigned int width, height;
    uint32_t* rgba;
};

class Subs
{
public:
    typedef std::list<Sub> subs_t;

    subs_t subs;
};

#endif /* SUBS_HPP */

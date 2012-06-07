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

typedef uint32_t u32;
typedef uint64_t u64;
class Sub
{
public:
	Sub(u32 w, u32 h)
	: width(w), height(h) {}
	
    u64 start_s, start_ms;
    u64 duration_ms;

    u32 width, height;
    u32* rgba;
};

class Subs
{
public:
    typedef std::list<Sub> subs_t;

    subs_t subs;
};

#endif /* SUBS_HPP */

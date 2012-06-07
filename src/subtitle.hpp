#ifndef SUBTITLE_HPP
#define SUBTITLE_HPP

#if HAVE_CSTDINT
# include <cstdint>
#else
extern "C" {
# include <stdint.h>
};
#endif

#include <list>
#include <string>

typedef uint32_t u32;
typedef uint64_t u64;

class SubImage
{
public:
	SubImage(u32 w, u32 h)
	: width(w), height(h) {}

    u64 start_s, start_ms;
    u64 duration_ms;

    u32 width, height;
    u32* rgba;
};

class Subtitle
{
public:
    std::string title, lang;

    typedef std::list<SubImage> subimages_t;

    subimages_t images;
};

#endif /* SUBTITLE_HPP */

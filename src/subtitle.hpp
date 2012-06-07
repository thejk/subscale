#ifndef SUBTITLE_HPP
#define SUBTITLE_HPP

#include "common.hpp"
#include <list>
#include <string>

class SubImage
{
public:
	SubImage(u32 w, u32 h)
	: width(w), height(h) {
		rgba = new u32[w*h];
	}

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

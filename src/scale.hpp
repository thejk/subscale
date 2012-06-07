#ifndef SCALE_HPP
#define SCALE_HPP

#include "subtitle.hpp"

SubImage scale_nn(const SubImage& sub, float scale, bool debug = false) {
	u32 scaled_width = sub.width * scale;
	u32 scaled_height = sub.height * scale;
	printf("old size (%d, %d)\n", sub.width, sub.height);
	SubImage scaled(scaled_width, scaled_height);
	for(u32 x = 0; x < scaled.width; ++x) {
		for(u32 y = 0; y < scaled.height; ++y) {
			u32 oldx = x/scale;
			u32 oldy = y/scale;
			if(debug)
				printf("from (%d, %d) to (%d, %d)\n", oldx, oldy, x, y);
			scaled.rgba[x+scaled.width*y] = sub.rgba[oldx + oldy * sub.width];
		}
	}
	printf("new size (%d, %d)\n", scaled.width, scaled.height);
	return scaled;
}

#endif /* SCALE_HPP */

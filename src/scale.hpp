#ifndef SCALE_HPP
#define SCALE_HPP

#include "subtitle.hpp"

SubImage scale_nn(const SubImage& sub, float scale) {
	int scaled_width = sub.width * scale;
	int scaled_height = sub.height * scale;
	SubImage scaled(scaled_width, scaled_height);
	for(int x = 0; x < scaled.width; ++x) {
		for(int y = 0; y < scaled.height; ++y) {
			int oldx = x*scale;
			int oldy = y*scale;
			printf("scaling (%d, %d) -> (%d, %d)\n", oldx, oldy, x, y);
			scaled.rgba[x+scaled.width*y] = sub.rgba[oldx + oldy * sub.width];
		}
	}
	return scaled;
}

#endif /* SCALE_HPP */

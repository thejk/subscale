#ifndef SCALE_HPP
#define SCALE_HPP

#include <math.h>
#include "subtitle.hpp"
#include "iostream"

struct Pixel {
	Pixel(u32 rgba) {
		data = new u8[4];
		memcpy(data, &rgba, 4);
	}
	Pixel(u8 r, u8 g, u8 b, u8 a) {
		data = new u8[4];
		data[0] = r;
		data[1] = g;
		data[2] = b;
		data[3] = a;
	}
	Pixel operator*(float val) {
		Pixel retr(data[0] * val, data[1] * val, data[2] * val, data[4] * val);
		return retr;
	}
	Pixel operator*(u32 val) {
		Pixel retr(data[0] * val, data[1] * val, data[2] * val, data[4] * val);
		return retr;
	}
	Pixel operator/(float val) {
		Pixel retr(data[0] / val, data[1] / val, data[2] / val, data[4] / val);
		return retr;
	}
	Pixel operator+(const Pixel& rhs) {
		u8 r = data[0] + rhs.data[0];
		u8 g = data[1] + rhs.data[1];
		u8 b = data[2] + rhs.data[2];
		u8 a = data[3] + rhs.data[3];
		return Pixel(r, g, b, a);
	}
	operator u32() {
		u32 retr;
		memcpy(&retr, data, 4);
		return retr;
	}
	u8* data;
};
std::ostream& operator<<(std::ostream& out, Pixel& p) {
	out <<"(" <<p.data[0] <<", " <<p.data[1] <<", " <<p.data[2] <<")";
	return out;
}

struct NNScaler {
	void operator()(const SubImage& old, SubImage& scaled, float scale, u32 newx, u32 newy, bool debug) {
		u32 oldx = newx/scale;
		u32 oldy = newy/scale;
		if(debug)
			printf("from (%d, %d) to (%d, %d)\n", oldx, oldy, newx, newy);
		scaled.rgba[newx + scaled.width * newy] = old.rgba[oldx + oldy * old.width];
	}
};

struct BLScaler {
	void operator()(const SubImage& old, SubImage& scaled, float scale, u32 newx, u32 newy, bool debug) {
		u32 q1x = ceil(newx/scale);
		u32 q1y = ceil(newy/scale);
		u32 q2x = floor(newx/scale) + 1;
		u32 q2y = floor(newy/scale) + 1;

		if(debug)
			printf("from q1(%d, %d) and q2(%d, %d) to (%d, %d)\n", q1x, q1y, q2x, q2y, newx, newy);

		Pixel p1 = Pixel(old.rgba[q1x + old.width * q1y]) * (1.0f-newx/scale) * (1.0f-newy/scale);
		Pixel p2 = Pixel(old.rgba[q2x + old.width * q1y]) * newx * (1.0f - newy/scale);
		Pixel p3 = Pixel(old.rgba[q1x + old.width * q2y]) * (1.0f - newx/scale) * newy;
		Pixel p4 = Pixel(old.rgba[q2x + old.width * q2y]) * newx * newy;

		scaled.rgba[newx + newy * scaled.width] = p1 + p2 + p3 + p4;
	}
};

template <class scalerType>
SubImage scale_helper(const SubImage& sub, float scale, scalerType scaler, bool debug) {
	if(debug)
		printf("old size (%d, %d)\n", sub.width, sub.height);
	u32 scaled_width = sub.width * scale;
	u32 scaled_height = sub.height * scale;
	SubImage scaled(scaled_width, scaled_height);
	for(u32 x = 0; x < scaled.width; ++x) {
		for(u32 y = 0; y < scaled.height; ++y) {
			scaler(sub, scaled, scale, x, y, debug);
		}
	}
	if(debug)
		printf("new size (%d, %d)\n", scaled.width, scaled.height);
	return scaled;
}

SubImage scale_nn(const SubImage& sub, float scale, bool debug = false) {
	return scale_helper(sub, scale, NNScaler(), debug);
}

SubImage scale_bl(const SubImage& sub, float scale, bool debug = false) {
	return scale_helper(sub, scale, BLScaler(), debug);
}

#endif /* SCALE_HPP */

#include "scale.hpp"
#include <iostream>
using namespace std;

void test_scale() {
	u32 size = 4;
	SubImage img(size,size);
	for(u32 y = 0; y < 4; ++y) {
		for(u32 x = 0; x < 4; ++x) {
			img.rgba[x+4*y] = x+4*y;
		}
	}
	cout <<"Testing nearest-neighbor scaling" <<endl;
	SubImage scaled_nn = scale_nn(img, 0.5f, true);
	cout <<"done" <<endl;

	cout <<"Testing bilinear scaling" <<endl;
	SubImage scaled_bl = scale_bl(img, 0.5f, true);
	cout <<"done" <<endl;
}

int main(int argc, char** argv)
{
	(void)argc;
	(void)argv;
	test_scale();
    return 0;
}

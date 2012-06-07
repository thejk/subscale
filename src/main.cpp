#include "common.hpp"

#include "scale.hpp"
#include "format_sup.hpp"
#include <iostream>
#include <iomanip>
#include <fstream>
using namespace std;

void verify_nn(SubImage& sub) {
	assert(sub.rgba[0] == 0);
	assert(sub.rgba[3] == 10);
}
void verify_bl(SubImage& sub) {
	assert(sub.rgba[0] == 0);
}

void test_scale() {
	u32 size = 4;
	SubImage img(size,size);
	cout <<"Image:" <<endl;
	for(u32 y = 0; y < 4; ++y) {
		for(u32 x = 0; x < 4; ++x) {
			img.rgba[x+4*y] = x + 4 * y;
			cout <<setw(2) <<x+4*y <<" ";
		}
		cout <<endl;
	}
	cout <<"Testing nearest-neighbor scaling" <<endl;
	SubImage scaled_nn = scale_nn(img, 0.5f, false);
	verify_nn(scaled_nn);
	cout <<"done" <<endl;

	cout <<"Testing bilinear scaling" <<endl;
	SubImage scaled_bl = scale_bl(img, 0.5f, true);
	verify_bl(scaled_bl);
	cout <<"done" <<endl;
}

int main(int argc, char** argv)
{
    if (argc < 1)
    {
        test_scale();
        return 0;
    }
    else
    {
        std::list<Subtitle> subtitles;
        std::ifstream* in = new std::ifstream();
        in->open(argv[1], std::ios_base::in | std::ios_base::binary);
        return load_sup(in, subtitles) ? 0 : -1;
    }
}

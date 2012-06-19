#include <iostream>
#include <iomanip>
#include <fstream>

#include "common.hpp"
#include "scale.hpp"
#include "format_sup.hpp"
#include "bitmap.hpp"

using namespace std;

void verify_nn(SubImage& sub) {
	cout <<"NN scaled image: " <<endl;
	for(u32 y = 0; y < sub.height; ++y) {
		for(u32 x = 0; x < sub.width; ++x) {
			cout <<setw(2) <<sub.rgba[x+y*sub.width] <<" ";
		}
		cout <<endl;
	}
	assert(sub.rgba[0] == 0);
	assert(sub.rgba[3] == 10);
}

void verify_bl(SubImage& sub) {
	assert(sub.rgba[0] == 0);
	cout <<"BL scaled image: " <<endl;
	for(u32 y = 0; y < sub.height; ++y) {
		for(u32 x = 0; x < sub.width; ++x) {
			cout <<setw(2) <<sub.rgba[x+y*sub.width] <<" ";
		}
		cout <<endl;
	}
}

void test_scale() {
	u32 size = 4;
	SubImage img(size,size);
	cout <<"Image:" <<endl;
	for(u32 y = 0; y < size; ++y) {
		for(u32 x = 0; x < size; ++x) {
			img.rgba[x + size * y] = x + size * y;
			cout <<setw(2) <<x + size * y <<" ";
		}
		cout <<endl;
	}
	cout <<"Testing nearest-neighbor scaling" <<endl;
	SubImage scaled_nn = scale_nn(img, 0.5f, false);
	verify_nn(scaled_nn);
	cout <<"done" <<endl;

	cout <<"Testing bilinear scaling" <<endl;
	SubImage scaled_bl = scale_bl(img, 0.5f, false);
	verify_bl(scaled_bl);
	cout <<"done" <<endl;
}

int main(int argc, char** argv)
{
    if (argc < 2)
    {
        test_scale();
        return 0;
    }
    else if(*argv[1] == 'r')
    {
        std::list<Subtitle> subtitles;
        std::ifstream* in = new std::ifstream();
        in->open(argv[2], std::ios_base::in | std::ios_base::binary);
        return load_sup(in, subtitles) ? 0 : -1;
    }
    else if(*argv[1] == 'w')
    {
    	/*std::list<Subtitle> subtitles;
        std::ifstream* in = new std::ifstream();
        in->open(argv[2], std::ios_base::in | std::ios_base::binary);
        load_sup(in, subtitles);*/
        u32 size = 16;
		SubImage img(size,size);
		for(u32 y = 0; y < size; ++y) {
			for(u32 x = 0; x < size; ++x) {
				img.rgba[x + size * y] = (x + size * y) <<24 | 0xFF;
			}
		}
        writeBitmap("test.bmp", img);
    }
}

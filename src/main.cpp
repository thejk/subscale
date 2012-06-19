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
    if (*argv[1] == 't')
    {
        test_scale();
        return 0;
    }
    else if(*argv[1] == 'w')
    {
    	assert(argc == 4);
    	float factor = atof(argv[2]);
    	cout <<"Scaling factor " <<factor <<endl;
    	std::list<Subtitle> subtitles;
        std::ifstream* in = new std::ifstream();
        in->open(argv[3], std::ios_base::in | std::ios_base::binary);
        load_sup(in, subtitles);
        unsigned int i = 1;
        for (std::list<Subtitle>::iterator sub(subtitles.begin()); sub != subtitles.end(); ++sub, ++i)
        {
            unsigned int j = 1;
            for (Subtitle::subimages_t::iterator subimg(sub->images.begin()); subimg != sub->images.end(); ++subimg, ++j)
            {
                char filename[50];
                snprintf(filename, sizeof(filename), "test%02u-%02u.bmp",
                         i, j);
                SubImage scaled = scale_bl(*subimg, factor);
                writeBitmap(filename, scaled);
            }
        }
    }
}

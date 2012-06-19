#include "format_sup.hpp"
#include <fstream>
using namespace std;

struct FileHeader {
	FileHeader() :
	   id1(0x42), id2(0x4d), fileSize(0), reserved(0), offset(54) 
	{ }

	//file header
	u8 id1, id2;
	u32 fileSize;
	u32 reserved;
	u32 offset;
};
ostream& operator<<(ostream& stream, FileHeader header) {
	cout <<"before FileHeader write, pos = " <<stream.tellp() <<endl;
	stream.write((const char*)&header.id1, 1);
	stream.write((const char*)&header.id2, 1);
	stream.write((const char*)&header.fileSize, 4);
	stream.write((const char*)&header.reserved, 4);
	stream.write((const char*)&header.offset, 4);
	cout <<"after FileHeader write, pos = " <<stream.tellp() <<endl;
	return stream;
}

struct DIBHeader {
	DIBHeader(s32 w, s32 h) :
	  headerSize(40), width(w), height(h),
	  numColourPanes(1), bitsPerPixel(32),
	  compression(0) /* none */, dataSize(0), 
	  horisontalRes(0), verticalRes(0), 
	  numColourInPalette(0), numImportColours(0)
	{}

	//dib header
	u32 headerSize;
	s32 width;
	s32 height;
	u16 numColourPanes;
	u16 bitsPerPixel;
	u32 compression;
	u32 dataSize;
	s32 horisontalRes;
	s32 verticalRes;
	u32 numColourInPalette;
	u32 numImportColours;
};
ostream& operator<<(ostream& stream, DIBHeader header) {
	cout <<"before DIBHeader write, pos = " <<stream.tellp() <<endl;
	stream.write((const char*)&header.headerSize, 4);
	stream.write((const char*)&header.width, 4);
	stream.write((const char*)&header.height, 4);
	stream.write((const char*)&header.numColourPanes, 2);
	stream.write((const char*)&header.bitsPerPixel, 2);
	stream.write((const char*)&header.compression, 4);
	stream.write((const char*)&header.dataSize, 4);
	stream.write((const char*)&header.horisontalRes, 4);
	stream.write((const char*)&header.verticalRes, 4);
	stream.write((const char*)&header.numColourInPalette, 4);
	stream.write((const char*)&header.numImportColours, 4);
	cout <<"after DIBHeader write, pos = " <<stream.tellp() <<endl;
	return stream;
}

void writeFileHeader(ofstream& stream) {
	stream <<FileHeader();
	stream.flush();
}

void writeDIBHeader(ofstream& stream, SubImage& sub) {
	stream <<DIBHeader(sub.width, sub.height);
	stream.flush();
}

inline u32 rgba_to_bgr(u32 rgba) {
	u8* data = new u8[4];
	data[0] = (rgba & 0x0000FF00) >>8; //blue
	data[1] = (rgba & 0x00FF0000) >>16; //green
	data[2] = (rgba & 0xFF000000) >>24; //red
	data[3] = 0x0;
	u32 val = *(u32*)&data[0];
	cout <<"bgr(" <<(u16)data[0] <<", " <<(u16)data[1] <<", " <<(u16)data[2] <<") = " <<val <<endl;
	return val;
}

void writeImage(ofstream& stream, SubImage& sub) {
	cout <<"before Image write, pos = " <<stream.tellp() <<endl;
	for(u32 y = 0; y < sub.height; ++y) {
		for(u32 x = 0; x < sub.width; ++x) {
			u32 bgr = rgba_to_bgr(sub.rgba[x+y*sub.width]);
			stream.write((const char*)&bgr, 4);
		}
	}
	cout <<"before Image write, pos = " <<stream.tellp() <<endl;
	stream.flush();
}

void writeFileSize(ofstream& stream, std::string path) {
	ifstream reader(path.c_str(), ios::ate);
	u32 size = reader.tellg();
	stream.seekp(2);
	stream.write((const char*)&size, 4);
	stream.flush();
}

void writeBitmap(std::string path, SubImage& sub) {
	cout <<"size fh: " << sizeof(FileHeader) <<endl;
	cout <<"size dh: " << sizeof(DIBHeader) <<endl;
	cout <<"size: +" << sizeof(FileHeader) + sizeof(DIBHeader) <<endl;
	ofstream writer(path.c_str(), ios::trunc | ios::binary);
	writeFileHeader(writer);
	writeDIBHeader(writer, sub);
	writeImage(writer, sub);
	writeFileSize(writer, path);
	writer.close();
}
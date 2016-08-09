#include <assert.h>
#include <fstream>

#include "imageloader.h"
#include "Orbitersdk.h"

using namespace std;

Image::Image(char* pxs, int w, int h) : pixels(pxs), width(w), height(h) {

}

Image::~Image() {
	delete[] pixels;
}

Image* loadBMP(const char* filename) {
	if (!filename)
		return 0;

	UINT8* datBuff[2] = { 0, 0 };
	UINT8* px = 0;
	BITMAPFILEHEADER* header = 0;
	BITMAPINFOHEADER* info = 0;

	std::ifstream file(filename, std::ios::binary);
	if (!file) { DIE("AttitudeIndicatorMFD::Could not find file") }

	datBuff[0] = new UINT8[sizeof(BITMAPFILEHEADER)];
	datBuff[1] = new UINT8[sizeof(BITMAPINFOHEADER)];
	file.read((char*)datBuff[0], sizeof(BITMAPFILEHEADER));
	file.read((char*)datBuff[1], sizeof(BITMAPINFOHEADER));
	header = (BITMAPFILEHEADER*)datBuff[0];
	info = (BITMAPINFOHEADER*)datBuff[1];

	if (header->bfType != 0x4D42) { DIE("AttitudeIndicatorMFD::Not a bitmap file") }

	px = new UINT8[info->biSizeImage];
	file.seekg(header->bfOffBits);
	file.read((char*)px, info->biSizeImage);

	UINT8 tmp = 0;
	for (unsigned long i = 0; i < info->biSizeImage - (info->biSizeImage % 3); i += 3) {
		tmp = px[i];
		px[i] = px[i + 2];
		px[i + 2] = tmp;
	}

	return new Image((char*)px, info->biWidth, info->biHeight);
}
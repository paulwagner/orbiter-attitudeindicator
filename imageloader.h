#ifndef IMAGELOADER_H
#define IMAGELOADER_H

#define DIE(s) oapiWriteLog(s); return 0;

class Image {
	public:
		Image(char* pxs, int w, int h);
		~Image();
		char* pixels;
		int width;
		int height;
};

Image* loadBMP(const char* filename);

#endif

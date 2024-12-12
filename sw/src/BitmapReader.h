#ifndef H_BITMAP_READER
#define H_BITMAP_READER

#include "inc/ff.h"
#include <stdint.h>

typedef struct {
	uint32_t dataSize;
	uint32_t dataStartOffset;
} bitmapHeader;

#define BMP_OK 0
#define BMP_FAILED 1

// interrupts must be enabled, TFT_CS high, and SD_CS low
int bitmapReaderInit(void);
int readBitmapHeader(FIL* fp, bitmapHeader* bh);
int readBitmap(const TCHAR* name, uint16_t* buf);
int findFile(const TCHAR* name);
void listFiles(void);


// gets metadata from the provided bitmap file.
// returns the data in bitmapHeader.


#endif
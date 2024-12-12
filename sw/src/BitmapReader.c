
#include "BitmapReader.h"
#include <stdio.h>
#include "inc/diskio.h"

/*

unsigned char readBuf[255]; 

FIL br_file;
bitmapHeader br_bh;
UINT numBytesRead;
static FATFS g_sFatFs;
FILINFO finfo;
DIR root;
FRESULT Fresult;

#define BMP_HEADER_NUM_BYTES 14

int bitmapReaderInit(void) {
	Timer5_Init();
	DSTATUS ds = disk_initialize(0);
	if(ds) return BMP_FAILED;
	
	Fresult = f_mount(&g_sFatFs, "", 0);
  if(Fresult) return BMP_FAILED;
	
	return BMP_OK;
}

int readBitmapHeader(FIL* fp, bitmapHeader* bh) {
	f_read(fp, readBuf, BMP_HEADER_NUM_BYTES, &numBytesRead);
	if(numBytesRead != BMP_HEADER_NUM_BYTES) return BMP_FAILED;
	if(readBuf[0] != 'B' || readBuf[1] != 'M') return BMP_FAILED;
	
	bh->dataSize = *((uint32_t*)(readBuf + 2));
	bh->dataStartOffset = *((uint32_t*)(readBuf + 10));
	bh->dataSize -= bh->dataStartOffset;
	
	return BMP_OK;
}

int readBitmap(const TCHAR* name, uint16_t* buf) {
	if(f_open(&br_file, name, FA_READ) != FR_OK) return BMP_FAILED;
	if(readBitmapHeader(&br_file, &br_bh) != BMP_OK) return BMP_FAILED;
	
	if(f_lseek(&br_file, br_bh.dataStartOffset) != FR_OK) return BMP_FAILED;
	if(f_read(&br_file, buf, br_bh.dataSize, &numBytesRead) != FR_OK) return BMP_FAILED;
	if(numBytesRead != br_bh.dataSize) return BMP_FAILED;
	
	return BMP_OK;
}

int findFile(const TCHAR* name) {
	if(f_open(&br_file, name, FA_READ) != FR_OK) return BMP_FAILED;
	
	return BMP_OK;
}



void listFiles(void) {
	Fresult = f_opendir(&root, "");
	if(Fresult) printf("failed to open dir: %d\n", Fresult);
	else printf("opened dir: %ld\n", root.sect);
	
	Fresult = f_readdir(&root, &finfo);
	while(root.sect != 0) {
			if(Fresult) printf("failed to read dir: %d\n", Fresult);
			else {
				printf("in dir: %s\n", finfo.fname);
			}
			Fresult = f_readdir(&root, &finfo);
	}
}
*/

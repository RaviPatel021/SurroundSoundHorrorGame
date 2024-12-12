
#include "AudioReader.h"
#include <stdio.h>
#include "inc/diskio.h"
#include "inc/UART.h"
#include "scarygame.h"


#define SDC_CS           (*((volatile uint32_t *)0x40007200))
#define SDC_CS_LOW       0           // CS controlled by software
#define SDC_CS_HIGH      0x80
#define TFT_CS                  (*((volatile uint32_t *)0x40004020))
#define TFT_CS_LOW              0           // CS controlled by software
#define TFT_CS_HIGH             0x08

uint8_t arFifoLock;

#define AR_READBUF_SIZE 2048
unsigned char arReadBuf[AR_READBUF_SIZE];
uint32_t arReadBufIndex;
UINT arReadBufNumBytes;


#define AR_FIFO_SIZE 2048
struct audioUnit arFifo[AR_FIFO_SIZE];
uint32_t arFifoRemoveIndex;
uint32_t arFifoAddIndex;
uint32_t arFifoNumEntries;

FIL ar_file;

static FATFS g_sFatFs;
FILINFO finfo;
DIR root;
FRESULT Fresult;


const uint32_t f44100 = 1814;
const uint32_t f22050 = f44100 * 2;
const uint32_t f8000 = 10417;

#define DOLOOP 1
#define DONTLOOP 0

uint32_t numLoops = 0;


struct directionalSound {
	const TCHAR* filename;
	enum soundDirection direction;
	uint32_t samplingReloadVal; // reload val associated with the wav sampling rate
	uint8_t shouldLoop;
};

struct directionalSound gunshotSound = {"gsf.csv", SDIR_FRONT, f22050, DONTLOOP};

struct directionalSound monsterSound = {"goose.csv", SDIR_FRONT, f22050, DOLOOP};


struct directionalSound loseSound = {"lose.csv", SDIR_FRONT, f8000, DONTLOOP};
struct directionalSound winSound = {"win.csv", SDIR_FRONT, f8000, DONTLOOP};
struct directionalSound ateYouSound = {"dead.csv", SDIR_FRONT, f8000, DONTLOOP};


struct directionalSound* soundPlayingNow = &monsterSound;

// defined in startup.s
uint32_t StartCritical(void); 
void EndCritical(uint32_t ibit);

int refillReadBuf(void);
int getNextChar(char* c);
int isNum(char c);
int readNum(uint32_t* num);
int arFifoGetLock(void);
void arFifoReleaseLock(void);

uint8_t hitEof; // bool

void enableSDCard(void) {
	TFT_CS = TFT_CS_HIGH; // disable screen
	SDC_CS = SDC_CS_LOW; // enable sd card
}

void arFifoClear(void) {
	arFifoGetLock();
	arFifoAddIndex = 0;
	arFifoRemoveIndex = 0;
	arFifoNumEntries = 0;
	arFifoReleaseLock();
}

enum soundDirection getCurrentSoundDirection(void) {
	return soundPlayingNow->direction;
}

void clearReadBuf(void) {
	for(int i=0; i < AR_READBUF_SIZE; i++) {
		arReadBuf[i] = 0;
	}
	arReadBufIndex = 0;
	arReadBufNumBytes = 0;
}

// initialize circular array fifo
void arFifoInit(void) {
	arFifoLock = 0;
	arFifoClear();
}

uint8_t playingMonsterSound(void) {
	return (soundPlayingNow == &monsterSound);
}

uint32_t getFileSamplingReloadVal(void) {
	// may change this to pull this info from the file,
	// for now just hardcode it
	//return 3628; // timer units
	//const uint32_t f44100 = 1814;
	//const uint32_t f22050 = f44100 * 2;
	
	uint32_t reload = soundPlayingNow->samplingReloadVal;
//	uint32_t score = getScore();
//	
//	// monster gets faster for each level
//	if(playingMonsterSound()) {
//		if(score <= 7) {
//			reload = reload - score * reload / 8;
//		}
//		else reload = reload - 7 * reload / 8;
//	}
	
	
	return reload;
}

uint32_t getNumLoops(void) {
	return numLoops;
}

int readFrom(struct directionalSound* ds) {
	soundPlayingNow = ds;
	clearReadBuf();
	arFifoClear();
	
	enableSDCard();
	
	int status = f_open(&ar_file, soundPlayingNow->filename, FA_READ);
	
	if(status != FR_OK) {
		UART_OutString("FR error code"); UART_OutUDec(status); UART_OutString("\r\n");
		return ARD_FAILED;
	}
	hitEof = 0;
	numLoops = 0;
	
	status = refillReadBuf();
	if(status == ARD_FAILED || status == ARD_EOF) return ARD_FAILED;
	
	return ARD_OK;
}

void closeCurrentFile(void) {
	enableSDCard();
	f_close(&ar_file);
}

int audioReaderInit(void) {
	Timer5_Init();
	arFifoInit();
	
	enableSDCard();
	
	DSTATUS ds = disk_initialize(0);
	if(ds) return ARD_FAILED;
	
	Fresult = f_mount(&g_sFatFs, "", 0);
  if(Fresult) return ARD_FAILED;
	
	if(readFrom(&monsterSound) != ARD_OK) return ARD_FAILED;
	
	return ARD_OK;
}

int arFifoGetLock(void) {
	uint8_t status;
	
	uint32_t ibit = StartCritical();
	if(arFifoLock) status = ARD_FAILED; // lock already taken
	else {
		arFifoLock = 1; // get lock
		status = ARD_OK;
	}
	EndCritical(ibit);
	
	return status;
}

void arFifoReleaseLock(void) {
	uint32_t ibit = StartCritical();
	arFifoLock = 0;
	EndCritical(ibit);
}

int arFifoFull(void) {
	return arFifoNumEntries >= AR_FIFO_SIZE;
}

int arFifoEmpty(void) {
	return arFifoNumEntries <= 0;
}


int arFifoAdd(struct audioUnit unit) {
	if(arFifoFull()) return ARD_FAILED;
	
	if(arFifoGetLock() != ARD_OK) return ARD_FAILED;
	
	arFifo[arFifoAddIndex] = unit;
	arFifoAddIndex = (arFifoAddIndex + 1) % AR_FIFO_SIZE;
	arFifoNumEntries++;
	
	arFifoReleaseLock();
	
	return ARD_OK;
}


int arFifoRemove(struct audioUnit* unit) {
	if(arFifoEmpty()) return 0;
	
	if(arFifoGetLock() != ARD_OK) return ARD_FAILED;
	
	*unit = arFifo[arFifoRemoveIndex];
	arFifoRemoveIndex = (arFifoRemoveIndex + 1) % AR_FIFO_SIZE;
	arFifoNumEntries--;
	
	arFifoReleaseLock();
	
	return ARD_OK;
}

void onNoLoopEof(void) { 
	if(soundPlayingNow == &gunshotSound) {
		onGunshotSoundEnd();
	}
	else if(soundPlayingNow == &winSound || soundPlayingNow == &loseSound) {
		onWinLoseSoundEnd();
	}
	else if(soundPlayingNow == &ateYouSound) onDeathSoundEnd();
}

// fills readBuf to max size and resets index pointer
int refillReadBuf(void) {
	if(hitEof) return ARD_EOF;
	
	enableSDCard();
	
	if(f_read(&ar_file, &arReadBuf, AR_READBUF_SIZE, &arReadBufNumBytes) != FR_OK) return ARD_FAILED;
	if(arReadBufNumBytes == 0) {
		if(soundPlayingNow->shouldLoop) {
			f_lseek(&ar_file, 0); // reset to start of file
			if(f_read(&ar_file, &arReadBuf, AR_READBUF_SIZE, &arReadBufNumBytes) != FR_OK) return ARD_FAILED;
			
			numLoops++;
			onAudioLoop(); // function from audio.c
			onMonsterLoop(numLoops);
			
		}
		else {
			hitEof = 1;
			onNoLoopEof();
			return ARD_EOF;
		}
		
	}
	arReadBufIndex = 0;
	return ARD_OK;
}

// reads next char from file (actually from the read buffer though)
int getNextChar(char* c) {
	// refill buffer if there are no characters left to read from it
	if(arReadBufIndex >= arReadBufNumBytes) {
		int status = refillReadBuf();
		if(status != ARD_OK) return status;
	}
	
	*c = arReadBuf[arReadBufIndex];
	arReadBufIndex++;
	
	return ARD_OK;
}

int isNum(char c) {
	return '0' <= c && c <= '9';
}


// reads a number until it reaches a comma
int readNum(uint32_t* num) {
	char c;
	int sum = 0;
	int status;
	uint8_t started = 0; // bool to see if we've started building a number
	do {
		status = getNextChar(&c);
		if(status == ARD_FAILED) return ARD_FAILED;
		else if(status == ARD_EOF) {
			if(started) {
				*num = sum; // found EOF while building number, return that number
				return ARD_OK;
			}
			else return ARD_EOF;
		}
		
		if(isNum(c)) {
			sum = sum * 10 + (c - '0');
		}
		else {
			// end of number
			*num = sum;
			return ARD_OK;
		}
		
		started = 1;
	} while(1);
	
}



int arAtEof(void) {
	return hitEof;
}

int readPair(struct audioUnit* unit) {
	int status;
	status = readNum(&unit->left);
	if(status != ARD_OK) return status;
	status = readNum(&unit->right);
	if(status != ARD_OK) return status;
	return ARD_OK;
}

int readIntoAudioFifo(void) {
	struct audioUnit unit;
	if(readPair(&unit) != ARD_OK) return ARD_FAILED;
	if(arFifoAdd(unit) != ARD_OK) return ARD_FAILED;

	return ARD_OK;
}



int getFromAudioFifo(struct audioUnit* unit) {
	if(arFifoRemove(unit) != ARD_OK) return ARD_FAILED;
	return ARD_OK;
}

int readFromGunshot(enum soundDirection dir) {
	closeCurrentFile();
	struct directionalSound* ds;
	ds = &gunshotSound;
	ds->direction = dir;
	if(readFrom(ds) != ARD_OK) return ARD_FAILED;
	return ARD_OK;
}

int readFromMonster(enum soundDirection dir) {
	closeCurrentFile();
	struct directionalSound* ds;
	ds = &monsterSound;
	ds->direction = dir;
	
	const uint32_t score = getScore();
	if(getScore() == 0) ds->filename = "goose.csv";
	else if(getScore() <= 2) ds->filename = "goose2.csv";
	else if(getScore() % 3 == 0) ds->filename = "cat.csv";
	else if(getScore() % 3 == 1) ds->filename = "sgrowl.csv";
	else if(getScore() % 3 == 2) ds->filename = "grunt1.csv";
	
	if(readFrom(ds) != ARD_OK) return ARD_FAILED;
	return ARD_OK;
}

int readFromDeathSound(void) {
	closeCurrentFile();
	struct directionalSound* ds = &ateYouSound;
	
	if(readFrom(ds) != ARD_OK) return ARD_FAILED;
	return ARD_OK;
}

int readFromWinLoseSounds(uint8_t shotInCorrectDirection) {
	closeCurrentFile();
	struct directionalSound* ds;
	
	if(shotInCorrectDirection) ds = &winSound;
	else ds = &loseSound;
	
	if(readFrom(ds) != ARD_OK) return ARD_FAILED;
	return ARD_OK;
}
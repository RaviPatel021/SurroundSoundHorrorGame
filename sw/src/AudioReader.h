#ifndef H_AUDIO_READER
#define H_AUDIO_READER

#include "inc/ff.h"
#include "audio.h"
#include <stdint.h>

#define ARD_EOF 2
#define ARD_OK 1
#define ARD_FAILED 0

struct audioUnit {
	uint32_t left;
	uint32_t right;
};

// interrupts must be enabled, TFT_CS high, and SD_CS low
int audioReaderInit(void);

int readIntoAudioFifo(void);
int getFromAudioFifo(struct audioUnit* unit);
int arFifoFull(void);
int arFifoEmpty(void);
int arAtEof(void);

uint32_t getFileSamplingReloadVal(void); // timer units
enum soundDirection getCurrentSoundDirection(void);

int readFromMonster(enum soundDirection dir);
int readFromGunshot(enum soundDirection dir);
int readFromDeathSound(void);
int readFromWinLoseSounds(uint8_t shotInCorrectDirection);

uint32_t getNumLoops(void);

#endif
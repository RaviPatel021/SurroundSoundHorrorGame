// File **********Transmitter.h***********
// Solution to Lab9 (not to be shown to students)
// Programs to implement transmitter functionality   
// EE445L Spring 2021
//    Jonathan W. Valvano 4/4/21
// 2-bit input, positive logic switches, positive logic software
#ifndef __AUDIO_H__
#define __AUDIO_H__

#include <stdint.h>

enum soundDirection {
	SDIR_FRONT,
	SDIR_DIAGLEFT,
	SDIR_LEFT,
	SDIR_DIAGRIGHT,
	SDIR_RIGHT,
	SDIR_OTHER,
};

extern uint32_t soundPlaying;

void audioInit(void);
void stopPlaying(void);
void startPlaying();
void raiseVolume(void);
void lowerVolume(void);
void setVolume(uint32_t num);
void onAudioLoop(void); // should be called by audioreader.c

#endif

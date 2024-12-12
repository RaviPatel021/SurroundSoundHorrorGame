// File **********Transmitter.c***********
// Lab 5
// Programs to implement transmitter functionality   
// EE445L Spring 2021
//    Jonathan W. Valvano 4/4/21
// Hardware/software assigned to transmitter
//   UART0, possible source of input data (conflicts with TExaSdisplay)
//   PC7-PC4 Port_C_Init, possible source of input data
//   Timer0A periodic interrupt to generate input
//   FreqFifo linkage from Encoder to SendData
//   Timer1A periodic interrupt create a sequence of frequencies
//   SSI1 PD3 PD1 PD0 TLV5616 DAC output
//   SysTick ISR output to DAC


#include "../inc/tm4c123gh6pm.h"
#include "../inc/TLV5616.h"
#include "inc/Timer1A.h"
#include "audio.h"
#include "led.h"
#include "inc/Timer2A.h"
#include "inc/Timer4A.h"
#include "AudioReader.h"
#include "inc/UART.h"


#define NOTE_NULL 0 
#define NOTE_C4 1
#define NOTE_D4b 2
#define NOTE_D4 3
#define NOTE_E4b 4
#define NOTE_E4 5
#define NOTE_F4 6
#define NOTE_G4b 7
#define NOTE_G4 8
#define NOTE_A4b 9
#define NOTE_A4 10
#define NOTE_B4b 11
#define NOTE_B4 12

#define NOTE_C5 13
#define NOTE_D5b 14
#define NOTE_D5 15
#define NOTE_E5b 16
#define NOTE_E5 17
#define NOTE_F5 18
#define NOTE_G5b 19
#define NOTE_G5 20
#define NOTE_A5b 21
#define NOTE_A5 22
#define NOTE_B5b 23
#define NOTE_B5 24

#define NOTE_B3 25

#define NUM_NOTES 208

#define WAVE_SIZE 64

uint32_t soundPlaying;

#define MAX_VOLUME 1000
#define MIN_VOLUME 1
#define DEFAULT_VOLUME 10	
#define VOLUME_INC_FACTOR 2
uint32_t volume;

				
const uint32_t volControlReload = 4000000;

void raiseVolumeLinear(uint32_t amt);
void lowerVolumeLinear(uint32_t amt);
				
struct audioUnit rebalanceAudio(struct audioUnit unit) {
	struct audioUnit out;
	
	uint32_t soundRatio = 100;
	
	enum soundDirection dir = getCurrentSoundDirection();
	switch(dir) {
		case SDIR_RIGHT:
			out.right = unit.right;
			out.left = unit.left / soundRatio;
			break;
		case SDIR_LEFT:
			out.right = unit.right / soundRatio;
			out.left = unit.left;
			break;
		default: 
			out.right = unit.right;
			out.left = unit.left;
			break;
	}
	
	return out;
}

struct audioUnit audioOutputUnit;
struct audioUnit prevUnit;
void audioFileOutputHandler(void) {
	uint16_t output;
	
	if(arFifoEmpty() || getFromAudioFifo(&audioOutputUnit) != ARD_OK) {
		audioOutputUnit = prevUnit;
	}

	audioOutputUnit = rebalanceAudio(audioOutputUnit);
	
	output = (uint16_t)(audioOutputUnit.left & 0x0FFF);
	output = (output * volume) / MAX_VOLUME; // change amplitude depending on volume
	DAC_OutLeft(output);
	
	output = (uint16_t)(audioOutputUnit.right & 0x0FFF);
	
	output = (output * volume) / MAX_VOLUME; // change amplitude depending on volume
	DAC_OutRight(output);

	
	
	prevUnit = audioOutputUnit;


}

void onAudioLoop(void) {
	raiseVolume();
}

void volumeControlHandler(void) {
	raiseVolumeLinear(1);
}

void audioInit(void) {
	DAC_Init(0);
	Timer1A_Init(&audioFileOutputHandler, getFileSamplingReloadVal(), 2);
	//Timer2A_Init(&rightNoteWaveHandler, 0, 2);
	//Timer4A_Init(&volumeControlHandler, volControlReload, 2);
	
	volume = DEFAULT_VOLUME;
	stopPlaying();
}

void startPlaying(void) {
	Timer1A_Change_Period(getFileSamplingReloadVal());
	Timer1A_Start();
	
	//Timer2A_Change_Period(otherNoteReload());
	//Timer2A_Start();
	
	//Timer4A_Start();

	soundPlaying = 1;
}

void stopPlaying(void) {
	Timer1A_Stop();
	//Timer2A_Stop();
	//Timer4A_Stop();
	soundPlaying = 0;
}

void setVolume(uint32_t num) {
	volume = num;
	if(volume > MAX_VOLUME) volume = MAX_VOLUME;
	if(volume < MIN_VOLUME) volume = MIN_VOLUME;
}

void raiseVolumeLinear(uint32_t amt) {
	volume += amt;
	if(volume > MAX_VOLUME) volume = MAX_VOLUME;
}

void lowerVolumeLinear(uint32_t amt) {
	volume -= amt;
	if(volume < MIN_VOLUME) volume = MIN_VOLUME;
}

// uses log scale
void raiseVolume(void) {
	volume *= VOLUME_INC_FACTOR;
	if(volume > MAX_VOLUME) volume = MAX_VOLUME;
}

void lowerVolume(void) {
	volume /= VOLUME_INC_FACTOR;
	if(volume < MIN_VOLUME) volume = MIN_VOLUME;
}


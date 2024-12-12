
#include <stdint.h>
#include "../inc/tm4c123gh6pm.h"
#include "../inc/Texas.h"
#include "../inc/PLL.h"
#include "../inc/ST7735.h"
#include "audio.h"
#include "switch.h"
#include "inc/Timer0A.h"
#include "inc/LaunchPad.h"
#include "inc/UART.h"
#include "AudioReader.h"
#include "inc/ST7735withSD.h"
#include "digitalCompass.h"
#include "inc/I2C0.h"
#include "random.h"
#include "inc/Timer2A.h"


void DisableInterrupts(void);           // Disable interrupts
void EnableInterrupts(void);            // Enable interrupts

uint8_t shotInCorrectDirection = 0;
enum soundDirection currentMonsterDir;
uint8_t gameStarted = 0;
uint32_t score = 0;

uint32_t highScore = 0;

uint8_t enableTriggerButton = 0;

uint32_t startingMonsterVolume = 20;

uint8_t drawGunAngleFlag = 0;

void drawGunAngle(void) {
	ST7735_DrawString(2, 14, "Angle:      ", ST7735_BLUE);
	ST7735_SetCursor(10, 14);
	ST7735_SetTextColor(ST7735_BLUE);
	ST7735_OutUDec(checkAndTrigger());
}

void debugHandler(void) {
	PF1 ^= 0x2;
	drawGunAngleFlag = 1;
}



void drawUpdatedScore(void) {
	ST7735_DrawString(2, 6, "Score: ", ST7735_BLUE);
	ST7735_SetCursor(10, 6);
	ST7735_SetTextColor(ST7735_BLUE);
	ST7735_OutUDec(score);
}

void drawHighScore(void) {
	ST7735_DrawString(2, 12, "High Score: ", ST7735_BLUE);
	ST7735_SetCursor(14, 12);
	ST7735_SetTextColor(ST7735_BLUE);
	ST7735_OutUDec(highScore);
}



void startGame(void) {
	gameStarted = 1;
	enableTriggerButton = 1;
	setVolume(startingMonsterVolume);
	startPlaying();
	ST7735_FillScreen(ST7735_BLACK);
	ST7735_DrawString(2, 2, "Shoot where you", ST7735_BLUE);
	ST7735_DrawString(2, 3, "hear the monster!", ST7735_BLUE);
	drawHighScore();
}

void onMonsterLoop(uint32_t numLoops) {
//	if((score <= 2 && numLoops >= 3) || 
//		 (3 <= score && score < 5 && numLoops >= 2) ||
//		 (5 <= score && numLoops >= 1)) {
	
	if(numLoops == 3) {
		stopPlaying();
		shotInCorrectDirection = 0;
		enableTriggerButton = 0;
		setVolume(100);
		readFromDeathSound();
		startPlaying();
	}
	
}

void shootInDirection(enum soundDirection dir) {
	shotInCorrectDirection = dir == getCurrentSoundDirection();
	enableTriggerButton = 0;
	stopPlaying();
	setVolume(200);
	readFromGunshot(dir);
	startPlaying(); // play gun noise
}

void resetGame(void) {
	currentMonsterDir = getRandomDirection();
	readFromMonster(currentMonsterDir);
	gameStarted = 0;
	score = 0;
	enableTriggerButton = 0;
	shotInCorrectDirection = 0;
	ST7735_FillScreen(ST7735_BLACK);
	ST7735_DrawString(1, 2, "point gun forward", ST7735_BLUE);
	ST7735_DrawString(1, 3, "and press calibrate", ST7735_BLUE);
	ST7735_DrawString(1, 4, "to begin", ST7735_BLUE);
	drawHighScore();
}

uint32_t getScore(void) {
	return score;
}

void nextStage(void) {
	stopPlaying();
	
	if(shotInCorrectDirection) {
		score++;
		drawUpdatedScore();
		setVolume(startingMonsterVolume * score);
		currentMonsterDir = getRandomDirection();
		readFromMonster(currentMonsterDir);
		shotInCorrectDirection = 0;
		enableTriggerButton = 1;
		startPlaying();
		
	}
	else { // lost game
		if(score > highScore) highScore = score;
		resetGame();
	}
	
	
}

void onGunshotSoundEnd(void) {
	stopPlaying();
	
	if(!shotInCorrectDirection) {
		setVolume(100);
		readFromDeathSound();
	}
	else {
		setVolume(50);
		readFromWinLoseSounds(shotInCorrectDirection);
	}
	startPlaying();
}

void onWinLoseSoundEnd(void) {
	nextStage();
}

void onDeathSoundEnd(void) {
	stopPlaying();
	setVolume(100);
	readFromWinLoseSounds(shotInCorrectDirection);
	startPlaying();
}

int main(void){
  DisableInterrupts();
  PLL_Init(Bus80MHz);
	LaunchPad_Init();
	Timer0A_Init(&debugHandler, 80000000 / 5, 1);	
	switchInit();
  audioInit();
	UART_Init();
	I2C_Init();
	compassInit();
	ST7735_InitR(INITR_GREENTAB);
  EnableInterrupts();
	
	
	
	
	struct audioUnit unit;
	if(audioReaderInit() == ARD_OK) {
		UART_OutString("SDCard init worked\r\n");
		while(readIntoAudioFifo() == ARD_OK) {}
		UART_OutString("Filled up fifo\r\n");
	}
	else UART_OutString("SDCard init failed\r\n");
	
	if(readFromMonster(currentMonsterDir) != ARD_OK) {
		UART_OutString("Failed to read monster sound file\r\n");
	}		
	
	setSeed(1212);
	resetGame();
	
	
	while(1){
//		if(drawGunAngleFlag) {
//			drawGunAngleFlag = 0;
//			drawGunAngle();
//		}
//		
		if(!arFifoFull() && !arAtEof()) {
			readIntoAudioFifo();
		}
		
		// shoot
		if(gameStarted && enableTriggerButton && switchPressed(pin0)) {
			enum soundDirection dir = getDirection();
			/*
			switch(dir) {
				case SDIR_FRONT:
					UART_OutString("FRONT");
					break;
				case SDIR_RIGHT:
					UART_OutString("RIGHT");
					break;
				case SDIR_LEFT:
					UART_OutString("LEFT");
					break;
				default:
					UART_OutString("BACK");
					break;
			}
			UART_OutString("\r\n");
			*/
			
			shootInDirection(dir);
		}
		
		// calibrate
		if(switchPressed(pin1)) {
			checkAndCalibrateCompass();
			UART_OutString("calibrated\r\n");
			if(!gameStarted) startGame();
			
		}
  }
	
	
}



int main2(void) {
	DisableInterrupts();
  PLL_Init(Bus80MHz);
	LaunchPad_Init();
	Timer0A_Init(&debugHandler, 80000000, 1);	
	switchInit();
  audioInit();
	UART_Init();
  EnableInterrupts();

	struct audioUnit unit;
	
	if(audioReaderInit() == ARD_OK) {
		UART_OutString("SDCard init worked\r\n");
		while(readIntoAudioFifo() == ARD_OK) {
			UART_OutString("Reading into fifo\r\n");
		}
		while(getFromAudioFifo(&unit) == ARD_OK) {
			UART_OutString("Getting from fifo\r\n");
			UART_OutUDec(unit.left);
			UART_OutChar(',');
			UART_OutUDec(unit.right);
			UART_OutString("\r\n");
		}
		
	}
	else UART_OutString("SDCard init failed\r\n");
	while(1) {
		
	}

}

int16_t compassVal = 0;

// for compass hard iron calibration
int main456(void) {
	DisableInterrupts();
  PLL_Init(Bus80MHz);
	LaunchPad_Init();
	Timer0A_Init(&debugHandler, 80000000 / 5, 1);	
	switchInit();
  audioInit();
	UART_Init();
	I2C_Init();
	compassInit();
	ST7735_InitR(INITR_GREENTAB);
  EnableInterrupts();
	
	Clock_Delay1ms(20);
	
	while(1){

		/*
		if(switchPressed(pin0)) {
			enum soundDirection dir = getDirection();
			switch(dir) {
				case SDIR_FRONT:
					UART_OutString("FRONT");
					break;
				case SDIR_RIGHT:
					UART_OutString("RIGHT");
					break;
				case SDIR_LEFT:
					UART_OutString("LEFT");
					break;
				default:
					UART_OutString("BACK");
					break;
			}
			UART_OutString("\r\n");
		}
		if(switchPressed(pin1)) {
			checkAndCalibrateCompass();
			UART_OutString("calibrated\r\n");
		}
		
		
		compassVal = compassRead();
		if(compassVal != -1) {
			UART_OutUDec(compassVal); UART_OutString("\r\n");
		}
		*/
		
		hardIronCalDebug();
		Clock_Delay1ms(100);
	}
	
}



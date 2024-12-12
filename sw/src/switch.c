// File **********Switch.c***********
// Lab4 )
// Programs to interface with Switch buttons   
// EE445L Fall 2015
//    Jonathan W. Valvano 9/22/15
// 2-bit input, positive logic switches, positive logic software
// define your hardware interface
// bit1 PE1 Voice switch 
// bit0 PE0 Play switch 


#include <stdint.h>
#include "../inc/tm4c123gh6pm.h"
#include "inc/Timer3A.h"
#include "switch.h"

uint8_t swPressed = 0;
volatile uint8_t ignorePresses = 0;

void ignorePressesHandler(void) {
	ignorePresses = 0;
}
    
void switchInit(void){
	const uint32_t porte = 0x10;
	const uint32_t pins = pin3 | pin2 | pin1 | pin0;
	
	SYSCTL_RCGCGPIO_R |= porte; //initialize porte
	while((SYSCTL_PRGPIO_R&porte) == 0){}; //wait for porte to be ready
	GPIO_PORTE_DIR_R &= ~pins; // PE3-0 set as input
  GPIO_PORTE_PUR_R |= pins; // enable pull-up resistors on PE3-0, we use negative logic
  GPIO_PORTE_DEN_R |= pins; //digital enable for PE3-0 
		
	Timer3A_Init(&ignorePressesHandler, 80000, 3);
	ignorePresses = 0;
}	


void ignorePressesFor1ms(void) {
	// start one shot timer that lasts 1ms
	ignorePresses = 1;
	Timer3A_Start();
}

uint32_t switchPressed(uint32_t pin) {
	uint32_t positive = GPIO_PORTE_DATA_R & pin;
	
	if (swPressed & pin) {
		if (positive){ 
			swPressed &= ~pin;
			
			if (!ignorePresses) ignorePressesFor1ms();
		}
	}
	else {
		if (!positive) {
			swPressed |= pin;
			
			if (!ignorePresses) {
				ignorePressesFor1ms();
				return 1;
			}
		}
	}
	
	return 0;
}

uint32_t switchReleased(uint32_t pin) {
	uint32_t positive = GPIO_PORTE_DATA_R & pin;
	
	if (swPressed & pin) {
		if (positive){ 
			swPressed &= ~pin;
			
			if (!ignorePresses) ignorePressesFor1ms();
		}
	}
	else {
		if (!positive) {
			swPressed |= pin;
			
			if (!ignorePresses) {
				ignorePressesFor1ms();
				return 1;
			}
		}
	}
	
	return 0;
}

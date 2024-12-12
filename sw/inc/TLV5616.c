// TLV5616.c
// Runs on TM4C123
// Use SSI1 to send a 16-bit code to the TLV5616 and return the reply.
// Daniel Valvano
// EE445L Fall 2015
//    Jonathan W. Valvano 9/22/15

/* This example accompanies the book
   "Embedded Systems: Real Time Interfacing to ARM Cortex M Microcontrollers",
   ISBN: 978-1463590154, Jonathan Valvano, copyright (c) 2014

 Copyright 2014 by Jonathan W. Valvano, valvano@mail.utexas.edu
    You may use, edit, run or distribute this file
    as long as the above copyright notice remains
 THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
 OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
 MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
 VALVANO SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL,
 OR CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
 For more information about my classes, my research, and my books, see
 http://users.ece.utexas.edu/~valvano/
 */

// SSIClk (SCLK) connected to PD0
// SSIFss (FS)   connected to PD1
// SSITx (DIN)   connected to PD3

#include <stdint.h>
#include "../inc/tm4c123gh6pm.h"


//----------------   DAC_Init     -------------------------------------------
// Initialize TLV5616 12-bit DAC
// assumes bus clock is 80 MHz
// inputs: initial voltage output (0 to 4095)
// outputs:none

void DACInitLeft(uint16_t data);
void DACInitRight(uint16_t data);

void DAC_Init(uint16_t data){
	DACInitLeft(data);
	DACInitRight(data);
}

void DACInitLeft(uint16_t data) {
const uint32_t portd = 0x8;
	const uint32_t ssi3 = 0x8;
	const uint32_t pin0 = 0x1;
	const uint32_t pin1 = 0x2;
	const uint32_t pin2 = 0x4;
	const uint32_t pin3 = 0x8;
	const uint32_t pins = pin0 | pin1 | pin2 | pin3;
	// Consider the following registers:
	// SYSCTL_RCGCSSI_R, SSI3_CR1_R, SSI3_CPSR_R, SSI3_CR0_R, SSI3_DR_R, SSI3_CR1_R
	// PD0 SSI0Clk, PD1 SSI0Fss, PD2 SSI0Rx, PD3 SSI0Tx
	
	SYSCTL_RCGCSSI_R |= ssi3;
	
	SYSCTL_RCGCGPIO_R |= portd;
	while((SYSCTL_PRGPIO_R&portd) == 0){};
		
		GPIO_PORTD_AFSEL_R |= pins;
	GPIO_PORTD_PCTL_R = (GPIO_PORTD_PCTL_R&0xFFFF0000)+0x00001111; //assigns the SSI signals to the approporaite pins
	GPIO_PORTD_DIR_R |= (pin0|pin1|pin3);
	GPIO_PORTD_DEN_R |= pins;
	GPIO_PORTD_AMSEL_R = 0; // disable analog functionality on PA ... idt thsi is needed
		
	//now SSI3 initializations yippee
	SSI3_CR1_R = 0x00000000; //sets master mode (0), disables SSI
	SSI3_CR0_R &= ~(0x0000FFF0); // SCR = 0 SPO = 0 Freescale
	SSI3_CR0_R |= 0x80; // SPH = 1;
	SSI3_CR0_R |= 0x0F; // DSS = 16-bit data bc of how it works read the datasheet fuvckasses
	SSI3_CPSR_R = 0x04; // clock prescale divider = 4
	SSI3_DR_R = data; // load �data� into transmit FIFO
	SSI3_CR1_R |= 0x00000002; // enable SSI
}

void DACInitRight(uint16_t data) {
		const uint32_t portb = 0x2;
		const uint32_t pin4 = 0x10;
		const uint32_t pin5 = 0x20;
		const uint32_t pin6= 0x40;
		const uint32_t pin7 = 0x80;
		const uint32_t pins = pin4 | pin5 | pin6 | pin7;
		
	  // Consider the following registers:
	  // SYSCTL_RCGCSSI_R, SSI1_CR1_R, SSI1_CPSR_R, SSI1_CR0_R, SSI1_DR_R, SSI1_CR1_R
		
		SYSCTL_RCGCSSI_R |= 0x04; //setting up ssi module 2 yippee
		// PB4 SSI0Clk, PB5 SSI0Fss, PB6 SSI0Rx, PB7 SSI0Tx
		SYSCTL_RCGCGPIO_R |= portb;
		while((SYSCTL_PRGPIO_R&portb) == 0){}; //wait till porta is ready
		GPIO_PORTB_AFSEL_R |= pins; //pb4-7
		GPIO_PORTB_PCTL_R = (GPIO_PORTB_PCTL_R&0x000FFFF)+0x22220000; //assigns the SSI signals to the approporaite pins
		GPIO_PORTB_DIR_R |= (pin4 | pin5 | pin7);
		GPIO_PORTB_DEN_R |= pins; // digital enable pa2-5
		GPIO_PORTB_AMSEL_R = 0; // disable analog functionality on PA ... idt thsi is needed
		
		//now SSI0 initializations yippee
		SSI2_CR1_R = 0x00000000; //sets master mode (0), disables SSI
		SSI2_CR0_R &= ~(0x0000FFF0); // SCR = 0 SPO = 0 Freescale
		SSI2_CR0_R |= 0x80; // SPH = 1;
		SSI2_CR0_R |= 0x0F; // DSS = 16-bit data bc of how it works read the datasheet fuvckasses
		SSI2_CPSR_R = 0x04; // clock prescale divider = 4
		SSI2_DR_R = data; // load �data� into transmit FIFO
		SSI2_CR1_R |= 0x00000002; // enable SSI
}

// --------------     DAC_Out   --------------------------------------------
// Send data to TLV5616 12-bit DAC
// inputs:  voltage output (0 to 4095)
// 
void DAC_OutLeft(uint16_t code){
    // write this
    // Consider the following registers:
	  // SSI1_SR_R, SSI1_DR_R
		while((SSI3_SR_R&0x00000002)==0){};// SSI Transmit FIFO Not Full, waits for full
		SSI3_DR_R = code; //data out
		while((SSI3_SR_R&0x00000004)==0){};// SSI Receive FIFO Not Empty
		uint32_t receive = SSI3_DR_R; // acknowledge response 
}

void DAC_OutRight(uint16_t code){
    // write this
    // Consider the following registers:
	  // SSI1_SR_R, SSI1_DR_R
		while((SSI2_SR_R&0x00000002)==0){};// SSI Transmit FIFO Not Full, waits for full
		SSI2_DR_R = code; //data out
		while((SSI2_SR_R&0x00000004)==0){};// SSI Receive FIFO Not Empty
		uint32_t receive = SSI2_DR_R; // acknowledge response 
}

// --------------     DAC_OutNonBlocking   ------------------------------------
// Send data to TLV5616 12-bit DAC without checking for room in the FIFO
// inputs:  voltage output (0 to 4095)
// 
void DAC_Out_NB(uint16_t code){
    // Consider writing this (If it is what your heart desires)????
    // Consider the following registers:
	  // SSI1_SR_R, SSI1_DR_R
		while((SSI0_SR_R&0x00000002)==0){};// SSI Transmit FIFO Not Full
		SSI0_DR_R = code;
}

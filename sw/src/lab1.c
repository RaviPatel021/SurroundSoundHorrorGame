// ----------------------------------------------------------------------------
// Lab1.c
// Jonathan Valvano
// July 8, 2024
// Possible main program to test the lab
// Feel free to edit this to match your specifications
#include <stdint.h>
#include "inc/PLL.h"
#include "../inc/tm4c123gh6pm.h"
#include "../inc/I2C0.h"

void Clock_Delay1ms(uint32_t n);
void LED_Init(void);
void LED_Out(uint32_t data);
void Switch_Init(void);
uint32_t Switch_In(void);
int32_t fixed_atan(int16_t y, int16_t x);
int32_t fixed_atan2(int16_t y, int16_t x);
struct State {
	uint32_t Out;
	uint32_t Next[2];
}typedef State_t;

#define R0 0
#define T1 1
#define R1 2
#define T2 3
#define R2 4
#define T3 5
#define R3 6
#define T4 7
#define R4 8
#define T5 9
#define R5 10
#define T0 11

            // 270 degrees in fixed-point (270 * (1 << FIXED_POINT_SHIFT))



State_t FSM[12]={
	{0x0, {R0, T1}},
	{0x0, {R1, T1}},
	{0x0, {R1, T2}},
	{0x0, {R2, T2}},
	{0x0, {R2, T3}},
	{0x01, {R3, T3}},
	{0x01, {R3, T4}},
	{0x01, {R4, T4}},
	{0x01, {R4, T5}},
	{0x01, {R5, T5}},
	{0x01, {R5, T0}},
	{0x0, {R0, T0}},
};

uint32_t atanTable[90] = {
	9, 26, 44, 61, 79, 96, 114, 132, 149, 167,
	185, 203, 222, 240, 259, 277, 296, 315, 335, 354,
	374, 394, 414, 435, 456, 477, 499, 521, 543, 566,
	589, 613, 637, 662, 687, 713, 740, 767, 795, 824,
	854, 885, 916, 949, 983, 1018, 1054, 1091, 1130, 1171, 
	1213, 1257, 1303, 1351, 1402, 1455, 1511, 1570, 1632, 1698,
	1768, 1842, 1921, 2006, 2097, 2194, 2300, 2414, 2539, 2675,
	2824, 2989, 3172, 3376, 3606, 3867, 4165, 4511, 4915, 5396,
	5976, 6691, 7596, 8777, 10385, 12706, 16350, 22904, 38188, 114589};
	
uint32_t currentState;
uint32_t Input;
int16_t dataX[100];
int16_t dataY[100];
int16_t dataZ[100];
int16_t degrees[100];
int16_t tempX;
int16_t tempY;
//int32_t convertedData[100];
static uint32_t pointer;
int main(void){
  PLL_Init(Bus80MHz); 
	I2C_Init();
  //LED_Init();
  //Switch_Init();
  // Write something to declare required variables
  // Write something to initalize the state of the FSM, LEDs, and variables as needed
	I2C_Send2(0x0D, 0x0A, 0x80);
	//currentState = R0;
	I2C_Send2(0x0D, 0x0B, 0x01);
	I2C_Send2(0x0D, 0x09, 0x1D);
	
	//I2C_Send1(0x0D, 0x0B);
	//int8_t test = I2C_Recv(0x0D);
	//I2C_Send1(0x0D, 0x0D);
	//int8_t test2 = I2C_Recv(0x0D);
	pointer = 0;
	int8_t ready;
  while(1){
      // Write something using Switch_In() and LED_Out() to implement the behavior in the lab doc
		
		//Input = Switch_In();
		//currentState = FSM[currentState].Next[Input];
		//LED_Out(FSM[currentState].Out);
		I2C_Send1(0x0D, 0x06);
		ready = I2C_Recv(0x0D)&0x01;
		if(ready==1){
			I2C_Send1(0x0D, 0x00);
			tempX = I2C_Recv2(0x0D);
			dataX[pointer%100] = tempX;
			
			I2C_Send1(0x0D, 0x02);
			tempY = I2C_Recv2(0x0D);
			dataY[pointer%100] = tempY;
			
			//convertedData[pointer%100] = fixed_atan2(tempY, tempX);
			
			I2C_Send1(0x0D, 0x04);
			dataZ[pointer%100] = I2C_Recv2(0x0D);
			  uint32_t ratio = (((dataY[pointer%100]<0) ? -1*dataY[pointer%100] : dataY[pointer%100])*1000)/ ((dataX[pointer%100]<0) ? -1*dataX[pointer%100] : dataX[pointer%100]);
				for(int i=0; i<90; i++){
					if(ratio <= atanTable[i]){
						if((dataX[pointer%100]>0) && (dataY[pointer%100]>0)){
							degrees[(pointer++)%100] = 180-i;
							break;
						}
						else if((dataX[pointer%100]>0) && (dataY[pointer%100]<0)){
							degrees[(pointer++)%100] = 180+i;
							break;
						}
						else if((dataX[pointer%100]<0) && (dataY[pointer%100]<0)){
							degrees[(pointer++)%100] = 360-i;
							break;
						}
						else{
							degrees[(pointer++)%100] = i;
							break;
						}
						
					}
					else{
						if(i==89){
							if((dataX[pointer%100]>0) && (dataY[pointer%100]>0)){
								degrees[(pointer++)%100] = 90;
								break;
							}
							else if((dataX[pointer%100]>0) && (dataY[pointer%100]<0)){
								degrees[(pointer++)%100] = 270;
								break;
							}
							else if((dataX[pointer%100]<0) && (dataY[pointer%100]<0)){
								degrees[(pointer++)%100] = 270;
								break;
							}
							else{
								degrees[(pointer++)%100] = 90;
								break;
							}
						}
					}
				}
			
		}
  } 
} 




void LED_Init(void){
    // Write something to initalize the GPIOs that drive the LEDs based on your EID as defined in the lab doc.
	SYSCTL_RCGCGPIO_R |= 0x00000004;  // activate clock for Port C

  Clock_Delay1ms(10);        // allow time for clock to start

	
  GPIO_PORTC_DIR_R |= 0x60;         // PC5 out
  GPIO_PORTC_DEN_R |= 0x60;         // enable digital I/O on PC5


}
void LED_Out(uint32_t data){
		if(data){
			GPIO_PORTC_DATA_R |= 0x60;
		}
		else{
			GPIO_PORTC_DATA_R &= ~0x60;
		}
}
void Switch_Init(void){
    // write something to initalize the GPIO that take input from the switches based on your EID as defined in the lab doc
	SYSCTL_RCGCGPIO_R |= 0x00000010;  // activate clock for Port E

  Clock_Delay1ms(10);        // allow time for clock to start

  GPIO_PORTE_DIR_R &= ~0x08;         // PE3 in
  GPIO_PORTE_DEN_R |= 0x08;         // enable digital I/O on PE3
}
uint32_t Switch_In(void){
  // write something that reads the state of the GPIO pin as required
	Clock_Delay1ms(10);
  return (GPIO_PORTE_DATA_R&0x08)==0x08;
}

void Clock_Delay(uint32_t ulCount){
  while(ulCount){
    ulCount--;
  }
}

// ------------Clock_Delay1ms------------
// Simple delay function which delays about n milliseconds.
// Inputs: n, number of msec to wait
// Outputs: none
void Clock_Delay1ms(uint32_t n){
  while(n){
    Clock_Delay(23746);  // 1 msec, tuned at 80 MHz, originally part of LCD module
    n--;
  }
}

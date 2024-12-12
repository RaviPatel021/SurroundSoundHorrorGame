#include "digitalCompass.h"
#include "../inc/tm4c123gh6pm.h"
#include "inc/UART.h"
#include "../inc/PLL.h"
#include "inc/LaunchPad.h"
#include <stdint.h>
#include "audio.h"
#include "inc/ST7735withSD.h"



void DisableInterrupts(void);           // Disable interrupts
void EnableInterrupts(void);            // Enable interrupts
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

int16_t maxX, maxY, minX, minY;

void printNum(int16_t x) {
	if(x < 0) {
		UART_OutChar('-');
		x = -x;
	}
	UART_OutUDec(x);
}

void drawNum(int16_t x) {
	if(x < 0) {
		ST7735_OutChar('-');
		x = -x;
	}
	ST7735_OutUDec(x);
}
	
void compassInit(void){
	
	const uint32_t porte = 0x10; 
	const uint32_t pin = 0x03; // Mask for PE1 
	
	SYSCTL_RCGCGPIO_R |= porte; // Initialize PORTE 
	while((SYSCTL_PRGPIO_R & porte) == 0) {} // Wait for PORTE to be ready 
	
	GPIO_PORTE_DIR_R &= ~pin;   // PE1 set as input
	GPIO_PORTE_PUR_R |= pin;    // Enable pull-up resistor on PE1, we use negative logic
	GPIO_PORTE_DEN_R |= pin;    // Digital enable for PE1
		
	int16_t val = 0;
		
	do {
	
	
		Clock_Delay1ms(10);
		I2C_Send2(0x0D, 0x0A, 0x80); // soft reset 
		Clock_Delay1ms(10);
		I2C_Send2(0x0D, 0x0B, 0x01); 
		Clock_Delay1ms(10);
		I2C_Send2(0x0D, 0x09, 0x11);
		Clock_Delay1ms(10);	
		//UART_OutChar('.');
		val = compassRead();
	} while(val == -1);
	
	UART_OutString("compass init success with angle: ");
	UART_OutUDec(val); UART_OutString("\r\n");
}


int16_t tempX = 1;
int16_t tempY = 1;
int16_t tempZ = 1;

void remapValues(int16_t* x, int16_t* y) {
	const int16_t offsetX = -41;
	const int16_t offsetY = 360;
	const int16_t rangeX = 1413;
	const int16_t rangeY = 1594;
	
	*x = (*x - offsetX);
	*y = (*y - offsetY) * rangeX / rangeY;
}


	

static int8_t ready;
static int16_t calAngle = 0;

int16_t calcAngle(void) {
	uint32_t ratio = 0;

	if (tempX != 0) {
			ratio = (uint32_t)((tempY < 0 ? -tempY : tempY) * 1000) / (tempX < 0 ? -tempX : tempX);
	} else {
			// Handle the case where tempX is zero
			if (tempY > 0) {
					return 90;  // 90 degrees
			} else if (tempY < 0) {
					return 270;   // 270 degrees
			} else {
					return 0;     // 0 degrees (both X and Y are zero)
			}
	}

	// Find the angle using a lookup table (this is an approximation)
	for (int i = 0; i < 90; i++) {
			if (ratio <= atanTable[i]) {
					if (tempX > 0 && tempY > 0) {
							return 180 - i; // 1st quadrant
					} else if (tempX > 0 && tempY < 0) {
							return 180 + i; // 4th quadrant
					} else if (tempX < 0 && tempY < 0) {
							return 360 - i; // 3rd quadrant
					} else {
							return i; // 2nd quadrant
					}
			}
	}

	// In case no match is found, return 90 or 270 based on quadrant
	if (tempX > 0 && tempY > 0) return 90; // 1st quadrant
	if (tempX > 0 && tempY < 0) return 270; // 4th quadrant
	if (tempX < 0 && tempY < 0) return 270; // 3rd quadrant
	return 90; // 2nd quadrant
}

 void hardIronCalDebug(void) {
	 
	I2C_Send1(0x0D, 0x00);
	tempX = I2C_Recv2(0x0D);
	
	I2C_Send1(0x0D, 0x02);
	tempY = I2C_Recv2(0x0D);

	I2C_Send1(0x0D, 0x04);
	tempZ = I2C_Recv2(0x0D);
	
	if(tempX > maxX) maxX = tempX;
	if(tempX < minX) minX = tempX;
	if(tempY > maxY) maxY = tempY;
	if(tempY < minY) minY = tempY;
	 
	int16_t angle = calcAngle();
	 
	remapValues(&tempX, &tempY);
	 
	int16_t mappedAngle = calcAngle();
	
	ST7735_FillScreen(ST7735_BLACK);
	ST7735_SetCursor(1,1); ST7735_OutUDec(angle);
	
	ST7735_DrawString(1, 2, "rangeX: ", ST7735_BLUE); ST7735_SetCursor(9,2); drawNum(maxX - minX);
	ST7735_DrawString(1, 3, "offsetX: ", ST7735_BLUE); ST7735_SetCursor(10,3); drawNum((minX + maxX) / 2);
	
	ST7735_DrawString(1, 4, "rangeY: ", ST7735_BLUE); ST7735_SetCursor(9,4); drawNum(maxY - minY);
	ST7735_DrawString(1, 5, "offsetY: ", ST7735_BLUE); ST7735_SetCursor(10,5); drawNum((minY + maxY) / 2);
	
	ST7735_SetCursor(1,6); ST7735_OutUDec(mappedAngle);
	
}

int16_t getAngle(void){
//	I2C_Send1(0x0D, 0x00); // X lsb
//	ux = I2C_Recv(0x0D);
//	I2C_Send1(0x0D, 0x01); // x msb
//	tempX = ((I2C_Recv(0x0D) << 8) | ux);
//	
//	
//	I2C_Send1(0x0D, 0x02); 
//	uy = I2C_Recv(0x0D);
//	I2C_Send1(0x0D, 0x03); 	
//	tempY = ((I2C_Recv(0x0D) << 8) | uy);
//	
//	I2C_Send1(0x0D, 0x04);
//	uz = I2C_Recv(0x0D);
//	I2C_Send1(0x0D, 0x05); 
//	tempZ = ((I2C_Recv(0x0D) << 8) | uz);

	I2C_Send1(0x0D, 0x00);
	tempX = I2C_Recv2(0x0D);
	
	I2C_Send1(0x0D, 0x02);
	tempY = I2C_Recv2(0x0D);

	I2C_Send1(0x0D, 0x04);
	tempZ = I2C_Recv2(0x0D);
	
	remapValues(&tempX, &tempY);
	
	
	
	
//	UART_OutString("x=");
//	if(tempX<0){
//		UART_OutString("-");
//		UART_OutUDec(-1*tempX);
//	}
//	else{
//		UART_OutUDec(tempX);
//	}			

//	UART_OutString(", y=");
//	if(tempY<0){
//		UART_OutString("-");
//		UART_OutUDec(-1*tempY);
//	}
//	else{
//		UART_OutUDec(tempY);
//	}

//	UART_OutString(", z=");
//	if(tempZ<0){
//		UART_OutString("-");
//		UART_OutUDec(-1*tempZ);
//	}
//	else{
//		UART_OutUDec(tempZ);
//		
//	}
//	UART_OutString("\r\n");

	return calcAngle();
}


int16_t compassRead(void){
	I2C_Send1(0x0D, 0x06); // Status register (QMC5883L)
	ready = I2C_Recv(0x0D) & 0x01; // Check if data is ready (DRDY bit)
	if(ready==1){
		int16_t angle = getAngle();
		return angle;
	}
	return -1; // Return -1 if data is not ready

}

void checkAndCalibrateCompass(void){ 
		int16_t value = compassRead();
		if(value != -1){
			calAngle = value;	
		}
		Clock_Delay1ms(20);
}


// returns angle
int16_t checkAndTrigger(void){
	int16_t value = compassRead();

	if(value != -1){
		value = (value - calAngle + 360)%360;
	}	
	Clock_Delay1ms(20);
	
	return value;
}

enum soundDirection getDirection(void) {
	int16_t angle = checkAndTrigger();
	//UART_OutUDec(angle); UART_OutString("\r\n");
	/*
	if(angle >= 337 ||  angle < 23) return SDIR_FRONT;
	if(23 <= angle && angle < 68) return SDIR_DIAGLEFT;
	if(68 <= angle && angle < 158) return SDIR_LEFT;
	if(68 <= angle && angle < 158) return SDIR_LEFT;
	return SDIR_OTHER;
	*/
	if(angle >= 315 ||  angle < 45) return SDIR_FRONT;
	if(45 <= angle && angle < 135) return SDIR_LEFT;
	if(225 <= angle && angle < 315) return SDIR_RIGHT;
	return SDIR_OTHER;
}






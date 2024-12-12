#include "led.h"

#define pin0 0x1
#define pin1 0x2
#define pin2 0x4
#define pin3 0x8
#define portb 0x2


//init PB0 and PB1 as LED
void ledInit(void){
	const uint32_t pins = pin0 | pin1 | pin2 | pin3;
	SYSCTL_RCGCGPIO_R |= portb;
	while((SYSCTL_PRGPIO_R&portb) == 0){};
	GPIO_PORTB_DIR_R |= pins;
	GPIO_PORTB_DEN_R |= pins;
}

void ledOn(uint32_t pin){
	GPIO_PORTB_DATA_R |= pin;
}

void ledOff(uint32_t pin){
	GPIO_PORTB_DATA_R &= ~(pin);
}


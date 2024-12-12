#ifndef LED_H
#define LED_H

#include "../inc/tm4c123gh6pm.h"
#include <stdint.h>


void ledInit(void);
void ledOn(uint32_t pin);

// -- ** WARNING ** --
// ONLY USE IF LED SHOULD TURN OFF
// DO NOT USE IF LED SHOULD TURN ON
void ledOff(uint32_t pin);


#endif

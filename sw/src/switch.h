// File **********Switch.h***********
// Lab 4
// Programs to interface with Switch buttons   
// EE445L Fall 2015
//    Jonathan W. Valvano 9/22/15
// 2-bit input, positive logic switches, positive logic software

#ifndef SWITCH_H
#define SWITCH_H

#include <stdint.h>

#define pin0 0x1
#define pin1 0x2
#define pin2 0x4
#define pin3 0x8
    
void ignorePressesHandler(void);
void switchInit(void);
void ignorePressesFor1ms(void);
uint32_t switchPressed(uint32_t pin);


#endif

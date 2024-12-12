
#include <stdint.h>
#include "../inc/I2C0.h"
#include "audio.h"


// I2C0SCL connected to PB2
// I2C0SDA connected to PB3

void compassInit(void);
void checkAndCalibrateCompass(void);
int16_t checkAndTrigger(void);
int16_t compassRead(void);
enum soundDirection getDirection(void);
 void hardIronCalDebug(void);

void Clock_Delay1ms(uint32_t n);

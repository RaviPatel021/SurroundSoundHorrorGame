#ifndef H_SCARYGAME
#define H_SCARYGAME

#include <stdint.h>

void onGunshotSoundEnd(void);
void onWinLoseSoundEnd(void);
void onDeathSoundEnd(void);
uint32_t getScore(void);
void onMonsterLoop(uint32_t numLoops);
#endif
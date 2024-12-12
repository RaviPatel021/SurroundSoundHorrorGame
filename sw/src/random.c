#include "random.h"
#include "scarygame.h"

uint32_t rseed = 17202;

void setSeed(uint32_t s) {
	rseed = s;
}

uint32_t hash(uint32_t x) {
	return x * 971 + 1487;
}

enum soundDirection getRandomDirection(void) {
	uint32_t score = getScore();
	// first 3 directions hardcoded for tutorial purposes
	if(score == 0) return SDIR_LEFT;
	if(score == 1) return SDIR_RIGHT;
	if(score == 2) return SDIR_FRONT;
	
	rseed = hash(rseed);
	return (enum soundDirection)(rseed % 3);
}
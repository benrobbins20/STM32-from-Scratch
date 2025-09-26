
#ifndef SYSTICK_H_
#define SYSTICK_H_

#include "stm32f4xx.h"
void msDelay(int msdelay);
void systick_1hz_interrupt(void);

#endif /* SYSTICK_H_ */

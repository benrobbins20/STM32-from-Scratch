#include "stm32f4xx.h"


#define CSR_ENABLE				(1U<<0)
#define CSR_CLKSOURCE			(1U<<2)
#define CSR_COUNTFLAG			(1U<<16)
#define CSR_TICKINT				(1U<<1)

#define SYSTICK_LOAD_VAL		16000
#define ONE_SEC_LOAD			16000000


void msDelay(int msdelay) {
	// set the reload value to 16000 cycles ~ 1ms
	SysTick->LOAD = SYSTICK_LOAD_VAL;

	// set current value to 0 to reset counter
	SysTick->VAL = 0;

	// turn on systick and enable internal clock
	SysTick->CTRL = CSR_ENABLE | CSR_CLKSOURCE; // 001 | 100 == 101, this sets STCSR register to 0x5

	// wait until counter reaches 0 which will pop the count flag, exit for loop and reset the control register
	for (int i = 0;i < msdelay;i++) {
		while ((SysTick->CTRL & CSR_COUNTFLAG) == 0){}
	}
	SysTick->CTRL = 0; // reset
}

void systick_1hz_interrupt(void) {
	// set reload value count of 16000000
	SysTick->LOAD = ONE_SEC_LOAD - 1;

	// reset the current value
	SysTick->VAL = 0;

	// enable internal systick clock source
	SysTick->CTRL = CSR_ENABLE | CSR_CLKSOURCE;

	// enable interrupt for systick in the control registere
	SysTick->CTRL |= CSR_TICKINT;

}

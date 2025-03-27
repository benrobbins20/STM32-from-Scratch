#include "stm32f4xx.h"

#define TIM2EN			(1U<<0)
#define TIM3EN			(1U<<1)
#define CR1_CEN			(1U<<0)
#define OCM1_TOGGLE		((1U<<4) | (1U<<5)) // 0011 0000
#define CCER_CC1E		(1U<<0)
#define AFRL5			(1U<<20)
#define AFRL6			(1U<<25)
#define CCMR_CC1S		(1U<<0)

void timer2_1Hz_init(void) {
	/*
	stm32f4 base clock 16MHz
	prescalar 1600 divisions
	period 10000 cycles
	16000000 /
	16000000
	--------
	1Hz
	*/

	RCC->APB1ENR |= TIM2EN;
	TIM2->PSC = 1600 - 1;
	TIM2->ARR = 10000 -1;
	TIM2->CNT = 0; // this would have to be 32 bit mode to fit 16 million counts
	TIM2->CR1 = CR1_CEN; // can overwrite this entire register
}

void timer2_1Hz_compare(void) {
	// configure gpioa for alternate function
	RCC->AHB1ENR |= (1U<<0);
	GPIOA->MODER &= ~(1U<<10);
	GPIOA->MODER |= (1U<<11);

	// set alternate function for AF01 -> gpioa timer2
	GPIOA->AFR[0] |= AFRL5;

	/* use the timer compare function to trigger led at 1 Hz
	 16000000 /
	 (1600*10000)
	 -----------
	 1Hz
	 */
	RCC->APB1ENR |= TIM2EN;
	TIM2->PSC = 1600 - 1;
	TIM2->ARR = 10000 - 1;

	// toggle when counter equals
	TIM2->CCMR1 = OCM1_TOGGLE;

	// timer 2 channel 1 compare mode
	TIM2->CCER |= CCER_CC1E;

	// clear counter and start
	TIM2->CNT = 0;
	TIM2->CR1 = CR1_CEN;
}

void timer3_input_capture(void) {

	// configure GPIOA pin 6 for alternate function 13,12 = 10
	RCC->AHB1ENR |= (1U<<0);
	GPIOA->MODER &= ~(1U<<12);
	GPIOA->MODER |= (1U<<13);

	// set PA6 alternate function TIM3_CH1, 27,26,25,24 -> 0010
	GPIOA->AFR[0] |= AFRL6;

	RCC->APB1ENR |= TIM3EN;
	TIM3->PSC = 16000 - 1;

	// set timer 3 channel 1 to input mode
	TIM3->CCMR1 |= CCMR_CC1S;

	// enable timer 3 channel 1, default to rising edge signal
	TIM3->CCER = CCER_CC1E;
	TIM3->CR1 = CR1_CEN;

}


#include "exti.h"

#define GPIOCEN		(1U<<2)
#define SYSCFGEN	(1U<<14)
#define EXTI_PC		(1U<<5)
#define PIN13		(1U<<13)

// setup the push button to trigger the interrupt
void pc13_exti_init(void) {
	__disable_irq();

	// set gpioc:PC13 to input mode
	RCC->AHB1ENR |= GPIOCEN;
	GPIOC->MODER &= ~(1U<<26);
	GPIOC->MODER &= ~(1U<<27);

	// provide clock access to the system configuration register to configure interrupt
	RCC->APB2ENR |= SYSCFGEN;

	// last register of exti config register [3]
	SYSCFG->EXTICR[3] |= EXTI_PC;

	// unmask ping 13
	EXTI->IMR |= (1U<<13);

	// enable falling trigger for pin 13
	EXTI->FTSR |= (1U<<13);

	// use cortex m4 library to enable exti
	NVIC_EnableIRQ(EXTI15_10_IRQn);

	__enable_irq();
}

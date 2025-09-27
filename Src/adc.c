#include "stm32f4xx.h"
#include "adc.h"

#define GPIOAEN			(1U<<0)
#define ADC1EN			(1U<<8)
#define ADC_CH1			(1U<<0)
#define ADC_SEQ_LEN		0x00
#define CR2_ADON		(1U<<0)
#define CR2_SWSTART		(1U<<30)
#define SR_EOC			(1U<<1)
#define CR2_CONT		(1U<<1)
#define CR1_EOCIE		(1U<<5)

void adc_init(void) {
	RCC->AHB1ENR |= GPIOAEN; // port a is gpioA
	GPIOA->MODER |= (1U<<2); // set gpioa to analog mode 11
	GPIOA->MODER |= (1U<<3);

	// enable clock access APB2->ADC1EN
	RCC->APB2ENR |= ADC1EN;
	// set only one adc sequence, reset sqr1 sqr2 sqr3
	ADC1->SQR3 = ADC_CH1;
	// set length to only 1 conversion (0000)
	ADC1->SQR1 = ADC_SEQ_LEN;
	// toggle adc onwith control register 2, bit 0
	ADC1->CR2 |= CR2_ADON;
}

// set up interrupt to be triggered by adc end-of-conversion
void adc_interrupt_init(void) {
	RCC->AHB1ENR |= GPIOAEN;
	// analog mode
	GPIOA->MODER |= (1U<<2);
	GPIOA->MODER |= (1U<<3);

	// enable clock access APB2->ADC1EN
	RCC->APB2ENR |= ADC1EN;

	// enable EOC interrupt
	ADC1->CR1 |= CR1_EOCIE;

	// enable interrupt in NVIC
	NVIC_EnableIRQ(ADC_IRQn);

	// set only one adc sequence, reset sqr1 sqr2 sqr3
	ADC1->SQR3 = ADC_CH1;

	// set length to only 1 conversion (0000)
	ADC1->SQR1 = ADC_SEQ_LEN;

	// toggle adc on with control register 2, bit 0
	ADC1->CR2 |= CR2_ADON;
}

void start_conversion(void) {
	// set CR2 continuous sampling flag
	ADC1->CR2 |= CR2_CONT;
	// set CR2 swstart bit
	ADC1->CR2 |= CR2_SWSTART;
}

uint32_t adc_read(void) {
	// check adc status of EOC, end of conversion bit
	// while both are not equal to 1, wait until equal to 1 before reading
	while (!(ADC1->SR & SR_EOC)) {}

	// return the data from the adc
	return (ADC1->DR);
}

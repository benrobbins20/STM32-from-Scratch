#include <stdio.h>
#include <stdint.h>
#include "stm32f4xx.h"

#include "uart.h"
#include "adc.h"
#include "systick.h"
#include "timer.h"
#include "exti.h"

#define GPIOAEN			(1U<<0)
#define PIN5			(1U<<5)

static void exti_callback(void);
static void uart_callback(void);
static void adc_callback(void);
static void systick_callback(void);
static void timer2_callback(void);

// for ADC
uint32_t sensor_value;

// for uart input
char key;

int main(void) {
	// enable LED
	RCC->AHB1ENR |= GPIOAEN;
	GPIOA->MODER |= (1U<<10);
	GPIOA->MODER &= ~(1U<<11);

	// User button interrupt
	// pc13_exti_init();
	// usart2_tx_init();

	// uart input (RX) interrupt
	// usart2_rx_interrupt_init();

	// ADC EOC interrupt enable
	// adc_interrupt_init();
	// usart2_tx_init();
	// start_conversion();

	// SysTick interrupt
	// usart2_tx_init();
	// systick_1hz_interrupt();

	// Timer 2 interrupt
	usart2_tx_init();
	timer2_1Hz_interrupt_init();

	while(1) {}
}

static void exti_callback(void) {
	printf("button pressed\n\r");
	GPIOA->ODR ^= PIN5;
}

void EXTI15_10_IRQHandler (void) {
	// if bit 13 is set to 1, interrupt occured -> perform routine
	if((EXTI->PR & LINE13) != 0) {
		// writing 1 to the pending register is reset
		EXTI->PR |= LINE13;

		// print
		exti_callback();
	}
}

static void uart_callback(void) {
	key = USART2->DR;
	// turn on LED with the input 1
	if (key == '1') {
		GPIOA->ODR |= PIN5;
	}
	else {
		GPIOA->ODR &= ~(PIN5);
	}
}

void USART2_IRQHandler(void) {
	// check the uart status register for not empty
	if (USART2->SR & SR_RXNE) {
		uart_callback();
	}
}

static void adc_callback(void) {
	sensor_value = ADC1->DR;
	printf("Val: %d\n\r", (int)sensor_value);
}

void ADC_IRQHandler(void) {
	// if status register is populated, clear status bit and do callback()
	if ((ADC1->SR & SR_EOC) != 0) {
		ADC1->SR &= ~(SR_EOC);
		adc_callback();
	}
}

static void systick_callback(void) {
	printf("1 second\n\r");
	GPIOA->ODR ^= PIN5;
}

void SysTick_Handler(void) {
	systick_callback();
}

static void timer2_callback(void) {
	printf("1 second\n\r");
	GPIOA->ODR ^= PIN5;
}

void TIM2_IRQHandler(void) {
	TIM2->SR &= ~SR_UIF; // clear status register
	timer2_callback();
}

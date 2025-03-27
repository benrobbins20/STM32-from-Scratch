#include "uart.h"

// GPIO registers
#define GPIOAEN			(1U<<0)
#define UART2EN			(1U<<17)

// UART registers
#define CR1_TE			(1U<<3)
#define CR1_RE			(1U<<2)
#define CR1_UE			(1U<<13)
#define SR_TXE			(1U<<7)
#define SR_RXNE			(1U<<5)
#define CR1_RXNEIE		(1U<<5)

// stm32 clock speed default 16MHz
#define SYS_FREQ 		16000000
// abp1 bus same clock speed
#define APB1_CLK  		SYS_FREQ
// standard uart baud rate
#define UART_BAUDRATE	115200

// declare functions
static void set_baudrate(USART_TypeDef *USARTx, uint32_t PClk, uint32_t Baudrate);
static uint16_t compute_uart_div(uint32_t PClk, uint32_t Baudrate);
void usart2_rxtx_init();
void usart2_tx_init();
void usart2_write(int ch);
char usart2_read(void);

// use __io directive to redirect printf
int __io_putchar(int ch) {
	usart2_write(ch);
	return ch;
}

// set uart bits 5,4 to 10, set uart bits 7,6 to 10
void usart2_rxtx_init(void) {
	RCC->AHB1ENR |= GPIOAEN;
	// USART2_RX
	// MODER PA2 bits 5,4->1,0
	GPIOA->MODER &= ~(1U<<4); // bit 4 mask to 0
	GPIOA->MODER |= (1U<<5); // bit 5 mask to 1

	// USART2_TX
	// MODER bits PA3 bits 7,6->1,0
	 GPIOA->MODER &= ~(1U<<6);
	 GPIOA->MODER |= (1U<<7);

	// TX alternate function pin2
	// set pin2 AFT, AFRL lower half at index 0, 11,10,9,8 -> 0111 (AF07)
	GPIOA->AFR[0] |= (1U<<8);
	GPIOA->AFR[0] |= (1U<<9);
	GPIOA->AFR[0] |= (1U<<10);
	GPIOA->AFR[0] &= ~(1U<<11);

	// RX alternate function pin3
	// set pin3 AFRL 15,14,13,12 -> 0111 (AF07)
	GPIOA->AFR[0] |= (1U<<12);
	GPIOA->AFR[0] |= (1U<<13);
	GPIOA->AFR[0] |= (1U<<14);
	GPIOA->AFR[0] &= ~(1U<<15);

	// APB1 bus clock access to uart2
	RCC->APB1ENR |= UART2EN;

	// confure baudrate to BRR register of usart2
	set_baudrate(USART2, APB1_CLK, UART_BAUDRATE);

	// transmit enable and receive enable
	USART2->CR1 = (CR1_TE | CR1_RE);

	// enable uart UE bit 12, start and stop bits are set to default with above line, 0 is reset for 32 bit register
	USART2->CR1 |= CR1_UE;
}

// initialize only TX
void usart2_tx_init(void) {
	RCC->AHB1ENR |= GPIOAEN;

	// USART2_TX
	// alternate function configured in gpio mode
	// PA2 bits 5,4->1,0
	GPIOA->MODER &= ~(1U<<4); // bit 4 mask to 0
	GPIOA->MODER |= (1U<<5); // bit 5 mask to 1

	//USART2_RX, MODER bits PA3 bits 7,6->10
	// GPIOA->MODER &= ~(1U<<6);
	// GPIOA->MODER |= (1U<<7);

	// set pin2 alternate function, AFRL lower half at index 0, 11,10,9,8 -> 0111
	GPIOA->AFR[0] |= (1U<<8);
	GPIOA->AFR[0] |= (1U<<9);
	GPIOA->AFR[0] |= (1U<<10);
	GPIOA->AFR[0] &= ~(1U<<11);

	// APB1 bus clock access to uart2
	RCC->APB1ENR |= UART2EN;

	// confure baudrate to BRR register of usart2
	set_baudrate(USART2, APB1_CLK, UART_BAUDRATE);

	// set the TE bit on usart control register, do not |=, can clobber the entire register
	USART2->CR1 = CR1_TE;

	// enable uart UE bit 12, start and stop bits are set to default with above line, 0 is reset for 32 bit register
	USART2->CR1 |= CR1_UE;
}

// initialize only RX
void usart2_rx_init(void) {
	RCC->AHB1ENR |= GPIOAEN;
	// USART2_RX
	// MODER bits PA3 bits 7,6->1,0
	 GPIOA->MODER &= ~(1U<<6);
	 GPIOA->MODER |= (1U<<7);

	// set pin3 AFRL 15,14,13,12 -> 0111 (AF07)
	GPIOA->AFR[0] |= (1U<<12);
	GPIOA->AFR[0] |= (1U<<13);
	GPIOA->AFR[0] |= (1U<<14);
	GPIOA->AFR[0] &= ~(1U<<15);

	// APB1 bus clock access to uart2
	RCC->APB1ENR |= UART2EN;

	// confure baudrate to BRR register of usart2
	set_baudrate(USART2, APB1_CLK, UART_BAUDRATE);

	// transmit enable and receive enable
	USART2->CR1 |= CR1_RE;

	// enable uart UE bit 12, start and stop bits are set to default with above line, 0 is reset for 32 bit register
	USART2->CR1 |= CR1_UE;
}

// interrupt on usart receive not empty (RXNE)
void usart2_rx_interrupt_init(void) {

	RCC->AHB1ENR |= GPIOAEN;
	// USART2_RX
	// MODER bits PA3 bits 5,4->1,0
	GPIOA->MODER &= ~(1U<<6);
	GPIOA->MODER |= (1U<<7);

	// RX alternate function
	// set pin3 AFRL 15,14,13,12 -> 0111 (AF07)
	GPIOA->AFR[0] |= (1U<<12);
	GPIOA->AFR[0] |= (1U<<13);
	GPIOA->AFR[0] |= (1U<<14);
	GPIOA->AFR[0] &= ~(1U<<15);

	// APB1 bus clock access to uart2
	RCC->APB1ENR |= UART2EN;

	// confure baudrate to BRR register of usart2
	set_baudrate(USART2, APB1_CLK, UART_BAUDRATE);

	// transmit enable and receive enable
	USART2->CR1 = CR1_RE;

	// enable receive not empty
	USART2->CR1 |= CR1_RXNEIE;

	// Enable UART2 interrupt in NVIC
	NVIC_EnableIRQ(USART2_IRQn);

	// enable uart UE bit 12, start and stop bits are set to default with above line, 0 is reset for 32 bit register
	USART2->CR1 |= CR1_UE;
}

static void set_baudrate(USART_TypeDef *USARTx, uint32_t PClk, uint32_t Baudrate) {
	USARTx->BRR = compute_uart_div(PClk, Baudrate);
}

static uint16_t compute_uart_div(uint32_t PClk, uint32_t Baudrate) {
	// (clock speed + baud/2) / baud for bandwidth per division
	return ((PClk + (Baudrate/2U)) / Baudrate);
}

void usart2_write(int ch) {
	// keep checking the transmit status register, only breaks out of while loop when register is clear
	while (!(USART2->SR & SR_TXE)){}
	// write data to usart data register with AND mask
	USART2->DR = (ch & 0xFF);
}

char usart2_read(void) {
	// check the receive status not empty register
	while (!(USART2->SR & SR_RXNE)) {}
	// return the read data input from pc
	return USART2->DR;
}

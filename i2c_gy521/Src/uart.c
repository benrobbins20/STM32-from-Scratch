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

// dma config
#define DMA1EN			(1U<<21)
#define DMA1_SX6_EN		(1U<<0)
#define CHSEL4			(1U<<27)
#define MINC			(1U<<10)
#define DIR				(1U<<6)
#define TCIE			(1U<<4)
#define DMAT			(1U<<7)

// stm32 clock speed default 16MHz
#define SYS_FREQ 		16000000
// abp1 bus same clock speed
#define APBx_CLK  		SYS_FREQ
// standard uart baud rate
#define UART_BAUDRATE	115200

// declare functions
static void set_baudrate(USART_TypeDef *USARTx, uint32_t PClk, uint32_t Baudrate);
static uint16_t compute_uart_div(uint32_t PClk, uint32_t Baudrate);
void usart2_rxtx_init();
void usart2_tx_init();
void usart2_write(int ch);
void usart1_write(int ch);
char usart2_read(void);

// use __io directive to redirect printf
int __io_putchar(int ch) {
	usart2_write(ch);
	usart1_write(ch);
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
	set_baudrate(USART2, APBx_CLK, UART_BAUDRATE);

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
	set_baudrate(USART2, APBx_CLK, UART_BAUDRATE);

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
	set_baudrate(USART2, APBx_CLK, UART_BAUDRATE);

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
	set_baudrate(USART2, APBx_CLK, UART_BAUDRATE);

	// transmit enable and receive enable
	USART2->CR1 = CR1_RE;

	// enable receive not empty
	USART2->CR1 |= CR1_RXNEIE;

	// Enable UART2 interrupt in NVIC
	NVIC_EnableIRQ(USART2_IRQn);

	// enable uart UE bit 12, start and stop bits are set to default with above line, 0 is reset for 32 bit register
	USART2->CR1 |= CR1_UE;
}

// usart1 config
void usart1_txrx_init(void) {
	// usart1 PA9-TX PA10-RX
	RCC->AHB1ENR |= GPIOAEN;

	// alternate function mode; 21,20 -> 10; 19,18 -> 10
	GPIOA->MODER &= ~((1U << 18) | (1U << 20));
	GPIOA->MODER |=  ((1U << 19) | (1U << 21));

	// set AF7 for PA9 PA10, in AFR HIGH!
	// PA9 -> AF7 0111
	GPIOA->AFR[1] |= (1U << 4) | (1U<<5) | (1U<<6);
	GPIOA->AFR[1] &= ~(1U<<7);
	// PA10 -> AF7 0111
	GPIOA->AFR[1] |= (1U << 8) | (1U<<9) | (1U<<10);
	GPIOA->AFR[1] &= ~(1U<<11);

	// usart1 on apb2 bus
	RCC->APB2ENR |= (1U<<4);

	// USART1 defined in f411xe header file
	set_baudrate(USART1, APBx_CLK, UART_BAUDRATE);

	// transmit enable and receive enable
	USART1->CR1 = (CR1_TE | CR1_RE);

	// enable uart UE bit 12, start and stop bits are set to default with above line, 0 is reset for 32 bit register
	USART1->CR1 |= CR1_UE;
}


// write direct to memory
void usart_tx_dma_init(uint32_t src, uint32_t dst, uint32_t len) {
	RCC->AHB1ENR |= DMA1EN;

	// disable stream 6 temporarily to reconfigure
	DMA1_Stream6->CR &= ~(DMA1_SX6_EN);

	// wait until stream 6 is disabled, do nothing until bit is set to 0
	while (DMA1_Stream6->CR & DMA1_SX6_EN) {}

	// clear all stream 6 flags 21:18:16
	DMA1->HIFCR |= (1U<<21);
	DMA1->HIFCR |= (1U<<20);
	DMA1->HIFCR |= (1U<<19);
	DMA1->HIFCR |= (1U<<18);
	DMA1->HIFCR |= (1U<<16);

	// perepheral access register, set the destination
	DMA1_Stream6->PAR = dst;

	// memory 0 address register
	DMA1_Stream6->M0AR = src;

	// set length of buffer, number of data registers
	DMA1_Stream6->NDTR = len;

	// set dma1 stream 6 to channel 4 - usart2 tx, can clear all bits
	DMA1_Stream6->CR = CHSEL4;

	// set auto increment
	DMA1_Stream6->CR |= MINC;

	// direction mem to peripheral - 01
	DMA1_Stream6->CR |= DIR;

	// enable transfer complete interrupt
	DMA1_Stream6->CR |= TCIE;

	// clear the fifo control register, set direct mode
	DMA1_Stream6->FCR = 0;

	// reenable stream 6
	DMA1_Stream6->CR |= DMA1_SX6_EN;

	// enable dma tx in usart2 control register
	USART2->CR3 |= DMAT;

	// enable interrupt in nvic
	NVIC_EnableIRQ(DMA1_Stream6_IRQn);
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

void usart1_write(int ch) {
	// keep checking the transmit status register, only breaks out of while loop when register is clear
	while (!(USART1->SR & SR_TXE)){}
	// write data to usart data register with AND mask
	USART1->DR = (ch & 0xFF);
}

char usart1_read(void) {
	while (!(USART1->SR & SR_RXNE)) {} // do nothing while receive empty

	return USART1->DR;
}

char usart2_read(void) {
	// check the receive status not empty register
	while (!(USART2->SR & SR_RXNE)) {}
	// return the read data input from pc
	return USART2->DR;
}

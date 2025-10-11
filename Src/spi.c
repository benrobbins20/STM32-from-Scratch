/*
 * spi.c
 *
 *  Created on: Sep 27, 2025
 *      Author: benro
 */
#include "spi.h"

#define SPI1_EN			(1U<<12)
#define GPIOB_EN		(1U<<1)
#define SPI1_AF			(0x2A)
#define SPI1_AF5		(0x555)
#define SPI1_BAUD		(0x01)
#define SPI1_TXE		(1U<<1)
#define SPI1_RXNE		(1U<<0)
#define SPI1_BSY		(1U<<7)
#define GPIOB_ODR10		(1U<<10)


// write bits 101010<<6
uint32_t write_reg(uint32_t reg, uint32_t value, int offset, uint32_t mask ) {
	// write zero to mask bits offset
	reg &= ~(mask << offset); // set value = 0
	// write value into field
	reg |= (value << offset);


	return reg;
}


void init_spi1_gpio(void) {
	// GPIOB
	RCC->AHB1ENR |= GPIOB_EN;

	// AF05 PB3 - SCK, PB4 - MISO, PB5 - MOSI, GPIOB MODER
	GPIOB->MODER = write_reg(GPIOB->MODER, SPI1_AF, 6, 0x3F);

	// write 0101 AF5, port b, pins 3,4,5
	GPIOB->AFR[0] = write_reg(GPIOB->AFR[0], SPI1_AF5, 12, 0xFFF);

	// manual CS gpio
	GPIOB->MODER |= (1U<<20);
}

void configure_spi(void) {
	// APB2 100 MHz
	RCC->APB2ENR |= SPI1_EN;

	// 16MHz / 4 baud rate
	SPI1->CR1 = write_reg(SPI1->CR1, SPI1_BAUD, 3, 0x07);


	// mode 3 CPOL & CPHA, receive on rising edge, send on falling edge
	SPI1->CR1 |= (1U<<0);
	SPI1->CR1 |= (1U<<1);

	// disable listen only mode, write 0 to bit 10
	SPI1->CR1 &= ~(1U<<10);

	// ensure default MSB transfer
	SPI1->CR1 &= ~(1U<<7);

	// set master mode bit 2
	SPI1->CR1 |= (1U<<2);

	// default 8 bit payload (register size of slave)
	SPI1->CR1 &= ~(1U<<11);

	// typical in master mode to take control of the slave select
	// use a gpio output pin to control the SS SPI pin
	SPI1->CR1 |= (1U<<9);
	SPI1->CR1 |= (1U<<8);

	//turn on spi
	SPI1->CR1 |= (1U<<6);

}

void spi1_transmit(uint8_t *data, uint32_t size) {
	// spi1 takes data
	uint32_t i = 0;
	uint8_t _temp; // read reg to clear

	while(i < size) {

		// Reads "While bit 1 is not high"
		while(!(SPI1->SR & SPI1_TXE)){}


		SPI1->DR = data[i];
		i++;
	}

	// after transmit is finished wait for TXE again
	while(!(SPI1->SR & SPI1_TXE)){}

	// while busy flag is set
	while(SPI1->SR & SPI1_BSY){}

	// sequentially read DR and SR into temp variable to clear overrun after all data is sent
	// this is reading the whole byte into the temp variable
	// another load instruction of the status register clears the overrun
	// its not necessarily a hack, its just using the functionality of spi to trigger the fifo queue to advance.
	_temp = SPI1->DR;
	_temp = SPI1->SR;
}

void spi1_receive(uint8_t *data, uint32_t size) {
	while(size > 0) {

		// sending a dummy byte triggers the master to start clock sequence and send data
		SPI1->DR = 0;

		// while receive not empty is not set
		// while receive is empty,  wait
		while(!(SPI1->SR & SPI1_RXNE)){}

		// fill the data buffer
		*data++ = SPI1->DR;
		size--;

	}
}

// trigger the slave select line with a GPIO
void cs_enable(void) {
	GPIOB->ODR &= ~GPIOB_ODR10; // write output LOW to trigger slave
}

void cs_disable(void) {
	GPIOB->ODR |= (GPIOB_ODR10);
}


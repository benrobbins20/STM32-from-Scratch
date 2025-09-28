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



// write bits 101010<<
uint32_t write_reg(uint32_t reg, uint32_t value, int offset, uint32_t mask ) {
	mask = 0x3F << offset;
	value = (value & mask) << offset;
	reg &= ~(mask); // set value = 0
	reg |= (value); // set value 2A<<6
	return reg;
}


void init_spi1_gpio(void) {
	// GPIOB
	RCC->AHB1ENR |= GPIOB_EN;

	// AF05 PB3 - SCK, PB4 - MISO, PB5 - MOSI, GPIOB MODER
	write_reg(GPIOB->MODER, SPI1_AF, 6, 0x3F);

	// write 0101 AF5, port b, pins 3,4,5
	write_reg(GPIOB->AFR[0], SPI1_AF5, 12, 0xFFF);

}

void configure_spi(void) {
	// APB2 100 MHz
	RCC->APB2ENR |= SPI1_EN;

	// 16MHz / 4 baud rate
	write_reg(SPI1->CR1, SPI1_BAUD, 3, 0x07);


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



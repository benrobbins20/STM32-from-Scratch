#include "stm32f4xx.h"

#define GPIOBENR		(1U<<1)
#define I2C1EN			(1U<<21)
#define STD_MAX_RISE_TIME	17
#define I2C_PE			(1U<<0)
#define SR2_BUSY		(1U<<1) // R
#define CR1_START		(1U<<8)	// RW
#define SR1_START		(1U<<0) // R
#define SR1_ADDR		(1U<<1) // R
#define SR1_TXE			(1U<<7)
#define SR1_RXNE		(1U<<6) // R
#define CR1_STOP		(1U<<9)
#define SR1_BTF			(1U<<2)
#define CR1_ACK			(1U<<10)

#define I2C_100KHZ		80



// function/macro for the freq field of i2c control register
// take the register and integer 16,  and the position
//
uint32_t set_freq(uint32_t reg, uint32_t value, uint8_t pos) {
	// 6 bit mask = 0x3f 0011:3 1111:f
	uint32_t mask = 0x3f << pos; // position is bits 0-5, so no change in mask, position = 0
	value = (value & 0x3f) << pos; // 010000 & 111111 = 010000
	reg &= ~(mask); // 000000 &= 101111, keeps 6-31 intact, clears 0-5
	reg |= value; // write bits
	return reg;
}

uint32_t set_standard_mode(uint32_t reg, uint32_t value, uint8_t pos) {
	// 8 bit mask, the CCR is technically 12 bits bit only setting 0101 0000
	uint32_t mask = 0xff << pos; // 1111 1111 << 0 = 1111 1111
	value = (value & 0xff) << pos; // 0101 0000 & 1111 1111 << 0 = 0101 0000
	reg &= ~(mask); // XXXX XXXX &= 1010 1111 essentially clearing the bit fields you want so 64,16; bits 6,4
	reg |= value; // write 1 to 64, 16
	return reg;
}

void i2c_init(void) {
	// clock access for AHB1 which is used to configure alternate functions for GPIOB
	RCC->AHB1ENR |= GPIOBENR;

	// alt func pb8;I2C1-SCL;bits 17,16:10
	GPIOB->MODER |= (1U<<17);
	GPIOB->MODER &= ~(1U<<16);

	// alt func pb9;I2C1-SDA;bits 19,18:10
	GPIOB->MODER |= (1U<<19);
	GPIOB->MODER &= ~(1U<<18);

	// alt func 4 pb8;3,2,1,0:0100
	GPIOB->AFR[1] &= ~(1U<<3);
	GPIOB->AFR[1] |= (1U<<2);
	GPIOB->AFR[1] &= ~(1U<<1);
	GPIOB->AFR[1] &= ~(1U<<0);

	// alt func 4 pb9;7,6,5,4:0100
	GPIOB->AFR[1] &= ~(1U<<7);
	GPIOB->AFR[1] |= (1U<<6);
	GPIOB->AFR[1] &= ~(1U<<5);
	GPIOB->AFR[1] &= ~(1U<<4);

	// set otype to open drain for pb8 pb9
	GPIOB->OTYPER |= (1U<<8);
	GPIOB->OTYPER |= (1U<<9);

	// set pull up mode for pb8 pb9 bits:01
	GPIOB->PUPDR &= ~(1U<<17);
	GPIOB->PUPDR |= (1U<<16);
	GPIOB->PUPDR &= ~(1U<<19);
	GPIOB->PUPDR |= (1U<<18);

	// all of the i2c pin set up complete, enable clock access to i2c1 on apb1
	RCC->APB1ENR |= I2C1EN;

	// set reset bit to 1 and then zero to reset the i2c
	I2C1->CR1 |= (1U<<15);
	I2C1->CR1 &= ~(1U<<15);

	// set i2c1 clock frequency to 16 MHz
	// I2C1->CR2 = set_freq(I2C1->CR2, 16, 0);
	I2C1->CR2 = (1U<<4);

	// standard mode 100 khz
	I2C1->CCR = I2C_100KHZ;

	// set default trise time 17
	I2C1->TRISE = STD_MAX_RISE_TIME;

	// enable i2c1 PE
	I2C1->CR1 |= I2C_PE;

}

// read bytes from a slave address,
// char is 1 byte, may also want to do uint8_t
void i2c1_readbyte(char saddr, char maddr, char* data) {
	// used as garbage variable for writing to it from data register before sending or receiving additional data
	volatile int tmp;

	// block until not busy
	while (I2C1->SR2 & (SR2_BUSY)) {}

	// set the start flag
	I2C1->CR1 |= CR1_START;

	// wait until the start flag is received
	while (!(I2C1->SR1 & SR1_START)){}

	// send the slave addr, wait for slave addr match
	I2C1->DR = saddr << 1;
	while (!(I2C1->SR1 & SR1_ADDR)){}

	// "read" the sr2 register, clearing the register
	tmp = I2C1->SR2; // read this in memory

	// send the memory address from the device
	I2C1->DR = maddr;

	// wait for SR2 txe = 1 = DR is empty, meaning can write again?
	while (!(I2C1->SR1 & SR1_TXE)){}; // while txe == 0- waiting

	// restart and wait
	I2C1->CR1 |= CR1_START;
	while (!(I2C1->SR1 & SR1_START)){}

	// set slave read (1U<<0)
	I2C1->DR = saddr << 1 | 1;
	while (!(I2C1->SR1 & SR1_ADDR)) {}

	// we want to disable ack for some reason, clear it
	I2C1->CR1 &= ~(CR1_ACK);

	// clear addr flag
	tmp = I2C1->SR2;

	I2C1->CR1 |= CR1_STOP;

	// wait for RXNE
	while (!(I2C1->SR1 & SR1_RXNE)){}

	// store byte, didnt know you could do this, store byte in the pointer location of data, then increment data i byte?
	*data++ = I2C1->DR;
}


void i2c1_burstread(char saddr, char maddr, int n, char* data) {
	// for writing/clearing to it to data register before sending or receiving additional data
	volatile int tmp;

	// block until not busy
	while (I2C1->SR2 & (SR2_BUSY)) {}

	// set the start flag
	I2C1->CR1 |= CR1_START;

	// wait until the start flag is received
	while (!(I2C1->SR1 & (SR1_START))){}

	// send the slave address 7 bits, shift left and leave bit 0 = 0 for write
	I2C1->DR = saddr << 1;

	// wait for address flag SR1 bit 1
	while (!(I2C1->SR1 & (SR1_ADDR))){} // will set to 1 when slave address is matched

	// "read" the sr2 register, clearing the register after SR receive slave address
	tmp = I2C1->SR2;

	// send mem addr, wait for TXE
	I2C1->DR = maddr;
	while (!(I2C1->SR1 & SR1_TXE)) {}

	// restart and wait for start bit
	I2C1->CR1 |= CR1_START;
	while (!(I2C1->SR1 & (SR1_START))) {}

	// send slave read, wait for addr, clean
	I2C1->DR = saddr << 1 | 1;
	while (!(I2C1->SR1 & (SR1_ADDR))){}
	tmp = I2C1->SR2;

	// enable ack
	I2C1->CR1 |= (CR1_ACK);


	// read n bytes, set ack to 0 when done
	while (n > 0U) // always compare to unsigned int, best practice
	{
		// 1 byte remaining, disable ack, generate stop and wait for RXNE
		if (n == 1U) {

			I2C1->CR1 &= ~(CR1_ACK);
			I2C1->CR1 |= CR1_STOP;
			while (!(I2C1->SR1 & SR1_RXNE)){};

			// do same thing as single byte, store in data, break out of loop
			(*data++) = I2C1->DR;
			break;
		}

		// reading stream of bytes
		else {
			// wait for receive
			while (!(I2C1->SR1 & SR1_RXNE)) {}; // not empty, wait until while NE bit is 0

			// straightforward, store byte in data and decrement n
			(*data++) = I2C1->DR;
			n--;
		}
	}
}

void i2c1_burstwrite(char saddr, char maddr, int n, char* data) {

	volatile int tmp;

	// wait for not busy
	while (I2C1->SR2 & (SR2_BUSY)) {}

	// start and wait
	I2C1->CR1 |= CR1_START;
	while (!(I2C1->SR1 & (SR1_START))){}

	// send slave write, clear addr flag
	I2C1->DR = saddr << 1;
	while (!(I2C1->SR1 & (SR1_ADDR))){} // will set to 1 when slave address is matched
	tmp = I2C1->SR2;

	// send mem addr, wait for TXE
	while (!(I2C1->SR1 & (SR1_TXE))) {}
	I2C1->DR = maddr;

	// write data n length into data
	for (int i = 0; i < n; i++) {
		// wait until not full
		while (!(I2C1->SR1 & (SR1_TXE))) {}
		// write to data register
		I2C1->DR = (*data++);
	}

	// wait for byte transfer finished
	while (!(I2C1->SR1 & (SR1_BTF))){}

	// stop
	I2C1->CR1 |= CR1_STOP;
}



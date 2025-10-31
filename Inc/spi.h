/*
 * spi.h
 *
 *  Created on: Sep 27, 2025
 *      Author: benro
 */

#ifndef SPI_H_
#define SPI_H_
#include "stm32f4xx.h"

void init_spi1_gpio(void);
void configure_spi(void);
void spi1_transmit(uint8_t *data, uint32_t size);
void spi1_receive(uint8_t *data, uint32_t size);
void cs_enable(void);
void cs_disable(void);

#endif /* SPI_H_ */

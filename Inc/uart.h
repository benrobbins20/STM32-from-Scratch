/*
 * uart.h
 *
 *  Created on: Feb 15, 2025
 *      Author: benro
 */
#include <stdint.h>
#include "stm32f4xx.h"

#ifndef UART_H_
#define UART_H_

void usart2_rxtx_init(void); // set up send and receive
void usart2_tx_init(void); // only set up send (for just printing to uart)
void usart2_rx_init(void);
void usart2_rx_interrupt_init(void);
char usart2_read(void);
void usart_tx_dma_init(uint32_t src, uint32_t dst, uint32_t len);

// for reading the status register in main
#define SR_RXNE			(1U<<5)
#define HISR_TCIF6		(1U<<21)
#define HIFCR_CTCIF6	(1U<<21)
#endif /* UART_H_ */

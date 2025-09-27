#include <stdio.h>
#include <stdint.h>
#include "stm32f4xx.h"
//#include "stm32f4xx_hal.h"
//#include "stm32f4xx_hal_i2c.h"
//#include "stm32f4xx_hal_i2c_ex.h"

#include "uart.h"
#include "adc.h"
#include "systick.h"
#include "timer.h"
#include "exti.h"
#include "adxl345.h"
#include "gy521.h"

#define GPIOAEN			(1U<<0)
#define PIN5			(1U<<5)

static void exti_callback(void);
static void uart2_callback(void);
static void adc_callback(void);
static void systick_callback(void);
static void timer2_callback(void);
static void uart1_callback(void);

// for ADC
uint32_t sensor_value;

// for UART input
char key;
char key1;

// byte for the mpu6050 device id check
uint8_t whoami;

// external reference to data gyro receive buffer
extern uint8_t data_recv[6];

// ADXL345
// 16 bits for combined axes
// uint16_t x,y,z;

// GY521/MPU6050
// int16! these need to be 2's compliment 16 bit signed int
int16_t x, y, z;
int16_t gy_x, gy_y, gy_z;

// floats t0 store 16 bit int * scale factor
float xg, yg, zg;
float Gy_x, Gy_y, Gy_z;

// scale factor milli-g / 1000
const float SCALE_FACTOR = 0.0078;

// scale factors for accel and gyro
const float MPU6050_ACCEL_SCALE = 16384.0;
const float MPU6050_GYRO_SCALE = 131.0;


int main(void) {
	// enable fpu
	// system control block, set bits 20-23 'full access'
	SCB->CPACR |= (0xF << 20);

	// flush data sync barrier and instruction sync barrier
	__DSB(); // let all memory accesses complete
	__ISB(); // flush 'queue' of instructions in pipeline so execution starts fresh with FPU enabled

	// SCB_GetFPUType();


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
	// usart2_tx_init();
	// timer2_1Hz_interrupt_init();

	// DMA write a message
//	static char message[] = "bob loblaw\n\r";
//	usart2_rxtx_init();
//	usart_tx_dma_init((uint32_t) message, (uint32_t) &USART2->DR,sizeof(message) - 1);
//
//	// write to usart1
//	usart1_txrx_init();
//
//	while(1) {
//		printf("bobobo\n\r");
//		for (volatile int i = 0; i < 1000000; i++);
//		key1 = usart1_read();
//	}


	// read accel data from gy521/mpu6050
	i2c_init();
	whoami = get_mpu6050_id();
	mpu_init();

	// read accel values from gy521
	if (whoami == 0x68) {
		while(1) {
			gy521_read_accel();
			x = (gy521_accel_data[0] << 8 | gy521_accel_data[1]);
			y = (gy521_accel_data[2] << 8 | gy521_accel_data[3]);
			z = (gy521_accel_data[4] << 8 | gy521_accel_data[5]);

			xg = (float) x / MPU6050_ACCEL_SCALE;
			yg = (float) y / MPU6050_ACCEL_SCALE;
			zg = (float) z / MPU6050_ACCEL_SCALE;


//			gy_x = (int16_t)(gy521_gyro_data[0] << 8 | gy521_gyro_data[1]);
//			gy_y = (int16_t)(gy521_gyro_data[2] << 8 | gy521_gyro_data[3]);
//			gy_z = (int16_t)(gy521_gyro_data[4] << 8 | gy521_gyro_data[5]);
//
//			Gy_x = (float) gy_x / MPU6050_GYRO_SCALE;
//			Gy_y = (float) gy_y / MPU6050_GYRO_SCALE;
//			Gy_z = (float) gy_z / MPU6050_GYRO_SCALE;

		}
	}



	// use i2c to read accelerometer
	// adxl_init();
//	while(1) {
//		// read_values fills 6 byte buffer
//		adxl_read_values(DATA_START);
//		x = ((data_recv[1]<<8) | (data_recv[0]));
//		y = ((data_recv[3]<<8) | (data_recv[2]));
//		z = ((data_recv[5]<<8) | (data_recv[4]));
//
//		// apply scale factor
//		xg = (x * SCALE_FACTOR);
//		yg = (y * SCALE_FACTOR);
//		zg = (z * SCALE_FACTOR);
//	}
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

static void uart2_callback(void) {
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
		uart2_callback();
	}
}

static void uart1_callback(void) {
	key = USART1->DR;
	// turn on LED with the input 1
	if (key == '1') {
		GPIOA->ODR |= PIN5;
	}
	else {
		GPIOA->ODR &= ~(PIN5);
	}
}



void USART1_IRQHandler(void) {
	// check the uart status register for not empty
	if (USART1->SR & SR_RXNE) {
		uart1_callback();
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

static void uart_dma_callback(void) {
	GPIOA->ODR |= PIN5;
}

void DMA1_Stream6_IRQHandler(void) {
	// check transfer control flag in HISR and clear
	if (DMA1->HISR & HISR_TCIF6) {
		DMA1->HIFCR |= HIFCR_CTCIF6; // write 1 to clear flag

		// do callback
		uart_dma_callback();
	}
}

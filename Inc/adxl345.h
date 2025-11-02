/*
 * adxl345.h
 *
 *  Created on: Jul 23, 2025
 *      Author: benro
 */

#ifndef ADXL345_H_
#define ADXL345_H_

#include "i2c.h"
#include "spi.h"
#include <stdint.h>
// #include "stm32f4xx.h"

#define DEVICE_ID_R		0x00
#define DEVICE_ADDR		0x53
#define PWR_CTRL		0x2D
#define DATA_FMT_R		0x31
#define DATA_START		0x32

// 4g threshold bits 1,0:01
#define RANGE_4G		(1U<<0)
#define RESET			0x00
#define SET_MEASURE		(1U<<3) // technically equals 0b1000=0x08


// ensure i2c_init(0 is ran to configure AF, default speed, etc
void adxl_i2c_init(void);
extern uint8_t adxl345_i2c_data_recv[6];
extern uint8_t mpu6050_i2c_data_recv[6];

// read XYZ data registers 0x31-0x37
void adxl_i2c_read_values(uint8_t reg);

// spi implementation
extern uint8_t adxl345_spi_data_recv[6]; // expose this as an extern buffer for main to store data in
void adxl_init_spi(void);
void adxl_spi_read(uint8_t reg, uint8_t *rxdata);
void adxl_spi_write(uint8_t reg, uint8_t value);


#endif /* ADXL345_H_ */

/*
 * adxl345.h
 *
 *  Created on: Jul 23, 2025
 *      Author: benro
 */

#ifndef ADXL345_H_
#define ADXL345_H_

#include "i2c.h"
#include <stdint.h>


#define DEVICE_ID_R		0x00
#define DEVICE_ADDR		0x53
#define PWR_CTRL		0x2D
#define DATA_FMT_R		0x31
#define DATA_START		0x32

// 4g threshold bits 1,0:01
#define RANGE_4G		(1U<<0)
#define RESET			0x00
#define SET_MEASURE		(1U<<3) // technically equals 0b1000=0x08



void adxl_init(void);

// read XYZ data registers 0x31-0x37
void adxl_read_values(uint8_t reg);

#endif /* ADXL345_H_ */

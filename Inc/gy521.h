/*
 * gy521.h
 *
 *  Created on: Sep 24, 2025
 *      Author: benro
 */

#ifndef GY521_H_
#define GY521_H_

uint8_t get_mpu6050_id();
void gy521_read_accel(void);

extern uint8_t gy521_accel_data[6];
extern uint8_t gy521_gyro_data[6];
void mpu_init(void);



#endif /* GY521_H_ */

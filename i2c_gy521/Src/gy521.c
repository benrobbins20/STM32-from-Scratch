#include "stm32f4xx.h"
#include "i2c.h"

#define MPU6050_SLAVE_ADDR		(0x68)
#define WHOAMI_REG				(0x75)

uint8_t id_check;

uint8_t get_mpu6050_id() {
	return i2c1_getbyte(MPU6050_SLAVE_ADDR, WHOAMI_REG);
}



#include "stm32f4xx.h"
#include "i2c.h"
#include "gy521.h"

#define MPU6050_SLAVE_ADDR		(0x68)
#define WHOAMI_REG				(0x75)
#define PWR_MGMT_REG			(0x6B)
#define SMPRT_DIV_REG			(0x19)
#define SMPRT_DIV				(0x07)
#define GYRO_CONFIG_REG			(0x1B)
#define ACCEL_CONFIG_REG		(0x1C)
#define ACCEL_START_ADDR		(0x3B)

uint8_t id_check;

// array of 6 bytes for accel data
uint8_t gy521_data_recv[6];


uint8_t get_mpu6050_id() {
	return i2c1_getbyte(MPU6050_SLAVE_ADDR, WHOAMI_REG);
}

void mpu_init(void) {
	wake_mpu6050();
	write_sample_rate();
	set_gyro_accel_range();
}

// private stuff
void wake_mpu6050(void) {
	uint8_t data[1];
	data[0] == 0x00;
	// send 1 byte of info 0x00 to slave
	i2c1_burstwrite(MPU6050_SLAVE_ADDR, PWR_MGMT_REG, 1, data);
}
void write_sample_rate(void) {
	uint8_t data[1];
	data[0] = SMPRT_DIV;

	i2c1_burstwrite(MPU6050_SLAVE_ADDR, SMPRT_DIV_REG, 1, data);
}
void set_gyro_accel_range(void) {
	uint8_t data[1];
	data[0] == 0x00;

	i2c1_burstwrite(MPU6050_SLAVE_ADDR, GYRO_CONFIG_REG, 1, data); // +- 2g range
	i2c1_burstwrite(MPU6050_SLAVE_ADDR, ACCEL_CONFIG_REG, 1, data); // +- 250 deg/s. (tilt slowly)
}

// read the accel data into array that main can access (rather than writing to a pointer to array in main)
void gy521_read_accel(void) {
	// pass a byte pointer to burst read and then 'incrementing, increments a byte and reads into DR
	i2c1_burstread(MPU6050_SLAVE_ADDR, ACCEL_START_ADDR, 6, (uint8_t*)gy521_data_recv);
}







#include "adxl345.h"

// char pointer for single
char id_byte;

// buffer for data
uint8_t data_recv[6];

void adxl_read_register(uint8_t reg) {

	// read the address register at 0x53 in memory
	i2c1_readbyte(DEVICE_ADDR, reg, &id_byte);
}

void adxl_write(uint8_t reg, char value) {
	char data[1];
	data[0] = value;

	// just "burst write" 1 byte
	i2c1_burstwrite(DEVICE_ADDR, reg, 1, data);

}

// read XYZ data registers 0x31-0x37
void adxl_read_values(uint8_t reg) {

	// burst read 6 data registers
	i2c1_burstread(DEVICE_ADDR, reg, 6, (char *)data_recv);
}


void adxl_init(void) {

	// run i2c init function
	i2c_init();
	// verify device id 0xE5 for adxl
	adxl_read_register(DEVICE_ID_R);

	// write +- 4g (01) to data format register (1,0), write byte 0x01
	adxl_write(DATA_FMT_R, RANGE_4G);

	// send reset to power control register
	adxl_write(PWR_CTRL, RESET);

	// set power control measure bit
	adxl_write(PWR_CTRL, SET_MEASURE);
}

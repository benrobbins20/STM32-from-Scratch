#include "adxl345.h"

// char pointer for single
char id_byte;

#define SPI_MULTIBYTE_EN	0x40
#define READ_OPERATION		0x80

// buffer for data
uint8_t adxl345_data_recv[6];

// I2C
void adxl_i2c_read_register(uint8_t reg) {

	// read the address register at 0x53 in memory
	i2c1_readbyte(DEVICE_ADDR, reg, &id_byte);
}

void adxl_i2c_write(uint8_t reg, char value) {
	char data[1];
	data[0] = value;

	// just "burst write" 1 byte
	i2c1_burstwrite(DEVICE_ADDR, reg, 1, data);

}

// read XYZ data registers 0x31-0x37
void adxl_i2c_read_values(uint8_t reg) {

	// burst read 6 data registers
	i2c1_burstread(DEVICE_ADDR, reg, 6, (char *)adxl345_data_recv);
}

void adxl_i2c_init(void) {

	// verify device id 0xE5 for adxl
	adxl_i2c_read_register(DEVICE_ID_R);

	// write +- 4g (01) to data format register (1,0), write byte 0x01
	adxl_i2c_write(DATA_FMT_R, RANGE_4G);

	// send reset to power control register
	adxl_i2c_write(PWR_CTRL, RESET);

	// set power control measure bit
	adxl_i2c_write(PWR_CTRL, SET_MEASURE);
}

// SPI
void adxl_spi_write(uint8_t reg, uint8_t value) {
	// pack data into an array with the address | multi-byte enable
	uint8_t data[2];

	data[0] = reg | SPI_MULTIBYTE_EN;
	data[1] = value;

	// pull slave line low
	cs_enable();

	spi1_transmit(data, 2);
	cs_disable();
}

// read data into receive buffer
void adxl_spi_read(uint8_t reg, uint8_t *rxdata) {

	reg |= READ_OPERATION;

	reg |= SPI_MULTIBYTE_EN;

	cs_enable();
	spi1_transmit(&reg, 1);

	spi1_receive(rxdata, 6);
	cs_disable();
}

void adxl_init_spi(void) {
	init_spi1_gpio();
	configure_spi();

	// write +- 4g (01) to data format register (1,0), write byte 0x01
	adxl_i2c_write(DATA_FMT_R, RANGE_4G);

	// send reset to power control register
	adxl_i2c_write(PWR_CTRL, RESET);

	// set power control measure bit
	adxl_i2c_write(PWR_CTRL, SET_MEASURE);
}





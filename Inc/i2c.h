#ifndef I2C_H_
#define I2C_H_

#include <stdint.h>

void i2c_init(void);
void i2c1_readbyte(char saddr, char maddr, char* data);
uint8_t i2c1_getbyte(uint8_t saddr, uint8_t maddr);
void i2c1_burstread(char saddr, char maddr, int n, char* data);
void i2c1_burstwrite(char saddr, char maddr, int n, char* data);




#endif /* I2C_H_ */

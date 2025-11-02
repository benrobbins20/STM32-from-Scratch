This repo is a showcase demonstrating Bare-Metal C implementation for basic STM32 functionality. 

Based off the Udemy class by Israel Gbati, [Bare-Metal From The Ground Up](https://www.udemy.com/course/embedded-systems-bare-metal-programming/) is a class that explain how to read the datasheet, user manual, and programming reference manual for an STM32. The particular flavor of board used for the class is a 32bit Nucleo F411RE. This board features clock speeds up to 100MHz, Arudino Uno compatible headers for expansino shields, and most important for a microcontroller novice, built-in ST-Link for flashing and debugging!

This class shows pracical examples of how to access registers to enable custom functionality on the board to turn on LED's, read digital and analog inputs, and implement control for many more features.

## GPIO

## UART

## Interrupts

## DMA
## I2C
### I2C was a little easier to read and decode communication signals because it is only a simple 2 wire protocol with a using a clock signal to pulse the databits for send and receive. The following scope captures show the basic configuration needed to begin reading accelerometer data from the ADXL345 sensor.
#### Read the device ID register by OR'ing the read bit with with the device address $$ 0x53 << 1 | 1 $$
<img width="696" height="240" alt="image" src="https://github.com/user-attachments/assets/92d62b95-804b-465d-b2e8-e412ad84985f" />

## SPI
### As part of writing and understanding bare-metal code, I really wanted to visualize the actual hardware signals. I FREAKING LOVE SCOPES. For SPI was able to capture some traces using 2 channel SDIO mode and just read MOSI and CLK initally to see what I'm actually configuring. I send 3 packets to configure the accelerometer range, reset signal to the power controller, and the set measure enable bit. Using my scope, I capture the signals being sent using the CLK line as a trigger source

<img width="826" height="516" alt="image" src="https://github.com/user-attachments/assets/5ad8e811-4f7f-4ae0-916e-2780a18f2634" />
<img width="872" height="483" alt="image" src="https://github.com/user-attachments/assets/cca97b35-c18e-4fbd-b7d4-2025bd0fdbd9" />
<img width="867" height="515" alt="image" src="https://github.com/user-attachments/assets/2e9d3db7-9bfa-4b04-aa91-246466499f27" />




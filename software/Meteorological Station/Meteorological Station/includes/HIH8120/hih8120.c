/*
 * hih8120.c
 *
 * Created: 08/07/2019 12:58:53
 *  Author: daniels
 */ 
#define F_CPU 8000000UL
#include <util/delay.h>
#include <stdio.h>
#include "../I2C/i2cmaster.h"

float hih8120_humidity = 1;
float hih8120_temperature_C = 1;

void hih8120_measure()
{
	i2c_start_wait(0x27 << 1);
	i2c_write((uint8_t) 0x01);
	i2c_stop();
	
	_delay_ms(100);
	
	i2c_start_wait((0x27 << 1) | I2C_READ);
	
	uint8_t humidity_hi = i2c_readAck();
	uint8_t humidity_lo = i2c_readAck();
	uint8_t temp_hi = i2c_readAck();
	uint8_t temp_lo = i2c_readNak();
	
	i2c_stop();
	
	// Calculate Relative Humidity
	hih8120_humidity = (float)((((humidity_hi & 0x3f) << 8) | humidity_lo) / ((pow(2,14) - 2)) * 100);

	// Calculate Temperature
	hih8120_temperature_C = (float) ((((temp_hi << 6) + (temp_lo >> 2)) / (pow(2, 14) - 2)) * 165 - 40);
}

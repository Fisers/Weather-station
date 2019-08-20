/*
 * hih8120.c
 *
 * Created: 08/07/2019 12:58:53
 *  Author: daniels
 */ 
#define F_CPU 16000000UL
#include <util/delay.h>
#include <stdio.h>
#include "../I2C/i2cmaster.h"
#include "../../Options.h"
#include "../SPI/SPI.h"

float hih8120_humidity = 1;
float hih8120_temperature_C = 1;

void hih8120_measure()
{
	PORT_CS &= ~(1<<HIH_CS);
	uint8_t humidity_hi = spi_rwSPI(0x00);
	uint8_t humidity_lo = spi_rwSPI(0x00);
	uint8_t temp_hi = spi_rwSPI(0x00);
	uint8_t temp_lo = spi_rwSPI(0x00);
	PORT_CS |= (1<<HIH_CS);
	PORT_CS &= ~(1<<HIH_CS);
	spi_rwSPI(0x00);
	PORT_CS |= (1<<HIH_CS);

	
 	// Calculate Relative Humidity
 	hih8120_humidity = (float)((((humidity_hi & 0x3f) << 8) | humidity_lo) / ((pow(2,14) - 2)) * 100);
 
 	// Calculate Temperature
 	hih8120_temperature_C = (float) ((((temp_hi << 6) + (temp_lo >> 2)) / (pow(2, 14) - 2)) * 165 - 40);
	 
	 
}

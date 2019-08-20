/*
 * BMP280_SPI.c
 *
 * Created: 05/07/2019 11:34:54
 * Author : daniels
 */ 

#include <avr/io.h>
#include "BMP280_SPI.h"
#include "../SPI/SPI.h"
#include "../../Options.h"

static void readCalibration() {
	_bmp280_calib.dig_T1 = spi_read16(0x88, BMP_CS);
	_bmp280_calib.dig_T2 = spi_readS16(0x8A, BMP_CS);
	_bmp280_calib.dig_T3 = spi_readS16(0x8C, BMP_CS);

	_bmp280_calib.dig_P1 = spi_read16(0x8E, BMP_CS);
	_bmp280_calib.dig_P2 = spi_readS16(0x90, BMP_CS);
	_bmp280_calib.dig_P3 = spi_readS16(0x92, BMP_CS);
	_bmp280_calib.dig_P4 = spi_readS16(0x94, BMP_CS);
	_bmp280_calib.dig_P5 = spi_readS16(0x96, BMP_CS);
	_bmp280_calib.dig_P6 = spi_readS16(0x98, BMP_CS);
	_bmp280_calib.dig_P7 = spi_readS16(0x9A, BMP_CS);
	_bmp280_calib.dig_P8 = spi_readS16(0x9C, BMP_CS);
	_bmp280_calib.dig_P9 = spi_readS16(0x9E, BMP_CS);
}

static float bmp280_compensate_T_int32(int32_t adc_T)
{
	int32_t var1, var2;
	var1 = ((((adc_T>>3) - ((int32_t)_bmp280_calib.dig_T1<<1))) * ((int32_t)_bmp280_calib.dig_T2)) >> 11;
	var2 = (((((adc_T>>4) - ((int32_t)_bmp280_calib.dig_T1)) * ((adc_T>>4) - ((int32_t)_bmp280_calib.dig_T1))) >> 12) * ((int32_t)_bmp280_calib.dig_T3)) >> 14;
	t_fine = var1 + var2;
	float T = (t_fine * 5 + 128) >> 8;
	return T / 100;
}

static float bmp280_compensate_pressure(int32_t adc_P)
{
	int64_t var1, var2, p;

	var1 = ((int64_t)t_fine) - 128000;
	var2 = var1 * var1 * (int64_t)_bmp280_calib.dig_P6;
	var2 = var2 + ((var1 * (int64_t)_bmp280_calib.dig_P5) << 17);
	var2 = var2 + (((int64_t)_bmp280_calib.dig_P4) << 35);
	var1 = ((var1 * var1 * (int64_t)_bmp280_calib.dig_P3) >> 8) +
	((var1 * (int64_t)_bmp280_calib.dig_P2) << 12);
	var1 =
	(((((int64_t)1) << 47) + var1)) * ((int64_t)_bmp280_calib.dig_P1) >> 33;

	if (var1 == 0) {
		return 0; // avoid exception caused by division by zero
	}
	p = 1048576 - adc_P;
	p = (((p << 31) - var2) * 3125) / var1;
	var1 = (((int64_t)_bmp280_calib.dig_P9) * (p >> 13) * (p >> 13)) >> 25;
	var2 = (((int64_t)_bmp280_calib.dig_P8) * p) >> 19;

	p = ((p + var1 + var2) >> 8) + (((int64_t)_bmp280_calib.dig_P7) << 4);
	return (float)p / 256;
}

void bmp280_init() {
	DDRB |= (1 << BMP_CS);
	PORT_CS &= ~(1<<BMP_CS); // Power On the module
	spi_rwSPI(0b01110100);
	spi_rwSPI(0xFF);
	
	spi_rwSPI(0xF4);
	spi_rwSPI(0b01010111);
	PORT_CS |= (1<<BMP_CS);
	
	readCalibration();
}

float bmp280_readPressure()
{
	uint8_t data[6]; // Array to hold temp/pressure values (pressure in indices 0:2, and temp in indices 3:5)
	uint32_t pressureUncomp, tempUncomp;
	
	spi_BurstRead(0xF7, data, 6, BMP_CS);
	tempUncomp = ((uint32_t)data[3]<<12) | ((uint32_t)data[4] << 4) | data[5]; // 20 bit temp val
	pressureUncomp = ((uint32_t)data[0]<<12) | ((uint32_t)data[1] << 4) | data[2]; // 20 bit pressure value
	bmp280_compensate_T_int32(tempUncomp);
	return bmp280_compensate_pressure(pressureUncomp);
}


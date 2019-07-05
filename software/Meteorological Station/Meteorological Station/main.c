#define F_CPU 8000000UL // Defining the CPU Frequency for Delay Calculation in delay.h
#define BAUD 9600

#include <stdlib.h>
#include <avr/io.h> // Contains all the I/O Register Macros
#include <avr/eeprom.h>
#include <util/delay.h> // Generates a Blocking Delay
#include <stdio.h>
#include "main.h"
#include "includes/SPI/SPI.h"
#include "includes/BMP280_SPI/BMP280_SPI.h"

#define BMP_CS PINB2

void uart_print(char *name, float val)
{
	char debug_buffer[10];

	printf(name);
	printf(" = ");

	dtostrf(val,1,2,debug_buffer);
	printf(debug_buffer);
	puts("\n");
}

void setCSPins()
{
	DDRB = 0xFF;
}

void initADC()
{
	ADCSRA |= (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
	ADMUX |= (1 << REFS0);
}

uint16_t adc_read()
{
	ADCSRA |= (1<<ADSC);
	while(ADCSRA & (1<<ADSC));
	
	return (ADC);
}

int16_t LargestDirectionValue;
void calibration()
{
	printf("Calibration started!");
	while(adc_read()+50 >= LargestDirectionValue)
	{
		if(adc_read() > LargestDirectionValue)
		{
			LargestDirectionValue = adc_read();
		}
		_delay_us(250);
	}
	eeprom_write_float(0, LargestDirectionValue);
	printf("Calibration complete!");
}

int main(void)
{	
	uart_init();
	stdout = &uart_output;
	puts("Hello");
	initADC();
	
	// calibration
	DDRD = (0 << PIND7);
	PORTD = (1 << PIND7);
	if((PIND&(1 << PIND7)) == 0)
	{
		calibration();
	}
	else {
		LargestDirectionValue = eeprom_read_float(0);
	}
	//
	
	setCSPins();
	spi_init_master();
	bmp280_init(BMP_CS);

	while(1) {
		float pressure = bmp280_readPressure();
		uart_print("Pressure", pressure);
		
		float angle = ((float)adc_read() / (float)LargestDirectionValue) * 360.0;
		uart_print("Wind Angle", angle);
		
		
		_delay_ms(1000);
	}
	
	return 0;
}






#define F_CPU 16000000UL // Defining the CPU Frequency for Delay Calculation in delay.h
#define BAUD 9600

#include <stdlib.h>
#include <avr/io.h> // Contains all the I/O Register Macros
#include <avr/eeprom.h>
#include <avr/pgmspace.h>
#include <util/delay.h> // Generates a Blocking Delay
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "main.h"

// FatFs
FATFS FatFs;
FIL Fil;

UINT bw;
//
unsigned int timeout;
rtc_t rtc;

static void initADC()
{
	ADCSRA |= (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
	ADMUX |= (1 << REFS0) | (1 << MUX1);
}

static uint16_t adc_read()
{
	ADCSRA |= (1<<ADSC);
	while(ADCSRA & (1<<ADSC));
	
	return (ADC);
}

int16_t LargestDirectionValue;
static void calibration()
{
	printf_P(PSTR("Calibration started!\n"));
	while(adc_read()+50 >= LargestDirectionValue)
	{
		if(adc_read() > LargestDirectionValue)
		{
			LargestDirectionValue = adc_read();
		}
		_delay_us(250);
	}
	eeprom_write_float(0, LargestDirectionValue);
	printf_P(PSTR("Calibration complete!\n"));
}

static void stringToIntArray(uint8_t * array, char * string, uint8_t * eepromAddr)
{
	uint8_t i = 0;
	char * p = string;
	while(*p) {
		if ( isdigit(*p) || ( (*p=='-'||*p=='+') && isdigit(*(p+1)) )) {
			array[i] = strtol(p, &p, 10);
			eeprom_write_byte((uint8_t*)eepromAddr+i, array[i]);
			i++;
			} else {
			p++;
		}
	}
}

static void getInput(char *input)
{
	char c;
	uint8_t i;
	for(i = 0; i < 16; i++) {
		c = getchar();
		if (c == '\n') break;
		input[i] = c;
	}
	input[i] = '\0';
}

static void ipconfig()
{
	static char input[16];
	

	puts("Input arduino IP");
	getInput(input);
	printf("Arduino IP: %s\n", input);
	stringToIntArray(myip, input, (uint8_t*)0x20);
	
	puts("Input server IP");
	getInput(input);
	printf("Server IP: %s\n", input);
	sprintf(serverip, "%s", input);
	eeprom_write_block((const void*)input, (void*)0x30, 16);
	
	puts("Input gateway IP");
	getInput(input);
	printf("Gateway IP: %s\n", input);
	stringToIntArray(gwip, input, (uint8_t*)0x40);
}

char packetStr[128];
uint8_t runOnce = 0;
int main(void)
{	
 	
	 
	uart_init();
	stdout = &uart_output;
	stdin  = &uart_input;
	puts_P(PSTR("Initializing"));

	i2c_init();
	spi_init_master();
	bmp280_init();
	
	wind_initWindSpeed();
	
	
	initADC();
	// Read IP values from eeprom
	for(uint8_t i = 0; i<4; i++)
	{
		myip[i] = eeprom_read_byte((uint8_t*)0x20+i);
		gwip[i] = eeprom_read_byte((uint8_t*)0x40+i);
	}
	eeprom_read_block((void*)serverip, (const void*)0x30, 16);
	//
	
	// calibration
	DDRD |= (0 << PIND5);
	PORTD |= (1 << PIND5);
	if((PIND&(1 << PIND5)) == 0)
	{
		calibration();
	}
	else {
		LargestDirectionValue = eeprom_read_float(0);
	}
	DDRD |= (0 << PIND6);
	PORTD |= (1 << PIND6);
	if((PIND&(1 << PIND6)) == 0)
	{
		ipconfig();
	}
	//
	timeout = eeprom_read_word((uint16_t*)0x10);
	DDRB |= (1 << SD_CS);
	f_mount(&FatFs, "", 0);
	Ether_init();

	while(1) {	
		float wind_speed;
		if(sendingPacket == 1)
		{
			Ether_SendPacket(packetStr);
			continue;
		}	
		if(runOnce == 0) {
			flag =0;
			TIFR1=(1<<ICF1);
			runOnce = 1;
			sec = 0;
		}
		if(flag < 2 && sec < 1) {
			continue;
		}

		_delay_ms(1000);
		runOnce = 0;
		wind_speed = 0.765 * wind_measureFrequency() + 0.35;
		if(wind_speed < 0 || wind_speed > 1000)
			wind_speed = 0;

		RTC_GetDateTime(&rtc);
		
		float angle = ((float)adc_read() / (float)LargestDirectionValue) * 360.0;
		if(angle > 360)
			angle = 360;
		
		hih8120_measure();
		
		char humidChar[10],tempChar[10],pressChar[10],anglChar[10],speedChar[10];
		dtostrf(hih8120_humidity,1,2,humidChar);
		dtostrf(hih8120_temperature_C,1,2,tempChar);
		dtostrf(bmp280_readPressure(),1,2,pressChar);
		dtostrf(angle,1,2,anglChar);
		dtostrf(wind_speed,1,2,speedChar);
		sprintf(packetStr, "[%02d:%02d:%02d] %s C , %s %% , %s Pa , %s , %s m/s", rtc.hour, rtc.min, rtc.sec,tempChar, humidChar, pressChar, anglChar, speedChar);
		
		char fileName[16];
		sprintf(fileName, "%02d-%02d-%02d.CSV", rtc.date, rtc.month, rtc.year);

		if(f_open(&Fil, fileName, FA_WRITE) == FR_OK)
		{
			f_lseek(&Fil, Fil.fsize);
			char sdData[snprintf(NULL,0,"%02d:%02d:%02d,%s,%s,%s,%s,%s\r\n", rtc.hour, rtc.min, rtc.sec, pressChar, anglChar, speedChar, humidChar, tempChar)];
			sprintf(sdData, "%02d:%02d:%02d,%s,%s,%s,%s,%s\r\n", rtc.hour, rtc.min, rtc.sec, pressChar, anglChar, speedChar, humidChar, tempChar);
			f_write(&Fil, sdData, sizeof(sdData), &bw);	// Write Data to the file
			f_close(&Fil);
		}
		else
		{
			if (f_open(&Fil, fileName, FA_WRITE | FA_CREATE_ALWAYS) == FR_OK) {	/* Create a file */
				f_write(&Fil, "Time,Pressure,Wind Angle,Wind Speed,Humidity,Temperature\r\n", 58, &bw);	// Create CSV header
				f_close(&Fil);								/* Close the file */
			}
		}

 		sec = 0;
 		Ether_SendPacket(packetStr);
	}
	
	return 0;
}






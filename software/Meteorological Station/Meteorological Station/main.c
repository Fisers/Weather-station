#define F_CPU 16000000UL // Defining the CPU Frequency for Delay Calculation in delay.h
#define BAUD 9600

#include <stdlib.h>
#include <avr/io.h> // Contains all the I/O Register Macros
#include <avr/eeprom.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <util/delay.h> // Generates a Blocking Delay
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "main.h"
#include <avr/wdt.h>
#include "Options.h"
#include "includes/SpaceInvaders/Space.h"



// Function Pototype
void wdt_init(void) __attribute__((naked)) __attribute__((section(".init3")));


// Function Implementation
void wdt_init(void)
{
	MCUSR = 0;
	wdt_disable();

	return;
}

// FatFs
FATFS FatFs;
FIL Fil;

UINT bw;
//
unsigned int magnetVal;
unsigned int halleff = 550;
unsigned int timeout;
rtc_t rtc;
int8_t gameMode = 0;

int brightness = 128;

static void initADC()
{
	ADCSRA |= (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
	ADMUX |= (1 << REFS0) | (1 << MUX1);
}

#define REF_AVCC (1<<REFS0)  // reference = AVCC = 3.3 V
uint16_t adc_read(uint8_t channel)
{
	ADMUX = REF_AVCC | channel;  // set reference and channel
	ADCSRA |= (1<<ADSC);         // start conversion
	while(ADCSRA & (1<<ADSC)); // wait for conversion complete
	return ADC;
}

int16_t LargestDirectionValue;
static void calibration()
{
	printf_P(PSTR("Calibration started!\n"));
	lcd_setXY(0x40,0x80);
	N5110_Data("Calibration");
 	lcd_setXY(0x41,0x80);
 	N5110_Data("Started");
	while(adc_read(0)+50 >= LargestDirectionValue)
	{
		wdt_reset();
		if(adc_read(0) > LargestDirectionValue)
		{
			PORTD |= (1 << PIND3);
			LargestDirectionValue = adc_read(0);
			dtostrf(LargestDirectionValue,1,2,pressChar);
			lcd_setXY(0x43,0x80);
			N5110_Data(pressChar);
		}
		_delay_us(250);
		PORTD &= ~(1 << PIND3);
	}
	eeprom_write_float(0x00, LargestDirectionValue);
	printf_P(PSTR("Calibration complete!\n"));
}

void stringToIntArray(uint8_t * array, char * string, uint8_t * eepromAddr)
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
	
	lcd_setXY(0x40,0x80);
	N5110_Data("Calibration");
	lcd_setXY(0x41,0x80);
	N5110_Data("Started");

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
char humidChar[10],tempChar[10],pressChar[16],anglChar[10],speedChar[10];
int page = 1;

ISR(PCINT2_vect)
{
	if((PINC&(1 << PINC4)) == 0)
	{
		if((PINC&(1 << PINC7)) == 0)
		{
			if(brightness + 25 > 255)
			{
				brightness = 255;
			}
			else
			{
				brightness += 25;
			}
		}
		else if((PINC&(1 << PINC6)) == 0)
		{
			if(brightness - 25 < 0)
			{
				brightness = 0;
			}
			else
			{
				brightness -= 25;
			}
		}
		else
		{
			page++;
			if(page >= 4) page = 1;
		
			printPage(page);
		}
	}
}

int main(void)
{	
	// init arrays
	//memset(bullets, 0, 32);
	//
 	DDRD = 0b10111100;
 	DDRA |= (1 << PINA1);
	PORTA |= (1 << PINA1);
	PORTD = 0b00111100;
	_delay_ms(500);
 	PORTD &= 0b11000111;
 	PORTA &= ~(1 << PINA1);
	PORTD &= ~(1 << PIND5);
	
	
	
 	

// 	uart_init();
// 	stdout = &uart_output;
// 	stdin  = &uart_input;
// 	puts_P(PSTR("Initializing"));
 	DDRC |= (0 << PINC6);
 	PORTC |= (1 << PINC6);
	DDRC |= (0 << PINC7);
	PORTC |= (1 << PINC7);
	DDRC |= (0 << PINC4);
	PORTC |= (1 << PINC4);
	
	 DDRD |= (1 << DDD7);
	 // PD6 is now an output

	 OCR2A = brightness;
	 // set PWM for 50% duty cycle


	 TCCR2A |= (1 << COM2A1);
	 // set none-inverting mode

	 TCCR2A |= (1 << WGM21) | (1 << WGM20);
	 // set fast PWM Mode

	 TCCR2B |= (1 << CS21);
	 // set prescaler to 8 and starts PWM
	 
	spi_init_master();
 	N5110_init();
	N5110_clear();
	
	DDRD |= (1 << PIND7);
	if((PINC&(1 << PINC4)) == 0 && (PINC&(1 << PINC6)) == 0) // Check for both calibration buttons pressed
	{
		SpaceStart();
	}
	
	wdt_reset();
	wdt_enable(WDTO_2S);
		
	DDRB |= (1 << SD_CS);
	PORT_CS |= (1<<HIH_CS);
	PORT_CS |= (1<<BMP_CS);
	PORT_CS |= (1<<SD_CS);
 	i2c_init();
 	bmp280_init();
	wind_initWindSpeed();
	initADC();
		
	// Read IP values from eeprom
		for(uint8_t i = 0; i<4; i++)
		{
			uint8_t temp;
			temp = eeprom_read_byte((uint8_t*)0x20+i);
			if(temp == 255)	break;
			myip[i] = eeprom_read_byte((uint8_t*)0x20+i);
			gwip[i] = eeprom_read_byte((uint8_t*)0x40+i);
		}
		eeprom_read_block((void*)serverip, (const void*)0x30, 16);
	//
	halleff = eeprom_read_word((const void*)0x60);

	// calibration
	wdt_reset();
	if((PINC&(1 << PINC6)) == 0)
	{
		calibration();
	}
	else {
		LargestDirectionValue = eeprom_read_float(0x00);
	}
	
		if((PINC&(1 << PINC7)) == 0)
		{
			wdt_disable();
			ipconfig();
			wdt_reset();
			wdt_enable(WDTO_2S);
		}
// 		//
 	timeout = eeprom_read_word((uint16_t*)0x10);
	wdt_reset();
	PORTD |= (1 << PIND5);
	PORT_CS &= ~(1<<SD_CS);
	f_mount(&FatFs, "", 0);
	PORT_CS |= (1<<SD_CS);
	PORTD &= ~(1 << PIND5);

	wdt_reset();
 	wdt_disable();
	PORTA |= (1 << PINA1);
	PORTB |= (1 << ETHER_CS);
	Ether_init();
	PORTA &= ~(1 << PINA1);
 	wdt_reset();
 	wdt_enable(WDTO_2S);
		
	DDRC |= (0 << PINC4);
	PORTC |= (1 << PINC4);
	 
	PCMSK2 |= (1 << PCINT20);
	PCICR |= (1 << PCIE2);
	 	
	sei();
	PORTD |= (1 << PIND3);
	while(1) {	
		float wind_speed;
		magnetVal = adc_read(2);

		if(sendingPacket == 1)
		{
			PORTA |= (1 << PINA1);
			Ether_SendPacket(packetStr);
			continue;
		}
		PORTA &= ~(1<<PINA1);
		wdt_reset();
		wdt_enable(WDTO_2S);
		PORTD |= (1 << PIND4);
		if(runOnce == 0) {
			flag =0;
			TIFR1=(1<<ICF1);
			runOnce = 1;
			sec = 0;
		}
		if(flag < 2 && sec < 2) {
			continue;
		}
		runOnce = 0;

		wdt_reset();
		//wind_speed = 0.765 * wind_measureFrequency() + 0.35;
		wind_speed = wind_measureFrequency();
		if(wind_speed < 0 || wind_speed > 1000)
			wind_speed = 0;
			
		if(magnetVal < halleff)
		{
			OCR2A = brightness;
			TCCR2A |= (1 << COM2A1);
			TCCR2A |= (1 << WGM21) | (1 << WGM20);
			TCCR2B |= (1 << CS21);
		}
		else
		{
			OCR2A = 255;
			TCCR2A |= (1 << COM2A1);
			TCCR2A |= (1 << WGM21) | (1 << WGM20);
			TCCR2B |= (1 << CS21);
		}
		
 		RTC_GetDateTime(&rtc);

		wdt_reset();
		
		float angle = ((float)adc_read(0) / (float)LargestDirectionValue) * 360.0;
		if(angle > 360)
			angle = 360;
		
		hih8120_measure();
		dtostrf(hih8120_humidity,1,2,humidChar);
		dtostrf(hih8120_temperature_C,1,2,tempChar);
		
		printf("%s\n", humidChar);
		printf("%s C\n", tempChar);
 		dtostrf(bmp280_readPressure(),1,2,pressChar);
		printf("%s Pa\n", pressChar);
		 
		
		printf("%s Pa", pressChar);
		dtostrf(angle,1,2,anglChar);
		dtostrf(wind_speed,1,2,speedChar);
		sprintf(packetStr, "%02d:%02d:%02d,%s,%s,%s,%s,%s", rtc.hour, rtc.min, rtc.sec,tempChar, humidChar, pressChar, anglChar, speedChar);
		printPage(page);
		
		wdt_reset();
		
		PORTD &= ~(1<<PIND4);
 		PORTD |= (1 << PIND5);
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
 		PORTD &= ~(1<<PIND5);
// 		_delay_ms(1000);
		
 		wdt_reset();
 		wdt_disable();
// 		
		PORTA |= (1 << PINA1);
		sec = 0;
		Ether_SendPacket(packetStr);
	}
	
	return 0;
}






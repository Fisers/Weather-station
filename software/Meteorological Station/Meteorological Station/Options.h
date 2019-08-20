/*
 * Options.h
 *
 * Created: 11/07/2019 10:34:09
 *  Author: daniels
 */ 


#ifndef OPTIONS_H_
#define OPTIONS_H_

#include "includes/RTC/rtc.h"

#define F_CPU 16000000UL // Defining the CPU Frequency for Delay Calculation in delay.h
#define BAUD 9600
#define PORT_CS PORTB
#define BMP_CS PINB3
#define ETHER_CS PINB2
#define SD_CS PINB1
#define HIH_CS PINB4

#define PORT_LCD PORTC
#define PORT_SPI PORTB
#define DDR_SPI DDRB
#define DDR_LCD DDRC

/*
 * LCD's pins
 */
#define LCD_SCE PC5
#define LCD_RST PC2
#define LCD_DC PC3
#define LCD_DIN PB5
#define LCD_CLK PB7

extern unsigned int magnetVal;
extern unsigned int halleff;
extern unsigned int timeout; // in seconds
extern rtc_t rtc;

extern uint8_t myip[4];
extern char serverip[16];
extern uint8_t gwip[4];


extern char humidChar[10],tempChar[10],pressChar[16],anglChar[10],speedChar[10];

void stringToIntArray(uint8_t * array, char * string, uint8_t * eepromAddr);



#endif /* OPTIONS_H_ */
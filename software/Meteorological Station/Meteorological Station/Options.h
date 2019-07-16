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
#define PORT_CS PORTD
#define BMP_CS PIND2
#define ETHER_CS PINB1
#define SD_CS PINB1

extern unsigned int timeout; // in seconds
extern rtc_t rtc;

extern uint8_t myip[4];
extern char serverip[16];
extern uint8_t gwip[4];



#endif /* OPTIONS_H_ */
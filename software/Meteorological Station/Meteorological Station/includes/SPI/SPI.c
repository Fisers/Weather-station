/*
 * SPI.c
 *
 * Created: 05/07/2019 11:18:37
 * Author : daniels
 */ 

#include <avr/io.h>
#include "SPI.h"
#include "../../Options.h"

#define SCK PINB7
#define MISO PINB6
#define MOSI PINB5


// SPI
// Initialize SPI Master Device (with SPI interrupt)
void spi_init_master (void)
{
// 	DDRB |= (1<<MOSI)|(1<<SCK);
// 	DDRB &= ~(1<<MISO);
// 	SPCR |= (1<<SPE)|(1<<MSTR)|(1<<SPR1)|(1<<SPR0);
// 	PORTB |= (1<<MISO);
// 	PORTB &= ~(1<<MISO);
	/*SET MOSI, SCK AND CS AS OUTPUT, REST OF REGISTER INPUT*/
	DDRB = (1<<DDB5) | (1<<DDB7) | (1<<DDB4);
	/*ENABLE SPI, MASTER, SET CLOCK RATE FCK/128 */
	SPCR = (1<<SPE)|(1<<MSTR)|(1<<SPR1)|(1<<SPR0);
}

/********************************************
SPI READ/WRITE
********************************************/

uint8_t spi_rwSPI( uint8_t byteword)
{
	SPDR = byteword; // put the byteword into data register
	while(!(SPSR & (1<<SPIF)));	// Wait for transmission complete
	return SPDR;
}

/********************************************
BURST READ
********************************************/

void spi_BurstRead(uint8_t addr, uint8_t buffer[], uint8_t countdown, int pin) {

	PORT_CS &= ~(1<<pin);
	spi_rwSPI(addr);
	for (int i = 0; i < countdown; i++) {
		*buffer++ = spi_rwSPI(0x00);
	}
	PORT_CS |= (1<<pin);
}

uint16_t spi_read16(uint8_t addr, int pin)
{
	PORT_CS &= ~(1<<pin);
	spi_rwSPI(addr);
	uint8_t buffer[2];
	buffer[0] = spi_rwSPI(0x00);
	buffer[1] = spi_rwSPI(0x00);
	uint16_t receivedVal = (buffer[1] << 8) | buffer[0];
	PORT_CS |= (1<<pin);
	return receivedVal;
}

int16_t spi_readS16(uint8_t addr, int pin)
{
	return (int16_t)spi_read16(addr, pin);
}


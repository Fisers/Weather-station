#ifndef SPI_H
#define SPI_H

extern void spi_init_master (void);
extern uint8_t spi_rwSPI( uint8_t byteword);
extern void spi_BurstRead(uint8_t addr, uint8_t buffer[], uint8_t countdown, int pin);
extern uint16_t spi_read16(uint8_t addr, int pin);
extern int16_t spi_readS16(uint8_t addr, int pin);

#endif
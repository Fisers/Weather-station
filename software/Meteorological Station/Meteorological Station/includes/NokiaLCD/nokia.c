/*
 * nokia.c
 *
 * Created: 25/07/2019 11:24:06
 *  Author: daniels
 */ 
#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <string.h>
#include "../SPI/SPI.h"
#include "Font.h"
#include "../../Options.h"
#include <avr/pgmspace.h>

void N5110_Cmnd(char DATA)
{
	PORT_LCD &= ~(1<<LCD_DC);				/* make DC pin to logic zero for command operation */
	PORT_LCD &= ~(1<<LCD_SCE);				/* enable SS pin to slave selection */
	spi_rwSPI(DATA);				/* send data on data register */
	PORT_LCD |= (1<<LCD_DC);				/* make DC pin to logic high for data operation */
	PORT_LCD |= (1<<LCD_SCE);	
}

void N5110_Data(char *DATA)
{
	PORT_LCD |= (1<<LCD_DC);									/* make DC pin to logic high for data operation */
	PORT_LCD &= ~(1<<LCD_SCE);									/* enable SS pin to slave selection */
	int lenan = strlen(DATA);							/* measure the length of data */
	for (int g=0; g<lenan; g++)
	{
		for (int index=0; index<5; index++)
		{
			spi_rwSPI(ASCII[DATA[g] - 0x20][index]);	/* send the data on data register */
		}
		spi_rwSPI(0x00);
	}
	PORT_LCD |= (1<<LCD_SCE);
}

void N5110_Reset()					/* reset the Display at the beginning of initialization */
{
	PORT_LCD &= ~(1<<LCD_RST);
	_delay_ms(100);
	PORT_LCD |= (1<<LCD_RST);
}

void N5110_init()
{
	DDR_LCD |= (1 << LCD_SCE);
	DDR_LCD |= (1 << LCD_RST);
	DDR_LCD |= (1 << LCD_DC);
	
 	_delay_ms(10);
 	N5110_Reset();					/* reset the display */
 	N5110_Cmnd(0x21);				/* command set in addition mode */
	N5110_Cmnd(0xC2);				/* set the voltage by sending C0 means VOP = 5V */
	N5110_Cmnd(0x06);				/* set the temp. coefficient to 3 */
	N5110_Cmnd(0x13);				/* set value of Voltage Bias System */
	N5110_Cmnd(0x20);				/* command set in basic mode */
	
	/* LCD in normal mode */
	N5110_Cmnd(0x09);

	/* Clear LCD RAM */
 	N5110_Cmnd(0x80);
	N5110_Cmnd(0x40);
	for (int i = 0; i < 504; i++)
		N5110_Cmnd(0x00);

	/* Activate LCD */
	N5110_Cmnd(0x08);
 	N5110_Cmnd(0x0C);				/* display result in normal mode */
}

void lcd_setXY(char x, char y)		/* set the column and row */
{
	N5110_Cmnd(x);
	N5110_Cmnd(y);
}

void N5110_clear()					/* clear the Display */
{
	PORT_LCD &= ~(1<<LCD_SCE);
	PORT_LCD |= (1<<LCD_DC);
	for (int k=0; k<=503; k++)
	{
		spi_rwSPI(0x00);
	}
	PORT_LCD &= ~(1<<LCD_DC);
	PORT_LCD |= (1<<LCD_SCE);
}

void N5110_image(const unsigned char *image_data, int8_t inverted)		/* clear the Display */
{
	PORT_LCD &= ~(1<<LCD_SCE);
	PORT_LCD |= (1<<LCD_DC);
	if(inverted == 1)
	{
		for (int k=0; k<=503; k++)
		{
			spi_rwSPI(pgm_read_byte(image_data + k));
		}
	}
	else
	{
		for (int k=0; k<=503; k++)
		{
			spi_rwSPI(image_data[k]);
		}
	}
	
	PORT_LCD &= ~(1<<LCD_DC);
	PORT_LCD |= (1<<LCD_SCE);
}

void printPage(int page)
{
	char charMyIP[16], charGWIP[16];
	N5110_clear();
	switch(page)
	{
		case (1):
			lcd_setXY(0x40,0x80);
			N5110_Data("Temperature:");
			lcd_setXY(0x41,0x80);
			N5110_Data(tempChar);
			N5110_Data(" C");
			lcd_setXY(0x42,0x80);
			N5110_Data("Humidity:");
			lcd_setXY(0x43,0x80);
			N5110_Data(humidChar);
			N5110_Data(" %");
			lcd_setXY(0x44,0x80);
			N5110_Data("Pressure:");
			lcd_setXY(0x45,0x80);
			N5110_Data(pressChar);
			N5110_Data(" Pa");
			break;
		case (2):
			lcd_setXY(0x40,0x80);
			N5110_Data("Wind Speed:");
			lcd_setXY(0x41,0x80);
			N5110_Data(speedChar);
			N5110_Data(" m/s");
			lcd_setXY(0x43,0x80);
			N5110_Data("Wind Angle:");
			lcd_setXY(0x44,0x80);
			N5110_Data(anglChar);
			break;
		case (3):
			lcd_setXY(0x40,0x80);
			N5110_Data("Server IP:");
			lcd_setXY(0x41,0x80);
			N5110_Data(serverip);
			lcd_setXY(0x42,0x80);
			N5110_Data("Station IP:");
			lcd_setXY(0x43,0x80);
			sprintf(charMyIP, "%d.%d.%d.%d", myip[0], myip[1], myip[2], myip[3]);
			N5110_Data(charMyIP);
			lcd_setXY(0x44,0x80);
			N5110_Data("Gateway IP:");
			lcd_setXY(0x45,0x80);
			sprintf(charGWIP, "%d.%d.%d.%d", gwip[0], gwip[1], gwip[2], gwip[3]);
			N5110_Data(charGWIP);
			break;
	}
	PORT_LCD |= (1<<LCD_SCE);
}
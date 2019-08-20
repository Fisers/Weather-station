/*
 * nokia.h
 *
 * Created: 25/07/2019 11:32:42
 *  Author: daniels
 */ 


#ifndef NOKIA_H_
#define NOKIA_H_

void N5110_Data(char *DATA);
void N5110_init();
void lcd_setXY(char x, char y);
void N5110_clear();
void N5110_image(const unsigned char *image_data, int8_t inverted);
void printPage(int page);


#endif /* NOKIA_H_ */
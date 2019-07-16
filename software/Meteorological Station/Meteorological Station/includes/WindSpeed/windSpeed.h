/*
 * windSpeed.h
 *
 * Created: 08/07/2019 09:11:58
 *  Author: daniels
 */ 


#ifndef WINDSPEED_H_
#define WINDSPEED_H_

extern volatile uint8_t flag;

float wind_frequency;

float wind_measureFrequency();
void wind_initWindSpeed();


#endif /* WINDSPEED_H_ */
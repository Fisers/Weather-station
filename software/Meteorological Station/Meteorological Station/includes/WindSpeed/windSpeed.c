#define F_CPU 16000000UL
#include <avr/io.h>
#include <avr/interrupt.h>
#include "windSpeed.h"


volatile unsigned int t_1=0;
volatile unsigned int t_2=0;
unsigned int period=0;

//capture Flag
volatile uint8_t flag;

static void timer1_init()
{
	
	// Starting timer 1 in normal mode
	TCCR1B= 0x00;
	TCCR1A= 0x00;
	// setting interrupt flag register to 0.
	TIFR1=0x00;
	// timer 1 setup with a pre-scalar of 256
	TCCR1B |=(1<<CS12);
	//Input capture on rising edge
	TCCR1B|=(1<<ICES1);
	
	// setting the timer/counter i/o locations to 0.
	TCNT1H=0x00;
	TCNT1L=0x00;
	// enabling input capture
	TIMSK1=(1<<ICIE1);
	// enabling global interrupt
	sei();
}

void restartTimer()
{
	TCNT1 = 0x00;
}


void wind_initWindSpeed()
{
	// initialize timer
	timer1_init();

	// setting PB0 as input for
	DDRD |= (0<<PORTD5);
	//Enable PB0 pull up resistor
	PORTD |= (1<<5);
}

float wind_measureFrequency()
{
	if (flag>=2)
	{
		flag =0;
		period= (t_2-t_1);
		TIFR1=(1<<ICF1);
		
		return (16000000UL/period)/256;
	}
	return -1;
}

ISR (TIMER1_CAPT_vect)
{
	if (flag==0)
	{
		t_1=ICR1;
		TIFR1=(1<<ICF1);

	}
	else if (flag==1)
	{
		t_2= ICR1;
		TIFR1=(1<<ICF1);
		
	}
	flag ++;
}
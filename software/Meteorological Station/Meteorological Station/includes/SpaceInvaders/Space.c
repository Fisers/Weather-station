/*
 * Space.c
 *
 * Created: 26/08/2019 09:07:04
 *  Author: daniels
 */ 
#include "../../Options.h"
#include <stdlib.h>
#include <avr/io.h> // Contains all the I/O Register Macros
#include <avr/eeprom.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <util/delay.h> // Generates a Blocking Delay
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "Graphics/invader.h"
#include "Level.h"
#include "Graphics/player.h"
#include "Graphics/bullet.h"
#include "../Ethernet/test_web_client.h"
#include "../NokiaLCD/nokia.h"
#include <avr/wdt.h>
#include "SplashScreen.h"


int8_t playerPos = 16, enemyPos = 0; // Max player positions - 8
int bullets, invaderBullet[5];
int enemy[21];
int cursorPos = 0, tempPos = 0;
uint16_t score = 0, highscore = 0;
static void drawEnemy()
{
	tempPos = cursorPos;
	for(int i=0; i < sizeof(invader); i++)
	{
		Level[i+tempPos] = invader[i];
		cursorPos = tempPos + i;
	}
	tempPos = cursorPos;
	for(int i=1; i <= sizeof(invader); i++)
	{
		Level[i+tempPos] = invader[sizeof(invader) - i];
		cursorPos = tempPos + i;
	}
	cursorPos += 3;
}

static void drawPlayer()
{
	tempPos = cursorPos;
	for(int i=0; i < sizeof(player); i++)
	{
		Level[i+tempPos] = player[i];
		cursorPos = tempPos + i;
	}
	tempPos = cursorPos;
	for(int i=1; i <= sizeof(player); i++)
	{
		Level[i+tempPos] = player[sizeof(player) - i];
		cursorPos = tempPos + i;
	}
	cursorPos += 3;
}

static void invaderShoot()
{
	for(int i=0; i < 5; i++)
	{
		if(invaderBullet[i] == 0)
		{
			cursorPos -= 7;
			Level[cursorPos] = bullet;
			invaderBullet[i] = cursorPos;
			cursorPos += 7;
			return;
		}
	}
}

static void shoot()
{
	if(bullets == 0)
	{
		cursorPos = 408 + (4 * playerPos) + 5;
		Level[cursorPos] = bullet;
		bullets = cursorPos;
		return;
	}
}

static void resetEnemy()
{
	for(int r=0; r < 3; r++)
	{
		for(int i=0; i < 7; i++)
		{
			enemy[(r*7)+i] = 0;
		}
	}
	enemyPos = 0;
	sec = 0;
}

static void gameover()
{
	N5110_clear();
	if(score > highscore || (highscore < 1 && highscore > 254))
	{
		highscore = score;
	}
	eeprom_write_word((uint16_t*)0x50, highscore);
	lcd_setXY(0x41,0x90);
	N5110_Data("GAME OVER");
	lcd_setXY(0x43,0x89);
	N5110_Data("High Score");
	lcd_setXY(0x44,0xA1);
	char scoreChar[10];
	itoa(highscore,scoreChar,10);
	N5110_Data(scoreChar);
	
	_delay_ms(5000);
	score = 0;
	resetEnemy();
}

int count = 0;
static void updateBullets()
{
	if(bullets != 0)
	{
		bullets -= 84;
		for(int e=21; e >= 0; e--)
		{
			if(enemy[e] == -1) continue;
			//printf("BulletPos %d  |  EnemyPos %d\n", bullets[i], enemy[e]);
			if(bullets > enemy[e] && bullets < enemy[e]+10)
			{
				score++;
				enemy[e] = -1;
				bullets = 0;
				break;
			}
		}
		
		if(bullets <= 2)
		{
			bullets = 0;
			return;
		}
		Level[bullets] = bullet;
		
	}
	if(count >= 5)
	{
		for(int i=0; i < 5; i++)
		{
			if(invaderBullet[i] != 0)
			{
				invaderBullet[i] += 84;
				if(invaderBullet[i] > 408 + (4 * playerPos) && invaderBullet[i] < 408 + (4 * playerPos) + 10)
				{
					gameover();
				}
				if(invaderBullet[i] >= 504)
				{
					invaderBullet[i] = 0;
					continue;
				}
			}
		}
		count = 0;
	}
	for(int i=0; i < 5; i++)
	{
		if(invaderBullet[i] != 0)
		{
			Level[invaderBullet[i]] = bullet;
		}
	}
	count++;
}

static volatile uint8_t watchdog_counter=0;
ISR(WDT_vect) {
	//WDTCSR |= (1<<WDIE);
	//wdt_reset();
	watchdog_counter++;
}

void prevent_wdt_reset() {
	if(MCUSR & (1<<WDRF)){            // If a reset was caused by the Watchdog Timer...
		MCUSR &= ~(1<<WDRF);                 // Clear the WDT reset flag
		WDTCSR |= (1<<WDCE) | (1<<WDE);   // Enable the WD Change Bit
		WDTCSR = 0x00;                      // Disable the WDT
	}
}

void SpaceStart()
{
	highscore = eeprom_read_word((uint16_t*)0x50);
	if(highscore <= 0 || highscore > 254)
		highscore = 0;
	
	init_cnt2();
	puts_P(PSTR("Space Invaders activated!"));
	N5110_image(SplashScreen, 1);
	_delay_ms(5000);
	sec = 0;
	prevent_wdt_reset();
	MCUSR = 0;                          // reset various flags
	WDTCSR |= 0b00011000;               // see docs, set WDCE, WDE
	WDTCSR =  0b01000000 | 0b100001;    // set WDIE, and appropriate delay
	sei();
	
	while(1)
	{
		WDTCSR |= (1<<WDIE);
		if(watchdog_counter == 3)
		{
			watchdog_counter = 0;
			software_reset();
		}
		lcd_setXY(0x40,0x80);
	
		//N5110_clear();
		for(int i=0; i < sizeof(Level); i++)
		{
			Level[i] = 0x00;
		}
		updateBullets();
		if((PINC&(1 << PINC7)) == 0)
		{
			wdt_reset();
			watchdog_counter = 0;
			shoot();
		}
		if((PINC&(1 << PINC6)) == 0)
		{
			wdt_reset();
			watchdog_counter = 0;
			if(408 + (4 * playerPos) < 492) playerPos++;
		}
		else if((PINC&(1 << PINC4)) == 0)
		{
			wdt_reset();
			watchdog_counter = 0;
			if(408 + (4 * playerPos) > 420) playerPos--;
		}
		int timer = 10 - (score / 10);
		if(sec >= timer)
		{
			sec = 0;
			enemyPos++;
		}
		cursorPos = enemyPos * 84;
		int full = 0;
		for(int r=0; r < 2; r++)
		{
			for(int i=0; i < 7; i++)
			{
				if(enemy[(r*7)+i] == -1)
				{
					cursorPos += 12;
					continue;
				}
				if(cursorPos > 408)
				{
					gameover();
				}
				full++;
				enemy[(r*7)+i] = cursorPos;
				drawEnemy();
				if(rand() % 50 < 1)
				{
					invaderShoot();
				}
			}
		}
		if(full == 0)
		{
			resetEnemy();
		}
		cursorPos = 408 + (4 * playerPos);
		drawPlayer();
	
	
		N5110_image(Level, 0);
		lcd_setXY(0x44,0x80);
		char scoreChar[10];
		itoa(score,scoreChar,10);
		N5110_Data(scoreChar);
	
		_delay_ms(100);
	}
}
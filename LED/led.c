/*
 *  led.c
 *
 *  The simple interrupt driven LED blink driver
 *
 *  Created on: 30 Aug 2017
 *  Version 0.0
 *
 *  Created by Piotr Jankowski
 *  http://www.diymat.co.uk
 *
 */


#include <stdint.h>

#include "stm32f1xx_hal.h"
#include "stm32f1xx.h"      //replace with your uC one
#include "led.h"


static LED_TypeDef LEDS[LEDNUM];
static uint16_t LED_Times[LEDNUM][5];  //[0] - on time, [0] off time, [1]&[2] - current counters
static uint8_t LED_type;


void OffLED(int led)
{
    if(LED_type & (1 << led))
    	LEDS[led].GPIO -> ODR &= ~(1 << LEDS[led].pin);
    else
    	LEDS[led].GPIO -> ODR |= (1 << LEDS[led].pin);
}

void OnLED(int led)
{
    if(!(LED_type & (1 << led)))
    	LEDS[led].GPIO -> ODR &= ~(1 << LEDS[led].pin);
    else
    	LEDS[led].GPIO -> ODR |= (1 << LEDS[led].pin);
}

void SetTimeLED(int led, uint16_t ontime, uint16_t offtime, uint16_t nblinks)
{
	GPIO_TypeDef *gpio = LEDS[led].GPIO;

	OffLED(led);
	LEDS[led].GPIO = NULL;
	LED_Times[led][ONTIME]= LED_Times[led][ONTIMECOUNTER] = ontime / LED_DIVIDER;
	LED_Times[led][OFFTIME]= LED_Times[led][OFFTIMECOUNTER] = offtime / LED_DIVIDER;
	LED_Times[led][NBLINKS] = nblinks ? (nblinks - 1) : UINT16_MAX;
	LEDS[led].GPIO = gpio;
	if(ontime) OnLED(led);
}


int AddLED(GPIO_TypeDef *gpio, uint8_t pin, int type, void (*callback)(int))  // type means what level switches the LED on
{
	int result = 0;

	(void)callback;

	for(int i = 0; i < LEDNUM; i++)
	{
		if(LEDS[i].GPIO == NULL)
		{
			for(int j = 0; j < 4; j++)
				LED_Times[i][j] = 0;
			LEDS[i].GPIO = gpio;
			LEDS[i].pin = pin;
			LED_type |= ((!!type) << i);
			OffLED(i);

			#if CALLBACKSUPPORT == 1
			LEDS[i].CallBack = callback;
			#endif

			result = i;
			break;
		}
	}
	return result;
}


static void _ToggleLED(GPIO_TypeDef *gpio, int pin)
{
	gpio -> ODR ^= (1 << pin);
}

void ToggleLED(int led)
{
	LEDS[led].GPIO -> ODR ^= (1 << LEDS[led].pin);
}


void LED_ISR_Callback(void)
{

#if LED_DIVIDER > 1
	if(HAL_GetTick() % LED_DIVIDER) return;
#endif

	for(int i = 0; i < LEDNUM; i++)
	{
		if(LEDS[i].GPIO != NULL && LED_Times[i][ONTIME])
		{
			if(!LED_Times[i][ONTIME]) continue;
			if((!LED_Times[i][ONTIMECOUNTER] && LED_Times[i][OFFTIMECOUNTER] == LED_Times[i][OFFTIME]) || (!LED_Times[i][OFFTIMECOUNTER] && !LED_Times[i][ONTIMECOUNTER]))
			{
				_ToggleLED(LEDS[i].GPIO, LEDS[i].pin);
				if(!LED_Times[i][OFFTIMECOUNTER] && !LED_Times[i][ONTIMECOUNTER])
				{
					if(!LED_Times[i][NBLINKS])
					{
						OffLED(i);
						#if CALLBACKSUPPORT == 1
						if(LEDS[i].CallBack != NULL) LEDS[i].CallBack(i);
						#endif
					}
					else
					{
						LED_Times[i][OFFTIMECOUNTER] = LED_Times[i][OFFTIME];
						LED_Times[i][ONTIMECOUNTER] = LED_Times[i][ONTIME];
						if(LED_Times[i][NBLINKS] != UINT16_MAX)
							LED_Times[i][NBLINKS]--;
					}
				}
			}
			if(LED_Times[i][ONTIMECOUNTER] && LED_Times[i][ONTIMECOUNTER] != UINT16_MAX)
				LED_Times[i][ONTIMECOUNTER]--;
			else
				LED_Times[i][OFFTIMECOUNTER]--;
		}
	}
}

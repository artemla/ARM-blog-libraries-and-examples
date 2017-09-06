/*
 * kbd.c
 *
 *  Created on: 6 Sep 2017
 *      Author: PJ
 */

#include <stdint.h>
#include "stm32f1xx.h"
#include "kbd.h"


volatile static BUTTON_Status_T Buttons[NKEYS];

int KBD_addKey(GPIO_TypeDef *gpio, int pin, int type)
{
	int result = -1;

	for(int i = 0; i < NKEYS; i ++)
	{
		if(Buttons[i].gpio == NULL)
		{
			Buttons[i].gpio = gpio;
			Buttons[i].pinmask = (1 << pin);
			Buttons[i].status_data = 0;
			Buttons[i].type = type;
			Buttons[i].debouncing = 0;
			Buttons[i].counter = DEBOUNCE;
			result = i;
			break;
		}
	}
	return result;
}

static inline int KBD_GetPIN(int key)
{
	int status = Buttons[key].gpio -> IDR & Buttons[key].pinmask;

	return Buttons[key].type ? !status : !! status;
}

int KBD_GetKey(int key)
{
	int result = Buttons[key].status;

	Buttons[key].status = NOKEY;
	return result;

}

static inline void _KBDSetKey(int key, int st)
{
	Buttons[key].status = st;
}

void KBD_SetKey(int key, int st)
{
	_KBDSetKey(key,st);
}

#ifdef USE_HAL_DRIVER
#define GetTick()     HAL_GetTick()
#else
static volatile unit32_t _kbd_ticks;

static inline uint32_t KBD_GetTick(void)
{
	return ++_kbd_ticks;
}
#endif


void KBD_ISR_Callback(void)
{
	int key;

	if((GetTick() & (__KBD_DIVIDER - 1))) return;										// every __KBD_DIVIDER interrupt served to save the processor time
 	for(int i = 0; i < NKEYS; i++)
	{
		if(Buttons[i].gpio == NULL) continue;											//look for the configured ketys
		if((key = KBD_GetPIN(i)) && !Buttons[i].status_data) continue;                  //wait for the key release after the reset
		if(key != Buttons[i].previous_key)
		{
			if(--Buttons[i].debounced && Buttons[i].counter) continue;					//debounce
			Buttons[i].debounced = 1;
			if(key) Buttons[i].wait_for_realease  = 1;
			Buttons[i].previous_key = key;
		}
		else
		{
			if(Buttons[i].debounced)
			{
				if(key)
				{
					if(++Buttons[i].counter >= LONGCLICK)
					{
						_KBDSetKey(i, LONGKEY);
						Buttons[i].debounced = 0;
						Buttons[i].wait_for_realease = 0;
						Buttons[i].counter = DEBOUNCE;
						continue;
					}

				}
				else
				{
					if(Buttons[i].debounced)
					{
						Buttons[i].debounced = 0;
						if(Buttons[i].wait_for_realease)
						{
							_KBDSetKey(key, KEY);
							Buttons[i].debounced = 0;
							Buttons[i].wait_for_realease = 0;
							Buttons[i].counter = DEBOUNCE;
							continue;
						}
					}
				}
			}
		}
	}
}

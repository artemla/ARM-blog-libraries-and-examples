/*
 *  kbd.h
 *
 *  The simple interrupt driven button driver
 *  Supports two events - keypressed & longkey
 *
 *  Created on: 5 Sep 2017
 *  Version 0.0
 *
 *  Created by Piotr Jankowski
 *  http://www.diymat.co.uk
 *
 */

#ifndef KBD_H_
#define KBD_H_

#include <stdint.h>

typedef enum
{
	NOKEY,
	KEY,
	LONGKEY,
	DOUBLECLICK,
	KEYERROR,
}KEYSTROKES_T;

typedef enum
{
	HI = 0,
	LO = 1,
}KEYTYPE_T;

typedef struct {
	GPIO_TypeDef  *gpio;
	union
	{
		uint8_t status_data;
		struct
		{
			uint8_t type				: 1;
			uint8_t pressed				: 1;
			uint8_t wait_for_realease	: 1;
			uint8_t ignore_release		: 1;
			uint8_t wait_for_DC			: 1;
			uint8_t previous_key		: 1;
			uint8_t debouncing			: 1;
			uint8_t debounced			: 1;
		};
	};
	uint8_t 	pinmask;
	uint16_t	counter;
	uint8_t		status;
}BUTTON_Status_T;

#define __KBD_DIVIDER		8
#define NKEYS	  			5
#define DEBOUNCE			(64 / __KBD_DIVIDER)
#define LONGCLICK			(2000 / __KBD_DIVIDER)
#define DC_GAP				(200 / __KBD_DIVIDER)

int KBD_addKey(GPIO_TypeDef *gpio, int pin, int type);
int KBD_GetKey(int key);
void KBD_SetKey(int key, int st);
void KBD_ISR_Callback(void);

#endif /* KBD_H_ */

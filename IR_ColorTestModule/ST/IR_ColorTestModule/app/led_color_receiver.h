
#ifndef LED_COLOR_RECEIVER_H
#define LED_COLOR_RECEIVER_H

#include "includes.h"

void LEDColor_Initialization(void);
uint8_t LEDColor_Read(uint16_t* c, uint16_t * red, uint16_t * green, uint16_t *blue, uint16_t * ir);

#endif





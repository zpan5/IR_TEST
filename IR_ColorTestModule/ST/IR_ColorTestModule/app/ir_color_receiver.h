
#ifndef IR_COLOR_RECEIVER_H
#define IR_COLOR_RECEIVER_H
#include "includes.h"
void IRColor_Initialization(void);

void IRColor_Read(uint16_t* c, uint16_t * red, uint16_t * green, uint16_t *blue, uint16_t * ir);

#endif




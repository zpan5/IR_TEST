#ifndef IR_COLOR_TEST_H
#define IR_COLOR_TEST_H

#include "includes.h"

void IRColorTest_Init(void);

void IRColorTest_Read(uint16_t *c, uint16_t *red, uint16_t *green, uint16_t *blue, uint16_t *ir);
void IRColorTest_CallbackRegister(void (*Callback)(uint16_t strength));
void IR_ColorTestEnable(uint8_t enable);
#endif




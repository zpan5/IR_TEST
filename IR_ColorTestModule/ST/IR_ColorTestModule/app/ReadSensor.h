#ifndef READ_SENSOR_H
#define READ_SENSOR_H
#include  <bsp.h>
#include "includes.h"

#define RGBCn 4

typedef struct{
	uint16_t red;
	uint16_t green;
	uint16_t blue;
	uint16_t clear;
} RGBC;

uint8_t ReadRGBCValue(RGBC * buffer, CPU_INT08U channel);
void ReadSensorTaskCreate(void);
#endif


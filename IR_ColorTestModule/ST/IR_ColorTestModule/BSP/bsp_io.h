#ifndef BSP_IO_H
#define BSP_IO_H
#include <bsp.h>
#include "includes.h"

void BSP_IO_Init(void);
void BSP_IO_Set(INT8U number);
void BSP_IO_Reset(INT8U number);
INT8U BSP_IO_Read(INT8U number);

#endif


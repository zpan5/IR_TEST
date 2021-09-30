#ifndef BSP_I2C_1_H
#define BSP_I2C_1_H
#include "bsp.h"

void I2C_1_Initialization(void);
CPU_INT08U I2C_1_Write(CPU_INT08U address, CPU_INT08U *dataP, CPU_INT08U dataLength);
CPU_INT08U I2C_1_Read(CPU_INT08U address, CPU_INT08U *dataP, CPU_INT08U dataLength);
CPU_INT08U I2C_1_CmdRead(CPU_INT08U address,CPU_INT08U command, CPU_INT08U *dataP, CPU_INT08U dataLength);
CPU_INT08U I2C_1_CmdWrite(CPU_INT08U address,CPU_INT08U command, CPU_INT08U *dataP, CPU_INT08U dataLength);

#endif




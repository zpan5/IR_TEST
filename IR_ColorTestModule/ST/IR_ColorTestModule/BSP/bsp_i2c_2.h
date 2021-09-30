#ifndef BSP_I2C_2_H
#define BSP_I2C_2_H
#include "bsp.h"

void I2C_2_Initialization(void);
CPU_INT08U I2C_2_Write(CPU_INT08U address, CPU_INT08U *dataP, CPU_INT08U dataLength);
CPU_INT08U I2C_2_Read(CPU_INT08U address, CPU_INT08U *dataP, CPU_INT08U dataLength);
CPU_INT08U I2C_2_CmdRead(CPU_INT08U address,CPU_INT08U command, CPU_INT08U *dataP, CPU_INT08U dataLength);
CPU_INT08U I2C_2_CmdWrite(CPU_INT08U address,CPU_INT08U command, CPU_INT08U *dataP, CPU_INT08U dataLength);

#endif









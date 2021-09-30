
/*
************************************************************************************************
*							Board Supply Pakage
*						ST Microelectronics STM32
*							on the 
*						Control board 1_2_1_v_1_1
*Filename		: w25q64.h
*Version		: V1.00
*Programer(s)	: 
************************************************************************************************
*/

/*
************************************************************************************************
*						Module
*
*Note(s)	: (1) This header file is protected from multiple pre-processor through use of the 
*				  BSP present pre-processor macro definition.
*
************************************************************************************************
*/

#ifndef BSP_W25Q64_H
#define BSP_W25Q64_H

void BSP_SPI_1_Init(void);

CPU_INT16U BSP_SPI_W25Q64_ReadID(void);

void BSP_SPI_W25Q64_WriteCmd(FunctionalState newstate);
void BSP_SPI_W25Q64_EreaseSector(CPU_INT32U addr);
void BSP_SPI_W25Q64_PageProgram(CPU_INT32U addr, CPU_INT08U *p_str, CPU_INT08U num);


CPU_INT08U BSP_SPI_W25Q64_ReadByte(CPU_INT32U addr);
void BSP_SPI_W25Q64_ReadStr(CPU_INT32U addr, CPU_INT08U *p_str, CPU_INT08U len);





#endif 








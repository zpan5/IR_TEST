#ifndef BSP_I2C1_H
#define BSP_I2C1_H
/*
*********************************************************************************************************
*                                             INCLUDE FILES
*********************************************************************************************************
*/

#include  <bsp.h>
#include  <stm32f2xx_i2c.h>

/*
*********************************************************************************************************
*                                               DATA TYPES
*********************************************************************************************************
*/


void BSP_I2C1_ISR_Handler(void);
void BSP_I2C1_ER_ISR_Handler(void);
void BSP_DMA1_CH1_ISR_Handler(void);
void BSP_DMA1_CH7_ISR_Handler(void);
void BSP_I2C1_Init(void);
void BSP_I2C1_SendBytes(uint8_t dev_addr, uint8_t *data, uint8_t len);
void BSP_I2C1_ReadBytes(uint8_t dev_addr, uint8_t *data, uint8_t len);
void BSP_I2C1_SendBytesC(uint8_t dev_addr, uint8_t reg_addr, uint8_t *data, uint8_t len);
void BSP_I2C1_ReadBytesC(uint8_t dev_addr, uint8_t reg_addr, uint8_t *data, uint8_t len);

void I2C1TestTaskCreate(void);

//#define 


#endif

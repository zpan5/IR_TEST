#ifndef BSP_I2C_IO_H
#define BSP_I2C_IO_H

#include  <bsp.h>

uint8_t BSP_I2C_IO_Init(void);
uint8_t BSP_I2C_IO_SendBytes(uint8_t dev_addr, uint8_t *data, uint8_t len);
uint8_t BSP_I2C_IO_ReadBytes(uint8_t dev_addr, uint8_t *data, uint8_t len);
uint8_t BSP_I2C_IO_SendBytesC(uint8_t dev_addr, uint8_t reg_addr, uint8_t *data, uint8_t len);
uint8_t BSP_I2C_IO_ReadBytesC(uint8_t dev_addr, uint8_t reg_addr, uint8_t *data, uint8_t len);


#endif

#include "includes.h"
#include "bsp_i2c_io.h"

/*************************************************************************************
*													private define
*************************************************************************************/
#define BSP_I2C_IO_SDA_CLK 		RCC_AHB1Periph_GPIOA
#define BSP_I2C_IO_SDA_PORT		GPIOA
#define BSP_I2C_IO_SDA_PIN		GPIO_Pin_3
#define BSP_I2C_IO_SCL_CLK		RCC_AHB1Periph_GPIOA
#define BSP_I2C_IO_SCL_PORT		GPIOA
#define BSP_I2C_IO_SCL_PIN		GPIO_Pin_2

#define BSP_I2C_IO_SDA_High   BSP_I2C_IO_SDA_PORT->BSRRL=BSP_I2C_IO_SDA_PIN
#define BSP_I2C_IO_SDA_Low		BSP_I2C_IO_SDA_PORT->BSRRH=BSP_I2C_IO_SDA_PIN
#define BSP_I2C_IO_SCL_High		BSP_I2C_IO_SCL_PORT->BSRRL=BSP_I2C_IO_SCL_PIN
#define BSP_I2C_IO_SCL_Low		BSP_I2C_IO_SCL_PORT->BSRRH=BSP_I2C_IO_SCL_PIN

#define START_DELAY 1000
#define BSP_I2C_IO_SCL_LOW_DELAY 1000
#define BSP_I2C_IO_SCL_HIGH_DELAY 1000

/*************************************************************************************
*													local function prototype 
*************************************************************************************/
uint8_t BSP_I2C_IO_delay(uint32_t delay);
uint8_t BSP_I2C_IO_Start(void);
uint8_t BSP_I2C_IO_sendAddrr(uint8_t addr);
uint8_t BSP_I2C_IO_waitACK(void);
uint8_t BSP_I2C_IO_sendData(uint8_t data);
uint8_t BSP_I2C_IO_sendStop(void) ;

uint8_t BSP_I2C_IO_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	
	RCC_AHB1PeriphClockCmd(BSP_I2C_IO_SDA_CLK, ENABLE);
	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Pin = BSP_I2C_IO_SDA_PIN;
	
	GPIO_SetBits(BSP_I2C_IO_SDA_PORT, BSP_I2C_IO_SDA_PIN);
	GPIO_Init(BSP_I2C_IO_SDA_PORT, &GPIO_InitStructure);
	
	RCC_AHB1PeriphClockCmd(BSP_I2C_IO_SDA_CLK, ENABLE);
	
	GPIO_InitStructure.GPIO_Pin = BSP_I2C_IO_SCL_PIN;
	GPIO_SetBits(BSP_I2C_IO_SCL_PORT, BSP_I2C_IO_SCL_PIN);
	GPIO_Init(BSP_I2C_IO_SCL_PORT, &GPIO_InitStructure);
	return 0;
}

uint8_t BSP_I2C_IO_SendBytes(uint8_t dev_addr, uint8_t *data, uint8_t len)
{
	uint32_t i;
	BSP_I2C_IO_Start();
	BSP_I2C_IO_sendAddrr((dev_addr<<1) & 0xFE);
	if (BSP_I2C_IO_waitACK() != 0) {
		return !0;
	}
	
	for(i = 0; i < len; i++) {
		BSP_I2C_IO_sendData(data[i]);
		if (BSP_I2C_IO_waitACK() != 0) {
			return !0;
		}
	}
	BSP_I2C_IO_sendStop();
	return 0;
}

uint8_t BSP_I2C_IO_ReadBytes(uint8_t dev_addr, uint8_t *data, uint8_t len)
{
	
	return 0;
}
uint8_t BSP_I2C_IO_SendBytesC(uint8_t dev_addr, uint8_t reg_addr, uint8_t *data, uint8_t len)
{
	return 0;
}
uint8_t BSP_I2C_IO_ReadBytesC(uint8_t dev_addr, uint8_t reg_addr, uint8_t *data, uint8_t len)
{
	return 0;
}

uint8_t BSP_I2C_IO_Start(void) 
{
	BSP_I2C_IO_SDA_High;
	BSP_I2C_IO_SCL_High;
	
	BSP_I2C_IO_delay(START_DELAY);
	BSP_I2C_IO_SDA_Low;
	BSP_I2C_IO_delay(START_DELAY);
	
	return 0;
}


uint8_t BSP_I2C_IO_sendAddrr(uint8_t addr) 
{
	uint8_t i; 
	for(i = 0; i < 8; i++) {
		BSP_I2C_IO_SCL_Low;
		if (((addr << i) & 0x80) != 0) {
			BSP_I2C_IO_SDA_High;
		}else {
			BSP_I2C_IO_SDA_Low;
		}
		BSP_I2C_IO_delay(BSP_I2C_IO_SCL_LOW_DELAY);
		BSP_I2C_IO_SCL_High;
		BSP_I2C_IO_delay(BSP_I2C_IO_SCL_HIGH_DELAY);
	}
	BSP_I2C_IO_SCL_Low;
	return 0;
}
uint8_t BSP_I2C_IO_waitACK(void) 
{
	uint32_t i;
	BSP_I2C_IO_SCL_Low;
	BSP_I2C_IO_SDA_High;
	BSP_I2C_IO_delay(BSP_I2C_IO_SCL_LOW_DELAY);
	BSP_I2C_IO_SCL_High;
	
	for(i = 0; i < BSP_I2C_IO_SCL_HIGH_DELAY; i++) {
		if (GPIO_ReadInputDataBit(BSP_I2C_IO_SDA_PORT, BSP_I2C_IO_SDA_PIN) == Bit_RESET) {
			BSP_I2C_IO_SCL_Low;
			return 0;
		}
	}
	BSP_I2C_IO_SCL_Low;
	return !0;
}
uint8_t BSP_I2C_IO_sendData(uint8_t data)
{
	uint8_t i; 
	for(i = 0; i < 8; i++) {
		BSP_I2C_IO_SCL_Low;
		if (((data << i) & 0x80) != 0) {
			BSP_I2C_IO_SDA_High;
		}else {
			BSP_I2C_IO_SDA_Low;
		}
		BSP_I2C_IO_delay(BSP_I2C_IO_SCL_LOW_DELAY);
		BSP_I2C_IO_SCL_High;
		BSP_I2C_IO_delay(BSP_I2C_IO_SCL_HIGH_DELAY);
	}
	BSP_I2C_IO_SCL_Low;
	return 0;
}
uint8_t BSP_I2C_IO_sendStop(void) 
{
	BSP_I2C_IO_SCL_Low;
	BSP_I2C_IO_SDA_Low;
	BSP_I2C_IO_delay(BSP_I2C_IO_SCL_LOW_DELAY);
	BSP_I2C_IO_SCL_High;
	BSP_I2C_IO_delay(BSP_I2C_IO_SCL_LOW_DELAY/2);
	BSP_I2C_IO_SDA_High;
	return 0;
}

uint8_t BSP_I2C_IO_delay(uint32_t delay)
{
	uint32_t i;
	for (i = 0; i < delay; i++) {
		__NOP();
	}
	return 0;
}



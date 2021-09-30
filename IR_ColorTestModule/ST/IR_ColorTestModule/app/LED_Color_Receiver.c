#include "led_color_receiver.h"
/****************************************************************************************************************
*																					Local define
****************************************************************************************************************/
#define VEML3328SL_ADDRESS								0x10
#define VEML3328SL_OPERATION_CMD					0x00
#define VEML3328SL_OPERATION_LSB					0x00
#define VEML3328SL_OPERATION_MSB					0x00
#define VEML3328SL_C_CMD									0x04
#define VEML3328SL_R_CMD									0x05
#define VEML3328SL_G_CMD									0x06
#define VEML3328SL_B_CMD									0x07
#define VEML3328SL_IR_CMD									0x08

/****************************************************************************************************************
**																			void LEDColor_Initialization(void)
**1. 	Power On VEML3328SL color sensor.   	SD1:		BIT:15			->		0
**																					SD0:		BIT:0				->		0
**2. 	Power On all channels.								SD_ALS:	BTI:14			->		0
**3.	Digtal Gain????												DG:			BIT:13:12		->		0:0
**4.  High sensitivity											SENS:		BIT:6				->		0
**5.	Integration time setting(50ms)				IT:			BIT:5:4			->		0:0
**6.	Auto mode															AF:			BIT:3				->		0
**7.	No trigger														TRIG		BIT:2				->		0

****************************************************************************************************************/

void LEDColor_Initialization(void)
{
	CPU_INT08U data[2];
	data[0] = VEML3328SL_OPERATION_LSB;
	data[1] =  VEML3328SL_OPERATION_MSB;
	I2C_1_CmdWrite(VEML3328SL_ADDRESS, VEML3328SL_OPERATION_CMD, data, 2);
}

uint8_t LEDColor_Read(uint16_t* c, uint16_t * red, uint16_t * green, uint16_t *blue, uint16_t * ir)
{
	CPU_INT08U data[2];
	if (I2C_1_CmdRead(VEML3328SL_ADDRESS, VEML3328SL_C_CMD, data, 2) != OS_ERR_NONE) {
		return !0;
	} else {
		*c = data[0] | (data[1] << 8);
	}
	
	if (I2C_1_CmdRead(VEML3328SL_ADDRESS, VEML3328SL_R_CMD, data, 2) != OS_ERR_NONE) {
		return !0;
	} else {
		*red = data[0] | (data[1] << 8);
	}
	
	if (I2C_1_CmdRead(VEML3328SL_ADDRESS, VEML3328SL_G_CMD, data, 2) != OS_ERR_NONE) {
		return !0;
	}else {
		*green = data[0] | (data[1] << 8);
	}
	
	if (I2C_1_CmdRead(VEML3328SL_ADDRESS, VEML3328SL_B_CMD, data, 2) != OS_ERR_NONE) {
		return !0;
	}else {
		*blue = data[0] | (data[1] << 8);
	}
	
	if (I2C_1_CmdRead(VEML3328SL_ADDRESS, VEML3328SL_IR_CMD, data, 2) != OS_ERR_NONE) {
		return !0;
	}else {
		*ir = data[0] | (data[1] << 8);
	}
	return OS_ERR_NONE;
}











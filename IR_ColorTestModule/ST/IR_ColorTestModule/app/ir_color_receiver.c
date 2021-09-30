#include "includes.h"
#include "ir_color_receiver.h"


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
																						SD0:		BIT:0				->		0
**2. 	Power On all channels.								SD_ALS:	BTI:14			->		0
**3.	Digtal Gain????												DG:			BIT:13:12		->		0:0
**4.  High sensitivity											SENS:		BIT:6				->		0
**5.	Integration time setting(50ms)				IT:			BIT:5:4			->		0:0
**6.	Auto mode															AF:			BIT:3				->		0
**7.	No trigger														TRIG		BIT:2				->		0

****************************************************************************************************************/

void IRColor_Initialization(void)
{
	CPU_INT08U data[2];
	data[0] = VEML3328SL_OPERATION_LSB;
	data[1] =  VEML3328SL_OPERATION_MSB;
	I2C_2_CmdWrite(VEML3328SL_ADDRESS, VEML3328SL_OPERATION_CMD, data, 2);
}
CPU_INT16U addressxxx;
void IRColor_Read(uint16_t * c, uint16_t * red, uint16_t * green, uint16_t *blue, uint16_t * ir)
{

	static CPU_INT08U data[2];
	data[0] = 0; data[1] = 0;
	I2C_2_CmdRead(VEML3328SL_ADDRESS, VEML3328SL_C_CMD, data, 2);
	*c = data[0] | (data[1] << 8);
	OSTimeDly(2);
	
	data[0] = 0; data[1] = 0;
	I2C_2_CmdRead(VEML3328SL_ADDRESS, VEML3328SL_R_CMD, data, 2);
	*red = data[0] | (data[1] << 8);
	OSTimeDly(2);
	
	data[0] = 0; data[1] = 0;
	I2C_2_CmdRead(VEML3328SL_ADDRESS, VEML3328SL_G_CMD, data, 2);
	*green = data[0] | (data[1] << 8);
	OSTimeDly(2);
	
	data[0] = 0; data[1] = 0;
	I2C_2_CmdRead(VEML3328SL_ADDRESS, VEML3328SL_B_CMD, data, 2);
	*blue = data[0] | (data[1] << 8);
	OSTimeDly(2);
	
	data[0] = 0; data[1] = 0;
	I2C_2_CmdRead(VEML3328SL_ADDRESS, VEML3328SL_IR_CMD, data, 2);
	*ir = data[0] | (data[1] << 8);
//	OSTimeDly(1);
//	
//	data[0] = 0; data[1] = 0;
//	I2C_2_CmdRead(VEML3328SL_ADDRESS, 0x0c, data, 2);
//	addressxxx = data[0] | (data[1] << 8);
	//OSTimeDly(1);
}













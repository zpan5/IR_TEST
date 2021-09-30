

/**************************************************************************************************************
*
*
*
**************************************************************************************************************/

#include "includes.h"
#include "ir_color_test.h"

typedef struct {
	uint16_t BspIr_clear;
	uint16_t BspIr_red;
  uint16_t BspIr_green;
	uint16_t BspIr_blue;
  uint16_t BspIr_ir;
}IR_COLOR_SENSOR;

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

/****************************************************************************************
**																Local variable 
****************************************************************************************/
static OS_STK IR_colorTest_TaskStk[IR_ColorTest_Task_STK_SIZE];
static uint16_t BspIr_clear, BspIr_red, BspIr_green,  BspIr_blue,  BspIr_ir;

volatile uint8_t run = !0;
void (*IRColorTest_Callback)(uint16_t strength);
/****************************************************************************************
**																Local function prototype 
****************************************************************************************/

void IR_ColorTest_Task(void *p_arg);

void IRColorTest_Init(void)
{

		OSTaskCreateExt((void (*)(void *)) IR_ColorTest_Task,            /* Create the start task                                */
                    (void           *) 0,
                    (OS_STK         *)&IR_colorTest_TaskStk[IR_ColorTest_Task_STK_SIZE - 1],
                    (INT8U           ) IR_ColorTest_Task_PRIO,
                    (INT16U          ) IR_ColorTest_Task_PRIO,
                    (OS_STK         *)&IR_colorTest_TaskStk[0],
                    (INT32U          ) IR_ColorTest_Task_STK_SIZE,
                    (void           *) 0,
                    (INT16U          )(OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR));

}

void IR_ColorTestEnable(uint8_t enable)
{
	run = enable;
}

void IR_ColorTest_Task(void *p_arg)
{
	CPU_INT08U data[2];
	
	BspIr_clear = 0;
	BspIr_red = 0;
	BspIr_green = 0;
  BspIr_blue = 0;
  BspIr_ir = 0;
	
	data[0] = VEML3328SL_OPERATION_LSB;
	data[1] = VEML3328SL_OPERATION_MSB;
	I2C_2_CmdWrite(VEML3328SL_ADDRESS, VEML3328SL_OPERATION_CMD, data, 2);
	
	for(;;) {
		OSTimeDly(30);
		if (run != 0) {
			IRColor_Read(&BspIr_clear, &BspIr_red, &BspIr_green, &BspIr_blue, &BspIr_ir);
		}
		

//		CPU_INT08U data[2];
//		I2C_2_CmdRead(VEML3328SL_ADDRESS, VEML3328SL_C_CMD, data, 2);
//		BspIr_clear = data[0] | (data[1] << 8);
//		
//		I2C_2_CmdRead(VEML3328SL_ADDRESS, VEML3328SL_R_CMD, data, 2);
//		BspIr_red = data[0] | (data[1] << 8);
//		
//		I2C_2_CmdRead(VEML3328SL_ADDRESS, VEML3328SL_G_CMD, data, 2);
//		BspIr_green = data[0] | (data[1] << 8);
//		
//		I2C_2_CmdRead(VEML3328SL_ADDRESS, VEML3328SL_B_CMD, data, 2);
//		BspIr_blue = data[0] | (data[1] << 8);
//		
//		I2C_2_CmdRead(VEML3328SL_ADDRESS, VEML3328SL_IR_CMD, data, 2);
//		BspIr_ir = data[0] | (data[1] << 8);
		
		IR_ReceiveStrengthCallback(BspIr_ir);   
		if (IRColorTest_Callback != NULL) {
			IRColorTest_Callback(BspIr_ir);
		}
	}

}


void IRColorTest_CallbackRegister(void (*Callback)(uint16_t strength))
{
	IRColorTest_Callback = Callback;
}


void IRColorTest_Read(uint16_t *c, uint16_t *red, uint16_t *green, uint16_t *blue, uint16_t *ir)
{
	*c = BspIr_clear;
	*red = BspIr_red;
	*green = BspIr_green;
	*blue = BspIr_blue;
	*ir = BspIr_ir;
}





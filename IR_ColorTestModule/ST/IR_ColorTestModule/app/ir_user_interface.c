

#include "includes.h"
#include "ir_user_interface.h"

/*************************************************************************************************
*																	  Local define
*************************************************************************************************/
#define LED_R_CURRENT_CONTROL_ADDR				0x0E
#define LED_G_CURRENT_CONTROL_ADDR				0x0D


/*************************************************************************************************
*																		Local variable
*************************************************************************************************/
// OS task stack variable define
static OS_STK	IR_UserInterface_CAN_Task_STK[IR_UserInterface_CAN_Task_STK_SIZE];
static OS_STK IR_UserInterface_USART_Task_STK[IR_UserInterface_UART_Task_STK_SIZE];

static  BSP_OS_SEM   	CanWait;
static INT8U 					canRXON = 0;
CAN_EventHandle_TypeDef CAN_EventHandle;
static CPU_INT08U can_buffer[50];
static CPU_INT08U can_bufferCounter;
/*************************************************************************************************
*																		Local function prototype
*************************************************************************************************/

void IR_UserInterfaceCAN_Task(void *p_arg);
void IR_UserInterfaceUart_Task(void *p_arg);
CPU_INT08U UserInterface(CPU_INT08U *in_arg, CPU_CHAR *out_arg);






void IR_UserInterfaceInitialization(void)
{
	 OSTaskCreateExt((void (*)(void *))   IR_UserInterfaceCAN_Task,            /* Create the start task                                */
                    (void            *) 0,
                    (OS_STK          *) &IR_UserInterface_CAN_Task_STK[IR_UserInterface_CAN_Task_STK_SIZE - 1],
                    (INT8U            ) IR_UserInterface_CAN_Task_PRIO,
                    (INT16U           ) IR_UserInterface_CAN_Task_PRIO,
                    (OS_STK          *) &IR_UserInterface_CAN_Task_STK[0],
                    (INT32U           ) IR_UserInterface_CAN_Task_STK_SIZE,
                    (void            *) 0,
                    (INT16U           ) (OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR));
										
	OSTaskCreateExt((void (*)(void *)) IR_UserInterfaceUart_Task,            /* Create the start task                                */
                    (void           *) 0,
                    (OS_STK         *)&IR_UserInterface_USART_Task_STK[IR_UserInterface_UART_Task_STK_SIZE - 1],
                    (INT8U           ) IR_UserInterface_UART_Task_PRIO,
                    (INT16U          ) IR_UserInterface_UART_Task_PRIO,
                    (OS_STK         *)&IR_UserInterface_USART_Task_STK[0],
                    (INT32U          ) IR_UserInterface_UART_Task_STK_SIZE,
                    (void           *) 0,
                    (INT16U          )(OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR));

	LEDColor_Initialization();
}

/*****************************************************************************************************************
*														can receive call back function 
* Description	: Waiting a data package from can bus.
*
* Arguments		: None
*
* Return			: None
*
*****************************************************************************************************************/
void UserInterface_CanWait(void)
{
	CPU_INT08U err;
	can_bufferCounter = 0;
	canRXON = !0;
	OSSemPend(CanWait,0, &err);
	
}

/*****************************************************************************************************************
*														can receive call back function 
* Description	: This function is be called when CAN controlller received data from CAN bus. It is to preprocess
*								package and store the package to local buffer "can_buffer", when a pakage receive is be done, it 
*								post a event to "CanWait"
*
* Arguments		: pdata is the data passed to the function.
*								length is used to indicate how many data be transfer to the function.
* Returns			: None
*
*****************************************************************************************************************/
void CanCallback(INT8U *pdata, INT8U length)
{
	int i;
	if (canRXON != 0){
		for (i = 0; i < length; i++) {
			can_buffer[can_bufferCounter++] = pdata[i];

			if (can_bufferCounter > 1) {
				if (can_buffer[can_bufferCounter-2] == '\r' && can_buffer[can_bufferCounter-1] == '\n') {
					can_buffer[can_bufferCounter-2] = '\0';
					OSSemPost(CanWait);
					canRXON = 0;
					break;
				}
			}
		}
	}
}

/*******************************************************************************************************
*																		can bus user interface
* Description	: This function is used to receive data from can bus and call ''.
*
* Arguments		: p_arg is the argument passed to IR User Interface by 'OsTaskCreate()'.
*
* Returns     : none
*
* Notes       : 1) The first line of code is used to prevent a compiler warning because 'p_arg' is not
*                  used.  The compiler should not generate any code for this statement.
*******************************************************************************************************/

void IR_UserInterfaceCAN_Task(void *p_arg)
{

	static CPU_INT08U outdata[50];
	(void) p_arg;
	
	CanWait = OSSemCreate(0);
	CAN_EventHandle.IDE = CAN_Id_Standard;
	CAN_EventHandle.RTR = CAN_RTR_Data;
	CAN_EventHandle.StdId = 0X04;
	
	CAN_EventHandle.Can_Callback = CanCallback;
	BSP_Can_EventRegister(&CAN_EventHandle);
	
	for(;;) {
		UserInterface_CanWait();
		if (UserInterface(can_buffer, (CPU_CHAR *)outdata) != 0) {
			BSP_CAN_Printf(DEVICE_CAN_ID, "%s", (CPU_CHAR*)outdata);
		}
	}
}
/*******************************************************************************************************
*																		serial user interface
* Description	: This function is used to receive data from serial port.
*
* Arguments		: p_arg is the argument passed to IR User Interface by 'OsTaskCreate()'.
* 
* Returns     : none
* 
* Notes       : 1) The first line of code is used to prevent a compiler warning because 'p_arg' is not
*                  used.  The compiler should not generate any code for this statement.
*******************************************************************************************************/

void IR_UserInterfaceUart_Task(void *p_arg)
{
	static CPU_INT08U data[50];
	static CPU_CHAR outdata[50];
	
	(void) p_arg;
	
	//BSP_Ser_Printf("123");
	//sprintf((CPU_CHAR*) outdata, "321");
	for(;;) {
		BSP_Ser_RdString(data, 50);
		if (UserInterface(data, &outdata[0]) != 0) {
			//BSP_Can_TransmitString(DEVICE_CAN_ID, "%s", (CPU_CHAR*)outdata);
			BSP_Ser_Printf("%s",  (CPU_CHAR *) outdata);
		}
	}
}

/*
*********************************************************************************************************
*                                          User interface
*
* Description : This function is be called when the bus input port (CAN bus and Serial bus) received a 
* 							command.
*
* Arguments   : in_arg   is the argument passed to 'IR_UserInterface' by the input ports.
*							: out_arg  is the argument returned to call function.
* Returns     : This function is used to indicate whether there have argument returned.
*
* Notes       : 1) The first line of code is used to prevent a compiler warning because 'p_arg' is not
*                  used.  The compiler should not generate any code for this statement.
*********************************************************************************************************
*/


CPU_INT08U UserInterface(CPU_INT08U *in_arg, CPU_CHAR *out_arg)
{
	CPU_INT08U outputOn;
	
	if (Str_Cmp_N((CPU_CHAR*)in_arg, "IRS:", 4) == 0){
		if (BSP_Ser_1_IRDA_Printf("%s\r\n", &in_arg[4]) == IR_DATA_MODULE) {
			sprintf((CPU_CHAR*) out_arg, "OK\r\n");
			outputOn = !0;
		}else {
			sprintf((CPU_CHAR*) out_arg, "ir tx in led mode\r\n");
			outputOn = !0;
		}
	}else if (Str_Cmp((CPU_CHAR *) in_arg, "IRCLR") == 0) {
		uint16_t clear;
		uint16_t red;
		uint16_t green;
		uint16_t blue;
		uint16_t ir;
		
		IRColorTest_Read(&clear, &red, &green, &blue, &ir);
		
		sprintf((CPU_CHAR*)out_arg,"IR%5d\r\n", ir);
		outputOn = !0;
	}else if (Str_Cmp((CPU_CHAR *) in_arg, "IRLED:On") == 0) {
		IR_LED_on();
		sprintf((CPU_CHAR*) out_arg, "OK\r\n");
		outputOn = !0;
		
	}else if (Str_Cmp((CPU_CHAR *) in_arg, "IRLED:Off") == 0) {
		IR_LED_off();
		sprintf((CPU_CHAR*) out_arg, "OK\r\n");
		outputOn = !0;
		
	}else if (Str_Cmp((CPU_CHAR*) in_arg, "LEDCLR") == 0){
		
		uint16_t clear;
		uint16_t red;
		uint16_t green;
		uint16_t blue;
		uint16_t ir;
		
		if (LEDColor_Read(&clear, &red, &green, &blue, &ir) != OS_ERR_NONE) {
			sprintf((CPU_CHAR*)out_arg,"LedColorSensorException\r\n");
		}else {
			sprintf((CPU_CHAR*)out_arg,"C%05dR%05dG%05dB%05dIR%05d\r\n", clear, red, green, blue, ir);
		}
		outputOn = !0;
	}
	else if (Str_Cmp_N((CPU_CHAR*) in_arg, "LED", 3) == 0) {
		CPU_INT32U strength;
		
		if(Str_Cmp_N((CPU_CHAR*) &in_arg[3], "R:", 2) == 0) {

			sscanf((CPU_CHAR *)&in_arg[5], "%d", &strength);
			if (strength < 1024){
				CPU_INT08U data[2];
				data[1] = strength & 0xff;
				data[0] = (strength >> 8) & 0xf;
				if(BSP_I2C_IO_SendBytes(LED_R_CURRENT_CONTROL_ADDR, data, 2) == 0) {
					sprintf((CPU_CHAR*) out_arg, "OK\r\n");
				}else {
					sprintf((CPU_CHAR*) out_arg, "I2C_BUS_ERROR\r\n");
				}
				outputOn = !0;
			}else {
				sprintf((CPU_CHAR*) out_arg, "out of range\r\n");
				outputOn = !0;
			}

		}else if(Str_Cmp_N((CPU_CHAR*) &in_arg[3], "G:", 2) == 0) {
			sscanf((CPU_CHAR *)&in_arg[5], "%d", &strength);
			if (strength < 1024){
				CPU_INT08U data[2];
				data[1] = strength & 0xff;
				data[0] = (strength >> 8) & 0xf;
				if(BSP_I2C_IO_SendBytes(LED_G_CURRENT_CONTROL_ADDR, data, 2) == 0) {
					sprintf((CPU_CHAR*) out_arg, "OK\r\n");
				}else {
					sprintf((CPU_CHAR*) out_arg, "I2C_BUS_ERROR\r\n");
				}
				outputOn = !0;
			}else {
				sprintf((CPU_CHAR*) out_arg, "out of range\r\n");
				outputOn = !0;
			}
		}else if(Str_Cmp_N((CPU_CHAR*) &in_arg[3], "B:", 2) == 0) {
			sscanf((CPU_CHAR *)&in_arg[5], "%d", &strength);
			if (strength < 1024){
			WriteAO(AO_CHANNEL1, strength);
			sprintf((CPU_CHAR*) out_arg, "OK\r\n");
			outputOn = !0;
			}else {
				sprintf((CPU_CHAR*) out_arg, "out of range\r\n");
				outputOn = !0;
			}
		}
		else if(Str_Cmp_N((CPU_CHAR*) &in_arg[3], "C:", 2) == 0) {
			sscanf((CPU_CHAR *)&in_arg[5], "%d", &strength);
			if (strength < 1024){
			WriteAO(AO_CHANNEL0, strength);
			sprintf((CPU_CHAR*) out_arg, "OK\r\n");
			outputOn = !0;
			}else {
				sprintf((CPU_CHAR*) out_arg, "out of range\r\n");
				outputOn = !0;
			}
		}else {
			sprintf(out_arg, "InvalidCmd:%s\r\n",in_arg);
		}
	}
	else if (Str_Cmp_N((CPU_CHAR *) in_arg, "IRTX:", 5) == 0 ) {
		
		CPU_INT32U strength;
		
		sscanf((CPU_CHAR *)&in_arg[5], "%d", &strength);
		if (strength < 256){
			IR_SetTxStrength(strength << 4);
			sprintf((CPU_CHAR*) out_arg, "OK\r\n");
			outputOn = !0;
		}else {
			sprintf((CPU_CHAR*) out_arg, "out of range\r\n");
			outputOn = !0;
		}
		
	}else {
		
		sprintf(out_arg, "InvalidCmd:%s\r\n",in_arg);
		
	}

	return outputOn;

}



#include "includes.h"
#include "ir_module.h"

/*************************************************************************************************
*																	Local define
*************************************************************************************************/
#define IR_LED_CURRENT_CONTROL_1_ADDR				0x0D
#define IR_LED_CURRENT_CONTROL_2_ADDR				0x0E
#define ON																	1
#define OFF																	0
#define IR_RX_BUFFER_SIZE										50

/*************************************************************************************************
*																	Local variable
*************************************************************************************************/
static OS_STK IR_Receive_TaskStk[IR_Receive_Task_STK_SIZE];
static CPU_INT08U IR_Receiving;
static CPU_INT08U *IR_ReceiveBuffer;
static CPU_INT08U IR_ReceiveCounter;
static CPU_INT08U IR_ReceiveBufferSize;
static CPU_INT32U IR_ReceiveTimeout;
static CPU_INT32U IR_ReceiveStrength;

static CPU_INT08U data[IR_RX_BUFFER_SIZE];

OS_EVENT *IR_RxWait;

/*************************************************************************************************
*																	Local function prototype 
*************************************************************************************************/
void IR_ReceiveInitialization(void);
void IR_ReceiveTask(void *p_arg);
CPU_INT08U IR_ReceiveWaitingData(CPU_INT08U *bufferP, CPU_INT08U bufferSize);


void IR_Initialization(void)
{
	BSP_Ser_1_IDRA_RdSetCallback(IR_ReceiveDataCallback);
	IR_ReceiveInitialization();
}


void IR_SetTxStrength(CPU_INT16U strength)
{
	CPU_INT08U data[2];
	data[1] = strength & 0xff;
	data[0] = (strength >> 8) & 0xf;
	I2C_3_Write(IR_LED_CURRENT_CONTROL_1_ADDR, data, 2);
	
	I2C_3_Write(IR_LED_CURRENT_CONTROL_2_ADDR, data, 2);
}


void IR_ReceiveInitialization(void)
{
	
	OSTaskCreateExt((void (*)(void *)) IR_ReceiveTask,            /* Create the start task                                */
                    (void           *) 0,
                    (OS_STK         *)&IR_Receive_TaskStk[IR_Receive_Task_STK_SIZE - 1],
                    (INT8U           ) IR_Receive_Task_PRIO,
                    (INT16U          ) IR_Receive_Task_PRIO,
                    (OS_STK         *)&IR_Receive_TaskStk[0],
                    (INT32U          ) IR_Receive_Task_STK_SIZE,
                    (void           *) 0,
                    (INT16U          )(OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR));
}

void IR_SendingData(CPU_INT08U *pData)
{
	
}

/**************************************************************************************
* 														 IR receive strength callback
* Description : This function is called to update ir strength.
*
* Arguments		: strength is the ir strength pass to this function.
*
* Returns			: none.
*
**************************************************************************************/

void IR_ReceiveStrengthCallback(uint16_t strength)
{
	if (IR_Receiving == OFF) {
		return ;
	}
	IR_ReceiveStrength = strength > IR_ReceiveStrength?strength:IR_ReceiveStrength;
}


/********************************************************************************************
* 														 IR receive data Callback
* Description : This function is called when IR serial module received a data. It is to 
*								preproccess received data of ir module and package the data. When a pacakge
*								is received post a event to IR_RxWait, and close ir receive function.
*
* Arguments		: data is the ir module receved pass to this function.
*
* Returns			: none.
*
* Note				: This function is need to registered to the IR receive module.
*
*******************************************************************************************/

void IR_ReceiveDataCallback(CPU_INT08U data)
{
	if (IR_Receiving == OFF) {
		return ;
	}
	if (OSTimeGet() - IR_ReceiveTimeout > 100){
		IR_ReceiveCounter = 0;
	}
	IR_ReceiveTimeout = OSTimeGet();
	
	IR_ReceiveBuffer[IR_ReceiveCounter] = data;
	IR_ReceiveCounter++;
	
	if (IR_ReceiveCounter > 1 && 
		IR_ReceiveBuffer[IR_ReceiveCounter - 2] == '\r' && 
		IR_ReceiveBuffer[IR_ReceiveCounter - 1] == '\n'	) {
			IR_ReceiveBuffer[IR_ReceiveCounter-2] = '\0';
			OSSemPost(IR_RxWait);
			IR_Receiving = OFF;
	}
	
	if (IR_ReceiveCounter >= IR_ReceiveBufferSize - 1) {
		IR_ReceiveCounter = 0;
	}
}

/***************************************************************************************
*								           ir receive waiting data
* Description : This function is to waiting a package be received from ir rx module.
*
* Arguments		: bufferP is a pointer of buffer used to store data be received from ir module
*								bufferSize is argument mark size of buffer.
*
* Returns			: err indicate a error 
*
***************************************************************************************/

CPU_INT08U IR_ReceiveWaitingData(CPU_INT08U *bufferP, CPU_INT08U bufferSize)
{
	CPU_INT08U err;	
	IR_ReceiveBuffer = bufferP;	
	IR_ReceiveCounter = 0;	
	IR_ReceiveBufferSize = bufferSize;
	IR_ReceiveTimeout = OSTimeGet();
	IR_ReceiveStrength =0;
	IR_Receiving = ON;
	OSSemPend(IR_RxWait, 0, &err);	
	
	return err;
}

/******************************************************************************************
** when a package is received from ir interface, It transfer it to can bus;
******************************************************************************************/

void IR_ReceiveTask(void *p_arg)
{
	
	IR_Receiving = OFF;
	
	IR_RxWait = OSSemCreate(0);
	IRColorTest_CallbackRegister(IR_ReceiveStrengthCallback);
	BSP_Ser_1_IDRA_RdSetCallback(IR_ReceiveDataCallback);
	for(;;)
	{
		IR_ReceiveWaitingData(data, IR_RX_BUFFER_SIZE);
		
		// Sending the received data to CAN bus
		BSP_CAN_Printf(DEVICE_CAN_ID,"IRR%05dData%s\r\n", IR_ReceiveStrength, data);
		OSTimeDly(100);
	}

}








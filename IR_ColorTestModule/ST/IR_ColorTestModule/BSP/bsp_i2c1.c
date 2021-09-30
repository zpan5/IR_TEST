#include "bsp_i2c1.h"


#define I2C_Receive_Start 					1
#define I2C_Receive_Start_Again			7
#define I2C_Receive_Addr 						2
#define I2C_Receive_Addr_Again			8
#define I2C_Receive_Reg 						3
#define I2C_Receive_Data_2					4
#define I2C_Receive_NA   						5
#define I2C_Receive_ED							6

#define I2C_Transmit_Start  				10
#define I2C_Transmit_Addr						11
#define I2C_Transmit_Reg						12
#define I2C_Transmit_Reg_Start			13
#define I2C_Transmit_Addr_Again			14

#define T_Command_Data 	1
#define T_Data				2

int8_t I2C1_State = 0;
int8_t I2C1_Transfer_Mode;

int8_t DeviceAddr;
int8_t RegAddr;
int8_t *ReceiveBuffer;
int8_t ReceiveNumber;
int8_t ReceiveCounter;

int8_t *TransmittedData;
extern int8_t a;
int32_t TimeOut;
OS_EVENT *FlagI2C1_Transmitter;
OS_EVENT *FlagI2C1_Receiver;
OS_EVENT *FlagI2C1_Lock;

static OS_STK I2C1Test_TaskStk[I2C1Test_STK_SIZE];

void BSP_I2C1_ISR_Handler(void)
{
	/*Judge mode of transmite or receive*/
//	CPU_INT32U I2C1_SR1;
	
	switch(I2C1_State){
		case I2C_Receive_Start:
			if (I2C1_Transfer_Mode == T_Command_Data) {
				if (I2C_GetITStatus(I2C1, I2C_IT_SB) == SET) {
					I2C_Send7bitAddress(I2C1, (DeviceAddr << 1), I2C_Direction_Transmitter);
					I2C1_State = I2C_Receive_Addr;
				}
			} else if (I2C1_Transfer_Mode == T_Data) {
				if (I2C_GetITStatus(I2C1, I2C_IT_SB) == SET) {
					I2C_Send7bitAddress(I2C1, (DeviceAddr << 1), I2C_Direction_Receiver);
					I2C1_State = I2C_Receive_Addr;
				}
			}
		break;
		case I2C_Receive_Start_Again:
			if (I2C_GetITStatus(I2C1, I2C_IT_SB) == SET) {
				I2C_Send7bitAddress(I2C1, (DeviceAddr << 1), I2C_Direction_Receiver);
				I2C1_State = I2C_Receive_Addr_Again;
			}
		
			break;
		case I2C_Receive_Addr:
			if (I2C1_Transfer_Mode == T_Command_Data) {
				if(I2C_GetITStatus(I2C1, I2C_IT_ADDR) == SET) {
					I2C_ITConfig(I2C1, I2C_IT_EVT, DISABLE);
					I2C1_State = I2C_Receive_Reg;
					OSSemPost(FlagI2C1_Receiver);
				}
			}else if ( I2C1_Transfer_Mode == T_Data) {
				if (I2C_GetITStatus(I2C1, I2C_IT_ADDR) == SET) {
					I2C_ITConfig(I2C1, I2C_IT_EVT, DISABLE);
					OSSemPost(FlagI2C1_Receiver);
				}
			}
			break;
		case I2C_Receive_Addr_Again:
			if (I2C_GetITStatus(I2C1, I2C_IT_ADDR) == SET) {
				I2C_ITConfig(I2C1, I2C_IT_EVT, DISABLE);
				OSSemPost(FlagI2C1_Receiver);
			}
			break;
		case I2C_Receive_Reg:
			if (I2C_GetITStatus(I2C1, I2C_IT_BTF)) {
				I2C_GenerateSTART(I2C1, ENABLE);
				I2C1_State = I2C_Receive_Start_Again;
			}
			break;
		case I2C_Transmit_Start:
			if(I2C_GetITStatus(I2C1, I2C_IT_SB) == SET) {
				I2C_Send7bitAddress(I2C1, (DeviceAddr << 1), I2C_Direction_Transmitter);
				I2C1_State = I2C_Transmit_Addr;
			}
			break;
		case I2C_Transmit_Addr:
			if (I2C1_Transfer_Mode == T_Command_Data) {
				if(I2C_GetITStatus(I2C1, I2C_IT_ADDR) == SET) {
					I2C_ITConfig(I2C1, I2C_IT_EVT, DISABLE);
					OSSemPost(FlagI2C1_Transmitter);
					
					I2C1_State = I2C_Transmit_Reg;
				}
			}else if (I2C1_Transfer_Mode == T_Data){ 
					if(I2C_GetITStatus(I2C1, I2C_IT_ADDR) == SET) {
						I2C_ITConfig(I2C1, I2C_IT_EVT, DISABLE);
						
						OSSemPost(FlagI2C1_Transmitter);
					}
			}else{
				for(;;);
			}
			break;	
			
		case I2C_Transmit_Reg:
			if(I2C_GetITStatus(I2C1, I2C_IT_BTF) == SET) {
				I2C_ITConfig(I2C1, I2C_IT_EVT, DISABLE);
				
				OSSemPost(FlagI2C1_Transmitter);
			}
			break;
			
		default:
			break; 
	}
}

void BSP_I2C1_ER_ISR_Handler(void)
{
	for(;;);
}

 void BSP_DMA1_CH1_ISR_Handler(void)
{
	if( DMA_GetFlagStatus(DMA1_Stream0, DMA_FLAG_HTIF0) == SET) {
		DMA_ClearFlag(DMA1_Stream0, DMA_FLAG_TCIF0);
		DMA_ITConfig(DMA1_Stream0, DMA_IT_TC, DISABLE);
		DMA_Cmd(DMA1_Stream0, DISABLE);
		OSSemPost(FlagI2C1_Receiver);
	}
	 
  I2C_DMACmd(I2C1,DISABLE);
}


void BSP_DMA1_CH7_ISR_Handler(void)
{
	if( DMA_GetFlagStatus(DMA1_Stream6, DMA_FLAG_HTIF6) == SET) {
		DMA_ClearFlag(DMA1_Stream6, DMA_FLAG_TCIF6);
		DMA_ITConfig(DMA1_Stream6, DMA_IT_TC, DISABLE);
		DMA_Cmd(DMA1_Stream6, DISABLE);
		OSSemPost(FlagI2C1_Transmitter);
	}
	 
  I2C_DMACmd(I2C1,DISABLE);
}


void BSP_I2C1_Init(void)
{
	I2C_InitTypeDef I2C_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);  
	
	I2C_InitStructure.I2C_Ack = I2C_Ack_Enable;
	I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
	I2C_InitStructure.I2C_ClockSpeed = 10000;
	I2C_InitStructure.I2C_DutyCycle = I2C_DutyCycle_2;
	I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;
	I2C_Init(I2C1, &I2C_InitStructure);
	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_9;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_25MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource6, GPIO_AF_I2C1);
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource9, GPIO_AF_I2C1);
	
	I2C_Cmd(I2C1, ENABLE);
	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Pin	=  GPIO_Pin_5;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_25MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	GPIO_ResetBits(GPIOB, GPIO_Pin_5);
	OSTimeDly(2);
	GPIO_SetBits(GPIOB, GPIO_Pin_5);
	
	// Reset Multiplexer of I2C bus 
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Pin	=  GPIO_Pin_5;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_25MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	GPIO_ResetBits(GPIOB, GPIO_Pin_5);
	OSTimeDly(2);
	GPIO_SetBits(GPIOB, GPIO_Pin_5);
	
	
	FlagI2C1_Transmitter = OSSemCreate(0);
	FlagI2C1_Receiver = OSSemCreate(0);
	FlagI2C1_Lock = OSSemCreate(1);
	
	BSP_IntVectSet(BSP_INT_ID_I2C1_EV, BSP_I2C1_ISR_Handler);
  BSP_IntEn(BSP_INT_ID_I2C1_EV);
	 
	BSP_IntVectSet(BSP_INT_ID_I2C1_ER, BSP_I2C1_ER_ISR_Handler); 
  BSP_IntEn(BSP_INT_ID_I2C1_EV);
	
	BSP_IntVectSet(BSP_INT_ID_DMA1_CH1, BSP_DMA1_CH1_ISR_Handler); 
  BSP_IntEn(BSP_INT_ID_DMA1_CH1);	
	
	BSP_IntVectSet(BSP_INT_ID_DMA1_CH7, BSP_DMA1_CH7_ISR_Handler); 
  BSP_IntEn(BSP_INT_ID_DMA1_CH7);	
}

void BSP_I2C1_DMA_Cofig(int8_t direction, uint8_t * buff, uint8_t size)
{
	DMA_InitTypeDef DMA_InitStructure;
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1, ENABLE);
	
	DMA_InitStructure.DMA_BufferSize = size;
	DMA_InitStructure.DMA_Channel = DMA_Channel_1;
	DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Enable;
	DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_1QuarterFull;
	DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)buff;
	DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
	DMA_InitStructure.DMA_PeripheralBaseAddr = ((CPU_INT32U)0x40005410);
	DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
	DMA_InitStructure.DMA_Priority = DMA_Priority_Low;
	if (direction == 0){ 
		DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;
		DMA_DeInit(DMA1_Stream0);
		DMA_Init(DMA1_Stream0, &DMA_InitStructure);
	}else {
		DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToPeripheral;
		DMA_DeInit(DMA1_Stream6);
		DMA_Init(DMA1_Stream6, &DMA_InitStructure);
	}
}

void BSP_I2C1_SendBytesC(uint8_t dev_addr, uint8_t reg_addr, uint8_t *data, uint8_t len)
{
	CPU_INT08U err;
	OSSemPend(FlagI2C1_Lock,0,&err);
	DeviceAddr = dev_addr;
	RegAddr = reg_addr;
	
	I2C1_State = I2C_Transmit_Start;
	I2C1_Transfer_Mode = T_Command_Data;
	
	I2C_AcknowledgeConfig(I2C1, ENABLE);
	BSP_I2C1_DMA_Cofig(1, data, len);
	
	I2C_DMALastTransferCmd(I2C1, ENABLE);
	
	I2C_ITConfig(I2C1,I2C_IT_EVT, ENABLE);
	
	I2C_GenerateSTART(I2C1, ENABLE);
	
	OSSemPend(FlagI2C1_Transmitter,0,&err);
	
	TimeOut = 10000;
	while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED))
	{
		if (TimeOut-- == 0) 
			break;
	}
	I2C_SendData(I2C1, RegAddr);
	I2C_ITConfig(I2C1,I2C_IT_EVT, ENABLE);
	
	
	OSSemPend(FlagI2C1_Transmitter,0,&err);
	TimeOut = 10000;
	while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED))
	{
		if (TimeOut-- == 0)
			break;
	}
	DMA_ClearFlag(DMA1_Stream6, DMA_FLAG_TCIF6);
	DMA_Cmd(DMA1_Stream6, ENABLE);
	I2C_DMACmd(I2C1,ENABLE);
	
	DMA_ITConfig(DMA1_Stream6, DMA_IT_TC, ENABLE);
	
	OSSemPend(FlagI2C1_Transmitter,0,&err);
	
	TimeOut = 10000;
	while ((!I2C_GetFlagStatus(I2C1,I2C_FLAG_BTF)))  
  {
		if (TimeOut-- == 0) 
			while(1);
  }
  
  I2C_GenerateSTOP(I2C1, ENABLE);
	
	OSSemPost(FlagI2C1_Lock);
}

void BSP_I2C1_SendBytes(uint8_t dev_addr, uint8_t *data, uint8_t len)
{
	CPU_INT08U err;
	OSSemPend(FlagI2C1_Lock, 0, &err);
	I2C1_State = I2C_Transmit_Start;
	I2C1_Transfer_Mode = T_Data;
	
	DeviceAddr = dev_addr;
	
	I2C_AcknowledgeConfig(I2C1, ENABLE);
	BSP_I2C1_DMA_Cofig(1, data, len);
	
	I2C_DMALastTransferCmd(I2C1, ENABLE);
	
	I2C_ITConfig(I2C1,I2C_IT_EVT, ENABLE);
	
	I2C_GenerateSTART(I2C1, ENABLE);
	
	OSSemPend(FlagI2C1_Transmitter, 0, &err);
	TimeOut = 10000;
	while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED))
	{
		if (TimeOut-- == 0) 
			break;
	}
	DMA_ClearFlag(DMA1_Stream6, DMA_FLAG_TCIF6);
	DMA_Cmd(DMA1_Stream6, ENABLE);
	I2C_DMACmd(I2C1,ENABLE);
	
	DMA_ITConfig(DMA1_Stream6, DMA_IT_TC, ENABLE);
	OSSemPend(FlagI2C1_Transmitter,0, &err);
	
	TimeOut = 10000;
	while ((!I2C_GetFlagStatus(I2C1,I2C_FLAG_BTF)))  
  {
		if (TimeOut-- == 0) 
			while(1);
  }
  
  I2C_GenerateSTOP(I2C1, ENABLE);
	
	OSSemPost(FlagI2C1_Lock);
}


void BSP_I2C1_ReadBytesC(uint8_t dev_addr, uint8_t reg_addr, uint8_t *data, uint8_t len)
{
	CPU_INT08U err;
	
	OSSemPend(FlagI2C1_Lock, 0, &err);
	
	I2C1_State = I2C_Receive_Start;
	I2C1_Transfer_Mode = T_Command_Data;
	
	DeviceAddr = dev_addr;
	RegAddr = reg_addr;
	
	I2C_AcknowledgeConfig(I2C1, ENABLE);
	I2C_DMALastTransferCmd(I2C1, ENABLE);
	
	
	BSP_I2C1_DMA_Cofig(0, data, len);
	
	I2C_ITConfig(I2C1, I2C_IT_EVT, ENABLE);
	
	I2C_GenerateSTART(I2C1, ENABLE);
	
	OSSemPend(FlagI2C1_Receiver, 0, &err);
	
	TimeOut = 10000;
	while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED)) {
		if (TimeOut-- == 0)
			while(1);
  }
	I2C_SendData(I2C1, RegAddr);
	I2C_ITConfig(I2C1, I2C_IT_EVT, ENABLE);
	
	OSSemPend(FlagI2C1_Receiver, 0, &err);
	TimeOut = 10000;
	while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED)) {
		if (TimeOut-- == 0)
			while(1);
  }
	DMA_ClearFlag(DMA1_Stream0, DMA_IT_TC);
	I2C_DMACmd(I2C1, ENABLE);
	DMA_Cmd(DMA1_Stream0, ENABLE);
	
	DMA_ITConfig(DMA1_Stream0, DMA_IT_TC, ENABLE);
	
	OSSemPend(FlagI2C1_Receiver, 0, &err);	
	
  I2C_GenerateSTOP(I2C1, ENABLE);
	
	OSSemPost(FlagI2C1_Lock);
}
	
void BSP_I2C1_ReadBytes(uint8_t dev_addr, uint8_t *data, uint8_t len)
{
	CPU_INT08U err;
	
	OSSemPend(FlagI2C1_Lock, 0, &err);
	I2C1_State = I2C_Receive_Start;
	I2C1_Transfer_Mode = T_Data;
	
	DeviceAddr = dev_addr;
	I2C_AcknowledgeConfig(I2C1, DISABLE);
	I2C_DMALastTransferCmd(I2C1, ENABLE);
	
	BSP_I2C1_DMA_Cofig(0, data, len);
	
	I2C_ITConfig(I2C1, I2C_IT_EVT, ENABLE);
	
	I2C_GenerateSTART(I2C1, ENABLE);
	
	OSSemPend(FlagI2C1_Receiver,0, &err);
	
	TimeOut = 10000;
	while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED)) {
		if (TimeOut-- == 0)
			while(1);
	}
	
	I2C_DMACmd(I2C1, ENABLE);	
	DMA_Cmd(DMA1_Stream0, ENABLE);
	DMA_ITConfig(DMA1_Stream0, DMA_IT_TC, ENABLE);
	
	OSSemPend(FlagI2C1_Receiver, 0, &err);
	
	I2C_GenerateSTOP(I2C1, ENABLE);
	
	OSSemPost(FlagI2C1_Lock);
}

void I2C1TestTask(void *arg)
{
//	CPU_INT08U err;
	INT8U buffer[10] = {1,2,3,4,5,6,7,8,9,10};
	BSP_I2C1_Init();
	for(;;)
	{
		buffer[0] = 0x01;
		BSP_I2C1_SendBytes(0x70,&buffer[0], 1);
		OSTimeDly(1);
		buffer[0] = 0xff;
		BSP_I2C1_ReadBytes(0x70, &buffer[0], 1);
		buffer[0] = 0x03;
		OSTimeDly(1);
		BSP_I2C1_SendBytesC(0x29, 0xa0, buffer, 1);
		buffer[0] = 0xff;
		OSTimeDly(1);
		BSP_I2C1_ReadBytesC(0x29, 0xa0, buffer, 1);
		OSTimeDly(1);
	}
}

void I2C1TestTaskCreate(void)
{
	
	OSTaskCreateExt((void (*)(void*)) I2C1TestTask, 
									(void *) 0,
									(OS_STK*) &I2C1Test_TaskStk[I2C1Test_STK_SIZE-1],
									(INT8U) APP_TASK_I2C1Test_PRIO,
									(INT8U) APP_TASK_I2C1Test_PRIO,
									(OS_STK*) &I2C1Test_TaskStk[0],
									(INT32U) I2C1Test_STK_SIZE,
									(void *) 0,
									(INT16U) OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);
}

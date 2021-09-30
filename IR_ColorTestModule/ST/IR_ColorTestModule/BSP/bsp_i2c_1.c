#include "bsp_i2c_1.h"

/*********************************************************************************************************************
*																					Local define
*********************************************************************************************************************/

//**************************************  Hardware configuration  ****************************************************
#define BSP_I2C_1												I2C1
#define BSP_I2C_1_CLK										RCC_APB1Periph_I2C1

#define BSP_I2C_1_SCL_PIN								GPIO_Pin_6
#define BSP_I2C_1_SCL_GPIO_PORT					GPIOB
#define BSP_I2C_1_SCL_GPIO_CLK					RCC_AHB1Periph_GPIOB
#define BSP_I2C_1_SCL_SOURCE						GPIO_PinSource6
#define BSP_I2C_1_SCL_AF								GPIO_AF_I2C1


#define BSP_I2C_1_SDA_PIN								GPIO_Pin_7
#define BSP_I2C_1_SDA_GPIO_PORT					GPIOB
#define BSP_I2C_1_SDA_GPIO_CLK					RCC_AHB1Periph_GPIOB
#define BSP_I2C_1_SDA_SOURCE						GPIO_PinSource7
#define BSP_I2C_1_SDA_AF								GPIO_AF_I2C1

#define BSP_I2C_1_DR										((CPU_INT32U)0x40005410)



#define I2C_Rx_Start 								1
#define I2C_Rx_Start_Again					7
#define I2C_Rx_Addr 								2
#define I2C_Rx_Addr_Again						8
#define I2C_Rx_Data			 						3
#define I2C_Receive_Data_2					4
#define I2C_Receive_NA   						5
#define I2C_Receive_ED							6
#define I2C_Rx_Cmd									9
#define I2C_Rx_Stop									20

#define I2C_Tx_Start			  				10
#define I2C_Tx_Addr									11								
#define I2C_Transmit_Reg						12
#define I2C_Transmit_Reg_Start			13
#define I2C_Transmit_Addr_Again			14
#define I2C_Tx_Cmd									15
#define I2C_Tx_Data									16
#define I2C_Tx_Stop									17

#define T_Command_Data 	1
#define T_Data				2



/*********************************************************************************************************************
* 																							Local Variable
*********************************************************************************************************************/
int8_t 		I2C_1_State = 0;
int8_t 		I2C_1_Transfer_Mode;

uint8_t 	I2C_1_DeviceAddr;
int8_t 		I2C_1_RegAddr;

volatile uint8_t 	*I2C_1_RxBuffer;
volatile int8_t 		I2C_1_RxLength;
volatile int8_t 		I2C_1_RxCounter;

volatile uint8_t 	*I2C_1_TxData;
volatile uint8_t   I2C_1_TxLength;
volatile uint8_t   I2C_1_TxCounter;

volatile uint8_t 	I2C_1_cmd;

volatile uint32_t 	I2C_1_rate = 100000;
volatile uint8_t 		I2C_1_stop_counter = 0;
uint16_t BSP_I2C_1_err;
//int32_t 	I2C_1_TimeOut;


OS_EVENT *I2C_1_TxWait;
OS_EVENT *I2C_1_RxWait;
OS_EVENT *I2C_1_Lock;

/*********************************************************************************************************************
* 																							Local function prototype
*********************************************************************************************************************/
void BSP_I2C_1_ER_Callback(uint16_t err_code);
void BSP_I2C_1_ER_ISR_Handler(void);
void BSP_I2C_1_ISR_Handler(void);


void I2C_1_PeripheralInit(void)
{
	I2C_InitTypeDef I2C_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB1PeriphClockCmd(BSP_I2C_1_CLK, ENABLE);
	
	RCC_AHB1PeriphClockCmd(BSP_I2C_1_SDA_GPIO_CLK, ENABLE);
	RCC_AHB1PeriphClockCmd(BSP_I2C_1_SCL_GPIO_CLK, ENABLE);
	
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);  
	
	/* Reset I2Cx IP */
	RCC_APB1PeriphResetCmd(BSP_I2C_1_CLK, ENABLE);
	/* Release reset signal of I2Cx IP */
  RCC_APB1PeriphResetCmd(BSP_I2C_1_CLK, DISABLE);
	
	GPIO_InitStructure.GPIO_Pin = BSP_I2C_1_SCL_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(BSP_I2C_1_SCL_GPIO_PORT, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = BSP_I2C_1_SDA_PIN;
	GPIO_Init(BSP_I2C_1_SDA_GPIO_PORT, &GPIO_InitStructure);
	
	
	GPIO_PinAFConfig(BSP_I2C_1_SCL_GPIO_PORT, BSP_I2C_1_SCL_SOURCE, BSP_I2C_1_SCL_AF);
	GPIO_PinAFConfig(BSP_I2C_1_SDA_GPIO_PORT, BSP_I2C_1_SDA_SOURCE, BSP_I2C_1_SDA_AF);
	
	I2C_DeInit(BSP_I2C_1);
	
	I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;
	I2C_InitStructure.I2C_DutyCycle = I2C_DutyCycle_2;
	I2C_InitStructure.I2C_OwnAddress1 = 0xA0;
	I2C_InitStructure.I2C_Ack = I2C_Ack_Enable;
	I2C_InitStructure.I2C_ClockSpeed = I2C_1_rate;
	
	I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
	
	I2C_Init(BSP_I2C_1, &I2C_InitStructure);
	
	
	
	I2C_ITConfig(BSP_I2C_1, I2C_IT_ERR , ENABLE);
	
	I2C_Cmd(BSP_I2C_1, ENABLE);
	
}

void I2C_1_Initialization(void)
{
	I2C_InitTypeDef I2C_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
	
	I2C_1_TxWait = OSSemCreate(0);
	I2C_1_RxWait = OSSemCreate(0);
	I2C_1_Lock = OSSemCreate(1);
	
	
	RCC_APB1PeriphClockCmd(BSP_I2C_1_CLK, ENABLE);
	
	RCC_AHB1PeriphClockCmd(BSP_I2C_1_SDA_GPIO_CLK, ENABLE);
	RCC_AHB1PeriphClockCmd(BSP_I2C_1_SCL_GPIO_CLK , ENABLE);
	
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);  
	
	/* Reset I2Cx IP */
	RCC_APB1PeriphResetCmd(BSP_I2C_1_CLK, ENABLE);
	/* Release reset signal of I2Cx IP */
  RCC_APB1PeriphResetCmd(BSP_I2C_1_CLK, DISABLE);
	
	GPIO_InitStructure.GPIO_Pin = BSP_I2C_1_SCL_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;//GPIO_PuPd_NOPULL;
	GPIO_Init(BSP_I2C_1_SCL_GPIO_PORT, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = BSP_I2C_1_SDA_PIN;
	GPIO_Init(BSP_I2C_1_SDA_GPIO_PORT, &GPIO_InitStructure);
	
	
	GPIO_PinAFConfig(BSP_I2C_1_SCL_GPIO_PORT, BSP_I2C_1_SCL_SOURCE, BSP_I2C_1_SCL_AF);
	GPIO_PinAFConfig(BSP_I2C_1_SDA_GPIO_PORT, BSP_I2C_1_SDA_SOURCE, BSP_I2C_1_SDA_AF);
	
	I2C_DeInit(BSP_I2C_1);
	
	I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;
	I2C_InitStructure.I2C_DutyCycle = I2C_DutyCycle_2;
	I2C_InitStructure.I2C_OwnAddress1 = 0xA0;
	I2C_InitStructure.I2C_Ack = I2C_Ack_Enable;
	I2C_InitStructure.I2C_ClockSpeed = I2C_1_rate;
	
	I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
	
	I2C_Init(BSP_I2C_1, &I2C_InitStructure);
	
	
	
	I2C_ITConfig(BSP_I2C_1, I2C_IT_ERR , ENABLE);
	
	I2C_Cmd(BSP_I2C_1, ENABLE);
	
	BSP_IntVectSet(BSP_INT_ID_I2C1_EV, BSP_I2C_1_ISR_Handler);
  BSP_IntEn(BSP_INT_ID_I2C1_EV);
	 
	BSP_IntVectSet(BSP_INT_ID_I2C1_ER, BSP_I2C_1_ER_ISR_Handler); 
  BSP_IntEn(BSP_INT_ID_I2C1_ER);
	
//	I2C_1_Write(0x01, "123", 3);
	
}


void I2C_1_InitializationBitRate(CPU_INT32U BitRate)
{
	I2C_InitTypeDef I2C_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
	
	I2C_1_rate = BitRate;
	
	RCC_APB1PeriphClockCmd(BSP_I2C_1_CLK, ENABLE);
	
	RCC_AHB1PeriphClockCmd(BSP_I2C_1_SDA_GPIO_CLK, ENABLE);
	RCC_AHB1PeriphClockCmd(BSP_I2C_1_SCL_GPIO_CLK , ENABLE);
	
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);  
	
	/* Reset I2Cx IP */
	RCC_APB1PeriphResetCmd(BSP_I2C_1_CLK, ENABLE);
	/* Release reset signal of I2Cx IP */
  RCC_APB1PeriphResetCmd(BSP_I2C_1_CLK, DISABLE);
	
	GPIO_InitStructure.GPIO_Pin = BSP_I2C_1_SCL_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(BSP_I2C_1_SCL_GPIO_PORT, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = BSP_I2C_1_SDA_PIN;
	GPIO_Init(BSP_I2C_1_SDA_GPIO_PORT, &GPIO_InitStructure);
	
	
	GPIO_PinAFConfig(BSP_I2C_1_SCL_GPIO_PORT, BSP_I2C_1_SCL_SOURCE, BSP_I2C_1_SCL_AF);
	GPIO_PinAFConfig(BSP_I2C_1_SDA_GPIO_PORT, BSP_I2C_1_SDA_SOURCE, BSP_I2C_1_SDA_AF);
	
	I2C_DeInit(BSP_I2C_1);
	
	I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;
	I2C_InitStructure.I2C_DutyCycle = I2C_DutyCycle_2;
	I2C_InitStructure.I2C_OwnAddress1 = 0xA0;
	I2C_InitStructure.I2C_Ack = I2C_Ack_Enable;
	I2C_InitStructure.I2C_ClockSpeed = I2C_1_rate;
	
	I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
	
	I2C_Init(BSP_I2C_1, &I2C_InitStructure);
	
	
	
	I2C_ITConfig(BSP_I2C_1, I2C_IT_ERR , ENABLE);
	
	I2C_Cmd(BSP_I2C_1, ENABLE);
}
CPU_INT08U I2C_1_Write(CPU_INT08U address, CPU_INT08U *dataP, CPU_INT08U dataLength)
{
	CPU_INT08U  err;
	
	I2C_1_PeripheralInit();
	
	OSSemPend(I2C_1_Lock, 0, &err);
	
	I2C_1_Transfer_Mode = T_Data;
	I2C_1_DeviceAddr = address;
	I2C_1_TxData = dataP;
	I2C_1_TxLength = dataLength;
	
	I2C_1_State = I2C_Tx_Start;
	I2C_1_TxCounter = 0;
	
	I2C_AcknowledgeConfig(BSP_I2C_1, ENABLE);
	
	I2C_ITConfig(BSP_I2C_1,I2C_IT_EVT | I2C_IT_BUF, ENABLE);//
	I2C_GenerateSTART(BSP_I2C_1, ENABLE);
	
	OSSemPend(I2C_1_TxWait, 0, &err);
	
	OSSemPost(I2C_1_Lock);
	
	
	return err | BSP_I2C_1_err;
}
CPU_INT08U I2C_1_Read(CPU_INT08U address, CPU_INT08U *dataP, CPU_INT08U dataLength)
{
	CPU_INT08U err;
	I2C_1_PeripheralInit();
	OSSemPend(I2C_1_Lock, 0, &err);
	
	I2C_1_Transfer_Mode = T_Data;
	I2C_1_DeviceAddr = address;
	I2C_1_RxBuffer = dataP;
	I2C_1_RxLength = dataLength;
	
	I2C_1_State = I2C_Rx_Start;
	I2C_1_RxCounter = 0;
	
	I2C_AcknowledgeConfig(BSP_I2C_1, ENABLE);
	
	I2C_ITConfig(BSP_I2C_1,I2C_IT_EVT, ENABLE);//|I2C_IT_BUF
	I2C_GenerateSTART(BSP_I2C_1, ENABLE);
	
	OSSemPend(I2C_1_RxWait, 0, &err);
	
	OSSemPost(I2C_1_Lock);
	
	return err | BSP_I2C_1_err;
}
CPU_INT08U I2C_1_CmdRead(CPU_INT08U address,CPU_INT08U command, CPU_INT08U *dataP, CPU_INT08U dataLength)
{
	CPU_INT08U err;
	I2C_1_PeripheralInit();
	OSSemPend(I2C_1_Lock, 0, &err);
	
	I2C_1_Transfer_Mode = T_Command_Data;
	I2C_1_DeviceAddr = address;
	I2C_1_cmd = command;
	I2C_1_RxBuffer = dataP;
	I2C_1_RxLength = dataLength;
	
	I2C_1_State = I2C_Rx_Start;
	I2C_1_RxCounter = 0;
	
	I2C_1_stop_counter = 0;
	
	I2C_AcknowledgeConfig(BSP_I2C_1, ENABLE);
	
	I2C_ITConfig(BSP_I2C_1,I2C_IT_EVT, ENABLE);//|I2C_IT_BUF
	I2C_GenerateSTART(BSP_I2C_1, ENABLE);
	
	OSSemPend(I2C_1_RxWait, 2000, &err);
	
	OSSemPost(I2C_1_Lock);
	
	return err | BSP_I2C_1_err;

}
CPU_INT08U I2C_1_CmdWrite(CPU_INT08U address,CPU_INT08U command, CPU_INT08U *dataP, CPU_INT08U dataLength)
{
	CPU_INT08U err;
	
	I2C_1_PeripheralInit();
	OSSemPend(I2C_1_Lock, 0, &err);
	
	I2C_1_Transfer_Mode = T_Command_Data;
	I2C_1_DeviceAddr = address;
	I2C_1_cmd = command;
	I2C_1_TxData = dataP;
	I2C_1_TxLength = dataLength;
	
	I2C_1_State = I2C_Tx_Start;
	I2C_1_TxCounter = 0;
	
	I2C_AcknowledgeConfig(BSP_I2C_1, ENABLE);
	
	I2C_ITConfig(BSP_I2C_1,I2C_IT_EVT, ENABLE);
	I2C_GenerateSTART(BSP_I2C_1, ENABLE);
	
	OSSemPend(I2C_1_TxWait, 0, &err);
	
	OSSemPost(I2C_1_Lock);
	
	
	return err | BSP_I2C_1_err;
}

uint32_t I2C_1_status;
uint32_t I2C_1_test_counter;
void BSP_I2C_1_ISR_Handler(void)
{
	/*Judge mode of transmite or receive*/
//	CPU_INT32U I2C1_SR1;
	
	switch(I2C_1_State) {
		case I2C_Rx_Start:
			if (I2C_1_Transfer_Mode == T_Command_Data) {
				if (I2C_GetLastEvent(BSP_I2C_1) == I2C_EVENT_MASTER_MODE_SELECT 	// EV5
					/*|| I2C_GetITStatus(BSP_I2C_1, I2C_IT_SB) == SET*/) {
					I2C_Send7bitAddress(BSP_I2C_1, (I2C_1_DeviceAddr << 1), I2C_Direction_Transmitter);
					I2C_1_State = I2C_Rx_Addr;
				}else {
					BSP_I2C_1_ER_Callback(!0);
				}
			} else if(	I2C_1_Transfer_Mode == T_Data) {
				if (I2C_GetLastEvent(BSP_I2C_1) == I2C_EVENT_MASTER_MODE_SELECT    //EV5
					/*|| I2C_GetITStatus(BSP_I2C_1, I2C_IT_SB) == SET*/) {
					I2C_Send7bitAddress(BSP_I2C_1, (I2C_1_DeviceAddr << 1), I2C_Direction_Receiver);
					I2C_1_State = I2C_Rx_Addr;
				}else {
					BSP_I2C_1_ER_Callback(!0);
				}
			}else {
				BSP_I2C_1_ER_Callback(!0);
			}
		break;
		case I2C_Rx_Start_Again:
			I2C_1_test_counter = 0;
			while(I2C_GetLastEvent(BSP_I2C_1) != I2C_EVENT_MASTER_MODE_SELECT) I2C_1_test_counter++;
			if (I2C_GetLastEvent(BSP_I2C_1) == I2C_EVENT_MASTER_MODE_SELECT 	// EV5
				/*|| I2C_GetITStatus(BSP_I2C_1, I2C_IT_SB) == SET*/) {
				I2C_Send7bitAddress(BSP_I2C_1, (I2C_1_DeviceAddr << 1), I2C_Direction_Receiver);
				I2C_1_State = I2C_Rx_Addr_Again;
			}else {
				BSP_I2C_1_ER_Callback(!0);
			}
		
			break;
		case I2C_Rx_Addr:
			if (I2C_1_Transfer_Mode == T_Command_Data) {
				if(I2C_GetLastEvent(BSP_I2C_1) == I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED 	    // EV6
					/*|| I2C_GetITStatus(BSP_I2C_1, I2C_IT_ADDR) == SET*/) {
					
					I2C_SendData(BSP_I2C_1, I2C_1_cmd);
					I2C_1_State = I2C_Rx_Cmd;
				}else {
					BSP_I2C_1_ER_Callback(!0);
				}	
			}else if (I2C_1_Transfer_Mode == T_Data) {
				if (I2C_GetLastEvent(BSP_I2C_1) == I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED 	    // EV6
					/*|| I2C_GetITStatus(BSP_I2C_1, I2C_IT_ADDR) == SET*/) {
					I2C_ITConfig(BSP_I2C_1, I2C_IT_BUF, ENABLE);
					I2C_1_State = I2C_Rx_Data;
				}else {
					BSP_I2C_1_ER_Callback(!0);
				}
			}
			break;
		case I2C_Rx_Cmd:
			if (I2C_GetLastEvent(BSP_I2C_1) == I2C_EVENT_MASTER_BYTE_TRANSMITTED 	    // EV8_2
				/*|| I2C_GetITStatus(BSP_I2C_1, I2C_IT_BTF) == SET*/)  {
				I2C_GenerateSTART(BSP_I2C_1, ENABLE);
				I2C_1_State = I2C_Rx_Start_Again;
			}else {
				BSP_I2C_1_ER_Callback(!0);
			}
			break;
		case I2C_Rx_Addr_Again:
			I2C_1_test_counter = 0;
			while((I2C_GetLastEvent(BSP_I2C_1) & I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED) != I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED) I2C_1_test_counter++;
//			if (I2C_GetLastEvent(BSP_I2C_1) == I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED      // EV6
//					|| I2C_GetITStatus(BSP_I2C_1, I2C_IT_ADDR) == SET/**/) {
			if (I2C_1_test_counter < 0x1000){		
					I2C_ITConfig(BSP_I2C_1, I2C_IT_BUF, ENABLE);
					//I2C_1_RxBuffer[I2C_1_RxCounter] = I2C_ReceiveData(BSP_I2C_1);
					I2C_1_State = I2C_Rx_Data;
			}else {
				BSP_I2C_1_ER_Callback(!0);
			}
			break;
		case I2C_Rx_Data:
			I2C_1_test_counter = 0;
			while((I2C_GetLastEvent(BSP_I2C_1)& I2C_EVENT_MASTER_BYTE_RECEIVED) != I2C_EVENT_MASTER_BYTE_RECEIVED) I2C_1_test_counter++;
			if (I2C_GetLastEvent(BSP_I2C_1) == I2C_EVENT_MASTER_BYTE_RECEIVED          // EV7
				||I2C_GetITStatus(BSP_I2C_1, I2C_IT_RXNE)/**/) {
					
				I2C_1_RxBuffer[I2C_1_RxCounter++] = I2C_ReceiveData(BSP_I2C_1);
				if (I2C_1_RxCounter < I2C_1_RxLength) {
					//I2C_1_RxBuffer[I2C_1_RxCounter++] = I2C_ReceiveData(BSP_I2C_1);
				}else {
					I2C_GenerateSTOP(BSP_I2C_1, ENABLE);
					//I2C_ITConfig(BSP_I2C_1, I2C_IT_BUF | I2C_IT_EVT, DISABLE);
					I2C_ITConfig(BSP_I2C_1, I2C_IT_BUF, DISABLE);
					//OSSemPost(I2C_1_RxWait);
					I2C_1_State = I2C_Rx_Stop;
					BSP_I2C_1_err = 0;
				}
			}else {
				BSP_I2C_1_ER_Callback(!0);
			}
			break;
		case I2C_Rx_Stop:
			I2C_1_test_counter = 0;
			while((I2C_GetLastEvent(BSP_I2C_1)& I2C_EVENT_MASTER_BYTE_RECEIVED) != I2C_EVENT_MASTER_BYTE_RECEIVED) I2C_1_test_counter++;
			if (I2C_GetLastEvent(BSP_I2C_1) == I2C_EVENT_MASTER_BYTE_RECEIVED          // EV7
				/*||I2C_GetITStatus(BSP_I2C_1, I2C_IT_RXNE)*/) {
					
					I2C_ITConfig(BSP_I2C_1, I2C_IT_BUF | I2C_IT_EVT, DISABLE);
					OSSemPost(I2C_1_RxWait);
					I2C_1_stop_counter++;
					if (I2C_1_stop_counter == 2)
					BSP_I2C_1_err = 0;
				
			}else {
				BSP_I2C_1_ER_Callback(!0);
			}
			break;
		case I2C_Tx_Start:
			if(I2C_GetLastEvent(BSP_I2C_1) == I2C_EVENT_MASTER_MODE_SELECT   // EV5
				/*|| I2C_GetITStatus(BSP_I2C_1, I2C_IT_SB) == SET*/) {
				//I2C_ClearITPendingBit();
				I2C_Send7bitAddress(BSP_I2C_1, (I2C_1_DeviceAddr << 1), I2C_Direction_Transmitter);
				
				I2C_1_State = I2C_Tx_Addr;
			}else {
				BSP_I2C_1_ER_Callback(!0);
			}
			break;
		case I2C_Tx_Addr:
			if (I2C_1_Transfer_Mode == T_Command_Data) {
				if(I2C_GetLastEvent(BSP_I2C_1) == I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED 					// EV6
					/*||I2C_GetITStatus(BSP_I2C_1, I2C_IT_ADDR) == SET*/) {
					
					//I2C_ITConfig(BSP_I2C_1, I2C_IT_EVT, DISABLE);
					I2C_SendData(BSP_I2C_1, I2C_1_cmd);
					//OSSemPost(FlagBSP_I2C_1_Transmitter);
					I2C_1_State = I2C_Tx_Data;
					
					I2C_ITConfig(BSP_I2C_1, I2C_IT_BUF, ENABLE);
				}
			}else if (I2C_1_Transfer_Mode == T_Data){ 
					if(I2C_GetLastEvent(BSP_I2C_1) == I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED 				// EV6
						/*|| I2C_GetITStatus(BSP_I2C_1, I2C_IT_ADDR) == SET*/) {
						//I2C_ITConfig(BSP_I2C_1, I2C_IT_EVT, DISABLE);
						
						//OSSemPost(FlagI2C1_Transmitter);
						I2C_ITConfig(BSP_I2C_1, I2C_IT_BUF, ENABLE);
						I2C_SendData(BSP_I2C_1, I2C_1_TxData[I2C_1_TxCounter]);
						
						I2C_1_TxCounter++;
						I2C_1_State = I2C_Tx_Data;
						
						
					}
			}else{
				BSP_I2C_1_ER_Callback(!0);
			}
			break;	
			
		case I2C_Tx_Data:
			
		  I2C_1_status = I2C_GetLastEvent(BSP_I2C_1);
			if ( I2C_1_status == I2C_EVENT_MASTER_BYTE_TRANSMITTING //||   //          EV8
					//I2C_GetITStatus(BSP_I2C_1, I2C_IT_TXE) == SET
					) {
				//I2C_ITConfig(BSP_I2C_1, I2C_IT_EVT, DISABLE);
				
				//OSSemPost(FlagI2C1_Transmitter);
				
				if (I2C_1_TxCounter < I2C_1_TxLength) {
					I2C_SendData(BSP_I2C_1, I2C_1_TxData[I2C_1_TxCounter]);
					I2C_1_TxCounter++;
					I2C_1_State = I2C_Tx_Data;
				}else {
					I2C_ITConfig(BSP_I2C_1, I2C_IT_BUF, DISABLE);
					I2C_1_State = I2C_Tx_Data;
				}
				
			}else if (I2C_1_status == I2C_EVENT_MASTER_BYTE_TRANSMITTED 
								/*||I2C_GetITStatus(BSP_I2C_1, I2C_IT_BTF) == SET*/) {
				
				I2C_GenerateSTOP(BSP_I2C_1, ENABLE);
				I2C_ITConfig(BSP_I2C_1, I2C_IT_BUF | I2C_IT_EVT, DISABLE);
				OSSemPost(I2C_1_TxWait);
				BSP_I2C_1_err = 0;
			}else{
				BSP_I2C_1_ER_Callback(!0);
			}
			break;
			
		default:
			break; 
	}



}

void BSP_I2C_1_ER_ISR_Handler(void)
{
	
	BSP_I2C_1_err = I2C_ReadRegister(BSP_I2C_1, I2C_Register_SR1) & 0xFF00;
	if ( BSP_I2C_1_err  != 0x00) {
		BSP_I2C_1->SR1 &= 0x00FF;
	} 
	
	BSP_I2C_1_err = !0;
	I2C_ITConfig(BSP_I2C_1, I2C_IT_EVT, DISABLE);
	if (I2C_1_TxWait->OSEventGrp != 0)
		OSSemPost(I2C_1_TxWait);
	if (I2C_1_RxWait->OSEventGrp != 0)
		OSSemPost(I2C_1_RxWait);
	
	I2C_1_PeripheralInit();
}

void BSP_I2C_1_ER_Callback(uint16_t err_code)
{
	BSP_I2C_1_err = err_code;
	I2C_ITConfig(BSP_I2C_1, I2C_IT_BUF | I2C_IT_EVT, DISABLE);
	if (I2C_1_TxWait->OSEventGrp != 0)
		OSSemPost(I2C_1_TxWait);
	if (I2C_1_RxWait->OSEventGrp != 0)
		OSSemPost(I2C_1_RxWait);
	
	I2C_1_PeripheralInit();

}

#include "bsp_i2c_2.h"


/*********************************************************************************************************************
*																					Local define
*********************************************************************************************************************/

//**************************************  Hardware configuration  ****************************************************
#define BSP_I2C_2												I2C2
#define BSP_I2C_2_CLK										RCC_APB1Periph_I2C2

#define BSP_I2C_2_SCL_PIN								GPIO_Pin_10
#define BSP_I2C_2_SCL_GPIO_PORT					GPIOB
#define BSP_I2C_2_SCL_GPIO_CLK					RCC_AHB1Periph_GPIOB
#define BSP_I2C_2_SCL_SOURCE						GPIO_PinSource10
#define BSP_I2C_2_SCL_AF								GPIO_AF_I2C2


#define BSP_I2C_2_SDA_PIN								GPIO_Pin_11
#define BSP_I2C_2_SDA_GPIO_PORT					GPIOB
#define BSP_I2C_2_SDA_GPIO_CLK					RCC_AHB1Periph_GPIOB
#define BSP_I2C_2_SDA_SOURCE						GPIO_PinSource11
#define BSP_I2C_2_SDA_AF								GPIO_AF_I2C2

//define BSP_I2C_2_DR										((CPU_INT32U)0x40005C10)

//************************************** I2C run state ****************************************************************************


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
int8_t 		I2C_2_State = 0;
int8_t 		I2C_2_Transfer_Mode;

uint8_t 	I2C_2_DeviceAddr;
int8_t 		I2C_2_RegAddr;

volatile uint8_t 	*I2C_2_RxBuffer;
volatile int8_t 		I2C_2_RxLength;
volatile int8_t 		I2C_2_RxCounter;

volatile uint8_t 	*I2C_2_TxData;
volatile uint8_t   I2C_2_TxLength;
volatile uint8_t   I2C_2_TxCounter;

volatile uint8_t 	I2C_2_cmd;

volatile uint32_t 	I2C_2_rate = 100000;
volatile uint8_t 		I2C_stop_counter = 0;
uint16_t BSP_I2C_2_err;
//int32_t 	I2C_2_TimeOut;


OS_EVENT *I2C_2_TxWait;
OS_EVENT *I2C_2_RxWait;
OS_EVENT *I2C_2_Lock;


volatile uint8_t bsp_i2c_2_work_mode;

#define WORK_MODE_CMD_READ 			1
#define WORK_MODE_READ					2
#define WORK_MODE_CMD_SEND			3
#define WORK_MODE_SEND					4


/*********************************************************************************************************************
* 																							Local function prototype
*********************************************************************************************************************/
void BSP_I2C_2_ER_Callback(uint16_t err_code);
void BSP_I2C_2_ER_ISR_Handler(void);
void BSP_I2C_2_ISR_Handler(void);


void I2C_2_PeripheralInit(void)
{
	I2C_InitTypeDef I2C_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB1PeriphClockCmd(BSP_I2C_2_CLK, ENABLE);
	
	RCC_AHB1PeriphClockCmd(BSP_I2C_2_SDA_GPIO_CLK, ENABLE);
	RCC_AHB1PeriphClockCmd(BSP_I2C_2_SCL_GPIO_CLK, ENABLE);
	
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);  
	
	/* Reset I2Cx IP */
	RCC_APB1PeriphResetCmd(BSP_I2C_2_CLK, ENABLE);
	/* Release reset signal of I2Cx IP */
  RCC_APB1PeriphResetCmd(BSP_I2C_2_CLK, DISABLE);
	
	GPIO_InitStructure.GPIO_Pin = BSP_I2C_2_SCL_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(BSP_I2C_2_SCL_GPIO_PORT, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = BSP_I2C_2_SDA_PIN;
	GPIO_Init(BSP_I2C_2_SDA_GPIO_PORT, &GPIO_InitStructure);
	
	
	GPIO_PinAFConfig(BSP_I2C_2_SCL_GPIO_PORT, BSP_I2C_2_SCL_SOURCE, BSP_I2C_2_SCL_AF);
	GPIO_PinAFConfig(BSP_I2C_2_SDA_GPIO_PORT, BSP_I2C_2_SDA_SOURCE, BSP_I2C_2_SDA_AF);
	
	I2C_DeInit(BSP_I2C_2);
	
	I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;
	I2C_InitStructure.I2C_DutyCycle = I2C_DutyCycle_2;
	I2C_InitStructure.I2C_OwnAddress1 = 0xA0;
	I2C_InitStructure.I2C_Ack = I2C_Ack_Enable;
	I2C_InitStructure.I2C_ClockSpeed = I2C_2_rate;
	
	I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
	
	I2C_Init(BSP_I2C_2, &I2C_InitStructure);
	
	
	
	I2C_ITConfig(BSP_I2C_2, I2C_IT_ERR , ENABLE);
	
	I2C_Cmd(BSP_I2C_2, ENABLE);
	
}

void I2C_2_Initialization(void)
{
	I2C_InitTypeDef I2C_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
		
		
	I2C_2_TxWait = OSSemCreate(0);
	I2C_2_RxWait = OSSemCreate(0);
	I2C_2_Lock = OSSemCreate(1);
	
	
	RCC_APB1PeriphClockCmd(BSP_I2C_2_CLK, ENABLE);
	
	RCC_AHB1PeriphClockCmd(BSP_I2C_2_SDA_GPIO_CLK, ENABLE);
	RCC_AHB1PeriphClockCmd(BSP_I2C_2_SCL_GPIO_CLK , ENABLE);
	
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);  
	
	/* Reset I2Cx IP */
	RCC_APB1PeriphResetCmd(BSP_I2C_2_CLK, ENABLE);
	/* Release reset signal of I2Cx IP */
  RCC_APB1PeriphResetCmd(BSP_I2C_2_CLK, DISABLE);
	
	GPIO_InitStructure.GPIO_Pin = BSP_I2C_2_SCL_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;//GPIO_PuPd_NOPULL;
	GPIO_Init(BSP_I2C_2_SCL_GPIO_PORT, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = BSP_I2C_2_SDA_PIN;
	GPIO_Init(BSP_I2C_2_SDA_GPIO_PORT, &GPIO_InitStructure);
	
	
	GPIO_PinAFConfig(BSP_I2C_2_SCL_GPIO_PORT, BSP_I2C_2_SCL_SOURCE, BSP_I2C_2_SCL_AF);
	GPIO_PinAFConfig(BSP_I2C_2_SDA_GPIO_PORT, BSP_I2C_2_SDA_SOURCE, BSP_I2C_2_SDA_AF);
	
	I2C_DeInit(BSP_I2C_2);
	
	I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;
	I2C_InitStructure.I2C_DutyCycle = I2C_DutyCycle_2;
	I2C_InitStructure.I2C_OwnAddress1 = 0xA0;
	I2C_InitStructure.I2C_Ack = I2C_Ack_Enable;
	I2C_InitStructure.I2C_ClockSpeed = I2C_2_rate;
	
	I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
	
	I2C_Init(BSP_I2C_2, &I2C_InitStructure);
	
	
	
	NVIC_InitStructure.NVIC_IRQChannel = I2C2_EV_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x3;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x03;
	NVIC_Init(&NVIC_InitStructure);
	
	I2C_ITConfig(BSP_I2C_2, I2C_IT_ERR , ENABLE);
	
	I2C_Cmd(BSP_I2C_2, ENABLE);
	
	BSP_IntVectSet(BSP_INT_ID_I2C2_EV, BSP_I2C_2_ISR_Handler);
  BSP_IntEn(BSP_INT_ID_I2C2_EV);
	 
	BSP_IntVectSet(BSP_INT_ID_I2C2_ER, BSP_I2C_2_ER_ISR_Handler); 
  BSP_IntEn(BSP_INT_ID_I2C2_ER);
	
//	I2C_2_Write(0x01, "123", 3);
	
}


void I2C_2_InitializationBitRate(CPU_INT32U BitRate)
{
	I2C_InitTypeDef I2C_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
	
	I2C_2_rate = BitRate;
	
	RCC_APB1PeriphClockCmd(BSP_I2C_2_CLK, ENABLE);
	
	RCC_AHB1PeriphClockCmd(BSP_I2C_2_SDA_GPIO_CLK, ENABLE);
	RCC_AHB1PeriphClockCmd(BSP_I2C_2_SCL_GPIO_CLK , ENABLE);
	
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);  
	
	/* Reset I2Cx IP */
	RCC_APB1PeriphResetCmd(BSP_I2C_2_CLK, ENABLE);
	/* Release reset signal of I2Cx IP */
  RCC_APB1PeriphResetCmd(BSP_I2C_2_CLK, DISABLE);
	
	GPIO_InitStructure.GPIO_Pin = BSP_I2C_2_SCL_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(BSP_I2C_2_SCL_GPIO_PORT, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = BSP_I2C_2_SDA_PIN;
	GPIO_Init(BSP_I2C_2_SDA_GPIO_PORT, &GPIO_InitStructure);
	
	
	GPIO_PinAFConfig(BSP_I2C_2_SCL_GPIO_PORT, BSP_I2C_2_SCL_SOURCE, BSP_I2C_2_SCL_AF);
	GPIO_PinAFConfig(BSP_I2C_2_SDA_GPIO_PORT, BSP_I2C_2_SDA_SOURCE, BSP_I2C_2_SDA_AF);
	
	I2C_DeInit(BSP_I2C_2);
	
	I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;
	I2C_InitStructure.I2C_DutyCycle = I2C_DutyCycle_2;
	I2C_InitStructure.I2C_OwnAddress1 = 0xA0;
	I2C_InitStructure.I2C_Ack = I2C_Ack_Enable;
	I2C_InitStructure.I2C_ClockSpeed = I2C_2_rate;
	
	I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
	
	I2C_Init(BSP_I2C_2, &I2C_InitStructure);
	
	
	
	I2C_ITConfig(BSP_I2C_2, I2C_IT_ERR , ENABLE);
	
	I2C_Cmd(BSP_I2C_2, ENABLE);
}
CPU_INT08U I2C_2_Write(CPU_INT08U address, CPU_INT08U *dataP, CPU_INT08U dataLength)
{
	CPU_INT08U  err;
	
	I2C_2_PeripheralInit();
	
	OSSemPend(I2C_2_Lock, 0, &err);
	
	I2C_2_Transfer_Mode = T_Data;
	I2C_2_DeviceAddr = address;
	I2C_2_TxData = dataP;
	I2C_2_TxLength = dataLength;
	
	I2C_2_State = I2C_Tx_Start;
	I2C_2_TxCounter = 0;
	
	I2C_AcknowledgeConfig(BSP_I2C_2, ENABLE);
	
	I2C_ITConfig(BSP_I2C_2,I2C_IT_EVT | I2C_IT_BUF, ENABLE);//
	I2C_GenerateSTART(BSP_I2C_2, ENABLE);
	
	OSSemPend(I2C_2_TxWait, 0, &err);
	
	OSSemPost(I2C_2_Lock);
	
	
	return err | BSP_I2C_2_err;
}
CPU_INT08U I2C_2_Read(CPU_INT08U address, CPU_INT08U *dataP, CPU_INT08U dataLength)
{
	CPU_INT08U err;
	I2C_2_PeripheralInit();
	OSSemPend(I2C_2_Lock, 0, &err);
	
	I2C_2_Transfer_Mode = T_Data;
	I2C_2_DeviceAddr = address;
	I2C_2_RxBuffer = dataP;
	I2C_2_RxLength = dataLength;
	
	I2C_2_State = I2C_Rx_Start;
	I2C_2_RxCounter = 0;
	
	I2C_AcknowledgeConfig(BSP_I2C_2, ENABLE);
	
	I2C_ITConfig(BSP_I2C_2,I2C_IT_EVT, ENABLE);//|I2C_IT_BUF
	I2C_GenerateSTART(BSP_I2C_2, ENABLE);
	
	OSSemPend(I2C_2_RxWait, 0, &err);
	
	OSSemPost(I2C_2_Lock);
	
	return err | BSP_I2C_2_err;
}
CPU_INT08U I2C_2_CmdRead(CPU_INT08U address,CPU_INT08U command, CPU_INT08U *dataP, CPU_INT08U dataLength)
{
	CPU_INT08U err;
	I2C_2_PeripheralInit();
	OSSemPend(I2C_2_Lock, 0, &err);
	
	I2C_2_Transfer_Mode = T_Command_Data;
	I2C_2_DeviceAddr = address;
	I2C_2_cmd = command;
	I2C_2_RxBuffer = dataP;
	I2C_2_RxLength = dataLength;
	
	I2C_2_State = I2C_Rx_Start;
	I2C_2_RxCounter = 0;
	
	I2C_stop_counter = 0;
	
	I2C_AcknowledgeConfig(BSP_I2C_2, ENABLE);
	
	I2C_ITConfig(BSP_I2C_2,I2C_IT_EVT, ENABLE);//|I2C_IT_BUF
	I2C_GenerateSTART(BSP_I2C_2, ENABLE);
	
	OSSemPend(I2C_2_RxWait, 5000, &err);
	
	OSSemPost(I2C_2_Lock);
	
	return err | BSP_I2C_2_err;

}
CPU_INT08U I2C_2_CmdWrite(CPU_INT08U address,CPU_INT08U command, CPU_INT08U *dataP, CPU_INT08U dataLength)
{
	CPU_INT08U err;
	
	I2C_2_PeripheralInit();
	OSSemPend(I2C_2_Lock, 0, &err);
	
	I2C_2_Transfer_Mode = T_Command_Data;
	I2C_2_DeviceAddr = address;
	I2C_2_cmd = command;
	I2C_2_TxData = dataP;
	I2C_2_TxLength = dataLength;
	
	I2C_2_State = I2C_Tx_Start;
	I2C_2_TxCounter = 0;
	
	I2C_AcknowledgeConfig(BSP_I2C_2, ENABLE);
	
	I2C_ITConfig(BSP_I2C_2,I2C_IT_EVT, ENABLE);
	I2C_GenerateSTART(BSP_I2C_2, ENABLE);
	
	OSSemPend(I2C_2_TxWait, 0, &err);
	
	OSSemPost(I2C_2_Lock);
	
	
	return err | BSP_I2C_2_err;
}

uint32_t status;
uint32_t test_counter;
void BSP_I2C_2_ISR_Handler(void)
{
	/*Judge mode of transmite or receive*/
//	CPU_INT32U I2C1_SR1;
	
	switch(I2C_2_State) {
		case I2C_Rx_Start:
			if (I2C_2_Transfer_Mode == T_Command_Data) {
				if (I2C_GetITStatus(BSP_I2C_2, I2C_IT_SB)== SET 	// EV5
					/*|| I2C_GetITStatus(BSP_I2C_2, I2C_IT_SB) == SET*/) {
					I2C_Send7bitAddress(BSP_I2C_2, (I2C_2_DeviceAddr << 1), I2C_Direction_Transmitter);
					I2C_2_State = I2C_Rx_Addr;
				}else {
					BSP_I2C_2_ER_Callback(!0);
				}
			} else if(	I2C_2_Transfer_Mode == T_Data) {
				if (I2C_GetLastEvent(BSP_I2C_2) == I2C_EVENT_MASTER_MODE_SELECT    //EV5
					/*|| I2C_GetITStatus(BSP_I2C_2, I2C_IT_SB) == SET*/) {
					I2C_Send7bitAddress(BSP_I2C_2, (I2C_2_DeviceAddr << 1), I2C_Direction_Receiver);
					I2C_2_State = I2C_Rx_Addr;
				}else {
					BSP_I2C_2_ER_Callback(!0);
				}
			}else {
				BSP_I2C_2_ER_Callback(!0);
			}
		break;
		case I2C_Rx_Start_Again:
			test_counter = 0;
			while((I2C_GetLastEvent(BSP_I2C_2)& I2C_EVENT_MASTER_MODE_SELECT) != I2C_EVENT_MASTER_MODE_SELECT) 
				test_counter++;
			if (I2C_GetLastEvent(BSP_I2C_2) == I2C_EVENT_MASTER_MODE_SELECT 	// EV5
				/*|| I2C_GetITStatus(BSP_I2C_2, I2C_IT_SB) == SET*/) {
				I2C_Send7bitAddress(BSP_I2C_2, (I2C_2_DeviceAddr << 1), I2C_Direction_Receiver);
				I2C_2_State = I2C_Rx_Addr_Again;
			}else {
				BSP_I2C_2_ER_Callback(!0);
			}
		
			break;
		case I2C_Rx_Addr:
			if (I2C_2_Transfer_Mode == T_Command_Data) {
				if(I2C_GetLastEvent(BSP_I2C_2) == I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED 	    // EV6
					/*|| I2C_GetITStatus(BSP_I2C_2, I2C_IT_ADDR) == SET*/) {
					
					I2C_SendData(BSP_I2C_2, I2C_2_cmd);
					I2C_2_State = I2C_Rx_Cmd;
				}else {
					BSP_I2C_2_ER_Callback(!0);
				}	
			}else if (I2C_2_Transfer_Mode == T_Data) {
				if (I2C_GetLastEvent(BSP_I2C_2) == I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED 	    // EV6
					/*|| I2C_GetITStatus(BSP_I2C_2, I2C_IT_ADDR) == SET*/) {
					I2C_ITConfig(BSP_I2C_2, I2C_IT_BUF, ENABLE);
					I2C_2_State = I2C_Rx_Data;
				}else {
					BSP_I2C_2_ER_Callback(!0);
				}
			}
			break;
		case I2C_Rx_Cmd:
			if (I2C_GetLastEvent(BSP_I2C_2) == I2C_EVENT_MASTER_BYTE_TRANSMITTED 	    // EV8_2
				/*|| I2C_GetITStatus(BSP_I2C_2, I2C_IT_BTF) == SET*/)  {
				I2C_GenerateSTART(BSP_I2C_2, ENABLE);
				I2C_2_State = I2C_Rx_Start_Again;
			}else {
				BSP_I2C_2_ER_Callback(!0);
			}
			break;
		case I2C_Rx_Addr_Again:
			test_counter = 0;
			while((I2C_GetLastEvent(BSP_I2C_2) & I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED) != I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED) 
				test_counter++;
//			if (I2C_GetLastEvent(BSP_I2C_2) == I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED      // EV6
//					|| I2C_GetITStatus(BSP_I2C_2, I2C_IT_ADDR) == SET/**/) {
			if (test_counter < 0x1000){		
					I2C_ITConfig(BSP_I2C_2, I2C_IT_BUF, ENABLE);
					//I2C_2_RxBuffer[I2C_2_RxCounter] = I2C_ReceiveData(BSP_I2C_2);
					I2C_2_State = I2C_Rx_Data;
			}else {
				BSP_I2C_2_ER_Callback(!0);
			}
			break;
		case I2C_Rx_Data:
			test_counter = 0;
			while((I2C_GetLastEvent(BSP_I2C_2)& I2C_EVENT_MASTER_BYTE_RECEIVED) != I2C_EVENT_MASTER_BYTE_RECEIVED) 
				test_counter++;
			if (I2C_GetLastEvent(BSP_I2C_2) == I2C_EVENT_MASTER_BYTE_RECEIVED          // EV7
				||I2C_GetITStatus(BSP_I2C_2, I2C_IT_RXNE)/**/) {
					
				I2C_2_RxBuffer[I2C_2_RxCounter++] = I2C_ReceiveData(BSP_I2C_2);
				if (I2C_2_RxCounter < I2C_2_RxLength) {
					//I2C_2_RxBuffer[I2C_2_RxCounter++] = I2C_ReceiveData(BSP_I2C_2);
				}else {
					I2C_GenerateSTOP(BSP_I2C_2, ENABLE);
					//I2C_ITConfig(BSP_I2C_2, I2C_IT_BUF | I2C_IT_EVT, DISABLE);
					I2C_ITConfig(BSP_I2C_2, I2C_IT_BUF, DISABLE);
					//OSSemPost(I2C_2_RxWait);
					I2C_2_State = I2C_Rx_Stop;
					BSP_I2C_2_err = 0;
				}
			}else {
				BSP_I2C_2_ER_Callback(!0);
			}
			break;
		case I2C_Rx_Stop:
			test_counter = 0;
			while((I2C_GetLastEvent(BSP_I2C_2)& I2C_EVENT_MASTER_BYTE_RECEIVED) != I2C_EVENT_MASTER_BYTE_RECEIVED) 
				test_counter++;
			if (I2C_GetLastEvent(BSP_I2C_2) == I2C_EVENT_MASTER_BYTE_RECEIVED          // EV7
				/*||I2C_GetITStatus(BSP_I2C_2, I2C_IT_RXNE)*/) {
					
					I2C_ITConfig(BSP_I2C_2, I2C_IT_BUF | I2C_IT_EVT, DISABLE);
					OSSemPost(I2C_2_RxWait);
					I2C_stop_counter++;
					if (I2C_stop_counter == 2)
					BSP_I2C_2_err = 0;
				
			}else {
				BSP_I2C_2_ER_Callback(!0);
			}
			break;
		case I2C_Tx_Start:
			if(I2C_GetLastEvent(BSP_I2C_2) == I2C_EVENT_MASTER_MODE_SELECT   // EV5
				/*|| I2C_GetITStatus(BSP_I2C_2, I2C_IT_SB) == SET*/) {
				//I2C_ClearITPendingBit();
				I2C_Send7bitAddress(BSP_I2C_2, (I2C_2_DeviceAddr << 1), I2C_Direction_Transmitter);
				
				I2C_2_State = I2C_Tx_Addr;
			}else {
				BSP_I2C_2_ER_Callback(!0);
			}
			break;
		case I2C_Tx_Addr:
			if (I2C_2_Transfer_Mode == T_Command_Data) {
				if(I2C_GetLastEvent(BSP_I2C_2) == I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED 					// EV6
					/*||I2C_GetITStatus(BSP_I2C_2, I2C_IT_ADDR) == SET*/) {
					
					//I2C_ITConfig(BSP_I2C_2, I2C_IT_EVT, DISABLE);
					I2C_SendData(BSP_I2C_2, I2C_2_cmd);
					//OSSemPost(FlagBSP_I2C_2_Transmitter);
					I2C_2_State = I2C_Tx_Data;
					
					I2C_ITConfig(BSP_I2C_2, I2C_IT_BUF, ENABLE);
				}
			}else if (I2C_2_Transfer_Mode == T_Data){ 
					if(I2C_GetLastEvent(BSP_I2C_2) == I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED 				// EV6
						/*|| I2C_GetITStatus(BSP_I2C_2, I2C_IT_ADDR) == SET*/) {
						//I2C_ITConfig(BSP_I2C_2, I2C_IT_EVT, DISABLE);
						
						//OSSemPost(FlagI2C1_Transmitter);
						I2C_ITConfig(BSP_I2C_2, I2C_IT_BUF, ENABLE);
						I2C_SendData(BSP_I2C_2, I2C_2_TxData[I2C_2_TxCounter]);
						
						I2C_2_TxCounter++;
						I2C_2_State = I2C_Tx_Data;
						
						
					}
			}else{
				BSP_I2C_2_ER_Callback(!0);
			}
			break;	
			
		case I2C_Tx_Data:
			
		  status = I2C_GetLastEvent(BSP_I2C_2);
			if ( status == I2C_EVENT_MASTER_BYTE_TRANSMITTING //||   //          EV8
					//I2C_GetITStatus(BSP_I2C_2, I2C_IT_TXE) == SET
					) {
				//I2C_ITConfig(BSP_I2C_2, I2C_IT_EVT, DISABLE);
				
				//OSSemPost(FlagI2C1_Transmitter);
				
				if (I2C_2_TxCounter < I2C_2_TxLength) {
					I2C_SendData(BSP_I2C_2, I2C_2_TxData[I2C_2_TxCounter]);
					I2C_2_TxCounter++;
					I2C_2_State = I2C_Tx_Data;
				}else {
					I2C_ITConfig(BSP_I2C_2, I2C_IT_BUF, DISABLE);
					I2C_2_State = I2C_Tx_Data;
				}
				
			}else if (status == I2C_EVENT_MASTER_BYTE_TRANSMITTED 
								/*||I2C_GetITStatus(BSP_I2C_2, I2C_IT_BTF) == SET*/) {
				
				I2C_GenerateSTOP(BSP_I2C_2, ENABLE);
				I2C_ITConfig(BSP_I2C_2, I2C_IT_BUF | I2C_IT_EVT, DISABLE);
				OSSemPost(I2C_2_TxWait);
				BSP_I2C_2_err = 0;
			}else{
				BSP_I2C_2_ER_Callback(!0);
			}
			break;
			
		default:
			break; 
	}
	
	
//	switch (I2C_GetLastEvent(BSP_I2C_2))
//  {
//    /* EV5 */
//    case I2C_EVENT_MASTER_MODE_SELECT :

//      /* Send slave Address for write */
//      I2C_Send7bitAddress(BSP_I2C_2, (uint8_t)(I2C_2_DeviceAddr << 1), I2C_Direction_Transmitter);
//      break;
//		
//    /* EV6 */
//    case I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED:
//      /* Send the I2C transaction code */
//		switch(bsp_i2c_2_work_mode) {
//			case WORK_MODE_CMD_READ:
//			case WORK_MODE_CMD_SEND:
//				I2C_SendData(BSP_I2C_2, I2C_2_cmd);
//			break;
//			case WORK_MODE_READ:
//			case WORK_MODE_SEND:
//				I2C_SendData(BSP_I2C_2, I2C_2_TxData[I2C_2_TxCounter++]); 
//				break;
//			default:
//				BSP_I2C_2_ER_Callback(!0);
//				break;
//		}
//      break;
//		
//    /* EV8 */
//    case I2C_EVENT_MASTER_BYTE_TRANSMITTING:
//    case I2C_EVENT_MASTER_BYTE_TRANSMITTED:      
//     if (I2C_2_TxCounter >= I2C_2_TxLength)
//      {
//        /* Send STOP condition */
//        I2C_GenerateSTOP(BSP_I2C_2, ENABLE);
//        I2C_ITConfig(BSP_I2C_2, I2C_IT_EVT | I2C_IT_BUF, DISABLE);
//				OSSemPost(I2C_2_TxWait);
//				BSP_I2C_2_err = 0;
//      }
//      else
//      {
//        /* Transmit Data TxBuffer */
//        I2C_SendData(BSP_I2C_2, I2C_2_TxData[I2C_2_TxCounter++]); 
//      }
//      break;
//		case 
//    default:
//			
//      break;
//  }
	
	
}

void BSP_I2C_2_ER_ISR_Handler(void)
{
	
	BSP_I2C_2_err = I2C_ReadRegister(BSP_I2C_2, I2C_Register_SR1) & 0xFF00;
	if ( BSP_I2C_2_err  != 0x00) {
		BSP_I2C_2->SR1 &= 0x00FF;
	} 
	
	BSP_I2C_2_err = !0;
	I2C_ITConfig(BSP_I2C_2, I2C_IT_EVT, DISABLE);
	if (I2C_2_TxWait->OSEventGrp != 0)
		OSSemPost(I2C_2_TxWait);
	if (I2C_2_RxWait->OSEventGrp != 0)
		OSSemPost(I2C_2_RxWait);
	
	I2C_2_PeripheralInit();
}

void BSP_I2C_2_ER_Callback(uint16_t err_code)
{
	BSP_I2C_2_err = err_code;
	I2C_ITConfig(BSP_I2C_2, I2C_IT_BUF | I2C_IT_EVT, DISABLE);
	if (I2C_2_TxWait->OSEventGrp != 0)
		OSSemPost(I2C_2_TxWait);
	if (I2C_2_RxWait->OSEventGrp != 0)
		OSSemPost(I2C_2_RxWait);
	
	I2C_2_PeripheralInit();

}




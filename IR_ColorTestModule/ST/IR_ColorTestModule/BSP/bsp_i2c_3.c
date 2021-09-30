

#include "bsp_i2c_3.h"

/*********************************************************************************************************************
*																					Local define
*********************************************************************************************************************/

//**************************************  Hardware configuration  ****************************************************
#define BSP_I2C_3												I2C3
#define BSP_I2C_3_CLK										RCC_APB1Periph_I2C3

#define BSP_I2C_3_SCL_PIN								GPIO_Pin_8
#define BSP_I2C_3_SCL_GPIO_PORT					GPIOA
#define BSP_I2C_3_SCL_GPIO_CLK					RCC_AHB1Periph_GPIOA
#define BSP_I2C_3_SCL_SOURCE						GPIO_PinSource8
#define BSP_I2C_3_SCL_AF								GPIO_AF_I2C3


#define BSP_I2C_3_SDA_PIN								GPIO_Pin_9
#define BSP_I2C_3_SDA_GPIO_PORT					GPIOC
#define BSP_I2C_3_SDA_GPIO_CLK					RCC_AHB1Periph_GPIOC
#define BSP_I2C_3_SDA_SOURCE						GPIO_PinSource9
#define BSP_I2C_3_SDA_AF								GPIO_AF_I2C3

#define BSP_I2C_3_DR										((CPU_INT32U)0x40005C10)


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
int8_t 		I2C_3_State = 0;
int8_t 		I2C_3_Transfer_Mode;

uint8_t 	I2C_3_DeviceAddr;
int8_t 		I2C_3_RegAddr;

uint8_t 	*I2C_3_RxBuffer;
int8_t 		I2C_3_RxLength;
int8_t 		I2C_3_RxCounter;

uint8_t 	*I2C_3_TxData;
uint8_t   I2C_3_TxLength;
uint8_t   I2C_3_TxCounter;

uint8_t 	I2C_3_cmd;

uint16_t BSP_I2C_3_err;
//int32_t 	I2C_3_TimeOut;


OS_EVENT *I2C_3_TxWait;
OS_EVENT *I2C_3_RxWait;
OS_EVENT *I2C_3_Lock;
/*********************************************************************************************************************
* 																							Local function prototype
*********************************************************************************************************************/

void BSP_I2C_3_ER_ISR_Handler(void);
void BSP_I2C_3_ISR_Handler(void);


void I2C_3_PeripheralInit(void)
{
	I2C_InitTypeDef I2C_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB1PeriphClockCmd(BSP_I2C_3_CLK, ENABLE);
	
	RCC_AHB1PeriphClockCmd(BSP_I2C_3_SDA_GPIO_CLK, ENABLE);
	RCC_AHB1PeriphClockCmd(BSP_I2C_3_SCL_GPIO_CLK , ENABLE);
	
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);  
	
	/* Reset I2Cx IP */
	RCC_APB1PeriphResetCmd(BSP_I2C_3_CLK, ENABLE);
	/* Release reset signal of I2Cx IP */
  RCC_APB1PeriphResetCmd(BSP_I2C_3_CLK, DISABLE);
	
	GPIO_InitStructure.GPIO_Pin = BSP_I2C_3_SCL_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(BSP_I2C_3_SCL_GPIO_PORT, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = BSP_I2C_3_SDA_PIN;
	GPIO_Init(BSP_I2C_3_SDA_GPIO_PORT, &GPIO_InitStructure);
	
	
	GPIO_PinAFConfig(BSP_I2C_3_SCL_GPIO_PORT, BSP_I2C_3_SCL_SOURCE, BSP_I2C_3_SCL_AF);
	GPIO_PinAFConfig(BSP_I2C_3_SDA_GPIO_PORT, BSP_I2C_3_SDA_SOURCE, BSP_I2C_3_SDA_AF);
	
	I2C_DeInit(BSP_I2C_3);
	
	I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;
	I2C_InitStructure.I2C_DutyCycle = I2C_DutyCycle_2;
	I2C_InitStructure.I2C_OwnAddress1 = 0xA0;
	I2C_InitStructure.I2C_Ack = I2C_Ack_Enable;
	I2C_InitStructure.I2C_ClockSpeed = 100000;
	
	I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
	
	I2C_Init(BSP_I2C_3, &I2C_InitStructure);
	
	
	
	I2C_ITConfig(BSP_I2C_3, I2C_IT_ERR , ENABLE);
	
	I2C_Cmd(BSP_I2C_3, ENABLE);
	
}



void I2C_3_Initialization(void)
{
	I2C_InitTypeDef I2C_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
	
	I2C_3_TxWait = OSSemCreate(0);
	I2C_3_RxWait = OSSemCreate(0);
	I2C_3_Lock = OSSemCreate(1);
	
	
	RCC_APB1PeriphClockCmd(BSP_I2C_3_CLK, ENABLE);
	
	RCC_AHB1PeriphClockCmd(BSP_I2C_3_SDA_GPIO_CLK, ENABLE);
	RCC_AHB1PeriphClockCmd(BSP_I2C_3_SCL_GPIO_CLK , ENABLE);
	
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);  
	
	/* Reset I2Cx IP */
	RCC_APB1PeriphResetCmd(BSP_I2C_3_CLK, ENABLE);
	/* Release reset signal of I2Cx IP */
  RCC_APB1PeriphResetCmd(BSP_I2C_3_CLK, DISABLE);
	
	GPIO_InitStructure.GPIO_Pin = BSP_I2C_3_SCL_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(BSP_I2C_3_SCL_GPIO_PORT, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = BSP_I2C_3_SDA_PIN;
	GPIO_Init(BSP_I2C_3_SDA_GPIO_PORT, &GPIO_InitStructure);
	
	
	GPIO_PinAFConfig(BSP_I2C_3_SCL_GPIO_PORT, BSP_I2C_3_SCL_SOURCE, BSP_I2C_3_SCL_AF);
	GPIO_PinAFConfig(BSP_I2C_3_SDA_GPIO_PORT, BSP_I2C_3_SDA_SOURCE, BSP_I2C_3_SDA_AF);
	
	I2C_DeInit(BSP_I2C_3);
	
	I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;
	I2C_InitStructure.I2C_DutyCycle = I2C_DutyCycle_2;
	I2C_InitStructure.I2C_OwnAddress1 = 0xA0;
	I2C_InitStructure.I2C_Ack = I2C_Ack_Enable;
	I2C_InitStructure.I2C_ClockSpeed = 100000;
	
	I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
	
	I2C_Init(BSP_I2C_3, &I2C_InitStructure);
	
	
	
	I2C_ITConfig(BSP_I2C_3, I2C_IT_ERR , ENABLE);
	
	I2C_Cmd(BSP_I2C_3, ENABLE);
	
	BSP_IntVectSet(BSP_INT_ID_I2C3_EV, BSP_I2C_3_ISR_Handler);
  BSP_IntEn(BSP_INT_ID_I2C3_EV);
	 
	BSP_IntVectSet(BSP_INT_ID_I2C3_ER, BSP_I2C_3_ER_ISR_Handler); 
  BSP_IntEn(BSP_INT_ID_I2C3_ER);
	
//	I2C_3_Write(0x01, "123", 3);
	
}


CPU_INT08U I2C_3_Write(CPU_INT08U address, CPU_INT08U *dataP, CPU_INT08U dataLength)
{
	CPU_INT08U err;
	OSSemPend(I2C_3_Lock, 0, &err);
	
	I2C_3_Transfer_Mode = T_Data;
	I2C_3_DeviceAddr = address;
	I2C_3_TxData = dataP;
	I2C_3_TxLength = dataLength;
	
	I2C_3_State = I2C_Tx_Start;
	I2C_3_TxCounter = 0;
	
	I2C_AcknowledgeConfig(BSP_I2C_3, ENABLE);
	
	I2C_ITConfig(BSP_I2C_3,I2C_IT_EVT | I2C_IT_BUF, ENABLE);//
	I2C_GenerateSTART(BSP_I2C_3, ENABLE);
	
	OSSemPend(I2C_3_TxWait, 0, &err);
	
	OSSemPost(I2C_3_Lock);
	
	
	return err | BSP_I2C_3_err;
}


CPU_INT08U I2C_3_CmdWrite(CPU_INT08U address, CPU_INT08U cmd, CPU_INT08U *dataP, CPU_INT08U dataLength)
{
	CPU_INT08U err;
	OSSemPend(I2C_3_Lock, 0, &err);
	
	I2C_3_Transfer_Mode = T_Command_Data;
	I2C_3_DeviceAddr = address;
	I2C_3_cmd = cmd;
	I2C_3_TxData = dataP;
	I2C_3_TxLength = dataLength;
	
	I2C_3_State = I2C_Tx_Start;
	I2C_3_TxCounter = 0;
	
	I2C_AcknowledgeConfig(BSP_I2C_3, ENABLE);
	
	I2C_ITConfig(BSP_I2C_3,I2C_IT_EVT, ENABLE);
	I2C_GenerateSTART(BSP_I2C_3, ENABLE);
	
	OSSemPend(I2C_3_TxWait, 0, &err);
	
	OSSemPost(I2C_3_Lock);
	
	
	return err | BSP_I2C_3_err;
}


CPU_INT08U I2C_3_Read(CPU_INT08U address, CPU_INT08U *dataP, CPU_INT08U dataLength)
{
	CPU_INT08U err;
	
	OSSemPend(I2C_3_Lock, 0, &err);
	
	I2C_3_Transfer_Mode = T_Data;
	I2C_3_DeviceAddr = address;
	I2C_3_RxBuffer = dataP;
	I2C_3_RxLength = dataLength;
	
	I2C_3_State = I2C_Rx_Start;
	I2C_3_RxCounter = 0;
	
	I2C_AcknowledgeConfig(BSP_I2C_3, ENABLE);
	
	I2C_ITConfig(BSP_I2C_3,I2C_IT_EVT, ENABLE);//|I2C_IT_BUF
	I2C_GenerateSTART(BSP_I2C_3, ENABLE);
	
	OSSemPend(I2C_3_RxWait, 0, &err);
	
	OSSemPost(I2C_3_Lock);
	
	return err | BSP_I2C_3_err;
}

CPU_INT08U I2C_3_CmdRead(CPU_INT08U address,CPU_INT08U cmd, CPU_INT08U *dataP, CPU_INT08U dataLength)
{
	CPU_INT08U err;
	
	OSSemPend(I2C_3_Lock, 0, &err);
	
	I2C_3_Transfer_Mode = T_Data;
	I2C_3_DeviceAddr = address;
	I2C_3_cmd = cmd;
	I2C_3_RxBuffer = dataP;
	I2C_3_RxLength = dataLength;
	
	I2C_3_State = I2C_Rx_Start;
	I2C_3_RxCounter = 0;
	
	I2C_AcknowledgeConfig(BSP_I2C_3, ENABLE);
	
	I2C_ITConfig(BSP_I2C_3,I2C_IT_EVT, ENABLE);//|I2C_IT_BUF
	I2C_GenerateSTART(BSP_I2C_3, ENABLE);
	
	OSSemPend(I2C_3_RxWait, 0, &err);
	
	OSSemPost(I2C_3_Lock);
	
	return err | BSP_I2C_3_err;
}



void BSP_I2C_3_ISR_Handler(void)
{
	/*Judge mode of transmite or receive*/
//	CPU_INT32U I2C1_SR1;
	
	switch(I2C_3_State) {
		case I2C_Rx_Start:
			if (I2C_3_Transfer_Mode == T_Command_Data) {
				if (I2C_GetITStatus(BSP_I2C_3, I2C_IT_SB) == SET) {
					I2C_Send7bitAddress(BSP_I2C_3, (I2C_3_DeviceAddr << 1), I2C_Direction_Transmitter);
					I2C_3_State = I2C_Rx_Addr;
				}
			} else if(I2C_3_Transfer_Mode == T_Data) {
				if (I2C_GetITStatus(BSP_I2C_3, I2C_IT_SB) == SET) {
					I2C_Send7bitAddress(BSP_I2C_3, (I2C_3_DeviceAddr << 1), I2C_Direction_Receiver);
					I2C_3_State = I2C_Rx_Addr;
				}
			}
		break;
		case I2C_Rx_Start_Again:
			if (I2C_GetITStatus(BSP_I2C_3, I2C_IT_SB) == SET) {
				I2C_Send7bitAddress(BSP_I2C_3, (I2C_3_DeviceAddr << 1), I2C_Direction_Receiver);
				I2C_3_State = I2C_Rx_Addr_Again;
			}
		
			break;
		case I2C_Rx_Addr:
			if (I2C_3_Transfer_Mode == T_Command_Data) {
				if(I2C_GetITStatus(BSP_I2C_3, I2C_IT_ADDR) == SET) {
					
					I2C_SendData(BSP_I2C_3, I2C_3_cmd);
					I2C_3_State = I2C_Rx_Cmd;
				}
			}else if ( I2C_3_Transfer_Mode == T_Data) {
				if (I2C_GetITStatus(BSP_I2C_3, I2C_IT_ADDR) == SET) {
					I2C_ITConfig(BSP_I2C_3, I2C_IT_BUF, ENABLE);
					I2C_3_State = I2C_Rx_Data;
				}
			}
			break;
		case I2C_Rx_Cmd:
			if (I2C_GetITStatus(BSP_I2C_3, I2C_IT_BTF) == SET)  {
				I2C_GenerateSTART(BSP_I2C_3, ENABLE);
				I2C_3_State = I2C_Rx_Start_Again;
			}
			break;
		case I2C_Rx_Addr_Again:
			if (I2C_GetITStatus(BSP_I2C_3, I2C_IT_ADDR) == SET) {
					I2C_ITConfig(BSP_I2C_3, I2C_IT_BUF, ENABLE);
					I2C_3_State = I2C_Rx_Data;
			}
			break;
		case I2C_Rx_Data:
			if (I2C_GetITStatus(BSP_I2C_3, I2C_IT_RXNE)) {
				I2C_3_RxBuffer[I2C_3_RxLength++] = I2C_ReceiveData(BSP_I2C_3);
				if (I2C_3_RxCounter < I2C_3_RxLength) {
					I2C_3_RxBuffer[I2C_3_RxCounter++] = I2C_ReceiveData(BSP_I2C_3);
				}else {
					I2C_GenerateSTOP(BSP_I2C_3, ENABLE);
					I2C_ITConfig(BSP_I2C_3, I2C_IT_BUF | I2C_IT_EVT, DISABLE);
					OSSemPost(I2C_3_RxWait);
					BSP_I2C_3_err = 0;
				}
			}
			break;
		case I2C_Tx_Start:
			if(I2C_GetITStatus(BSP_I2C_3, I2C_IT_SB) == SET) {
				//I2C_ClearITPendingBit();
				I2C_Send7bitAddress(BSP_I2C_3, (I2C_3_DeviceAddr << 1), I2C_Direction_Transmitter);
				
				I2C_3_State = I2C_Tx_Addr;
			}
			break;
		case I2C_Tx_Addr:
			if (I2C_3_Transfer_Mode == T_Command_Data) {
				if(I2C_GetLastEvent(BSP_I2C_3) == I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED ||					// EV6
					I2C_GetITStatus(BSP_I2C_3, I2C_IT_ADDR) == SET) {
					
					//I2C_ITConfig(BSP_I2C_3, I2C_IT_EVT, DISABLE);
					//OSSemPost(FlagBSP_I2C_3_Transmitter);
					I2C_SendData(BSP_I2C_3, I2C_3_cmd);
					I2C_3_State = I2C_Tx_Data;
					
					I2C_ITConfig(BSP_I2C_3, I2C_IT_BUF, ENABLE);
				}
			}else if (I2C_3_Transfer_Mode == T_Data){ 
					if(I2C_GetLastEvent(BSP_I2C_3) == I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED ||				// EV6
						I2C_GetITStatus(BSP_I2C_3, I2C_IT_ADDR) == SET) {
						//I2C_ITConfig(BSP_I2C_3, I2C_IT_EVT, DISABLE);
						
						//OSSemPost(FlagI2C1_Transmitter);
						I2C_ITConfig(BSP_I2C_3, I2C_IT_BUF, ENABLE);
						I2C_SendData(BSP_I2C_3, I2C_3_TxData[I2C_3_TxCounter]);
						
						I2C_3_TxCounter++;
						I2C_3_State = I2C_Tx_Data;
						
						
					}
			}else{
				for(;;);
			}
			break;	
			
		case I2C_Tx_Data:
		{
		  uint32_t status = I2C_GetLastEvent(BSP_I2C_3);
			if ( status == I2C_EVENT_MASTER_BYTE_TRANSMITTING //||   //          EV8
					//I2C_GetITStatus(BSP_I2C_3, I2C_IT_TXE) == SET
					) {
				//I2C_ITConfig(BSP_I2C_3, I2C_IT_EVT, DISABLE);
				
				//OSSemPost(FlagI2C1_Transmitter);
				
				if (I2C_3_TxCounter < I2C_3_TxLength) {
					I2C_SendData(BSP_I2C_3, I2C_3_TxData[I2C_3_TxCounter]);
					I2C_3_TxCounter++;
					I2C_3_State = I2C_Tx_Data;
				}else {
					I2C_ITConfig(BSP_I2C_3, I2C_IT_BUF, DISABLE);
					I2C_3_State = I2C_Tx_Data;
				}
				
			}else if (status == I2C_EVENT_MASTER_BYTE_TRANSMITTED ||
								I2C_GetITStatus(BSP_I2C_3, I2C_IT_BTF) == SET) {
				
				I2C_GenerateSTOP(BSP_I2C_3, ENABLE);
				I2C_ITConfig(BSP_I2C_3, I2C_IT_BUF | I2C_IT_EVT, DISABLE);
				OSSemPost(I2C_3_TxWait);
				BSP_I2C_3_err = 0;
			}else{
				while(1);
			}
		}
			break;
			
		default:
			break; 
	}



}

void BSP_I2C_3_ER_ISR_Handler(void)
{
	
	BSP_I2C_3_err = I2C_ReadRegister(BSP_I2C_3, I2C_Register_SR1) & 0xFF00;
	if ( BSP_I2C_3_err  != 0x00) {
		BSP_I2C_3->SR1 &= 0x00FF;
	} 
	
	BSP_I2C_3_err = !0;
	I2C_ITConfig(BSP_I2C_3, I2C_IT_EVT, DISABLE);
	OSSemPost(I2C_3_TxWait);
	I2C_3_PeripheralInit();
}





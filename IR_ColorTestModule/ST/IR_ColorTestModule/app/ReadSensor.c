#include "ReadSensor.h"


/*------------------------------------------------------------------------------
    Hardware Configuration 
------------------------------------------------------------------------------*/
/** 
  * @brief  I2C port definitions  
  */ 
#define IOE_I2C                          I2C1
#define IOE_I2C_CLK                      RCC_APB1Periph_I2C1
#define IOE_I2C_SCL_PIN                  GPIO_Pin_6
#define IOE_I2C_SCL_GPIO_PORT            GPIOB
#define IOE_I2C_SCL_GPIO_CLK             RCC_AHB1Periph_GPIOB
#define IOE_I2C_SCL_SOURCE               GPIO_PinSource6
#define IOE_I2C_SCL_AF                   GPIO_AF_I2C1
#define IOE_I2C_SDA_PIN                  GPIO_Pin_9
#define IOE_I2C_SDA_GPIO_PORT            GPIOB
#define IOE_I2C_SDA_GPIO_CLK             RCC_AHB1Periph_GPIOB
#define IOE_I2C_SDA_SOURCE               GPIO_PinSource9
#define IOE_I2C_SDA_AF                   GPIO_AF_I2C1
#define IOE_I2C_DR                       ((uint32_t)0x40005410)

/** 
  * @brief  IOE DMA definitions  
  */
#define IOE_DMA_CLK                      RCC_AHB1Periph_DMA1
#define IOE_DMA_CHANNEL                  DMA_Channel_1

#ifndef IOE_DMA_TX_STREAM
 #define IOE_DMA_TX_STREAM               DMA1_Stream6
#endif /* IOE_DMA_TX_STREAM */

#ifndef IOE_DMA_TX_TCFLAG 
 #define IOE_DMA_TX_TCFLAG               DMA_FLAG_TCIF6
#endif /* IOE_DMA_TX_TCFLAG */

#ifndef IOE_DMA_RX_STREAM
 #define IOE_DMA_RX_STREAM               DMA1_Stream0
#endif /* IOE_DMA_RX_STREAM */

#ifndef IOE_DMA_RX_TCFLAG 			
 #define IOE_DMA_RX_TCFLAG               DMA_FLAG_TCIF0
#endif /* IOE_DMA_RX_TCFLAG */

#define IOE_TimeoutUserCallback() 1

static  OS_STK          AppTaskReadSensorStk[APP_TASK_READ_SENSOR_STK_SIZE];

typedef enum
{
  IOE_DMA_TX = 0,
  IOE_DMA_RX = 1
}IOE_DMADirection_TypeDef;

int8_t a = 1;
RGBC rgbc_sensor[RGBCn];
void RGBC_DMA_Config(IOE_DMADirection_TypeDef Direction, uint8_t* buffer, uint8_t size);
void initI2C1(void);
//void  BSP_I2C1_ISR_Handler (void);
//void  BSP_I2C1_ER_ISR_Handler (void);
void initI2C1(void)
{
	I2C_InitTypeDef I2C_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);		// 
	
	I2C_InitStructure.I2C_Ack = I2C_Ack_Enable;
	I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
	I2C_InitStructure.I2C_ClockSpeed = 10000;
	I2C_InitStructure.I2C_DutyCycle = I2C_DutyCycle_2;
	I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;
	
	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
	GPIO_InitStructure.GPIO_Pin	= GPIO_Pin_6 | GPIO_Pin_9;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_25MHz;
	
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource6, GPIO_AF_I2C1);
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource9, GPIO_AF_I2C1);
	
	I2C_Init(I2C1, &I2C_InitStructure);	
	
	I2C_Cmd(I2C1, ENABLE);
	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Pin	=  GPIO_Pin_5;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_25MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	GPIO_ResetBits(GPIOB, GPIO_Pin_5);
	OSTimeDly(100);
	GPIO_SetBits(GPIOB, GPIO_Pin_5);
	
	BSP_IntVectSet(BSP_INT_ID_I2C1_EV, BSP_I2C1_ISR_Handler);
  BSP_IntEn(BSP_INT_ID_I2C1_EV);
	 
	BSP_IntVectSet(BSP_INT_ID_I2C1_ER, BSP_I2C1_ER_ISR_Handler); 
  BSP_IntEn(BSP_INT_ID_I2C1_EV);	
	
	BSP_IntVectSet(BSP_INT_ID_DMA1_CH1, BSP_DMA1_CH1_ISR_Handler); 
  BSP_IntEn(BSP_INT_ID_DMA1_CH1);	
	
	BSP_IntVectSet(BSP_INT_ID_DMA1_CH7, BSP_DMA1_CH1_ISR_Handler); 
  BSP_IntEn(BSP_INT_ID_DMA1_CH7);	
	
}




#define TIMEOUT_MAX 6000 // This value define maximal of timeout waitingof i2c loops 
RGBC rgbc_data;
uint8_t Wait;
uint32_t RGBC_TimeOut = TIMEOUT_MAX; // Value of timeout when communication fails

static void IOE_DMA_Config(IOE_DMADirection_TypeDef Direction, uint8_t* buffer)
{
  DMA_InitTypeDef DMA_InitStructure;
  
  RCC_AHB1PeriphClockCmd(IOE_DMA_CLK, ENABLE);
  
  /* Initialize the DMA_Channel member */
  DMA_InitStructure.DMA_Channel = IOE_DMA_CHANNEL;
  
  /* Initialize the DMA_PeripheralBaseAddr member */
  DMA_InitStructure.DMA_PeripheralBaseAddr = IOE_I2C_DR;
  
  /* Initialize the DMA_Memory0BaseAddr member */
  DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)buffer;
  
  /* Initialize the DMA_PeripheralInc member */
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
  
  /* Initialize the DMA_MemoryInc member */
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
  
  /* Initialize the DMA_PeripheralDataSize member */
  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
  
  /* Initialize the DMA_MemoryDataSize member */
  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
  
  /* Initialize the DMA_Mode member */
  DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
  
  /* Initialize the DMA_Priority member */
  DMA_InitStructure.DMA_Priority = DMA_Priority_Low;
  
  /* Initialize the DMA_FIFOMode member */
  DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Enable;
  
  /* Initialize the DMA_FIFOThreshold member */
  DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_1QuarterFull;
  
  /* Initialize the DMA_MemoryBurst member */
  DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
  
  /* Initialize the DMA_PeripheralBurst member */
  DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
  
  /* If using DMA for Reception */
  if (Direction == IOE_DMA_RX)
  {    
    /* Initialize the DMA_DIR member */
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;
    
    /* Initialize the DMA_BufferSize member */
    DMA_InitStructure.DMA_BufferSize = 6;
    
    DMA_DeInit(IOE_DMA_RX_STREAM);
    
    DMA_Init(IOE_DMA_RX_STREAM, &DMA_InitStructure);
  }
  /* If using DMA for Transmission */
  else if (Direction == IOE_DMA_TX)
  { 
    /* Initialize the DMA_DIR member */
    DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToPeripheral;
    
    /* Initialize the DMA_BufferSize member */
    DMA_InitStructure.DMA_BufferSize = 1;
    
    DMA_DeInit(IOE_DMA_TX_STREAM);
    
    DMA_Init(IOE_DMA_TX_STREAM, &DMA_InitStructure);
  }
}
/**
  * @brief  Writes a value in a register of the device through I2C.
  * @param  DeviceAddr: The address of the IOExpander, could be : IOE_1_ADDR
  *         or IOE_2_ADDR. 
  * @param  RegisterAddr: The target register address
  * @param  RegisterValue: The target register value to be written 
  * @retval IOE_OK: if all operations are OK. Other value if error.
  */
uint8_t RGBC_I2C_WriteDeviceRegister(uint8_t DeviceAddr, uint8_t RegisterAddr, uint8_t RegisterValue)
{
  uint8_t read_verif = 0;
  uint8_t IOE_BufferTX[2] = {0,0};
  I2C_AcknowledgeConfig(IOE_I2C, ENABLE);
  /* Get Value to be written */
  IOE_BufferTX[0] = RegisterValue;
 // IOE_BufferTX[1] = DeviceAddr;
  /* Configure DMA Peripheral */
  RGBC_DMA_Config(IOE_DMA_TX, (uint8_t*)(&IOE_BufferTX[0]), 1);
  
  /* Enable the I2C peripheral */
  I2C_GenerateSTART(IOE_I2C, ENABLE);
  
  /* Test on SB Flag */
  RGBC_TimeOut = TIMEOUT_MAX;
  while (I2C_GetFlagStatus(IOE_I2C,I2C_FLAG_SB) == RESET) 
  {
    if (RGBC_TimeOut-- == 0) return(IOE_TimeoutUserCallback());
  }
  
  /* Transmit the slave address and enable writing operation */
  I2C_Send7bitAddress(IOE_I2C, DeviceAddr, I2C_Direction_Transmitter);
	
  /* Test on ADDR Flag */
  RGBC_TimeOut = TIMEOUT_MAX;
  while (!I2C_CheckEvent(IOE_I2C, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED))
  {
    if (RGBC_TimeOut-- == 0) 
		return(IOE_TimeoutUserCallback());
  }

	
  /* Transmit the first address for r/w operations */
  I2C_SendData(IOE_I2C, RegisterAddr);
  
  /* Test on TXE FLag (data dent) */
  RGBC_TimeOut = TIMEOUT_MAX;
  while ((!I2C_GetFlagStatus(IOE_I2C,I2C_FLAG_TXE)) && (!I2C_GetFlagStatus(IOE_I2C,I2C_FLAG_BTF)))  
  {
    if (RGBC_TimeOut-- == 0) return(IOE_TimeoutUserCallback());
  }
  
	
  /* Enable I2C DMA request */
  I2C_DMACmd(IOE_I2C,ENABLE);
  
  /* Enable DMA TX Channel */
  DMA_Cmd(IOE_DMA_TX_STREAM, ENABLE);
	
  /* Wait until DMA Transfer Complete */
  RGBC_TimeOut = TIMEOUT_MAX;
  while (!DMA_GetFlagStatus(IOE_DMA_TX_STREAM,IOE_DMA_TX_TCFLAG))
  {
    if (RGBC_TimeOut-- == 0) return(IOE_TimeoutUserCallback());
  }  
  
  /* Wait until BTF Flag is set before generating STOP */
  RGBC_TimeOut = 2 * TIMEOUT_MAX;
  while ((!I2C_GetFlagStatus(IOE_I2C,I2C_FLAG_BTF)))  
  {
		if (RGBC_TimeOut-- == 0) return(IOE_TimeoutUserCallback());
  }
  
  /* Send STOP Condition */
  I2C_GenerateSTOP(IOE_I2C, ENABLE);
  
	

  /* Disable DMA TX Channel */
  DMA_Cmd(IOE_DMA_TX_STREAM, DISABLE);
  
  /* Disable I2C DMA request */  
  I2C_DMACmd(IOE_I2C,DISABLE);
  
  /* Clear DMA TX Transfer Complete Flag */
  DMA_ClearFlag(IOE_DMA_TX_STREAM,IOE_DMA_TX_TCFLAG);
  
#ifdef VERIFY_WRITTENDATA
  /* Verify (if needed) that the loaded data is correct  */
  
  /* Read the just written register*/
  read_verif = I2C_ReadDeviceRegister(DeviceAddr, RegisterAddr);
  /* Load the register and verify its value  */
  if (read_verif != RegisterValue)
  {
    /* Control data wrongly transfered */
    read_verif = IOE_FAILURE;
  }
  else
  {
    /* Control data correctly transfered */
    read_verif = 0;
  }
#endif
  
  /* Return the verifying value: 0 (Passed) or 1 (Failed) */
  return (read_verif);
}

static void RGBC_DMA_Config(IOE_DMADirection_TypeDef Direction, uint8_t* buffer, uint8_t size)
{
  DMA_InitTypeDef DMA_InitStructure;
  
  RCC_AHB1PeriphClockCmd(IOE_DMA_CLK, ENABLE);
  
  /* Initialize the DMA_Channel member */
  DMA_InitStructure.DMA_Channel = IOE_DMA_CHANNEL;
  
  /* Initialize the DMA_PeripheralBaseAddr member */
  DMA_InitStructure.DMA_PeripheralBaseAddr = IOE_I2C_DR;
  
  /* Initialize the DMA_Memory0BaseAddr member */
  DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)buffer;
  
  /* Initialize the DMA_PeripheralInc member */
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
  
  /* Initialize the DMA_MemoryInc member */
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
  
  /* Initialize the DMA_PeripheralDataSize member */
  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
  
  /* Initialize the DMA_MemoryDataSize member */
  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
  
  /* Initialize the DMA_Mode member */
  DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
  
  /* Initialize the DMA_Priority member */
  DMA_InitStructure.DMA_Priority = DMA_Priority_Low;
  
  /* Initialize the DMA_FIFOMode member */
  DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Enable;
  
  /* Initialize the DMA_FIFOThreshold member */
  DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_1QuarterFull;
  
  /* Initialize the DMA_MemoryBurst member */
  DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
  
  /* Initialize the DMA_PeripheralBurst member */
  DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
  
  /* If using DMA for Reception */
  if (Direction == IOE_DMA_RX)
  {    
    /* Initialize the DMA_DIR member */
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;
    
    /* Initialize the DMA_BufferSize member */
    DMA_InitStructure.DMA_BufferSize = size;
    
    DMA_DeInit(IOE_DMA_RX_STREAM);
    
    DMA_Init(IOE_DMA_RX_STREAM, &DMA_InitStructure);
  }
  /* If using DMA for Transmission */
  else if (Direction == IOE_DMA_TX)
  { 
    /* Initialize the DMA_DIR member */
    DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToPeripheral;
    
    /* Initialize the DMA_BufferSize member */
    DMA_InitStructure.DMA_BufferSize = size;
    
    DMA_DeInit(IOE_DMA_TX_STREAM);
    
    DMA_Init(IOE_DMA_TX_STREAM, &DMA_InitStructure);
  }
}
/**
  * @brief  Reads a buffer of 2 bytes from the device registers.
  * @param  DeviceAddr: The address of the device, could be : IOE_1_ADDR
  *         or IOE_2_ADDR. 
  * @param  RegisterAddr: The target register address (between 00x and 0x24)
  * @retval A pointer to the buffer containing the two returned bytes (in halfword).  
 **/
uint8_t RGBC_I2C_ReadDataBuffer(uint8_t DeviceAddr, uint8_t RegisterAddr, uint8_t * buffer, uint8_t size)
{  
	I2C_AcknowledgeConfig(IOE_I2C, ENABLE);
  /* Configure DMA Peripheral */
  RGBC_DMA_Config(IOE_DMA_RX, (uint8_t*)buffer, size);
  
  /* Enable DMA NACK automatic generation */
  I2C_DMALastTransferCmd(IOE_I2C, ENABLE);
  
  /* Enable the I2C peripheral */
  I2C_GenerateSTART(IOE_I2C, ENABLE);
  
  /* Test on SB Flag */
  RGBC_TimeOut = TIMEOUT_MAX;
  while (!I2C_GetFlagStatus(IOE_I2C,I2C_FLAG_SB)) 
  {
    if (RGBC_TimeOut-- == 0) 
			return(IOE_TimeoutUserCallback());
  }
  
  /* Send device address for write */
  I2C_Send7bitAddress(IOE_I2C, DeviceAddr, I2C_Direction_Transmitter);
  
  /* Test on ADDR Flag */
  RGBC_TimeOut = TIMEOUT_MAX;
  while (!I2C_CheckEvent(IOE_I2C, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED))
  {
    if (RGBC_TimeOut-- == 0) 
			return(IOE_TimeoutUserCallback());
  }
  
  /* Send the device's internal address to write to */
  I2C_SendData(IOE_I2C, RegisterAddr);  
  
  /* Test on TXE FLag (data dent) */
  RGBC_TimeOut = TIMEOUT_MAX;
  while ((!I2C_GetFlagStatus(IOE_I2C,I2C_FLAG_TXE)) && (!I2C_GetFlagStatus(IOE_I2C,I2C_FLAG_BTF)))  
  {
    if (RGBC_TimeOut-- == 0) 
			return(IOE_TimeoutUserCallback());
  }
  
  /* Send START condition a second time */  
  I2C_GenerateSTART(IOE_I2C, ENABLE);
  
  /* Test on SB Flag */
  RGBC_TimeOut = TIMEOUT_MAX;
  while (!I2C_GetFlagStatus(IOE_I2C,I2C_FLAG_SB)) 
  {
    if (RGBC_TimeOut-- == 0) 
			return(IOE_TimeoutUserCallback());
  }
  
  /* Send IOExpander address for read */
  I2C_Send7bitAddress(IOE_I2C, DeviceAddr, I2C_Direction_Receiver);
  
  /* Test on ADDR Flag */
 RGBC_TimeOut = TIMEOUT_MAX;
  while (!I2C_CheckEvent(IOE_I2C, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED))   
  {
    if (RGBC_TimeOut-- == 0) 
			return(IOE_TimeoutUserCallback());
  }
  
  /* Enable I2C DMA request */
  I2C_DMACmd(IOE_I2C,ENABLE);
  
  /* Enable DMA RX Channel */
  DMA_Cmd(IOE_DMA_RX_STREAM, ENABLE);
	
	
  /* Wait until DMA Transfer Complete */
  RGBC_TimeOut = 10 * TIMEOUT_MAX;
  while (!DMA_GetFlagStatus(IOE_DMA_RX_STREAM, IOE_DMA_RX_TCFLAG))
  {
    if (RGBC_TimeOut-- == 0) 
			return(IOE_TimeoutUserCallback());
  }        

	
  /* Send STOP Condition */
  I2C_GenerateSTOP(IOE_I2C, ENABLE);
  
  /* Disable DMA RX Channel */
  DMA_Cmd(IOE_DMA_RX_STREAM, DISABLE);
  
  /* Disable I2C DMA request */  
  I2C_DMACmd(IOE_I2C,DISABLE);
  
  /* Clear DMA RX Transfer Complete Flag */
  DMA_ClearFlag(IOE_DMA_RX_STREAM,IOE_DMA_RX_TCFLAG);
  
  /* return a pointer to the IOE_Buffer */
  return 0;
}

uint8_t BusSelect(int8_t DeviceAddr, uint8_t channel)
{
	uint8_t temp;
  uint8_t IOE_BufferTX = 0;
  IOE_BufferTX = 1 << channel;
	
  /* Enable the I2C peripheral */
  I2C_GenerateSTART(IOE_I2C, ENABLE);
  
  /* Test on SB Flag */
  RGBC_TimeOut = TIMEOUT_MAX;
  while (I2C_GetFlagStatus(IOE_I2C,I2C_FLAG_SB) == RESET) 
  {
    if (RGBC_TimeOut-- == 0) 
			return(IOE_TimeoutUserCallback());
  }
  
  /* Transmit the slave address and enable writing operation */
  I2C_Send7bitAddress(IOE_I2C, DeviceAddr, I2C_Direction_Transmitter);
  
  /* Test on ADDR Flag */
  RGBC_TimeOut = TIMEOUT_MAX;
  while (!I2C_CheckEvent(IOE_I2C, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED))
  {
    if (RGBC_TimeOut-- == 0) 
			return(IOE_TimeoutUserCallback());
  }
  
  /* Transmit the first address for r/w operations */
  I2C_SendData(IOE_I2C, IOE_BufferTX);
  
  /* Test on TXE FLag (data dent) */
  RGBC_TimeOut = TIMEOUT_MAX;
  while ((!I2C_GetFlagStatus(IOE_I2C,I2C_FLAG_TXE)) && (!I2C_GetFlagStatus(IOE_I2C,I2C_FLAG_BTF)))  
  {
    if (RGBC_TimeOut-- == 0) 
			return(IOE_TimeoutUserCallback());
  }
	
	 /* Wait until BTF Flag is set before generating STOP */
  RGBC_TimeOut = TIMEOUT_MAX;
  while ((!I2C_GetFlagStatus(IOE_I2C,I2C_FLAG_BTF)))  
  {
		if (RGBC_TimeOut-- == 0) 
			return(IOE_TimeoutUserCallback());
  }
  
  /* Send STOP Condition */
  I2C_GenerateSTOP(IOE_I2C, ENABLE);
	
	//I2C_AcknowledgeConfig(IOE_I2C, DISABLE);
	
	//I2C_AcknowledgeConfig(IOE_I2C, DISABLE);
	/* Enable the I2C peripheral */
	
	
  I2C_GenerateSTART(IOE_I2C, ENABLE);
	
	/* Test on SB Flag */
  RGBC_TimeOut = TIMEOUT_MAX;
  while (I2C_GetFlagStatus(IOE_I2C,I2C_FLAG_SB) == RESET) 
  {
    if (RGBC_TimeOut-- == 0) return(IOE_TimeoutUserCallback());
  }
	
	 /* Transmit the slave address and enable writing operation */
  I2C_Send7bitAddress(IOE_I2C, DeviceAddr, I2C_Direction_Receiver);
	//I2C_NACKPositionConfig(I2C1, I2C_NACKPosition_Current);
	I2C_AcknowledgeConfig(IOE_I2C, DISABLE);
	
	/* Test on ADDR Flag */
  RGBC_TimeOut = TIMEOUT_MAX;
	while (!I2C_CheckEvent(IOE_I2C, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED))
  {
    if (RGBC_TimeOut-- == 0) 
			return(IOE_TimeoutUserCallback());
  }
	
	/* Test on SB Flag */
  RGBC_TimeOut = TIMEOUT_MAX;
  while (I2C_GetFlagStatus(IOE_I2C,I2C_FLAG_RXNE ) == RESET) 
  {
    if (RGBC_TimeOut-- == 0) return(IOE_TimeoutUserCallback());
  }
	
	temp = I2C_ReceiveData(IOE_I2C);

	/* Send STOP Condition */
  I2C_GenerateSTOP(IOE_I2C, ENABLE);
	
	if (temp == IOE_BufferTX){
		return 0;
	}else{
		return !0;
	}
}

//
// This function used init RGBC Sensor if successful return 0 other return !0;
//
uint8_t RGBCSensorInit(void)
{
	CPU_INT08U i;
//	CPU_INT08U temp;
//	static uint8_t buffer[8];
	for (i = 0; i < RGBCn; i++){
		BusSelect(0xe0,i);

		
		if (RGBC_I2C_WriteDeviceRegister((0x29 << 1),0xA0,0x03) != 0) return !0;

		if (RGBC_I2C_WriteDeviceRegister((0x29 << 1),0xA0,0x03) != 0) return !0;

		if (RGBC_I2C_WriteDeviceRegister((0x29 << 1),0xA1,0xFF) !=0) return !0;

		if (RGBC_I2C_WriteDeviceRegister((0x29 << 1),0xA3,0xFF) !=0) return !0;

		if (RGBC_I2C_WriteDeviceRegister((0x29 << 1),0xA4,0x00) !=0) return !0;

		if (RGBC_I2C_WriteDeviceRegister((0X29 << 1),0xAF,0x03) !=0) return !0;

	}
	return 0;
}



uint8_t ReadRGBCValue(RGBC * buffer, CPU_INT08U channel)
{
	if ((channel < 1) || (channel > RGBCn)) {
		return 1;
	}		
	
	buffer->blue = rgbc_sensor[channel -1].blue;
	buffer->clear = rgbc_sensor[channel -1].clear;
	buffer->green = rgbc_sensor[channel-1].green;
	buffer->red = rgbc_sensor[channel-1].red;
	
	return 0;
}


uint8_t UpdateRGBC(void)
{
	uint8_t i;
	uint8_t bu_temp[8];
//	uint8_t buffer[8];
	
	for(i = 0; i < RGBCn; i++) {
		
		BusSelect(0xe0,i);
		//buffer[0] = 1 << i;
		//BSP_I2C1_SendBytes(0x70,&buffer[0], 1);
		
		if (RGBC_I2C_ReadDataBuffer((0X29<<1), 0xB4, bu_temp, 8) != 0) return !0;
		//BSP_I2C1_SendBytesC(0x29, 0xB4, buffer, 8);
		rgbc_sensor[i].clear = bu_temp[0] + (bu_temp[1] << 8);
		rgbc_sensor[i].red = bu_temp[2] + (bu_temp[3] << 8);
		rgbc_sensor[i].green = bu_temp[4] + (bu_temp[5] << 8);
		rgbc_sensor[i].blue = bu_temp[6] + (bu_temp[7] << 8);
	}
	OSTimeDly(5);
	return 0;
}


static  void  ReadSensorTask (void *p_arg)
{
//	CPU_INT08U err;
	
	initI2C1();
	//BSP_I2C1_Init();
	RGBCSensorInit();
	for(;;) {
		
		UpdateRGBC();
		
	}
}
/*
void  BSP_I2C1_ISR_Handler (void)
{
		
}

void  BSP_I2C1_ER_ISR_Handler (void)
{
		
}
*/

void ReadSensorTaskCreate(void)
{
		OSTaskCreateExt((void (*)(void *)) ReadSensorTask,             /* Create the start task                                */
                    (void           *) 0,
                    (OS_STK         *)&AppTaskReadSensorStk[APP_TASK_READ_SENSOR_STK_SIZE - 1],
                    (INT8U           ) APP_TASK_READ_SENSOR_PRIO,
                    (INT16U          ) APP_TASK_READ_SENSOR_PRIO,
                    (OS_STK         *)&AppTaskReadSensorStk[0],
                    (INT32U          ) APP_TASK_READ_SENSOR_STK_SIZE,
                    (void           *) 0,
                    (INT16U          )(OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR));
}







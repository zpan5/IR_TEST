
/*
******************************************************************************************
*							Board Supply Pakage
*						ST Microelectronics STM32
*							on the 
*						Control board 1_2_1_v_1_1
*Filename		: w25q64.c
*Version		: V1.00
*Programer(s)	: 
*****************************************************************************************
*/
/*
*****************************************************************************************
*							Include Files
*****************************************************************************************
*/
#define  BSP_25Q64_MOUDLE
#include <bsp.h>
#include <bsp_w25q64.h>

/*
****************************************************************************************
*							Local Defines
****************************************************************************************
*/
#define W25Q64_CS_PORT						GPIOA
#define W25Q64_CS_PIN 						GPIO_Pin_4	
#define W25Q64_CS_PORT_CLK				RCC_AHB1Periph_GPIOA

#define SPI_1_CLK_PORT						GPIOA
#define SPI_1_CLK_PIN							GPIO_Pin_5 	
#define SPI_1_CLK_PORT_CLK				RCC_AHB1Periph_GPIOA
#define SPI_1_CLK_SOURCE  				GPIO_PinSource5

#define SPI_1_MOSI_PORT						GPIOA
#define SPI_1_MOSI_PIN						GPIO_Pin_7
#define SPI_1_MOSI_PORT_CLK				RCC_AHB1Periph_GPIOA
#define SPI_1_MOSI_SOURCE					GPIO_PinSource7

#define SPI_1_MISO_PORT						GPIOA
#define SPI_1_MISO_PIN						GPIO_Pin_6
#define SPI_1_MISO_PORT_CLK				RCC_AHB1Periph_GPIOA
#define SPI_1_MISO_SOURCE					GPIO_PinSource6

#define SPI_1_PORT								SPI1	
#define SPI_1_PORT_CLK						RCC_APB2Periph_SPI1

//#define VS1003B_CS_PORT						GPIOB
//#define VS1003B_CS_PIN						GPIO_Pin_0
//#define VS1003B_CS_PORT_CLK				RCC_AHB1Periph_GPIOB

//#define WIRELESS_2_4_CS_PORT			GPIOB
//#define WIRELESS_2_4_CS_PIN				GPIO_Pin_2
//#define WIRELESS_2_4_CS_PORT_CLK	RCC_AHB1Periph_GPIOB

/* Select SPI FLASH: Chip Select pin low  */
#define W25Q64_CS_LOW()       GPIO_ResetBits(W25Q64_CS_PORT, W25Q64_CS_PIN)
/* Deselect SPI FLASH: Chip Select pin high */
#define W25Q64_CS_HIGH()      GPIO_SetBits(W25Q64_CS_PORT, W25Q64_CS_PIN)


#define DUMMY						0xff
#define WRITE_ENABLE		0x06
#define WRITE_DISABLE		0x04
#define READ_STAT_REG		0x05
#define WRITE_STAT_REG	0x01
#define READ_DATA				0x03
#define FAST_READ				0x0b
#define PAGE_PROGRAM		0x02
#define BLOCK_ERASE			0xd8
#define SECTOR_ERASE		0x20
#define CHIP_ERASE			0xc7
#define POWER_DOWN			0xb9
#define DEVICE_ID				0x90


#define SPI_FLASH_SPI                           SPI1
#define SPI_FLASH_SPI_CLK                       RCC_APB2Periph_SPI1

#define SPI_FLASH_SPI_SCK_PIN                   GPIO_Pin_5                  /* PA.05 */
#define SPI_FLASH_SPI_SCK_GPIO_PORT             GPIOA                       /* GPIOA */
#define SPI_FLASH_SPI_SCK_GPIO_CLK              RCC_AHB1Periph_GPIOA
#define SPI_FLASH_SPI_SCK_SOURCE                GPIO_PinSource5

#define SPI_FLASH_SPI_MISO_PIN                  GPIO_Pin_6                  /* PA.06 */
#define SPI_FLASH_SPI_MISO_GPIO_PORT            GPIOA                       /* GPIOA */
#define SPI_FLASH_SPI_MISO_GPIO_CLK             RCC_AHB1Periph_GPIOA
#define SPI_FLASH_SPI_MISO_SOURCE               GPIO_PinSource6

#define SPI_FLASH_SPI_MOSI_PIN                  GPIO_Pin_7                  /* PA.07 */
#define SPI_FLASH_SPI_MOSI_GPIO_PORT            GPIOA                       /* GPIOA */
#define SPI_FLASH_SPI_MOSI_GPIO_CLK             RCC_AHB1Periph_GPIOA
#define SPI_FLASH_SPI_MOSI_SOURCE               GPIO_PinSource7

#define SPI_FLASH_CS_PIN                        GPIO_Pin_4                  /* PA.04 */
#define SPI_FLASH_CS_GPIO_PORT                  GPIOA                       /* GPIOC */
#define SPI_FLASH_CS_GPIO_CLK                   RCC_AHB1Periph_GPIOA

//#define SPI_2_4G_CS_PIN                         GPIO_Pin_2                  /* PB.02 */
//#define SPI_2_4G_CS_GPIO_PORT                   GPIOB                       /* GPIOB */
//#define SPI_2_4G_CS_GPIO_CLK                    RCC_AHB1Periph_GPIOB

//#define SPI_VS1003_CS_PIN                       GPIO_Pin_0                  /* PB.00 */
//#define SPI_VS1003_GPIO_PORT                    GPIOB                       /* GPIOB */
//#define SPI_VS1003_GPIO_CLK                     RCC_AHB1Periph_GPIOB


/***************************************************************************************
*							Local Constants
****************************************************************************************
*/
/*
****************************************************************************************
*							Local Data Types
****************************************************************************************
*/
/*
****************************************************************************************
*							Local Tables
****************************************************************************************
*/
/*
****************************************************************************************
*							Local Global Variables
****************************************************************************************
*/
/*
****************************************************************************************
*							Local Function Prototypes
****************************************************************************************
*/
CPU_INT08U BSP_SPI_1_SendByte(CPU_INT08U byte);
	

/*
*********************************************************************************************************
*                           Local Configuration Errors
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*********************************************************************************************************
**                                         Glabal Functions
*********************************************************************************************************
*********************************************************************************************************
*/



/*
*********************************************************************************************************
*									BSP_SPI_1_Init()
*Description	: Initialize W25q64.
*Argument(s)	: none.
*Return(s)		: none.
*Caller(s)		: Aplication.
*Note(s)		: none.
********************************************************************************************************
*/
void BSP_SPI_1_Init(void)
{
	SPI_InitTypeDef SPI_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
	
	RCC_AHB1PeriphClockCmd(W25Q64_CS_PORT_CLK , ENABLE);
	
	GPIO_InitStructure.GPIO_Pin		= W25Q64_CS_PIN;
	GPIO_InitStructure.GPIO_Mode	= GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_Speed	= GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_OType	= GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd	= GPIO_PuPd_UP;
	
	GPIO_Init(W25Q64_CS_PORT, &GPIO_InitStructure);
	GPIO_SetBits(W25Q64_CS_PORT, W25Q64_CS_PIN);
	
//	GPIO_InitStructure.GPIO_Pin		= SPI_1_MOSI_PIN;
//	GPIO_Init(SPI_1_MOSI_PORT, &GPIO_InitStructure);
//	GPIO_SetBits(SPI_1_MOSI_PORT, SPI_1_MOSI_PIN);
	
//	GPIO_InitStructure.GPIO_Pin		= WIRELESS_2_4_CS_PIN;
//	GPIO_Init(WIRELESS_2_4_CS_PORT,&GPIO_InitStructure);
//	RCC_AHB1PeriphClockCmd(WIRELESS_2_4_CS_PORT_CLK , ENABLE);
//	GPIO_SetBits(WIRELESS_2_4_CS_PORT, WIRELESS_2_4_CS_PIN);
//	
//	
//	GPIO_InitStructure.GPIO_Pin		= VS1003B_CS_PIN;
//	GPIO_Init(VS1003B_CS_PORT, &GPIO_InitStructure);
//	RCC_AHB1PeriphClockCmd(VS1003B_CS_PORT_CLK , ENABLE);
//	GPIO_SetBits(VS1003B_CS_PORT, VS1003B_CS_PIN);


	RCC_APB2PeriphClockCmd(SPI_1_PORT_CLK, ENABLE);
  
	RCC_AHB1PeriphClockCmd(SPI_1_MOSI_PORT_CLK | SPI_1_MISO_PORT_CLK | SPI_1_CLK_PORT_CLK, ENABLE);
	
	GPIO_InitStructure.GPIO_Pin		= SPI_1_MISO_PIN;
	GPIO_InitStructure.GPIO_Mode	= GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed	= GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_OType	= GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd	= GPIO_PuPd_NOPULL;
	GPIO_Init(SPI_1_MISO_PORT, &GPIO_InitStructure);
	
	
	GPIO_InitStructure.GPIO_Pin		= SPI_1_MOSI_PIN;
	GPIO_InitStructure.GPIO_Mode	= GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed	= GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_OType	= GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd	= GPIO_PuPd_NOPULL;
	
	GPIO_Init(SPI_1_MOSI_PORT, &GPIO_InitStructure);
	
//	GPIO_SetBits(SPI_1_MOSI_PORT, SPI_1_MOSI_PIN);
	
	GPIO_InitStructure.GPIO_Pin		= SPI_1_CLK_PIN;
	GPIO_Init(SPI_1_CLK_PORT, &GPIO_InitStructure);
	
	
							
	GPIO_PinAFConfig(SPI_1_MOSI_PORT, SPI_1_MOSI_SOURCE, GPIO_AF_SPI1);
	GPIO_PinAFConfig(SPI_1_MISO_PORT, SPI_1_MISO_SOURCE, GPIO_AF_SPI1);
	GPIO_PinAFConfig(SPI_1_CLK_PORT,  SPI_1_CLK_SOURCE,  GPIO_AF_SPI1);
	
	W25Q64_CS_HIGH();
	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_128;
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
	SPI_InitStructure.SPI_CRCPolynomial = 7;
	SPI_Init(SPI_1_PORT, &SPI_InitStructure);
//	RCC_APB2PeriphClockCmd(SPI_1_PORT_CLK, ENABLE);
	
	SPI_Cmd(SPI_1_PORT, ENABLE);

}

/*
*******************************************************************************************************
*							BSP_SPI_1_SendByte()
*Description	: Sends a byte through the SPI (SPI1) interface and return the byte received from the 
*				  SPI bus.
*Argument(s)	: byte  It is be sended data.
*Return(s)		: That is be received data.
*Caller(s)		: Application.
*Note(s)		: note.
******************************************************************************************************
*/
CPU_INT08U BSP_SPI_1_SendByte(CPU_INT08U byte)
{
	
	while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET){			/*Wait DR register is avarible*/
		;
	}
	
	SPI_I2S_SendData(SPI1, byte);		/*Send byte through the SPI1 interface*/
	
	while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET) {
		;
	}
	return SPI_I2S_ReceiveData(SPI1);
}



/*
*********************************************************************************************************
*									BSP_SPI_W25Q64_ReadID()
*Description	: On the SPI bus reads the flash idenctification. 
*Argument(s)	: none.
*Return(s)		: none.
*Caller(s)		: Aplication.
*Note(s)		: none.
********************************************************************************************************
*/
CPU_INT16U BSP_SPI_W25Q64_ReadID(void)
{
	CPU_INT08U temp = 0;
	static CPU_INT16U temp2 = 0;
	W25Q64_CS_LOW();

	BSP_SPI_1_SendByte(DEVICE_ID);
	
	for(temp = 0; temp < 2; temp++) {
		BSP_SPI_1_SendByte(0x00);
	}
	BSP_SPI_1_SendByte(0x00);
	temp2 |= (BSP_SPI_1_SendByte(DUMMY) << 8) & 0xff00;
	temp2 |= BSP_SPI_1_SendByte(DUMMY) & 0xff;
	
	W25Q64_CS_HIGH();
	
	return temp2;
}

/*******************************************************************************************************
*						BSP_SPI_W25Q64_WriteCmd()
*Description	: W25Q64 write enable.
*Argument(s)	: newstate  new state of the W25Q64 Write sate.
					This parameter can be: ENABLE or DISABLE.
*Return(s)		: none.
*Caller(s)		: Application.
*Note(s)		: none.
*******************************************************************************************************
*/
void BSP_SPI_W25Q64_WriteCmd(FunctionalState newstate)
{
	W25Q64_CS_LOW();
	if (newstate == ENABLE) {
		BSP_SPI_1_SendByte(WRITE_ENABLE);
	}
	else if (newstate == DISABLE) {
		BSP_SPI_1_SendByte(WRITE_DISABLE);
	}
	W25Q64_CS_HIGH();
}

/***************************************************************************************************
*						BSP_SPI_W25Q64_ReadByte()
*Description	: From specific address retrieved a byte.
*Argument(s)	: addr  address of the reads data.
*Return(s)		: From the specifical address retrieved data.
*Caller(s)		: Application.
*Note(s)		: none.
***************************************************************************************************
*/
CPU_INT08U BSP_SPI_W25Q64_ReadByte(CPU_INT32U addr)
{
	W25Q64_CS_LOW();
	BSP_SPI_1_SendByte(READ_DATA);
	BSP_SPI_1_SendByte((CPU_INT08U)(addr & 0xff0000) >> 16);
	BSP_SPI_1_SendByte((CPU_INT08U)(addr & 0xff00) >> 8);
	BSP_SPI_1_SendByte((CPU_INT08U)(addr & 0xff));
	W25Q64_CS_HIGH();
	return (BSP_SPI_1_SendByte(DUMMY));
}

/************************************************************************************************
*						BSP_SPI_W25Q64_ReadStr()
*Description	: From the specifical start read a string data(fixed lenght).
*Argument(s)	: addr  read start address.
*				  p_str  pointer It is be used save the readed data.
*				  len	need to read data amount.
*Return(s)		: none.
*Caller(s)		: Application.
*Note(s)		: (1) UP to 256 bytes.
***********************************************************************************************
*/
void BSP_SPI_W25Q64_ReadStr(CPU_INT32U addr, CPU_INT08U *p_str, CPU_INT08U len)
{
	CPU_INT08U i;
	W25Q64_CS_LOW();
	BSP_SPI_1_SendByte(FAST_READ);
	BSP_SPI_1_SendByte((CPU_INT08U)(addr & 0xff0000) >> 16);
	BSP_SPI_1_SendByte((CPU_INT08U)(addr & 0xff00) >> 8);
	BSP_SPI_1_SendByte((CPU_INT08U)(addr & 0xff));
	for (i = 0; i < len; i++) {
		*p_str++ = (CPU_INT08U) BSP_SPI_1_SendByte(DUMMY);
	}
	W25Q64_CS_HIGH();
}

/*
*****************************************************************************************************
*						BSP_SPI_W25Q64_EraseSector()
*Description	: The function sets all memory within a specified sector(4K-bytes) of w25q64 to the 
*				  erased state of all 1s.
*Argument(s)	: addr  start address.
*Return(s)		: none.
*Caller(s)		: none.
*****************************************************************************************************
*/
void BSP_SPI_W25Q64_EreaseSector(CPU_INT32U addr)
{
	CPU_INT32U i;
	
	i = addr &0xfffff000;
	
	W25Q64_CS_LOW();
	
	BSP_SPI_1_SendByte(SECTOR_ERASE);
	BSP_SPI_1_SendByte((CPU_INT08U)(i & 0xff0000) >> 16);
	BSP_SPI_1_SendByte((CPU_INT08U)(i & 0xff00) >> 8);
	BSP_SPI_1_SendByte((CPU_INT08U)(i & 0xff));
	
	W25Q64_CS_HIGH();
}

/*
********************************************************************************************************
*						BSP_SPI_W25Q64_PageProgram()
*Description	: The function allows up to 256 bytes of data to be programmed at previously erased to 
*				  all 1s (ffh) memory locations.
*Argument(s)	: p_str Pointer point the programmed data.
*				  num   Amount of programed data.
*				  addr  start addree.
*Return(s)		: none.
*Caller(s)		: Application.
*Note(s)		: (1)Before executes this function the BSP_SPI_W25Q64_WriteCmd(Enable) should executes
*				  (2)If an entrie 256 byte page is to be programmed, the last address bytes byte(the 8 
*					 least significant address bits) should be set to 0.
*******************************************************************************************************
*/
void BSP_SPI_W25Q64_PageProgram(CPU_INT32U addr, CPU_INT08U *p_str, CPU_INT08U num)
{
	CPU_INT08U i;
	//if ((addr &0x01) == 0x01) return;
	W25Q64_CS_LOW();
	
	BSP_SPI_1_SendByte(PAGE_PROGRAM);
	
	BSP_SPI_1_SendByte((CPU_INT08U) ((addr >> 16) & 0xff));
	BSP_SPI_1_SendByte((CPU_INT08U) ((addr >> 8 ) & 0xff));
	BSP_SPI_1_SendByte((CPU_INT08U) (addr * 0xff));
	for (i = 0; i < num; i++) {
		*p_str++ = BSP_SPI_1_SendByte(DUMMY);
	}
	W25Q64_CS_LOW();
}


void BSP_SPI_W25Q64_Test(void)
{
	
	BSP_SPI_1_Init();

	BSP_SPI_W25Q64_ReadID();
}

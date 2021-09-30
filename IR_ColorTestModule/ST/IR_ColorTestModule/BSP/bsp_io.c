




#include "bsp_io.h"


/******************************************************************************************************
***********************************************Local define********************************************
******************************************************************************************************/
#define BSP_GPIO_OUTPUT_NUMBER		8
/******************************************************************************************************
***********************************************Local type define***************************************
******************************************************************************************************/

typedef struct{
  GPIO_TypeDef* GPIO_port;
  INT16U GPIO_pin;
}BSP_GPIO_TypeDef;


/******************************************************************************************************
***********************************************Local Variable******************************************
******************************************************************************************************/

BSP_GPIO_TypeDef BSP_OutputPort[BSP_GPIO_OUTPUT_NUMBER] = {
GPIOC, GPIO_Pin_0,
GPIOC, GPIO_Pin_1,
GPIOC, GPIO_Pin_2,
GPIOC, GPIO_Pin_3,
GPIOC, GPIO_Pin_4,
GPIOC, GPIO_Pin_13,
GPIOC, GPIO_Pin_14,
GPIOC, GPIO_Pin_15,
};

/******************************************************************************************************
***********************************************Local Function prototype********************************
******************************************************************************************************/




void BSP_IO_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;

	for ( INT8U i = 0; i < BSP_GPIO_OUTPUT_NUMBER; i++) {
		GPIO_ResetBits(BSP_OutputPort[i].GPIO_port,BSP_OutputPort[i].GPIO_pin );
		GPIO_InitStructure.GPIO_Pin = BSP_OutputPort[i].GPIO_pin;
		GPIO_Init(BSP_OutputPort[i].GPIO_port, &GPIO_InitStructure);
	}
	
}
void BSP_IO_Set(INT8U number)
{
	GPIO_SetBits(BSP_OutputPort[number-1].GPIO_port, BSP_OutputPort[number-1].GPIO_pin);

}
void BSP_IO_Reset(INT8U number)
{
	GPIO_ResetBits(BSP_OutputPort[number-1].GPIO_port, BSP_OutputPort[number-1].GPIO_pin);

}


INT8U BSP_IO_Read(INT8U number)
{
	return GPIO_ReadOutputDataBit(BSP_OutputPort[number-1].GPIO_port, BSP_OutputPort[number-1].GPIO_pin);
}



/*
******************************************************************************************
*							Board Supply Pakage
*						ST Microelectronics STM32
*							on the 
*						AIO_V1.0 BOARD
*Filename		: bsp_aio.c
*Version		: V1.00
*Programer(s)	: ZhuPan
*****************************************************************************************
*/
/*
*****************************************************************************************
*							Include Files
*****************************************************************************************
*/
#include <bsp.h>
#include <bsp_aio.h>
/*
****************************************************************************************
*							Local Defines
****************************************************************************************
*/


/***************************************************************************************
*							Local Constants
****************************************************************************************
*/
#define AIn 9
#define AOn 2
#define AI_MF_N  32

#define AO_CLK	RCC_APB1Periph_DAC
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
OS_EVENT *AI_Event;
CPU_INT16U AI_value[AIn];
CPU_INT16U AI_valueR[AIn];
CPU_INT32U AI_valueF[AIn];
//CPU_INT16U AI_valueFM[AIn][AI_MF_N];
GPIO_TypeDef*     AI_PORT[AIn] = {	GPIOD,
																
												};

const CPU_INT16U  AI_PIN[AIn]  = {	GPIO_Pin_13,
													
	
												};
const CPU_INT32U  AI_PORT_CLK[AIn]  = {	RCC_AHB1Periph_GPIOD,
													
												};
const CPU_INT08U	AI_CHANNEL[AIn] = {	ADC_Channel_3,
																			ADC_Channel_2,
																			ADC_Channel_1,
																			ADC_Channel_0,
																			ADC_Channel_13,
																			ADC_Channel_12,
																			ADC_Channel_11,
																			ADC_Channel_10,
																			ADC_Channel_6
																			};

GPIO_TypeDef*     AO_PORT[AOn] = {	GPIOA,
												GPIOA				
												};

const CPU_INT16U  AO_PIN[AOn]  = {	GPIO_Pin_4,
												GPIO_Pin_5
												};
const CPU_INT32U  AO_PORT_CLK[AOn]  = {	RCC_AHB1Periph_GPIOA,
														RCC_AHB1Periph_GPIOA
												};

AI_FactorTypedef AI_factor[AIn]= {{4, 273, -29.9},
																	{4, 273, -29.9},
																	{4, 273, -29.9},
																	{4, 273, -29.9},
																	{4, 273, -29.9},
																	{4, 273, -29.9},
																	{4, 273, -29.9},
																	{4, 273, -29.9},
																	{15, 273, 0}};


/*
****************************************************************************************
*							Local Function Prototypes
****************************************************************************************
*/
void BSP_ADC1_2_ISR_Handler(void);	
																	
void BSP_ADC1_DMA_ISR_Handler(void);

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
*									CPU_INT08U AO_Init(void)
*Description	: Initialize AO.
*Argument(s)	: NO.
*Return(s)		: NO.
*Caller(s)		: Aplication.
*Note(s)		: none.
********************************************************************************************************
*/
CPU_INT08U AO_Init(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;
	DAC_InitTypeDef  DAC_InitStructure;
	
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_DAC, ENABLE);	
	RCC_AHB1PeriphClockCmd(AO_PORT_CLK[0], ENABLE);
	
	GPIO_InitStructure.GPIO_Pin = AO_PIN[0];
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AN;
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;
	GPIO_Init(AO_PORT[0], &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = AO_PIN[1];
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AN;
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;
	GPIO_Init(AO_PORT[1], &GPIO_InitStructure);
	
	DAC_DeInit();
	
	DAC_InitStructure.DAC_LFSRUnmask_TriangleAmplitude = DAC_LFSRUnmask_Bit0;
	DAC_InitStructure.DAC_OutputBuffer = DAC_OutputBuffer_Enable;
	DAC_InitStructure.DAC_Trigger = DAC_Trigger_None;
	DAC_InitStructure.DAC_WaveGeneration = DAC_WaveGeneration_None;
	
	DAC_Init(DAC_Channel_1, &DAC_InitStructure);
	/* Enable DAC Channel2 */
	DAC_Cmd(DAC_Channel_1, ENABLE);
	/* Set DAC Channel1 DHR12L register */
	DAC_SetChannel1Data(DAC_Align_12b_R, 0x0000);
	
	DAC_Init(DAC_Channel_2, &DAC_InitStructure);
	 /* Enable DAC Channel2 */
	DAC_Cmd(DAC_Channel_2, ENABLE);
	/* Set DAC Channel2 DHR12R register */
	DAC_SetChannel2Data(DAC_Align_12b_R, 0x0000);

//	WriteAO(AO_CHANNEL0, 1);
//	WriteAO(AO_CHANNEL1, 1);
	return 0;
}

/*
*********************************************************************************************************
*									CPU_INT08U WriteAO(CPU_INT08U channel, CPU_INT16U data);
*Description	: Write data to specified channel.
*Argument(s)	: channel indicate which channel need to be configuration.
						model  indicate which channel work module
						data A point used store data be read or write
*Return(s)		: The operation whether success.
*Caller(s)		: Aplication.
*Note(s)		: none.
********************************************************************************************************
*/
CPU_INT08U WriteAO(CPU_INT08U channel, CPU_INT16U data)
{
	if((channel != AO_CHANNEL0 && channel != AO_CHANNEL1) || data > 0xfff) return 0;
	if(channel == AO_CHANNEL0){
		DAC_SetChannel1Data(DAC_Align_12b_R, (CPU_INT32U) data);
	}else if(channel == AO_CHANNEL1){
		DAC_SetChannel2Data(DAC_Align_12b_R, (CPU_INT32U) data);
	}
	
	return 1;
}


/*
*********************************************************************************************************
*									CPU_INT08U AI_Init(void)
*Description	: Initialize AO.
*Argument(s)	: NO.
*Return(s)		: NO.
*Caller(s)		: Aplication.
*Note(s)		: none.
********************************************************************************************************
*/
CPU_INT08U AI_Init(void)
{	
	INT8U buffer[4];
	ADC_InitTypeDef       ADC_InitStructure;
	ADC_CommonInitTypeDef ADC_CommonInitStructure;
	GPIO_InitTypeDef 			GPIO_InitStructure;
	DMA_InitTypeDef 			DMA_InitStructure;
	INT8U i;
	
	AI_Event = OSSemCreate(0);
	
	/* Enable peripheral clocks */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOB | RCC_AHB1Periph_GPIOC, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1 , ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2,ENABLE );
	
	DMA_InitStructure.DMA_Channel = DMA_Channel_0; 
  DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)&AI_valueR;//&ADCDualConvertedValue;
  DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&ADC1->DR;//ADC_CCR_ADDRESS;
  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;
  DMA_InitStructure.DMA_BufferSize = AIn;
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
  DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
  DMA_InitStructure.DMA_Priority = DMA_Priority_High;
  DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;         
  DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_HalfFull;
  DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
  DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
  DMA_Init(DMA2_Stream0, &DMA_InitStructure);

  /* DMA2_Stream0 enable */
  DMA_Cmd(DMA2_Stream0, ENABLE);
	DMA_ITConfig(DMA2_Stream0,DMA_IT_TC,ENABLE);
	BSP_IntVectSet		(BSP_INT_ID_DMA2_CH1 , BSP_ADC1_DMA_ISR_Handler);
	BSP_IntEn			(BSP_INT_ID_DMA2_CH1);

	/* Configure ADC Channel All pin as analog input */ 
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 |GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL ;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_6 ;//| GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL ;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
/*	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL ;
	GPIO_Init(GPIOB, &GPIO_InitStructure);*/
	
	/* ADC Common configuration *************************************************/
	ADC_CommonInitStructure.ADC_Mode = ADC_DualMode_RegSimult; //ADC_Mode_Independent;
	ADC_CommonInitStructure.ADC_TwoSamplingDelay = ADC_TwoSamplingDelay_20Cycles;
	ADC_CommonInitStructure.ADC_DMAAccessMode = ADC_DMAAccessMode_Disabled;  
	ADC_CommonInitStructure.ADC_Prescaler = ADC_Prescaler_Div6; 
	ADC_CommonInit(&ADC_CommonInitStructure);
	
	
	/* ADC1 regular channel 12 configuration ************************************/
	ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;
	ADC_InitStructure.ADC_ScanConvMode =ENABLE;// DISABLE;
	ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;//DISABLE;£E
	ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None;
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T1_CC1;
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
	ADC_InitStructure.ADC_NbrOfConversion = AIn;
	ADC_Init(ADC1, &ADC_InitStructure);
	for (i = 0; i < AIn; i ++) {
		ADC_RegularChannelConfig(ADC1, AI_CHANNEL[i], i+1, ADC_SampleTime_480Cycles);
	}
	
	/*Enable interrupt of ADC 1*/
//	ADC_ITConfig(ADC1, ADC_IT_EOC, ENABLE);
//	BSP_IntVectSet		(BSP_INT_ID_ADC1_2 , BSP_ADC1_2_ISR_Handler);
//	BSP_IntEn			(BSP_INT_ID_ADC1_2 );
	/* Enable ADC1 **************************************************************/
	/* Enable ADC1 DMA */
	ADC_DMARequestAfterLastTransferCmd(ADC1, ENABLE);
  ADC_DMACmd(ADC1, ENABLE);
//	ADC_Cmd(ADC1, ENABLE);
//	/* Start ADC1 Software Conversion */ 
//	ADC_SoftwareStartConv(ADC1);

	BSP_SPI_W25Q64_ReadStr(0x1000, (CPU_INT08U*) buffer, 0x04);
	if ((buffer[0] == 0xff && buffer[1] == 0xff && buffer[2] == 0xff && buffer[3] == 0xff) || 
			(buffer[0] == 0x00 && buffer[1] == 0x00 && buffer[2] == 0x00 && buffer[3] == 0x00)){
		AI_FactorSave();
	}else {
		BSP_SPI_W25Q64_ReadStr(0x1000, (CPU_INT08U*) AI_factor, sizeof (AI_factor));
	}
	return 1;
}
void swap(CPU_INT16U *p,CPU_INT16U *q) {
   int t;
   
   t=*p; 
   *p=*q; 
   *q=t;
}
void sort(CPU_INT16U a[],int n) { 
   int i,j;

   for(i = 0;i < n-1;i++) {
      for(j = 0;j < n-i-1;j++) {
         if(a[j] > a[j+1])
            swap(&a[j],&a[j+1]);
      }
   }
}
CPU_INT08U channeln = 0;
CPU_INT16U channelnF = 0;
void BSP_ADC1_2_ISR_Handler(void)
{
	if( ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC) == SET) {
		ADC_ClearFlag(ADC1, ADC_FLAG_EOC);
		
		//AI_valueFM[channeln][channelnF] = ADC_GetConversionValue(ADC1);
		AI_valueF[channeln] += ADC_GetConversionValue(ADC1);
		//AI_valueF[channeln++] += AI_valueFM[channel][channelnF];
		channeln++;
		if (channeln == AIn) {
			channeln = 0;
			channelnF++;
		}
		if (channelnF == 256)
		{
			CPU_INT08U i;
			for (i = 0; i < AIn; i++)
			{
				AI_value[i] = AI_valueF[i] >> 8;
				AI_valueF[i] = 0;
			}
			channelnF = 0;
		}
		ADC_RegularChannelConfig(ADC1, AI_CHANNEL[channeln], 1, ADC_SampleTime_480Cycles);
		ADC_SoftwareStartConv(ADC1);
	}
}

void BSP_ADC1_DMA_ISR_Handler(void)
{
	INT8U i;
	DMA_ClearITPendingBit(DMA2_Stream0, DMA_IT_TCIF0);
	for(i = 0; i < AIn; i++) {
		AI_valueF[i] += AI_valueR[i];
	}
	if ( ++channelnF == 256)
	{
		ADC_Cmd(ADC1, DISABLE);
		channelnF = 0;
		for (i = 0; i < AIn; i++)
		{
			AI_value[i] = AI_valueF[i] >> 8;
			AI_valueF[i] = 0;
		}
			OSSemPost(AI_Event);
	}
}
/*
*********************************************************************************************************
*									CPU_INT08U WriteAO(CPU_INT08U channel, CPU_INT16U data);
*Description	: Read or write data with specified channel.
*Argument(s)	: channel indicate which channel need to be configuration.
						model  indicate which channel work module
						data A point used store data be read or write
*Return(s)		: The operation whether success.
*Caller(s)		: Aplication.
*Note(s)		: none.
********************************************************************************************************
*/
CPU_INT16U ReadAI(CPU_INT08U channel)
{
	return AI_value[channel];
}

/*
*********************************************************************************************************
*									FP32 ReadAI_True(CPU_INT08U channel)
*Description	: Read the specified channel valve of AI; The valve is be converted to true valve corresponded
								to environment.  ref to TestBoardV1.0
*Argument(s)	: channel indicate which channel need to be configuration.
						model  indicate which channel work module
						data A point used store data be read or write
*Return(s)		: The operation whether success.
*Caller(s)		: Aplication.
*Note(s)		: none.
********************************************************************************************************
*/
FP32 ReadAI_True(CPU_INT08U channel)
{
//	FP32 f;
	CPU_INT08U err;
	ADC_Cmd(ADC1, ENABLE);
	/* Start ADC1 Software Conversion */ 
	ADC_SoftwareStartConv(ADC1);
	OSSemPend(AI_Event,0, &err);
	return (FP32)(AI_value[channel] * AI_factor[channel].a )/AI_factor[channel].c + AI_factor[channel].b;
}


void AI_FactorSet(CPU_INT08U ch, CPU_INT08U factor, FP32 value)
{
	if (factor == AI_FACTOR_A) {
		AI_factor[ch].a = value;
	}else if (factor == AI_FACTOR_B){
		AI_factor[ch].b = value;
	}else if (factor == AI_FACTOR_C) {
		AI_factor[ch].c = value;
	}
}
FP32 AI_FactorRead(CPU_INT08U ch, CPU_INT08U factor)
{
	if (factor == AI_FACTOR_A) {
		return AI_factor[ch].a;
	}else if (factor == AI_FACTOR_B){
		return AI_factor[ch].b;
	}else if (factor == AI_FACTOR_C) {
		return AI_factor[ch].c ;
	}
	return 0;
}



CPU_INT08U AI_FactorSave(void)
{
	AI_FactorTypedef AI_factorRead[AIn];
	INT8U i;
	BSP_SPI_W25Q64_EreaseSector(0x1000);
	BSP_SPI_W25Q64_PageProgram(0x1000, (CPU_INT08U*) &AI_factor, sizeof (AI_factor));
	BSP_SPI_W25Q64_ReadStr(0x1000, (CPU_INT08U*) &AI_factorRead, sizeof (AI_factorRead));
	for (i = 0; i < AIn; i++)
	{
		if (AI_factor[i].a != AI_factorRead[i].a  
			&& AI_factor[i].b != AI_factorRead[i].b
			&& AI_factor[i].c != AI_factorRead[i].c)
		return 0x01;
	}
	return 0x00;
}











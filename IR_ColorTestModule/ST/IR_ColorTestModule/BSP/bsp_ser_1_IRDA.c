
/*
*********************************************************************************************************
*
*                                    MICRIUM BOARD SUPPORT PACKAGE
*                                       SERIAL (UART) INTERFACE
*
*                                     ST Microelectronics STM32
*                                              on the
*
*                                           TestBoardV1.0
*                                         Evaluation Board
*
* Filename      : BSP_Ser_1_IRDA.c
* Version       : V1.00
* Programmer(s) : SL
* Brief					: RS485
*********************************************************************************************************
*/


#define  BSP_SER_MODULE
#include <bsp.h>
#include <bsp_ser_1_IRDA.h>



/*
**********************************************************************************
*						LOCAL GLOBAL VARIABLES
**********************************************************************************
*/
static BSP_OS_SEM		BSP_Ser_1_IRDA_TxWait;
static BSP_OS_SEM		BSP_Ser_1_IRDA_TxLock;

static BSP_OS_SEM		BSP_Ser_1_IRDA_RxWait;
static BSP_OS_SEM 	BSP_Ser_1_IRDA_RxLock;
static CPU_INT08U		BSP_Ser_1_IRDA_RxData;
void (*BSP_Ser_1_IRDA_RdCallback)(uint8_t strength);

CPU_CHAR 		tx_buf[81u];
CPU_INT08U  tx_bufSize;
CPU_INT08U  tx_bufCounter;

uint16_t TimerPeriod = 0;
uint16_t Channel1Pulse = 0;
IR_LED_STATUS IR_LED_status;



/*
**********************************************************************************
*						LOCAL FUNCTION PROTOTYPES
**********************************************************************************
*/


void BSP_Ser_1_IRDA_ISR_Handler(void);
void BSP_Ser_1_IRDA_WrStr(CPU_CHAR *p_str);
void BSP_Ser_1_IRDA_WrByteUnlocked ( CPU_INT08U c);

void Time_Init(void);
/*
**********************************************************************************
*						BSP_Ser_1_IRDA_Init()
*
*Description:Initialize a serial port for communication
*Argument(s):baud_rate 			The desier RS232 baud rate.
*return(s)	:none.
*Caller(s)	:Application.
*
*Note(s)	:none.
***********************************************************************************
*/
void BSP_Ser_1_IRDA_Init(CPU_INT32U baud_rate)
{
	GPIO_InitTypeDef	GPIO_InitStructure;
	USART_InitTypeDef	USART_InitStructure;
	USART_ClockInitTypeDef USART_CLK_InitStructure;
	
	BSP_OS_SemCreate(&BSP_Ser_1_IRDA_TxWait, 0, "Serial 1 Tx Wait");
	BSP_OS_SemCreate(&BSP_Ser_1_IRDA_TxLock, 1, "Serial 1 Tx Lock");
	
	BSP_OS_SemCreate(&BSP_Ser_1_IRDA_RxWait, 0, "Serial 1 Rx Wait");
	BSP_OS_SemCreate(&BSP_Ser_1_IRDA_RxLock,	1,  "Serial 1 Lock");
	
	USART_InitStructure.USART_BaudRate 				= baud_rate;
	USART_InitStructure.USART_WordLength 			= USART_WordLength_8b;
	USART_InitStructure.USART_StopBits				= USART_StopBits_1;
	USART_InitStructure.USART_Parity					= USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl	= USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode					= USART_Mode_Rx | USART_Mode_Tx;
	
	USART_CLK_InitStructure.USART_Clock				= USART_Clock_Disable;
	USART_CLK_InitStructure.USART_CPOL				= USART_CPOL_Low;
	USART_CLK_InitStructure.USART_CPHA				= USART_CPHA_2Edge;
	USART_CLK_InitStructure.USART_LastBit			= USART_LastBit_Disable;
	

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
	
	
															/************ SETUP USART1 GPIO ****************/
															/*********** Configure GPIOA.9 as push-pull*****/
	GPIO_InitStructure.GPIO_Pin		= GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Mode	= GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_OType	= GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_PuPd	= GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource9, GPIO_AF_USART1);
	
															/**Configure GPIOA.10 input floating.**********/
	GPIO_InitStructure.GPIO_Pin 	= GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Mode 	= GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_PuPd	= GPIO_PuPd_NOPULL;
	GPIO_Init			(GPIOA, &GPIO_InitStructure);
	GPIO_PinAFConfig	(GPIOA, GPIO_PinSource10, GPIO_AF_USART1);
	


															/**Setup USART1*******************************/
	USART_Init			(USART1, &USART_InitStructure);
	USART_ClockInit		(USART1, &USART_CLK_InitStructure);
	USART_Cmd			(USART1, ENABLE);
	//USART_IrDAConfig(USART1, USART_IrDAMode_Normal);
	//USART_IrDAConfig(USART1, USART_IrDAMode_Normal);
	//USART_IrDAConfig(USART1, USART_IrDAMode_LowPower);
	//USART_SetPrescaler(USART1,0x0f);
	//USART_IrDACmd(USART1,ENABLE);
	

	
	BSP_IntVectSet		(BSP_INT_ID_USART1, BSP_Ser_1_IRDA_ISR_Handler);
	BSP_IntEn					(BSP_INT_ID_USART1);
	
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
	Time_Init();
	
	IR_LED_status  = IR_DATA_MODULE;
}


/******************************************************************************************
*						BSP_Ser_ISR_Handler()
*
*Description:	Serial 1 ISR.
*Argument(s):	none
*Return(s)	:	none
*Caller(s)	:	This is a ISR.
*Note(s)	:	none.
*******************************************************************************************
*/
void BSP_Ser_1_IRDA_ISR_Handler(void) 
{
	CPU_REG32	status;
	CPU_REG32   interrupt_set_bit;
	
	status = USART1->SR;
	interrupt_set_bit = USART1->CR1;
	if(DEF_BIT_IS_SET(interrupt_set_bit, DEF_BIT_05)){
		if(DEF_BIT_IS_SET(status, DEF_BIT_05)) {
			BSP_Ser_1_IRDA_RxData = USART_ReceiveData(USART1) & 0xFF;
			
			if (BSP_Ser_1_IRDA_RdCallback != NULL) {
				BSP_Ser_1_IRDA_RdCallback(BSP_Ser_1_IRDA_RxData);
			}
       
			 //
			 if (BSP_Ser_1_IRDA_RxWait->OSEventGrp != 0) {
				 BSP_OS_SemPost(&BSP_Ser_1_IRDA_RxWait);
			 }
		}
	}
	if(DEF_BIT_IS_SET(interrupt_set_bit, DEF_BIT_07)){
		if(DEF_BIT_IS_SET(status, DEF_BIT_07) ){
			if (BSP_Ser_1_IRDA_TxWait->OSEventGrp != NULL) {
				BSP_OS_SemPost(&BSP_Ser_1_IRDA_TxWait);
				USART_ITConfig(USART1, USART_IT_TXE, DISABLE);
			}else {
				if (tx_buf[tx_bufCounter] != '\0') {
					USART_SendData(USART1, tx_buf[tx_bufCounter++]);
				}else {
					USART_ITConfig(USART1, USART_IT_TXE, DISABLE);
					USART_ITConfig(USART1, USART_IT_TC, ENABLE);
					
				}
			}
			
		}
	}
	
	if(DEF_BIT_IS_SET(interrupt_set_bit,  DEF_BIT_06)) {
		if(DEF_BIT_IS_SET(status, DEF_BIT_06) ){
			USART_ClearFlag(USART1, USART_FLAG_TC);
			BSP_OS_SemPost(&BSP_Ser_1_IRDA_TxLock);
			USART_ITConfig(USART1, USART_IT_TC, DISABLE);
			Channel1Pulse = (uint16_t) (((uint32_t) 3 * (TimerPeriod - 1)) / 3);
			TIM_SetCompare2(TIM3, Channel1Pulse);
		}
	}
}

/*
**************************************************************************************************
*						BSP_Ser_1_IRDA_RdByteUnlocked()
*Description:	Receive a single byte from serial port (USART1).
*Argument(s):	delay  receive timeout.
*Return(s):		The received byte.
*Caller(s):		BSP_Ser_RdByte().
*
*Note(s):		none.
*************************************************************************************************
*/

CPU_INT08U BSP_Ser_1_IRDA_RdByteUnlocked(CPU_INT32U delay, CPU_INT08U *err)
{
	CPU_INT08U rx_byte;
	//USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
	*err = BSP_OS_SemWait(&BSP_Ser_1_IRDA_RxWait, delay);
	//USART_ITConfig(USART1, USART_IT_RXNE, DISABLE);

   if (*err == DEF_OK){
		rx_byte = BSP_Ser_1_IRDA_RxData;
		return rx_byte;
	}
	else{
		return 0;
	}
}

/*
**************************************************************************************************
*						BSP_Ser_1_IRDA_RxByte()
*Description:	The function read a byte from USART1;
*Argument(s):	none
*Return(s):		The uart1 received a byte;
*Caller(s):		Application.
*Note(s):		(1) This function blocks until a data is received.
*				(2)	It can not be called frome an ISR.
*************************************************************************************************
*/
CPU_INT08U BSP_Ser_1_IRDA_RdByte(void)
{
	CPU_INT08U rx_byte;
	CPU_INT08U err;
	
	BSP_OS_SemWait(&BSP_Ser_1_IRDA_RxLock, 0);
	
	BSP_Ser_1_IRDA_RdByteUnlocked(0, &err);
	
	BSP_OS_SemPost(&BSP_Ser_1_IRDA_RxLock);
	
	return rx_byte;
}

/*	
**************************************************************************************************
*						BSP_Ser_1_IRDA_Printf()
*Description:	Formatted output to serial port.
*				This function code writs a string to a serial port. This call blocks until a  
*				character appears at the port and the last character is a Carriage.
*Argument(s):	Format string flowing the c format convention.
*Return(s):		none.
*Caller(s):		Application.
*Note(s):		none.
***************************************************************************************************
*/

IR_LED_STATUS BSP_Ser_1_IRDA_Printf(CPU_CHAR *format, ...)
{
	if (IR_LED_status != IR_DATA_MODULE) return IR_LED_status;
	//CPU_CHAR 	buf_str[81u];
	va_list		v_args;
	BSP_OS_SemWait(&BSP_Ser_1_IRDA_TxLock, 0);
	va_start(v_args, format);
	
	 tx_bufSize = vsnprintf((char *) &tx_buf[0]
					 ,(CPU_SIZE_T) sizeof(tx_buf)
					 ,(char const *) format
					 ,v_args);
	va_end(v_args);
	tx_bufCounter = 0;
	
					 
	Channel1Pulse = (uint16_t) (((uint32_t) 2 * (TimerPeriod - 1)) / 3);
	TIM_SetCompare2(TIM3, Channel1Pulse);
	if ( TIM_GetCounter(TIM3) != 0) {
		TIM_SetCounter(TIM3, 0);
	}
	
	USART_ITConfig(USART1, USART_IT_TXE, ENABLE);
	
	
	return IR_LED_status;
}

/*
*************************************************************************
*					BSP_Ser_1_IRDA_WrByteUnlocked()
*Description:	Writes a single byte to a serial port.
*Argument(s):	c character to output.
*return(s):		none.
*Caller(s):		
*Note(s):		(1) This function blocks until room is available in the UART 
*				for the byte to be sent.
*************************************************************************
*/
void BSP_Ser_1_IRDA_WrByteUnlocked(CPU_INT08U c)
{
	USART_SendData(USART1, c);
	USART_ITConfig(USART1, USART_IT_TXE, ENABLE);
	BSP_OS_SemWait(&BSP_Ser_1_IRDA_TxWait, 10);
	USART_ITConfig(USART1, USART_IT_TXE, DISABLE);
}
/*
**************************************************************************
*					BSP_Ser_1_IRDA_WrByte()
*Description:	Writes a single byte to a serial port(uart1).
*Argument(s):	tx_byte 	The character to output.
*Return(s):		none
*Caller(s):		Application
*Note(s):		none
**************************************************************************
*/
void BSP_Ser_1_IRDA_WrByte(CPU_INT08U tx_byte)
{
	BSP_OS_SemWait(&BSP_Ser_1_IRDA_RxLock, 0);
	
	
	
	BSP_Ser_1_IRDA_WrByteUnlocked(tx_byte);

	BSP_OS_SemPost(&BSP_Ser_1_IRDA_RxLock);

}
	
	
/*
**************************************************************************
*					BSP_Ser_1_IRDA_WrStr()
*Description:	Transmits a string.
*Argument(s):	p_str	Pointer to the string that will be transmitted.
*Caller(s):		Application.
*Return(s):		none.
*Note(s):		none.
*************************************************************************
*/
void BSP_Ser_1_IRDA_WrStr( CPU_CHAR	*p_str)
{
	CPU_BOOLEAN err;
	err = BSP_OS_SemWait(&BSP_Ser_1_IRDA_TxLock, 0);
	
	if (err != DEF_OK) {
		return;
	}
	
	tx_bufSize = 0;
	tx_bufCounter = 0;
	while((*p_str) != (CPU_CHAR) 0) {
		tx_buf[tx_bufSize] = p_str[tx_bufSize];
		tx_bufSize++;
	}
	USART_ITConfig(USART1, USART_IT_TXE, ENABLE);
	
}


/*
**************************************************************************
*					BSP_Ser_1_IRDA_WrStrLen()
*Description:	Transmits a string.
*Argument(s):	p_str	Pointer to the string that will be transmitted.
*				num 	amount that is be transmitted string.
*Caller(s):		Application.
*Return(s):		none.
*Note(s):		none.
*************************************************************************
*/
void BSP_Ser_1_IRDA_WrStrLen( CPU_INT08U	*p_str, CPU_INT08U num)
{
	CPU_BOOLEAN err;
	CPU_INT08U i;
	err = BSP_OS_SemWait(&BSP_Ser_1_IRDA_TxLock, 0);
	
	if (err != DEF_OK) {
		return;
	}
	
	for (i = 0; i < num; i++) {
		BSP_Ser_1_IRDA_WrByteUnlocked(*p_str++);
	}
	
	BSP_OS_SemPost(&BSP_Ser_1_IRDA_TxLock);
}


void BSP_Ser_1_IDRA_RdSetCallback(void (*rxCallback)(CPU_INT08U data))
{
	BSP_Ser_1_IRDA_RdCallback = rxCallback;

}


CPU_INT08U BSP_Ser_1_IRDA_RdString(CPU_INT08U *string, CPU_INT08U length)
{

	CPU_INT08U err;
	CPU_INT08U i;
	BSP_OS_SemWait(&BSP_Ser_1_IRDA_RxLock, 0);
	USART_ITConfig(UART4, USART_IT_RXNE, ENABLE);
	
	i = 0;
	while (1) {
		BSP_OS_SemWait(&BSP_Ser_1_IRDA_RxWait, 0);
		string[i++] = BSP_Ser_1_IRDA_RxData;
		while(1) {
			err = BSP_OS_SemWait(&BSP_Ser_1_IRDA_RxWait, 0);
			string[i++] = BSP_Ser_1_IRDA_RxData;
			
			if (err == DEF_OK) {
				if (string[i-2] == '\r' && string[i-1] == '\n') {
					string[i] = '\0';
					
					USART_ITConfig(UART4, USART_IT_RXNE, DISABLE);
					BSP_OS_SemPost(&BSP_Ser_1_IRDA_RxLock);
					return i;
				}else if (i == length - 1) {
					i = 0;
					break;
				}
			}else {
				i = 0;
				break;
			}
		}
	}
	
}




void Time_Init(void) 
{
	GPIO_InitTypeDef GPIO_InitStructure;
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	TIM_ICInitTypeDef  TIM_ICInitStructure;
	TIM_OCInitTypeDef  TIM_OCInitStructure;
	

	
	/* TIM1 clock enable */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3 ,ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
	
	
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;											// TIM3_CH1			Input
  GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource6, GPIO_AF_TIM3);
	
	
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;											// TIM3_CH2				Output
  GPIO_Init(GPIOA, &GPIO_InitStructure);
  GPIO_PinAFConfig(GPIOA, GPIO_PinSource7, GPIO_AF_TIM3);  

	TimerPeriod = (SystemCoreClock / 38000 / 2 ) - 1;//17570
  /* Compute CCR1 value to generate a duty cycle at 50% for channel 1 and 1N */
  Channel1Pulse = (uint16_t) (((uint32_t) 2 * (TimerPeriod - 2)) / 3);

	 /* Time base configuration */
  TIM_TimeBaseStructure.TIM_Period = TimerPeriod;
  TIM_TimeBaseStructure.TIM_Prescaler = 0; //4
  TIM_TimeBaseStructure.TIM_ClockDivision = 0;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;

  TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);
	
	
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM2;
  TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
//  TIM_OCInitStructure.TIM_OutputNState = TIM_OutputNState_Enable;
  TIM_OCInitStructure.TIM_Pulse = Channel1Pulse;
  TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
  TIM_OCInitStructure.TIM_OCNPolarity = TIM_OCNPolarity_Low;//High;
  TIM_OCInitStructure.TIM_OCIdleState = TIM_OCIdleState_Set;//Set;
  TIM_OCInitStructure.TIM_OCNIdleState = TIM_OCIdleState_Reset;
  
  TIM_OC2Init(TIM3, &TIM_OCInitStructure);				
	
	
	  /* TIM1 Input Capture Configuration */
  TIM_ICInitStructure.TIM_Channel = TIM_Channel_1;
  TIM_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Falling;
  TIM_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI;
  TIM_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;
  TIM_ICInitStructure.TIM_ICFilter = 1;

  TIM_ICInit(TIM3, &TIM_ICInitStructure);
	
	/* TIM1 Input trigger configuration: External Trigger connected to TI2 */
	TIM_SelectInputTrigger(TIM3, TIM_TS_TI1FP1);
  TIM_SelectSlaveMode(TIM3, TIM_SlaveMode_Gated);
	
	  /* TIM1 Main Output Enable */
  TIM_CtrlPWMOutputs(TIM3, ENABLE);
	
	TIM_Cmd(TIM3, ENABLE);
	
}

void IR_LED_on(void){
	GPIO_InitTypeDef GPIO_InitStructure;
	IR_LED_status = IR_LED;
	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;											// TIM3_CH2				Output
  GPIO_Init(GPIOA, &GPIO_InitStructure);
	GPIO_SetBits(GPIOA, GPIO_Pin_7);
}

void IR_LED_off(void) {
	GPIO_InitTypeDef GPIO_InitStructure;
	
	GPIO_ResetBits(GPIOA, GPIO_Pin_7);
	
	
	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;											// TIM3_CH2				Output
  GPIO_Init(GPIOA, &GPIO_InitStructure);
  GPIO_PinAFConfig(GPIOA, GPIO_PinSource7, GPIO_AF_TIM3);  
	
	IR_LED_status = IR_DATA_MODULE;
	
}



/*
*********************************************************************************************************
*
*                                        BOARD SUPPORT PACKAGE
*                                            Can communication
*
*                                     ST Microelectronics STM32
*                                              on the
*
*                                           8 Channel LED Analyzer V1.2
*
* Filename      : bsp_ioe.c
* Version       : V1.00
* Programmer(s) : SL
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                             INCLUDE FILES
*********************************************************************************************************
*/
#include "bsp.h"
#include "bsp_can.h"
#include "readSensor.h"



/*
*********************************************************************************************************
*                                            LOCAL DEFINES
*********************************************************************************************************
*/
#define USE_CAN1
#define CANx                       CAN1
#define CAN_CLK                    (RCC_APB1Periph_CAN1 | RCC_APB1Periph_CAN2)//RCC_APB1Periph_CAN2	
#define CAN_RX_PIN                 GPIO_Pin_11
#define CAN_TX_PIN                 GPIO_Pin_12
#define CAN_GPIO_PORT              GPIOA
#define CAN_GPIO_CLK               RCC_AHB1Periph_GPIOA
#define CAN_AF_PORT                GPIO_AF_CAN1
#define CAN_RX_SOURCE              GPIO_PinSource11
#define CAN_TX_SOURCE              GPIO_PinSource12     


#define BSP_CAN_PRINTF_STR_BUF_SIZE	80
/*
*********************************************************************************************************
*                                           LOCAL CONSTANTS
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                          LOCAL DATA TYPES
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                            LOCAL TABLES
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                       LOCAL GLOBAL VARIABLES
*********************************************************************************************************
*/

CAN_InitTypeDef        CAN_InitStructure;
CAN_FilterInitTypeDef  CAN_FilterInitStructure;
CanTxMsg TxMessage;
CanRxMsg RxMessage;

static CAN_EventHandle_TypeDef *CAN_Event_HandlePointer;

//static  BSP_OS_SEM   BSP_CanTxWait;
static 	BSP_OS_SEM	 BSP_CanTxLock;

//static  BSP_OS_SEM   BSP_CanRxWait;
//static  BSP_OS_SEM   BSP_CanRxLock;

//static  CPU_INT08U   BSP_CanRxData;

CPU_CHAR CAN_tx_buffer[BSP_CAN_PRINTF_STR_BUF_SIZE];
CPU_CHAR *CAN_tx_bufferP = CAN_tx_buffer;



/*
*********************************************************************************************************
*                                      LOCAL FUNCTION PROTOTYPES
*********************************************************************************************************
*/
void BSP_Can_RX_ISR_Handler(void);
void BSP_Can_TX_ISR_Handler(void);
/*
*********************************************************************************************************
*                                     LOCAL CONFIGURATION ERRORS
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*********************************************************************************************************
**                                         GLOBAL FUNCTIONS
*********************************************************************************************************
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                          BSP_Can_Init()
*
* Description : Initialize a Can port for communication.
*
* Argument(s) : baud_rate           The desire CAN baud rate.
*
* Return(s)   : none.
*
* Caller(s)   : Application
*
* Note(s)     : none.
*********************************************************************************************************
*/
void BSP_Can_Init(void)
{
  GPIO_InitTypeDef  GPIO_InitStructure;
  
	//BSP_CanTxWait = OSSemCreate(0);
	BSP_CanTxLock = OSSemCreate(1);
	
//	BSP_CanRxWait = OSSemCreate(0);
//	BSP_CanRxLock = OSSemCreate(1);
	
  /* CAN GPIOs configuration **************************************************/

  /* Enable GPIO clock */
  RCC_AHB1PeriphClockCmd(CAN_GPIO_CLK, ENABLE);

  /* Connect CAN pins to AF9 */
  GPIO_PinAFConfig(CAN_GPIO_PORT, CAN_RX_SOURCE, CAN_AF_PORT);
  GPIO_PinAFConfig(CAN_GPIO_PORT, CAN_TX_SOURCE, CAN_AF_PORT); 
  
  /* Configure CAN RX and TX pins */
  GPIO_InitStructure.GPIO_Pin = CAN_RX_PIN | CAN_TX_PIN;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;
  GPIO_Init(CAN_GPIO_PORT, &GPIO_InitStructure);

  /* CAN configuration ********************************************************/  
  /* Enable CAN clock */
  RCC_APB1PeriphClockCmd(CAN_CLK, ENABLE);
  
  /* CAN register init */
  CAN_DeInit(CANx);

  /* CAN cell init */
  CAN_InitStructure.CAN_TTCM = DISABLE;
  CAN_InitStructure.CAN_ABOM = DISABLE;
  CAN_InitStructure.CAN_AWUM = DISABLE;
  CAN_InitStructure.CAN_NART = DISABLE;
  CAN_InitStructure.CAN_RFLM = DISABLE;
  CAN_InitStructure.CAN_TXFP = DISABLE;
  CAN_InitStructure.CAN_Mode = CAN_Mode_Normal;//CAN_Mode_LoopBack;
  CAN_InitStructure.CAN_SJW = CAN_SJW_1tq;
    
  /* CAN Baudrate = 1MBps (CAN clocked at 30 MHz) */
  CAN_InitStructure.CAN_BS1 = CAN_BS1_6tq;
  CAN_InitStructure.CAN_BS2 = CAN_BS2_8tq;
  CAN_InitStructure.CAN_Prescaler = 2;
  CAN_Init(CANx, &CAN_InitStructure);

  /* CAN filter init */
#ifdef  USE_CAN1
  CAN_FilterInitStructure.CAN_FilterNumber = 0;
#else /* USE_CAN2 */
  CAN_FilterInitStructure.CAN_FilterNumber = 14;
#endif  /* USE_CAN1 */
  CAN_FilterInitStructure.CAN_FilterMode = CAN_FilterMode_IdMask;
  CAN_FilterInitStructure.CAN_FilterScale = CAN_FilterScale_32bit;
  CAN_FilterInitStructure.CAN_FilterIdHigh = 0x0000;
  CAN_FilterInitStructure.CAN_FilterIdLow = 0x0000;
  CAN_FilterInitStructure.CAN_FilterMaskIdHigh = 0x0000;
  CAN_FilterInitStructure.CAN_FilterMaskIdLow = 0x0000;
  CAN_FilterInitStructure.CAN_FilterFIFOAssignment = 0;
  CAN_FilterInitStructure.CAN_FilterActivation = ENABLE;
  CAN_FilterInit(&CAN_FilterInitStructure);
  
  /* Transmit Structure preparation */
  TxMessage.StdId = 0x321;
  TxMessage.ExtId = 0x01;
  TxMessage.RTR = CAN_RTR_DATA;
  TxMessage.IDE = CAN_ID_STD;
  TxMessage.DLC = 1;
  
  /* Enable FIFO 0 message pending Interrupt */
  
	
	BSP_IntVectSet(BSP_INT_ID_CAN1_RX0, BSP_Can_RX_ISR_Handler);
  BSP_IntEn(BSP_INT_ID_CAN1_RX0);
	CAN_ITConfig(CANx, CAN_IT_FMP0, ENABLE);
	
	
	BSP_IntVectSet(BSP_INT_ID_CAN1_TX, BSP_Can_TX_ISR_Handler);
  BSP_IntEn(BSP_INT_ID_CAN1_TX);
	
	
}

void BSP_Can_RX_ISR_Handler(void)
{
  CAN_EventHandle_TypeDef *temp_CAN_Event_HandlePointer;
  temp_CAN_Event_HandlePointer = CAN_Event_HandlePointer;
  CAN_Receive(CANx, CAN_FIFO0, &RxMessage);
//	CAN_Receive(CAN2, CAN_FIFO1, &RxMessage);
	while( temp_CAN_Event_HandlePointer != NULL)
  {
    if( (temp_CAN_Event_HandlePointer->StdId == RxMessage.StdId) && 
        (temp_CAN_Event_HandlePointer->RTR == RxMessage.RTR) && 
        (temp_CAN_Event_HandlePointer->IDE == RxMessage.IDE)) 
    {
//            INT8U i;
//            for ( i = 0; i < RxMessage.DLC; i++)
//            {
//                    temp_CAN_Event_HandlePointer->data[i] = RxMessage.Data[i];
//            }
//            temp_CAN_Event_HandlePointer->DLC = RxMessage.DLC;
            temp_CAN_Event_HandlePointer->Can_Callback(RxMessage.Data, RxMessage.DLC);
            break;
    }
    temp_CAN_Event_HandlePointer = (temp_CAN_Event_HandlePointer->next);
  }
	
}


/*
*********************************************************************************************************
*                                          BSP_Can_EventRegister()
*
* Description : This function hang a function into can used process received data.
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : APP.
*
* Note(s)     : none.
*********************************************************************************************************
*/
INT8U BSP_Can_EventRegister(CAN_EventHandle_TypeDef * newHandle)
{
   CPU_SR   cpu_sr;
   CPU_CRITICAL_ENTER();
	CAN_EventHandle_TypeDef *temp_CAN_Event_HandlePointer;
	if ( newHandle != NULL)
	{
		temp_CAN_Event_HandlePointer = CAN_Event_HandlePointer;
		if (temp_CAN_Event_HandlePointer == NULL) 
		{
			CAN_Event_HandlePointer = newHandle;
		}else {
			while (1)
			{
				if (temp_CAN_Event_HandlePointer->next == NULL){
					temp_CAN_Event_HandlePointer->next = newHandle;
					newHandle->prev = temp_CAN_Event_HandlePointer;
                CPU_CRITICAL_EXIT();
					return 0;
				}else {
					temp_CAN_Event_HandlePointer = temp_CAN_Event_HandlePointer->next;
				}
			}
		}
      CPU_CRITICAL_EXIT();
		return 0;
	}
   CPU_CRITICAL_EXIT();
	return !0;
}
/*
*********************************************************************************************************
*                                          BSP_Can_EventUnregister()
*
* Description : This function remove a specified Can event from can chain table.
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : APP.
*
* Note(s)     : none.
*********************************************************************************************************
*/
INT8U BSP_Can_EventUnregister(CAN_EventHandle_TypeDef DeletedHandle)
{
	CAN_EventHandle_TypeDef *temp_CAN_Event_HandlePointer;
   CPU_SR   cpu_sr;
   CPU_CRITICAL_ENTER();
	if (CAN_Event_HandlePointer->StdId == DeletedHandle.StdId)
	{
		CAN_Event_HandlePointer = CAN_Event_HandlePointer->next;
		CAN_Event_HandlePointer->prev = NULL;
      
      CPU_CRITICAL_EXIT();
		return 0;
	}else {
		temp_CAN_Event_HandlePointer = CAN_Event_HandlePointer->next;
		while (1)
		{
			if (temp_CAN_Event_HandlePointer->StdId == DeletedHandle.StdId) {
				temp_CAN_Event_HandlePointer->prev->next = temp_CAN_Event_HandlePointer->next;
				if (temp_CAN_Event_HandlePointer->next != NULL)
					temp_CAN_Event_HandlePointer->next->prev = temp_CAN_Event_HandlePointer->prev;
            CPU_CRITICAL_EXIT();
				return 0;
			}else {
				temp_CAN_Event_HandlePointer = temp_CAN_Event_HandlePointer->next;
				if (temp_CAN_Event_HandlePointer == NULL) {
               CPU_CRITICAL_EXIT();
					return !0;
            }
			}
		}
	}
}



/*********************************************************************************************
*                       BSP_Can_TX_ISR_Handler()
*Description    : CAN Interrupt Service Routine.
*Argument(9)    : none.
*Return(s)      : none.
*Caller(s)      : This is an Interrupt Service Rountine
*Note(s)        : none.
*********************************************************************************************
*/

void BSP_Can_TX_ISR_Handler(void)
{
   if ( *CAN_tx_bufferP != '\0') {
      int i;
      for(i = 0; i < 8 && *CAN_tx_bufferP != '\0'; i++) {
         TxMessage.Data[i] = *CAN_tx_bufferP++;
      }
      TxMessage.DLC = i;
      CAN_Transmit(CANx, &TxMessage);
   }else {
      CAN_ITConfig(CANx, CAN_IT_TME, DISABLE);
      //CAN_ClearITPendingBit()
      OSSemPost(BSP_CanTxLock);
   }
   
}




/*
*******************************************************************************************
*						BSP_CAN_Printf()
*
*Description	: Formatted output to the CAN port.
*				  This function writes a string to a CAN port.
*				  This call blocks until a character appers at the port and the last character
*				  is a Carriage Return(0x0D).
*Argument(s)	: Format string following the C format Convertion.
*Caller(s)		: Application.
*Note(s)		: none.
*******************************************************************************************
*/
void  BSP_CAN_Printf(CPU_INT16U ID, CPU_CHAR * format, ...)
{
  CPU_INT08U err;
  va_list v_args;
  OSSemPend(BSP_CanTxLock, 0, &err);
  va_start(v_args, format);
  (void)vsnprintf((char 		*) &CAN_tx_buffer[0],
                                  (CPU_SIZE_T  ) sizeof(CAN_tx_buffer),
                                  (char const *) format,
                                                  v_args);
   va_end(v_args);
   
   CAN_tx_bufferP = CAN_tx_buffer;
   TxMessage.StdId = ID;
   TxMessage.ExtId = 0x01;
   TxMessage.RTR = CAN_RTR_DATA;
   TxMessage.IDE = CAN_ID_STD;
    if ( *CAN_tx_bufferP != '\0') {
      int i;
      for(i = 0; i < 8 && *CAN_tx_bufferP != '\0'; i++) {
         TxMessage.Data[i] = *CAN_tx_bufferP++;
      }
      TxMessage.DLC = i;
      CAN_Transmit(CANx, &TxMessage);
			CAN_ITConfig(CANx, CAN_IT_TME, ENABLE);
   }else {
      CAN_ITConfig(CANx, CAN_IT_TME, DISABLE);
      //CAN_ClearITPendingBit()
      OSSemPost(BSP_CanTxLock);
   }
	 
   
}



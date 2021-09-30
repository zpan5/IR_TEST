/*
*********************************************************************************************************
*                                              EXAMPLE CODE
*
*                             (c) Copyright 2013; Micrium, Inc.; Weston, FL
*
*                   All rights reserved.  Protected by international copyright laws.
*                   Knowledge of the source code may not be used to write a similar
*                   product.  This file may only be used in accordance with a license
*                   and should not be redistributed in any way.
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*
*                                            EXAMPLE CODE
*
*                                     ST Microelectronics STM32
*                                              on the
*
*                                           STM3220G-EVAL
*                                         Evaluation Board
*
* Filename      : app.c
* Version       : V1.00
* Programmer(s) : DC
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                             INCLUDE FILES
*********************************************************************************************************
*/

#include  <app_cfg.h>
#include  <includes.h>

/*
*********************************************************************************************************
*                                            LOCAL DEFINES
*********************************************************************************************************
*/
//typedef struct ;

/*
*********************************************************************************************************
*                                       LOCAL GLOBAL VARIABLES
*********************************************************************************************************
*/

/* ----------------- APPLICATION GLOBALS ------------------ */
static  OS_STK          AppTaskStartStk[APP_TASK_START_STK_SIZE];
OS_EVENT *print_data_UART3;
OS_EVENT *AckMbox;
OS_EVENT *TxMbox;


/*
*********************************************************************************************************
*                                         FUNCTION PROTOTYPES
*********************************************************************************************************
*/

static  void	  	AppEventCreate			(void); 
static  void		AppTaskStart            (void       *p_arg);
static  void 		AppTaskCreate           (void);

CPU_INT16U CRC_16(CPU_INT08U *Array, CPU_INT08U Len);
void XOR(CPU_CHAR *string);
/*
*********************************************************************************************************
*                                                main()
*
* Description : This is the standard entry point for C code.  It is assumed that your code will call
*               main() once you have performed all necessary initialization.
*
* Arguments   : none
*
* Returns     : none
*********************************************************************************************************
*/

int main(void)
{
#if (OS_TASK_NAME_EN > 0)
    CPU_INT08U  err;
#endif   

    BSP_IntDisAll();                                            /* Disable all interrupts.                              */
    
    CPU_Init();                                                 /* Initialize uC/CPU services.                          */
    
    OSInit();                                                   /* Initialize "uC/OS-II, The Real-Time Kernel"          */

    OSTaskCreateExt((void (*)(void *)) AppTaskStart,            /* Create the start task                                */
                    (void           *) 0,
                    (OS_STK         *)&AppTaskStartStk[APP_TASK_START_STK_SIZE - 1],
                    (INT8U           ) APP_TASK_START_PRIO,
                    (INT16U          ) APP_TASK_START_PRIO,
                    (OS_STK         *)&AppTaskStartStk[0],
                    (INT32U          ) APP_TASK_START_STK_SIZE,
                    (void           *) 0,
                    (INT16U          )(OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR));

#if (OS_TASK_NAME_EN > 0)
    OSTaskNameSet(APP_TASK_START_PRIO, "Start", &err);
#endif

    OSStart();                                                  /* Start multitasking (i.e. give control to uC/OS-II)   */
    return (1);
}

/*
*********************************************************************************************************
*                                          STARTUP TASK
*
* Description : This is an example of a startup task.  As mentioned in the book's text, you MUST
*               initialize the ticker only once multitasking has started.
*
* Arguments   : p_arg   is the argument passed to 'AppTaskStart()' by 'OSTaskCreate()'.
*
* Returns     : none
*
* Notes       : 1) The first line of code is used to prevent a compiler warning because 'p_arg' is not
*                  used.  The compiler should not generate any code for this statement.
*********************************************************************************************************
*/

static  void  AppTaskStart (void *p_arg)
{

	static CPU_INT08U counter_0 = 0, watchdog_state = 0;
	
	(void)p_arg;
	
    BSP_Init();                                                 /* Init BSP fncts.                                          */
		
    CPU_Init();                                                 /* Init CPU name & int. dis. time measuring fncts.          */

    Mem_Init();                                                 /* Initialize Memory managment                              */

    BSP_CPU_TickInit();                                         /* Start Tick Initialization                                */


#if (OS_TASK_STAT_EN > 0)
    OSStatInit();                                               /* Determine CPU capacity                                   */
#endif

#if (APP_CFG_SERIAL_EN == DEF_ENABLED)
#endif
    AppEventCreate();                                           /* Create Application Events                                */

	  AppTaskCreate();                                           /* Create Application Tasks                                 */


#if (WATCHDOG == 1)
	/* Check if the system has resumed from WWDG reset */
	if (RCC_GetFlagStatus(RCC_FLAG_WWDGRST) != RESET) {
		/* Clear reset flags */
		RCC_ClearFlag();
      /* WWDGRST flag set */
		/* Turn on LED1 */
    //  while(1){
         //BSP_LED_On(1);
    //     BSP_LED_Toggle(1);
     //    OSTimeDlyHMSM(0,0,5,0);
     // }
      watchdog_state = 1;
	}else{
		/* WWDGRST flag is not set */
		/* Turn off LED1 */
      watchdog_state = 0;
	//	BSP_LED_Off(5); 
	}

//	/* Setup SysTick Timer for 1 msec interrupts  */
//	if (SysTick_Config(SystemCoreClock / 1000)){ 
//		/* Capture error */ 
//		while (1);
//	}
	/* WWDG configuration */
	/* Enable WWDG clock */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_WWDG, ENABLE);

	/* WWDG clock counter = (PCLK1 (30MHz)/4096)/8 = 915 Hz (~1092 us)  */
	WWDG_SetPrescaler(WWDG_Prescaler_8);

	/* Set Window value to 80; WWDG counter should be refreshed only when the counter
	is below 80 (and greater than 64) otherwise a reset will be generated */
	WWDG_SetWindowValue(127);

	/* Enable WWDG and set counter value to 127, WWDG timeout = ~1092 us * 64 = 69.9 ms 
	In this case the refresh window is: ~1092 * (127-80) = 51.3 ms < refresh window < ~1092 * 64 = 69.9ms
	*/
	WWDG_Enable(127);
#endif
   				
//    TxMessage.StdId   = 0x001;
//		TxMessage.ExtId   = 0x01;
//		TxMessage.RTR     = CAN_RTR_DATA;
//    TxMessage.IDE     = CAN_ID_STD;
//		TxMessage.DLC     = 8;
//		TxMessage.Data[0] = '1';
//		TxMessage.Data[1] = '2';
//		TxMessage.Data[2] = '3';
//		TxMessage.Data[3] = '4';
//		TxMessage.Data[4] = '5';
//		TxMessage.Data[5] = '6';
//		TxMessage.Data[6] = '7';
//		TxMessage.Data[7] = '8';
//		CAN_Transmit(CAN2, &TxMessage);
//		BSP_SPI_W25Q64_ReadID();
		
		BSP_LED_Off(1);
		BSP_LED_On(2);

    while (DEF_TRUE) {                                          /* Task body, always written as an infinite loop.           */

       if(watchdog_state == 0) {
          if (counter_0++ > 3) {
            counter_0 = 0;
            BSP_LED_Toggle(1);
						BSP_LED_Toggle(2);
         }
      }else {
         if (counter_0++ > 10) {
            counter_0 = 0;
            BSP_LED_Toggle(1);
					  BSP_LED_Toggle(2);
         }
      }

		OSTimeDly(50);
#if (WATCHDOG == 1)
		WWDG_SetCounter(0x72);
#endif
		
    }
}

/*
*********************************************************************************************************
*                                      AppTaskEventCreate()
*
* Description : Create the application Events
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : App_TasStart()
*
* Note(s)     : none.
*********************************************************************************************************
*/

static  void  AppEventCreate (void)
{
		
}


/*
*********************************************************************************************************
*                                      AppTaskCreate()
*
* Description : Create the application tasks.
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : App_TasStart()
*
* Note(s)     : none.
*********************************************************************************************************
*/

static  void  AppTaskCreate (void)
{
	//ReadSensorTaskCreate();
	//I2C1TestTaskCreate();
	IR_UserInterfaceInitialization();
	IRColorTest_Init();
	IR_ReceiveInitialization();
}


/**********************************************************************************************
*							CRC16()
*Description	: Using the Modbus crc protocol calculate the CRC. 
*Argument(s)	: p_buff Pointer is point the data that is be calculate.
*				  len 	 The data amount is calculated.
*Return(s)		: CRC
*Caller(s)		: VerifyRxData()
*Note			: none.
***********************************************************************************************
*/

CPU_INT16U CRC_16(CPU_INT08U *Array, CPU_INT08U Len)
{

	CPU_INT32U  IX,IY,crc;

	crc=0xFFFF; //set all 1

	if (Len <= 0) crc = 0;
	else {
		Len--;
		for (IX=0;IX<=Len;IX++)	{

			crc=crc^(unsigned int)(Array[IX]);

			for(IY=0;IY<=7;IY++) {

				if ((crc&1)!=0 )
					crc=(crc>>1)^0xA001;
				else
					crc=crc>>1;   
			}
		}
	}
	return crc;
}


/*
*********************************************************************************************************
*                                              EXAMPLE CODE
*
*                          (c) Copyright 2003-2012; Micrium, Inc.; Weston, FL
*
*               All rights reserved.  Protected by international copyright laws.
*               Knowledge of the source code may NOT be used to develop a similar product.
*               Please help us continue to provide the Embedded community with the finest
*               software available.  Your honesty is greatly appreciated.
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*
*                                      APPLICATION CONFIGURATION
*
*                                     ST Microelectronics STM32
*                                              on the
*
*                                           STM3220G-EVAL
*                                         Evaluation Board
*
* Filename      : app_cfg.h
* Version       : V1.00
* Programmer(s) : DC
*********************************************************************************************************
*/

#ifndef  __APP_CFG_H__
#define  __APP_CFG_H__

#include  <stdio.h>

#define DEVICE_CAN_ID 4

/*
*********************************************************************************************************
*                                       MODULE ENABLE / DISABLE
*********************************************************************************************************
*/

#define  BSP_CFG_LED_SPI2_EN                    DEF_ENABLED       /* Enable/disable LEDs on SPI port.                     */
#define  BSP_CFG_LED_PIOC_EN                    DEF_ENABLED       /* Enable/disable PIOC LEDs.                            */

#define  APP_CFG_SERIAL_EN                      DEF_ENABLED

#define APP_DEBUG_EN                            DEF_ENABLED
/*
*********************************************************************************************************
*                                            TASK PRIORITIES
*********************************************************************************************************
*/

#define  APP_TASK_START_PRIO                             			2u

#define  OS_TASK_TMR_PRIO                       							(OS_LOWEST_PRIO - 2)

#define APP_TASK_KEY_TEST_PRIO																3u
#define APP_TASK_COMMUNICATION_VCP_PRIO												4u	
#define APP_TASK_READ_SENSOR_PRIO															5u

#define APP_TASK_I2C1Test_PRIO																6u

#define IR_UserInterface_CAN_Task_PRIO												7u
#define IR_UserInterface_UART_Task_PRIO												8u
#define IR_ColorTest_Task_PRIO																9u
#define IR_Receive_Task_PRIO																	10u
 




/*
*********************************************************************************************************
*                                            TASK STACK SIZES
*                             Size of the task stacks (# of OS_STK entries)
*********************************************************************************************************
*/

#define  APP_TASK_START_STK_SIZE                        				128u
#define	 APP_TASK_KEY_TEST_STK_SIZE															256u
#define  APP_TASK_STK_SIZE												      				256u

#define  APP_TASK_READ_SENSOR_STK_SIZE													1024u
#define  I2C1Test_STK_SIZE																			256u
#define  IR_UserInterface_CAN_Task_STK_SIZE											512u
#define  IR_UserInterface_UART_Task_STK_SIZE										512u

#define  IR_ColorTest_Task_STK_SIZE															512u
#define  IR_Receive_Task_STK_SIZE																512


/*
*********************************************************************************************************
*                                            BSP CFG
*********************************************************************************************************
*/

#define  BSP_CFG_SER_COMM_SEL                   BSP_SER_COMM_UART_01




/*
*********************************************************************************************************
*                                     TRACE / DEBUG CONFIGURATION
*********************************************************************************************************
*/
#if 0
#define  TRACE_LEVEL_OFF                                0
#define  TRACE_LEVEL_INFO                               1
#define  TRACE_LEVEL_DBG                                2
#endif

#define  APP_TRACE_LEVEL                                TRACE_LEVEL_INFO

#if (APP_CFG_SERIAL_EN == DEF_ENABLED)
#define  APP_TRACE                                      BSP_Ser_Printf
#define  APP_TRACE_1																		BSP_Ser_1_Printf
#else
#define  APP_TRACE                                      printf
#endif

#define  APP_TRACE_INFO_1(x)			      (void)(APP_TRACE_1 x)
#define  APP_TRACE_INFO(x)               ((APP_TRACE_LEVEL >= TRACE_LEVEL_INFO)  ? (void)(APP_TRACE x) : (void)0)
#define  APP_TRACE_DBG(x)                ((APP_TRACE_LEVEL >= TRACE_LEVEL_DBG)   ? (void)(APP_TRACE x) : (void)0)

#endif


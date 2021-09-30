#ifndef BSP_SER_1_IRDA_H
#define BSP_SER_1_IRDA_H
#include <bsp.h>
#include  <app_cfg.h>

// data type define
typedef enum IR_LED_STATUS{
	IR_DATA_MODULE,IR_LED
}IR_LED_STATUS;

// function prototype define 
void BSP_Ser_1_IRDA_Init(CPU_INT32U baud_rate);

IR_LED_STATUS BSP_Ser_1_IRDA_Printf(CPU_CHAR *format, ...);
void BSP_Ser_1_IRDA_WrByte(CPU_INT08U tx_byte);
void BSP_Ser_1_IRDA_WrStr(CPU_CHAR	*p_str);
void BSP_Ser_1_IRDA_WrStrLen( CPU_INT08U	*p_str, CPU_INT08U num);

CPU_INT08U BSP_Ser_1_IRDA_RdByte(void);
CPU_INT08U BSP_Ser_1_IRDA_RxStrTimeout(CPU_INT08U *p_str, CPU_INT08U *err);
void BSP_Ser_1_IDRA_RdSetCallback(void (*rxCallback)(CPU_INT08U data));

void IR_LED_on(void);
void IR_LED_off(void);

#endif




#ifndef IR_MODULE_H
#define IR_MODULE_H
#include "includes.h"

void IR_Initialization(void);
void IR_SendingData(CPU_INT08U *pData);
void IR_SetTxStrength(CPU_INT16U strength);

void IR_ReceiveDataCallback(CPU_INT08U data);

void IR_ReceiveStrengthCallback(uint16_t strength);

void IR_ReceiveInitialization(void);
#endif


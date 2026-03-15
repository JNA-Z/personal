#ifndef __WT901C_H__
#define __WT901C_H__

#include <stdint.h>

extern uint8_t GyrRxHeadBuffer[10];
extern uint8_t GyrRxDataBuffer[50];

void Gyroscope_Init(void);
void Gyroscope_Start(void);


#endif


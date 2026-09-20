#ifndef _UART4AI_H_
#define _UART4AI_H_

#include "_510PF.h"
#include "UI.h"
#include "KNL.h"
#include <string.h>
#include "UI_BUCCU[WSVGA].h"
#include "SF_API.h"
#include "EN_API.h"
#include "FWU_API.h"
#include "TIMER.h"
#include "VDO.h"
#include "Buzzer.h"
#include "PLY_API.h"
#include "REC_API.h"


void UART4AI_Init(void);
void UART4AI_Recv(char ch);
void ResendI_handle(char CAM_CH);
extern void UI_SendBSDRangeTo1126(UI_ParkinglinePoint_t tParkinglinePoint,uint8_t chn);
extern uint8_t Drawing_BoxFlag[4];
//extern uint8_t ubPwrUartFlag; //频繁上断电延时开启串�?extern uint32_t ubPwrUartCount; //


#endif /* _UART4AI_H_ */


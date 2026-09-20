#ifndef __TRIGGRT_H
#define __TRIGGRT_H

#include "TIMER.h"
#include "stdio.h"
#include "SADC.h"
#include "_510PF.h"
#include "APP_CFG.h"
#include "UI_BUCCU[WSVGA].h"

/*****************************************AD1*******************************************************/
//642  396
#define TRIGGER_S ((uwSADC_GetReport(SADC_CH1) > 592 && uwSADC_GetReport(SADC_CH1) < 692)||(uwSADC_GetReport(SADC_CH1) > 346 && uwSADC_GetReport(SADC_CH1) < 446))
					  

//515  396
#define TRIGGER_B ((uwSADC_GetReport(SADC_CH1) > 465 && uwSADC_GetReport(SADC_CH1) < 565)||(uwSADC_GetReport(SADC_CH1) > 346 && uwSADC_GetReport(SADC_CH1) < 446))
					
/*****************************************AD2*******************************************************/

//575	435	 480  378		 	
 #define TRIGGER_R ((uwSADC_GetReport(SADC_CH2) > 550 && uwSADC_GetReport(SADC_CH2) < 600)||(uwSADC_GetReport(SADC_CH2) > 410 && uwSADC_GetReport(SADC_CH2) < 450)||(uwSADC_GetReport(SADC_CH2) > 455 && uwSADC_GetReport(SADC_CH2) < 505)||(uwSADC_GetReport(SADC_CH2) > 353 && uwSADC_GetReport(SADC_CH2) < 403))  

//649  435  530  378
#define TRIGGER_L ((uwSADC_GetReport(SADC_CH2) > 624 && uwSADC_GetReport(SADC_CH2) < 674)||(uwSADC_GetReport(SADC_CH2) > 410 && uwSADC_GetReport(SADC_CH2) < 450)||(uwSADC_GetReport(SADC_CH2) > 505 && uwSADC_GetReport(SADC_CH2) < 550)||(uwSADC_GetReport(SADC_CH2) > 353 && uwSADC_GetReport(SADC_CH2) < 403))  
//758  530	480  378
#define TRIGGER_F ((uwSADC_GetReport(SADC_CH2) > 733 && uwSADC_GetReport(SADC_CH2) < 783)||(uwSADC_GetReport(SADC_CH2) > 505 && uwSADC_GetReport(SADC_CH2) < 550)||(uwSADC_GetReport(SADC_CH2) > 455 && uwSADC_GetReport(SADC_CH2) < 505)||(uwSADC_GetReport(SADC_CH2) > 353 && uwSADC_GetReport(SADC_CH2) < 403)) 


typedef struct TRIGGER_INF_S
{
	UI_CamNum_t tUI_ViewSel;
	
}TRIGGER_EVENT_t;

extern TRIGGER_EVENT_t Trigger_View;

void Trigger_init(void);

#endif

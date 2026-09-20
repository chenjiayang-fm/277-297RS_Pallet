#ifndef _UI_BUCCU_SUBSUBMENU_H_
#define _UI_BUCCU_SUBSUBMENU_H_

#include "UI_BUCCU[WSVGA].h"
#include "UI_BUCCU[WSVGA]_SUBMENU.h"

#define DATE_TIME_YEAR_DIST_X 130
#define DATE_TIME_YEAR_DIST_Y 118


void UI_StartPlayRecordFile(UI_RecPlayDispType_t tPlayDispType);
void UI_ShowUTCVal(void);

void UI_SystemDrawDateTimeYearButton(uint8_t DateTimeYearButtonHLIndex);


#endif


/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		cmd_rtc676x.c
	\brief		RTC676x command line
	\author		
	\version	0.1
	\date		2019/07/10
	\copyright	
*/
//------------------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cmd.h"
#include "CLI.h"
#include "RTC_API.h"
#include "FWU_API.h"
#include "APP_CFG.h"
#include "KNL.h"
#include "WDT.h"
#include "APP_HS.h"

//------------------------------------------------------------------------------
#ifdef CONFIG_CLI_CMD_RTC676x

void cmd_rtc676x_enter_RFMP_test_mode(void)
{
	APP_SetRfTestMode(APP_RF_TEST_ON);
	printf("===================================\n");
	printf("Set RFMP test mode on\n");
	printf("Reboot ...\n");
	printf("===================================\n");
	WDT_Disable(WDT_RST);
	WDT_RST_Enable(WDT_CLK_EXTCLK, 1);
	while(1);
}

void cmd_rtc676x_exit_RFMP_test_mode(void)
{
	APP_SetRfTestMode(APP_RF_TEST_OFF);
	printf("===================================\n");
	printf("Set RFMP test mode off\n");
	printf("Reboot ...\n");
	printf("===================================\n");
	WDT_Disable(WDT_RST);
	WDT_RST_Enable(WDT_CLK_EXTCLK, 1);
	while(1);
}

static void rtc676x_mp_test_usage() {
	printf(" pls check uasge!\n");
	printf("###################################\n");
	printf(" Usage : rfmp <OPT>\n");
	printf(" OPT:\n");
	printf("	1: enter RF mp test mode\n");
	printf("###################################\n");
}

int32_t cmd_rtc676x_mp_test(int argc, char* argv[])
{
	uint8_t OPT;

	if (argc < 2) {
		rtc676x_mp_test_usage();
		return cliFAIL;
	}

	OPT = strtoul(argv[1], NULL, 0);

	switch(OPT)
	{
		case 1:
			cmd_rtc676x_enter_RFMP_test_mode();
			break;
		default :
			rtc676x_mp_test_usage();
			return cliFAIL;
	}
		
	return cliPASS;
}

#endif

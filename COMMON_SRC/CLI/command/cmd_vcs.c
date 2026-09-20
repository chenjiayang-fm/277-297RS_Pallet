/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		cmd_vcs.c
	\brief		Version control command line
	\author		Hanyi Chiu
	\version	0.2
	\date		2020/05/04
	\copyright	Copyright(C) 2017 SONiX Technology Co., Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------
#include <stdio.h>
#include "cmd.h"
#include "CLI.h"

#ifdef CONFIG_CLI_CMD_VCS

#include <string.h>
#include "PROFILE_API.h"
#include "DDR_API.h"
#include "MMU_API.h"
#include "DMAC_API.h"
#include "INTC.h"
#include "UART.h"
#include "TIMER.h"
#include "WDT.h"
#include "RTC_API.h"
#include "I2C.h"
#include "CQ_API.h"
#include "CQI2C.h"
#include "SADC.h"
#include "BB_API.h"
#include "RF_API.h"
#include "TWC_API.h"
#include "PAIR.h"
#include "SF_API.h"
#include "SDIO_API.h"
#include "CIPHER_API.h"
#ifdef OP_AP
#include "LCD.h"
#include "OSD.h"
#include "LCD_HDMI_IT66121_API.h"
#endif
#include "H264_API.h"
#include "IMG_API.h"
#include "JPEG_API.h"
#include "ADO_API.h"
#ifdef OP_STA
#include "AE_API.h"
#include "AWB_API.h"
#include "IQ_API.h"
#include "ISP_API.h"
#endif
#include "FWU_API.h"
#ifdef OP_STA
#include "USBD_API.h"
#endif
#ifdef WIFI_TYPE_SDIO
#include "WiFi_API.h"
#endif
#include "MC_API.h"
#include "VCS_API.h"

typedef struct
{
	char *pLibName;
	uint16_t (*pvLibraryVcFunc)(void);	
}CmdVer_GetVersionFuncPtr_t;

//------------------------------------------------------------------------------
static void vcs_usage() {   
	printf(" pls check uasge!\n");
	printf("###################################\n");
	printf(" Usage : VCS <ID>\n");
	printf(" Example: show all library version\n");
	printf("	VCS all\n");
	printf("###################################\n");
}
//------------------------------------------------------------------------------
const CmdVer_GetVersionFuncPtr_t tCmdVer_GetVerFunc[] = 
{
	{"OS",			uwSYS_GetOsVersion},
	{"PROFILE",		uwPROFILE_GetVersion},
	{"DDR",			uwDDR_GetVersion},
	{"MMU",			uwMMU_GetVersion},
	{"DMAC",		uwDMAC_GetVersion},
//	{"UART",		uwUART_GetVersion},
//	{"TIMER",		uwTIMER_GetVersion},
//	{"WDT",			uwWDT_GetVersion},
	{"RTC",			uwRTC_GetVersion},
//	{"I2C",			uwI2C_GetVersion},
	{"CQ",			uwCQ_GetVersion},
//	{"CQI2C",		uwCQI2C_GetVersion},
//	{"SADC",		uwSADC_GetVersion},
#ifdef A7130
	{"BB",			uwBB_GetVersion},
	{"RFA7130",		uwRFA7130_GetVersion},
#endif
	{"TWC",			uwTWC_GetVersion},
//	{"PAIR",		uwPAIR_GetVersion},
	{"SF",			uwSF_GetVersion},
	{"SDIO",		uwSDIO_GetVersion},
	{"CIPHER",		uwCIPHER_GetVersion},
#ifdef OP_AP
//	{"LCD",			uwLCD_GetVersion},
//	{"OSD",			uwOSD_GetVersion},
#if (LCD_HDMI_OUT == LCD_HDMI_IT66121)
	{"HDMI",		uwHDMI_IT66121_GetVersion},
#endif
#endif
	{"H264",		uwH264_GetVersion},
	{"IMG",			uwIMG_GetVersion},
	{"JPEG",		uwJPEG_GetVersion},
	{"AUDIO",		uwADO_GetVersion},
#ifdef OP_STA
	{"AE",			uwAE_GetVersion},
	{"AWB",			uwAWB_GetVersion},
	{"IQ",			uwIQ_GetVersion},
	{"ISP",			uwISP_GetVersion},
#endif
	{"FWU",			uwFWU_GetVersion},
#ifdef OP_STA
	{"USBD",		uwUSBD_GetVersion},
#endif
#ifdef WIFI_TYPE_SDIO
	{"WIFI",		uwWiFi_GetVersion},
#endif	
	{"MC",          uwMC_GetVersion},
	{"VCS",         uwVCS_GetVersion},
};
//------------------------------------------------------------------------------
uint16_t uwCmdVer_GetLibraryVersion(char *LibraryName)
{
	uint16_t uwVCS_Cnt = 0, uwIdx;

	uwVCS_Cnt = sizeof tCmdVer_GetVerFunc / sizeof(CmdVer_GetVersionFuncPtr_t);
	for(uwIdx = 0; uwIdx < uwVCS_Cnt; uwIdx++)
	{
		if(!strcmp(LibraryName, tCmdVer_GetVerFunc[uwIdx].pLibName))
			return tCmdVer_GetVerFunc[uwIdx].pvLibraryVcFunc();
	}
	return 0xFFFF;
}
//------------------------------------------------------------------------------
void CmdVer_LibraryVersionList(char *LibraryName)
{
	uint16_t uwVCS_Cnt = 0, uwIdx;
	uint16_t uwVCS_Ver = 0;

	uwVCS_Cnt = sizeof tCmdVer_GetVerFunc / sizeof(CmdVer_GetVersionFuncPtr_t);
	printf("===================================\n");
	printf("        Library List  		 \n");
	if(!strcmp(LibraryName, "all") || !strcmp(LibraryName, "ALL"))
	{
		for(uwIdx = 0; uwIdx < uwVCS_Cnt; uwIdx++)
		{
			uwVCS_Ver = tCmdVer_GetVerFunc[uwIdx].pvLibraryVcFunc();
			printf("   [%s] 		  : V%d.%d\n", tCmdVer_GetVerFunc[uwIdx].pLibName, (uwVCS_Ver >> 8), (uwVCS_Ver & 0xFF));
		}
	}
	else
	{
		uwVCS_Ver = uwCmdVer_GetLibraryVersion(LibraryName);
		printf("   [%s] 		  : V%d.%d\n", LibraryName, (uwVCS_Ver == 0xFFFF)?0xFF:(uwVCS_Ver >> 8), (uwVCS_Ver == 0xFFFF)?0xFF:(uwVCS_Ver & 0xFF));
	}
	printf("===================================\n");
}
//------------------------------------------------------------------------------
int32_t cmd_vcs_getVer(int argc, char* argv[])
{
	if (argc != 2) {
		vcs_usage();
		return cliFAIL;	
	}

	CmdVer_LibraryVersionList(argv[1]);

	return cliPASS;	
}

#endif		//! End of #ifdef CONFIG_CLI_CMD_VCS

/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		cmd_i2c.c
	\brief		I2C control command line
	\author		Ocean
	\version	0.1
	\date		2017/10/12
	\copyright	Copyright(C) 2017 SONiX Technology Co., Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h> 
#include <string.h>
#include "cmd.h"
#include "CLI.h"
#include "FS_API.h"

//------------------------------------------------------------------------------
#ifdef CONFIG_CLI_CMD_SD

static void sd_speed_ctrl_usage() {   
	printf(" pls check uasge!\n");
	printf("###################################\n");
	printf(" Usage: sd_ctrl <Mode>\n");
	printf(" Mode:\n");
	printf("	0: Sd card speed test\n");
	printf("###################################\n");
}

int32_t cmd_sd_ctrl(int argc, char* argv[])
{
	uint32_t ulMode;
	FS_MEDIA_SPD_RPT SdSpdTestStatus;
	FS_RW_TEST_INTO_t info[6];
	uint32_t i;
	FS_MEDIA_SEL MediaSel = FS_MEDIA_0;
	
	if (argc < 2) {
		sd_speed_ctrl_usage();
		return cliFAIL;
	}
	
	ulMode = strtoul(argv[1], NULL, 0);
	
	switch(ulMode)
	{
		case 0:
			// HDx1	
			i = 0;		
			info[i].ulTargetDataRate  = 768;
			info[i].Vdo.ubNum         = 1;
			info[i].Vdo.Res[0]        = FS_RES_HD;
			info[i].Vdo.ulSize[0]     = 0x280000;
			info[i].Vdo.ulDataRate[0] = 768;
			// HDx2
			i = 1;
			info[i].ulTargetDataRate  = 768;
			info[i].Vdo.ubNum         = 2;
			info[i].Vdo.Res[0]        = FS_RES_HD;
			info[i].Vdo.ulSize[0]     = 0x180000;
			info[i].Vdo.ulDataRate[0] = 384;
			info[i].Vdo.Res[1]        = FS_RES_HD;
			info[i].Vdo.ulSize[1]     = 0x180000;
			info[i].Vdo.ulDataRate[1] = 384;
			// HDx4
			i = 2;
			info[i].ulTargetDataRate  = 768;
			info[i].Vdo.ubNum         = 4;
			info[i].Vdo.Res[0]        = FS_RES_HD;
			info[i].Vdo.ulSize[0]     = 0x100000;
			info[i].Vdo.ulDataRate[0] = 192;
			info[i].Vdo.Res[1]        = FS_RES_HD;
			info[i].Vdo.ulSize[1]     = 0x100000;
			info[i].Vdo.ulDataRate[1] = 192;
			info[i].Vdo.Res[2]        = FS_RES_HD;
			info[i].Vdo.ulSize[2]     = 0x100000;
			info[i].Vdo.ulDataRate[2] = 192;
			info[i].Vdo.Res[3]        = FS_RES_HD;
			info[i].Vdo.ulSize[3]     = 0x100000;
			info[i].Vdo.ulDataRate[3] = 192;
			// FHDx1+HDx1
			i = 3;
			info[i].ulTargetDataRate  = 2560;
			info[i].Vdo.ubNum         = 2;
			info[i].Vdo.Res[0]        = FS_RES_FHD;
			info[i].Vdo.ulSize[0]     = 0x500000;
			info[i].Vdo.ulDataRate[0] = 2048;
			info[i].Vdo.Res[1]        = FS_RES_HD;
			info[i].Vdo.ulSize[1]     = 0x200000;
			info[i].Vdo.ulDataRate[1] = 512;
			// FHDx1+HDx2
			i = 4;
			info[i].ulTargetDataRate  = 2560;
			info[i].Vdo.ubNum         = 3;
			info[i].Vdo.Res[0]        = FS_RES_FHD;
			info[i].Vdo.ulSize[0]     = 0x500000;
			info[i].Vdo.ulDataRate[0] = 2048;
			info[i].Vdo.Res[1]        = FS_RES_HD;
			info[i].Vdo.ulSize[1]     = 0x180000;
			info[i].Vdo.ulDataRate[1] = 256;
			info[i].Vdo.Res[2]        = FS_RES_HD;
			info[i].Vdo.ulSize[2]     = 0x180000;
			info[i].Vdo.ulDataRate[2] = 256;
			// FHDx1+VGAx3
			i = 5;
			info[i].ulTargetDataRate  = 2560;
			info[i].Vdo.ubNum         = 3;
			info[i].Vdo.Res[0]        = FS_RES_FHD;
			info[i].Vdo.ulSize[0]     = 0x500000;
			info[i].Vdo.ulDataRate[0] = 2048;
			info[i].Vdo.Res[1]        = FS_RES_VGA;
			info[i].Vdo.ulSize[1]     = 0x100000;
			info[i].Vdo.ulDataRate[1] = 170;
			info[i].Vdo.Res[2]        = FS_RES_VGA;
			info[i].Vdo.ulSize[2]     = 0x100000;
			info[i].Vdo.ulDataRate[2] = 170;
			info[i].Vdo.Res[3]        = FS_RES_VGA;
			info[i].Vdo.ulSize[3]     = 0x100000;
			info[i].Vdo.ulDataRate[3] = 170;

			SdSpdTestStatus = FS_MediaRWTest(MediaSel, info, 6);
			if( SdSpdTestStatus==FS_MEDIA_FAIL )
			{
				printf("CLI -> sd speed test result: sd card fail!!\n");
			}
			else if( SdSpdTestStatus==FS_MEDIA_PASS )
			{
				printf("CLI -> sd speed test result: sd card pass!!\n");
			}
			break;
	}
	
	return cliPASS;	
}

#endif

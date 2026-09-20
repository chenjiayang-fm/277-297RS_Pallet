/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		cmd_i2c.h
	\brief		I2C control command line header file
	\author		Ocean
	\version	0.1
	\date		2017/10/12
	\copyright	Copyright(C) 2017 SONiX Technology Co., Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------
#ifndef _CMD_SD_H
#define _CMD_SD_H

#include "_510PF.h"

int32_t cmd_sd_ctrl(int argc, char* argv[]);

#define CMD_TBL_SD		CMD_TBL_ENTRY(		\
	"sd",		2,      NULL,			\
	"sd		- Printf Command Table",	CFG_DEFAULT_CMD_LEVEL,\
	cmd_sd_tbl,		cmd_main_tbl			\
),

#define CMD_TBL_SD_CTRL		CMD_TBL_ENTRY(		\
	"sd_ctrl",		7,      cmd_sd_ctrl,	\
	"sd_ctrl         - Sd control",	CFG_DEFAULT_CMD_LEVEL,\
	NULL,		NULL			\
),

#endif


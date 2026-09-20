/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		cmd_rtc676x.h
	\brief		RTC676x command line header file
	\author		
	\version	0.22
	\date		2019/07/15
	\copyright	
*/
//------------------------------------------------------------------------------
#ifndef _CMD_RTC676x_H_
#define _CMD_RTC676x_H_

#include <stdint.h>

void cmd_rtc676x_exit_RFMP_test_mode(void);

int32_t cmd_rtc676x_mp_test(int argc, char* argv[]);

#define CMD_TBL_RTC676x    CMD_TBL_ENTRY(          \
	"rtc676x",		7,	NULL,       \
	"rtc676x		- Enter RTC676x",	CFG_DEFAULT_CMD_LEVEL,\
	cmd_rtc676x_tbl,		cmd_main_tbl			\
),

#define CMD_TBL_RTC676x_MP_TEST    CMD_TBL_ENTRY(          \
	"mptest",		6,	cmd_rtc676x_mp_test,       \
	"mptest		- RF MP test",	CFG_DEFAULT_CMD_LEVEL,\
	NULL,		NULL			\
),

#endif


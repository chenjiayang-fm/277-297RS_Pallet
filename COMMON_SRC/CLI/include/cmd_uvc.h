#ifndef S2019A

#ifndef _CMD_UVC_H_
#define _CMD_UVC_H_

#include <stdio.h>
#include <string.h>
#include "_510PF.h"

int32_t cmd_uvc_init(int argc, char* argv[]);
int32_t cmd_uvc_get_info(int argc, char* argv[]);		
int32_t cmd_uvc_start(int argc, char* argv[]);
int32_t cmd_uvc_stop(int argc, char* argv[]);

#define CMD_TBL_UVC		CMD_TBL_ENTRY(		\
	"+uvc",		4,      cmd_uvc_init,			\
	"+uvc		- UVC Command Table",	CFG_DEFAULT_CMD_LEVEL,\
	cmd_uvc_tbl,		cmd_main_tbl			\
),

#define CMD_TBL_UVC_GET_INFO	CMD_TBL_ENTRY(		\
	"uvc_get_info",	12,      cmd_uvc_get_info,	\
	"uvc_get_info	- UVC GET Stream Infomation",	CFG_DEFAULT_CMD_LEVEL,\
	NULL,		NULL			\
),

#define CMD_TBL_UVC_START	CMD_TBL_ENTRY(		\
	"uvc_start",	9,      cmd_uvc_start,	\
	"uvc_start	- UVC START Stream",	CFG_DEFAULT_CMD_LEVEL,\
	NULL,		NULL			\
),

#define CMD_TBL_UVC_STOP	CMD_TBL_ENTRY(		\
	"uvc_stop",	8,      	cmd_uvc_stop,	\
	"uvc_stop	- UVC STOP Stream",	CFG_DEFAULT_CMD_LEVEL,\
	NULL,		NULL			\
),

#endif

#endif //! End of #ifndef S2019A

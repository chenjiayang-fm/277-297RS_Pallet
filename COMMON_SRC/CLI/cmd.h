#ifndef _CMD_H_
#define _CMD_H_
#include "bsp_select.h"

//#define CONFIG_CLI_CMD_PRINTF 
#define CONFIG_CLI_CMD_H264
#define CONFIG_CLI_CMD_VCS
//#if (HOSTX2_EN)
//#define CONFIG_CLI_CMD_UVC
//#endif
#define CONFIG_CLI_CMD_SYSTEM
#define CONFIG_CLI_CMD_I2C
#if (defined(OP_STA) || defined(BSP_DVR_SDK))
#define CONFIG_CLI_CMD_ISP
#endif
#define CONFIG_CLI_CMD_SF
#define CONFIG_CLI_CMD_BB
//#ifdef OP_AP
//#define CONFIG_CLI_CMD_LCD
//#endif
//#define CONFIG_CLI_CMD_MC
#define CONFIG_CLI_CMD_ADO
#define CONFIG_CLI_CMD_DDR
#define CONFIG_CLI_CMD_PAIR
#define CONFIG_CLI_CMD_SD


#ifdef RTC676x
#define CONFIG_CLI_CMD_RTC676x
#endif

#ifdef CONFIG_CLI_CMD_PRINTF
#include "cmd_printf.h"
#endif

#ifdef CONFIG_CLI_CMD_H264
#include "cmd_h264.h"
#endif

#ifdef CONFIG_CLI_CMD_VCS
#include "cmd_vcs.h"
#endif

//#ifdef CONFIG_CLI_CMD_UVC
#include "cmd_uvc.h"
//#endif

#ifdef CONFIG_CLI_CMD_SYSTEM
#include "cmd_system.h"
#endif

#ifdef CONFIG_CLI_CMD_I2C
#include "cmd_i2c.h"
#endif

#ifdef CONFIG_CLI_CMD_ISP
#include <cmd_isp.h>
#endif

#ifdef CONFIG_CLI_CMD_SF
#include <cmd_sf.h>
#endif

#ifdef CONFIG_CLI_CMD_BB
#include <cmd_bb.h>
#endif

#ifdef CONFIG_CLI_CMD_LCD
#include "cmd_lcd.h"
#endif

#ifdef CONFIG_CLI_CMD_MC
#include "cmd_mc.h"
#endif

#ifdef CONFIG_CLI_CMD_ADO
#include "cmd_ado.h"
#endif

#ifdef CONFIG_CLI_CMD_DDR
#include "cmd_ddr.h"
#endif

#ifdef CONFIG_CLI_CMD_PAIR
#include "cmd_pair.h"
#endif

#ifdef CONFIG_CLI_CMD_SD
#include "cmd_sd.h"
#endif


#ifdef CONFIG_CLI_CMD_RTC676x
#include "cmd_rtc676x.h"
#endif


#endif

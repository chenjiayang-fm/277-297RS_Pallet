#ifndef _RTOSCLI_H_
#define _RTOSCLI_H_

#include "_510PF.h"
#include "APP_CFG_SEL.h"

/** \defgroup COM_INTERFACE Commumication Interface Modules
 * \ingroup CLI_MODULE
 * 
 * @{
 */ 
 
#define CMD_TBL_ENTRY(name, len_name, cmd, usage, uid_lv, next_lv, prev_lv)  {name, len_name, cmd, usage, uid_lv, next_lv, prev_lv} //!< CLI Table Entry Macro
#define CMDBUF_SIZE	128					//!< CLI Buffer Size
#define PROMPT	"\nSONIX (%s)>:\n"		//!< CLI Root Prompt
#define LOGIN_PROMPT	"\nLogin :\n"	//!< CLI Login Prompt
#define PASSWD_PROMPT	"Password :\n"	//!< CLI Password Prompt

//#undef CFG_ENABLE_LOGIN
#if ( APP_CFG_ENABLE_LOGIN == 1 )
	#define CFG_ENABLE_LOGIN
#endif
#define CFG_DEFAULT_CMD_LEVEL		9999	//!< CLI Default Level Value

//#define DEBUG_LV Kernel
#define K_Print(fmt,arg...) printf(fmt,##arg);						//!< Kernel Level Print Function
#define E_Print(fmt,arg...) if(Error >= DEBUG) printf(fmt,##arg);	//!< Error Level Print Function
#define W_Print(fmt,arg...) if(Warning >= DEBUG) printf(fmt,##arg);	//!< Warning Level Print Function
#define I_Print(fmt,arg...) if(Info >= DEBUG) printf(fmt,##arg);	//!< Information Level Print Function

#define cliPASS  0	//!< CLI Pass Value
#define cliFAIL  1	//!< CLI Fail Value

struct cmd_table {
	char	*name;								//!< Command name
	int		len_name;							//!< Lengh of Command name
	int 	(*Func)(int argc, char* argv[]);	//!< Corresponded Function to the Command
	char 	*usage;								//!< Usage Message
	int		cmd_lv;								//!< Command Level
	struct	cmd_table 	*next_lv;				//!< Command table next level pointer
	struct	cmd_table 	*prev_lv;				//!< Command table next previous pointer
};
/*!
\brief CLI quit command
\return(no)
\par [Example]
*/
int32_t CLI_cmd_quit(void);
/*!
\brief CLI help command
\return(no)
\par [Example]
*/
int32_t CLI_cmd_help(int argc, char* argv[]);
/*!
\brief CLI back command
\return(no)
\par [Example]
*/
int32_t CLI_cmd_back(int argc, char* argv[]);
/*!
\brief CLI logout command
\return(no)
\par [Example]
*/
int32_t CLI_cmd_logout(int argc, char* argv[]);
/*!
\brief CLI Show Device command
\return(no)
\par [Example]
*/
int32_t CLI_cmd_showDevId(int argc, char* argv[]);
/*!
\brief CLI Set Log Level  command
\return(no)
\par [Example]
*/
int32_t CLI_cmd_setloglv(int argc, char* argv[]);
/*!
\brief CLI Set Sprintf Level  command
\return(no)
\par [Example]
*/
int32_t CLI_cmd_setsprflv(int argc, char* argv[]);
/* Common command */

/*!
\brief Logout Table Entry  
*/
#define CMD_TBL_LOGOUT    CMD_TBL_ENTRY(          \
	"logout",		6,	CLI_cmd_logout,       \
	"logout		- Logout the system",	CFG_DEFAULT_CMD_LEVEL,\
	NULL,		NULL			\
),
/*!
\brief Quit Table Entry  
*/
#define CMD_TBL_QUIT    CMD_TBL_ENTRY(          \
	"quit",		4,	CLI_cmd_quit,       \
	"quit		- Quit the program",	CFG_DEFAULT_CMD_LEVEL,\
	NULL,		NULL			\
),
/*!
\brief Help Table Entry  
*/
#define CMD_TBL_HELP	CMD_TBL_ENTRY(		\
	"help",		4,	CLI_cmd_help,				\
	"help		- Show usage message",	CFG_DEFAULT_CMD_LEVEL,\
	NULL,		NULL			\
),
/*!
\brief Back Table Entry  
*/
#define CMD_TBL_BACK    CMD_TBL_ENTRY(          \
	"back",		4,	CLI_cmd_back,       \
	"back            - Back to prev level",  CFG_DEFAULT_CMD_LEVEL,\
	NULL,		NULL                    \
),
/*!
\brief Main Table Entry  
*/
#define CMD_TBL_MAIN    CMD_TBL_ENTRY(          \
	"main",		4,	NULL,       \
	"main		- Main the program",	CFG_DEFAULT_CMD_LEVEL,\
	NULL,		NULL			\
),
/*!
\brief Show Device ID Table Entry  
*/
#define CMD_TBL_SHOW_DEV_ID    CMD_TBL_ENTRY(          \
	"showid",		6,	CLI_cmd_showDevId,       \
	"showid		- Show Device ID",	CFG_DEFAULT_CMD_LEVEL,\
	NULL,		NULL			\
),
/*!
\brief Set Debug Level Table Entry  
*/
#define CMD_TBL_SET_DEBUG_LV    CMD_TBL_ENTRY(          \
	"setlvl",		6,	CLI_cmd_setloglv,       \
	"setlvl		- Set System Debug Level(0:Off 1:Critical 2:Error 3:Info 4~6:Debug1-Debug3)",	CFG_DEFAULT_CMD_LEVEL,\
	NULL,		NULL			\
),
/*!
\brief Set Sprintf Level Table Entry  
*/
#define CMD_TBL_SET_SPRFDBG_LV    CMD_TBL_ENTRY(          \
	"sprfl",		5,	CLI_cmd_setsprflv,       \
	"sprfl	",	CFG_DEFAULT_CMD_LEVEL,\
	NULL,		NULL			\
),


typedef void (*pvCLIRedirectCbFunc)(char *);
void CLI_register_redirect_cb_func(pvCLIRedirectCbFunc pCLIRedirectCbFunc);


/*!
\brief CLI Initial Function
\return(no)
*/
void CLI_init_rtos_cli(void);
/*!
\brief CLI Parse Command Function
\param cmd		Input Command String
\param isTAB	Tab key
\return result
*/
int32_t CLI_parse_cmd(char *cmd, int isTAB);
/*!
\brief CLI Show Prompt
\return(no)
*/
void CLI_show_cli_prompt(void);
/*!
\brief CLI Show Login Prompt
\return(no)
*/
void CLI_show_login_prompt(void);
/*!
\brief CLI Receive Input String by UART
\return(no)
*/
void CLI_rtoscli_recv(char ch);
/*!
\brief CLI Check Password
\return result
*/
int32_t CLI_ispasswd(void);
/*!
\brief CLI Check Login 
\return result
*/
int32_t CLI_islogin(void);
/*!
\brief CLI Receive Thread Initial 
\return result
*/
void CLI_recvInit(void);
/*!
\brief CLI Main Thread Initial 
\return result
*/
void CLI_Init(void);

#endif
/** @} */

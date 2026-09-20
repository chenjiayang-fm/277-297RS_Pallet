/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		RTC676x_CTRL.h
	\brief		RTC676x Control Header File
	\author		Justin
	\version	0.24
	\date		2021/05/14
	\copyright	Copyright(C) 2021 SONiX Technology Co.,Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------

#ifndef _RTC676x_CTRL_H_
#define _RTC676x_CTRL_H_

#include "_510PF.h"

#define MAX_PWR_ATT			( 4UL )
#define MIN_PWR_ATT			( 0UL )
#define MAX_PWR_MGC			( 0UL )
#define MIN_PWR_MGC			( 7UL )

#define SET_PWR_ATT			( 4UL ) //! Change this value to modify RTC676x ATT(Attenuation)
#define SET_PWR_MGC			( 2UL ) //! Change this value to modify RTC676x MGC(Mixer Gain)

typedef enum
{
	//Fixed ATT @4
	RW_PWR_ATT_HIGH_ACK 		= (SET_PWR_ATT>MAX_PWR_ATT)?MAX_PWR_ATT:(SET_PWR_ATT<MIN_PWR_ATT)?MIN_PWR_ATT:SET_PWR_ATT,
	RW_PWR_ATT_HIGH_BEACON		= (SET_PWR_ATT>MAX_PWR_ATT)?MAX_PWR_ATT:(SET_PWR_ATT<MIN_PWR_ATT)?MIN_PWR_ATT:SET_PWR_ATT,
	RW_PWR_ATT_HIGH_TX			= (SET_PWR_ATT>MAX_PWR_ATT)?MAX_PWR_ATT:(SET_PWR_ATT<MIN_PWR_ATT)?MIN_PWR_ATT:SET_PWR_ATT,
	
	RW_PWR_ATT_MEDIUM_ACK 		= (SET_PWR_ATT>MAX_PWR_ATT)?MAX_PWR_ATT:(SET_PWR_ATT<MIN_PWR_ATT)?MIN_PWR_ATT:SET_PWR_ATT,
	RW_PWR_ATT_MEDIUM_BEACON	= (SET_PWR_ATT>MAX_PWR_ATT)?MAX_PWR_ATT:(SET_PWR_ATT<MIN_PWR_ATT)?MIN_PWR_ATT:SET_PWR_ATT,
	RW_PWR_ATT_MEDIUM_TX		= (SET_PWR_ATT>MAX_PWR_ATT)?MAX_PWR_ATT:(SET_PWR_ATT<MIN_PWR_ATT)?MIN_PWR_ATT:SET_PWR_ATT,
	
	RW_PWR_ATT_LOW_ACK 			= (SET_PWR_ATT>MAX_PWR_ATT)?MAX_PWR_ATT:(SET_PWR_ATT<MIN_PWR_ATT)?MIN_PWR_ATT:SET_PWR_ATT,
	RW_PWR_ATT_LOW_BEACON		= (SET_PWR_ATT>MAX_PWR_ATT)?MAX_PWR_ATT:(SET_PWR_ATT<MIN_PWR_ATT)?MIN_PWR_ATT:SET_PWR_ATT,
	RW_PWR_ATT_LOW_TX			= (SET_PWR_ATT>MAX_PWR_ATT)?MAX_PWR_ATT:(SET_PWR_ATT<MIN_PWR_ATT)?MIN_PWR_ATT:SET_PWR_ATT,
	
	//Dynamic MGC 
	RW_PWR_MGC_HIGH_ACK 		= (SET_PWR_MGC+0UL>MIN_PWR_MGC)?MIN_PWR_MGC:(SET_PWR_MGC+0UL<MAX_PWR_MGC)?MAX_PWR_MGC:SET_PWR_MGC+0UL,
	RW_PWR_MGC_HIGH_BEACON		= (SET_PWR_MGC+0UL>MIN_PWR_MGC)?MIN_PWR_MGC:(SET_PWR_MGC+0UL<MAX_PWR_MGC)?MAX_PWR_MGC:SET_PWR_MGC+0UL,
	RW_PWR_MGC_HIGH_TX			= (SET_PWR_MGC+0UL>MIN_PWR_MGC)?MIN_PWR_MGC:(SET_PWR_MGC+0UL<MAX_PWR_MGC)?MAX_PWR_MGC:SET_PWR_MGC+0UL,
	
	RW_PWR_MGC_MEDIUM_ACK 		= (SET_PWR_MGC+1UL>MIN_PWR_MGC)?MIN_PWR_MGC:(SET_PWR_MGC+1UL<MAX_PWR_MGC)?MAX_PWR_MGC:SET_PWR_MGC+1UL,
	RW_PWR_MGC_MEDIUM_BEACON	= (SET_PWR_MGC+1UL>MIN_PWR_MGC)?MIN_PWR_MGC:(SET_PWR_MGC+1UL<MAX_PWR_MGC)?MAX_PWR_MGC:SET_PWR_MGC+1UL,
	RW_PWR_MGC_MEDIUM_TX		= (SET_PWR_MGC+1UL>MIN_PWR_MGC)?MIN_PWR_MGC:(SET_PWR_MGC+1UL<MAX_PWR_MGC)?MAX_PWR_MGC:SET_PWR_MGC+1UL,
	
	RW_PWR_MGC_LOW_ACK 			= (SET_PWR_MGC+2UL>MIN_PWR_MGC)?MIN_PWR_MGC:(SET_PWR_MGC+2UL<MAX_PWR_MGC)?MAX_PWR_MGC:SET_PWR_MGC+2UL,
	RW_PWR_MGC_LOW_BEACON		= (SET_PWR_MGC+2UL>MIN_PWR_MGC)?MIN_PWR_MGC:(SET_PWR_MGC+2UL<MAX_PWR_MGC)?MAX_PWR_MGC:SET_PWR_MGC+2UL,
	RW_PWR_MGC_LOW_TX			= (SET_PWR_MGC+2UL>MIN_PWR_MGC)?MIN_PWR_MGC:(SET_PWR_MGC+2UL<MAX_PWR_MGC)?MAX_PWR_MGC:SET_PWR_MGC+2UL,	
}RW_PWR;

typedef enum
{
	RW_PWR_HIGH,
	RW_PWR_MEDIUM,
	RW_PWR_LOW,
}RW_PWR_SEL;

typedef enum
{
	RW_PWR_PKT_ACK = 0,
	RW_PWR_PKT_BEACON,
	RW_PWR_PKT_TX,
}RW_PWR_PKT;
typedef enum
{	
	//Slave to Master
	RW_S2M_SZ_BPSK_1T_1200	= (384-8),	//384
	RW_S2M_SZ_BPSK_NT_1200	= (384-8),	//384	
	RW_S2M_SZ_BPSK_1T_1667	= 308,
	RW_S2M_SZ_BPSK_NT_1667	= 308,
	
	RW_S2M_SZ_QPSK_1T_1200 	= (788-8),	//788
	RW_S2M_SZ_QPSK_NT_1200	= (816-8),	//816	
	RW_S2M_SZ_QPSK_1T_1667 	= 684,
	RW_S2M_SZ_QPSK_NT_1667	= 684,	
	
	RW_S2M_SZ_16QAM_1T_1200	= (880-8),	//880	
	RW_S2M_SZ_16QAM_NT_1200	= (1432-8),	//1432
	RW_S2M_SZ_16QAM_1T_1667	= 1312,	
	RW_S2M_SZ_16QAM_NT_1667	= 1384,	
	
	//Master to Slave
	RW_M2S_SZ_BPSK		= 144,		//Without provide by Richwave
	RW_M2S_SZ_QPSK		= 144,		//162
	RW_M2S_SZ_16QAM		= 144,		//Without provide by Richwave
}RW_MAX_PKT; 									

//#define MAX_VDO_PACKET_LEN 		760
//#define MAX_ADO_PACKET_LEN		144
#define MAX_VDO_PACKET_LEN 			RW_S2M_SZ_16QAM_NT_1667
#define MAX_ADO_PACKET_LEN			RW_M2S_SZ_16QAM

//#define MAX_CMD_PACKET_LEN	 		36	//Type(1)+OPC(1)+Data(32)+ChkSum(2)
#define MAX_CMD_PACKET_LEN	 		38	//Mode(1)+Rsv(1)+Type(1)+OPC(1)+Data(32)+ChkSum(2)

//RTC6763 Transmit/Receive Size
#define PKT_RECV_LEN	RW_S2M_SZ_16QAM_NT_1667

//=====================================
//#define PACKET_BUF_LEN  1024
#define PACKET_BUF_LEN  1536


#define BUF_SZ_MAX_VDO				  (256*1024)
#define BUF_SZ_MAX_ADO				  (64*1024)
#define BUF_SZ_PKT				 	  (1536)

void RTC676x_Init(void);
void RTC676x_VdoThread(void const *arg);
void RTC676x_AdoThread(void const *arg);
void RTC676x_CmdThread(void const *arg);

//void RTC676x_SetPower(uint8_t ubMode);
void RTC676x_SetPower(RW_PWR_PKT tPkt,RW_PWR_SEL tPwrSel);
void update_id(void);
static void load_id(void);
void RTC676x_DelId(uint8_t ubStaNum);

void RTC676x_TestPutUartString(char *cmd);
void RTC676x_TestInit(void);

uint8_t ubRTC676x_GetRunState(void);
void RTC676x_Stop(void);
void RTC676x_Start(void);

#endif

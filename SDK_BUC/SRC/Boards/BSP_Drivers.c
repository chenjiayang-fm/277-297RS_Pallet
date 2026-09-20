/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		bsp_drivers.c
	\brief		BUC CU/CAM Demo driver
	\author		Hanyi Chiu
	\version	0.14
	\date		2022/02/23
	\copyright	Copyright(C) 2022 SONiX Technology Co., Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------
#if defined BUC_CU || defined BUC_CAM

#include <stdio.h>
#include "BSP.h"
#include "bsp_config.h"
#include "UART.h"
#include "WDT.h"
#include "TIMER.h"
#include "CQ_API.h"
#include "RTC_API.h"
#include "CIPHER_API.h"
#include "DMAC_API.h"
#include "SD_API.h"
#include "APBC.h"
#include "APP_CFG.h"
#include "KNL.h"
#include "EFUSE_API.h"
#include "GKEY.h"
#include "CLI.h"
#ifdef CFG_UART1_ENABLE
#include "UI_UART1.h"
#endif
#include "UART4AI.h"
void RETARGET_Init (UART1_Type *ptUart);
#if (defined(BSP_SN93711_FHD_REC_RX_V4) || defined(BSP_D_SN93701_SSD2828_RX_V5) || defined(BSP_D_SN93701_TC358778_RX_V6))
void BSP_DriversInit(void)
{
	WDT_Disable(WDT_RST);
 	WDT_RST_Enable(WDT_CLK_EXTCLK, WDT_TIMEOUT_CNT);
	UART_Init(UART_2, UART_CLOCK_96M, BR_115200, UART_1STOPBIT, NULL);
	UART_RegistRecvCb(UART_2, CLI_rtoscli_recv);
	RETARGET_Init(UART2);
	TIMER_Init();
	CQ_Init();
    APBC_Init();

	//! LED
	GPIO->GPIO_OE2 	= 1;
	GPIO->GPIO_O2  	= 1;
	GPIO->GPIO_OE3 	= 1;
	GPIO->GPIO_O3 	= 0;
	GPIO->GPIO_OE13	= 1;
	GPIO->GPIO_OE0	= 1;

	//! BL Control
	PWM->PWM3_RATE  	= 127;
	PWM->PWM3_PERIOD 	= 0xC00;
	PWM->PWM3_HIGH_CNT 	= 0xA00;
	//! BL Enable
	GPIO->GPIO_OE11 = 1;
	GPIO->GPIO_O11  = 0;
	PWM->PWM_EN3    = 0;

	//! Speaker
	GPIO->GPIO_OE12 = 1;

	//! LCD Power	
	GPIO->GPIO_OE10 = 1;
	GPIO->GPIO_O10  = 0;
	//! LCD Reset
	PWM->PWM11_RATE  	= 127;
	PWM->PWM11_PERIOD 	= 1;
	PWM->PWM11_HIGH_CNT = 1;
	PWM->PWM_EN11    	= 0;

#ifdef S2019A
	//! RF POWER
	RTC_SetGPO_1(0, RTC_PullDownEnable);
#endif
#ifdef A7130
	//! RTC GPIO1
	RTC_SetGPO_1(1, RTC_PullDownDisable);
#endif

	printd(DBG_CriticalLvl, "SONiX SNCC71 High Speed Mode Start!\n");
}
#endif
//------------------------------------------------------------------------------
#ifdef BSP_D_SNCC71_GM8285C_RX_V2
void BSP_DriversInit(void)
{
	WDT_Disable(WDT_RST);
 	WDT_RST_Enable(WDT_CLK_EXTCLK, WDT_TIMEOUT_CNT);

	//UART_Init(UART_1, UART_CLOCK_96M, BR_115200, UART_1STOPBIT, NULL);
	UART_Init(UART_2, UART_CLOCK_96M, BR_115200, UART_1STOPBIT, NULL);
	UART_RegistRecvCb(UART_2, UART4AI_Recv);

	

	UART_Init(UART_1, UART_CLOCK_96M, BR_115200, UART_1STOPBIT, NULL);
	UART_RegistRecvCb(UART_1,  CLI_rtoscli_recv);

	RETARGET_Init(UART1);

	TIMER_Init();
	CQ_Init();
    APBC_Init();

	//! BL Control	  //20210308 PWM_BACKLIGHT
	PWM->PWM8_RATE		= 96;
	PWM->PWM8_PERIOD	= 100;
	PWM->PWM8_HIGH_CNT	= 0;//?¨¨2??a¡À31a
	PWM->PWM_EN8		= 1;
	
	//! LCD Power	//20210308	PNL_POWER_CTRL
	PWM->PWM0_RATE		= 127;
	PWM->PWM0_PERIOD	= 1;
	PWM->PWM0_HIGH_CNT	= 1;
	PWM->PWM_EN0		= 1;
	
	//! LCD Reset  //20210308  POWER_VGH_CTRL
	PWM->PWM6_RATE		= 127;
	PWM->PWM6_PERIOD	= 1;
	PWM->PWM6_HIGH_CNT	= 1;
	PWM->PWM_EN6		= 1;

	//! RF Power Reset	20201217 RF_POWER_CTRL
	PWM->PWM7_RATE		= 127;
	PWM->PWM7_PERIOD	= 1;
	PWM->PWM7_HIGH_CNT	= 1;
	PWM->PWM_EN7		= 1;

	//volume
	PWM->PWM9_RATE		= 96;
	PWM->PWM9_PERIOD	= 100;
	PWM->PWM9_HIGH_CNT	= 0;
	PWM->PWM_EN9		= 1;

	//¦Ì?¦Ì??¨¬2a PVCC_DOWN_DET ?????a¨º?¨¨?
	GPIO->GPIO_OE5 = 0;

	// buzzer 20230316
	PWM->PWM4_RATE	   = 96;
	PWM->PWM4_PERIOD   = 370; // 1MHz / 370 = 2.7KHz
	PWM->PWM4_HIGH_CNT = 185; // 50% duty cycle
	PWM->PWM_EN4	   = 0;

	//rf433  CM4301_RESET
	GPIO->GPIO_OE2 = 1;
	GPIO->GPIO_O2 = 1;

	POWER_LED_IO_ENABLE		= 1;
	POWER_LED_IO = 0;


	//JTAG
	GLB->JTAG_EN = 0;
	GPIO->GPIO_OE20 = 1;
	GPIO->GPIO_O20 = 1;

	GPIO->GPIO_OE19 = 0;
	GPIO->GPIO_O19 = 0;

	printd(DBG_CriticalLvl, "SONiX SNCC71 High Speed Mode Start!\n");
}
#endif
//------------------------------------------------------------------------------
#ifdef BSP_D_SNCC71_TP2915_AHD_RX_V3
void BSP_DriversInit(void)
{
	WDT_Disable(WDT_RST);
 	WDT_RST_Enable(WDT_CLK_EXTCLK, WDT_TIMEOUT_CNT);
	UART_Init(UART_2, UART_CLOCK_96M, BR_115200, UART_1STOPBIT, NULL);
	UART_RegistRecvCb(UART_2, CLI_rtoscli_recv);
	RETARGET_Init(UART2);
	TIMER_Init();
	CQ_Init();
    APBC_Init();

	//! LED
	POWER_LED_IO	  	= 1;
	POWER_LED_IO_ENABLE	= 1;
	CAM1_LED_IO(0);
	CAM1_LED_IO_ENABLE(1);
	CAM1_LED_IO(0);
	CAM1_LED_IO_ENABLE(1);
	CAM2_LED_IO(0);
	CAM2_LED_IO_ENABLE(1);
	CAM3_LED_IO(0);
	CAM3_LED_IO_ENABLE(1);
	CAM4_LED_IO(0);
	CAM4_LED_IO_ENABLE(1);
	GPIO->GPIO_OE13	= 1;
	GPIO->GPIO_OE0	= 1;

	//! BL Control
	//PWM->PWM3_RATE  	= 127;
	//PWM->PWM3_PERIOD 	= 0xC00;
	//PWM->PWM3_HIGH_CNT 	= 0xA00;
	//! BL Enable
	PWM->PWM5_RATE  	= 127;
	PWM->PWM5_PERIOD 	= 1;
	PWM->PWM5_HIGH_CNT 	= 1;
	//PWM->PWM_EN3    	= 0;
	PWM->PWM_EN5    	= 0;

	//! Speaker
	GPIO->GPIO_OE12 = 1;

	//! LCD Power
	PWM->PWM4_RATE 		= 127;
	PWM->PWM4_PERIOD 	= 1;
	PWM->PWM4_HIGH_CNT 	= 1;
	PWM->PWM_EN4 		= 1;
	//! LCD Reset
	GPIO->GPIO_OE3 		= 1;
	LCD_RESET_O(0);

#if (BSP_RTC_TIMER_SEL == RTC_TIMER_EXTERNAL)
	//! RTC PCF85063A trigger
	GPIO->GPIO_OE10 = 0;
#endif

	//! RF Power Reset
	GPIO->GPIO_OE11 = 1;
	GPIO->GPIO_O11  = 1;

#ifndef RTC676x
	//! RTC GPIO1
	RTC_SetGPO_1(1, RTC_PullDownDisable);
#endif

	//! Detect NTSC or PAL
	GLB->PADIO_PUN1 = 0xFEFFFFFF;
	GPIO->GPIO_PULL_HIGH10 = 1;
	GPIO->GPIO_PULL_EN10 = 1;
	GPIO->GPIO_OE10 = 0;
	GPIO->GPIO_O10 = 0;
	printd(DBG_CriticalLvl, "SONiX SNCC71 High Speed Mode Start!\n");
}
#endif
//------------------------------------------------------------------------------
#ifdef BSP_D_SNCC71_NVP6021_AHD_RX_V4
void BSP_DriversInit(void) {
	WDT_Disable(WDT_RST);
 	WDT_RST_Enable(WDT_CLK_EXTCLK, WDT_TIMEOUT_CNT);
	UART_Init(UART_2, UART_CLOCK_96M, BR_115200, UART_1STOPBIT, NULL);
	UART_RegistRecvCb(UART_2, CLI_rtoscli_recv);
	RETARGET_Init(UART2);
	TIMER_Init();
	CQ_Init();
    APBC_Init();
	
	//! LED
	GPIO->GPIO_O11 = 1; //! CAM_LED: Default On
	GPIO->GPIO_OE11 = 1;
	GPIO->GPIO_O2 = 1; //! PWR_LED: Default Off
	GPIO->GPIO_OE2 = 1;
	
	//! Pre-Converter Reset Pin
	PWM->PWM1_RATE  	= 127;
	PWM->PWM1_PERIOD 	= 1;
	PWM->PWM1_HIGH_CNT 	= 0; //! Default Low
	PWM->PWM_EN1    	= 1;
	
	//! NVP6021 Reset Pin
	GPIO->GPIO_O8 = 0; //! default low
	GPIO->GPIO_OE8 = 1;
	
	//! RF Power control
//	RTC_SetGPO_1(0, RTC_PullDownEnable); //! Set low default
	RTC_SetGPO_1(1, RTC_PullDownDisable); //! Set high default
	
	printd(DBG_CriticalLvl, "SONiX SNCC71 High Speed Mode Start!\n");
}
#endif //! End of #ifdef BSP_D_SNCC71_NVP6021_AHD_RX_V4
//------------------------------------------------------------------------------
#if (defined(BSP_SNCC70_AHD_TP9950_TX_V1) || defined(BSP_SNCC70_AHD_RN6752M_TX_V1))
void BSP_DriversInit(void)
{
	WDT_Disable(WDT_RST);
	WDT_RST_Enable(WDT_CLK_EXTCLK, WDT_TIMEOUT_CNT);
#ifdef CFG_UART1_ENABLE
	UART_Init(UART_1, UART_CLOCK_96M, BR_115200, UART_1STOPBIT, NULL);
	UART_RegistRecvCb(UART_1, UART1_rtoscli_recv);
#endif
	UART_Init(UART_2, UART_CLOCK_96M, BR_115200, UART_1STOPBIT, NULL);
	UART_RegistRecvCb(UART_2, CLI_rtoscli_recv);
	RETARGET_Init(UART2);
	TIMER_Init();
	CQ_Init();
	APBC_Init();

    //! Reset RN6752
    GPIO->GPIO_OE6  = 1;
    GPIO->GPIO_O6   = 0;
    
	//! LED
	GPIO->GPIO_OE1 	= 1;
	GPIO->GPIO_O1	= 1;
	GPIO->GPIO_OE2 	= 1;
	GPIO->GPIO_O2	= 0;
	
	//! GPIO16 PULL UP
	GLB->TRSTN_PDN  = 0;
	GPIO->GPIO_OE16	= 0;
	GPIO->GPIO_O16	= 0;
	
#ifdef A7130
	//! RTC GPIO1 -> RF POWER
	RTC_SetGPO_1(1, RTC_PullDownDisable);
#endif
#ifdef RTC676x
	//! Richwave RF PWR CTRL
	GPIO->GPIO_OE12	= 1;
	GPIO->GPIO_O12	= 1;
#endif
	printd(DBG_CriticalLvl, "SONiX SNCC70 High Speed Mode Start!\n");
}
#endif
//------------------------------------------------------------------------------
#if (defined(BSP_D_SNCC70_TX_V2) || defined(BSP_SN93710_FHD_REC_TX_V4))
void BSP_DriversInit(void)
{
	WDT_Disable(WDT_RST);
	WDT_RST_Enable(WDT_CLK_EXTCLK, WDT_TIMEOUT_CNT);
#ifdef CFG_UART1_ENABLE
	UART_Init(UART_1, UART_CLOCK_96M, BR_115200, UART_1STOPBIT, NULL);
	UART_RegistRecvCb(UART_1, UART1_rtoscli_recv);
#endif
	UART_Init(UART_2, UART_CLOCK_96M, BR_115200, UART_1STOPBIT, NULL);
	UART_RegistRecvCb(UART_2, CLI_rtoscli_recv);
	RETARGET_Init(UART2);
	TIMER_Init();
	CQ_Init();
	APBC_Init();

    //! Reset RN6752
    GPIO->GPIO_OE6  = 1;
    GPIO->GPIO_O6   = 0;
    
	//! LED
	GPIO->GPIO_OE1 	= 1;
	GPIO->GPIO_O1	= 1;
	GPIO->GPIO_OE2 	= 1;
	GPIO->GPIO_O2	= 0;
	GPIO->GPIO_OE3 	= 1;
	GPIO->GPIO_O3	= 0;

	//! Speaker
	GPIO->GPIO_OE4 	= 1;

#ifdef S2019A
	RTC_SetGPO_1(1, RTC_PullDownDisable);
	GPIO->GPIO_O12	= 0;
	GPIO->GPIO_OE12	= 1;
#endif
#ifdef A7130
	//! RTC GPIO1 -> RF POWER
	RTC_SetGPO_1(1, RTC_PullDownDisable);
#endif
#ifdef RTC676x
	//! Richwave RF PWR CTRL
	GPIO->GPIO_OE12	= 1;
	GPIO->GPIO_O12	= 1;
#endif
#if (APP_DOORPHONE_ENABLE==1)
    GPIO->GPIO_OE8	= 0;
    GPIO->GPIO_OE9  = 0;
    GPIO->GPIO_OE10	= 0;
    GKEY_SetDetPin(8,0);
    GKEY_SetDetPin(9,1);
    GKEY_SetDetPin(10,1);
#endif
	printd(DBG_CriticalLvl, "SONiX SNCC70 High Speed Mode Start!\n");
}
#endif
//------------------------------------------------------------------------------
#ifdef BSP_D_SN93714_TX_V1
void BSP_DriversInit(void)
{
	WDT_Disable(WDT_RST);
	WDT_RST_Enable(WDT_CLK_EXTCLK, WDT_TIMEOUT_CNT);
	UART_Init(UART_1, UART_CLOCK_96M, BR_115200, UART_1STOPBIT, NULL);
	UART_RegistRecvCb(UART_1, CLI_rtoscli_recv);
	RETARGET_Init(UART1);
	TIMER_Init();
	CQ_Init();
	APBC_Init();

#if !APP_SD_FUNC_ENABLE	
	//! LED
	SIGNAL_LED_IO(0);	//! LED B
	SIGNAL_LED_IO_ENABLE(1);
#endif	
	POWER_LED_IO 			= 1;	//! LED R
	POWER_LED_IO_ENABLE		= 1;
	PAIRING_LED_IO			= 0;	//! LED G
	PAIRING_LED_IO_ENABLE	= 1;

#if (defined(RTC676x) || defined(S2019A))
	//! Richwave RF PWR CTRL
	//! S2019 RF PWR CTRL
	PWM->PWM1_RATE  	= 127;
	PWM->PWM1_PERIOD 	= 1;
	PWM->PWM1_HIGH_CNT	= 1;
	PWM->PWM_EN1    	= 1;
#endif
	//! Speaker
	GPIO->GPIO_OE12 	= 1;
	printd(DBG_CriticalLvl, "SONiX SN93714 High Speed Mode Start!\n");
}
#endif
//------------------------------------------------------------------------------
#ifdef BSP_D_SN93716_TX_V1
void BSP_DriversInit(void)
{
	WDT_Disable(WDT_RST);
	WDT_RST_Enable(WDT_CLK_EXTCLK, WDT_TIMEOUT_CNT);
	UART_Init(UART_2, UART_CLOCK_96M, BR_115200, UART_1STOPBIT, NULL);
	UART_RegistRecvCb(UART_2, CLI_rtoscli_recv);
	RETARGET_Init(UART2);
	TIMER_Init();
	CQ_Init();
	APBC_Init();

	PWM->PWM1_RATE  		= 127;
	PWM->PWM1_PERIOD 		= 1;
	POWER_LED_IO 			= 1;	//! LED R
	POWER_LED_IO_ENABLE		= 1;
	PAIRING_LED_IO			= 0;	//! LED B
	PAIRING_LED_IO_ENABLE	= 1;

	//! RF 5V Power
	PWM->PWM11_RATE  	= 127;
	PWM->PWM11_PERIOD 	= 1;
	
#if (defined(RTC676x) || defined(S2019A))
	//! Richwave RF PWR CTRL
	//! S2019 RF PWR CTRL
	PWM->PWM11_HIGH_CNT = 1;
#else
	PWM->PWM11_HIGH_CNT = 0;
#endif
	PWM->PWM_EN11    	= 1;
	//! RF Reset
	PWM->PWM3_RATE  	= 127;
	PWM->PWM3_PERIOD 	= 1;
	
	//! Speaker
	GPIO->GPIO_OE6 			= 1;
	printd(DBG_CriticalLvl, "SONiX SN93716 High Speed Mode Start!\n");
}
#endif
//------------------------------------------------------------------------------
#if (defined(BSP_SNCC72_AHD_RN6752M_TX_V1)||defined(BSP_SNCC72_AHD_TP9950_TX_V2))
void BSP_DriversInit(void)
{
	WDT_Disable(WDT_RST);
	WDT_RST_Enable(WDT_CLK_EXTCLK, WDT_TIMEOUT_CNT);
#ifdef CFG_UART1_ENABLE
	UART_Init(UART_1, UART_CLOCK_96M, BR_115200, UART_1STOPBIT, NULL);
	UART_RegistRecvCb(UART_1, UART1_rtoscli_recv);
#endif
	UART_Init(UART_2, UART_CLOCK_96M, BR_115200, UART_1STOPBIT, NULL);
	UART_RegistRecvCb(UART_2, CLI_rtoscli_recv);
	RETARGET_Init(UART2);
	TIMER_Init();
	CQ_Init();
	APBC_Init();

    //! Reset RN6752
    GPIO->GPIO_OE1  = 1;
    GPIO->GPIO_O1   = 0;
	
	//! LED
	GPIO->GPIO_O20	= 1;
	GPIO->GPIO_OE20	= 1;
	
	//! GPIO16 PULL UP
	GLB->TRSTN_PDN  = 0;
	GPIO->GPIO_OE16	= 0;
	GPIO->GPIO_O16	= 0;
	
	//GPIO Key
	GLB->TDI_PDN = 0;
    GKEY_SetDetPin(19,1);
	GPIO->GPIO_OE19 = 0;	//Input Mode	
	
	//! RF PWR CTRL
	GPIO->GPIO_O18	= 1;
	GPIO->GPIO_OE18	= 1;
	
	printd(DBG_CriticalLvl, "SONiX SNCC70 High Speed Mode Start!\n");
}
#endif
//------------------------------------------------------------------------------
#ifdef BSP_D_SNCC72_TX_V1
void BSP_DriversInit(void)
{
	WDT_Disable(WDT_RST);
	WDT_RST_Enable(WDT_CLK_EXTCLK, WDT_TIMEOUT_CNT);
#ifdef CFG_UART1_ENABLE
	UART_Init(UART_1, UART_CLOCK_96M, BR_115200, UART_1STOPBIT, NULL);
	UART_RegistRecvCb(UART_1, UART1_rtoscli_recv);
#endif
	UART_Init(UART_2, UART_CLOCK_96M, BR_115200, UART_1STOPBIT, NULL);
	UART_RegistRecvCb(UART_2, CLI_rtoscli_recv);
	RETARGET_Init(UART2);
	TIMER_Init();
	CQ_Init();
	APBC_Init();

    //! Reset RN6752
    GPIO->GPIO_OE1  = 1;
    GPIO->GPIO_O1   = 0;
	
	//! LED
	GPIO->GPIO_O20	= 1;
	GPIO->GPIO_OE20	= 1;
	
	//! GPIO16 PULL UP
	GLB->TRSTN_PDN  = 0;
	GPIO->GPIO_OE16	= 0;
	GPIO->GPIO_O16	= 0;
	
	//GPIO Key
	GLB->PADIO8 	= 0;
    GKEY_SetDetPin(8,0);
	GPIO->GPIO_OE8  = 0;	//Input Mode
	
	//speaker
	GLB->TCK_PDN 	= 1;
	GPIO->GPIO_OE17 = 1;	
	GPIO->GPIO_O17 	= 0;

	
	//! RF PWR CTRL
	GPIO->GPIO_O18	= 1;
	GPIO->GPIO_OE18	= 1;
	
	printd(DBG_CriticalLvl, "SONiX SNCC70 High Speed Mode Start!\n");
}
#endif
//------------------------------------------------------------------------------
#ifdef BSP_D_SN93712_VBM_TX_V2
void BSP_DriversInit(void)
{
	WDT_Disable(WDT_RST);
	WDT_RST_Enable(WDT_CLK_EXTCLK, WDT_TIMEOUT_CNT);
	UART_Init(UART_2, UART_CLOCK_96M, BR_115200, UART_1STOPBIT, NULL);
	UART_RegistRecvCb(UART_2, CLI_rtoscli_recv);
	RETARGET_Init(UART2);
	TIMER_Init();
	CQ_Init();

	//! RF Power
	GPIO->GPIO_OE4 = 1;
	GPIO->GPIO_O4 = 1;

//	//! Reset Sensor
//    GPIO->GPIO_OE1 = 1;
//    GPIO->GPIO_O1  = 0;

	//! Speaker
	GPIO->GPIO_OE16	= 1;

	//! LED
	GPIO->GPIO_OE18	= 1;
	GPIO->GPIO_O18	= 1;
	GPIO->GPIO_OE19	= 1;
	GPIO->GPIO_O19	= 0;

	//! Pairing Key
	//justin 2020.03.20
	GKEY_SetDetPin(6,1);
	GPIO->GPIO_PULL_EN6 = 0;
	GPIO->GPIO_OE6 	= 0;
#if (APP_DOORPHONE_ENABLE==1)
	GKEY_SetDetPin(8,1);
	GPIO->GPIO_PULL_EN8 = 0;
	GPIO->GPIO_OE8 	= 0;

	GKEY_SetDetPin(17,1);
	GPIO->GPIO_PULL_EN17 = 0;
	GPIO->GPIO_OE17 	= 0;

	GKEY_SetDetPin(20,1);
	GPIO->GPIO_PULL_EN20 = 0;
	GPIO->GPIO_OE20 	= 0;
#endif
	printd(DBG_CriticalLvl, "SONiX SN93712 High Speed Mode Start!\n");
}
#endif
//------------------------------------------------------------------------------

#endif //! End #if defined BUC_CU || defined BUC_CAM

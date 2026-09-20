/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		bsp_SN93712_VBM_TX.h
	\brief		BSP Config header file
	\author		Wales Wang
	\version	0.1
	\date		2020/04/07
	\copyright	Copyright(C) 2019 SONiX Technology Co., Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------
#ifndef _BSP_SN93712_VBM_TX_H_
#define _BSP_SN93712_VBM_TX_H_

//------------------------------------------------------------------------------
//! CAM Board
//------------------------------------------------------------------------------
#if defined(BSP_D_SN93712_VBM_TX_V2)
//------------------------------------------------------------------------------
//! RF 
#ifdef A7130
	#define RF_CKO_GPIONUM			12
	#define RF_TXSW_GPIONUM			5	
#endif
//------------------------------------------------------------------------------
//! Sensor GPIO
#define SENSOR_RST_IO_EN(en)        (GPIO->GPIO_OE14 = en)
#define SENSOR_RST_IO(rst)          (GPIO->GPIO_O1 = rst)
#define SENSOR_RESET_OUT_EN         (GPIO->GPIO_OE1)
#define SENSOR_RESET_OUT            (GPIO->GPIO_O1) 
//------------------------------------------------------------------------------
//! LCD GPIO
//------------------------------------------------------------------------------
//! Audio
//! I2S Mode Define
#define BSP_ADO_I2S_MODE_ENABLE		0			
//! AEC / NR Process by HW / SW Define
#define APP_ADO_AEC_NR_TYPE			AEC_NR_SW									
//! Speaker GPIO
#define SPEAKER_EN(en)              (GPIO->GPIO_O16 = en)
//------------------------------------------------------------------------------
//! Power GPIO
//------------------------------------------------------------------------------
//! LED GPIO
#define PAIRING_LED_IO              (GPIO->GPIO_O19)
#define PAIRING_LED_IO_ENABLE       (GPIO->GPIO_OE19)
#define POWER_LED_IO                (GPIO->GPIO_O18)
#define POWER_LED_IO_ENABLE         (GPIO->GPIO_OE18)
#define GET_SIGNAL_LED_IO           (GPIO->GPIO_O19)    
#define SIGNAL_LED_IO(x)            (GPIO->GPIO_O19 = x)
#define SIGNAL_LED_IO_ENABLE(x)     (GPIO->GPIO_OE19 = x)
//! GPIO Key Function
#define GKEY_EN                     1
#define AKEY_DISABLE
//! KEY & Auto Pair
#define AUTO_PAIR_PIN               0
//------------------------------------------------------------------------------
//! BSP RTC Time Select
#define BSP_RTC_TIMER_SEL			RTC_TIMER_NULL
//------------------------------------------------------------------------------
//! SD Card
#define BSP_SD_CARD					0
//! SD Dectect PIN Enable
#define BSP_SD_DEC_PIN_CUST_EN		0
//------------------------------------------------------------------------------
//! SF Write Protect use GPIO
#define SF_WP_GPIN					14		//!< 0~13, >=14 is no wp pin
#endif	//! End of #ifdef BSP_D_SN93712_VBM_TX_V2
#endif	//! End of #ifndef _BSP_SN93712_VBM_TX_H_

/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		bsp_SNCC71_NVP6021_AHD_RX.h
	\brief		BSP Config header file
	\author		futao_jiang
	\version	0.1
	\date		2022/01/18
	\copyright	Copyright(C) 2022 SONiX Technology Co., Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------
#ifndef _BSP_SNCC71_NVP6021_AHD_RX_H_
#define _BSP_SNCC71_NVP6021_AHD_RX_H_

#ifdef BSP_D_SNCC71_NVP6021_AHD_RX_V4

//------------------------------------------------------------------------------
//! RF GPIO
//------------------------------------------------------------------------------
#ifdef RTC676x
	//#define RF_MODULE_OLD			    //V1.2/V1.2s/V2.0 Module
	#define RF_MODULE_NEW			    //V3.0 Module
#endif //! End of #ifdef RTC676x
#ifdef A7130
	#define RF_CKO_GPIONUM			1
	#define RF_TXSW_GPIONUM			7
#endif //!< End of #ifdef A7130
#ifdef S2019A
	#define sPRF_PWR_CTRL(ctrl)	do{ RTC_SetGPO_1(ctrl, ((!ctrl)?RTC_PullDownEnable:RTC_PullDownDisable));	}while(0)
	#define sPRF_PWR_RST		do{ sPRF_PWR_CTRL(0); osDelay(100); sPRF_PWR_CTRL(1); osDelay(50);			}while(0)
#endif //!< End of #ifdef S2019A
	
//------------------------------------------------------------------------------
//! LCD GPIO
//------------------------------------------------------------------------------
//! LT9211 reset pin
#define LCD_AHD_PRE_CONVERTER_RST	(PWM->PWM1_HIGH_CNT)
//! NVP6021 reset pin
#define LCD_AHD_NVP6021_RST			(GPIO->GPIO_O8)
//! NVP6021 NTSC/PAL selection
#define LCD_AHD_IS_SET_PAL_OUT()	(GPIO->GPIO_I10)

#define LCDBL_ENABLE(en)			((void)0)
#define LCD_BL_CTRL(en)				((void)0)
#define	LCD_PWR_ENABLE				((void)0)
#define	LCD_PWR_DISABLE				((void)0)
#define LCD_RESET_O(x)				((void)0)
#define LCD_BACKLIGHT_CTRL(LvL)		((void)0)

//------------------------------------------------------------------------------
//! Audio GPIO
//------------------------------------------------------------------------------
//! I2S Mode Define
#define BSP_ADO_I2S_MODE_ENABLE				0
//! AEC / NR Process by HW / SW Define
#define APP_ADO_AEC_NR_TYPE					AEC_NR_SW	
//! Speaker GPIO
//#define SPEAKER_EN(en)						(GPIO->GPIO_O12 = en)

//------------------------------------------------------------------------------
//! Power GPIO
//------------------------------------------------------------------------------
//! Empty

//------------------------------------------------------------------------------
//! LED GPIO
//------------------------------------------------------------------------------
//! CAM1
#define	GET_CAM1_LED_IO				GPIO->GPIO_I11
#define	CAM1_LED_IO(x)				GPIO->GPIO_O11 = x
#define	CAM1_LED_IO_ENABLE(x)		GPIO->GPIO_OE11 = x
//! CAM2
#define	GET_CAM2_LED_IO				( 0 )
#define	CAM2_LED_IO(x)
#define	CAM2_LED_IO_ENABLE(x)
//! CAM3
#define	GET_CAM3_LED_IO				( 0 )
#define	CAM3_LED_IO(x)
#define	CAM3_LED_IO_ENABLE(x)
//! CAM4
#define	GET_CAM4_LED_IO				( 0 )
#define	CAM4_LED_IO(x)
#define	CAM4_LED_IO_ENABLE(x)
//! Power LED
#define POWER_LED_IO				GPIO->GPIO_O2
#define POWER_LED_ENABLE			GPIO->GPIO_OE2
//! Signal LED
#define	GET_SIGNAL_LED_IO			( 0 )
#define	SIGNAL_LED_IO(x)
#define	SIGNAL_LED_IO_ENABLE(x)

//------------------------------------------------------------------------------
//! BSP RTC Time Select
//------------------------------------------------------------------------------
#define BSP_RTC_TIMER_SEL  			RTC_TIMER_INTERNAL

//------------------------------------------------------------------------------
//! SD Dectect PIN Enable
//------------------------------------------------------------------------------
#define BSP_SD_DEC_PIN_CUST_EN		0

//------------------------------------------------------------------------------
//! SF Write Protect use GPIO
//------------------------------------------------------------------------------
#define SF_WP_GPIN					14		//!< 0~13, >=14 is no wp pin



#endif	//!< End of #ifdef BSP_D_SNCC71_NVP6021_AHD_RX_V4

#endif	//! End of #ifndef _BSP_SNCC71_NVP6021_AHD_RX_H_

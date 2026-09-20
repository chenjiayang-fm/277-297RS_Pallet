/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		bsp_SN93711_FHD_REC_RX.h
	\brief		BSP Config header file
	\author		Wales Wang
	\version	0.2
	\date		2021/12/01
	\copyright	Copyright(C) 2021 SONiX Technology Co., Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------
#ifndef _BSP_SN93711_FHD_REC_RX_H_
#define _BSP_SN93711_FHD_REC_RX_H_

//------------------------------------------------------------------------------
//! CU Board #1
//------------------------------------------------------------------------------
//! RF GPIO
#ifdef A7130
#define RF_CKO_GPIONUM			1
#define RF_TXSW_GPIONUM			7
#endif	//!< End of #ifdef A7130

#ifdef S2019A
#define sPRF_PWR_CTRL(ctrl)		    {																		    \
                                    RTC_SetGPO_1(ctrl, ((!ctrl)?RTC_PullDownEnable:RTC_PullDownDisable));   \
                                    }
#define sPRF_PWR_RST			    {													\
                                    sPRF_PWR_CTRL(0);                               \
                                    osDelay(100);                                   \
                                    sPRF_PWR_CTRL(1);                               \
                                    osDelay(50);                                    \
                                    }
#endif	//!< End of #ifdef S2019A
//------------------------------------------------------------------------------
//! Sensor GPIO
//------------------------------------------------------------------------------
//! LCD GPIO
#define LCDBL_ENABLE(en)												\
									{									\
										GPIO->GPIO_O11  = en;			\
										GPIO->GPIO_OE11 = en;			\
										PWM->PWM_EN3 	= en;			\
									}
#define LCD_BL_CTRL(en)				LCDBL_ENABLE(en)									
#define	LCD_PWR_ENABLE				(GPIO->GPIO_O10 = 0)
#define	LCD_PWR_DISABLE				(GPIO->GPIO_O10 = 1)
#define	LCD_RESET_O(x)				(PWM->PWM_EN11 = x)
#define	LCD_RESET(x)				{PWM->PWM11_HIGH_CNT = 0;			\
									 osDelay(x);						\
									 PWM->PWM11_HIGH_CNT = 1;}	
#define LCD_BACKLIGHT_CTRL(LvL)		(PWM->PWM3_HIGH_CNT = LvL)
//------------------------------------------------------------------------------
//! Video GPIO
//------------------------------------------------------------------------------
//! Audio
//! I2S Mode Define
#define BSP_ADO_I2S_MODE_ENABLE		0			
//! AEC / NR Process by HW / SW Define
#define APP_ADO_AEC_NR_TYPE			AEC_NR_SW	
//! Speaker GPIO
#define SPEAKER_EN(en)				(GPIO->GPIO_O12 = en)
//------------------------------------------------------------------------------
//! Power GPIO
//------------------------------------------------------------------------------
//! LED GPIO
#define	GET_SIGNAL_LED_IO			(GPIO->GPIO_O3)									
#define	SIGNAL_LED_IO(x)			(GPIO->GPIO_O3 = x)
#define	SIGNAL_LED_IO_ENABLE(x)		(GPIO->GPIO_OE3 = x)
#define POWER_LED_IO				(GPIO->GPIO_O2)
#define POWER_LED_IO_ENABLE			(GPIO->GPIO_OE2)

#if defined(BSP_D_SN93701_SSD2828_RX_V5)
//#define POWER_UVC1_IO_ENABLE		(GPIO->GPIO_OE6)
//#define POWER_UVC2_IO_ENABLE		(GPIO->GPIO_OE6)
//#define POWER_UVC1_IO				(GPIO->GPIO_O6)
//#define POWER_UVC2_IO				(GPIO->GPIO_O6)
#elif defined(BSP_D_SN93701_TC358778_RX_V6)
//#define POWER_UVC1_IO_ENABLE		(GPIO->GPIO_OE0)
//#define POWER_UVC2_IO_ENABLE		(GPIO->GPIO_OE13)
//#define POWER_UVC1_IO				(GPIO->GPIO_O0)
//#define POWER_UVC2_IO				(GPIO->GPIO_O13)
#endif

//------------------------------------------------------------------------------
//! BSP RTC Time Select
#define BSP_RTC_TIMER_SEL			RTC_TIMER_INTERNAL
//------------------------------------------------------------------------------
//! SD Dectect PIN Enable
#define BSP_SD_DEC_PIN_CUST_EN		0
//------------------------------------------------------------------------------
//! SF Write Protect use GPIO
#define SF_WP_GPIN					14		//!< 14 is no wp pin
#endif	//!< End of #ifndef _BSP_CONFIG_CU_SN93711_H_


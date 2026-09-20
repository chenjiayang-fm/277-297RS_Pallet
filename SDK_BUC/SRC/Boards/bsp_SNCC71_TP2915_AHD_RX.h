/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		bsp_SNCC71_TP2915_AHD_RX.h
	\brief		BSP Config header file
	\author		Wales Wang
	\version	0.2
	\date		2020/06/02
	\copyright	Copyright(C) 2020 SONiX Technology Co., Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------
#ifndef _BSP_SNCC71_TP2915_AHD_RX_H_
#define _BSP_SNCC71_TP2915_AHD_RX_H_

//------------------------------------------------------------------------------
//! CU Board
//------------------------------------------------------------------------------
#ifdef BSP_D_SNCC71_TP2915_AHD_RX_V3
//------------------------------------------------------------------------------
//! RF GPIO
#ifdef RTC676x
#define RF_MODULE_OLD			    //V1.2/V1.2s/V2.0 Module
//#define RF_MODULE_NEW			    //V3.0 Module
#endif
#ifdef A7130
#define RF_CKO_GPIONUM			1
#define RF_TXSW_GPIONUM			7
#endif	//!< End of #ifdef A7130

#ifdef S2019A
#define sPRF_PWR_CTRL(ctrl)		    {								\
									RTC_SetGPO_1(ctrl, ((!ctrl)?RTC_PullDownEnable:RTC_PullDownDisable));			\
                                    }
#define sPRF_PWR_RST			    {								\
                                    sPRF_PWR_CTRL(0);               \
                                    osDelay(100);                   \
                                    sPRF_PWR_CTRL(1);               \
                                    osDelay(50);                    \
                                    }
#endif
//------------------------------------------------------------------------------
//! Sensor GPIO
//------------------------------------------------------------------------------
//! LCD GPIO
#define LCDBL_ENABLE(en)
#define LCD_BL_CTRL(en)				((void)0)
#define	LCD_PWR_ENABLE				(PWM->PWM_EN4 = 1)
#define	LCD_PWR_DISABLE				(PWM->PWM_EN4 = 0)
#define LCD_RESET_O(x)				(GPIO->GPIO_O3 = x)
#define LCD_BACKLIGHT_CTRL(LvL)		
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
#define	GET_SIGNAL_LED_IO			(0)						
#define	SIGNAL_LED_IO(x)
#define	SIGNAL_LED_IO_ENABLE(x)			
#define CAM1_LED_IO_ENABLE(x)		{PWM->PWM0_RATE  	= 127;		\
									 PWM->PWM0_PERIOD 	= 1;		\
									 PWM->PWM_EN0    	= x;}
#define	GET_CAM1_LED_IO				(PWM->PWM0_HIGH_CNT)
#define	CAM1_LED_IO(x)				(PWM->PWM0_HIGH_CNT = (x & 1))	
#define CAM2_LED_IO_ENABLE(x)	   	{PWM->PWM1_RATE  	= 127;		\
									 PWM->PWM1_PERIOD 	= 1;		\
									 PWM->PWM_EN1    	= x;}
#define	GET_CAM2_LED_IO				(PWM->PWM1_HIGH_CNT)
#define	CAM2_LED_IO(x)				(PWM->PWM1_HIGH_CNT = (x & 1))	
#define CAM3_LED_IO_ENABLE(x)		(GPIO->GPIO_OE8 = x)
#define	GET_CAM3_LED_IO				(GPIO->GPIO_O8)
#define	CAM3_LED_IO(x)				(GPIO->GPIO_O8 = x)	
#define CAM4_LED_IO_ENABLE(x)	   	{PWM->PWM3_RATE  	= 127;		\
									 PWM->PWM3_PERIOD 	= 1;		\
									 PWM->PWM_EN3    	= x;}
#define	GET_CAM4_LED_IO				(PWM->PWM3_HIGH_CNT)
#define	CAM4_LED_IO(x)				(PWM->PWM3_HIGH_CNT = (x & 1))
#define POWER_LED_IO				(GPIO->GPIO_O2)
#define POWER_LED_IO_ENABLE			(GPIO->GPIO_OE2)	
//------------------------------------------------------------------------------
//! BSP RTC Time Select
#define BSP_RTC_TIMER_SEL  			RTC_TIMER_INTERNAL
//------------------------------------------------------------------------------
//! SD Dectect PIN Enable
#define BSP_SD_DEC_PIN_CUST_EN		0
//------------------------------------------------------------------------------
//! SF Write Protect use GPIO
#define SF_WP_GPIN					14		//!< 0~13, >=14 is no wp pin
#endif	//!< End of #ifdef BSP_D_SNCC71_TP2915_AHD_RX_V3
#endif	//! End of #ifndef _BSP_SNCC71_TP2915_AHD_RX_H_


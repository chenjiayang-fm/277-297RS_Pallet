/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		bsp_SNCC71_CARCAM_RX.h
	\brief		BSP Config header file
	\author		Wales Wang
	\version	0.1
	\date		2020/04/07
	\copyright	Copyright(C) 2019 SONiX Technology Co., Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------
#ifndef _BSP_SNCC71_CARCAM_RX_H_
#define _BSP_SNCC71_CARCAM_RX_H_

//------------------------------------------------------------------------------
//! CU Board
//------------------------------------------------------------------------------
#ifdef BSP_D_SNCC71_GM8285C_RX_V2
//------------------------------------------------------------------------------
//! RF GPIO
#ifdef A7130
#define RF_CKO_GPIONUM			1
#define RF_TXSW_GPIONUM			7
#endif	//!< End of #ifdef A7130

#ifdef S2019A
#define sPRF_PWR_CTRL(ctrl)		    {								\
                                    GPIO->GPIO_O11 = ctrl;          \
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
#define LCDBL_ENABLE(en)											\
//								    {								\
//									PWM->PWM_EN3 	= en;			\
//									PWM->PWM_EN5 	= en;			\
//								    }
//#define LCD_BL_CTRL(en)				LCDBL_ENABLE(en)
//#define	LCD_PWR_ENABLE				(PWM->PWM_EN4 = 1)
//#define	LCD_PWR_DISABLE				(PWM->PWM_EN4 = 0)
//#define LCD_RESET_O(x)				(PWM->PWM_EN6 = x)
//#define LCD_BACKLIGHT_CTRL(LvL)		(PWM->PWM3_HIGH_CNT = LvL)




#define GET_PWR_REPORT				(GPIO->GPIO_I5)	
#define SET_PANEL_PWM(x)            (PWM->PWM8_HIGH_CNT = x)
#define SET_VOLUME_PWM(x)            (PWM->PWM9_HIGH_CNT = x)
#define SET_VOLUME_IO          	  	(GPIO->GPIO_O20) 
#define BUZZER_DI(x)                do {                                 \
                                        if (tUI_CuSetting.ubBuzzer == 1) \
                                        {                                \
                                            PWM->PWM_EN4 = 1;            \
                                            osDelay(x);                  \
                                            PWM->PWM_EN4 = 0;            \
                                        }                                \
                                    } while(0)


//------------------------------------------------------------------------------
//! Video GPIO
//------------------------------------------------------------------------------
//! Audio
//! I2S Mode Define
#define BSP_ADO_I2S_MODE_ENABLE		0			
//! AEC / NR Process by HW / SW Define
#define APP_ADO_AEC_NR_TYPE			AEC_NR_SW									
//! Speaker GPIO
//#define SPEAKER_EN(en)				(GPIO->GPIO_O12 = en)	
//------------------------------------------------------------------------------
//! Power GPIO
//------------------------------------------------------------------------------
//! LED GPIO
#define	GET_SIGNAL_LED_IO			(GPIO->GPIO_O3)									
#define	SIGNAL_LED_IO(x)			(GPIO->GPIO_O3 = x)
#define	SIGNAL_LED_IO_ENABLE(x)		(GPIO->GPIO_OE3 = x)									
//#define POWER_LED_IO				(GPIO->GPIO_O2)
//#define POWER_LED_IO_ENABLE			(GPIO->GPIO_OE2)	
//------------------------------------------------------------------------------
//! BSP RTC Time Select
#define BSP_RTC_TIMER_SEL			RTC_TIMER_INTERNAL
//------------------------------------------------------------------------------
//! SD Dectect PIN Enable
#define BSP_SD_DEC_PIN_CUST_EN		0
//------------------------------------------------------------------------------
//! SF Write Protect use GPIO
#define SF_WP_GPIN					14		//!< 0~13, >=14 is no wp pin

//! STONKAM ST-HDW127DC-MB-V42 board
#define GET_PWR_REPORT				(GPIO->GPIO_I5)	           // ��ȡ��Դ״̬����Ƿ�γ���Դ
#define SET_PANEL_PWM(x)            (PWM->PWM8_HIGH_CNT = x)   // ���ñ����С
#define SET_VOLUME_PWM(x)           (PWM->PWM9_HIGH_CNT = x)   // ����������С
#define SET_VOLUME_IO          	  	(GPIO->GPIO_O20)           //音频4875的power使能
#define POWER_LED_IO				(GPIO->GPIO_O13)           // �����屳�����
#define POWER_LED_IO_ENABLE			(GPIO->GPIO_OE13)          // �����屳�����ʹ��


#endif	//!< End of #ifdef BSP_D_SNCC71_GM8285C_RX_V2
#endif	//! End of #ifndef _BSP_SNCC71_CARCAM_RX_H_


/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		BSP_Board.c
	\brief		BUC CU/CAM Demo Board
	\author		Hanyi Chiu
	\version	0.13
	\date		2022/02/23
	\copyright	Copyright(C) 2022 SONiX Technology Co., Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------
#if defined BUC_CU || defined BUC_CAM

#include "BSP.h"
#include "_510PF.h"
#include "APP_CFG.h"
#if (defined(BSP_SN93711_FHD_REC_RX_V4) || defined(BSP_D_SN93701_SSD2828_RX_V5) || defined(BSP_D_SN93701_TC358778_RX_V6))
void BSP_BoardInit(void)
{
	//! UART
	GLB->PADIO22 = 2;
	GLB->PADIO23 = 2;

	//! LCD
	//! GPIO SPI
	GLB->PADIO47 = 1;
	GLB->PADIO48 = 1;
	GLB->PADIO49 = 1;
	GLB->PADIO50 = 1;
	//! LCD Pin
	GLB->PADIO26 = 5;
	GLB->PADIO27 = 5;
	GLB->PADIO28 = 5;
	GLB->PADIO29 = 5;
	GLB->PADIO30 = 5;
	GLB->PADIO31 = 5;
	GLB->PADIO32 = 5;
	GLB->PADIO33 = 5;
	GLB->PADIO34 = 5;
	GLB->PADIO35 = 5;
	GLB->PADIO36 = 5;

	//! RF SPI
	GLB->PADIO18 = 1;
	GLB->PADIO19 = 1;
	GLB->PADIO20 = 1;
	GLB->PADIO15 = 0;
	GLB->PADIO21 = 0;

	//! SD
	GLB->PADIO0  = 4;
	GLB->PADIO1  = 4;
	GLB->PADIO2  = 4;
	GLB->PADIO3  = 4;
	GLB->PADIO4  = 4;
	GLB->PADIO5  = 4;
	//GLB->PADIO6  = 4;
	GLB->PADIO7  = 4;

	//! Speaker
	GLB->PADIO54 = 0;

	//! LED
	GLB->PADIO16 = 0;
	GLB->PADIO17 = 0;
//	GLB->PADIO55 = 7;
//	GLB->PADIO56 = 7;
	GLB->PADIO57 = 7;

	//! BL
	GLB->PADIO51 = 7;

	//! LCD Power
	GLB->PADIO10 = 7;
	GLB->PADIO24 = 7;
	GLB->PADIO38 = 7;
	GLB->PADIO52 = 0;
	//! LCD Reset
	GLB->PADIO11 = 7;
}
#endif
//------------------------------------------------------------------------------
#ifdef BSP_D_SNCC71_GM8285C_RX_V2
void BSP_BoardInit(void)
{
	//! UART1 for debug
	GLB->PADIO15 = 1; //UART1_RX
	GLB->PADIO51 = 2; //UART1_TX

	//! UART2 for receive coord infomation to draw red box and wakeup sleep Tx
	GLB->PADIO49 = 2; //UART2_TX
	GLB->PADIO50 = 2; //UART2_RX

	//! UART2 修改串口漏电问题，改为gpio然后拉低测试
//	GLB->PADIO49 = 0; //UART2_TX
//	GLB->PADIO50 = 0; //UART2_RX
//	GPIO->GPIO_OE7 = 1;
//	GPIO->GPIO_OE8 = 1;
//	GPIO->GPIO_O7 = 0;
//	GPIO->GPIO_O8 = 0;


	//! LCD
	//! GPIO SPI
	GLB->PADIO47 = 1;
	GLB->PADIO48 = 1;
	//GLB->PADIO49 = 1;
	//GLB->PADIO50 = 1;
	//! LCD Pin
	GLB->PADIO26 = 5;
	GLB->PADIO27 = 5;
	GLB->PADIO28 = 5;
	GLB->PADIO29 = 5;
	GLB->PADIO30 = 5;
	GLB->PADIO31 = 5;
	GLB->PADIO32 = 5;
	GLB->PADIO33 = 5;
	GLB->PADIO34 = 5;
	GLB->PADIO35 = 5;
	GLB->PADIO36 = 5;

	//! RF SPI
	GLB->PADIO18 = 1; // RF1_SPI_CLK
	GLB->PADIO19 = 1; // RF1_SPI_CS
	GLB->PADIO20 = 1; // RF1_SPI_MO
	GLB->PADIO17 = 0; // GPIO3 RF_RST
	GLB->PADIO21 = 0; // GPIO7 RF_SDO

	//! SD
	GLB->PADIO0  = 4;
	GLB->PADIO1  = 4;
	GLB->PADIO2  = 4;
	GLB->PADIO3  = 4;
	GLB->PADIO4  = 4;
	GLB->PADIO5  = 4;
	GLB->PADIO6  = 4;
	GLB->PADIO7  = 4;

	GLB->PADIO48 = 7; // PWM0 for PNL_POWER_CTRL use as GPIO
	GLB->PADIO22 = 7; // PWM6 for POWER_VGH_CTRL use as GPIO
	GLB->PADIO24 = 7; // PWM8 for PWM_BACKLIGHT adjust backlight
	GLB->PADIO25 = 7; // PWM9 for PWM_AUDIO adjust volume

	GLB->PADIO23 = 7; // PWM7 for RF_POWER_CTRL use as GPIO

	GLB->PADIO55 = 5; // I2C1_SCL
	GLB->PADIO56 = 5; // I2C1_SDA I2C for drive of TouchPanel
	GLB->PADIO53 = 7; // PWM5 for TOUCH_RST use as GPIO
	GLB->PADIO54 = 0; // GPIO12 for TOUCH_INT

	GLB->PADIO25 = 7; // PWM9 for PWM_AUDIO adjust volume

	GLB->PADIO47 = 0; // GPIO5 for PVCC_DOWN_DET

	GLB->PADIO16 = 0; // GPIO2 for CM4301_RESET

	GLB->PADIO13 = 0; // GPIO13 for LED_CTRL
	GLB->PADIO14 = 0; // GPIO0 for IR NEC Decode

	GLB->JTAG_EN = 0; //配置JTAG功能引脚为GPIO

	//buzzer
    GLB->PADIO52 = 7; //PWM4

	
 	// GPIO  0  1  2  3  4  5  6  7  8  9  10  11  12  13  14  15
    //       o  o  o  o     o     o                o   o

	// PWM   0  1  2  3  4  5  6  7  8  9  10  11  12  13  14  15
    //       o              o  o  o  o  o      
}
#endif
//------------------------------------------------------------------------------
#ifdef BSP_D_SNCC71_TP2915_AHD_RX_V3
void BSP_BoardInit(void)
{
	//! UART
	GLB->PADIO22 = 2;
	GLB->PADIO23 = 2;

	//! LCD
	//! GPIO SPI
	//GLB->PADIO47 = 0;
	//GLB->PADIO48 = 1;
	//GLB->PADIO49 = 1;
	//GLB->PADIO50 = 1;
	//! LCD Pin
	GLB->PADIO26 = 5;
	GLB->PADIO27 = 5;
	GLB->PADIO28 = 5;
	GLB->PADIO29 = 5;
	GLB->PADIO30 = 5;
	GLB->PADIO31 = 5;
	GLB->PADIO32 = 5;
	GLB->PADIO33 = 5;
	GLB->PADIO34 = 5;
	GLB->PADIO35 = 5;
	GLB->PADIO36 = 5;

	//! RF SPI
	GLB->PADIO18 = 1;
	GLB->PADIO19 = 1;
	GLB->PADIO20 = 1;
	GLB->PADIO15 = 0;
	GLB->PADIO21 = 0;

	//! SD
	GLB->PADIO0  = 4;
	GLB->PADIO1  = 4;
	GLB->PADIO2  = 4;
	GLB->PADIO3  = 4;
	GLB->PADIO4  = 4;
	GLB->PADIO5  = 4;
	GLB->PADIO6  = 4;
	GLB->PADIO7  = 4;

	//! Speaker
	GLB->PADIO54 = 0;

	//! LED
	GLB->PADIO16 = 0;
	GLB->PADIO48 = 7;
	GLB->PADIO49 = 7;
	GLB->PADIO50 = 0;
	GLB->PADIO51 = 7;
	//! NTSC
	GLB->PADIO10 = 7;
	GLB->PADIO24 = 0;
	GLB->PADIO38 = 7;
	GLB->PADIO66 = 7;

	//! BL
	//GLB->PADIO51 = 7;
	GLB->PADIO53 = 7;

	//! RF Power Reset
	GLB->PADIO11 = 0;

	//! LCD Power
	GLB->PADIO52 = 7;
	//! LCD Reset
	GLB->PADIO17 = 0;
	
	GLB->PADIO13 = 4;
	GLB->PADIO14 = 4;

#if (BSP_RTC_TIMER_SEL == RTC_TIMER_EXTERNAL)
	//! I2C2 for RTC PCF85063A
	GLB->PADIO13 = 4;
	GLB->PADIO14 = 4;
	//! RTC PCF85063A trigger
	GLB->PADIO10 = 7;	//! change to mode-PWM10
	GLB->PADIO24 = 0;	//! GPIO10
#endif
}
#endif
//------------------------------------------------------------------------------
#ifdef BSP_D_SNCC71_NVP6021_AHD_RX_V4
void BSP_BoardInit( void ) {
	//! UART2
	GLB->PADIO22 = 2;
	GLB->PADIO23 = 2;
	
	//! I2C2
	GLB->PADIO13 = 4;
	GLB->PADIO14 = 4;
	
	//! RF1 SPI(A7130)
	GLB->PADIO15 = 0; //! GPIO1
	GLB->PADIO18 = 1;
	GLB->PADIO19 = 1;
	GLB->PADIO20 = 1;
	GLB->PADIO21 = 0; //! GPIO7

	//! LCD
	//! LCD Pin mux
	GLB->PADIO26 = 5;
	GLB->PADIO27 = 5;
	GLB->PADIO28 = 5;
	GLB->PADIO29 = 5;
	GLB->PADIO30 = 5;
	GLB->PADIO31 = 5;
	GLB->PADIO32 = 5;
	GLB->PADIO33 = 5;
	GLB->PADIO34 = 5;
	GLB->PADIO35 = 5;
	GLB->PADIO36 = 5;
	//! Pre-Converter reset
	GLB->PADIO49 = 7; //! PWM1
	//! AHD Encoder(NVP6021) reset
	GLB->PADIO50 = 0; //! GPIO8

	//! SD
	GLB->PADIO0  = 4;
	GLB->PADIO1  = 4;
	GLB->PADIO2  = 4;
	GLB->PADIO3  = 4;
	GLB->PADIO4  = 4;
	GLB->PADIO5  = 4;
	GLB->PADIO7  = 4;

	//! LED
	GLB->PADIO25 = 0; //! CAM_LED = GPIO11
	GLB->PADIO16 = 0; //! PWR_LED = GPIO2
	
	//! NTSC/PAL detection
	GLB->PADIO10 = 7; //! Bypass GPIO10 in Padio 10
	GLB->PADIO24 = 0; //! GPIO10, Input=1 or HighZ=PAL, Input=0=NTSC
	GLB->PADIO24_PUN = 0; //! Enable internal pull up resistor
	
#if (BSP_RTC_TIMER_SEL == RTC_TIMER_EXTERNAL)
	#error "Not supported by current hardware!"
#endif
}
#endif //! End of #ifdef BSP_D_SNCC71_NVP6021_AHD_RX_V4
//------------------------------------------------------------------------------
#if (defined(BSP_SNCC70_AHD_TP9950_TX_V1) || defined(BSP_SNCC70_AHD_RN6752M_TX_V1))
void BSP_BoardInit(void)
{
	//! SD
	GLB->PADIO0 = 4;
	GLB->PADIO1 = 4;
	GLB->PADIO2 = 4;
	GLB->PADIO3 = 4;
	GLB->PADIO4 = 4;
	GLB->PADIO5 = 4;
//	GLB->PADIO6 = 4;
	GLB->PADIO7 = 4;

//RN6752 REST    
	GLB->PADIO6 = 0;	
	
	//! I2C-2
	GLB->PADIO13 = 4;
	GLB->PADIO14 = 4;
	
	//! RF_SPI
	GLB->PADIO18 = 1;
	GLB->PADIO19 = 1;
	GLB->PADIO20 = 1;
//	GLB->PADIO28 = 0;
    GLB->PADIO9 = 0;
	GLB->PADIO27 = 0;
#ifdef RTC676x
	//! Richwave RF PWR CTRL
	//GLB->PADIO12 = 1;
	GLB->PADIO26 = 0;
#endif	
	
	//! UART
	GLB->PADIO22 = 2;
	GLB->PADIO23 = 2;
	
	//! LED
	GLB->PADIO29 = 0;
	GLB->PADIO30 = 0;
	
	//! Auto Pair Pin (GPIO16)
	GLB->JTAG_EN = 0;
}
#endif
//------------------------------------------------------------------------------
#if (defined(BSP_D_SNCC70_TX_V2) || defined(BSP_SN93710_FHD_REC_TX_V4))
void BSP_BoardInit(void)
{
	//! UART
	GLB->PADIO22 = 2;
	GLB->PADIO23 = 2;

	//! RF_SPI
	GLB->PADIO18 = 1;
	GLB->PADIO19 = 1;
	GLB->PADIO20 = 1;

//	GLB->PADIO28 = 0;
#if (APP_DOORPHONE_ENABLE==0)
    GLB->PADIO9 = 0;
#endif
#if APP_MAC_FUNC_ENABLE
	GLB->PADIO27 = 4;	//MAX_TXD[0]
	GLB->PADIO28 = 4;	//MAX_TXD[1]
	
	GLB->PADIO38 = 0;	//GIO2(AMIC)	
	GLB->PADIO39 = 0;	//CKO(AMIC)
#else
	GLB->PADIO27 = 0;
#endif

	//! SD
	GLB->PADIO0 = 4;
	GLB->PADIO1 = 4;
	GLB->PADIO2 = 4;
	GLB->PADIO3 = 4;
	GLB->PADIO4 = 4;
	GLB->PADIO5 = 4;
	GLB->PADIO6 = 4;
	GLB->PADIO7 = 4;

	//! I2C-2
	GLB->PADIO13 = 4;
	GLB->PADIO14 = 4;

    //RN6752 REST
    GLB->PADIO6 = 0;

	//! Speaker
#if APP_MAC_FUNC_ENABLE
	GLB->PADIO32 = 4;	//MAX_RXD[1]
	
	GLB->PADIO43 = 0;	//SPK_EN
#else
	GLB->PADIO32 = 0;
#endif

	//! LED
#if APP_MAC_FUNC_ENABLE
	GLB->PADIO29 = 4;	//MAX_RX_DV
	GLB->PADIO30 = 4;	//MAX_RX_ER
	GLB->PADIO31 = 4;	//MAX_RXD[0]
	
	GLB->PADIO40 = 0;	//LED
	GLB->PADIO41 = 0;	//LED
	GLB->PADIO42 = 0;	//LED
#else
	GLB->PADIO29 = 0;
	GLB->PADIO30 = 0;
	GLB->PADIO31 = 0;
#endif

#ifdef S2019A
	//! Power ctrl
	GLB->PADIO12 = 1;
	GLB->PADIO26 = 0;
#endif
#ifdef RTC676x
	//! Richwave RF PWR CTRL
	//GLB->PADIO12 = 1;
	GLB->PADIO26 = 0;
#endif

#if !APP_MAC_FUNC_ENABLE
#if ( (BSP_ADO_I2S_MODE_ENABLE == 1) | (APP_ADO_AEC_NR_TYPE == AEC_NR_HW) )
	//! Tx I2S
#if (APP_DOORPHONE_ENABLE==0)
	GLB->PADIO37 = 6;
	GLB->PADIO38 = 6;
#endif
	GLB->PADIO39 = 6;
	GLB->PADIO40 = 6;
	GLB->PADIO41 = 6;
#endif
#endif
#if (APP_DOORPHONE_ENABLE==1)
    GLB->PADIO9 = 7;
    GLB->PADIO10 = 7;
    GLB->PADIO24 = 7;

    GLB->PADIO8  = 0;   // Pairing to RX2
	GLB->PADIO37 = 0;   // Calling Key1
	GLB->PADIO38 = 0;   // Calling Key2
#endif

#if APP_ADO_AEC_NR_TYPE == AEC_NR_HW
	//! I2C-1
	GLB->PADIO6  = 5;	//i2c_0 clk
	GLB->PADIO46 = 1;	//i2c_0 sda
#endif

#if APP_MAC_FUNC_ENABLE
	GLB->PADIO26 = 4;	//MAX_TXEN
	GLB->PADIO37 = 0;	//WIFI_PWREN
	
	GLB->PADIO33 = 4;	//MAX_MDC
	GLB->PADIO34 = 4;	//MAX_MDIO
	GLB->PADIO35 = 4;	//MAX_PHY_LINKSTS
	GLB->PADIO36 = 4;	//MAX_PDN_PHY
#endif
}
#endif
//------------------------------------------------------------------------------
#if (defined(BSP_SNCC72_AHD_RN6752M_TX_V1)||defined(BSP_SNCC72_AHD_TP9950_TX_V2))
void BSP_BoardInit(void)
{
#ifdef RTC676x
	//! RF_SPI(Richwave RTC676x RF)	
	GLB->PADIO4 = 6;	//SPI_MO
	GLB->PADIO5 = 6;	//SPI_MI
	GLB->PADIO6 = 6;	//SPI_CLK
   
	GLB->PADIO42 = 0;	//RSTN, GPIO 0	
	GLB->PADIO41 = 0;	//IRQN, GPIO 13 (Input)	
	//==============================
	//Set other mode for "GPI 13"	
	GLB->PADIO27 = 7;	
	//==============================	
#endif

#ifdef A7130
	//! A7130	
	GLB->PADIO4 = 3;
	GLB->PADIO6 = 3;
	GLB->PADIO7 = 3;
	GLB->PADIO5 = 0;
	GLB->PADIO41 = 0;
	GLB->PADIO27 = 7;
#endif	

	//! I2C-2
	GLB->PADIO13 = 4;
	GLB->PADIO14 = 4;

    //RN6752 REST
    GLB->PADIO15 = 0;
	
	//! UART
	GLB->PADIO22 = 2;
	GLB->PADIO23 = 2;

	//! RF PWR CTRL
	//! TMS GPIO18
	GLB->JTAG_EN = 0;
	
	//! KEY TDI GPIO19
	//! LCD TD0 GPIO20
}
#endif
//------------------------------------------------------------------------------
#ifdef BSP_D_SNCC72_TX_V1
void BSP_BoardInit(void)
{
#ifdef RTC676x
	//! RF_SPI(Richwave RTC676x RF)	
	GLB->PADIO4 = 6;	//SPI_MO
	GLB->PADIO5 = 6;	//SPI_MI
	GLB->PADIO6 = 6;	//SPI_CLK
   
	GLB->PADIO42 = 0;	//RSTN, GPIO 0	
	GLB->PADIO41 = 0;	//IRQN, GPIO 13 (Input)	
	//==============================
	//Set other mode for "GPI 13"	
	GLB->PADIO27 = 7;	
	//==============================	
#endif	

#ifdef A7130
	//! A7130	
	GLB->PADIO4 = 3;
	GLB->PADIO6 = 3;
	GLB->PADIO7 = 3;
	GLB->PADIO5 = 0;
	GLB->PADIO41 = 0;
	GLB->PADIO27 = 7;
#endif	

	//! I2C-2
	GLB->PADIO13 = 4;
	GLB->PADIO14 = 4;

    //RN6752 REST
    GLB->PADIO15 = 0;
	
	//! UART
	GLB->PADIO22 = 2;
	GLB->PADIO23 = 2;

	//! RF PWR CTRL
	//! TMS GPIO18
	GLB->JTAG_EN = 0;
	
	//! KEY TDI GPIO19
	//! LCD TD0 GPIO20
}
#endif
//------------------------------------------------------------------------------
#ifdef BSP_D_SN93712_VBM_TX_V2
void BSP_BoardInit(void)
{
	//! UART
	GLB->PADIO22 = 2;
	GLB->PADIO23 = 2;

	//! RF Power
	GLB->PADIO4  = 0;
	//! RF SPI IO
	GLB->PADIO39 = 2;
	GLB->PADIO42 = 2;
	GLB->PADIO41 = 2;
	//! RF GKO IO
	GLB->PADIO12 = 4;
	GLB->PADIO26 = 2;
	GLB->PADIO40 = 0;
	//! RF TXSW IO
	GLB->PADIO5  = 0;

	//! Reset Sensor
	GLB->PADIO1  = 4;
	GLB->PADIO15 = 0;

	//! I2C-2
	GLB->PADIO13 = 4;
	GLB->PADIO14 = 4;

	//! Pairing Key
	GLB->PADIO6  = 0;
#if (APP_DOORPHONE_ENABLE==1)
    GLB->PADIO8  = 0;
#endif
	//! LED
	GLB->JTAG_EN = 0;
}
#endif
//------------------------------------------------------------------------------
#ifdef BSP_D_SN93714_TX_V1
void BSP_BoardInit(void)
{
	//! AKEY PIN 
	GLB->PADIO8  = 7;	//!< ADC0
	
	//! RF Reset PIN 
//	GLB->PADIO9  = 0;
	
	//! Sensor
	GLB->PADIO13 = 4;	//!< I2C-2 SCL
	GLB->PADIO14 = 4;	//!< I2C-2 SDA
//	GLB->PADIO15 = 0;	//!< Sensor Reset Pin	(GPIO1)

#ifdef A7130
	//! RF_SPI
	GLB->PADIO18 = 1;	//!< RF CLK
	GLB->PADIO19 = 1;	//!< RF CS
	GLB->PADIO20 = 1;	//!< RF MO
//	GLB->PADIO21 = 0;	//!< RF CKO				(GPIO7)
//	GLB->PADIO55 = 0;	//!< RF TXSW			(GPIO13)	
	GLB->PADIO7  = 7;	
#endif
	GLB->PADIO33 = 7;	//!< RF 5V Power		(PWM1)
	GLB->PADIO41 = 7;
	//! SD
	GLB->PADIO26 = 2;	//!< SD3 CLK
	GLB->PADIO27 = 2;	//!< SD3 CMD
	GLB->PADIO28 = 2;	//!< SD3 D0
	GLB->PADIO29 = 2;	//!< SD3 D1
	GLB->PADIO30 = 2;	//!< SD3 D2
	GLB->PADIO31 = 2;	//!< SD3 D3
	GLB->PADIO36 = 0;	//!< SD CD				(GPIO8)
	GLB->PADIO22 = 7;
	
	//! SF_WP
//	GLB->PADIO32 = 0;	//!< NULL				(GPIO4)

	//! LED
//	GLB->PADIO34 = 0;	//!< LEDR				(GPIO6)
	GLB->PADIO35 = 7;	//!< LEDG				(PWM3)
//	GLB->PADIO36 = 0;	//!< LEDB				(GPIO8)

	//! I2C-1
	GLB->PADIO52 = 6;	//!< I2C-1 SCL
	GLB->PADIO53 = 6;	//!< I2C-1 SDA
	
	//! SPK_EN
//	GLB->PADIO54 = 0;	//!< 					(GPIO12)
	
	//! UART
	GLB->PADIO56 = 2;	//!< UART0 TX
	GLB->PADIO57 = 2;	//!< UART0 RX
//	GLB->PADIO23 = 0;
	
#if ( (BSP_ADO_I2S_MODE_ENABLE == 1) | (APP_ADO_AEC_NR_TYPE == AEC_NR_HW) )
	//! I2S
	GLB->PADIO32 = 2;	//!< MLCK
	GLB->PADIO33 = 2;	//!< BCLK
	GLB->PADIO34 = 2;	//!< LRCK
	GLB->PADIO35 = 2;	//!< DO
	GLB->PADIO36 = 2;	//!< D1
#endif
}
#endif
//------------------------------------------------------------------------------
#ifdef BSP_D_SN93716_TX_V1
void BSP_BoardInit(void)
{
	//! AKEY 
	GLB->PADIO8  = 7;	//!< ADC0
	
	//! LED
//	GLB->PADIO9  = 0;	//!< LED B				(GPO9)
	GLB->PADIO33 = 7;	//!< LED R				(PWM1)
	//! Sensor
	GLB->PADIO13 = 4;	//!< I2C-2 SCL
	GLB->PADIO14 = 4;	//!< I2C-2 SDA
//	GLB->PADIO15 = 0;	//!< Sensor Reset Pin	(GPIO1)
	
	//! RF_SPI
	GLB->PADIO18 = 1;	//!< RF CLK
	GLB->PADIO19 = 1;	//!< RF CS
	GLB->PADIO20 = 1;	//!< RF MO
//	GLB->PADIO21 = 0;	//!< RF CKO				(GPI7)
//	GLB->PADIO36 = 0;	//!< RF TXSW			(GPI8)
	GLB->PADIO35 = 7;	//!< RF Reset			(PWM3)
	GLB->PADIO43 = 7;	//!< RF 5V Power		(PWM11)
	GLB->PADIO7  = 7;
	//! UART
	GLB->PADIO22 = 2;
	GLB->PADIO23 = 2;	
	//! SF
//	GLB->PADIO32 = 0;	//!< SF WP PIN			(GPO4)
	//! SPK_EN
//	GLB->PADIO34 = 0;	//!< 					(GPO6)
	//! SD
	GLB->PADIO37 = 1;	//!< SD2 CLK
	GLB->PADIO38 = 1;	//!< SD2 CMD
	GLB->PADIO39 = 1;	//!< SD2 D0
	GLB->PADIO40 = 1;	//!< SD2 D1
	GLB->PADIO41 = 1;	//!< SD2 D2
	GLB->PADIO42 = 1;	//!< SD2 D3
	GLB->PADIO44 = 1;	//!< SD2 CD
	//! I2C-1
	GLB->PADIO45 = 1;	//!< I2C-1 SCL
	GLB->PADIO46 = 1;	//!< I2C-1 SDA
}
#endif
//------------------------------------------------------------------------------

#endif //! End #if defined BUC_CU || defined BUC_CAM

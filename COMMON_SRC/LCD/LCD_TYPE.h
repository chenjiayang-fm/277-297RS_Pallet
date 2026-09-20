/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		LCD_TYPE.H
	\brief		LCD TYPE Header
	\author		futao_jiang
	\version	2.0
	\date		2022/02/23
	\copyright	Copyright(C) 2022 SONiX Technology Co.,Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------
#ifndef _LCD_TYPE_H_
#define _LCD_TYPE_H_
#include "bsp_select.h"
//------------------------------------------------------------------------------
/**
* @brief LCD Panel Type (HW)
*/
#define	LCD_GPM1125A0_AU_UPS051	 		  (1)	//!< LCD GPM1125A0 AU UPS051 8-bit
#define LCD_GPM1125A0_RGB_DUMMY	 		  (2)	//!< LCD GPM1125A0 RGB Dummy
#define	LCD_GPM1125A0_BT601		 		  (3)	//!< LCD GPM1125A0 BT601
#define	LCD_GPM1125A0_BT656		 		  (4)	//!< LCD GPM1125A0 BT656
#define LCD_TM035KDH03_24HV		 		  (5)	//!< LCD TM035KDH03 RGB 24-bit
#define LCD_TM035KDH03_24DE      		  (6)	//!< LCD TM035KDH03 RGB 24-bit DE
#define LCD_TM035KDH03_8HV		 		  (7)	//!< LCD TM035KDH03 1 Pixel 3 Data
#define LCD_TM035KDH03_BT601	 		  (8)	//!< LCD TM035KDH03 BT601
#define LCD_TM035KDH03_BT656	 		  (9)	//!< LCD TM035KDH03 BT656
#define LCD_SN9C271A_YUV422		 		  (10)	//!< LCD SN9C271A YUV 422
#define LCD_SSD2828_					  (11)	//!< LCD SSD2828
#define LCD_SSD2828_Y50019N00N			  (12)	//!< LCD SSD2828
#define LCD_SSD2828_YN550FH002N			  (13)	//!< LCD SSD2828
#define LCD_TC358778_Y50019N00N			  (14)	//!< LCD TC358778
#define LCD_TM024HDH03_8080_8Bit2Byte 	  (19)	//!< LCD TM024HDH03 8080 Series 8-Bit 2-Byte
#define LCD_TM024HDH03_8080_8Bit3Byte	  (20)	//!< LCD TM024HDH03 8080 Series 8-Bit 3-Byte
#define LCD_TM024HDH03_8080_9Bit		  (21)	//!< LCD TM024HDH03 8080 Series 9-Bit 2-Byte
#define	LCD_TM023KDH03_8080_8Bit2Byte	  (22)	//!< LCD TM023KDH03 8080 Series 8-Bit 2-Byte
#define LCD_TM023KDH03_8080_8Bit3Byte	  (23)	//!< LCD TM023KDH03 8080 Series 8-Bit 3-Byte
#define	LCD_TM023KDH03_8080_16Bit		  (24)	//!< LCD TM023KDH03 8080 Series 16-Bit
#define LCD_HSD070IDW1_24DE				  (25)	//!< LCD_HSD070IDW1 RGB 24-bit DE
#define LCD_YN700WS008B					  (26)	//!< LCD_YN700WS008B RGB 24-bit DE
#define LCD_AHD_TP2915_RGB24              (28)  //!< LCD_AHD_TP2915 RGB 24-bit
#define LCD_AHD_NVP6021_RGB24			  (29)	//!< LCD_AHD_NVP6021 RGB 24-bit

//! Add New LCD Panel Type

#define LCD_NO_PANEL 					 (255)	//!< HW setting no LCD panel
//------------------------------------------------------------------------------
/**
* @brief LCD TV Type
*/
#define LCD_TV_NTSC_P					 (191)	//!< TV NTSC Progressive
#define LCD_TV_NTSC_I					 (192)	//!< TV NTSC Interlace	
#define LCD_TV_NTSC443_P				 (193)	//!< TV NTSC 443 Progressive	
#define LCD_TV_NTSC443_I				 (194)	//!< TV NTSC 443 Interlace	
#define LCD_TV_PAL_P					 (195)	//!< TV PAL Progressive
#define LCD_TV_PAL_I					 (196)	//!< TV PAL Interlace	
#define LCD_TV_PALM_P					 (197)	//!< TV PAL M Progressive
#define	LCD_TV_PALM_I					 (198)	//!< TV PAL M Interlace
#define LCD_NO_TV						 (255)   //!< HW no TV out
//------------------------------------------------------------------------------
/**
* @brief LCD HDMI Type (HW)
*/
#define LCD_HDMI_IT66121				 (200)	//!< HDMI TX IT66121
#define LCD_NO_HDMI						 (255)	//!< HW no HDMI out
//------------------------------------------------------------------------------
#if (defined(BSP_D_SNCC71_TP2915_AHD_RX_V3))
	#define LCD_PANEL		(LCD_AHD_TP2915_RGB24)
#elif (defined(BSP_D_SNCC71_NVP6021_AHD_RX_V4))
	#define LCD_PANEL		(LCD_AHD_NVP6021_RGB24)
#elif (defined(BSP_D_SNCC71_GM8285C_RX_V2))
	#define LCD_PANEL		(LCD_YN700WS008B)
#elif (defined(BSP_SN93711_FHD_REC_RX_V4)||defined(BSP_D_SN93701_SSD2828_RX_V5))
	#define LCD_PANEL		(LCD_SSD2828_Y50019N00N)
#elif (defined(BSP_D_SN93701_TC358778_RX_V6))
	#define LCD_PANEL		(LCD_TC358778_Y50019N00N)
#elif (defined(BSP_SN93511_DVR_V3))
	#define LCD_PANEL		(LCD_TC358778_Y50019N00N)
#elif (defined(BSP_SN9351X_RECORDING_CAM_V1))
	#define LCD_PANEL		(LCD_SSD2828_Y50019N00N)
#else
//! <<< Use Configuration Wizard in Context Menu >>>
//! <ol> LCD Pannel		<  1=> Pannel GPM1125A0 (AU UPS051)
//!    					<  2=> Pannel GPM1125A0 (RGB Dummy)
//!    					<  3=> Pannel GPM1125A0 (BT601)
//!    					<  4=> Pannel GPM1125A0 (BT656)
//!    					<  5=> Pannel TM035KDH03 (RGB 24-bit HV)
//!    					<  6=> Pannel TM035KDH03 (RGB 24-bit DE)
//!    					<  7=> Pannel LCD_TM035KDH03 (RGB 8-bit HV)
//!    					<  8=> Pannel TM035KDH03 (BT601)
//!    					<  9=> Pannel TM035KDH03 (BT656)
//!    					< 10=> Image Bridge SN9C271A / UVC Device (YUV422)
//!    					< 11=> Image Bridge SSD2828_ (RGB 24-bit DE)
//!    					< 12=> Image Bridge SSD2828 / Pannel Y50019N00N (RGB 24-bit DE)
//!    					< 13=> Image Bridge SSD2828 / PannelYN550FH002N (RGB 24-bit DE)
//!    					< 14=> Image Bridge TC358778 / Pannel Y50019N00N (RGB 24-bit DE)
//!    					< 19=> Pannel TM024HDH03 (8080 8-bit 2-byte)
//!    					< 20=> Pannel TM024HDH03 (8080_8-bit 3-byte)
//!    					< 21=> Pannel TM024HDH03 (8080 9-bit)
//!    					< 22=> Pannel TM023KDH03 (8080 8-bit 2-byte)
//!    					< 23=> Pannel TM023KDH03 (8080 8-bit 3-byte)
//!    					< 24=> Pannel TM023KDH03 (8080 16-bit)
//!    					< 25=> Pannel HSD070IDW1 (RGB 24-bit DE)
//!    					< 26=> Pannel YN700WS008B (RGB 24-bit DE)
//!    					< 28=> Image Bridge TP2915 for AHD (RGB 24-bit DE)
//! 					< 29=> Image Bridge NVP6021 for AHD (RGB 24-bit DE)
//!    					<255=> No Pannel
	#define LCD_PANEL		11
#endif                

//! <ol> TV out Format	<191=> NTSC Progressive
//!    					<192=> NTSC Interlace	
//!    					<193=> NTSC 443 Progressive	
//!    					<194=> NTSC 443 Interlace	
//!    					<195=> PAL Progressive
//!    					<196=> PAL Interlace	
//!    					<197=> PAL M Progressive
//!    					<198=> PAL M Interlace
//!    					<255=> No TV out		
#define LCD_TV_OUT		191
#define LCD_HDMI_OUT	(LCD_NO_HDMI)


//! <e> Enable TP2915 Debug Options
#define TP2915_DEBUG_EN							0
//! <ol> Fix TP2915 Output Format to			<  0=> AHD_TP2915_AHD1080P30
//!												<  1=> AHD_TP2915_AHD1080P25
//!    											<  2=> AHD_TP2915_AHD720P30
//!    											<  3=> AHD_TP2915_AHD720P25
//!    											<  4=> AHD_TP2915_TVI1080P30
//!    											<  5=> AHD_TP2915_TVI1080P25
//!    											<  6=> AHD_TP2915_TVI720P60
//!    											<  7=> AHD_TP2915_TVI720P30
//!    											<  8=> AHD_TP2915_TVI720P25
//!    											<  9=> AHD_TP2915_CVBS_NTSC
//!    											< 10=> AHD_TP2915_CVBS_PAL
#define TP2915_FORCE_OUTPUT_FORMAT				0
//! <q> Enable TP2915 Test Pattern
//! <i> Ignore all input data and set TP2915 internal test pattern enable
#define TP2915_TEST_PATTERN_EN					0
//! </e>

//! <e> Enable NVP6021 Debug Options
#define NVP6021_DEBUG_EN						0
//! <ol> Fix NVP6021 Output Format to			<  0=> AHD_720P25
//!												<  1=> AHD_720P30
//!												<  2=> AHD_720P50
//!												<  3=> AHD_720P60
//!												<  4=> AHD_1080P25
//!												<  5=> AHD_1080P30
#define NVP6021_FORCE_OUTPUT_FORMAT				1
//! <q> Enable NVP6021 Test Pattern
//! <i> Ignore all input data and set NVP6021 internal test pattern enable
#define NVP6021_TEST_PATTERN_EN					0
//! </e>
//! <ol> NVP6021 Pre-Converter type				<  0=>LT9211
//! 											<  1=>FPGA
//! 											<  2=>CPLD
#define NVP6021_PRE_CONVERTER_TYPE				0

//! <<< end of configuration section >>>
//------------------------------------------------------------------------------


#endif //! #ifndef _LCD_TYPE_H_

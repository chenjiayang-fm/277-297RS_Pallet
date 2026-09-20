/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file	    SEN_TYPE.h
	\brief		Sensor funcations header
	\author     Pierce
	\version    0.3
	\date		2020/04/27
	\copyright	Copyright(C) 2022 SONiX Technology Co.,Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------
#ifndef _SEN_TYPE_H_
#define _SEN_TYPE_H_
#include "bsp_select.h"
//------------------------------------------------------------------------------
//! DEFINITION
//------------------------------------------------------------------------------
//! Sensor Format
	#define SEN_OV9732				2
	#define SEN_OV9750  		    3
	#define SEN_AR0330				4
	#define SEN_H62				    5
	#define SEN_H65		            6
    #define SEN_IMX307  		    7
	#define SEN_IMX323  		    8
	#define SEN_SC2235  		    9
    #define SEN_SC2238H  		    10
    #define SEN_SC2239  		    11
    #define SEN_RN6752  		    12
	#define SEN_TP9950				13
	#define SEN_GC2053				14
	#define SEN_H66				    15
	#define SEN_H63				    16
	#define SEN_GL3004              17
	#define SEN_SC2332              18
	#define SEN_IMX462				19	
	#define SEN_H68				    20
	#define SEN_GC4653				21
	#define SEN_PS5268				22
  #define SEN_SC230AI				23
	#define SEN_GC1084				24
	#define SEN_GC2083				25	
	#if (defined(BSP_SNCC70_AHD_TP9950_TX_V1) || defined(BSP_SNCC72_AHD_TP9950_TX_V2))
		#define SEN_USE				    SEN_TP9950
	#elif (defined(BSP_SNCC70_AHD_RN6752M_TX_V1) || defined(BSP_SNCC72_AHD_RN6752M_TX_V1))
		#define SEN_USE				    SEN_RN6752
    #elif defined(OP_STA)
//! <<< Use Configuration Wizard in Context Menu >>>	
//! <ol> Tx Sensor      < 2=> Sensor OV9732
//!    					< 3=> Sensor OV9750
//!    					< 4=> Sensor AR0330
//!    					< 5=> Sensor H62
//!    					< 6=> Sensor H65
//!    					< 7=> Sensor IMX307
//!    					< 8=> Sensor IMX323
//!    					< 9=> Sensor SC2235
//!    					<10=> Sensor SC2238H
//!    					<11=> Sensor SC2239
//!    					<12=> Bridge RN6752
//!    					<13=> Bridge TP9950
//!    					<14=> Sensor GC2053
//!    					<15=> Sensor H66
//!    					<16=> Sensor H63
//!    					<17=> Bridge GL3004
//!    					<18=> Sensor SC2332
//!    					<19=> Sensor IMX462
//!    					<20=> Sensor H68
//!    					<21=> Sensor GC4653
//!    					<22=> Sensor PS5268
//!    					<23=> Sensor SC230AI
//!    					<24=> Sensor GC1084
//!    					<25=> Sensor GC2083

        #define SEN_USE		6
//!
	#elif defined(SN93511_DVR_V3) || defined(SN9351X_RECORDING_CAM_V1)
//! <ol> Rx Local Sensor< 2=> Sensor OV9732
//!    					< 3=> Sensor OV9750
//!    					< 4=> Sensor AR0330
//!    					< 5=> Sensor H62
//!    					< 6=> Sensor H65
//!    					< 7=> Sensor IMX307
//!    					< 8=> Sensor IMX323
//!    					< 9=> Sensor SC2235
//!    					<10=> Sensor SC2238H
//!    					<11=> Sensor SC2239
//!    					<12=> Bridge RN6752
//!    					<13=> Bridge TP9950
//!    					<14=> Sensor GC2053
//!    					<15=> Sensor H66
//!    					<16=> Sensor H63
//!    					<17=> Bridge GL3004
//!    					<18=> Sensor SC2332
//!    					<19=> Sensor IMX462
//!    					<20=> Sensor H68
//!    					<21=> Sensor GC4653
//!    					<22=> Sensor PS5268
//!    					<23=> Sensor SC230AI
//!    					<24=> Sensor GC1084
//!    					<25=> Sensor GC2083
		#define SEN_USE		25
//! <<< end of configuration section >>>		
    #endif
#endif

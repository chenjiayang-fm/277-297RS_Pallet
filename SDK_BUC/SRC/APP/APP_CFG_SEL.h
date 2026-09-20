/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		APP_CFG_SEL.h
	\brief		APP Configuration header file
	\author		Pierce
	\version	0.10
	\date		2022/03/01
	\copyright	Copyright(C) 2022 SONiX Technology Co.,Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------
#ifndef _APP_CFG_SEL_H_
#define _APP_CFG_SEL_H_
#include "bsp_select.h"

//------------------------------------------------------------------------------
//! <<< Use Configuration Wizard in Context Menu >>>
#define APP_FS_MEDIA0			0
#define APP_FS_MEDIA1			1
#define APP_FS_MEDIA_TYPE_SD		0
#define APP_FS_MEDIA_TYPE_SF		1

//------------------------------------------------------------------------------
#if defined(BUC_CU)
//! <h> BUC CU

//! <e> SD Card
#define APP_SD_FUNC_ENABLE				1
#if APP_SD_FUNC_ENABLE
//! <q> Recording Video		
#define APP_REC_FUNC_ENABLE				1
#if APP_REC_FUNC_ENABLE
#define APP_FS_MEDIA0_TYPE				APP_FS_MEDIA_TYPE_SD
#endif
#else
#define APP_REC_FUNC_ENABLE				0
#endif
//! </e>

//! <e> Capturing Photo
#define APP_PHOTOGRAPH_FUNC_ENABLE		0
//! <ol> Stored in				<0=> SD Card
//!    							<1=> Serial Flash
#define APP_PHOTO_STORE_SEL				0
//! </e>

#if APP_REC_FUNC_ENABLE 
	#if (APP_PHOTOGRAPH_FUNC_ENABLE && APP_PHOTO_STORE_SEL == APP_FS_MEDIA_TYPE_SF)	
		#define APP_DUAL_FS_ENABLE				1
		#define APP_FS_MEDIA1_TYPE				APP_FS_MEDIA_TYPE_SF
	#else
		#define APP_DUAL_FS_ENABLE				0		
	#endif
#elif APP_SD_FUNC_ENABLE
	#if (APP_PHOTOGRAPH_FUNC_ENABLE && APP_PHOTO_STORE_SEL == APP_FS_MEDIA_TYPE_SF)
		#define APP_DUAL_FS_ENABLE				1		
		#define APP_FS_MEDIA0_TYPE				APP_FS_MEDIA_TYPE_SF
		#define APP_FS_MEDIA1_TYPE				APP_FS_MEDIA_TYPE_SD	
	#else
		#define APP_DUAL_FS_ENABLE				0		
		#define APP_FS_MEDIA0_TYPE				APP_PHOTO_STORE_SEL
	#endif
#else
	#define APP_DUAL_FS_ENABLE				0
	#define APP_FS_MEDIA0_TYPE				APP_FS_MEDIA_TYPE_SF
#endif	
//! </h>
#else
#include "SEN_TYPE.h"
//! <h> BUC CAM
#define APP_SD_FUNC_ENABLE				0
#define APP_REC_FUNC_ENABLE				0
//! 0 is Main Stream
//! 1 is Sub Stream
#define APP_TXREC_STREAM_SEL		    1
#define APP_PHOTOGRAPH_FUNC_ENABLE		0
#define APP_FS_MEDIA0_TYPE				APP_FS_MEDIA_TYPE_SD
#define APP_DUAL_FS_ENABLE				0
#define APP_FS_MEDIA1_TYPE				APP_FS_MEDIA_TYPE_SF

//! <e> Auto Pairing	
#define APP_AUTO_PAIR					0
//! <ol> Pairing Mode			<0=> Once
//!    							<1=> Continuous
#define APP_AUTO_PAIR_MODE				1
//! </e>

//! <q> Horizontal Mirror (Sensor)
#define APP_SEN_HORIZONTAL_MIRROR_ENABLE	0
//! <i> Horizontal-Mirror Image (Image Only/without DS)

//! <q> UVC Camera		
#define APP_UVC_CAM_ENABLE              0
#define APP_TRX_FUNC_ENABLE				((APP_UVC_CAM_ENABLE)?0:1)
//! <i> Only UVC camera and no RF transmit / receive
//! </h>
#endif

#if (	defined(BSP_D_SNCC71_TP2915_AHD_RX_V3) ||\
		defined(BSP_D_SNCC71_NVP6021_AHD_RX_V4) ||\
		(defined(BUC_CAM) && !(defined(D_SN93712_VBM_TX_V2)))	)
#define APP_DUAL_HOST_ENABLE            0
#else
//! <q> USB Dual Host 		
#define APP_DUAL_HOST_ENABLE            0
//! <i> The dual UVC host no supports SNCC71_TP2915_AHD_RX
//! <i> If dual UVC host enable, the display mode fixes the 4T1R
//! <i> The dual UVC host of BUC CAM only supports D_SN93712_VBM_TX_V2
#endif

//! APP Display mode
#define	DISPLAY_1T1R  					1
#define	DISPLAY_2T1R  					2
#define	DISPLAY_4T1R  					4
#if (APP_DUAL_HOST_ENABLE && defined(BUC_CU))
#define DISPLAY_MODE					DISPLAY_4T1R 
#elif (APP_DUAL_HOST_ENABLE || APP_UVC_CAM_ENABLE)
#define DISPLAY_MODE					DISPLAY_1T1R 
#else
//! Display Mode
//! <ol> Display Mode			<1=> Display 1T1R
//!    							<2=> Display 2T1R
//!    							<4=> Display 4T1R
//! <i> AHD out only supports Display 1T1R, Unless AHD output fps >= 50 fps!
#define DISPLAY_MODE					4 









#endif

//! APP Audio Function Setting
//! <e> Audio
#define APP_ADO_FUNC_ENABLE				1
//! <q> Recoding Mixer Audio
#define APP_REC_FUNC_ADO_MIX_ENABLE		0
//! </e>

#ifdef A7130
//! Baseband timing setting

//! <h> RF
//! <ol> A7130 RF Timing 		<0=> Video First
//!    							<1=> Audio First
//!    							<2=> Video Only
//! <i> When audio disable the A7130 RF timing fixes Video Only
	#define APP_BB_TIMING_SET			0
	
	
//! Hopping channel number
//! RF Channel Number
//! <ol> Channel Number			<1=> 19 channels
//!    							<0=> 20 channels

#define APP_RF_HOPPING_CHANNEL_NUMBER    1



//! <q> BB Special Timing
#define APP_BB_SPECIAL_TIMING_ENABLE    0
#endif

//! Modulation Selection (with Richwave RF)
#ifdef RTC676x
#define MODULATION_FIXED_QPSK			0
#define MODULATION_FIXED_16QAM			1
#define MODULATION_AUTO					2
//! <ol> RTC676x RF Modulation	<0=> Modulation Fixed QPSK
//!    							<1=> Modulation Fixed 16QAM
//!    							<2=> Modulation Auto
#define APP_MODULATION_MODE				0
//! </h>
#if (APP_MODULATION_MODE == MODULATION_FIXED_QPSK)
	#define APP_EVEN_SLOT				1200
#else
	#define APP_EVEN_SLOT				1667
#endif
#endif

//! <h> File System
//! <ol> File System Path		<0=> \DCIM\100SONIX ~ \DCIM\999SONIX
//!    							<1=> \DCIM\VIDEO & \DCIM\PHOTO
#define APP_FS_CUSTOMER1_PATH_ENABLE	0	//customer recording folder at E:\DCIM\VIDEO and E:\DCIM\PHOTO

//! <ol> File Name				<0=> VDO_20200413_183030.MP4 & JPG_20200413_183030.JPG
//!    							<1=> SNX_0001 ~ SNX_9999
#define APP_FS_ROLLING_FILE_NAME		1
//! </h>

#ifdef OP_AP
//! <h> File list style
//! <ol> Type					<0=> original(words only)
//!								<1=> thumbnail
#if APP_REC_FUNC_ENABLE
#define APP_FS_FILE_LIST_STYLE        0
#else
#define APP_FS_FILE_LIST_STYLE        0
#endif
//! </h>
#endif
#ifdef OP_STA
#define APP_FS_FILE_LIST_STYLE        0
#endif

#if (BSP_RTC_TIMER_SEL == RTC_TIMER_NULL)
#define APP_TIMESTAMP_FUNC_ENABLE		0
#else
//! <q> Time Stamp				
#define APP_TIMESTAMP_FUNC_ENABLE		1
//! <i> When RTC timer is NULL the Time Stamp is disable
#endif
//! <q> Ethernet MAC
#define APP_MAC_FUNC_ENABLE				0

#if (defined(BSP_D_SN93714_TX_V1) || defined(BSP_D_SN93716_TX_V1))
#define APP_DOORPHONE_ENABLE			0
#define APP_DP_TXPS_MODE				0
#else
//! <e> Door Phone
#define APP_DOORPHONE_ENABLE			0
//! <ol> TX PowerSaving Mode       <0=> Off
//!    							   <1=> On
#define APP_DP_TXPS_MODE				1
//! <i> BSP SNCC72\SN93712 no support power saveing mode
//! </e>
#endif

//! <h> Command Line
//! <q> CLI Login function
#define APP_CFG_ENABLE_LOGIN			1
//! <i> Enable command line login function or not
//! </h>

//! <q> PC Connect
#define APP_PC_CONNECT_EN				0

//! <ol> USBD Composite Mode	<0x00=> NO USBD Composite
//!								<0x06=> USBD Composite UVC HID
//!    							<0x03=> USBD Composite UVC MSC
//!    							<0x12=> USBD Composite UVC MSCSPEC
//!    							<0x0A=> USBD Composite UVC UAC
#define APP_USBD_COMPOSITE_MODE					10 

//! <ol> USBD UVC Video Mode	<0x00=> USBD Normal
//!								<0x01=> USBD Sonix YUV 420 only
#if defined(BUC_CU)
#define APP_USBD_UVCVDO_MODE					0x02 //若要删除USB功能，改为0依然有部分地方需要修改，对照原版sdk修改
#else
#define APP_USBD_UVCVDO_MODE					0x00
#endif 

#if ( DISPLAY_MODE==2 && APP_USBD_UVCVDO_MODE == 1)
#define APP_UVC_SNXYUV_ONLY		2
#elif ( DISPLAY_MODE==4 && APP_USBD_UVCVDO_MODE == 1)
#define APP_UVC_SNXYUV_ONLY		4
#else
#define APP_UVC_SNXYUV_ONLY		0	
#endif

//! <i> UVC Video color space mode
//! <<< end of configuration section >>>

#if (defined(BUC_CAM) && BSP_DDRSIZE == 16 && (SEN_USE == SEN_RN6752 || SEN_USE == SEN_TP9950))
#define MIN_DRAM_ENABLE                 1
#else
#define MIN_DRAM_ENABLE                 APP_DUAL_HOST_ENABLE
#endif
#if (!APP_PHOTOGRAPH_FUNC_ENABLE && !APP_REC_FUNC_ENABLE)
#define APP_RAISE_OSHEAP_SIZE			(0)//(982*1024) // 000
#elif (!APP_DUAL_FS_ENABLE && APP_REC_FUNC_ENABLE)
#define APP_RAISE_OSHEAP_SIZE			(855*1024) // 010,110
#elif (!APP_DUAL_FS_ENABLE)
#define APP_RAISE_OSHEAP_SIZE			(411*1024) // 100
#elif (!APP_REC_FUNC_ENABLE)
#define APP_RAISE_OSHEAP_SIZE			(818*1024) // 101
#else
#define APP_RAISE_OSHEAP_SIZE			(982*1024) // 111
#endif
//------------------------------------------------------------------------------
//! 510PF_BUC_CU_A7130   (Disable Dual UVC Host)
#if (defined(BUC_CU) && defined(A7130) && (!MIN_DRAM_ENABLE))
#define osHeapSize               (1536*1024) + APP_RAISE_OSHEAP_SIZE	//1728	1536		
#define CODE_MAX_SIZE			 (0x00160000)				//!< I-Cache Max Size
#define DATA_MAX_SIZE			 (0x00150000)				//!< D-Cache Max Size = TLB Size + 
															//!<                    DATA_MAX_SIZE + 
															//!< 					osHeap

//------------------------------------------------------------------------------
//! 510PF_BUC_CU_RTC676x (Disable Dual UVC Host)
#elif (defined(BUC_CU) && defined(RTC676x) && (!MIN_DRAM_ENABLE))
#define osHeapSize               (2048*1024) + APP_RAISE_OSHEAP_SIZE
#define CODE_MAX_SIZE			 (0x00170000)				//!< I-Cache Max Size
#define DATA_MAX_SIZE			 (0x00150000)				//!< D-Cache Max Size = TLB Size +  
															//!<                    DATA_MAX_SIZE + 
															//!< 					osHeap

//------------------------------------------------------------------------------
//! 510PF_BUC_CU_S2019X
#elif (defined(BUC_CU) && defined(S2019A))
#define osHeapSize               (1536*1024) + APP_RAISE_OSHEAP_SIZE	//1728			1536		//2048
	#if (APP_USBD_COMPOSITE_MODE == 10)	// UVC UAC
#define CODE_MAX_SIZE			 (0x00200000)				//!< I-Cache Max Size
	#else
#define CODE_MAX_SIZE			 (0x00180000)				//!< I-Cache Max Size	//0x00180000	//0x00200000
	#endif
#define DATA_MAX_SIZE			 (0x00150000)				//!< D-Cache Max Size = TLB Size + 
															//!<                    DATA_MAX_SIZE + 
															//!< 					osHeap

//------------------------------------------------------------------------------
//! 510PF_BUC_CU_A7130   (Enable Dual UVC Host, HD LCD Pannel)
#elif (defined(BUC_CU) && defined(A7130) && MIN_DRAM_ENABLE)
#define osHeapSize               (1536*1024)				//!< For BUC_CU
#define CODE_MAX_SIZE			 (0x00160000)				//!< I-Cache Max Size	
//#define DATA_MAX_SIZE			 (0x0014D000) 				//!< D-Cache Max Size = TLB Size +
#define DATA_MAX_SIZE			 (0x0016D000) 				//!< D-Cache Max Size = TLB Size +
															//!<                    DATA_MAX_SIZE + 
															//!< 					osHeap

//------------------------------------------------------------------------------
//! 510PF_BUC_CU_RTC676x (Enable Dual UVC Host, WVGA LCD Pannel)
#elif (defined(BUC_CU) && defined(RTC676x) && MIN_DRAM_ENABLE)
#define osHeapSize               (1536*1024)				//!< For BUC_CU
#define CODE_MAX_SIZE			 (0x00160000)				//!< I-Cache Max Size	
#define DATA_MAX_SIZE			 (0x0014D000) 				//!< D-Cache Max Size = TLB Size + 
															//!<                    DATA_MAX_SIZE + 
															//!< 					osHeap

//------------------------------------------------------------------------------
//! 510PF_BUC_CAM_A7130   (SNCC70 or SN93700, SNCC72 or SN93712 + Sensor in)
#elif (defined(BUC_CAM) && defined(A7130) && !MIN_DRAM_ENABLE)
#define osHeapSize				 (1536*1024)
#define CODE_MAX_SIZE			 (0x00160000)				//!< I-Cache Max Size	

#define DATA_MAX_SIZE			 (0x00100000)				//!< D-Cache Max Size = TLB Size + 
//#define DATA_MAX_SIZE			 (0x00170000)				//!< D-Cache Max Size = TLB Size +

															//!<                    DATA_MAX_SIZE + 
															//!< 					osHeap

//------------------------------------------------------------------------------
//! 510PF_BUC_CAM_RTC676x (SNCC70 or SN93700, SNCC72 or SN93712 + Sensor in)
#elif (defined(BUC_CAM) && defined(RTC676x) && !MIN_DRAM_ENABLE)
#define osHeapSize				 (1536*1024)
#define CODE_MAX_SIZE			 (0x00150000)				//!< I-Cache Max Size	
#define DATA_MAX_SIZE			 (0x00100000)				//!< D-Cache Max Size = TLB Size +
															//!<                    DATA_MAX_SIZE + 
															//!< 					osHeap
															
//------------------------------------------------------------------------------
//! 510PF_BUC_CAM_S2019X 
#elif (defined(BUC_CAM) && defined(S2019A))
#define osHeapSize				 (1536*1024)
	#if (APP_USBD_COMPOSITE_MODE == 10)	// UVC UAC
#define CODE_MAX_SIZE			 (0x00200000)				//!< I-Cache Max Size
	#else
#define CODE_MAX_SIZE			 (0x00160000)				//!< I-Cache Max Size	//0x00160000	//0x00200000
	#endif
#define DATA_MAX_SIZE			 (0x00110000)				//!< D-Cache Max Size = TLB Size + 
															//!<                    DATA_MAX_SIZE + 
															//!< 					osHeap

//------------------------------------------------------------------------------
//! 510PF_BUC_CAM_A7130   (SNCC72 or SN93712 + AHD in)
#elif (defined(BUC_CAM) && defined(A7130) && MIN_DRAM_ENABLE)
#define osHeapSize				 (1536*1024)
#define CODE_MAX_SIZE			 (0x00140000)				//!< I-Cache Max Size
#define DATA_MAX_SIZE			 (0x00100000)				//!< D-Cache Max Size = TLB Size + 
															//!<                    DATA_MAX_SIZE + 
															//!< 					osHeap
															
//------------------------------------------------------------------------------
//! 510PF_BUC_CAM_RTC676x (SNCC72 or SN93712 + AHD in)
#elif (defined(BUC_CAM) && defined(RTC676x) && MIN_DRAM_ENABLE)
#define osHeapSize				 (1536*1024)
#define CODE_MAX_SIZE			 (0x00150000)				//!< I-Cache Max Size
#define DATA_MAX_SIZE			 (0x00100000)				//!< D-Cache Max Size = TLB Size + 
															//!<                    DATA_MAX_SIZE + 
															//!< 					osHeap
#endif
#endif

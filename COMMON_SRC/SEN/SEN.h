/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file	    SEN.h
	\brief		Sensor funcations header
	\author     BoCun
	\version    4.32
	\date		2020-12-22
	\copyright	Copyright(C) 2020 SONiX Technology Co.,Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------
#ifndef _SENSOR_H_
#define _SENSOR_H_
#include <stdbool.h>
#include <stdint.h>
#include "bsp_select.h"
//------------------------------------------------------------------------------
/*!	\file SEN.h
SENSOR FlowChart:
	\dot
	digraph sensor_flow {
		node [shape=record, fontname=Verdana, fontsize=10, fixedsize=true, width=4];
		"Set sensor default value. \n SEN_SetSensorInitVal()"->
		"Setup sensor relation IRQ. \n SEN_ISRInitial()"->
		"Set sensor output size and ISP process size. \n SEN_SetWindowSize()"->
		"Set ISP output size. \n SEN_SetResolution()"->
		"Set and start sensor. \n ubSEN_Open(&sensor_cfg)";
		"Set video steam. \n SEN_SetPathState(SENSOR_PATH1, 1)"->
		"Enable video stram. \n SEN_SetIspOutEn(TRUE)"->
		"Load IQ.bin to dram. \n IQ_ParserIQTable()"->
		"Strat ISP.(like as AE, AWB...etc) \n SEN_ISPInitial()"->
		"Output Image Source \n wait VSYNC, HSYNC, HW_END interrupt";	
		
		"Set and start sensor. \n ubSEN_Open(&sensor_cfg)"-> "Set video steam. \n SEN_SetPathState(SENSOR_PATH1, 1)"[label = "Pass"];		
		"Set and start sensor. \n ubSEN_Open(&sensor_cfg)"-> "Restart sensor initial" [label = "Fail"];	
	}
	\enddot
*/
//------------------------------------------------------------------------------
//! DEFINITION
//------------------------------------------------------------------------------
/*!
\brief Sensor master clock Type 
*/
enum _SENSOR_CLK_GEN
{
	SENSOR_96MHz = 0,           //!< 96MHz Master clock
	SENSOR_120MHz,              //!< 120MHz Master clock
};

/*!
\brief Sensor frame rate selection
*/
enum
{
	SEN_FPS05 = 5,              //!< frame rate = 05
	SEN_FPS10 = 10,             //!< frame rate = 10
    SEN_FPS15 = 15,             //!< frame rate = 15
    SEN_FPS20 = 20,             //!< frame rate = 20
    SEN_FPS25 = 25,             //!< frame rate = 25
    SEN_FPS30 = 30,             //!< frame rate = 30
};

/*!
\brief Sensor power frequency selection
*/
enum _SENSOR_POWER_FREQ
{
    SENSOR_PWR_FREQ_AUTO = 0,   //!< Auto power frequency
	SENSOR_PWR_FREQ_50HZ,	    //!< 50Hz power frequency
	SENSOR_PWR_FREQ_60HZ,       //!< 60Hz power frequency
};

/*!
\brief Sensor relation path selection
*/
enum _SENSOR_PATH
{
	SENSOR_PATH1 = 1,           //!< sensor output1 path
	SENSOR_PATH2,               //!< sensor output2 path 
	SENSOR_PATH3,               //!< sensor output3 path 
    ISP_3DNR,                   //!< ISP 3DNR path
    ISP_MD_W0,                  //!< ISP MD W0 path
    ISP_MD_W1,                  //!< ISP MD W1 path
    ISP_MD_W2,                  //!< ISP MD W2 path
    IQ_BIN_FILE,                //!< IQ bin file path
    IMG_TEMP1,                  //!< Image temp stroage path
};

/*!
\brief Sensor data stamp font index
*/
enum
{
    FONT_COLON = 11,            //!< font_"." index
    FONT_SPACE = 12,            //!< font_" " index
	FONT_B = 13,	            //!< font_"B" index
	FONT_K = 14,                //!< font_"K" index
    FONT_L = 15,                //!< font_"L" index
    FONT_W = 16,                //!< font_"W" index
    FONT_i = 17,                //!< font_"i" index
    FONT_n = 18,                //!< font_"n" index
    FONT_k = 19,                //!< font_"k" index
};

/*!
\brief Sensor IQ bin file status
*/
enum
{
    SEN_BYPASS_IQ = 0,          //!< bypass IQ bin file    
    SEN_LOAD_IQ = 1,            //!< load IQ bin file
};

/*!
\brief Sensor ISP scaler status 
*/
typedef enum
{
	SEN_NORMAL_MODE = 0,	    //!< Normal mode
	SEN_SCALEUP_MODE ,          //!< Scale-up mode
}SCALE_TYPE;

/*!
\brief Sensor MIPI status 
*/
enum
{
    MIPI_DISABLE = 0,           //!< MIPI disable index
    MIPI_ENABLE = 1,            //!< MIPI enable index
};

/*!
\brief Sensor MIPI lane number selection 
*/
typedef enum
{
    MIPI_1LANE = 0,             //!< MIPI one lane index
    MIPI_2LANE = 1,             //!< MIPI two lane index
}MIPI_LANE_NUM_TYPE;

/*!
\brief Sensor MIPI channel selection 
*/
typedef enum
{
    MIPI_CHANNEL0 = 0,          //!< MIPI channel_0 index
    MIPI_CHANNEL1 = 1,          //!< MIPI channel_1 index
    MIPI_CHANNEL2 = 2,          //!< MIPI channel_2 index
    MIPI_CHANNEL3 = 3,          //!< MIPI channel_3 index
}MIPI_CHANNEL_NUM_TYPE;

/*!
\brief Sensor initial status 
*/
enum
{
    SEN_InitFail    = 0,        //!< Sensor initial fail
    SEN_InitOk      = 1,        //!< Sensor initial ok
};

/*!
\brief Detect input video status
*/
enum
{
    SEN_ValidVideo      = 0,        //!< valid video
    SEN_NoVideo         = 1,        //!< no video 
};

/*!
\brief Sensor ISP test pattern selection 
*/
enum
{
	SEN_NORMAL = 0,             //!< Normal image output
	SEN_RAW_PATTERN,            //!< RAW test pattern output
    SEN_YUV_PATTERN,            //!< YUV test pattern output
};

/*!
\brief Sensor ISP path selection 
*/
typedef enum
{	
    BAYER_SENSOR_NORMAL,        //!< Raw data input
    BAYER_SENSOR_RAW_DATA,      //!< Raw data input and output
    YUV_SENSOR_RGB_IN,          //!< YUV data input(RGB path)
    YUV_SENSOR_YUV_IN,          //!< YUV data input(YUV path)
}ISP_PIPE_TYPE;

/*!
\brief Sensor cam input format index
*/
typedef enum
{	
    SENSOR_AHD_720P_PAL     = 0,            //!< AHD cam 720P@25fps
    SENSOR_AHD_720P_NTSC,                   //!< AHD cam 720P@30fps
    SENSOR_TVI_720P_PAL,                    //!< TVI cam 720P@25fps
    SENSOR_TVI_720P_NTSC,                   //!< TVI cam 720P@30fps
    SENSOR_CVI_720P_PAL,                    //!< CVI cam 720P@25fps
    SENSOR_CVI_720P_NTSC,                   //!< CVI cam 720P@30fps
    SENSOR_AHD_1080P_PAL,                   //!< AHD cam 1080P@25fps
    SENSOR_AHD_1080P_NTSC,                  //!< AHD cam 1080P@30fps
    SENSOR_TVI_1080P_PAL,                   //!< CVI cam 1080P@25fps
    SENSOR_TVI_1080P_NTSC,                  //!< CVI cam 1080P@30fps
    SENSOR_CVI_1080P_PAL    = 10,           //!< CVI cam 1080P@25fps
    SENSOR_CVI_1080P_NTSC,                  //!< CVI cam 1080P@30fps
    SENSOR_CVBS_PAL,                        //!< CVBS cam 572i@25fps
    SENSOR_CVBS_NTSC,                       //!< CVBS cam 480i@30fps
	SENSOR_NONO = 0xFF,                     
}SEN_INPUT_TYPE;

/*!
\brief I2C select
*/
#define I2C_TYPE_POLLING	0
#define I2C_TPYE_INTERRUPT	1

//------------------------------------------------------------------------------
//! STRUCT
//------------------------------------------------------------------------------
/*!
\brief Sensor input window structure 
*/
typedef struct {
	uint16_t uwHStart;						//!< Start pixel number for capturing after HSYNC.
	uint16_t uwVStart;						//!< Start line number for capturing after VSYNC.
	uint16_t uwHSize;						//!< Image input horizontal size
	uint16_t uwVSize;						//!< Image input vertical Size
} SEN_WINDOW;

/*!
\brief ISP output window structure 
*/
typedef struct {
	uint32_t ulFrameSize1;				    //!< Path1 frame size
	uint32_t ulFrameSize2;				    //!< Path2 frame size
	uint32_t ulFrameSize3;				    //!< Path3 frame size
	uint16_t uwHSize1;						//!< Path1 horizontal Size
	uint16_t uwVSize1;						//!< Path1 vertical Size
	uint16_t uwHSize2;						//!< Path2 horizontal Size
	uint16_t uwVSize2;						//!< Path2 vertical Size
	uint16_t uwHSize3;						//!< Path3 horizontal Size
	uint16_t uwVSize3;						//!< Path3 vertical Size
} SEN_PATH_INF;

/*!
\brief Sensor relation path structure 
*/
typedef struct {
	uint32_t ulPath1_Addr;				    //!< Video stream1 start address
	uint32_t ulPath2_Addr;				    //!< Video stream2 start address
	uint32_t ulPath3_Addr;				    //!< Video stream3 start address
	uint32_t ul3DNR_STA;				    //!< 3DNR ring buffer start address
	uint32_t ul3DNR_EDA;				    //!< 3DNR ring buffer end address
	uint32_t ulMD_M0_Addr;				    //!< MDv2 M0 address
	uint32_t ulMD_M1_Addr;				    //!< MDv2 M1 address
	uint32_t ulMD_M2_Addr;				    //!< MDv2 M2 address	
	uint32_t ulIQBin_Addr;				    //!< IQ bin file address
	uint32_t ulIMG_TEMP1_Addr;				//!< Image temp stroage address				    
} SEN_ADDRESS;

/*!
\brief Sensor relation setting structure 
*/
struct SENSOR_SETTING {
    ISP_PIPE_TYPE   tPathType;              //!< ISP pipe line selection
    SCALE_TYPE      tScaleType;             //!< ISP scale mode    
    
    SEN_PATH_INF xtSENPath;                 //!< ISP output path information
    SEN_WINDOW  xtSENWin;                   //!< sensor in window size information
    SEN_ADDRESS xtSENAddr;                  //!< sensor relation address information
    uint64_t ullDropTable[3];                //!< Frame drop table    
	uint32_t ulSensorMclk;                  //!< Sensor master clock
	uint32_t ulMaximumSensorFrameRate;      //!< Sensor maximun frame rate
    uint8_t ubFrameRate[3];                 //!< Frame rate control
	uint8_t ubSensorType;				    //!< Sensor type
    uint8_t ubLoadIQState;                  //!< Load IQ bin file state 
    uint8_t ubImageMode;                    //!< ISP image output mode 
	uint8_t	ubPath1Src;					    //!< Path1 source type
	uint8_t	ubPath2Src;					    //!< Path2 source type
	uint8_t	ubPath3Src;					    //!< Path3 source type
};

//#pragma pack(push) /* push current alignment to stack */
//#pragma pack(1) /* set alignment to 1 byte boundary */
/*!
\brief Sensor control information structure 
*/
typedef struct tagSENCtl {
	uint16_t uwExpLine;						//!< Exposure line
	uint16_t uwDmyLine;						//!< Dummy line
	uint32_t ulExpTime;						//!< Exposure time
	uint16_t uwGain;						//!< Gain control
    uint16_t uwTotalGain;					//!< Total gain control
    uint16_t uwBlueGain;					//!< Blue channel offset for AWB
    uint16_t uwRedGain;						//!< Red channel offset for AWB
    uint16_t uwGreenGain;					//!< Green channel offset for AWB
} tfSENCtl;

/*!
\brief Sensor object information structure 
*/
typedef struct tagSENObj {
	uint8_t 	ubState;					//!< Current state
	uint8_t 	ubPrevState;				//!< Previous state
	uint8_t 	ubLastInitIdx;              //!< Last initial index
	uint8_t 	ubLastFRIdx;                //!< Lase FR index
    tfSENCtl 	xtSENCtl;                   //!< Sensor control information
	uint8_t 	ubBuf[4];                   //!< Data temp buffer
	uint8_t 	ubImgMode;					//!< Image mirror/ flip control
	uint16_t 	uwMaxExpLine;				//!< Maximum exposure line
} tfSENObj;
//#pragma pack(pop)

/*!
\brief Sensor input information structure 
*/
typedef struct {
    uint8_t     ubInputType;                //!< Cam input format type
	uint8_t		ubVliadVideo;               //!< Input video status(valid or not)
	uint8_t 	ubVideoFormat;	            //!< Input video format(720P/1080P...etc)
	uint8_t 	ubVideoType;                //!< Input video type(AHD/TVI/CVI...etc)
    uint8_t 	ubVideoFPS;                 //!< Input video FPS(25/30)
    uint8_t     ubOldVideoFormat;           //!< Old input video format
	uint8_t 	ubOldVideoType;             //!< Old input video type
    uint8_t 	ubOldVideoFPS;              //!< Old input video FPS
    uint8_t     ubField;                    //!< Field status(Odd/Even)
    uint8_t     ubDetectVideo;              //!< Detect input cam status(0:no video/1:detect video)
}SEN_STATE;

/*!
\brief Sensor event process structure 
*/
typedef struct {
	uint8_t		ubSrcNum;                   //!< Source number(ISP path1/2/3)
	uint8_t 	ubCurNode;	                //!< Current node
	uint8_t 	ubNextNode;                 //!< Next node
	uint32_t	ulDramAddr1;                //!< Drma address1
	uint32_t	ulDramAddr2;	            //!< Drma address2
	uint32_t	ulSize;                     //!< Image size
	uint8_t 	ubPath;                     //!< Path
	uint8_t 	ubReserved1;
	uint32_t	ulTmCnt;
}SEN_EVENT_PROCESS;

/*!
\brief Sensor status structure 
*/
typedef struct {
    uint8_t     ubSensorInit;               //!< Sensor initial state
    uint8_t     ubSensorTrig;               //!< Sensor trigger flag
    uint8_t     ubSensorState;              //!< Sensor work state
    uint8_t     ubSensorFPS;                //!< Sensor FPS
}SEN_SENSOR_STATE_t;

/*!
\brief Sensor CVBS status structure 
*/
typedef struct {
    uint8_t     ubFieldState;               //!< Field state(odd/even)
    uint8_t     ubFieldSync;                //!< Field sync flag
    uint8_t     ubFieldMergeFin;            //!< Merge to frame state
}SEN_CVBS_STATE_t;

typedef void (*pvSEN_CbFunc)(void);
typedef uint8_t (*pvSEN_SyncCbFunc)(void);
typedef void (*pvSEN_HwEndIsrCbFunc)(uint8_t, uint32_t, uint32_t);
typedef uint32_t (*pSEN_IsrProcCb)(void);
typedef void (*pvSEN_CbAhdSetResFunc)(uint16_t, uint16_t);
typedef void (*pvSEN_CbAhdSetBufFunc)(uint16_t, uint16_t);
//------------------------------------------------------------------------------
//! DEFINITION
//------------------------------------------------------------------------------
//! Sensor Format
#include "SEN_TYPE.h"
	
//! Image for ISP
    #define ISP_HD				(0)
    #define ISP_FHD				(1)
    #define ISP_1296P			(2)

//! SENSOR Interface
    #define SEN_DVP				(0)
    #define SEN_MIPI			(1)
    
    #define SEN_30FPS           (0)
    #define SEN_60FPS			(1)

    #if (SEN_USE == SEN_OV9732)
        #include "OV9732.h"
        #define ISP_RES				ISP_HD
    #elif (SEN_USE == SEN_OV9750)
        #include "OV9750.h"
        #define ISP_RES				ISP_HD
    #elif (SEN_USE == SEN_AR0330)
        #include "AR0330.h"
        #define ISP_RES				ISP_1296P
    #elif (SEN_USE == SEN_H62)
        #include "H62.h"
        #define ISP_RES				ISP_HD
        #define SEN_INTERFACE       SEN_DVP
    #elif (SEN_USE == SEN_H63)
        #include "H63.h"
        #define ISP_RES				ISP_HD
    #elif (SEN_USE == SEN_H65)
        #include "H65.h"
        #define ISP_RES				ISP_HD
    #elif (SEN_USE == SEN_H66)
        #include "H66.h"
        #define ISP_RES				ISP_HD
    #elif (SEN_USE == SEN_IMX323)
        #include "IMX323.h"
        #define ISP_RES				ISP_FHD
    #elif (SEN_USE == SEN_SC2235)
        #include "SC2235.h"
        #define ISP_RES				ISP_FHD
    #elif (SEN_USE == SEN_SC2238H)
        #include "SC2238H.h"
        #define ISP_RES				ISP_FHD
    #elif (SEN_USE == SEN_SC2239)
        #include "SC2239.h"
        #define ISP_RES				ISP_FHD
    #elif (SEN_USE == SEN_RN6752)
        #include "RN6752.h"
        #define ISP_RES				ISP_FHD
    #elif (SEN_USE == SEN_IMX307)
        #include "IMX307.h"
        #define ISP_RES				ISP_FHD
    #elif (SEN_USE == SEN_TP9950)
		#include "TP9950.h"
		#define ISP_RES				ISP_FHD
    #elif (SEN_USE == SEN_GC2053)
        #include "GC2053.h"
        #define ISP_RES				ISP_FHD
		#define ISP_FPS				SEN_30FPS
        #define SEN_INTERFACE       SEN_DVP
    #elif (SEN_USE == SEN_GL3004)
		#include "GL3004.h"
		#define ISP_RES				ISP_FHD
    #elif (SEN_USE == SEN_SC2332)
		#include "SC2332.h"
		#define ISP_RES				ISP_FHD
	#elif (SEN_USE == SEN_IMX462)
        #include "IMX462.h"
        #define ISP_RES				ISP_FHD	
	#elif (SEN_USE == SEN_H68)
        #include "H68.h"
        #define ISP_RES				ISP_HD
		#define SEN_INTERFACE       SEN_DVP	
	#elif (SEN_USE == SEN_GC4653)
        #include "GC4653.h"
        #define ISP_RES				ISP_1296P
	#elif (SEN_USE == SEN_PS5268)
        #include "PS5268.h"
        #define ISP_RES				ISP_FHD
		#define SEN_INTERFACE       SEN_DVP	
	#elif (SEN_USE == SEN_GC1084)
        #include "GC1084.h"
        #define ISP_RES				ISP_HD		
	#elif (SEN_USE == SEN_GC2083)
        #include "GC2083.h"
        #define ISP_RES				ISP_FHD		
	#endif
	#if (defined(A7130) && BSP_DDRSIZE == 16 && ISP_RES != ISP_HD)
		#undef  ISP_RES
		#define ISP_RES				ISP_HD
	#endif

    #if (ISP_RES == ISP_HD)  
        #define ISP_WIDTH	1280
        #define ISP_HEIGHT	720
    #elif (ISP_RES == ISP_FHD)
        #define ISP_WIDTH	1920
        #define ISP_HEIGHT	1088 
    #elif (ISP_RES == ISP_1296P)
        #define ISP_WIDTH	2304
        #define ISP_HEIGHT	1296 
    #endif
    #define SEN_FrmEnd_ISR()    { AE_FrmEndIsr_Handler(); AWB_FrmEndIsr_Handler(); AF_FrmEndIsr_Handler(); }
    #define SEN_SRC_NONE	    0xFF				//!< Source Number (NONE) 
    #define ISP_ALG_REPORT_CNT	(1)
    #define SEN_DROP_FRAME      (1)
    #define SEN_RES_DROP_FRAME  (3)
    //bypass IQ?
    #if ((SEN_USE == SEN_RN6752) || (SEN_USE == SEN_TP9950))
        #define IQ_BIN_STATE    SEN_BYPASS_IQ
    #else
        #define IQ_BIN_STATE    SEN_LOAD_IQ//SEN_BYPASS_IQ
    #endif
    //I2C type
    #define I2C_TYPE_SELECT     I2C_TPYE_INTERRUPT	//I2C_TYPE_POLLING
    //Sensor Buffer
//    #if (BSP_DDRSIZE == 16)
//        #if (defined(RTC676x))
//            #define SEN_BUF_NUMBER  (1)
//        #else
//            #define SEN_BUF_NUMBER  (2)
//        #endif
//    #else
//        #define SEN_BUF_NUMBER  (2)
//    #endif
//==============================================================================
// FUNCTION
//==============================================================================
//------------------------------------------------------------------------
/*!	
\brief Get acitve flag
\param ubPath ISP Path
\return Flag
*/
uint8_t ubSEN_GetActiveFlg(uint8_t ubPath);
//------------------------------------------------------------------------
/*!
\brief 	Get first out flag
\return Flag
*/
uint8_t ubSEN_GetFirstOutFlg(void);

//------------------------------------------------------------------------
/*!
\brief 	Set change resoltuion state
\return Flag
*/
void SEN_SetResChgState(uint8_t ubPath, uint16_t uwH, uint16_t uwV);   
//------------------------------------------------------------------------
/*!
\brief ISP(AE, AF, AWB, MD, DIS...etc) initialize.
\return(no)
\par [Example]
\code 
		SEN_ISPInitial();	
\endcode	
*/
void SEN_ISPInitial(void);
//------------------------------------------------------------------------
/*!
\brief ISR(HW_END, VSYNC...etc) and task initialize
\return(no)
\par [Example]
\code 
		SEN_ISRInitial();	
\endcode	
*/
void SEN_ISRInitial(void);
//------------------------------------------------------------------------
/*!
\brief Sensor HSync interrupt
\return(no)
\par [Example]
\code 
		INTC_IrqSetup(INTC_SEN_HSYNC_IRQ, SEN_Hsync_ISR);
\endcode	
*/
void SEN_Hsync_ISR(void);
//------------------------------------------------------------------------
/*!
\brief Sensor Hw_end interrupt
\return(no)
\par [Example]
\code 
		INTC_IrqSetup(INTC_ISP_WIN_END_IRQ, SEN_HwEnd_ISR);
\endcode	
*/
void SEN_HwEnd_ISR(void);
//------------------------------------------------------------------------
/*!
\brief Sensor VSync interrupt.
\return(no)
\par [Example]
\code 
		INTC_IrqSetup(INTC_SEN_VSYNC_IRQ, SEN_Vsync_ISR);
\endcode	
*/
void SEN_Vsync_ISR(void);
//------------------------------------------------------------------------
/*!
\brief Initial Vsync task.
\return(no)
\par [Example]
\code 
		SEN_VsyncInit();
\endcode
*/
void SEN_VsyncInit(void);
//------------------------------------------------------------------------
/*!
\brief Setup Sensor resolution.
\param ubPath 	Select video path.
\param ulWidth 	Horizontal size.
\param ulHeight Vertical size.
\return(no)
\par [Example]
\code 
		SEN_SetResolution(SENSOR_PATH1, 1280, 720);
\endcode	
*/
void SEN_SetResolution(uint8_t ubPath, uint32_t ulWidth,uint32_t ulHeight);
//------------------------------------------------------------------------
/*!
\brief Set sensor init setting(window size, type).
\return(no)
*/
void SEN_SensorInitial(void);
//------------------------------------------------------------------------
/*!
\brief 	Sensor initialize
\return	SEN_InitFail/SEN_InitOk
\par [Example]
\code 
		SEN_InitProcess();	
\endcode		
*/
#if (defined(OP_STA) || (defined(OP_AP) && defined(BSP_DVR_SDK)))
uint8_t ubSEN_InitProcess(void);
#else
#define ubSEN_InitProcess()			((void)0)
#endif
//------------------------------------------------------------------------
/*!
\brief Set ISP rate.
\param ulbSP_div 		Number of divison.
\return(no)
\par [Example]
\code 
		SEN_SetISPRate(8);
		
		F(ISP_CLK)= DDR_PLL/ (2^SYS_RATE) /ISP_RATE.
		where SYS_RATE is 0x9000_0008[1:0]
\endcode
*/
void SEN_SetISPRate(uint8_t ulbSP_div);
//------------------------------------------------------------------------
/*!
\brief Set ISP AXI rate.
\param ubAXI_div 		Number of divison.
\return(no)
\par [Example]
\code 
		SEN_SetAXIRate(5);
		
		F(ISP_AXI_CLK)= SYS_CLK /ISP_AXI_RATE.
		where SYS_RATE is 0x9000_000C[6:3]
\endcode
*/
void SEN_SetAXIRate(uint8_t ubAXI_div);
//------------------------------------------------------------------------
/*!
\brief Set sensor rate.
\param ubSelBaseClk 	Select sensor clock generator.
\param ubSensorDiv 		Number of divison.
\return(no)
\par [Example]
\code 
		SEN_SetSensorRate(SENSOR_96MHz, 12);
		
		sensor clock = 96MHz(120MHz) / SEN_RATE
\endcode
*/
void SEN_SetSensorRate(uint8_t ubSelBaseClk, uint8_t ubSensorDiv);
//------------------------------------------------------------------------
/*!
\brief 	Set sensor output source
\param 	ubPath1Src	Source number for PATH1
\param 	ubPath2Src	Source number for PATH2
\param 	ubPath3Src	Source number for PATH3
\return (no)
\par [Example]
\code 
		SEN_SetPathSrc(KNL_SRC_NONE,KNL_SRC_NONE,KNL_SRC_1_SUB);
\endcode
*/
void SEN_SetPathSrc(uint8_t ubPath1Src,uint8_t ubPath2Src,uint8_t ubPath3Src);
//------------------------------------------------------------------------
/*!
\brief 	Get sensor output source
\param 	ubPath Sensor path
\return Source number
\par [Example]
\code 
		ubSEN_GetPathSrc(SENSOR_PATH1);
\endcode
*/
uint8_t ubSEN_GetPathSrc(uint8_t ubPath);
//------------------------------------------------------------------------
/*!
\brief Set path buffer address
\param ubPath	Output path
\param ulBufAddr	Buffer address
\return (no)
\par [Example]
\code 
		SEN_SetPathAddr(SENSOR_PATH1, ulTemp1);
\endcode
*/
void SEN_SetPathAddr(uint8_t ubPath,uint32_t ulBufAddr);
//------------------------------------------------------------------------
/*!
\brief 	Set sensor output resolution
\param 	ubPath	Output path
\param 	uwH	Horizontal resolution
\param 	uwV	Vertical resolution
\return (no)
\par [Example]
\code 
		SEN_SetOutResolution(ubPath, ulWidth, ulHeight);
\endcode
*/
void SEN_SetOutResolution(uint8_t ubPath,uint16_t uwH,uint16_t uwV);
//------------------------------------------------------------------------
/*!
\brief 	Set acitve flag
\param 	ubFlg	Flag
\param ubPath ISP Path
\return (no)
\par [Example]
\code 
		SEN_SetActiveFlg(1);
\endcode
*/
void SEN_SetActiveFlg(uint8_t ubPath,uint8_t ubFlg);
//------------------------------------------------------------------------
/*!
\brief Set first out flag
\param ubFlg	Flag
\return (no)
\par [Example]
\code 
		SEN_SetFirstOutFlg(1);
\endcode
*/
void SEN_SetFirstOutFlg(uint8_t ubFlg);
//------------------------------------------------------------------------
/*!
\brief Set the flag of change resolution state.
\param ubPath     ISP path.(SENSOR_PATH1/2/3)
\param ubFlg     TRUE/FALSE.
\return (no)n.
*/
void SEN_SetResChgFlg(uint8_t ubPath, uint8_t ubFlg);
//------------------------------------------------------------------------
/*!
\brief Get the flag of change resolution state.
\param ubPath     ISP path.(SENSOR_PATH1/2/3)
\return flag state.
*/
uint8_t ubSEN_GetResChgFlg(uint8_t ubPath);
//------------------------------------------------------------------------
/*!
\brief Set state change flag
\param ubFlg	Flag
\return (no)
\par [Example]
\code 
		SEN_SetStateChangeFlg(1);
\endcode
*/
void SEN_SetStateChangeFlg(uint8_t ubFlg);
//------------------------------------------------------------------------
/*!
\brief Get state change flag
\return Flag
\par [Example]
\code 
		ubSEN_GetStateChangeFlg(1);
\endcode
*/
uint8_t ubSEN_GetStateChangeFlg(void);
//------------------------------------------------------------------------
/*!
\brief Update path buffer address
\return (no)
\par [Example]
\code 
		SEN_UpdatePathAddr();
\endcode
*/
void SEN_UpdatePathAddr(void);
//------------------------------------------------------------------------
/*!
\brief Register event queue
\param tQueueId	Queue
\return (no)
\par [Example]
\code 
		SEN_RegisterEventQueue(KNL_ProcessQueue);
\endcode
*/
#if (defined(OP_STA) || (defined(OP_AP) && defined(BSP_DVR_SDK)))
    void SEN_RegisterEventQueue(osMessageQId tQueueId);
#else
    #define SEN_RegisterEventQueue(tQueueId)	((void)0)
#endif
//------------------------------------------------------------------------
/*!
\brief Register event node
\param ubEventNode	Event node
\return (no)
\par [Example]
\code 
		SEN_RegisterEventNode(KNL_NODE_SEN_YUV_BUF);
\endcode
*/
#if (defined(OP_STA) || (defined(OP_AP) && defined(BSP_DVR_SDK)))
    void SEN_RegisterEventNode(uint8_t ubEventNode);
#else
    #define SEN_RegisterEventNode(ubEventNode)	((void)0)
#endif
//------------------------------------------------------------------------------
/*!
\brief 	Set flag of UVC path, 1:UVC path exist.	
\par [Example]
\code		 
	 SEN_SetUvcPathFlag(1);
\endcode
*/
void SEN_SetUvcPathFlag(uint8_t ubFlag);
//------------------------------------------------------------------------
/*!
\brief 	Load IQ bin file.
\return	(no)	
*/
#if (defined(OP_STA) || (defined(OP_AP) && defined(BSP_DVR_SDK)))
    void SEN_LoadIQData(void);
#else
    #define SEN_LoadIQData()	((void)0)
#endif
//------------------------------------------------------------------------
/*!
\brief Get sensor type.
\return sensor name.
\par [Example]
\code 
        uint8_t ubType;
		ubType = ubSEN_GetSensorType();
        where ubType is SEN_XXXX
\endcode
*/
uint8_t ubSEN_GetSensorType(void);
//------------------------------------------------------------------------
/*!
\brief Get maximum frame rate.
\return Max frame rate.
*/
uint8_t ubSEN_GetMaxFrameRate(void);
//------------------------------------------------------------------------
/*!
\brief Load IQ bin file state.
\return 0=>load IQ bin, 1=>bypass ISP.
*/
uint8_t ubSEN_GetByPassIqState(void); 
//------------------------------------------------------------------------
/*!
\brief Get sensor state.
\return 1=>sensor work, 0=>sensor unusual.
*/
uint8_t ubSEN_GetSensorState(void);
//------------------------------------------------------------------------
/*!
\brief 	Get detect video input state.
\return 1:detect video, 0:No video 
*/
uint8_t ubSEN_GetDetectVideoState(void);
//------------------------------------------------------------------------
/*!
\brief 	Set detect video input state.
\param ubState      1:detect video, 0:No video
\return (no)
*/
void SEN_SetDetectVideoState(uint8_t ubState);
//------------------------------------------------------------------------
/*!
\brief Get IQ bin file address.
\return address.
*/
uint32_t ulSEN_IQBinAddr(void);
//------------------------------------------------------------------------
/*!
\brief Set ISP scale mode.
\param tMode        SEN_SCALEUP_MODE/SEN_NORMAL_MODE
\return (no)
\par [Example]
\code		 
    SEN_NORMAL_MODE
    ==> ISP scale-down/same
    SEN_SCALEUP_MODE
    ==> ISP scal-up
\endcode
*/
void SEN_SetScaleMode(SCALE_TYPE tMode);
//------------------------------------------------------------------------
/*!
\brief Get ISP scale mode.
\return scale mode.
*/
uint8_t ubSEN_GetScaleMode(void);
//------------------------------------------------------------------------
/*!
\brief 	Get sensor slave address(7bit).
\return(no)
*/
uint8_t ubSEN_SensorSlaveAddr(void);
//------------------------------------------------------------------------
/*!
\brief 	Set ISP test pattern.
\param ubSwitch		0:OFF 1:ON.
\return(no)
\par [Example]
\code		 
	 SEN_SetISPTestPattern(0);
     ==> Normal image output
     SEN_SetISPTestPattern(1);
     ==> ISP test pattern
\endcode
*/
void SEN_SetISPTestPattern(uint8_t ubSwitch);
//------------------------------------------------------------------------
/*!
\brief If ISP occur error, print error message.
\return (no)
*/
void SEN_ChkISPState(void); 
//------------------------------------------------------------------------
/*!
\brief Set video stream path state.
\param ubPath 		ISP path.(SENSOR_PATH1/2/3).
\param ubFlag 		0:OFF 1:ON.
\return(no)
\par [Example]
\code 
    Set
		SEN_SetPathState(SENSOR_PATH1,1);
    Then
        SEN_SetIspOutEn(TURE);
    image output to path1.
\endcode
*/
void SEN_SetPathState(uint8_t ubPath, uint8_t ubFlag);
//------------------------------------------------------------------------
/*!
\brief Set image write data to dram.
\param ubEn 		TRUE/FALSE.
\return(no)
\par [Example]
\code 
		SEN_SetIspOutEn(TURE);
        ==>HW_END interrupt and write data to dram.
        SEN_SetIspOutEn(FALSE);
        ==>No HW_END interrupt.
\endcode
*/
void SEN_SetIspOutEn(uint8_t ubEn);
//------------------------------------------------------------------------
/*!
\brief Set First raw data.
\param ubMirrorEn   mirror on/off.
\param ubFlipEn     flip  on/off.
\return(no)
\par [Example]
\code		 
     Normal
        B  Gb B  Gb ...
        Gr R  Gr R  ...
     Mirror
        Gb B  Gb B  ...
        R  Gr R  Gr ...
     Flip   
        Gr R  Gr R  ...
        B  Gr B  Gr ...
     Mirror & Flip
        R  Gr R  Gr ...
        Gr B  Gr B  ...
\endcode
*/
void SEN_SetRawReorder(uint8_t ubMirrorEn, uint8_t ubFlipEn);
//------------------------------------------------------------------------
/*!
\brief Get AHD cam state.
\return 0:ValidVideo, 1:NoVideo.
*/
uint8_t SEN_ReportAhdCamStatus(void);
//------------------------------------------------------------------------------
/*!
\brief 	Set ISP output according to USB format.
\return(no)
\par [Example]
\code 
USB format
    YUY2 ==> BAYER_SENSOR_RAW_DATA
    H264 ==> Other.(look sensor format type)
\endcode
*/
void SEN_CheckUvcFormat (void);
//------------------------------------------------------------------------
/*!
\brief Set ISP pipeline.
\param type         pipeline type.
\return(no)
\par [Example]
\code 
                        input       Bayer   RGB     YUV     output
BAYER_SENSOR_NORMAL     Raw data    O       O       O       Sonix420
BAYER_SENSOR_RAW_DATA   Raw data    X       X       X       Raw data
YUV_SENSOR_RGB_IN       YUV         X       O       O       Sonix420
YUV_SENSOR_RGB_IN       YUV         X       X       O       Sonix420
\endcode
*/
void SEN_SenosrOutputType(ISP_PIPE_TYPE type);
//------------------------------------------------------------------------
/*!
\brief Set sensor output size and ISP process size.
\return(no)
\par [Example]
\code 

                H	         (H_s, V_s)     H
		-----------------           -----------------
		|				|           |				|
		|	   SEN		| V     ==> |	   ISP		| V
		|				|           |				|
        -----------------           -----------------
        **sensor window size can look for sensor initial setting**
        **ISP window size must small or same than sensor image window size**
\endcode	
*/
void SEN_SetISPWindowSize(void);
//------------------------------------------------------------------------------
/*!
\brief 	Set frame drop.
\param ubPath		Frame drop path.
\param ubEnable		Frame drop switch.
\param ubN		    Set N.
\param ubM		    Set M.
\par [Example]
\code		 
	 Path1_fps = sensor_fps * (N1/M1).
     Path2_fps = sensor_fps * (N1/M1) * (N2/M2).
     Path3_fps = sensor_fps * (N1/M1) * (N3/M3).
     M > N when ubEnable = 1.
\endcode
*/
void SEN_SetFrameDrop(uint8_t ubPath, uint8_t ubEnable, uint8_t ubN, uint8_t ubM);
//------------------------------------------------------------------------------
/*!
\brief 	Get frame drop state.	
\param ubPath		Frame drop path.
\return drop state. 
\par [Example]
\code		 
     
     If sensor_fps = 30, set path1 to 10fps.
        SEN_SetFrameDrop(SENSOR_PATH1, 1, 1, 3);
     
	 bSEN_GetFrameDropState(SENSOR_PATH1);
        1 0 0 1 0 0 1 0 0 1
        0 0 1 0 0 1 0 0 1 0
        0 1 0 0 1 0 0 1 0 0
     1: frame isn't drop
     0: frame is drop. 
\endcode
*/
bool bSEN_GetFrameDropState(uint8_t ubPath);
//------------------------------------------------------------------------------
/*!
\brief Set frame rate.(FW control)
\param ubPath		Frame drop path.
\param ubFPS		Frame rate.
\return(no)
\par [Example]
\code		 
    uint8_t ubFPS = 10;
    SEN_SetFrameRate(SENSOR_PATH1, ubFPS)

    note: Set ubFPS need to check ubSEN_GetMaxFrameRate()
\endcode
*/
#if (defined(OP_STA) || (defined(OP_AP) && defined(BSP_DVR_SDK)))
    void SEN_SetFrameRate(uint8_t ubPath, uint8_t ubFPS);
#else
    #define SEN_SetFrameRate(ubPath, ubFPS)	((void)0)
#endif
//------------------------------------------------------------------------------
/*!
\brief Update frame drop table
\return(no)
*/
void SEN_UpdateFrameDropTable(void);
//------------------------------------------------------------------------------
/*!
\brief Get frame drop state.(FW control)
\param ubPath		Select path.
\return 1:keep 0:frame drop
\par [Example]
\code
    SEN_SetFrameRate(SENSOR_PATH1, ubFPS)
    If ubFPS = 10 and ubSEN_GetMaxFrameRate()= 30;
    
                      average drop
    1 1 1 1 1 1 1 1 1               1 1 0 1 1 0 1 1 0
    1 1 1 1 1 1 1 1 1   =======>    1 1 0 1 1 0 1 1 0
    1 1 1 1 1 1 1 1 1               1 1 0 1 1 0 1 1 0
    1 1 1                           1 1 0
    1:keep 0:drop
    
\endcode
*/
uint8_t ubSEN_FrameDropState(uint8_t ubPath);
//------------------------------------------------------------------------
/*!
\brief Select MIPI lane number and channel.
\param tLaneNumber      MIPI lane number.
\param tChannelSelect   MIPI channel select.
\return(no)
\par [Example]
\code		
    If tLaneNumber = 1lane
    tChannelSelect = 
            0==> CP0/CN0, DP0/DN0
            1==> CP1/CN1, DP1/DN1
            2==> CP2/CN2, DP2/DN2
            3==> CP3/CN3, DP3/DN3
    If tLaneNumber = 2lane
    tChannelSelect = 
            0==> CP0/CN0, DP0/DN0, DP1/DN1
            1==> CP2/CN2, DP2/DN2, DP3/DN3
\endcode
*/
void SEN_MIPIControl(MIPI_LANE_NUM_TYPE tLaneNumber, MIPI_CHANNEL_NUM_TYPE tChannelSelect);
//------------------------------------------------------------------------
/*!
\brief Sensor image stable call back function
\param pvCB     Callback function (image stable ready func)
\return (no)
*/
void SEN_SetIspFinishCbFunc(pvSEN_CbFunc pvCB);
//------------------------------------------------------------------------
/*!
\brief CVBS field status call back function
\param pvCB     Callback function (field status func)
\return (no)
*/
void SEN_SyncCvbsFieldCbFunc(pvSEN_SyncCbFunc pvCB);
//------------------------------------------------------------------------
/*!
\brief HW_END ISR call back function
\param pvCB     Callback function (HW_END ISR func)
\return (no)
*/
void SEN_SetHwEndIsrCbFunc(uint8_t ubPath, pvSEN_HwEndIsrCbFunc pvCB);

typedef enum
{
	SEN_VSYNC_PROC = 0,
	SEN_ISPOUT_PROC,
}SEN_IsrProcType_t;
void SEN_RegIsrProcCbFunc(SEN_IsrProcType_t tProcType, pSEN_IsrProcCb cb);
void SEN_SetAhdCamResCbFunc(pvSEN_CbAhdSetResFunc pvCB);
void SEN_SetAhdCamBufCbFunc(pvSEN_CbAhdSetBufFunc pvCB);
uint8_t ubSEN_GetResChgDropCnt(void);

void SEN_SetResChgDropCnt(uint8_t ubValue);
void SEN_SetYuvBufNub(uint8_t ubNb);
void SEN_SetAhdCamRes(uint16_t uwH, uint16_t uwV);
void SEN_SetAhdCamBuf(uint16_t uwH, uint16_t uwV);
uint32_t SEN_Get16MCvbsAddress(void);
uint32_t SEN_Get16MCvbsReleaseAddress(void);
uint8_t ubSEN_GetSensorBufNumber(void);
uint8_t SEN_GetAhdCamType(void);
void SEN_CaptureScaleUp(uint8_t ubEn, uint32_t ulAddr);
uint8_t ubSEN_CaptureScaleUpGet(void);
//==============================================================================
// SENSOR extern item
//==============================================================================
extern struct SENSOR_SETTING sensor_cfg;
extern SEN_STATE sensor_state;
#endif

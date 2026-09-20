/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file       REC_API.h
	\brief		Record header file
	\author     Wales
	\version	0.66
	\date		2022/01/17
	\copyright	Copyright(C) 2016 SONiX Technology Co.,Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------
#ifndef _REC_API_H_
#define _REC_API_H_

#include "_510PF.h"
#include "FS_API.h"

//------------------------------------------------------------------------------
//	DEFINITION
//------------------------------------------------------------------------------
#define REC_DEBUG_LV  DBG_CriticalLvl
#define BUF_REC_TEST_NUM    256

#define REC_SRC_NUM     (0x04)  //!< Define numbers of record Source 

#define REC_MSADPCM	    (1)
#define REC_ALAW		(6)
//------------------------------------------------------------------------------
//	MACRO DEFINITION
//------------------------------------------------------------------------------
// Record and Play memory seperate
#define REC_1FHD_MEMSZ	    (0x6DB400L)
#define REC_1SXGA_MEMSZ	    (0x572400L)
#define REC_1HD_MEMSZ	    (0x4B9E00L)
#define REC_1XGA_MEMSZ	    (0x462400L)
#define REC_1WSVGA_MEMSZ	(0x409400L)
#define REC_1SVGA_MEMSZ	    (0x3AB400L)
#define REC_1WVGA_MEMSZ	    (0x3AB400L)
#define REC_1VGA_MEMSZ	    (0x34D400L)
#define REC_HD_FHD_MEMSZ    (0x950400L)
#define REC_2HD_MEMSZ	    (0x8E4400L)
#define REC_4VGA_MEMSZ	    (0xB41800L)
// Record and Play Memory Common
#define REC_COM_1FHD_MEMSZ	    (0x55F000L)
#define REC_COM_1SXGA_MEMSZ	    (0x3E8000L)
#define REC_COM_1HD_MEMSZ	    (0x4B9E00L)
#define REC_COM_1XGA_MEMSZ	    (0x2D5000L)
#define REC_COM_1WSVGA_MEMSZ	(0x28A000L)
#define REC_COM_1SVGA_MEMSZ	    (0x226000L)
#define REC_COM_1WVGA_MEMSZ	    (0x226000L)
#define REC_COM_1VGA_MEMSZ	    (0x1C2000L)
#define REC_COM_HD_FHD_MEMSZ    (0x950400L)
#define REC_COM_2HD_MEMSZ	    (0x8E4400L)
#define REC_COM_4VGA_MEMSZ	    (0xB41800L)

// Record Only Memory Size
#define REC_ONLY_1FHD_MEMSZ     (0xE0800L)

#define REC_FILE_SIZE_THRESHOLD     300     // 300M, according to data rate 500KB, formula=500(KB)*60(s)*10(min)+500(header)*4(src)
#define REC_PHOTO_SIZE_THRESHOLD    300     
//------------------------------------------------------------------------------
//	DATA STRUCT DEFINITION
//------------------------------------------------------------------------------
typedef enum _RecMemMode
{
	REC_MEM_MODE_SEPERATE,   //!< Record and Play Used different memory buffer.
	REC_MEM_MODE_COMMON,     //!< Record and Play Used the same memory buffer.
	REC_MEM_MODE_MAX,
}REC_MEM_MODE_SEL;
#define REC_MEM_MODE    REC_MEM_MODE_COMMON

typedef enum _eRecFileFormat
{
	REC_FILE_AVI,   //!< AVI File format
	REC_FILE_MP4,   //!< MP4 File format
}EREC_FILE_FORMAT;

typedef enum _eRecTimeLapse
{
    REC_TIMELAPSE_OFF,
    REC_TIMELAPSE_ON,
    REC_TIMELAPSE_MAX,
}EREC_TIME_LAPSE;

typedef enum _RecRole
{
	REC_ROLE_AP,
	REC_ROLE_STA,
	REC_ROLE_MAX,
}REC_ROLE;
typedef enum _RecCamRole
{
	REC_CAM_REMOTE,
	REC_CAM_LOCAL,
	REC_CAM_MAX,
}REC_CAM_ROLE;

typedef enum _RecStreamType
{
	REC_STR_V1,
	REC_STR_A1,
	REC_STR_MAX,
	REC_STR_V2,	
}REC_STREAM_TYPE;

typedef enum _RecFrameType
{
	REC_P_VFRM = 0,
	REC_I_VFRM = 1,
	REC_SKIP_FRM = 2,
}REC_VDO_FRM_TYPE;

typedef enum _RecResolution
{
	REC_RES_NONE,		//!< Undefine
	REC_RES_FHD,		//!< FHD
	REC_RES_HD,		    //!< HD
	REC_RES_WVGA,		//!< WVGA
	REC_RES_VGA,		//!< VGA
}REC_RESOLUTION;

typedef enum _RecSrcConfig{
	REC_SRC_UNCONFIGED, //!< File format of record source is not configuration
	REC_SRC_CONFIGED,   //!< File format of record source is configuration
}REC_SRC_CONFIG;

typedef enum _RecSrcMode{
	REC_SRCMODE_CFG,	//!< Record SourceNumeber is according to configured number
	REC_SRCMODE_DSP,	//!< Record SourceNumeber is according to Display mode,1T,2T,4T
}REC_SRCMODE;

typedef enum _RecMode{
	REC_MODE_NORMAL,	//!< Stop Record type
	REC_MODE_PRERECORD,	//!< Record only once type
	REC_MODE_TIMELAPSE,	//!< Record continue type
}REC_MODE;

typedef enum _RecFsPath{
	REC_FS_PATH_DEFAULT,     //!< Video and Photo store in the same folder
	REC_FS_PATH_CUST1,       //!< Video and Photo store different folders
}REC_FS_PATH;

typedef enum _RecCmd{
	REC_CMD_START,      //!< start command
	REC_CMD_STOP,       //!< stop command
	REC_CMD_RESTART,    //!< re-start command
	REC_CMD_NEXT,       //!< to next file command
	REC_CMD_VIDEXT,		//!< Video stream come from external(wifi buffer) command
	REC_CMD_AUDEXT,		//!< Audio stream come from external(wifi buffer) command
	REC_CMD_VIDINT,		//!< Video stream come from internal(pre record buffer) command
	REC_CMD_AUDINT,		//!< Audio stream come from internal(pre record buffer) command
    REC_CMD_AVDUMMY,	//!< Audio Video stream dummycheck come from internal command
	REC_CMD_PREINIT,    //!< Pre-record initial command
	REC_CMD_PREVID,     //!< Switch video external command to pre record video command.
	REC_CMD_PREAUD,		//!< Switch audio external command to pre record audio command.
}REC_CMD;

typedef enum _RecEvent
{
	REC_EVENT_NORMAL = 0,   //!< Normal Record Event
	REC_EVENT_SCH = 1,      //!< Schdule Record Event
	REC_EVENT_MD = 2,       //!< Motion Record Event
}REC_EVENT;

typedef enum _RecState{
	REC_CREATE_NULL,    //!< Record thread is in IDEL status
	REC_CREATE_INIT,    //!< Record thread is in Initial status
	REC_CREATE_OK,      //!< Record thread create file and is in Recording status
	REC_CLOSE,          //!< Record thread close file status
}REC_STATE;

typedef enum _RecErrCode{
	REC_ERR_SRC_CONFIG,     //!< No any sourcees dont configured
	REC_ERR_FUNC_UNDEFINE,    //!< System timer doesnt install
	REC_ERR_PRE_MEMSIZE,    //!< Precord memory size is not enough
	REC_ERR_PRE_GOP,        //!< Precord GOP number is not enough
	REC_ERR_PRE_MEMINIT,    //!< Precord memory is not initial
	REC_ERR_MAX,
}REC_ERR_CODE;

typedef enum _RecExtFuncErrCode
{
	REC_ADO_ADR,		//!< Bit 0, 
	REC_ADO_SIZE,		//!< Bit 1, 
	REC_TIMER_1MS,		//!< Bit 2, 
	REC_EVENT_RECONE,	//!< Bit 3, 
	REC_DISP_TYPE,          //!< Bit 4, 
	REC_CFGDISP_TYPE,          //!< Bit 5, 	
	REC_DISP_SRC,           //!< Bit 6, 
	REC_SEARCH_SRC_LOCATE,  //!< Bit 7, 
	REC_TWC_CMD,            //!< Bit 8, 
	REC_FILE_CREATE,        //!< Bit 9, 
	REC_FUNC_MAX,           //!< Bit 10, 
}REC_EXTFUNC_ERR_CODE;

typedef enum _RecTimestamp{
	REC_TS_NORMAL,      //!< TimeStamp normal
	REC_TS_RESETSYNC,   //!< Tx reset reset timestamp sync time
	REC_TS_PRECSYNC,    //!< PreRecord trigger point reset timestamp sync time
	REC_TS_MAX,         //!< TimeStamp normal
}REC_TIMESTAMP;

typedef enum _RecWrStorage{
	REC_WR_SROTRAGE_BF,    //!< 
	REC_WR_SROTRAGE_AF,    //!< 
}REC_WR_STORAGE;
//------------------------------------------------------------------------------
//! brief Record Information
typedef struct _RecInfo {
	uint8_t     ubCh;           //!< Input Source
	uint8_t     ubCmd;          //!< Command
	uint8_t     ubPictureType;  //!< Frame Type
	uint8_t     ubRestartFg;    //!< System Restart Flag, it uses with "ubREC_VdoPrecReSync", let VdoAddSkipFrame() and vPrecCheckTimeResync() to achive the function of time switch(Tx time<->Rx time).
	uint32_t    ulDramAddr;     //!< Source frame  memory address
	uint32_t    ulSize;         //!< Source frame size
	uint32_t    ulTimeStamp;    //!< Frame time stamp
	uint32_t    ulIdx;          //!< 
	uint8_t     ubFrmSeq;       //!< 
	uint8_t     ubReserved[3];
}REC_INFO;

//------------------------------------------------------------------------------
//! brief Record File Format
typedef struct _RecFileFormat {
	uint8_t     ubConfiged;     //!< Source file format configuration flag
	uint8_t     ubCh;           //!< Source Channel
	uint8_t     ubSourceType;   //!< Defines source type
	uint8_t     ubCamRole;      //!< Define Camera Role, Remote or Local
	uint16_t    uwVFrmInterval; //!< Video Frame Interval
	uint16_t    uwAFrmInterval; //!< Audio Frame Interval
	uint32_t    ulHoSize1;      //!< Video1 horizontal resolution
	uint32_t    ulVoSize1;      //!< Video1 vertical resolution
	uint32_t    ulHoSize2;      //!< Video2 horizontal resolution
	uint32_t    ulVoSize2;      //!< Video2 vertical resolution
	uint32_t    ulSampleRate;   //!< Audio Sample Rate
	uint16_t    uwBlockAlign;   //!< Audio block alignment bytes.
	uint16_t    uwBSMaxBufNum;     //!< VDOBS Max Buffer Number
} REC_FILEFORMAT;

typedef struct _RecExtFuncCtrl
{
	uint32_t (*ulAdoSkipFrameAdrGet)(void);
	uint32_t (*ulAdoSkipFrameSizeGet)(void);
	uint32_t (*Time1msCntGet)(void);
	void (*EventRecOnceEnd)(void);
    uint8_t (*KNL_GetPreviewType)(void);
    uint8_t (*KNL_GetAPPCfgDispMode)(void);    
    uint8_t (*ubKNL_GetDispSrc)(uint8_t ubLoaction);
    uint8_t (*KNL_SearchLocationSrc)(uint8_t ubLoaction);
    uint8_t (*ubKNL_TwcSend)(uint8_t ubRole,uint8_t Opc,uint8_t *Data,uint8_t ubLen,uint8_t ubRetry);
    void (*KNL_RecFileCreate)(uint8_t ubCh,FS_KNL_CRE_PROCESS_t *tRecProc);
    void (*ResetH264IPCnt)(void);
}REC_EXTFUNC_CTRL;


typedef struct _RecMediaFuncCB
{
    void (*MediaCreateFile)(uint8_t ubCh);
    void (*ulMediaWriteInfoData)(uint8_t ubCh, uint8_t ubOffset, uint8_t *ptr, uint8_t ubSize);
    int32_t (*slMediaWriteOneFrame)(uint8_t ubCh, uint8_t ubStreamType, uint16_t uwFrameType, uint32_t ulAddr, uint32_t ulSize, uint32_t ulDesID, uint8_t ubFrmSeq);
    uint8_t (*ubMediaAudioFrameTypeGet)(void);
    void (*MediaClose)(uint8_t ubCh, FS_FILE_SUB_HIDN_INFO_t HidnInfo);
    uint32_t (*ulMediaAlignMoveLength)(uint8_t ubCh, uint8_t ubStreamType);
    uint16_t (*uwMediaMovieLength)(uint8_t ubCh);
    uint32_t (*ulMediaGetStrMaxFrmCnt)(uint8_t ubStreamType);
    void (*MediaSetSourceType)(uint8_t ubCh, uint8_t ubSourceType);
    void (*MediaVdo1SetFormat)(uint8_t ubCh, uint32_t ulHSize, uint32_t ulVSize, uint16_t uwFrmInterval);
    void (*MediaAdoSetFormat)(uint8_t ubCh,uint32_t ulSampleRate, uint16_t uwBlockAlign, uint16_t uwFrmInterval);
    void (*MediaConfigedSrcNum)(uint8_t ubSrcNum);
    uint32_t (*ulMediaGetFixedMemorySize)(void);
    void (*MediaSetRecMaxTime)(uint8_t ubVFPS, uint8_t ubAFPS, uint32_t ulMinute);
    uint32_t (*ulMediaGetSkipFrameSize)(void);
    uint32_t (*ulMediaGet1MINHeadSize)(uint8_t ubMaxVFPS, uint8_t ubMaxAFPS);
    void (*MediaClearUserDataSize)(uint8_t ubCh);
    void (*MediaWriteUserData)(uint8_t ubCh, uint8_t *ptr, uint32_t ulSize);
}REC_MEDIAFUNC_CALLBACK;

typedef struct _RecEmergency
{
    uint8_t ubCloseFileCheck;
    uint16_t uwGroupInf[2];
    uint16_t uwNowGroupIndex;
} REC_EMERGENCY;

//-----------------------------------------------------------------------------
// REC Move file status
typedef enum
{
	REC_MOVE_INIT = 0,	//!< Move file initial
	REC_MOVE_OK			//!< Move file ok
}REC_MOVE_STATUS;
//-----------------------------------------------------------------------------
// REC EMERGENCY status
typedef enum
{
	REC_EMERGENCY_OK = 0,	    //!< Rec emergency check ok
	REC_EMERGENCY_FAIL = 1	    //!< Rec emergency fail
}REC_EMERGENCY_STATUS;
//------------------------------------------------------------------------------
//	FUNCTION PROTOTYPE
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
/*!
\brief Print record error code
\return	(no)
*/
uint16_t uwREC_GetVersion(void);

//------------------------------------------------------------------------------
/*!
\brief Set to support audio record
\param	ubEn Audio enable
\return	(no)
*/
void vREC_AdoEnableSet(uint8_t ubEn);

//------------------------------------------------------------------------------
/*!
\brief Get Audio Enable flag
\return	0 Audio record disabled
        1 Audio record enabled
*/
uint8_t ubREC_AdoEnableGet(void);

//------------------------------------------------------------------------------
void vREC_CWithSenSet(uint8_t ubEn);

//------------------------------------------------------------------------------
/*!
\brief Record Initial function
\param	ubMode      Record SourceNumeber is according to configured number or Display mode,1T,2T,4T
\param	ubFsPath    Video and Photo store position    
\return	(no)
*/
void REC_Init(uint8_t ubMode, uint8_t ubFsPath);

//------------------------------------------------------------------------------
/*!
\brief 	Get the State of Record Task
\param	ubIn ubIn	Selet Source Channel
\return	REC_CREATE_NULL	Record Task is in IDEL status
		REC_CREATE_INIT	Record Task is in Initial status
		REC_CREATE_OK	Record Task create file and is in Recording status
		REC_CLOSE		Record Task close file status
*/
uint8_t ubREC_StateGet(uint8_t ubCh);

//------------------------------------------------------------------------------
/*!
\brief 	Record Once
\return	0 Record once start fail
		1 Record once start success
*/
uint8_t ubREC_Once(void);


//------------------------------------------------------------------------------
/*!
\brief 	Record Continue
\return	0 Record continue start fail
		1 Record continue start success
*/
uint8_t ubREC_Continue(void);

//------------------------------------------------------------------------------
/*!
\brief 	Record stop
\return	0 Record stop fail
		1 Record stop success
*/
uint8_t ubREC_Stop(void);

//------------------------------------------------------------------------------
/*!
\brief 	Get curremt record time
\return	record time
*/
uint32_t ulREC_CurrentTimeGet(void);

//------------------------------------------------------------------------------
/*!
\brief 	Record mode set
\param	memory size
\return	support max record time reference by memory size.
*/
uint32_t ulREC_ModeSet(REC_MEM_MODE_SEL ubMemMode, uint32_t ulMemSize);

REC_MEM_MODE_SEL REC_MemModeGet(void);
//------------------------------------------------------------------------------
/*!
\brief 	Record set file trigger event information
\param	ubEvent	0, Manual record
				1, Time Schedul record
				2, Motion trigger record
\return	(no)
*/
void REC_EventInfoSet(uint8_t ubEvent);

//------------------------------------------------------------------------------
/*!
\brief 	Get curremt record event
\return	0 Normal Record
        1 Schdule Record
        2 Motion Record
*/
uint8_t REC_EventInfoGet(void);

void REC_WriteUserData(uint8_t ubCh, uint8_t *Ptr, uint32_t ulSize);
//------------------------------------------------------------------------------
/*!
\brief 	Set record mode
\param	ubMode record mode
\return	0 Precord memory not initial
        1 Set ok
*/
uint8_t ubREC_SetRecordMode(uint8_t ubMode);

//------------------------------------------------------------------------------
/*!
\brief 	Get curremt record mode
\return	0 Normal Record
        1 Precord Record
        2 TimeLapse Record
*/
uint8_t ubREC_GetRecordMode(void);

//------------------------------------------------------------------------------
/*!
\brief Get Record Version
\return	Firmware Verison
*/
uint16_t uwREC_GetVersion(void);


void REC_SetLogLvl(uint8_t lv);

uint8_t ubREC_ExternFuncInit(REC_EXTFUNC_CTRL *pEventFuncTab);

uint8_t ubREC_MediaExternFuncInit(REC_MEDIAFUNC_CALLBACK *pEventFuncTab);

//------------------------------------------------------------------------------
/*!
\brief
\param	FileFormat A pointer point to file format strcture
\return	0 Configure fail
		1 Configure success.
*/
uint8_t REC_FileFormatConfigure(REC_FILEFORMAT *FileFormat);

//------------------------------------------------------------------------------
/*!
\brief 	Send Video Queue Command
\param	pRecInfo A pointer point to record information strcture
\return	0 Send fail
		1 Send success
*/
uint8_t ubREC_SendVDOQueue(REC_INFO *pRecInfo);

//------------------------------------------------------------------------------
/*!
\brief 	Send Audio Queue Command
\param	pRecInfo A pointer point to record information strcture
\return	0 Send fail
		1 Send success
*/
uint8_t ubREC_SendADOQueue(REC_INFO *pRecInfo);

//------------------------------------------------------------------------------
/*!
\brief Set record file format
\param	ubFmt record file format
\return	(no)
*/
void vREC_FileFormatSet(uint8_t ubFmt);

//------------------------------------------------------------------------------
/*!
\brief Get record file format
\return	Record file format
*/
uint8_t ubREC_FileFormatGet(void);

//------------------------------------------------------------------------------
/*!
\brief Get Record Source Number
\return	REC_SRCMODE_CFG, Record SourceNumeber is according to configured number
            REC_SRCMODE_DSP, Record SourceNumeber is according to Display mode,1T,2T,4T
*/
uint8_t ubREC_SrcModeGet(void);

//------------------------------------------------------------------------------
/*!
\brief 	Set record time for every file
\param	ubReset     
        0:Un Reset record time counter
        1:Reset record time counter
\param	ulTime Unit:Second
        0:Un change record time
\return	0:Over Support time. Timelapse except. Support MAX=5Min
		1:Setting Record time success.
*/
uint8_t REC_TimeSet(uint8_t ubReset,uint32_t ulTime);
uint32_t REC_GetRecTime(void);
//------------------------------------------------------------------------------
/*!
\brief 	Set record Emergency Trigger
\return	REC Emergency Status
        REC_EMERGENCY_OK
        REC_EMERGENCY_FAIL, not in rec mode or repeat command
*/
REC_EMERGENCY_STATUS REC_EmergencyTrigger(void);
//------------------------------------------------------------------------------
/*!
\brief 	Set record Emergency state
\param	State       REC Move file status     
\return	(no)
*/
void REC_SetEmergencyState(REC_MOVE_STATUS State);
//------------------------------------------------------------------------------
/*!
\brief 	REc Emergency Registered Queue 
\return	(no)
*/
void REC_RegisteredEmergencyQueue(osMessageQId *Queue);

void REC_BurningTestParameter(uint8_t ubEn,uint16_t uwInterval);

#if 0   // Test FS Write Speed
void REC_MonitorQueSet(uint8_t ubCh, uint32_t ulIdx);
void REC_MonitorQueGet(uint8_t ubCh, uint32_t ulIdx);
#endif
uint8_t REC_UpdateChkFinish(void);
void vREC_FrameEndPointEx(uint8_t ubCh,REC_VDO_FRM_TYPE FrameType, uint8_t ubDir,uint8_t ubFrmSeq);
void vREC_TimeLapseVideoFPS(uint8_t ubEn,uint8_t ubFPS);
#endif


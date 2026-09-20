/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file       PLY_API.h
	\brief		Play header file
	\author		Wales
	\version    0.65
	\date       2023/02/17
	\copyright	Copyright(C) 2016 SONiX Technology Co.,Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------
#ifndef _PLY_API_H_
#define _PLY_API_H_

#include "_510PF.h"
#include "FS_API.h"

//------------------------------------------------------------------------------
//	DEFINITION
//------------------------------------------------------------------------------
#define PLY_DEBUG_LV  DBG_InfoLvl

#define PLY_FRMTYPE_NULL        (0)             //!< Bitstream type in avi file is undefined
#define PLY_FRMTYPE_VDO         (0x63643030)    //!< Bitstream type in avi file is video
#define PLY_FRMTYPE_ADO         (0x62773130)    //!< Bitstream type in avi file is audio

//Operation Mode Define	
#define PLY_MODE_N              (0x00)  //!< To indicate system operate in preview mode
#define PLY_MODE_R              (0x01)  //!< To indicate system operate in preview with recording mode
#define PLY_MODE_P              (0x10)  //!< To indicate system operate in play mode
#define PLY_MODE_PR             (0x11)  //!< To indicate system operate in play with recording mode
//------------------------------------------------------------------------------
//	MACRO DEFINITION
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
//	DATA STRUCT DEFINITION
//------------------------------------------------------------------------------
typedef enum _PlyExtFuncErrCode
{
	PLY_ADO_START,		//!< Bit 0, 
	PLY_ADO_STOP,		//!< Bit 1, 
	PLY_ADO_CHPLYSET,	//!< Bit 2, 
	PLY_ADO_CHSET,		//!< Bit 3, 
	PLY_ADO_CHGET,		//!< Bit 4, 
	PLY_ADO_BUFWR,		//!< Bit 5, 
	PLY_ADO_BUFCHK,		//!< Bit 6, 
	PLY_ADO_BUFRST,		//!< Bit 7, 
	PLY_ADO_MUTE,		//!< Bit 8, 
	PLY_TIMER_1MS,		//!< Bit 9, 
	PLY_RTC_1S,			//!< Bit 10, 
	PLY_H264_ENTRY,		//!< Bit 11, 
	PLY_H264_CHGET,		//!< Bit 12,
	PLY_EVENT_FILE,		//!< Bit 13, 
	PLY_EVENT_TIMEBAR,	//!< Bit 14, 
	PLY_EVENT_PLAYOVER,	//!< Bit 15, 
	PLY_FUNC_MAX,
}PLY_EXTFUNC_ERR_CODE;

typedef enum _PlyEvent
{
    PLY_EVENT_STOP,
    PLY_EVENT_OVER,
}PLY_EVENT;

//! Indicate stbl information, for play used
typedef struct _ply_chunk_controller{
	uint32_t    ulFileOffset;
	uint32_t    ulFrmType;

	uint32_t    ulVDOTotalSTSC;
	uint32_t    ulVDOTotalChk;
	uint32_t    ulVDOChkIdx;
	uint32_t    ulVDOChkAdrOffset;
	uint32_t    ulVDOTotalFrm;
	uint32_t    ulVDOFrmIdxInFile;
	uint32_t    ulVDOFrmSize;

	uint32_t    ulADOTotalSTSC;
	uint32_t    ulADOTotalChk;
	uint32_t    ulADOChkIdx;
	uint32_t    ulADOChkAdrOffset;
	uint32_t    ulADOTotalFrm;
	uint32_t    ulADOFrmIdxInFile;
	uint32_t    ulADOFrmSize;
} PLY_CHUNK_CONTROLLER;

typedef struct _PlyAudioExtFuncTab
{
	void (*AdoStart)(void);
	void (*AdoStop)(void);
	void (*AdoSetPlayCh)(void);
	void	(*AdoCurChSet)(uint8_t ubCh);
	uint8_t (*ubAdoCurChGet)(void);
	void (*AdoBufWrite)(uint8_t EncType, uint32_t ulSrcAdr, uint32_t ulSize);
	uint8_t (*ubAdoBuffChk)(uint32_t ulSize);
	void (*AdoResetBuf)(void);
	void	(*AdoMute)(uint8_t ubEn);
} PLY_AUDIO_EXTFUNC_TAB;

typedef struct _PlyTimeExtFuncTab
{
	uint32_t (*Time1msCntGet)(void);
	void (*Time1sRTCGet)(uint32_t *ulVal);
}PLY_TIME_EXTFUNC_TAB;

typedef struct _PlyH264ExtFuncTab
{
	int32_t (*H264DecEntry)(uint8_t ubCh, uint32_t ulAddress, uint32_t ulSize);
	uint8_t (*H264DeceTargetChGet)(void);
}PLY_H264_EXTFUNC_TAB;

typedef struct _PlyEventExtFuncTab
{
	void (*EventFileOpened)( uint8_t ubCh);
	void (*EventTimeBarfresh)(void);
	void (*EventPlayOver)(PLY_EVENT tEvent);
    uint8_t (*ubEventTXBufUse)( uint8_t ubCh);
} PLY_EVENT_EXTFUNC_TAB;

typedef struct _PlyExtFuncCtrl
{
	PLY_AUDIO_EXTFUNC_TAB sAdoFunc;
	PLY_TIME_EXTFUNC_TAB sTimeFunc;
	PLY_H264_EXTFUNC_TAB sH264Func;
	PLY_EVENT_EXTFUNC_TAB sEventFunc;
}PLY_EXTFUNC_CTRL;

//! Indicate stbl information, for play used
typedef struct _ply_mp4hd_info{
	uint16_t    uwFTypeSz;
	uint32_t    ulFreeSz;
	uint16_t    uwMdatSz;
	uint32_t    ulHdTotalSz;
} PLY_MP4HD_INFO;

typedef struct _PlyMediaFuncCB
{
    uint32_t ulAVIHeadSize;
    uint32_t ulAVIChunkSize;
    uint32_t ulAVIThumbnailSize;
    uint32_t ulMP4ThumbnailSize;
    void (*LoadAVIHeader)(uint8_t ubCh, uint32_t ulAddr);
    uint32_t (*ulParseVdoIFrmTBL)(uint8_t ubCh, uint32_t ulAdr,PLY_CHUNK_CONTROLLER *PlyChkCtrl);
    uint32_t (*ulParse1stIFrmTBL)(uint8_t ubCh, uint32_t ulAdr,PLY_CHUNK_CONTROLLER *PlyChkCtrl);
    uint32_t (*ulParseAdoFrmTBL)(uint8_t ubCh, uint32_t ulAdr,PLY_CHUNK_CONTROLLER *PlyChkCtrl);
    uint8_t (*ubGetRes)(uint8_t ubCh, uint8_t ubStreamType);
    uint32_t (*ulGetMoviLen)(uint8_t ubCh);
    uint32_t (*ulGetMoviFrmCnt)(uint8_t ubCh, uint8_t ubStreamType);
    uint32_t (*ulGetScale)(uint8_t ubCh, uint8_t ubStreamType);
    uint32_t (*ulGetTotalVdoIFrm)(uint8_t ubCh, uint8_t ubStreamType) ;
    uint32_t (*ulGetAdoSampleRate)(uint8_t ubCh);
    uint8_t (*ubGetAdoCodec)(uint8_t ubCh);
    uint32_t (*ulGetAdoAvgBytesPerSec)(uint8_t ubCh);
    void (*vGetHeadInfo)(PLY_MP4HD_INFO *PlyMP4HdInfo);
    void (*ParseAdoFmt)(uint8_t ubCh, uint32_t ulAddr);
    void (*AssignStruct)(uint8_t ubCh, uint32_t ulAdr, uint32_t ulSize);
    uint32_t (*ulGetStreamTimeScale)(uint8_t ubCh, uint8_t ubStreamType);
    uint32_t (*ulParseMdatFrameInfo)(uint8_t ubCh,PLY_CHUNK_CONTROLLER *PlyChkCtrl);
    uint32_t (*ulGetVdoFrmSize)( uint8_t ubCh , uint32_t ulFrm);
    uint32_t (*ulGetStreamDelayTime)(uint8_t ubCh);
    uint32_t (*ulJumpReload)(uint8_t ubCh, PLY_CHUNK_CONTROLLER *PlyChkCtrl);
}PLY_MEDIAFUNC_CALLBACK;

//! brief Play Jump Type
typedef enum _PlyJumpType
{
	PLY_JUMP_FWD = 1,			//!< Jump forward
	PLY_JUMP_BWD = 2,			//!< Jump backward
	PLY_JUMP_KEEP = 3,			//!< No Jump
	PLY_JUMP_RESTART = 4			//!< Jump Restart
}PLY_JUMP_TYPE;

//! brief Start Play Mode
typedef enum _PlyStartMode
{
	PLY_MODE_STOPFRAME = 0,			//!< Start play and pause at first I frame
	PLY_MODE_NORMAL = 1,			//!< Start play without pause.
}PLY_START_MODE;

//! brief Start Play Mode
typedef enum _PlyPauseAct
{
	PLY_PAUSE_OFF = 0,			//!< Pause OFF
	PLY_PAUSE_ON = 1,			//!< Pause ON
}PLY_PAUSE_ACT;

//------------------------------------------------------------------------------
//	FUNCTION PROTOTYPE
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
/*!
\brief Get Play Version
\return	(no)
*/
uint16_t uwPLY_GetVersion(void);

void PLY_SetLogLvl(uint8_t lv);

uint8_t ubPLY_AudioExtFuncInit(PLY_AUDIO_EXTFUNC_TAB *sAdoFuncTab);
uint8_t ubPLY_TimeExtFuncInit(PLY_TIME_EXTFUNC_TAB *pTimeFuncTab);
uint8_t ubPLY_H264ExtFuncInit(PLY_H264_EXTFUNC_TAB *pH264FuncTab);
uint8_t ubPLY_EventExtFuncInit(PLY_EVENT_EXTFUNC_TAB *pEventFuncTab);
uint8_t ubPLY_MediaExternFuncInit(PLY_MEDIAFUNC_CALLBACK *pEventFuncTab);

//------------------------------------------------------------------------------
/*!
\brief 	Caculate requested memory buffer size
\param	ubSrcNum Numbers of total source channel
\return	Requested memory buffer size
*/
uint32_t ulPLY_GetBufSz(void);

//------------------------------------------------------------------------------
/*!
\brief 	Play memory pin pon buffer initial.
\param	ubSrcNum Numbers of total source channel
\return	(no)
*/
void PLY_SetStartAddr(uint32_t ulStartAdr);
//------------------------------------------------------------------------------
/*!
\brief 	Get current decoded video frame index
\param	ubSrcNum Numbers of total source channel
\return	current decoded video frame index
*/
uint32_t ulPLY_GetDecVdoFrm(uint8_t ubCh);

//------------------------------------------------------------------------------
/*!
\brief 	Set Operation Mode Play flag function
\param	ubEn Set Operation Mode Play flag
\return	(no)
*/
void PLY_SetOpModePly(uint8_t ubEn);

//------------------------------------------------------------------------------
/*!
\brief 	Set Operation Mode Record flag function
\param	ubEn Set Operation Mode Record flag
\return	(no)
*/
void PLY_SetOpModeRec(uint8_t ubEn);

//------------------------------------------------------------------------------
/*!
\brief 	Get Operation Mode function
\return	PLY_MODE_N, Kernel Operation mode is in preview mode.
		PLY_MODE_R, Kernel Operation mode is in preview with recording mode.
		PLY_MODE_PR, Kernel Operation mode is in play with recording mode.
		PLY_MODE_P, Kernel Operation mode is in play mode.
*/
uint8_t ubPLY_GetOpMode(void);

//------------------------------------------------------------------------------
/*!
\brief 	Chk RTC Time Set Ready function
\return	0, RTC Time Setting is not Ready
		1, RTC Time Setting is Ready
*/
uint8_t ubPLY_ChkTimeValid(void);

//------------------------------------------------------------------------------
/*!
\brief	Get Total Play time function
\return	Unit is second
*/
uint32_t ulPLY_GetTotalPlayTime(void);

//------------------------------------------------------------------------------
/*!
\brief 	Get Current Play time function
\return	Unit is second
*/
uint32_t ulPLY_GetSchedule(void);

//------------------------------------------------------------------------------
/*!
\brief 	Set Play Timer Section Number
\param	ubSectionNum Numbers of Section Number,Range 1~100
\return	(no)
*/
void PLY_SetTimeBarSectionNum(uint8_t ubSectionNum);

//------------------------------------------------------------------------------
/*!
\brief 	Get Play timer bar level function
\return	Range = 0-20
*/
uint8_t ubPLY_GetTimeBarLvl(void);

//------------------------------------------------------------------------------
/*!
\brief 	Play start function
\param	ubMode puase at first I frame
\param	ubPathNum Play mode, 1T,2T,3T,4T
\param	ubActiveFileCnt How many file will be play
\param	*pPtr A pointer point to File information structure
\return	0, Run play start function Unsuccessfully
        1, Run play start function successfully
        2, Recording, Not executed play start function
        3, File info Err, Not executed play start function
*/
uint8_t ubPLY_Start(PLY_START_MODE ubMode, uint8_t ubPathNum, uint8_t ubActiveFileCnt, FS_FILE_HIDDEN_INFO_t *pPtr);

//------------------------------------------------------------------------------
/*!
\brief 	Play jump function
\param	ubType PLY_JUMP_FWD,PLY_JUMP_BWD,PLY_JUMP_KEEP
\return	(no)
*/
uint8_t ubPLY_Jump(uint8_t ubType);

//------------------------------------------------------------------------------
/*!
\brief	Play Stop function
\return	0, Run play stop success
		-1, Queue xQueue_Play full
*/
uint8_t ubPLY_Stop(void);

//------------------------------------------------------------------------------
/*!
\brief 	Play pause function
\param	Set Pause Enable, Disable
\return	0, Fail
        1, Successs
*/
uint8_t ubPLY_Pause(PLY_PAUSE_ACT ubAct);

//------------------------------------------------------------------------------
/*!
\brief	Get Play pause status
\return	PLY_PAUSE_OFF, Normal Run
        PLY_PAUSE_ON, Play Pause
*/
PLY_PAUSE_ACT ubPLY_GetPauseStatus(void);

//------------------------------------------------------------------------------
/*!
\brief 	Set Play audio channel
\param	ubCh Source Channel
\return	(no)
*/
uint8_t ubPLY_AdoChannelSet(uint8_t ubCh);

//------------------------------------------------------------------------------
/*!
\brief	Get Play audio channel
\return	Nth Source channel be selected
*/
uint8_t ubPLY_AdoChannelGet(void);

//------------------------------------------------------------------------------
/*!
\brief	Total Path Number. Reference source configuration number at Recording.
\return	Total Path Number at Recording.
*/
uint8_t ubPLY_PathNumberGet(void);

//------------------------------------------------------------------------------
/*!
\brief 	Play Initial function
\param	ubSrcNum Numbers of total source channel
\param	ulStartAdr Input start address
\return	(no)
*/
void PLY_Init(void);

//------------------------------------------------------------------------------
uint8_t ubPLY_GetResolution(uint8_t ubCh);

void PLY_Send1msCountSamephore(void);
uint32_t ubPLY_ThumbnailOpen(FS_SRC_NUM SrcNum, uint32_t ulFirstClus, FS_NO_FAT_CHAIN_FLG NoFatChainFlag, uint32_t ubThmOutputAddr);
void PLY_FPS_Disable(void);
uint8_t PLY_FPS_ChkDisable(void);
#endif


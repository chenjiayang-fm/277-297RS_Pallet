/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		FS_API.h
	\brief		File system API header file
	\author		Chinwei Hsu
	\version    2.58
	\date       2023/8/29
	\copyright	Copyright(C) 2017 SONiX Technology Co.,Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------
#ifndef _FS_API_H_
#define _FS_API_H_

#include "_510PF.h"

#define FS_HIDDEN_FILE_VER			(0x18)

#define FS_FLD_NAME_MAX_LENGTH		(26)
#define FS_FILE_NAME_MAX_LENGTH		(35)

#define MAX_REC_NUM					(4)		// max record number
#define MAX_JPG_NUM					(1)		// jpg
#define MAX_USERDEF_NUM				(1)		// user define file

#define FS_FIXED_FAT32_EN   0
//-----------------------------------------------------------------------------
#pragma pack(push)
#pragma pack(1)
//-----------------------------------------------------------------------------
// FS initial flag
typedef enum
{
	FS_INIT_FAIL = 0,
	FS_INIT_OK
}FS_INIT_STATUS;
//-----------------------------------------------------------------------------
// FS media type
typedef enum
{
	FS_MEDIA_TYPE_SD = 0,
	FS_MEDIA_TYPE_SF
}FS_MEDIA_TYPE;
//-----------------------------------------------------------------------------
// FS media select
typedef enum
{
	FS_MEDIA_0 = 0,
	FS_MEDIA_1,
	FS_MEDIA_MAX
}FS_MEDIA_SEL;
//-----------------------------------------------------------------------------
// resolution access mode
typedef enum
{
	FS_RES_NONE,
	FS_RES_FHD,		//!< 1920x1088
	FS_RES_SXGA,	//!< 1280x1040
	FS_RES_HD,		//!< 1280x720
	FS_RES_XGA,		//!< 1024x768
	FS_RES_WSVGA,	//!< 1024x600
	FS_RES_SVGA,	//!< 800x600
	FS_RES_WVGA,	//!< 800x480
	FS_RES_VGA		//!< 640x480
}FS_RESOLUTION;
//-----------------------------------------------------------------------------
// fs recording data source num, dont modify
typedef enum
{
	FS_VDO_SRC_0 = 0,
	FS_VDO_SRC_1 = 1,
	FS_VDO_SRC_2 = 2,
	FS_VDO_SRC_3 = 3,
	FS_JPG_SRC_0 = 4,
	FS_USERDEF_SRC_0 = 5
}FS_SRC_NUM;
//-----------------------------------------------------------------------------
// fs send queue status
typedef enum
{
	FS_SEND_Q_FAIL = 0,	//!< send queue fail
	FS_SEND_Q_SUCCESS	//!< send queue success
}FS_SEND_Q_STATUS;
//-----------------------------------------------------------------------------
// lock file switch(when recording)
typedef enum
{
	LOCK_FILE_OFF = 0,	//!< lock file off
	LOCK_FILE_ON		//!< lock file on
}FS_LOCK_FILE_SWITCH;
//-----------------------------------------------------------------------------
// looping switch
typedef enum
{
	LOOPING_OFF = 0,	//!< looping off
	LOOPING_ON			//!< looping on
}FS_LOOPING_SWITCH;
//-----------------------------------------------------------------------------
// looping mode
typedef enum
{
	LOOPING_EMG = 0,
	LOOPING_TIMELAPS,
	LOOPING_NORMAL,
	LOOPING_CUST1_VIDEO,
	LOOPING_CUST1_PHOTO,
	LOOPING_MAX
}FS_LOOPING_MODE;
//-----------------------------------------------------------------------------
// fs queue status
typedef enum
{
	FS_NULL = 0,                	//!< fs queue status: initial->null
	FS_REC_CREATE_INI,				//!< fs queue status: create file initial
	FS_REC_CREATE_OK,				//!< fs queue status: create file ok
	FS_REC_CLOSED_INI,				//!< fs queue status: close file initial
	FS_REC_CLOSED_OK,				//!< fs queue status: close file ok
	FS_PLY_READ_INI,				//!< fs queue status: read file initial
	FS_PLY_READ_OK,		    		//!< fs queue status: read file ok
	FS_GET_INFO_INI,	    		//!< fs queue status: get info(folder/file) initial
	FS_GET_INFO_FAIL,	        	//!< fs queue status: get info(folder/file) fail
	FS_GET_INFO_OK,		        	//!< fs queue status: get info(folder/file) ok
	FS_MODIFY_FILE_HIDN_INFO_INI,	//!< fs queue status: modify file hidden info initial
	FS_MODIFY_FILE_HIDN_INFO_OK,	//!< fs queue status: modify file hidden info ok
	FS_MANUAL_DEL_INI,	    		//!< fs queue status: manual delete file initial
	FS_MANUAL_DEL_OK,	    		//!< fs queue status: manual delete file ok
	FS_DEL_FILE_BY_EXTNAME_INI,		//!< fs queue status: delete file by extension name initial
	FS_DEL_FILE_BY_EXTNAME_OK,		//!< fs queue status: delete file by extension name ok
	FS_LOOP_DEL_INI,	    		//!< fs queue status: loop delete file initial
	FS_LOOP_DEL_OK,		        	//!< fs queue status: loop delete file ok
	FS_MOVE_INI,					//!< fs queue status: move file initial
	FS_MOVE_OK,				    	//!< fs queue status: move file ok
}FS_Q_STATUS;
//-----------------------------------------------------------------------------
// media format status
typedef enum
{
	FORMAT_FAIL = 0,	//!< media format fail
	FORMAT_OK			//!< media format ok
}FS_FMT_STATUS;
//-----------------------------------------------------------------------------
// media in/out state
typedef enum 
{
	FS_MEDIA_OUT = 0,	//!< media out
	FS_MEDIA_IN			//!< media in
}FS_MEDIA_INOUT_STATUS;
//------------------------------------------------------------------------------
// media is ready access status
typedef enum
{
	FS_MEDIA_NOT_RDY = 0,	//!< media is not ready to access
	FS_MEDIA_GET_INFO_FAIL,	//!< media is info fail -> need format
	FS_MEDIA_RE_PLUGIN,		//!< media is info fail -> please re-plugin media
	FS_MEDIA_RDY			//!< media is ready to access
}FS_MEDIA_RDY_ACC;
//-----------------------------------------------------------------------------
// file attribute
typedef enum
{
	FILE_ATTR_READ_ONLY = 0x01,	//!< file is read-only
	FILE_ATTR_HIDDEN    = 0x02,	//!< file is hidden
	FILE_ATTR_ARCHIVE	= 0x20,	//!< file is archive
	FILE_ATTR_DIRECTORY = 0x10	//!< file is directory
}FS_FILE_ATTR;
//-----------------------------------------------------------------------------
// file create path
typedef enum
{
	FILE_PATH_NOSUPPORT = 0,	//!< system no support path
	FILE_PATH_DEFAULT,			//!< system path -> E:\DCIM\100SONIX
	FILE_PATH1,					//!< system path -> E:
	FILE_PATH2,					//!< system path -> E:\DCIM
	FILE_PATH3,					//!< system path -> E:\DCIM\EMG
	FILE_PATH4,					//!< system path -> E:\DCIM\TIMELAPS
	FILE_PATH5,					//!< customer1 path(video) -> E:\DCIM\VIDEO
	FILE_PATH6					//!< customer1 path(photo) -> E:\DCIM\PHOTO
}FS_FILE_PATH;
//-----------------------------------------------------------------------------
// media speed test report
typedef enum
{
	FS_MEDIA_FAIL,	//!< media speed test fail
	FS_MEDIA_PASS	//!< media speed test pass
}FS_MEDIA_SPD_RPT;
//-----------------------------------------------------------------------------
// File system function switch
typedef enum
{
	FS_OFF,
	FS_ON
}FS_SWITCH;
//-----------------------------------------------------------------------------
// SD card size
typedef enum 
{
	FS_CARD_SIZE_OTHERS = 0,
	FS_CARD_SIZE_256MB = 256,
	FS_CARD_SIZE_512MB = 512,
	FS_CARD_SIZE_1GB = 1024,
	FS_CARD_SIZE_2GB = 2048,
	FS_CARD_SIZE_4GB = 4096,
	FS_CARD_SIZE_8GB = 8192,
	FS_CARD_SIZE_16GB = 16384,
	FS_CARD_SIZE_32GB = 32768,
	FS_CARD_SIZE_64GB = 65536,
	FS_CARD_SIZE_128GB = 131072
}FS_CARD_SIZE;
//------------------------------------------------------------------------------
typedef enum
{
	RF_Send_VdoData_To_REC,
	RF_Send_AdoData_To_REC,
	
	REC_Recv_VdoData_From_RF,
	REC_Recv_AdoData_From_RF,
	
	REC_Send_Data_To_FS,
	FS_Recv_FrameSz_From_REC,
	
	FS_StreamBuf_Pointer,	/* ulCntIn, ulCntOut, ulNotYetWrtCnt, ulFullCnt */
	
	Remain_Qnum_FS_CreateFile,
	Remain_Qnum_FS_WriteFile,
	Remain_Qnum_FS_CloseFile,
	
	FS_Operation_FS_CreateFileProcess_start,
	FS_Operation_FS_WriteFileProcess_start,
	FS_Operation_FS_CloseFileProcess_start,
	
	FS_Operation_FS_CreateFileProcess_end,
	FS_Operation_FS_WriteFileProcess_end,
	FS_Operation_FS_CloseFileProcess_end,
		
	Media_Write_Start,	/* LBA, size */
	Media_Write_End,	/* write time */
	
	Media_Read_Start,	/* LBA, size */
	Media_Read_End,		/* read time */
	
	Media_Erase_Start,	/* address, size */
	Media_Erase_End		/* erase time */
}FS_DUMP_DBG_LOG_INFO_IDX;
//-----------------------------------------------------------------------------
// get folder/file info mode
typedef enum
{
	A_SINGLE_INFO = 0,	//!< only get a single information
	A_SERIES_INFO		//!< get a series of information
}FS_GET_FLD_FILE_INFO_MODE;
//-----------------------------------------------------------------------------
// folder structure type
typedef enum
{
	STANDARD_DCIM = 0,			//!< DCIM\100SONIX...
	DCIM_WITH_VDO_PHOTO_FLD,	//!< DCIM\VIDEO, DCIM\PHOTO
}FS_FLD_STRC_MODE;
//-----------------------------------------------------------------------------
// buffer write threshold
typedef enum
{
	FS_BUF_WRT_TH_128KB = 0x20000,
	FS_BUF_WRT_TH_256KB = 0x40000,
	FS_BUF_WRT_TH_512KB = 0x80000
}FS_BUF_WRT_TH;
//-----------------------------------------------------------------------------
// Fat chain valid flag
typedef enum 
{
	FAT_CHAIN_VALID = 0,
	FAT_CHAIN_INVALID,
	FAT_CHAIN_NULL
}FS_NO_FAT_CHAIN_FLG;
//-----------------------------------------------------------------------------
// buffer write threshold
typedef enum
{
    FS_TZ_NONE      = 0x00,             //!< None Supported
	FS_TZ_GMT       = 0x80,             //!< UTC=GMT, Greenwich Standard Time
	FS_TZ_CET       = 0x84,             //!< UTC+01:00, Central Europe Time
	FS_TZ_EEST      = 0x88,             //!< UTC+02:00, Eastern Europe Standard Time
	FS_TZ_MST       = 0x8C,             //!< UTC+03:00, Moscow Standard Time
	FS_TZ_AST       = 0x90,             //!< UTC+04:00, Arabian Standard Time
	FS_TZ_WAST      = 0x94,             //!< UTC+05:00, West Asia Standard Time
	FS_TZ_CAST      = 0x98,             //!< UTC+06:00, Central Asia Standard Time
	FS_TZ_NAST      = 0x9C,             //!< UTC+07:00, North Asia Standard Time
	FS_TZ_NAEST     = 0xA0,             //!< UTC+08:00, North Asia East Standard Time,GMT+08:00
	FS_TZ_TOKYOST   = 0xA4,             //!< UTC+09:00, Tokyo Standard Time
    FS_TZ_WPST      = 0xA8,             //!< UTC+10:00, West Pacific Standard Time
    FS_TZ_CPST      = 0xAC,             //!< UTC+11:00, Central Pacific Standard Time
    FS_TZ_NZST      = 0xB0,             //!< UTC+12:00, New Zealand Standard Time
    FS_TZ_TONGAST   = 0xB4,             //!< UTC+13:00, Tonga Standard Time
    FS_TZ_DST       = 0xD0,             //!< UTC-12:00, Dateline Standard Time
    FS_TZ_SST       = 0xD4,             //!< UTC-11:00, Samoa Standard Time
    FS_TZ_HST       = 0xD8,             //!< UTC-10:00, Hawaii Standard Time
    FS_TZ_ALASKAST  = 0xDC,             //!< UTC-09:00, Alaska Standard Time
    FS_TZ_PST       = 0xE0,             //!< UTC-08:00, Pacific Standard Time
    FS_TZ_MOUNTST   = 0xE4,             //!< UTC-07:00, Mountain Standard Time
    FS_TZ_CST       = 0xE8,             //!< UTC-06:00, Central Standard Time
    FS_TZ_EST       = 0xEC,             //!< UTC-05:00, Eastern Standard Time
    FS_TZ_ATLANTST  = 0xF0,             //!< UTC-04:00, Atlantic Standard time
    FS_TZ_NST       = 0xF2,             //!< UTC-03:30, Newfoundland Standard Time
    FS_TZ_GREENLANDST = 0xF4,           //!< UTC-03:00, Greenland Standard Time
    FS_TZ_MAST      = 0xF8,             //!< UTC-02:00, Mid-Atlantic Standard Time
    FS_TZ_AZORESST  = 0xFC              //!< UTC-01:00, Azores Standard Time
}FS_EXFAT_TIMEZONE;
//-----------------------------------------------------------------------------
// folder name information
typedef struct FS_FOLDER_NAME_INFORMATION
{
	char chName[FS_FLD_NAME_MAX_LENGTH+1];	//!< folder name
	uint8_t ubLen;							//!< folder name length
}FS_FLD_NAME_INFO_t;
//-----------------------------------------------------------------------------
// file name information
typedef struct FS_FILE_NAME_INFORMATION
{
	char chName[FS_FILE_NAME_MAX_LENGTH+1];	//!< file name
	uint8_t ubLen;							//!< file name length
	char chExt[3+1];						//!< file extension name
	uint8_t ubReserved[3];					//!< used for meeting a multiple of 4
}FS_FILE_NAME_INFO_t;
//-----------------------------------------------------------------------------
// Time info
typedef struct FS_CALENDAR_INFORMATION
{
	uint16_t uwYear;	//!< year
	uint8_t ubMonth;	//!< month
	uint8_t ubDay;		//!< day
	uint8_t ubHour;		//!< hour
	uint8_t ubMin;		//!< minute
	uint8_t ubSec;		//!< second
	uint8_t ubReserved[1];	//!< used for meeting a multiple of 4
}FS_CALENDAR_t;
//-----------------------------------------------------------------------------
// for kernal call -> create process information
typedef struct FS_KNL_CREATE_PROCESS
{
	FS_MEDIA_SEL MediaSel;
	
	FS_SRC_NUM SrcNum;
	
	uint8_t ubRecGroupFileNum;	//!< how many files in this record group
	
	FS_FILE_NAME_INFO_t FileName;
	
	FS_FILE_ATTR FileAttr;
	FS_FILE_PATH FilePath;
	
	uint16_t uwGroupIdx;	//!< group index
}FS_KNL_CRE_PROCESS_t;
//-----------------------------------------------------------------------------
// for kernal call -> sub hidden info information
typedef struct FS_SUB_HIDDEN_INFORMATION
{
	uint8_t ubEvent;			//!< event number
	uint8_t ubPreviewMode;		//!< preview mode
    uint8_t ubSrcLocate[4];		//!< source location, this represents the video/photo will show where it is, ex: ubSrcLocate[ location ] = source number
	uint16_t uwRes_HSize;		//!< H size of resolution
	uint16_t uwRes_VSize;		//!< V size of resolution
	uint16_t uwMovieLen;		//!< movie length
    uint8_t ubActiveFileNum;	//!< active file number
    uint8_t ubDispMode;			//!< display mode
    uint8_t ubFileDL;			//!< File Download Flag
    uint8_t ubReserved[9];	    //!< used for meeting a multiple of 4
}FS_FILE_SUB_HIDN_INFO_t;
//-----------------------------------------------------------------------------
// file hidden information
typedef struct FS_FILE_HIDDEN_INFO
{
	uint8_t ubVer;		//!< used for checking if this hidden info enable or not; ubVer!=0 => enable; ubVer=0 => disable
	
	FS_FLD_NAME_INFO_t FldName;		//!< size=28
	FS_FILE_NAME_INFO_t FileName;	//!< size=44
	
	uint32_t ulRtcLongTime;
	FS_CALENDAR_t CreTime;		//!< < size=8
	uint8_t ubCreTimeSecOfs;	//!< second offset of creating time(0 or 1)
	
	uint32_t ulFirstClus;	
	uint64_t ullFileSize;	//!< file size, unit:bytes
	
	FS_NO_FAT_CHAIN_FLG NoFatChainFlag;	//!<Fat chain valid flag
	
	FS_SRC_NUM SrcNum;
	FS_FILE_PATH FilePath;	
	uint16_t uwGroupIdx;
	uint8_t ubFileLockBit;	//!< file lock bit; 1:lock, 0:unlock
	
	FS_FILE_SUB_HIDN_INFO_t SubHidnInfo;	//!< size=24
}FS_FILE_HIDDEN_INFO_t;
//----------------------------------------------------------------------------- 
// file information
typedef struct FS_FILEINFO
{
	FS_FILE_NAME_INFO_t FileName;
	
	uint8_t ubFileAttr;
	
	FS_CALENDAR_t CreTime;
	
	uint32_t ulFirstClus;
	
	uint64_t ullFileSize;
	
	FS_NO_FAT_CHAIN_FLG NoFatChainFlag;
	
	FS_FILE_PATH FilePath;
	
	uint8_t ubReserved[1];	//!< used for meeting a multiple of 4
}FS_FILE_INFO_t;
//-----------------------------------------------------------------------------
// folder information
typedef struct FS_FLD_INFO
{
	FS_FLD_NAME_INFO_t FldName;
	
	uint8_t ubFileAttr;
	
	FS_CALENDAR_t CreTime;
	
	uint32_t ulFirstClus;
	
	FS_FILE_PATH FilePath;
	
	uint8_t ubReserved[2];	//!< used for meeting a multiple of 4
}FS_FLD_INFO_t;
//-----------------------------------------------------------------------------
// FS get a single file info
typedef struct FS_GET_A_SINGLE_FILE_INFORMATION
{
	FS_FILE_HIDDEN_INFO_t *OutputHidnFileInfo;
	FS_FILE_INFO_t *OutputFileInfo;
	FS_FLD_NAME_INFO_t FldName;
	FS_FILE_NAME_INFO_t FileName;
}FS_GET_A_SINGLE_FILE_INTO_t;
//-----------------------------------------------------------------------------
// FS get a series of file info
typedef struct FS_GET_A_SERIES_FILE_INFORMATION
{
	FS_FILE_HIDDEN_INFO_t *OutputHidnFileInfo;
	FS_FILE_INFO_t *OutputFileInfo;
	uint32_t *OutputValidNum;
	FS_FLD_NAME_INFO_t FldName;
	uint32_t ulOffset;	//!< read file offset: 0,1,2,...
	uint32_t ulNum;		//!< read file number
}FS_GET_A_SERIES_FILE_INTO_t;
//-----------------------------------------------------------------------------
// FS get a single folder info
typedef struct FS_GET_A_SINGLE_FLD_INFORMATION
{
	FS_FLD_INFO_t *OutputFldInfo;
	FS_FLD_NAME_INFO_t FldName;
}FS_GET_A_SINGLE_FLD_INTO_t;
//-----------------------------------------------------------------------------
// FS get a series of folder info
typedef struct FS_GET_A_SERIES_FLD_INFORMATION
{
	FS_FLD_INFO_t *OutputFldInfo;
	uint32_t *OutputValidNum;
	uint32_t ulOffset;	//!< read folder offset: 0,1,2,...
	uint32_t ulNum;		//!< read folder number
}FS_GET_A_SERIES_FLD_INTO_t;
//-----------------------------------------------------------------------------
// for kernal call -> manual delete file process information
typedef struct FS_KNL_MANUAL_DEL_PROCESS
{
	FS_MEDIA_SEL MediaSel;
	
	FS_FLD_NAME_INFO_t FldName;
	
	FS_FILE_NAME_INFO_t FileName;
	
	FS_FILE_PATH FilePath;
}FS_KNL_MANUAL_DEL_PROCESS_t;
//-----------------------------------------------------------------------------
// for kernal call -> delete file by ext name process information
typedef void(*FS_DelFileByExtNameHook)(uint8_t ubPercent);
typedef struct FS_KNL_DEL_FILE_BY_EXTNAME_PROCESS
{
	FS_MEDIA_SEL MediaSel;
	
	FS_FLD_NAME_INFO_t FldName;
		
	char chFileExtName[3+1];	//!< file extension name
	
	FS_FILE_PATH FilePath;
	
	FS_DelFileByExtNameHook CbFunc;
}FS_KNL_DEL_FILE_BY_EXTNAME_PROCESS_t;
//-----------------------------------------------------------------------------
// for kernal call -> move file process information
typedef struct FS_KNL_MOVE_PROCESS
{
	FS_MEDIA_SEL MediaSel;
	
	FS_FILE_PATH SrcFilePath;			//!< source file delete path
	FS_FLD_NAME_INFO_t SrcFldName;		//!< source folder name
	FS_FILE_NAME_INFO_t SrcFileName;	//!< source file name
	
	FS_FILE_PATH DstFilePath;			//!< destination file delete path
	FS_FLD_NAME_INFO_t DstFldName;		//!< destination folder name
	FS_FILE_NAME_INFO_t DstFileName;	//!< destination file name
}FS_KNL_MOVE_PROCESS_t;
//-----------------------------------------------------------------------------
typedef struct FS_DUMP_DBG_LOG
{
	uint32_t ul1msCnt;
	uint8_t ubSrcNum;
	FS_DUMP_DBG_LOG_INFO_IDX InfoIdx;
	uint32_t ulValue[4];
	uint8_t ubReserved[10];	//!< used for meeting 32bytes
}FS_DUMP_DBG_LOG_t;
//-----------------------------------------------------------------------------
typedef struct FS_DUMP_DBG_LOG_LIB_VERSION
{
	uint8_t ubREC_Lib_Ver[2];	//!< [0]:MAJOR version; [1]:MINOR version
	uint8_t ubPLY_Lib_Ver[2];	//!< [0]:MAJOR version; [1]:MINOR version
}FS_DUMP_DBG_LOG_LIB_VER_t;
//-----------------------------------------------------------------------------
typedef struct FS_VIDEO_STREAM_BUFFER_INFORMAION
{
	FS_SWITCH BufEnable;				//!< buffer enable
	uint8_t ubNum;						//!< total buffer number
	FS_RESOLUTION Res[MAX_REC_NUM];		//!< resolution
	uint32_t ulSize[MAX_REC_NUM];		//!< buffer size, unit: bytes
	uint32_t ulDataRate[MAX_REC_NUM];	//!< unit: KBytes/s
}FS_VDO_STREAM_BUF_INFO_t;				//!< allocated data rate, only used for media read/write test
//-----------------------------------------------------------------------------
typedef struct FS_PHOTO_STREAM_BUFFER_INFORMAION
{
	FS_SWITCH BufEnable;	//!< buffer enable
	uint32_t ulSize;		//!< buffer size, unit: bytes
}FS_JPG_STREAM_BUF_INFO_t;
//-----------------------------------------------------------------------------
typedef struct FS_USER_DEFINE_STREAM_BUFFER_INFORMAION
{
	FS_SWITCH BufEnable;	//!< buffer enable
	uint32_t ulSize;		//!< buffer size, unit: bytes
}FS_USERDEF_STREAM_BUF_INFO_t;
//-----------------------------------------------------------------------------
typedef struct FS_STREAM_BUFFER_INFORMAION
{
	FS_VDO_STREAM_BUF_INFO_t Vdo;			//!< video stream buffer setting
	FS_JPG_STREAM_BUF_INFO_t Jpg;			//!< photo stream buffer setting
	FS_USERDEF_STREAM_BUF_INFO_t UserDef;	//!< user define stream buffer setting
}FS_STREAM_BUF_INFO_t;
//-----------------------------------------------------------------------------
/*!
\brief FS media exist status hook
\return 0:media is out, 1: media is in
*/
typedef uint8_t(*FS_MediaExistHook)(void);
//-----------------------------------------------------------------------------
/*!
\brief FS media check interface setup status hook
\return 0:fail, 1:pass
*/
typedef uint8_t(*FS_MediaIFSetupStatusHook)(void);
//-----------------------------------------------------------------------------
/*!
\brief FS media identify hook
\return media total sectors(unit:sectors; 1 sector=512bytes), return 0 is fail
*/
typedef uint32_t(*FS_MediaIdentifyHook)(void);
//-----------------------------------------------------------------------------
/*!
\brief FS media plug-in/out hook
\return(no)
*/
typedef void(*FS_MediaPlugInOutHook)(void);
//-----------------------------------------------------------------------------
/*!
\brief FS media read hook
\param uint32_t		source dram address
\param uint32_t		destination LBA
\param uint32_t		source dram size
\return 0:fail, 1:pass
*/
typedef uint8_t(*FS_MediaWriteHook)(uint32_t, uint32_t, uint32_t);
//-----------------------------------------------------------------------------
/*!
\brief FS media write hook
\param uint32_t		destination dram address
\param uint32_t		source LBA
\param uint32_t		source LBA size
\return 0:fail, 1:pass
*/
typedef uint8_t(*FS_MediaReadHook)(uint32_t, uint32_t, uint32_t);
//-----------------------------------------------------------------------------
/*!
\brief FS media erase hook
\param uint8_t		erase mode
\param uint32_t		source LBA
\param uint32_t		source LBA size
\return 0:fail, 1:pass
*/
typedef uint8_t(*FS_MediaEraseHook)(uint8_t, uint32_t, uint32_t);
//-----------------------------------------------------------------------------
// FS parameter for kernal setting
//-----------------------------------------------------------------------------
typedef struct KNL_FS_SUB_PARAMETER
{
	//------------------------------------------------------------------------------------------------------------------------------------------------------------
	FS_MEDIA_TYPE MediaType;		//!< media type
	//------------------------------------------------------------------------------------------------------------------------------------------------------------
	uint32_t ulSF_TotalSz;		//!< SF total size(unit:Bytes); it's necessary if MediaType=FS_MEDIA_TYPE_SF
	uint32_t ulSF_UseStartOfs;	//!< SF write start offset(unit:Bytes); it's necessary if MediaType=FS_MEDIA_TYPE_SD and will write FAT start from this
	uint32_t ulSF_UseSz;		//!< SF used size(unit:Bytes); it's necessary if MediaType=FS_MEDIA_TYPE_SF and will use this size start from ulSF_UsedStartOfs
	//------------------------------------------------------------------------------------------------------------------------------------------------------------
	FS_MediaExistHook MediaExist;			//!< callback function of media exist
	FS_MediaIFSetupStatusHook MediaIFSetup;	//!< callback function of media interface setup status
	FS_MediaIdentifyHook MediaIdentify;		//!< callback function of media identify
	FS_MediaPlugInOutHook MediaPlugInHook;	//!< callback function of media plug-in hook function(executing after media is ready)
	FS_MediaPlugInOutHook MediaPlugOutHook;	//!< callback function of media plug-out hook function
	FS_MediaWriteHook MediaWrite;			//!< callback function of media write
	FS_MediaReadHook MediaRead;				//!< callback function of media read
	FS_MediaEraseHook MediaErase;			//!< callback function of media erase
	uint8_t ubMediaEreseMode_SE;			//!< callback function of media erase mode:SE(4KB only)
	uint8_t ubMediaEreseMode_LBE;			//!< callback function of media erase mode:LBE(64KB only)
	//------------------------------------------------------------------------------------------------------------------------------------------------------------
	uint32_t ulMediaInDebounceNum;	//!< check times per 20ms, num=1,2,3,... 
	uint32_t ulMediaOutDebounceNum;	//!< check times per 20ms, num=1,2,3,... 
	//------------------------------------------------------------------------------------------------------------------------------------------------------------
	FS_BUF_WRT_TH BufWrtTh;			//!< buffer write threshold
	FS_STREAM_BUF_INFO_t StreamBuf;	//!< stream buffer setting, every buffer size is must larger than BufWrtTh and is multiple of 0x8000Bytes
	//------------------------------------------------------------------------------------------------------------------------------------------------------------
	uint16_t uwMaxRollingGroupIdxNum;	//!< if value=65535, the range of group idx is 0~65534
	//------------------------------------------------------------------------------------------------------------------------------------------------------------
	FS_FLD_STRC_MODE FldStrcMode;				//!< folder sturcture mode
	//------------------------------------------------------------------------------------------------------------------------------------------------------------
	uint32_t ulRecFldMaxSupFileNum;				//!< max support file num(max=1000) in recording folder(without emergency and timelapse folder)
	//------------------------------------------------------------------------------------------------------------------------------------------------------------
	char chDefRecFldName[5];					//!< based on FldStrucMode=STANDARD_DCIM, default rec folder name (without rolling num->100/101...)
												//!< if fld name is 100SONIX, then fill SONIX in it
	//------------------------------------------------------------------------------------------------------------------------------------------------------------
	FS_FLD_NAME_INFO_t Cust1_DefVideoFldName;	//!< based on FldStrucMode=DCIM_WITH_VDO_PHOTO_FLD, default customer1 rec video folder name, ex:VIDEO
	FS_FLD_NAME_INFO_t Cust1_DefPhotoFldName;	//!< based on FldStrucMode=DCIM_WITH_VDO_PHOTO_FLD, default customer1 rec photo folder name, ex:PHOTO
	//------------------------------------------------------------------------------------------------------------------------------------------------------------
	FS_SWITCH EmgFuncEnable;				//!< emergency folder enable
	uint32_t ulEmgFldMaxSupFileNum;			//!< max support file num(max=1000) in emergency folder
	FS_FLD_NAME_INFO_t DefEmgFldName;		//!< emergency folder name
	//------------------------------------------------------------------------------------------------------------------------------------------------------------
	FS_SWITCH TimelapseFuncEnable;			//!< timelapse folder enable
	uint32_t ulTimelapseFldMaxSupFileNum;	//!< max support file num(max=1000) in timelapse folder
	FS_FLD_NAME_INFO_t DefTimelapseFldName;	//!< default timelapse folder name
	//------------------------------------------------------------------------------------------------------------------------------------------------------------
	char chDefRecFileName[FS_FILE_NAME_MAX_LENGTH-4];	//!< default rec file name (without rolling num), if file name is SNX_0001, then fill SNX_ in it
	uint8_t ubDefRecFileNameLen;						//!< default file name length (without rolling num)
	//------------------------------------------------------------------------------------------------------------------------------------------------------------
}FS_KNL_SUB_PARA_t;
//-----------------------------------------------------------------------------
/*!
\brief FS get ddr capacity hook
\return ddr size, unit:bytes, return 0 represets fail
*/
typedef uint32_t(*FS_GetDdrCapacityHook)(void);
//-----------------------------------------------------------------------------
/*!
\brief FS dma fill hook
\param uint32_t		destination address
\param uint32_t		fill pattern
\param uint32_t		fill size
\return(no)
*/
typedef void(*FS_DmaFillHook)(uint32_t, uint32_t, uint32_t);
//-----------------------------------------------------------------------------
/*!
\brief FS dma copy hook
\param uint32_t		destination address
\param uint32_t		source address
\param uint32_t		size
\return(no)
*/
typedef void(*FS_DmaCopyHook)(uint32_t, uint32_t, uint32_t);
//-----------------------------------------------------------------------------
/*!
\brief FS get calendar hook
\return FS_CALENDAR_t
*/
typedef FS_CALENDAR_t(*FS_GetCalendarHook)(void);
//-----------------------------------------------------------------------------
/*!
\brief FS get 1ms count hook
\return 1ms count
*/
typedef uint32_t(*FS_Get1msHook)(void);
//-----------------------------------------------------------------------------
/*!
\brief FS get RTC long time hook
\return RTC long time
*/
typedef uint32_t(*FS_GetRtcLongTimeHook)(void);
//-----------------------------------------------------------------------------
/*!
\brief FS 32-bit fat search hook
\param uint32_t		target data of searching
\param uint32_t		start address of searching
\param uint32_t		search length
\param uint32_t		report address
\return 0xFFFFFFFF:fail, otherwise:report length
*/
typedef uint32_t(*FS_FatSearchHook)(uint32_t, uint32_t, uint32_t, uint32_t);
//-----------------------------------------------------------------------------
/*!
\brief FS watch dog timer clear hook
\return(no)
*/
typedef void(*FS_WdtTimerClrHook)(void);
//-----------------------------------------------------------------------------
/*!
\brief FS CRC32 hook
\param uint32_t		target data of searching
\param uint32_t		start address of searching
\return 32-bit crc value
*/
typedef uint32_t(*FS_Crc32Hook)(uint32_t, uint32_t);
//-----------------------------------------------------------------------------
/*!
\brief FS store debug info for kernal hook
\param const char*	function name
\param uint32_t		line(the position in function)
\return(no)
*/
typedef void(*FS_StoreKnlDbgInfoHook)(const char*, uint32_t);
//-----------------------------------------------------------------------------
/*!
\brief FS force stopping write file hook
\return(no)
*/
typedef void(*FS_ForceStoppingWrtFileHook)(void);
//-----------------------------------------------------------------------------
typedef struct KNL_FS_PARAMETER
{
	//---------------------------------------------------------------------------------------------------------------------------
	uint8_t ubMediaNum;								//!< media number, min=1, max=FS_MEDIA_MAX
	uint8_t ubFixedFat32;                           //!< Only for storage size >= 50GB
	uint8_t ubTimeZone;                             //!< Only active at exfat
	FS_KNL_SUB_PARA_t MediaInfo[FS_MEDIA_MAX];		//!< media information
	uint32_t ulHiddenInfOffset;
	uint32_t ulHiddenInfSize;	
	//---------------------------------------------------------------------------------------------------------------------------
	uint32_t ulBufStartAddr;	//!< FS buffer start address
	//---------------------------------------------------------------------------------------------------------------------------
	FS_GetDdrCapacityHook DdrCapacity;		//!< callback function of getting ddr capacity
	FS_DmaFillHook DmaFill;					//!< callback function of DMA fill
	FS_DmaCopyHook DmaCopy;					//!< callback function of DMA copy
	FS_Get1msHook Get1ms;					//!< callback function of getting 1ms
	FS_GetCalendarHook RtcCalendar;			//!< callback function of RTC calendar
	FS_GetRtcLongTimeHook GetRtcLongTime;	//!< callback function of getting RTC long time
	FS_FatSearchHook FatSearch;				//!< callback function of fat search
	FS_WdtTimerClrHook WdtTimerClr;			//!< callback function of watch dog timer clear
	FS_Crc32Hook Crc32;						//!< callback function of CRC32
	//---------------------------------------------------------------------------------------------------------------------------
	FS_SWITCH DumpDbgLogEnable;						//!< dump debug log function enable
	FS_DUMP_DBG_LOG_LIB_VER_t DumpDbgLogLibVer;		//!< dump debug log lib version
	//---------------------------------------------------------------------------------------------------------------------------
	FS_StoreKnlDbgInfoHook StoreKnlDbgInfoHook;		//!< store debug info for kernal
	//---------------------------------------------------------------------------------------------------------------------------
	FS_ForceStoppingWrtFileHook ForceStoppingWrtFileHook;	//!< force stopping write file
	//---------------------------------------------------------------------------------------------------------------------------
}FS_KNL_PARA_t;
//------------------------------------------------------------------------------
// emergency file information
typedef struct FS_EMERGENCY_FILE_INFO
{
	FS_FLD_NAME_INFO_t FldName;
	
	FS_FILE_NAME_INFO_t FileName;
	
	FS_CALENDAR_t CreTime;
	uint8_t ubCreTimeSecOfs;	//!< create time second offset(0 or 1)
	
	uint16_t uwGroupIdx;	//!< group index
    
    FS_SRC_NUM SrcNum;	//!< sorce number
}FS_EMERG_FILE_INFO_t;
//-----------------------------------------------------------------------------
// FS write file buffer content mofify
typedef struct FS_WRT_BUF_CONTENT_MODIFY
{
	uint32_t ulStartOfs;	//!< the content offset that user want to modify
	uint32_t ulSrcAddr;		//!< the content address that user want to modify
	uint32_t ulSize;		//!< the content size that user want to modify 
}FS_WRT_BUF_CNT_MOD_t;
//------------------------------------------------------------------------------
// FS boot sector information
typedef struct FS_BOOTSECTOR_INFORMATION
{
	FS_MEDIA_TYPE MediaType;
	
	uint32_t ulSF_TotalSz;
	uint32_t ulSF_UseStartOfs;
	uint32_t ulSF_UseSz;
	
	uint32_t ulBytePerSec;
	uint32_t ulSecPerClus;
	uint32_t ulClusSz;
	uint32_t ulSecPerFat;
	
	uint32_t ulFat1Lba;
	uint32_t ulFat2Lba;
	uint32_t ulRootLba;
	uint32_t ulDataLba;
}FS_BOOTSECTOR_INTO_t;
//-----------------------------------------------------------------------------
// FS read/write test information
typedef struct FS_READ_WRITE_TEST_INFORMATION
{
	uint32_t ulTargetDataRate;		//!< unit:KBytes/s
	FS_VDO_STREAM_BUF_INFO_t Vdo;	//!< video stream buffer information
}FS_RW_TEST_INTO_t;
//-----------------------------------------------------------------------------
#pragma pack(pop)
//-----------------------------------------------------------------------------

//==============================================================================
// FS External API
//==============================================================================
/*!
\brief FS version
\return ((FS_MAJORVER << 8) + FS_MINORVER)
*/
uint16_t uwFS_GetVersion(void);
//-----------------------------------------------------------------------------
/*!
\brief create file
\param KnlCreProc 	parameter of create information
\return FS_SEND_Q_STATUS
*/
FS_SEND_Q_STATUS FS_CreateFile(FS_KNL_CRE_PROCESS_t *KnlCreProc);
//-----------------------------------------------------------------------------
/*!
\brief write file
\param MediaSel			media select
\param SrcNum 			source number
\param ulSrcDramAddr 	source dram address
\param ulSize 			data size
\param ModCnt 			the buffer content that user want to modify
\return FS_SEND_Q_STATUS
*/
FS_SEND_Q_STATUS FS_WriteFile(FS_MEDIA_SEL MediaSel, FS_SRC_NUM SrcNum, uint32_t ulSrcDramAddr, uint32_t ulSize, FS_WRT_BUF_CNT_MOD_t *ModCnt);
//-----------------------------------------------------------------------------
/*!
\brief close file
\param MediaSel		media select
\param SrcNum		source number
\param HidnInfo		hidden info, if you dont update hidden info, just fill it NULL
\return FS_SEND_Q_STATUS
*/
FS_SEND_Q_STATUS FS_CloseFile(FS_MEDIA_SEL MediaSel, FS_SRC_NUM SrcNum, FS_FILE_SUB_HIDN_INFO_t *SubHidnInfo);
//-----------------------------------------------------------------------------
/*!
\brief update avi part-1 header(avi size and hdrl header); data range:0~499bytes
\param MediaSel			media select
\param SrcNum 			source number
\param ulSrcDramAddr 	source dram address
\param ulSize 			data size
\return FS_SEND_Q_STATUS
*/
FS_SEND_Q_STATUS FS_UpdateAviHeader1(FS_MEDIA_SEL MediaSel, FS_SRC_NUM SrcNum, uint32_t ulSrcDramAddr, uint32_t ulSize);
//-----------------------------------------------------------------------------
/*!
\brief update avi part-2 header(movi size); data range:66548(avi header1 size+hidden info size+thumbnail size)~66559bytes
\param MediaSel			media select
\param SrcNum 			source number
\param ulSrcDramAddr 	source dram address
\param ulSize 			data size
\return FS_SEND_Q_STATUS
*/
FS_SEND_Q_STATUS FS_UpdateAviHeader2(FS_MEDIA_SEL MediaSel, FS_SRC_NUM SrcNum, uint32_t ulSrcDramAddr, uint32_t ulSize);
//-----------------------------------------------------------------------------
/*!
\brief update mp4 header
\param MediaSel			media select
\param SrcNum 			source number
\param ulSrcDramAddr	source dram address
\param ulSize			data size
\return FS_SEND_Q_STATUS
*/
FS_SEND_Q_STATUS FS_UpdateMp4Header(FS_MEDIA_SEL MediaSel, FS_SRC_NUM SrcNum, uint32_t ulSrcDramAddr, uint32_t ulSize);
//-----------------------------------------------------------------------------
/*!
\brief read file data
\param MediaSel				media select
\param ulDestDramAddr		destination dram address
\param SrcNum 				source number
\param ulFirstClus 			first cluster of file
\param NoFatChainFlag		no fat chain flag
\param ullReadAddr			read data address->the actual data offset
\param ulSize 				read size(the maximum limit is 16MB)
\return FS_SEND_Q_STATUS
*/
FS_SEND_Q_STATUS FS_ReadFile(FS_MEDIA_SEL MediaSel, uint32_t ulDestDramAddr, FS_SRC_NUM SrcNum, uint32_t ulFirstClus, FS_NO_FAT_CHAIN_FLG NoFatChainFlag, uint64_t ullReadAddr, uint32_t ulSize);
//-----------------------------------------------------------------------------
/*!
\brief get file hidden information
\param MediaSel			media select
\param GetInfoMode		get info mode
\param SingleInfo		Needed when searching a single file info
\param SeriesInfo		Needed when searching a series of file info
\return FS_SEND_Q_STATUS
*/
FS_SEND_Q_STATUS FS_GetFileHiddenInfo(FS_MEDIA_SEL MediaSel, FS_GET_FLD_FILE_INFO_MODE GetInfoMode, FS_GET_A_SINGLE_FILE_INTO_t SingleInfo, FS_GET_A_SERIES_FILE_INTO_t SeriesInfo, FS_FILE_PATH FilePath);
//-----------------------------------------------------------------------------
/*!
\brief modify file hidden information
\param MediaSel				media select
\param NewFileHidnInfo		new file hidden info
\param ulFirstClus 			first cluster of file
\return FS_Q_UPDATE_INFO_STATUS
*/
FS_SEND_Q_STATUS FS_ModifyFileHiddenInfo(FS_MEDIA_SEL MediaSel, FS_FILE_HIDDEN_INFO_t *NewFileHidnInfo, uint32_t ulFirstClus);
//-----------------------------------------------------------------------------
/*!
\brief get file information
\param MediaSel			media select
\param GetInfoMode		get info mode
\param SingleInfo		Needed when searching a single file info
\param SeriesInfo		Needed when searching a series of file info
\param FilePath			data path
\return FS_SEND_Q_STATUS
*/
FS_SEND_Q_STATUS FS_GetFileInfo(FS_MEDIA_SEL MediaSel, FS_GET_FLD_FILE_INFO_MODE GetInfoMode, FS_GET_A_SINGLE_FILE_INTO_t SingleInfo, FS_GET_A_SERIES_FILE_INTO_t SeriesInfo, FS_FILE_PATH FilePath);
//-----------------------------------------------------------------------------
/*!
\brief get folder information
\param MediaSel			media select
\param GetInfoMode		get info mode
\param SingleInfo		Needed when searching a single file info
\param SeriesInfo		Needed when searching a series of file info
\param FilePath			data path
\return FS_SEND_Q_STATUS
*/
FS_SEND_Q_STATUS FS_GetFldInfo(FS_MEDIA_SEL MediaSel, FS_GET_FLD_FILE_INFO_MODE GetInfoMode, FS_GET_A_SINGLE_FLD_INTO_t SingleInfo, FS_GET_A_SERIES_FLD_INTO_t SeriesInfo, FS_FILE_PATH FilePath);
//-----------------------------------------------------------------------------
/*!
\brief manual delete file
\param KnlDelProc		parameter of delete information
\return FS_SEND_Q_STATUS
*/
FS_SEND_Q_STATUS FS_ManualDeleteFile(FS_KNL_MANUAL_DEL_PROCESS_t *KnlDelProc);
//-----------------------------------------------------------------------------
/*!
\brief delete file by extension name
\param KnlDelProc		parameter of delete information
\return FS_SEND_Q_STATUS
*/
FS_SEND_Q_STATUS FS_DeleteFileByExtName(FS_KNL_DEL_FILE_BY_EXTNAME_PROCESS_t *KnlDelProc);
//-----------------------------------------------------------------------------
/*!
\brief move file(move file on a media only)
\param KnlMoveProc		parameter of move information
\return FS_SEND_Q_STATUS
*/
FS_SEND_Q_STATUS FS_MoveFile(FS_KNL_MOVE_PROCESS_t *KnlMoveProc);
//-----------------------------------------------------------------------------
/*!
\brief lock file while recording
\param MediaSel			media select
\param LockSwitch		lock switch:LOCK_FILE_OFF/LOCK_FILE_ON
\return(no)
*/
void FS_LockFileWhileRecording(FS_MEDIA_SEL MediaSel, FS_LOCK_FILE_SWITCH LockSwitch);
//-----------------------------------------------------------------------------
/*!
\brief get status of lower layer create file process
\param MediaSel		media select
\param SrcNum		source number
\return FS_Q_STATUS
*/
FS_Q_STATUS FS_ChkCreateStatus(FS_MEDIA_SEL MediaSel, FS_SRC_NUM SrcNum);
//-----------------------------------------------------------------------------
/*!
\brief get status of lower layer close file process
\param MediaSel		media select
\param SrcNum		source number
\return FS_Q_STATUS
*/
FS_Q_STATUS FS_ChkCloseStatus(FS_MEDIA_SEL MediaSel, FS_SRC_NUM SrcNum);
//-----------------------------------------------------------------------------
/*!
\brief get status of lower layer read file process
\param MediaSel		media select
\param SrcNum		source number
\return FS_Q_STATUS
*/
FS_Q_STATUS FS_ChkReadStatus(FS_MEDIA_SEL MediaSel, FS_SRC_NUM SrcNum);
//-----------------------------------------------------------------------------
/*!
\brief get status of lower layer get (normal/hidden) file information process
\param MediaSel		media select
\return FS_Q_STATUS
*/
FS_Q_STATUS FS_ChkGetFileInfoStatus(FS_MEDIA_SEL MediaSel);
//-----------------------------------------------------------------------------
/*!
\brief get status of lower layer modify file hidden information process
\param MediaSel		media select
\return FS_Q_STATUS
*/
FS_Q_STATUS FS_ChkModifyFileHiddenInfoStatus(FS_MEDIA_SEL MediaSel);
//-----------------------------------------------------------------------------
/*!
\brief get status of lower layer get (normal/hidden) folder information process
\param MediaSel		media select
\return FS_Q_STATUS
*/
FS_Q_STATUS FS_ChkGetFldInfoStatus(FS_MEDIA_SEL MediaSel);
//-----------------------------------------------------------------------------
/*!
\brief get status of lower layer delete file process
\param MediaSel		media select
\return FS_Q_STATUS
*/
FS_Q_STATUS FS_ChkManualDelStatus(FS_MEDIA_SEL MediaSel);
//-----------------------------------------------------------------------------
/*!
\brief get status of lower layer delete file by extension name process
\param MediaSel		media select
\return FS_Q_STATUS
*/
FS_Q_STATUS FS_ChkDelFileByExtNameStatus(FS_MEDIA_SEL MediaSel);
//-----------------------------------------------------------------------------
/*!
\brief get status of lower layer looping delete process
\param MediaSel		media select
\return FS_Q_STATUS
*/
FS_Q_STATUS FS_ChkLoopDelStatus(FS_MEDIA_SEL MediaSel);
//-----------------------------------------------------------------------------
/*!
\brief set status equals null of lower layer close file process
\param MediaSel		media select
\param SrcNum		source number
\return(none)
*/
void FS_SetCloseStatusNull(FS_MEDIA_SEL MediaSel, FS_SRC_NUM SrcNum);
//-----------------------------------------------------------------------------
/*!
\brief get status of move file process
\param MediaSel		media select
\return FS_Q_STATUS
*/
FS_Q_STATUS FS_ChkMoveFileStatus(FS_MEDIA_SEL MediaSel);
//-----------------------------------------------------------------------------
/*!
\brief file system initial function
\param info		FS_KNL_PARA_t
\return FS_INIT_STATUS
*/
FS_INIT_STATUS FS_Init(FS_KNL_PARA_t *info);
//-----------------------------------------------------------------------------
/*!
\brief file system uninitial function
\return(no)
*/
void FS_UnInit(void);
//-----------------------------------------------------------------------------
/*!
\brief get file system total buffer size
\param info		FS_KNL_PARA_t
\return total buffer size(unit:bytes)
*/
uint32_t ulFS_GetTotalBufSize(FS_KNL_PARA_t *info);
//-----------------------------------------------------------------------------
/*!
\brief Media format(fat32 for 8~32GB and exfat for 64GB~128GB)
\param MediaSel			media select
\return FS_FMT_STATUS
*/
FS_FMT_STATUS FS_MediaFormat(FS_MEDIA_SEL MediaSel);
//-----------------------------------------------------------------------------
/*!
\brief check media exist
\param MediaSel		media select
\return FS_MEDIA_INOUT_STATUS
*/
FS_MEDIA_INOUT_STATUS FS_ChkMediaExist(FS_MEDIA_SEL MediaSel);
//-----------------------------------------------------------------------------
/*!
\brief check media ready access status
\param MediaSel		media select
\return FS_MEDIA_RDY_ACC
*/
FS_MEDIA_RDY_ACC FS_ChkMediaRdy(FS_MEDIA_SEL MediaSel);
//-----------------------------------------------------------------------------
/*!
\brief reset media status to re-identify media
\param MediaSel		media select
\return(none)
*/
void FS_MediaReset(FS_MEDIA_SEL MediaSel);
//-----------------------------------------------------------------------------
/*!
\brief get media free space
\param MediaSel		media select
\return free space; unit:Mbytes
*/
uint32_t ulFS_GetFreeSpace(FS_MEDIA_SEL MediaSel);
//-----------------------------------------------------------------------------
/*!
\brief get media total space
\param MediaSel		media select
\return total space; unit:Mbytes
*/
uint32_t ulFS_GetTotalSpace(FS_MEDIA_SEL MediaSel);
//-----------------------------------------------------------------------------
/*!
\brief turn on the looping
\param MediaSel		media select
\param Mode			looping mode
\param ulValue		if looping mode = LOOPING_EMG, then the meaning of value is file number
						=>delete file when file num in emg folder > value
					if looping mode = LOOPING_TIMELAPS, then the meaning of value is file number
						=>delete file when file num in timelapse folder > value
					if looping mode = LOOPING_NORMAL, then the meaning of value is the remain free space(unit:MB)
						=>delete file when free size < value
					if looping mode = LOOPING_CUST1_VIDEO, then the meaning of value is the remain free space(unit:MB)
						=>delete file when free size < value or file num in Video folder > ulRecFldMaxSupFileNum
					if looping mode = LOOPING_CUST1_PHOTO, then the meaning of value is the remain free space(unit:MB)
						=>delete file when free size < value or file num in Video folder > ulRecFldMaxSupFileNum
\return(no)
*/
void FS_LoopingOn(FS_MEDIA_SEL MediaSel, FS_LOOPING_MODE Mode, uint32_t ulValue);
//-----------------------------------------------------------------------------
/*!
\brief turn off the looping
\param MediaSel		media select
\param Mode			looping mode
\return(no)
*/
void FS_LoopingOff(FS_MEDIA_SEL MediaSel, FS_LOOPING_MODE Mode);
//-----------------------------------------------------------------------------
/*!
\brief get looping status(on/off)
\param MediaSel		media select
\param Mode			looping mode
\return FS_LOOPING_SWITCH
*/
FS_LOOPING_SWITCH FS_GetLoopingStatus(FS_MEDIA_SEL MediaSel, FS_LOOPING_MODE Mode);
//-----------------------------------------------------------------------------
/*!
\brief get latest file group index
\param MediaSel		media select
\param SrcNum		source number
\return group index
*/
uint16_t uwFS_GetLatestGroupIdx(FS_MEDIA_SEL MediaSel, FS_SRC_NUM SrcNum);
//-----------------------------------------------------------------------------
/*!
\brief get maximum rolling group index num
\param MediaSel		media select
\return group index
*/
uint16_t uwFS_GetMaxRollingGroupIdxNum(FS_MEDIA_SEL MediaSel);
//-----------------------------------------------------------------------------
/*!
\brief get latest file name
\param MediaSel		media select
\param Output		output file name
\return file name length
*/
uint32_t ulFS_GetLatestFileName(FS_MEDIA_SEL MediaSel, char *Output);
//-----------------------------------------------------------------------------
/*!
\brief tempory file name handle
\param MediaSel			media select
\param chOutName		output file name
\param ulOutNameLen		output file name length
\param chInName			input file name
\param ulInNameLen		input file name length
\return(no)
*/
void FS_FileNameHandle(FS_MEDIA_SEL MediaSel, char *chOutName, uint32_t ulOutNameLen, char *chInName, uint32_t ulInNameLen);
//-----------------------------------------------------------------------------
/*!
\brief media speed test
\param MediaSel				media select
\param info					read/write test information
\param ubRWTestInfoNum		read/write test information number, max value=50
\return FS_MEDIA_SPD_RPT
*/
FS_MEDIA_SPD_RPT FS_MediaRWTest(FS_MEDIA_SEL MediaSel, FS_RW_TEST_INTO_t *info, uint8_t ubRWTestInfoNum);
//-----------------------------------------------------------------------------
/*!
\brief get current 2 groups close files for moving to emergency folder
\param MediaSel			media select
\param uwTarGrpIdx1		first group index
\param uwTarGrpIdx2		second group index
\param ubOutInfoNum		output file information number
\param OutInfo			output file information
\return(no)
*/
void FS_GetEmgFileInfo(FS_MEDIA_SEL MediaSel, uint16_t uwTarGrpIdx1, uint16_t uwTarGrpIdx2, uint8_t *ubOutInfoNum, FS_EMERG_FILE_INFO_t *OutInfo);
//-----------------------------------------------------------------------------
/*!
\brief get emergency file number
\param MediaSel			media select
\return file number
*/
uint32_t ulFS_GetEmgFileNum(FS_MEDIA_SEL MediaSel);
//-----------------------------------------------------------------------------
/*!
\brief get timelapse file number
\param MediaSel			media select
\return file number
*/
uint32_t ulFS_GetTimelapseFileNum(FS_MEDIA_SEL MediaSel);
//-----------------------------------------------------------------------------
/*!
\brief get emergency folder total file size(unit:KBytes)
\param MediaSel			media select
\return total file size(unit:Bytes)
*/
uint64_t ullFS_GetEmgFldTotalFileSz(FS_MEDIA_SEL MediaSel);
//-----------------------------------------------------------------------------
/*!
\brief get timelapse folder total file size(unit:KBytes)
\param MediaSel			media select
\return total file size(unit:Bytes)
*/
uint64_t ullFS_GetTimelapseFldTotalFileSz(FS_MEDIA_SEL MediaSel);
//-----------------------------------------------------------------------------
/*!
\brief get record folder total file size(unit:KBytes)
\param MediaSel			media select
\return total file size(unit:Bytes)
*/
uint64_t ullFS_GetRecFldTotalFileSz(FS_MEDIA_SEL MediaSel);
//-----------------------------------------------------------------------------
/*!
\brief get sd card size classify
\param MediaSel		media select
\return FS_CARD_SIZE
*/
FS_CARD_SIZE FS_GetSdCardSize(FS_MEDIA_SEL MediaSel);
//-----------------------------------------------------------------------------
/*!
\brief get FS boot sector information
\param MediaSel		media select
\return FS_BOOTSECTOR_INTO_t
*/
FS_BOOTSECTOR_INTO_t FS_GetBootSectorInfo(FS_MEDIA_SEL MediaSel);
//-----------------------------------------------------------------------------
/*!
\brief get latest calendar from hidden file in a certain one media
\param MediaSel		media select
\return FS_CALENDAR_t
*/
FS_CALENDAR_t FS_GetLatestCalendar(FS_MEDIA_SEL MediaSel);
//-----------------------------------------------------------------------------
/*!
\brief delete all files in Emg folder(for testing)
\param MediaSel		media select
\return(no)
*/
void FS_DelAllFileInEmgFld(FS_MEDIA_SEL MediaSel);
//-----------------------------------------------------------------------------
/*!
\brief FS dump debug log -> dump data to dram
\param MediaSel		media select
\param Info			dump information
\return(no)
*/
void FS_DumpDBGLog_Dump(FS_MEDIA_SEL MediaSel, FS_DUMP_DBG_LOG_t Info);
//-----------------------------------------------------------------------------
/*!
\brief FS dump debug log -> stop dumping data to dram and write a file to media
\param MediaSel		media select
\return(no)
*/
void FS_DumpDBGLog_Stop(FS_MEDIA_SEL MediaSel);
//-----------------------------------------------------------------------------
/*!
\brief FS switch rec folder name(for debug using)
\param MediaSel		media select
\param RecFldName	record folder name
\return(no)
*/
void FS_SwitchRecFld(FS_MEDIA_SEL MediaSel, FS_FLD_NAME_INFO_t *RecFldName);
//-----------------------------------------------------------------------------
/*!
\brief get media type from a certain media
\param MediaSel		media select
\return FS_MEDIA_TYPE
*/
FS_MEDIA_TYPE FS_GetMediaType(FS_MEDIA_SEL MediaSel);
//-----------------------------------------------------------------------------
/*!
\brief Set EXFAT file format time zone
\param TimeZone		Time Zone
\return(no)
*/
void FS_ExFATTimeZoneSetting(FS_EXFAT_TIMEZONE TimeZone);
void ulFS_HiddenInfSizeSetting(uint32_t ulOffset,uint32_t ulSize);
uint32_t ulFS_HiddenInfSizeGet(void);
#endif

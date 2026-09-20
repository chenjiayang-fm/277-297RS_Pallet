/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		FD_API.h
	\brief		Face Detection / Face Recogition
	\author		Nick Huang
	\version	1.1
	\date		2019/03/18
	\copyright	Copyright(C) 2019 SONiX Technology Co., Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------
#ifndef _FD_API_
#define _FD_API_
//------------------------------------------------------------------------------
#include "_510PF.h"

typedef enum
{
    FD_FAIL = -1,
    FD_SUCCESS = 0,
    FD_READY_TO_CAPTURE = 1,
    FD_READY_TO_CAPTURE_NEXT,
    FD_CAPTURE_TIMEOUT,
    FD_CAPTURE_SUCCESS
} FD_Status_t;

typedef void (*FD_ReportInfo) (uint8_t ubFaceCnt);
typedef void (*FD_ReportFace) (int level, int left, int top, int right, int bottom);
typedef void (*FD_ReportMatched) (int level, int left, int top, int right, int bottom, char* szName);

typedef struct
{
    uint32_t ulImgWidth;
    uint32_t ulImgHeight;
    FD_ReportInfo pReportInfoFunc;
    FD_ReportFace pReportFaceFunc;
    FD_ReportMatched pReportMatchedFunc;
} FD_InitInfo_t;
//------------------------------------------------------------------------------
void FD_Init(FD_InitInfo_t* pInitInfo);
uint16_t uwFD_GetVersion(void);
void FD_FeedImage(uint32_t* pImageAddr);
uint32_t ulFD_GetValidCnt(void);
uint32_t ulFD_GetUsedCnt(void);
int iFD_GetIndexFromName(char* name);
FD_Status_t tFD_GetNameFromIndex(uint32_t ulIndex, char* name);
FD_Status_t tFD_DeleteUser(char* name);
void FD_StartCapture(char* name, osMessageQId tCaptureMessage);
void FD_TriggerCapture(uint32_t ulTimeout);
void FD_StopCapture(void);
void FD_Suspend(void);
void FD_Resume(void);

#endif

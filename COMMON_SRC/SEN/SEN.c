/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		SEN.c
	\brief		Sensor relation function
	\author		BoCun
	\version	4.37
	\date		2020-12-22
	\copyright	Copyright(C) 2020 SONiX Technology Co.,Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "Math.h"
#include "SEN.h"
#include "INTC.h"
#include "TIMER.h"
#include "IQ_API.h"
#include "IQ_PARSER_API.h"
#include "ISP_API.h"
#include "ISP_FONT_API.h"
#include "AE_API.h"
#include "AF_API.h"
#include "AWB_API.h"
#include "DIS_API.h"
#include "MD_API.h"
#include "BUF.h"
#include "BSP.h"
#include "DMAC_API.h"
#include "KNL.h"
#include "USBD_API.h"
#if (defined(OP_STA) && (APP_USBD_COMPOSITE_MODE & USBD_MULTI_UVC))
#include "UI_BUCCAM.h"
#endif

//------------------------------------------------------------------------------
SEN_CVBS_STATE_t    tCvbsState;
SEN_SENSOR_STATE_t  tSensorState;
osSemaphoreId 	SEM_SEN_VSyncRdy;
osSemaphoreId 	SEM_MergeFrameRdy;
osMessageQId tSEN_ExtEventQueue;
static void SEN_DoVsyncThread(void const *argument);
static void SEN_ChkSensorStateThread(void const *argument);
static void SEN_CvbsFieldMergeThread(void const *argument);
static pvSEN_CbFunc pSEN_CbFunc = NULL;
static pvSEN_SyncCbFunc pSEN_SyncCbFunc = NULL;
static pvSEN_HwEndIsrCbFunc pSEN_HwEndIsrCbFunc[4] = {NULL, NULL, NULL, NULL};
uint8_t ubSEN_VIDEO[3]={0,0,0};
uint8_t ubSEN_ActiveFlg[3] = {0,0,0};//PATH1,PATH2,PATH3
uint8_t ubSEN_FirstOutFlg = 1;
uint8_t ubSEN_FirstVsync = 0;
uint8_t ubSEN_StateChangeFlg = 0;
uint8_t ubSEN_EventNode;
uint8_t ubSEN_UvcPathFlag = 0;
uint8_t ubSEN_ResChgFlg[3] = {0,0,0};
uint8_t ubSEN_SetDefMaxResolution = 0;
uint16_t uwSEN_DefH,uwSEN_DefV;
uint32_t ulSEN_Cvbs16MDataAddr = 0;
uint8_t  ubSEN_BufNub = 2;
uint8_t ubSEN_IspRate_Backup = 0;
uint8_t ubSEN_AxiRate_Backup = 0;
uint8_t ubSEN_CapScaleUp=0;
uint8_t ubSEN_CapFrmCnt=0;
uint32_t ulSEN_CapAddr;

static pSEN_IsrProcCb pSEN_IsrProcFunc[2] = {NULL, NULL};
static pvSEN_CbAhdSetResFunc pSEN_CbAhdSetResFunc = NULL;
static pvSEN_CbAhdSetBufFunc pSEN_CbAhdSetBufFunc = NULL;
//------------------------------------------------------------------------------
void SEN_SetOutResolution(uint8_t ubPath, uint16_t uwH, uint16_t uwV)
{
	if(ubPath == SENSOR_PATH1)
	{
		if(!ubSEN_SetDefMaxResolution)
		{
			uwSEN_DefH = uwH;
			uwSEN_DefV = uwV;
			//printf("uwSEN_DefH = %d uwSEN_DefV = %d\n",uwH, uwV); 
			ubSEN_SetDefMaxResolution = 1;
		}		
		// path1 image horizontal/vertical size
		sensor_cfg.xtSENPath.uwHSize1 = uwH;
		sensor_cfg.xtSENPath.uwVSize1 = uwV;
		// Path1 Image frame size
		sensor_cfg.xtSENPath.ulFrameSize1 = (uint32_t)uwH * (uint32_t)uwV * 3 / 2;
	}
	else if(ubPath == SENSOR_PATH2)
	{
        // path2 image horizontal/vertical size
		sensor_cfg.xtSENPath.uwHSize2 = uwH;
		sensor_cfg.xtSENPath.uwVSize2 = uwV;
		// Path2 Image frame size
		sensor_cfg.xtSENPath.ulFrameSize2 = (uint32_t)uwH * (uint32_t)uwV * 3 / 2;
	}
	else if(ubPath == SENSOR_PATH3)
	{
        // path3 image horizontal/vertical size
		sensor_cfg.xtSENPath.uwHSize3 = uwH;
		sensor_cfg.xtSENPath.uwVSize3 = uwV;
		// Path3 Image frame size 
		sensor_cfg.xtSENPath.ulFrameSize3 = (uint32_t)uwH * (uint32_t)uwV * 3 / 2;
	}
	// Set scaler
	printd(DBG_Debug3Lvl,"P=%d,h=%d,v=%d,H=%d,V=%d\n",ubPath, uwH, uwV, sensor_cfg.xtSENWin.uwHSize, sensor_cfg.xtSENWin.uwVSize);
	if((sensor_cfg.ubSensorType == SEN_TP9950) && ((sensor_state.ubInputType == SENSOR_CVBS_NTSC) || (sensor_state.ubInputType == SENSOR_CVBS_PAL)))
	{	
		ISP_SetScaler(ubPath, uwH, (2*uwV), sensor_cfg.xtSENWin.uwHSize, sensor_cfg.xtSENWin.uwVSize);	
	}
	else
		ISP_SetScaler(ubPath, uwH, uwV, sensor_cfg.xtSENWin.uwHSize, sensor_cfg.xtSENWin.uwVSize);
	// Set scale parameter
	ISP_ScalerSetting(((uwH>sensor_cfg.xtSENWin.uwHSize) || (uwV>sensor_cfg.xtSENWin.uwVSize))? ((ubPath == SENSOR_PATH1)?SEN_SCALEUP_MODE:SEN_NORMAL_MODE):SEN_NORMAL_MODE);
}

//------------------------------------------------------------------------------
#if (defined(OP_STA) || (defined(OP_AP) && defined(BSP_DVR_SDK)))
void SEN_RegisterEventQueue(osMessageQId tQueueId)
{
	tSEN_ExtEventQueue = tQueueId;
}
#endif

//------------------------------------------------------------------------------
#if (defined(OP_STA) || (defined(OP_AP) && defined(BSP_DVR_SDK)))
void SEN_RegisterEventNode(uint8_t ubEventNode)
{
	ubSEN_EventNode = ubEventNode;
}
#endif

//------------------------------------------------------------------------------
void SEN_SetPathAddr(uint8_t ubPath,uint32_t ulBufAddr)
{
	if(ubPath == SENSOR_PATH1)
	{
		SEN->STR1_STA = ulBufAddr >> 8;       
        sensor_cfg.xtSENAddr.ulPath1_Addr = ulBufAddr;
	}
	else if(ubPath == SENSOR_PATH2)
	{
		SEN->STR2_STA = ulBufAddr >> 8;        
        sensor_cfg.xtSENAddr.ulPath2_Addr = ulBufAddr;
	}
	else if(ubPath == SENSOR_PATH3)
	{
		SEN->STR3_STA = ulBufAddr >> 8;        
        sensor_cfg.xtSENAddr.ulPath3_Addr = ulBufAddr;
	}
	else if(ubPath == ISP_3DNR)
	{
		SEN->NR3D_STA = ulBufAddr >> 8;
        SEN->NR3D_EDA = (ulBufAddr + ISP_WIDTH*ISP_HEIGHT*10/8) >> 8;
        
        sensor_cfg.xtSENAddr.ul3DNR_STA = ulBufAddr;
        sensor_cfg.xtSENAddr.ul3DNR_EDA = (ulBufAddr + ISP_WIDTH*ISP_HEIGHT*10/8);
	}    
	else if(ubPath == ISP_MD_W0)
	{
		SEN->MD0_STA = ulBufAddr >> 8;        
        sensor_cfg.xtSENAddr.ulMD_M0_Addr = ulBufAddr;
	} 
	else if(ubPath == ISP_MD_W1)
	{
		SEN->MD1_STA = ulBufAddr >> 8;        
        sensor_cfg.xtSENAddr.ulMD_M1_Addr = ulBufAddr;
	} 
	else if(ubPath == ISP_MD_W2)
	{
		SEN->MD2_STA = ulBufAddr >> 8;       
        sensor_cfg.xtSENAddr.ulMD_M2_Addr = ulBufAddr;
	}  
	else if(ubPath == IQ_BIN_FILE)
	{      
        //bin file address
        sensor_cfg.xtSENAddr.ulIQBin_Addr = ulBufAddr;
	}
	else if(ubPath == IMG_TEMP1)
	{
        if((sensor_state.ubInputType == SENSOR_CVBS_NTSC)||(sensor_state.ubInputType == SENSOR_CVBS_PAL))
            SEN->STR1_STA = ulBufAddr >> 8;
        sensor_cfg.xtSENAddr.ulIMG_TEMP1_Addr = ulBufAddr;
	}
}

//------------------------------------------------------------------------------
void SEN_UpdatePathAddr(void)
{
	SEN->COLOR_TRIG_5 = 1;
}

//------------------------------------------------------------------------------
void SEN_SetStateChangeFlg(uint8_t ubFlg)
{
	ubSEN_StateChangeFlg = ubFlg;
}

//------------------------------------------------------------------------------
uint8_t ubSEN_GetStateChangeFlg(void)
{
	return ubSEN_StateChangeFlg;
}

//------------------------------------------------------------------------------
uint8_t ubSEN_GetSensorBufNumber(void)
{
    return ubSEN_BufNub;
}

//------------------------------------------------------------------------------
void SEN_SetUvcPathFlag(uint8_t ubFlag)
{
	if(ubSEN_UvcPathFlag != ubFlag)
	{
		ubSEN_UvcPathFlag = ubFlag;
	}
}

//------------------------------------------------------------------------------
void SEN_SetActiveFlg(uint8_t ubPath,uint8_t ubFlg)
{
	if(ubPath == SENSOR_PATH1)
	{
		ubSEN_ActiveFlg[0] = ubFlg;
	}
	else if(ubPath == SENSOR_PATH2)
	{
		ubSEN_ActiveFlg[1] = ubFlg;
	}
	else if(ubPath == SENSOR_PATH3)
	{
		ubSEN_ActiveFlg[2] = ubFlg;
	}
}

//------------------------------------------------------------------------------
uint8_t ubSEN_GetActiveFlg(uint8_t ubPath)
{
	if(ubPath == SENSOR_PATH1)
	{
		return ubSEN_ActiveFlg[0];
	}
	else if(ubPath == SENSOR_PATH2)
	{
		return ubSEN_ActiveFlg[1];
	}
	else if(ubPath == SENSOR_PATH3)
	{
		return ubSEN_ActiveFlg[2];
	}
	printf("Err @ubSEN_GetActiveFlg\r\n");
	return 0;
}

//------------------------------------------------------------------------------
void SEN_SetFirstOutFlg(uint8_t ubFlg)
{
	ubSEN_FirstOutFlg = ubFlg;
}

//------------------------------------------------------------------------------
uint8_t ubSEN_GetFirstOutFlg(void)
{
	return ubSEN_FirstOutFlg;
}

//------------------------------------------------------------------------------
void SEN_SetResChgFlg(uint8_t ubPath, uint8_t ubFlg)
{
	ubSEN_ResChgFlg[ubPath] = ubFlg;
}

//------------------------------------------------------------------------------
uint8_t ubSEN_GetResChgFlg(uint8_t ubPath)
{
	return ubSEN_ResChgFlg[ubPath];
}

//------------------------------------------------------------------------------
void SEN_SetResChgState(uint8_t ubPath, uint16_t uwH, uint16_t uwV)
{
	SEN_SetResChgFlg(ubPath, TRUE);
	SEN_SetResolution(ubPath, uwH, uwV);
    if(ubSEN_GetSensorType() == SEN_TP9950)
        SEN_SetResChgDropCnt(0);
}

uint8_t ubSEN_ResChgDropCnt = SEN_RES_DROP_FRAME;
uint8_t ubSEN_GetResChgDropCnt(void)
{
    return ubSEN_ResChgDropCnt;
}

void SEN_SetResChgDropCnt(uint8_t ubValue)
{
    ubSEN_ResChgDropCnt = ubValue;
}

//------------------------------------------------------------------------------
void SEN_SetPathSrc(uint8_t ubPath1Src,uint8_t ubPath2Src,uint8_t ubPath3Src)
{
	sensor_cfg.ubPath1Src = ubPath1Src;
	sensor_cfg.ubPath2Src = ubPath2Src;
	sensor_cfg.ubPath3Src = ubPath3Src;
}

//------------------------------------------------------------------------------
uint8_t ubSEN_GetPathSrc(uint8_t ubPath)
{
	if(ubPath == SENSOR_PATH1)
	{
		return sensor_cfg.ubPath1Src;
	}
	else if(ubPath == SENSOR_PATH2)
	{
		return sensor_cfg.ubPath2Src;
	}
	else if(ubPath == SENSOR_PATH3)
	{
		return sensor_cfg.ubPath3Src;
	}
	else
	{ 
		printf("Err @ubSEN_GetPathSrc\r\n");
		return 0xFF;
	}
}

//------------------------------------------------------------------------------
void SEN_Hsync_ISR(void)
{
	// Clear HSYNC flag.
	if( SEN->SEN_HSYNC_INT_FLAG )
	{
		SEN->SEN_HSYNC_INT_CLR = 1;	
	}   
	INTC_IrqClear(INTC_SEN_HSYNC_IRQ);
}

uint8_t ubSEN_ScaleDropCnt = 0;
uint8_t ubSEN_ScaleDropFrame = SEN_DROP_FRAME;
uint8_t ubSEN_CvbsFrameCut = 0;
uint32_t ulSEN_CvbsPALUsFrame = 0x00767677;
uint32_t ulSEN_CvbsNtscUsFrame = 0x66676666;
//------------------------------------------------------------------------------
void SEN_HwEnd_ISR(void)
{
	uint32_t ulSEN_NextYuv1Addr, ulSEN_NextYuv2Addr, ulSEN_NextYuv3Addr;

	SEN_EVENT_PROCESS tProcess;
	uint8_t ubSrc;

	static uint8_t ubSEN_ImgStabCnt  = 6;
	static uint8_t ubSEN_IspReadyFlg = 0;
	static uint8_t ubSEN_YsumStabCnt        = 1;
	static uint8_t ubSEN_YsumReportReadyFlg = 0;
    
	ulSEN_NextYuv1Addr = ulSEN_NextYuv1Addr;//Avoid Warning
	ulSEN_NextYuv2Addr = ulSEN_NextYuv2Addr;//Avoid Warning
	ulSEN_NextYuv3Addr = ulSEN_NextYuv3Addr;//Avoid Warning
	tProcess = tProcess;					//Avoid Warning
	ubSrc = ubSrc;							//Avoid Warning
	// Clear HW_END flag.
	if( SEN->HW_END_INT_FLAG )
	{
		SEN->HW_END_INT_CLR = 1;
	}
	INTC_IrqClear(INTC_ISP_WIN_END_IRQ);
	SEN->IMG_TX_EN = 0;

	// frame end to get 3A report.
	SEN_FrmEnd_ISR();
	
	if(pSEN_IsrProcFunc[SEN_ISPOUT_PROC])
		tProcess.ulTmCnt = pSEN_IsrProcFunc[SEN_ISPOUT_PROC]();

    if(ubSEN_GetFirstOutFlg())
    {
        if(ubSEN_UvcPathFlag)
            SEN_SetFirstOutFlg(0);
        if(ubSEN_GetActiveFlg(SENSOR_PATH1))
        {
            ubBUF_ReleaseSenYuvBuf(SEN->STR1_STA << 8);
        }
        if(ubSEN_GetActiveFlg(SENSOR_PATH2))
        {
            ubBUF_ReleaseSenYuvBuf(SEN->STR2_STA << 8);
        }
        if(ubSEN_GetActiveFlg(SENSOR_PATH3))
        {
            ubBUF_ReleaseSenYuvBuf(SEN->STR3_STA << 8);
        }
    }
    else
    {	
        if(ubKNL_GetSenThenEncEnable())
        {
            if((sensor_state.ubInputType == SENSOR_CVBS_NTSC)||(sensor_state.ubInputType == SENSOR_CVBS_PAL))
            {
				if(!ubKNL_GetChgResFlg())
                {
#if (SEN_USE == SEN_RN6752)					
					if((!ubSEN_GetResChgFlg(SENSOR_PATH1)) && (ubSEN_ScaleDropCnt >= 1))
					{
						ubSEN_ScaleDropCnt = 0; 
						if(sensor_state.ubInputType == SENSOR_CVBS_PAL)
						{	
							if((ulSEN_CvbsPALUsFrame&((uint32_t)0x1<<ubSEN_CvbsFrameCut)) != 0)
							{
								ubSEN_CvbsFrameCut = ((ubSEN_CvbsFrameCut + 1) >= 25)?0:(ubSEN_CvbsFrameCut + 1);
								if(pSEN_HwEndIsrCbFunc[SENSOR_PATH1] != NULL)
								{
									ulSEN_Cvbs16MDataAddr = (SEN->STR1_STA << 8);
									pSEN_HwEndIsrCbFunc[SENSOR_PATH1](SENSOR_PATH1, SEN->STR1_STA, sensor_cfg.xtSENPath.ulFrameSize1);
								}
							}
							else
							{
								ubSEN_CvbsFrameCut = ((ubSEN_CvbsFrameCut + 1) >= 25)?0:(ubSEN_CvbsFrameCut + 1);
								ubBUF_ReleaseSenYuvBuf(SEN->STR1_STA << 8);
								SEN_SetResChgFlg(SENSOR_PATH1, FALSE);	
							}
						}
						else
						{
							if((ulSEN_CvbsNtscUsFrame&((uint32_t)0x1<<ubSEN_CvbsFrameCut)) != 0)
							{
								ubSEN_CvbsFrameCut = ((ubSEN_CvbsFrameCut + 1) >= 30)?0:(ubSEN_CvbsFrameCut + 1);
								if(pSEN_HwEndIsrCbFunc[SENSOR_PATH1] != NULL)
								{
									ulSEN_Cvbs16MDataAddr = (SEN->STR1_STA << 8);
									pSEN_HwEndIsrCbFunc[SENSOR_PATH1](SENSOR_PATH1, SEN->STR1_STA, sensor_cfg.xtSENPath.ulFrameSize1);
								}
							}
							else
							{
								ubSEN_CvbsFrameCut = ((ubSEN_CvbsFrameCut + 1) >= 30)?0:(ubSEN_CvbsFrameCut + 1);
								ubBUF_ReleaseSenYuvBuf(SEN->STR1_STA << 8);
								SEN_SetResChgFlg(SENSOR_PATH1, FALSE);	
							}
						}		
					}
					else
					{
						ubSEN_ScaleDropCnt++;
						ubBUF_ReleaseSenYuvBuf(SEN->STR1_STA << 8);
						SEN_SetResChgFlg(SENSOR_PATH1, FALSE);
					}
#endif										
#if (SEN_USE == SEN_TP9950)	
					if((!ubSEN_GetResChgFlg(SENSOR_PATH1)) && (ubSEN_ScaleDropCnt >= 1))
					{
						ubSEN_ScaleDropCnt = 0;
	
						if(pSEN_HwEndIsrCbFunc[SENSOR_PATH1] != NULL)
						{
							ulSEN_Cvbs16MDataAddr = (SEN->STR1_STA << 8) + (sensor_cfg.xtSENPath.uwHSize1*(sensor_cfg.xtSENPath.uwVSize1/2)*3/1);
							pSEN_HwEndIsrCbFunc[SENSOR_PATH1](SENSOR_PATH1, SEN->STR1_STA, sensor_cfg.xtSENPath.ulFrameSize1);
						}
					}
					else
					{
						ubSEN_ScaleDropCnt++;
						ubBUF_ReleaseSenYuvBuf(SEN->STR1_STA << 8);
						SEN_SetResChgFlg(SENSOR_PATH1, FALSE);
					}
#endif
                }
			
            }
            else
            {
                if(!ubKNL_GetChgResFlg())
                {
                    if(pSEN_HwEndIsrCbFunc[SENSOR_PATH1] != NULL)
                    {
                        pSEN_HwEndIsrCbFunc[SENSOR_PATH1](SENSOR_PATH1, SEN->STR1_STA, sensor_cfg.xtSENPath.ulFrameSize1);
                    }    
                }				
            }   
        }
        else
        {	
            //Path1-Q
            if(ubSEN_GetActiveFlg(SENSOR_PATH1))
            {
                if((sensor_state.ubInputType == SENSOR_CVBS_NTSC)||(sensor_state.ubInputType == SENSOR_CVBS_PAL))
                {
#if (SEN_USE == SEN_RN6752)					
					if((!ubSEN_GetResChgFlg(SENSOR_PATH1)) && (ubSEN_ScaleDropCnt >= 1))
					{
						ubSEN_ScaleDropCnt = 0; 
						if(sensor_state.ubInputType == SENSOR_CVBS_PAL)
						{	
							if((ulSEN_CvbsPALUsFrame&((uint32_t)0x1<<ubSEN_CvbsFrameCut)) != 0)
							{
								ubSEN_CvbsFrameCut = ((ubSEN_CvbsFrameCut + 1) >= 25)?0:(ubSEN_CvbsFrameCut + 1);
								ulSEN_NextYuv1Addr = ulBUF_GetSen1YuvFreeBuf();
								if(ulSEN_NextYuv1Addr != BUF_FAIL)
								{
									ubSrc = ubSEN_GetPathSrc(SENSOR_PATH1);
									tProcess.ubSrcNum		= ubSrc;
									tProcess.ubCurNode	 	= 0;
									tProcess.ubNextNode 	= ubSEN_EventNode;
									tProcess.ulSize			= sensor_cfg.xtSENPath.ulFrameSize1;
									tProcess.ulDramAddr1	= SEN->STR1_STA << 8;	//YUV Address
									sensor_cfg.xtSENAddr.ulPath1_Addr = tProcess.ulDramAddr1;
									if(osMessagePut(tSEN_ExtEventQueue, &tProcess, 0) != osOK)
									{
										ubBUF_ReleaseSenYuvBuf(SEN->STR1_STA << 8);
										printf("SEN_Q->Full !!!!\r\n");
									}
									else
										SEN->STR1_STA = ulSEN_NextYuv1Addr >> 8;
									return;
								}
								else
								{
									//ubBUF_ReleaseSenYuvBuf(SEN->STR1_STA << 8);
								}
								SEN->VIDEO_STR_EN_1 = (ulSEN_NextYuv1Addr == BUF_FAIL)? 0:1;
							}
							else
							{
								ubSEN_CvbsFrameCut = ((ubSEN_CvbsFrameCut + 1) >= 25)?0:(ubSEN_CvbsFrameCut + 1);
								ubBUF_ReleaseSenYuvBuf(SEN->STR1_STA << 8);
								SEN_SetResChgFlg(SENSOR_PATH1, FALSE);	
							}
						}
						else
						{
							if((ulSEN_CvbsNtscUsFrame&((uint32_t)0x1<<ubSEN_CvbsFrameCut)) != 0)
							{
								ubSEN_CvbsFrameCut = ((ubSEN_CvbsFrameCut + 1) >= 30)?0:(ubSEN_CvbsFrameCut + 1);
								ulSEN_NextYuv1Addr = ulBUF_GetSen1YuvFreeBuf();
								if(ulSEN_NextYuv1Addr != BUF_FAIL)
								{
									ubSrc = ubSEN_GetPathSrc(SENSOR_PATH1);
									tProcess.ubSrcNum		= ubSrc;
									tProcess.ubCurNode	 	= 0;
									tProcess.ubNextNode 	= ubSEN_EventNode;
									tProcess.ulSize			= sensor_cfg.xtSENPath.ulFrameSize1;
									tProcess.ulDramAddr1	= SEN->STR1_STA << 8;	//YUV Address
									sensor_cfg.xtSENAddr.ulPath1_Addr = tProcess.ulDramAddr1;
									if(osMessagePut(tSEN_ExtEventQueue, &tProcess, 0) != osOK)
									{
										ubBUF_ReleaseSenYuvBuf(SEN->STR1_STA << 8);
										printf("SEN_Q->Full !!!!\r\n");
									}
									else
										SEN->STR1_STA = ulSEN_NextYuv1Addr >> 8;
									return;
								}
								else
								{
									//ubBUF_ReleaseSenYuvBuf(SEN->STR1_STA << 8);
								}
								SEN->VIDEO_STR_EN_1 = (ulSEN_NextYuv1Addr == BUF_FAIL)? 0:1;
							}
							else
							{
								ubSEN_CvbsFrameCut = ((ubSEN_CvbsFrameCut + 1) >= 30)?0:(ubSEN_CvbsFrameCut + 1);
								ubBUF_ReleaseSenYuvBuf(SEN->STR1_STA << 8);
								SEN_SetResChgFlg(SENSOR_PATH1, FALSE);	
							}
						}		
					}
					else
					{
						ubSEN_ScaleDropCnt++;
						ubBUF_ReleaseSenYuvBuf(SEN->STR1_STA << 8);
						SEN_SetResChgFlg(SENSOR_PATH1, FALSE);
					}
					
#endif					
#if (SEN_USE == SEN_TP9950)	
					if((!ubSEN_GetResChgFlg(SENSOR_PATH1)) && (ubSEN_ScaleDropCnt >= 1))
					{
						ubSEN_ScaleDropCnt = 0;
						if(sensor_state.ubInputType == SENSOR_CVBS_PAL)
						{	
//							if((ulSEN_CvbsPALUsFrame&((uint32_t)0x1<<ubSEN_CvbsFrameCut)) != 0)
							{
//								ubSEN_CvbsFrameCut = ((ubSEN_CvbsFrameCut + 1) >= 25)?0:(ubSEN_CvbsFrameCut + 1);
								ulSEN_NextYuv1Addr = ulBUF_GetSen1YuvFreeBuf();
								if(ulSEN_NextYuv1Addr != BUF_FAIL)
								{
									ubSrc = ubSEN_GetPathSrc(SENSOR_PATH1);
									tProcess.ubSrcNum		= ubSrc;
									tProcess.ubCurNode	 	= 0;
									tProcess.ubNextNode 	= ubSEN_EventNode;
									tProcess.ulSize			= sensor_cfg.xtSENPath.ulFrameSize1;
									tProcess.ulDramAddr1	= SEN->STR1_STA << 8;	//YUV Address
									sensor_cfg.xtSENAddr.ulPath1_Addr = tProcess.ulDramAddr1;
									if(osMessagePut(tSEN_ExtEventQueue, &tProcess, 0) != osOK)
									{
										ubBUF_ReleaseSenYuvBuf(SEN->STR1_STA << 8);
										printf("SEN_Q->Full !!!!\r\n");
									}
									else
										SEN->STR1_STA = ulSEN_NextYuv1Addr >> 8;
									return;
								}
								else
								{
									//ubBUF_ReleaseSenYuvBuf(SEN->STR1_STA << 8);
								}
								SEN->VIDEO_STR_EN_1 = (ulSEN_NextYuv1Addr == BUF_FAIL)? 0:1;
							}
//							else
//							{
//								ubSEN_CvbsFrameCut = ((ubSEN_CvbsFrameCut + 1) >= 25)?0:(ubSEN_CvbsFrameCut + 1);
//								ubBUF_ReleaseSenYuvBuf(SEN->STR1_STA << 8);
//								SEN_SetResChgFlg(SENSOR_PATH1, FALSE);	
//							}
						}
						else
						{
//							if((ulSEN_CvbsNtscUsFrame&((uint32_t)0x1<<ubSEN_CvbsFrameCut)) != 0)
							{
//								ubSEN_CvbsFrameCut = ((ubSEN_CvbsFrameCut + 1) >= 30)?0:(ubSEN_CvbsFrameCut + 1);
								ulSEN_NextYuv1Addr = ulBUF_GetSen1YuvFreeBuf();
								if(ulSEN_NextYuv1Addr != BUF_FAIL)
								{
									ubSrc = ubSEN_GetPathSrc(SENSOR_PATH1);
									tProcess.ubSrcNum		= ubSrc;
									tProcess.ubCurNode	 	= 0;
									tProcess.ubNextNode 	= ubSEN_EventNode;
									tProcess.ulSize			= sensor_cfg.xtSENPath.ulFrameSize1;
									tProcess.ulDramAddr1	= SEN->STR1_STA << 8;	//YUV Address
									sensor_cfg.xtSENAddr.ulPath1_Addr = tProcess.ulDramAddr1;
									if(osMessagePut(tSEN_ExtEventQueue, &tProcess, 0) != osOK)
									{
										ubBUF_ReleaseSenYuvBuf(SEN->STR1_STA << 8);
										printf("SEN_Q->Full !!!!\r\n");
									}
									else
										SEN->STR1_STA = ulSEN_NextYuv1Addr >> 8;
									return;
								}
								else
								{
									//ubBUF_ReleaseSenYuvBuf(SEN->STR1_STA << 8);
								}
								SEN->VIDEO_STR_EN_1 = (ulSEN_NextYuv1Addr == BUF_FAIL)? 0:1;
							}
//							else
//							{
//								ubSEN_CvbsFrameCut = ((ubSEN_CvbsFrameCut + 1) >= 30)?0:(ubSEN_CvbsFrameCut + 1);
//								ubBUF_ReleaseSenYuvBuf(SEN->STR1_STA << 8);
//								SEN_SetResChgFlg(SENSOR_PATH1, FALSE);	
//							}
						}		
					}
					else
					{
						ubSEN_ScaleDropCnt++;
						ubBUF_ReleaseSenYuvBuf(SEN->STR1_STA << 8);
						SEN_SetResChgFlg(SENSOR_PATH1, FALSE);
					}
		
//                    SEN->STR1_STA = sensor_cfg.xtSENAddr.ulIMG_TEMP1_Addr >> 8;
//                    osSemaphoreRelease(SEM_MergeFrameRdy);
//                    if(tCvbsState.ubFieldMergeFin)
//                    {
//                        if(!ubSEN_GetResChgFlg(SENSOR_PATH1) && sensor_state.ubDetectVideo)    
//                        {
//                            if(ubSEN_FrameDropState(SENSOR_PATH1))
//                            {
//                                ulSEN_NextYuv1Addr = ulBUF_GetSen1YuvFreeBuf();
//                                if(ulSEN_NextYuv1Addr != BUF_FAIL)
//                                {
//                                    ubSrc = ubSEN_GetPathSrc(SENSOR_PATH1);
//                                    tProcess.ubSrcNum		= ubSrc;
//                                    tProcess.ubCurNode	 	= 0;
//                                    tProcess.ubNextNode 	= ubSEN_EventNode;
//                                    tProcess.ulSize			= sensor_cfg.xtSENPath.ulFrameSize1;
//                                    tProcess.ulDramAddr1	= sensor_cfg.xtSENAddr.ulPath1_Addr;
//                                    if(osMessagePut(tSEN_ExtEventQueue, &tProcess, 0) != osOK)
//                                    {
//                                        ubBUF_ReleaseSenYuvBuf(sensor_cfg.xtSENAddr.ulPath1_Addr);
//                                        printf("SEN_Q->Full !!!!\r\n");
//                                    }
//                                    else
//                                        sensor_cfg.xtSENAddr.ulPath1_Addr = ulSEN_NextYuv1Addr;
//                                }
//                                else
//                                {
//                                    //ubBUF_ReleaseSenYuvBuf(sensor_cfg.xtSENAddr.ulPath1_Addr);
//                                }
//                                SEN->VIDEO_STR_EN_1 = (ulSEN_NextYuv1Addr == BUF_FAIL)? 0:1;
//                            }
//                        }
//                        else
//                        {
//                            ubBUF_ReleaseSenYuvBuf(sensor_cfg.xtSENAddr.ulPath1_Addr);
//                            SEN_SetResChgFlg(SENSOR_PATH1, FALSE);
//                        }
//                        tCvbsState.ubFieldMergeFin = 0;
//                    }
#endif
                }else{
                    if(ubSEN_GetScaleMode() == SEN_SCALEUP_MODE)
                    {
                        if((!ubSEN_GetResChgFlg(SENSOR_PATH1)) && (ubSEN_ScaleDropCnt >= ubSEN_ScaleDropFrame))
                        {
                            ulSEN_NextYuv1Addr = ulBUF_GetSen1YuvFreeBuf();
                            if(ulSEN_NextYuv1Addr != BUF_FAIL)
                            {
                                ubSrc = ubSEN_GetPathSrc(SENSOR_PATH1);
                                tProcess.ubSrcNum		= ubSrc;
                                tProcess.ubCurNode	 	= 0;
                                tProcess.ubNextNode 	= ubSEN_EventNode;
                                tProcess.ulSize			= sensor_cfg.xtSENPath.ulFrameSize1;
                                tProcess.ulDramAddr1	= SEN->STR1_STA << 8;	//YUV Address
                                sensor_cfg.xtSENAddr.ulPath1_Addr = tProcess.ulDramAddr1;
                                if(osMessagePut(tSEN_ExtEventQueue, &tProcess, 0) != osOK)
                                {
                                    ubBUF_ReleaseSenYuvBuf(SEN->STR1_STA << 8);
                                    printf("SEN_Q->Full !!!!\r\n");
                                }
                                else
                                    SEN->STR1_STA = ulSEN_NextYuv1Addr >> 8;
                                ubSEN_ScaleDropCnt = 0;
                                return;
                            }
                            else
                            {
                                //ubBUF_ReleaseSenYuvBuf(SEN->STR1_STA << 8);
                            }
                            SEN->VIDEO_STR_EN_1 = (ulSEN_NextYuv1Addr == BUF_FAIL)? 0:1;
                        }
                        else
                        {
                            ubSEN_ScaleDropCnt++;
                            ubBUF_ReleaseSenYuvBuf(SEN->STR1_STA << 8);
                            SEN_SetResChgFlg(SENSOR_PATH1, FALSE);
                        }
                    }else{
#if (defined(OP_STA) && (defined(BSP_VBM_SDK)||defined(BSP_RVCS_SDK)) &&defined(VDO_SUBPATH_ENABLE)&&(VDO_SUBPATH_ENABLE!=0))
                        if((!ubSEN_GetResChgFlg(SENSOR_PATH1)) && sensor_state.ubDetectVideo && !ubKNL_GetChgResFlg())
#else
                        if((!ubSEN_GetResChgFlg(SENSOR_PATH1)) && sensor_state.ubDetectVideo)
#endif
                        {
                    #if (defined(OP_AP) && defined(BSP_DVR_SDK))
                            //STORAGE keep 30fps.(don't frame drop)
							if(ubSEN_UvcPathFlag == 0)
								SEN_SetFrameRate(SENSOR_PATH1, ((ubREC_GetRecordMode() == REC_MODE_TIMELAPSE)? 1:30));
                    #endif
                            if(ubSEN_FrameDropState(SENSOR_PATH1))
                            {
                                if(ubSEN_CapScaleUp == 1)
                                {
                                    ubSEN_CapFrmCnt++;
                                    #if 0
                                    if(ubSEN_CapFrmCnt%3 == 1)
                                        ulSEN_NextYuv1Addr = ulSEN_CapAddr;
                                    else
                                        ulSEN_NextYuv1Addr = BUF_FAIL;
                                    #else
                                    ulSEN_NextYuv1Addr = ulSEN_CapAddr;
                                    #endif
                                }
                                else    
                                    ulSEN_NextYuv1Addr = ulBUF_GetSen1YuvFreeBuf();
                                if(ulSEN_NextYuv1Addr != BUF_FAIL)
                                {
									if(pSEN_HwEndIsrCbFunc[SENSOR_PATH1])
									{
										uint32_t ulYuvAddr = 0;
										ulYuvAddr = SEN->STR1_STA << 8;
										pSEN_HwEndIsrCbFunc[SENSOR_PATH1](SENSOR_PATH1, ulYuvAddr, sensor_cfg.xtSENPath.ulFrameSize1);
										sensor_cfg.xtSENAddr.ulPath1_Addr = ulYuvAddr;
										SEN->STR1_STA = ulSEN_NextYuv1Addr >> 8;
									}
									else
									{
										ubSrc = ubSEN_GetPathSrc(SENSOR_PATH1);
										tProcess.ubSrcNum		= ubSrc;
										tProcess.ubCurNode	 	= 0;
										tProcess.ubNextNode 	= ubSEN_EventNode;
										tProcess.ulSize			= sensor_cfg.xtSENPath.ulFrameSize1;
										tProcess.ulDramAddr1	= SEN->STR1_STA << 8;	//YUV Address
										sensor_cfg.xtSENAddr.ulPath1_Addr = tProcess.ulDramAddr1;
										if(ubSEN_CapScaleUp == 0 || (ubSEN_CapScaleUp == 1 && ubSEN_CapFrmCnt == 9) )
										{
    										if(osMessagePut(tSEN_ExtEventQueue, &tProcess, 0) != osOK)
    										{
    											ubBUF_ReleaseSenYuvBuf(SEN->STR1_STA << 8);
    											printf("SEN_Q->Full !!!!\r\n");
    										}
    										else
    										{
    										    if(ubSEN_CapScaleUp == 1 && ubSEN_CapFrmCnt == 9)
    										        return;
    											SEN->STR1_STA = ulSEN_NextYuv1Addr >> 8;
											}
										}
										else
											SEN->STR1_STA = ulSEN_NextYuv1Addr >> 8;
									}
                                }
                                else
                                {
                                    if(ubSEN_CapScaleUp != 1)
                                        ubBUF_ReleaseSenYuvBuf(SEN->STR1_STA << 8);
                                }
                                SEN->VIDEO_STR_EN_1 = (ulSEN_NextYuv1Addr == BUF_FAIL)? 0:1;
                            }
                        }
                        else
                        {
#if (defined(OP_STA) && (defined(BSP_VBM_SDK)||defined(BSP_RVCS_SDK)) &&defined(VDO_SUBPATH_ENABLE)&&(VDO_SUBPATH_ENABLE!=0))
                            if(!ubKNL_GetChgResFlg())
#endif
                            {
                                ubBUF_ReleaseSenYuvBuf(SEN->STR1_STA << 8);
                                SEN_SetResChgFlg(SENSOR_PATH1, FALSE);
                            }
                        }
                    }
                }
            }
            //Path2-Q
            if(ubSEN_GetActiveFlg(SENSOR_PATH2))
            {
                if(!ubSEN_GetResChgFlg(SENSOR_PATH2) && sensor_state.ubDetectVideo)
                {
                    if(ubSEN_FrameDropState(SENSOR_PATH2))
                    {
                        ulSEN_NextYuv2Addr = ulBUF_GetSen2YuvFreeBuf();
                        if(ulSEN_NextYuv2Addr != BUF_FAIL)
                        {
							if(pSEN_HwEndIsrCbFunc[SENSOR_PATH2])
							{
								uint32_t ulYuvAddr = 0;
								ulYuvAddr = SEN->STR2_STA << 8;
								pSEN_HwEndIsrCbFunc[SENSOR_PATH2](SENSOR_PATH2, ulYuvAddr, sensor_cfg.xtSENPath.ulFrameSize2);
								sensor_cfg.xtSENAddr.ulPath2_Addr = ulYuvAddr;
								SEN->STR2_STA = ulSEN_NextYuv2Addr >> 8;
							}
							else
							{
								ubSrc = ubSEN_GetPathSrc(SENSOR_PATH2);
								tProcess.ubSrcNum		= ubSrc;
								tProcess.ubCurNode	 	= 0;
								tProcess.ubNextNode 	= ubSEN_EventNode;
								tProcess.ulSize			= sensor_cfg.xtSENPath.ulFrameSize2;
								tProcess.ulDramAddr1	= SEN->STR2_STA << 8;	//YUV Address
								sensor_cfg.xtSENAddr.ulPath2_Addr = tProcess.ulDramAddr1;
								if(osMessagePut(tSEN_ExtEventQueue, &tProcess, 0) != osOK)
								{
									ubBUF_ReleaseSenYuvBuf(SEN->STR2_STA << 8);
									printf("SEN_Q->Full !!!!\r\n");
								}
								else
									SEN->STR2_STA = ulSEN_NextYuv2Addr >> 8;
							}
                        }
                        else
                        {
                            ubBUF_ReleaseSenYuvBuf(SEN->STR2_STA << 8);
                        }
                        SEN->VIDEO_STR_EN_2 = (ulSEN_NextYuv2Addr == BUF_FAIL)? 0:1;
                    }
                }
                else
                {
                    ubBUF_ReleaseSenYuvBuf(SEN->STR2_STA << 8);
                    SEN_SetResChgFlg(SENSOR_PATH2, FALSE);
                }
            }
            //Path3-Q
            if(ubSEN_GetActiveFlg(SENSOR_PATH3))
            {
                if(!ubSEN_GetResChgFlg(SENSOR_PATH3) && sensor_state.ubDetectVideo)
                {
                    if(ubSEN_FrameDropState(SENSOR_PATH3))
                    {
                        ulSEN_NextYuv3Addr = ulBUF_GetSen3YuvFreeBuf();
                        if(ulSEN_NextYuv3Addr != BUF_FAIL)
                        {
                            ubSrc = ubSEN_GetPathSrc(SENSOR_PATH3);
                            tProcess.ubSrcNum		= ubSrc;
                            tProcess.ubCurNode	 	= 0;
                            tProcess.ubNextNode 	= ubSEN_EventNode;
                            tProcess.ulSize			= sensor_cfg.xtSENPath.ulFrameSize3;
                            tProcess.ulDramAddr1	= SEN->STR3_STA << 8;	//YUV Address
                            sensor_cfg.xtSENAddr.ulPath3_Addr = tProcess.ulDramAddr1;
                            if(osMessagePut(tSEN_ExtEventQueue, &tProcess, 0) != osOK)
                            {
                                ubBUF_ReleaseSenYuvBuf(SEN->STR3_STA << 8);
                                printf("SEN_Q->Full !!!!\r\n");
                            }
                            else
                                SEN->STR3_STA = ulSEN_NextYuv3Addr >> 8;
                        }
                        else
                        {
                            ubBUF_ReleaseSenYuvBuf(SEN->STR3_STA << 8);
                        } 
                        SEN->VIDEO_STR_EN_3 = (ulSEN_NextYuv3Addr == BUF_FAIL)? 0:1;
                    }
                }
                else
                {
                    ubBUF_ReleaseSenYuvBuf(SEN->STR3_STA << 8);
                    SEN_SetResChgFlg(SENSOR_PATH3, FALSE);
                }
            }
        }
    }

    SEN->COLOR_TRIG_5 = 1;

    if(ubSEN_GetStateChangeFlg())
    {		
        SEN_SetStateChangeFlg(0);

        if(ubSEN_GetActiveFlg(SENSOR_PATH1))
        {
            SEN->VIDEO_STR_EN_1 = 1;
        }
        else
        {
            SEN->VIDEO_STR_EN_1 = 0;
        }

        if(ubSEN_GetActiveFlg(SENSOR_PATH2))
        {
            SEN->VIDEO_STR_EN_2 = 1;
        }
        else
        {
            SEN->VIDEO_STR_EN_2 = 0;
        }

        if(ubSEN_GetActiveFlg(SENSOR_PATH3))
        {
            SEN->VIDEO_STR_EN_3 = 1;
        }
        else
        {
            SEN->VIDEO_STR_EN_3 = 0;
        }
    }
	SEN->IMG_TX_EN = 1;	
	//================================================================================

	if (ubSEN_UvcPathFlag) {
        SEN_CheckUvcFormat();
	}

	if(!ubSEN_IspReadyFlg)
	{
		if(!(--ubSEN_ImgStabCnt))
		{
			if(pSEN_CbFunc)
				pSEN_CbFunc();
			ubSEN_IspReadyFlg = 1;
		}
	}
	if(!ubSEN_YsumReportReadyFlg)
	{
		if(!(--ubSEN_YsumStabCnt))
		{
            //SEN_SetEnvironment();
			ubSEN_YsumReportReadyFlg = 1;
		}
	}
}

//------------------------------------------------------------------------------
void SEN_Vsync_ISR(void)
{
	if((pSEN_IsrProcFunc[SEN_VSYNC_PROC]) && (SEN->IMG_TX_EN))
		pSEN_IsrProcFunc[SEN_VSYNC_PROC]();
    // Clear VSYNC flag.
	if( SEN->SEN_VSYNC_INT_FLAG )
	{
		SEN->SEN_VSYNC_INT_CLR = 1;
	} 
#if (defined(OP_STA) && (APP_USBD_COMPOSITE_MODE&USBD_MULTI_UVC))
	UI_IspCtSeting(0,0,0,1);	
#endif
	INTC_IrqClear(INTC_SEN_VSYNC_IRQ);
    ISP_ProtLsc();
    ubSEN_FirstVsync = 1;
	osSemaphoreRelease(SEM_SEN_VSyncRdy);
}

//------------------------------------------------------------------------------
void SEN_ParaInit(void)
{
    memset(&tCvbsState, 0, sizeof(SEN_CVBS_STATE_t));
    memset(&tSensorState, 0, sizeof(SEN_SENSOR_STATE_t));
    memset(&sensor_cfg.ubFrameRate ,30 ,sizeof(sensor_cfg.ubFrameRate));
    
    sensor_cfg.tScaleType = SEN_NORMAL_MODE;
    sensor_cfg.ulMaximumSensorFrameRate = 30;
    sensor_state.ubDetectVideo = 1;

    // AE call back function initial
    AE_CbFuncInit();
}

//------------------------------------------------------------------------------
void SEN_SensorInitial(void)
{
    // Sensor init
    tSensorState.ubSensorInit = ubSEN_Open(&sensor_cfg);
    // Set sensor type.
    SEN_SetSensorType();
    // Set sensor output size and ISP process size.
    SEN_SetISPWindowSize();
}

//------------------------------------------------------------------------------
#if (defined(OP_STA) || (defined(OP_AP) && defined(BSP_DVR_SDK)))
uint8_t ubSEN_InitProcess(void)
{
    static bool bSEN_InitFlg = FALSE;
	uint8_t ubSrc;

	if(bSEN_InitFlg == FALSE)
	{     
		sensor_state.ubInputType = SENSOR_NONO;
        SEN_ParaInit();        
		SEN_ISRInitial();
		SEN_SensorInitial();
		SEN_ISPInitial();
        // have interrupt, no video write to dram.
        SEN->VIDEO_STR_EN_1 = 0;
        SEN->VIDEO_STR_EN_2 = 0;
        SEN->VIDEO_STR_EN_3 = 0;
        SEN->IMG_TX_EN = 1;
        
		bSEN_InitFlg = TRUE;
	}

	//Path1
	ubSrc = ubSEN_GetPathSrc(SENSOR_PATH1);
	if(ubSrc != SEN_SRC_NONE)
	{
		SEN_SetResolution(SENSOR_PATH1, sensor_cfg.xtSENPath.uwHSize1, sensor_cfg.xtSENPath.uwVSize1);
	}
	//Path2
	ubSrc = ubSEN_GetPathSrc(SENSOR_PATH2);
	if(ubSrc != SEN_SRC_NONE)
	{
		SEN_SetResolution(SENSOR_PATH2, sensor_cfg.xtSENPath.uwHSize2, sensor_cfg.xtSENPath.uwVSize2);
	}	
	//Path3
	ubSrc = ubSEN_GetPathSrc(SENSOR_PATH3);
	if(ubSrc != SEN_SRC_NONE)
	{
		SEN_SetResolution(SENSOR_PATH3, sensor_cfg.xtSENPath.uwHSize3, sensor_cfg.xtSENPath.uwVSize3);
	}		
	
	// set video stream
	//Path1
	if(ubSEN_GetActiveFlg(SENSOR_PATH1))
	{
		SEN_SetPathState(SENSOR_PATH1, 1);
	}
	else
	{
		SEN_SetPathState(SENSOR_PATH1, 0);
	}

	//Path2
	if(ubSEN_GetActiveFlg(SENSOR_PATH2))
	{
		SEN_SetPathState(SENSOR_PATH2, 1);
	}
	else
	{
		SEN_SetPathState(SENSOR_PATH2, 0);
	}

	//Path3
	if(ubSEN_GetActiveFlg(SENSOR_PATH3))
	{
		SEN_SetPathState(SENSOR_PATH3, 1);
	}
	else
	{
		SEN_SetPathState(SENSOR_PATH3, 0);
	}
    
    return tSensorState.ubSensorInit;
}
#endif

//------------------------------------------------------------------------------
void SEN_ISRInitial(void)
{
    // Setup sensor relation IRQ
	SEM_SEN_VSyncRdy = NULL;
    SEM_MergeFrameRdy = NULL;
	SEN_VsyncInit();
	// Sensor HSYNC/VSYNC/HWEND IRQ	
	INTC_IrqSetup(INTC_SEN_HSYNC_IRQ, SEN_Hsync_ISR);
	INTC_IrqEnable(INTC_SEN_HSYNC_IRQ);		
	INTC_IrqSetup(INTC_ISP_WIN_END_IRQ, SEN_HwEnd_ISR);
	INTC_IrqEnable(INTC_ISP_WIN_END_IRQ);		
	INTC_IrqSetup(INTC_SEN_VSYNC_IRQ, SEN_Vsync_ISR);
	INTC_IrqEnable(INTC_SEN_VSYNC_IRQ);	
	// disable interrupt first
	SEN->HW_END_INT_EN = 0;
	SEN->SEN_VSYNC_INT_EN = 0;
	SEN->SEN_HSYNC_INT_EN = 0;	
}

//------------------------------------------------------------------------------
void SEN_VsyncInit(void)
{
	// Semaphore Create
	osSemaphoreDef(SEM_SEN_VSyncRdy);
	SEM_SEN_VSyncRdy    = osSemaphoreCreate(osSemaphore(SEM_SEN_VSyncRdy), 1);
	osSemaphoreDef(SEM_MergeFrameRdy);
	SEM_MergeFrameRdy    = osSemaphoreCreate(osSemaphore(SEM_MergeFrameRdy), 1);
	// Task Create
	osThreadDef(SEN_DoVsync, SEN_DoVsyncThread, osPriorityNormal, 1, 1024);
	osThreadCreate(osThread(SEN_DoVsync), NULL);
	osThreadDef(SEN_DoTest, SEN_ChkSensorStateThread, osPriorityNormal, 1, 1024);
	osThreadCreate(osThread(SEN_DoTest), NULL);
	osThreadDef(SEN_DoMerge, SEN_CvbsFieldMergeThread, osPriorityNormal, 1, 1024);
	osThreadCreate(osThread(SEN_DoMerge), NULL);    
}

//------------------------------------------------------------------------------
void SEN_ISPInitial(void)
{
	// AE and AWB report flag init
	ISP_Init();
	//Init AE,AWB,AF
    AE_Init();
    AWB_Init();
    AF_Init();
    IQ_DynamicInit();
    // parser IQ
	IQ_Init();
	// call back function
    IQ_SensorSlaveAddrCbFunc(ubSEN_SensorSlaveAddr);
    ISP_IqBypassCbFunc(ubSEN_GetByPassIqState);
    IQ_IspBypassStateCbFunc(ubSEN_GetByPassIqState);    
    IQ_ReadIQTable();
    //Set frame rate to 30fps
	IQ_SetDynFrameRate(30);
    // open 3DNR frame buffer compression
    ISP_Set3DNR_FBC();
    // ISP data stamp relation
#if (defined(BSP_DVR_SDK) && (defined(OP_AP) || (defined(OP_STA)&&defined(APP_TXREC_STREAM_SEL)&&(APP_TXREC_STREAM_SEL!=0))))
    ISP_DateStampInit(1);
#elif (defined(BSP_VBM_SDK) && defined(OP_STA) && defined(APP_TXREC_STREAM_SEL) && (APP_TXREC_STREAM_SEL!=0))
    ISP_DateStampInit(1);
#else
    ISP_DateStampInit(0);
#endif
    ISP_SetOsdSwitch(DS_PATH1, DS_OFF);
    ISP_LoadOsdFontTable(DS_PATH1, &ullDS_FontTable[0], 0, 0);
    ISP_SetOsdSwitch(DS_PATH1, DS_ON);
#if (defined(BSP_DVR_SDK) && (defined(OP_AP) || (defined(OP_STA)&&defined(APP_TXREC_STREAM_SEL)&&(APP_TXREC_STREAM_SEL!=0))))
    ISP_SetOsdSwitch(DS_PATH2, DS_OFF);
    ISP_LoadOsdFontTable(DS_PATH2, &ullDS_FontTable[0], 0, 0);
    ISP_SetOsdSwitch(DS_PATH2, DS_ON);
#elif (defined(BSP_VBM_SDK) && defined(OP_STA) && defined(APP_TXREC_STREAM_SEL) && (APP_TXREC_STREAM_SEL!=0))
    ISP_SetOsdSwitch(DS_PATH2, DS_OFF);
    ISP_LoadOsdFontTable(DS_PATH2, &ullDS_FontTable[0], 0, 0);
    ISP_SetOsdSwitch(DS_PATH2, DS_ON);
#endif
    
#if (defined(BSP_DVR_SDK)||(defined(OP_STA) && defined(RVCS_APP)))
    ubISP_SetScaleMode(ISP_SCALE_NORMAL_MODE);  	
#else
    if((sensor_cfg.ubSensorType == SEN_TP9950) || (sensor_cfg.ubSensorType == SEN_RN6752))
        ubISP_SetScaleMode(ISP_SCALE_NORMAL_MODE);
    else
        ubISP_SetScaleMode(ISP_SCALE_SAME_RATIO); 
#endif
}

#if (defined(OP_STA) || (defined(OP_AP) && defined(BSP_DVR_SDK)))
//------------------------------------------------------------------------------
void SEN_LoadIQData(void)
{
	IQ_LoadDataFromStorage();
}
#endif

//------------------------------------------------------------------------------
void SEN_SetResolution(uint8_t ubPath, uint32_t ulWidth, uint32_t ulHeight)
{
	// Set sensor struct value.
	SEN_SetOutResolution(ubPath, ulWidth, ulHeight);
    // Set scaler FIR
    IQ_SetResolution_SDK(ulWidth, ulHeight, ubIQ_GetDynFrameRate(), ubPath);
    // Set current preview ulWidth size
	IQ_SetISPRes();
}

//------------------------------------------------------------------------------
void SEN_SetISPRate(uint8_t ubISP_div)
{
    GLB->ISP_RATE = ubISP_div;	
}

//------------------------------------------------------------------------------
void SEN_SetAXIRate(uint8_t ubAXI_div)
{
    GLB->ISP_AXI_RATE = ubAXI_div;
}

//------------------------------------------------------------------------------
void SEN_SetSensorRate(uint8_t ubSelBaseClk, uint8_t ubSensorDiv)
{
	SEN->SEN_BASE_CLK_SEL = ubSelBaseClk;
	SEN->SEN_RATE = ubSensorDiv;
    
    sensor_cfg.ulSensorMclk = (SEN->SEN_BASE_CLK_SEL == 0)? (96000000/ubSensorDiv):(120000000/ubSensorDiv);	
}

//------------------------------------------------------------------------------
static void SEN_ChkSensorStateThread(void const *argument)
{
#if ((SEN_USE == SEN_RN6752) || (SEN_USE == SEN_TP9950))
    SEN_STATE tRpoState;
	uint8_t ubRptData[3];
	tRpoState.ubVideoFormat = 0xFF;
	tRpoState.ubVideoType	= 0xFF;
	tRpoState.ubVideoFPS	= 0xFF;
#endif	
	while(1)
    {
        if(tSensorState.ubSensorInit == SEN_InitFail)
        {
            SEN_SetStateChangeFlg(0);
        }
        tSensorState.ubSensorTrig = 1;
        tCvbsState.ubFieldSync = 1;
#if ((SEN_USE == SEN_RN6752) || (SEN_USE == SEN_TP9950)) 
    #if (defined(RTC676x))		
		if(ubKNL_GetRtCommLinkStatus(0))
    #elif (defined(A7130))			
		if(ubKNL_GetRtCommLinkStatus(KNL_MASTER_AP))
    #else
		if (1)
    #endif		
		{			
			if(sensor_state.ubVliadVideo == SEN_ValidVideo)
			{
				if((tRpoState.ubVideoFormat != sensor_state.ubOldVideoFormat) ||
				   (tRpoState.ubVideoType   != sensor_state.ubOldVideoType) ||
				   (tRpoState.ubVideoFPS    != sensor_state.ubOldVideoFPS))
				{
					if(ubSEN_SetDefMaxResolution)
					{
						if((uwSEN_DefH == 1280) && (uwSEN_DefV == 720) && (sensor_state.ubOldVideoFormat == 2))
						{
							ubRptData[0] = 1;	
						}
						else
						{
							ubRptData[0] = sensor_state.ubOldVideoFormat;
						}		
						ubRptData[1] = sensor_state.ubOldVideoType;
						ubRptData[2] = sensor_state.ubOldVideoFPS;
						tTWC_Send(TWC_AP_MASTER,TWC_TX_AHD_RPT,ubRptData,3,50);
						
						tRpoState.ubVideoFormat =  sensor_state.ubOldVideoFormat;
						tRpoState.ubVideoType	=  sensor_state.ubOldVideoType;
						tRpoState.ubVideoFPS	=  sensor_state.ubOldVideoFPS;
					}
				}
			}
		}
		else
		{
			tRpoState.ubVideoFormat =  0xFF;
			tRpoState.ubVideoType	=  0xFF;
			tRpoState.ubVideoFPS	=  0xFF;
		}
#endif	
        osDelay(1000);
    }
}

//------------------------------------------------------------------------------
void SEN_ImgStableFunc(void)
{
    AE_SetCtrlTable();
    AE_SetIqValue();
    AWB_SetCtrlTable();
    AE_SetIvalue(ubAE_GetPID_Ivalue());
    sensor_cfg.ulMaximumSensorFrameRate = ubIQ_GetIQBinFrameRate();
    IQ_SetDynFrameRate(ubIQ_GetIQBinFrameRate());
}

static void SEN_DoVsyncThread(void const *argument)
{
    uint8_t ubOldSensorFPS = 0xff;
    uint8_t ubDropCnt = 0;
    uint8_t ub3ACtrlTableCnt = 0;
    uint8_t ub3ACtrlTableFg = FALSE;
    uint8_t ubAlgReportRdyFg = FALSE;
	// update ISP after Vsync	
	while(1) {
		osSemaphoreWait(SEM_SEN_VSyncRdy ,osWaitForever);
        if(ubSEN_FirstVsync == 1)
        {
            if (ubSEN_GetByPassIqState() == SEN_LOAD_IQ)
            {
                // using initial AE value when AE unstable.(accelerate AE stable)
                if(ub3ACtrlTableFg == FALSE)
                {
                    if(ub3ACtrlTableCnt > 7)
                    {
                        SEN_ImgStableFunc();
                        ub3ACtrlTableFg = TRUE;
                    }
                    ub3ACtrlTableCnt++;
                }
                // drop "ISP_ALG_REPORT_CNT" frame then process AE, AWB and dynamic IQ alg.
                if(ubDropCnt >= ISP_ALG_REPORT_CNT)
                {
                    ubAlgReportRdyFg = TRUE;
                }else{
                    ubDropCnt++;
                }
                
                if(ubAlgReportRdyFg)
                {
                    AWB_VSync();
                    AE_VSync();
                    IQ_DynamicVSync();
                }
                AF_VSync();
            }
            //
            if(tSensorState.ubSensorTrig)
            {
                tSensorState.ubSensorState = ubSEN_CheckSensorState();                
                tSensorState.ubSensorTrig = 0;
            }
            if((sensor_cfg.ubSensorType == SEN_TP9950)||(sensor_cfg.ubSensorType == SEN_RN6752))
            {
                tSensorState.ubSensorFPS = (sensor_state.ubVideoFPS == 1)?30:25;
                sensor_cfg.ulMaximumSensorFrameRate = tSensorState.ubSensorFPS;
            }else{
                if(ubAE_GetPwrFreq() == SENSOR_PWR_FREQ_50HZ)
                {
                    tSensorState.ubSensorFPS = (ubAE_GetCurrExpIdx() == 0)?ubIQ_GetDynFrameRate():(((100 / ubAE_GetCurrExpIdx()) > ubIQ_GetDynFrameRate())?ubIQ_GetDynFrameRate():(100 / ubAE_GetCurrExpIdx()));
                }else{
                    tSensorState.ubSensorFPS = (ubAE_GetCurrExpIdx() == 0)?ubIQ_GetDynFrameRate():(((120 / ubAE_GetCurrExpIdx()) > ubIQ_GetDynFrameRate())?ubIQ_GetDynFrameRate():(120 / ubAE_GetCurrExpIdx()));
                }            
            }
            

            if(ubOldSensorFPS != tSensorState.ubSensorFPS)
            {
                SEN_UpdateFrameDropTable();
                ubOldSensorFPS = tSensorState.ubSensorFPS;
            }
            //
            tCvbsState.ubFieldState ^= 1;
            if(tCvbsState.ubFieldSync)
            {
                if(pSEN_SyncCbFunc != NULL)
                {
                    tCvbsState.ubFieldState = pSEN_SyncCbFunc();
                }
                tCvbsState.ubFieldSync = 0;
            }
            //Check ISP state
            SEN_ChkISPState();
        }
	}
}

static void SEN_CvbsFieldMergeThread(void const *argument)
{
    uint16_t i;
    while(1)
    {
        osSemaphoreWait(SEM_MergeFrameRdy ,osWaitForever);
        
        if(sensor_cfg.ubSensorType == SEN_TP9950)
        {
//          ***Must set ubISP_SetScaleMode(ISP_SCALE_NORMAL_MODE);***
//          TP9950 CVBS send frame image (top is even field and bottom is odd field)
            for(i=0;i<((sensor_cfg.xtSENPath.uwVSize1/2)/2);i++)
            {
                tDMAC_MemCopy(sensor_cfg.xtSENAddr.ulIMG_TEMP1_Addr+i*sensor_cfg.xtSENPath.uwHSize1*3+(sensor_cfg.xtSENPath.uwHSize1*(sensor_cfg.xtSENPath.uwVSize1/2)*3/2), sensor_cfg.xtSENAddr.ulPath1_Addr+sensor_cfg.xtSENPath.uwHSize1*(2*i)*3, sensor_cfg.xtSENPath.uwHSize1*3, NULL);
                tDMAC_MemCopy(sensor_cfg.xtSENAddr.ulIMG_TEMP1_Addr+i*sensor_cfg.xtSENPath.uwHSize1*3, sensor_cfg.xtSENAddr.ulPath1_Addr+sensor_cfg.xtSENPath.uwHSize1*(2*i+1)*3, sensor_cfg.xtSENPath.uwHSize1*3, NULL);
            }
            tCvbsState.ubFieldMergeFin = 1;

            if(ubKNL_GetSenThenEncEnable())
            {
                if(!ubKNL_GetChgResFlg())
                {    
                    if(pSEN_HwEndIsrCbFunc[SENSOR_PATH1] != NULL)
                    {
                        pSEN_HwEndIsrCbFunc[SENSOR_PATH1](SENSOR_PATH1, sensor_cfg.xtSENAddr.ulPath1_Addr, sensor_cfg.xtSENPath.ulFrameSize1);
                    }       
                }
            }
        }else if(sensor_cfg.ubSensorType == SEN_RN6752){
//          RN6752 CVBS send field image(even field or odd field)
            if(tCvbsState.ubFieldState)   
            {
                for(i=0;i<((sensor_cfg.xtSENPath.uwVSize1/2)/2);i++)
                {
                    tDMAC_MemCopy(sensor_cfg.xtSENAddr.ulIMG_TEMP1_Addr+i*sensor_cfg.xtSENPath.uwHSize1*3, sensor_cfg.xtSENAddr.ulPath1_Addr+sensor_cfg.xtSENPath.uwHSize1*(2*i+1)*3, sensor_cfg.xtSENPath.uwHSize1*3, NULL);
                }
                tCvbsState.ubFieldMergeFin = 1;
                if(ubKNL_GetSenThenEncEnable())
                {
                    if(!ubKNL_GetChgResFlg())
                    {
                        if(pSEN_HwEndIsrCbFunc[SENSOR_PATH1] != NULL)
                        {
                            pSEN_HwEndIsrCbFunc[SENSOR_PATH1](SENSOR_PATH1, sensor_cfg.xtSENAddr.ulPath1_Addr, sensor_cfg.xtSENPath.ulFrameSize1);
                        }       
                    }
                }
            }else{
                for(i=0;i<((sensor_cfg.xtSENPath.uwVSize1/2)/2);i++)
                {
                    tDMAC_MemCopy(sensor_cfg.xtSENAddr.ulIMG_TEMP1_Addr+i*sensor_cfg.xtSENPath.uwHSize1*3, sensor_cfg.xtSENAddr.ulPath1_Addr+sensor_cfg.xtSENPath.uwHSize1*(2*i)*3, sensor_cfg.xtSENPath.uwHSize1*3, NULL);
                }
            }
        }	
    }
}

//------------------------------------------------------------------------------
uint8_t ubSEN_GetSensorType(void)
{
    return sensor_cfg.ubSensorType;
}

//------------------------------------------------------------------------------
uint8_t ubSEN_GetMaxFrameRate(void)
{
    return sensor_cfg.ulMaximumSensorFrameRate;
}

//------------------------------------------------------------------------------
uint8_t ubSEN_GetByPassIqState(void)
{
    return sensor_cfg.ubLoadIQState = IQ_BIN_STATE;
}

//------------------------------------------------------------------------------
uint8_t ubSEN_GetSensorState(void)
{
    return tSensorState.ubSensorState;
}

//------------------------------------------------------------------------------
uint8_t ubSEN_GetDetectVideoState(void)
{
    return sensor_state.ubDetectVideo;
}

//------------------------------------------------------------------------------
void SEN_SetDetectVideoState(uint8_t ubState)
{
    sensor_state.ubDetectVideo = ubState;
}

//------------------------------------------------------------------------------
uint32_t ulSEN_IQBinAddr(void)
{
    return sensor_cfg.xtSENAddr.ulIQBin_Addr;
}

//------------------------------------------------------------------------------
void SEN_SetScaleMode(SCALE_TYPE tMode)
{
	sensor_cfg.tScaleType = tMode;
}

//------------------------------------------------------------------------------
uint8_t ubSEN_GetScaleMode(void)
{
	return sensor_cfg.tScaleType;
}

//------------------------------------------------------------------------------//
uint8_t ubSEN_SensorSlaveAddr(void)
{
    return SEN_SLAVE_ADDR;
}

//------------------------------------------------------------------------------
void SEN_SetISPTestPattern(uint8_t ubSwitch)
{
    if(ubSwitch)
    {
        if(sensor_cfg.tPathType == BAYER_SENSOR_NORMAL)
            SEN->TEST_PAT_TYPE = SEN_RAW_PATTERN;
        else
            SEN->TEST_PAT_TYPE = SEN_YUV_PATTERN;
    }
    else
        SEN->TEST_PAT_TYPE = SEN_NORMAL;
}

//------------------------------------------------------------------------------
void SEN_ChkISPState(void)
{
	uint16_t uwCheckValue;    
	// for debug
	uwCheckValue = SEN->REG_0x10F0;
	if(uwCheckValue & 0x3)
	{
		if(uwCheckValue & 0x1)
			printf("Scale BF\n");
		if(uwCheckValue & 0x2)
			printf("Scale TEr\n");

		SEN->REG_0x10F0 = 0xc;
	}
	uwCheckValue = SEN->REG_0x1300;
	if(uwCheckValue & 0x1ff)
	{
		if(uwCheckValue & 0x1)
			printf("path1 BF\n");
		if(uwCheckValue & 0x2)
			printf("path2 BF\n");
		if(uwCheckValue & 0x4)
			printf("path3 BF\n");
		if(uwCheckValue & 0x8)
			printf("3DNR WF\n");	
		if(uwCheckValue & 0x10)
			printf("3DNR RdE\n");	  
		if(uwCheckValue & 0x20)
			printf("MD WrF\n");
		if(uwCheckValue & 0x40)
			printf("MD RdE\n");
		if(uwCheckValue & 0x80)
			printf("MD W1 WrF\n");
		if(uwCheckValue & 0x100)
			printf("MD W2 WrF\n");
      
		SEN->REG_0x1300 = 0x1ff;
	}
	uwCheckValue = SEN->REG_0x1304;
	if(uwCheckValue & 0x07)
	{
		if(uwCheckValue & 0x01)
			printf("path1 FIFO full!!\n");
		if(uwCheckValue & 0x02)
			printf("path2 FIFO full!!\n");
		if(uwCheckValue & 0x04)
			printf("path3 FIFO full!!\n");		        
    }
}

//------------------------------------------------------------------------------
void SEN_SetPathState(uint8_t ubPath, uint8_t ubFlag)
{
	switch(ubPath)
	{
		case SENSOR_PATH1:
			ubSEN_VIDEO[0] = ubFlag;		
			break;
		case SENSOR_PATH2:
			ubSEN_VIDEO[1] = ubFlag;
			break;
		case SENSOR_PATH3:
			ubSEN_VIDEO[2] = ubFlag;
			break;
		default:
			break;			
	}
}

//------------------------------------------------------------------------------
void SEN_SetIspOutEn(uint8_t ubEn)
{
	SEN->HW_END_INT_EN = ubEn;
	SEN->IMG_TX_EN = ubEn;
	SEN->VIDEO_STR_EN_1 = (ubEn)?ubSEN_VIDEO[0]:0;
	SEN->VIDEO_STR_EN_2 = (ubEn)?ubSEN_VIDEO[1]:0;
	SEN->VIDEO_STR_EN_3 = (ubEn)?ubSEN_VIDEO[2]:0;
}

//------------------------------------------------------------------------------
void SEN_SetRawReorder(uint8_t ubMirrorEn, uint8_t ubFlipEn)
{
	if (ubMirrorEn && ubFlipEn) {
        ISP_SetBlockGainTable(3);
		SEN->RAW_REORDER = ((ulIQ_GetIspReorderPattern() >> 24) & 0xff);
	} else if (ubFlipEn) {
        ISP_SetBlockGainTable(1);
		SEN->RAW_REORDER = ((ulIQ_GetIspReorderPattern() >> 16) & 0xff);
	} else if (ubMirrorEn) {
        ISP_SetBlockGainTable(2);
		SEN->RAW_REORDER = ((ulIQ_GetIspReorderPattern() >> 8) & 0xff);
	} else {
        ISP_SetBlockGainTable(0);
		SEN->RAW_REORDER = ((ulIQ_GetIspReorderPattern() >> 0) & 0xff);
	}	
}

//------------------------------------------------------------------------------
uint8_t SEN_ReportAhdCamStatus(void)
{
#if ((SEN_USE == SEN_RN6752) || (SEN_USE == SEN_TP9950)) 
	return sensor_state.ubVliadVideo;
#else
	return 0;
#endif
}

//------------------------------------------------------------------------------//
//                         sensor output type                                   //
//------------------------------------------------------------------------------//
//------------------------------------------------------------------------------
void SEN_CheckUvcFormat(void)
{
	if(UVC_GetVdoFormat() == USB_UVC_VS_FORMAT_UNCOMPRESSED)
	{
        SEN_SenosrOutputType(BAYER_SENSOR_RAW_DATA);
        SEN_SetFrameRate(SENSOR_PATH1, 1);
	}else{
        SEN_SenosrOutputType(sensor_cfg.tPathType);
        SEN_SetFrameRate(SENSOR_PATH1, 30);
	}
}

//------------------------------------------------------------------------------
void SEN_SenosrOutputType(ISP_PIPE_TYPE tType)
{
    SEN->BS_BYPASS = 0;   
    if(BAYER_SENSOR_NORMAL == tType)
    {
        SEN->RAW_BP_EN = 0;
        SEN->RAW_BP_MODE = 1; 
        SEN->ISP_MODE = 0;
        SEN->ISP_BRIG = 0;
    }else if(BAYER_SENSOR_RAW_DATA == tType){
        SEN->RAW_BP_EN = 1;
        SEN->RAW_BP_MODE = 1; 
        SEN->ISP_MODE = 0;
        SEN->ISP_BRIG = 0; 
    }else if(YUV_SENSOR_RGB_IN == tType){
        SEN->RAW_BP_EN = 0;
        SEN->RAW_BP_MODE = 0; 
        SEN->ISP_MODE = 1;
        SEN->ISP_BRIG = 0; 
    }else if(YUV_SENSOR_YUV_IN == tType){
        SEN->RAW_BP_EN = 0;
        SEN->RAW_BP_MODE = 0; 
        SEN->ISP_MODE = 1;
        SEN->ISP_BRIG = 1; 
    }
}

//------------------------------------------------------------------------------//
//                         window size                                          //
//------------------------------------------------------------------------------//
//------------------------------------------------------------------------------
void SEN_SetISPWindowSize(void)
{
	SEN_SetSensorImageSize();
    
	SEN->H_START    = sensor_cfg.xtSENWin.uwHStart;
    SEN->V_START    = sensor_cfg.xtSENWin.uwVStart;
	SEN->H_SIZE     = sensor_cfg.xtSENWin.uwHSize >> 1;
	SEN->V_SIZE     = sensor_cfg.xtSENWin.uwVSize >> 1;
}

//------------------------------------------------------------------------------//
//                         ISP frame drop                                       //
//------------------------------------------------------------------------------//
//------------------------------------------------------------------------------
void SEN_SetFrameDrop(uint8_t ubPath, uint8_t ubEnable, uint8_t ubN, uint8_t ubM)
{
    if(SENSOR_PATH1 == ubPath)
    {
        SEN->FRM_DROP_SCALER1_M = ubM;
        SEN->FRM_DROP_SCALER1_N = ubN;        
        SEN->FRM_DROP_SCALER1_EN = ubEnable;
    }else if(SENSOR_PATH2 == ubPath){
        SEN->FRM_DROP_SCALER2_M = ubM;
        SEN->FRM_DROP_SCALER2_N = ubN;
        SEN->FRM_DROP_SCALER2_EN = ubEnable;
    }else if(SENSOR_PATH3 == ubPath){
        SEN->FRM_DROP_SCALER3_M = ubM;
        SEN->FRM_DROP_SCALER3_N = ubN;        
        SEN->FRM_DROP_SCALER3_EN = ubEnable;
    }
}

//------------------------------------------------------------------------------
bool bSEN_GetFrameDropState(uint8_t ubPath)
{
    if(SENSOR_PATH1 == ubPath)
    {
        return SEN->FRM_DROP1_INF;
    }else if(SENSOR_PATH2 == ubPath){
        return SEN->FRM_DROP2_INF;
    }else if(SENSOR_PATH3 == ubPath){
        return SEN->FRM_DROP3_INF;
    }
    return 1;
}

//------------------------------------------------------------------------------//
//                         FW frame drop                                        //
//------------------------------------------------------------------------------//
//------------------------------------------------------------------------------
uint64_t ullSEN_FrameDropUseMN(uint8_t m, uint8_t n)
{
    float fBase, fValue;
    uint8_t i;    
    uint8_t ubCnt = 1;
    uint64_t ullDropTable = 0;
    
    if((m>n) || (m==0) || (n==0))
        return 0xffffffffffffffff;
    
    fBase = (float)m/n;
   
    for(i=0;i<n;i++)
    {
        fValue = fBase * (i+1);
        
        if(ubCnt == floor(fValue))
        {
            ubCnt++;
            ullDropTable = ullDropTable + ((uint64_t)1<<i);
        }else{
            ullDropTable = ullDropTable + ((uint64_t)0<<i);
        }
    }
    return ullDropTable;
}

//------------------------------------------------------------------------------
void SEN_UpdateFrameDropTable(void)
{
    uint8_t i;
    for(i=0;i<3;i++)
    {
        sensor_cfg.ullDropTable[i] = ullSEN_FrameDropUseMN(sensor_cfg.ubFrameRate[i], tSensorState.ubSensorFPS);
    }
}

//------------------------------------------------------------------------------
#if (defined(OP_STA) || (defined(OP_AP) && defined(BSP_DVR_SDK)))
void SEN_SetFrameRate(uint8_t ubPath, uint8_t ubFPS)
{
    static uint8_t ubOldPath1FrameRate = 0;
    static uint8_t ubOldPath2FrameRate = 0;
    static uint8_t ubOldPath3FrameRate = 0;
    
    ubFPS = ((ubFPS > ubSEN_GetMaxFrameRate()) || (ubFPS == 0))? ubSEN_GetMaxFrameRate() : ubFPS;
    if(SENSOR_PATH1 == ubPath)
    {
        if(ubOldPath1FrameRate == ubFPS)
            return;
        ubOldPath1FrameRate = ubFPS;
        sensor_cfg.ubFrameRate[0] = ubFPS;
    }else if(SENSOR_PATH2 == ubPath){
        if(ubOldPath2FrameRate == ubFPS)
            return;
        ubOldPath2FrameRate = ubFPS;
        sensor_cfg.ubFrameRate[1] = ubFPS;
    }else if(SENSOR_PATH3 == ubPath){
        if(ubOldPath3FrameRate == ubFPS)
            return;
        ubOldPath3FrameRate = ubFPS;
        sensor_cfg.ubFrameRate[2] = ubFPS;
    }
    
    SEN_UpdateFrameDropTable();
}
#endif
//------------------------------------------------------------------------------
uint8_t ubSEN_FrameDropState(uint8_t ubPath)
{
    static uint8_t ubDropCount1 = 0, ubDropCount2 = 0, ubDropCount3 = 0;      
    uint8_t ubState = 0;

    if(SENSOR_PATH1 == ubPath)
    {
        ubState = ((sensor_cfg.ullDropTable[0] >> ubDropCount1)& 0x01);
        ubDropCount1++;
        if (ubDropCount1 >= tSensorState.ubSensorFPS)
            ubDropCount1 = 0;
    }else if(SENSOR_PATH2 == ubPath){  
        ubState = ((sensor_cfg.ullDropTable[1] >> ubDropCount2)& 0x01);
        ubDropCount2++;
        if (ubDropCount2 >= tSensorState.ubSensorFPS)
            ubDropCount2 = 0;
    }else if(SENSOR_PATH3 == ubPath){   
        ubState = ((sensor_cfg.ullDropTable[2] >> ubDropCount3)& 0x01);
        ubDropCount3++;
        if (ubDropCount3 >= tSensorState.ubSensorFPS)
            ubDropCount3 = 0;
    }
    return ubState;
}

//------------------------------------------------------------------------------//
//                         MIPI                                                 //
//------------------------------------------------------------------------------//
//------------------------------------------------------------------------------
void SEN_MIPIControl(MIPI_LANE_NUM_TYPE tLaneNumber, MIPI_CHANNEL_NUM_TYPE tChannelSelect)
{
        MIPI->MIPI_EN = MIPI_DISABLE;
        //MIPI->MIPI_PD = 0;
        if(tLaneNumber == MIPI_1LANE)
        {
            MIPI->DLAN_NUM = MIPI_1LANE;
            MIPI->DATA_CHANNEL0_SEL = tChannelSelect;
            MIPI->LANE0_CLK_SEL = 0;
        }else if(tLaneNumber == MIPI_2LANE){
            MIPI->DLAN_NUM = MIPI_2LANE;
            if(tChannelSelect == MIPI_CHANNEL0)
            {
                MIPI->DATA_CHANNEL0_SEL = 0;
                MIPI->DATA_CHANNEL1_SEL = 1; 
                MIPI->LANE0_CLK_SEL = 0;
                MIPI->LANE1_CLK_SEL = 0; 
                MIPI->LANE2_CLK_SEL = 0; 
                MIPI->LANE3_CLK_SEL = 1;
            }else if(tChannelSelect == MIPI_CHANNEL1){
                MIPI->DATA_CHANNEL0_SEL = 2;
                MIPI->DATA_CHANNEL1_SEL = 3; 
                MIPI->LANE0_CLK_SEL = 0;
                MIPI->LANE1_CLK_SEL = 1; 
                MIPI->LANE2_CLK_SEL = 0; 
                MIPI->LANE3_CLK_SEL = 0;
            }
        } 
        MIPI->MIPI_EN = MIPI_ENABLE;
}

//------------------------------------------------------------------------------//
//                         CB func                                              //
//------------------------------------------------------------------------------//
//------------------------------------------------------------------------------
void SEN_SetIspFinishCbFunc(pvSEN_CbFunc pvCB)
{
	pSEN_CbFunc = pvCB;
}

//------------------------------------------------------------------------------
void SEN_SyncCvbsFieldCbFunc(pvSEN_SyncCbFunc pvCB)
{
	pSEN_SyncCbFunc = pvCB;
}	

//------------------------------------------------------------------------------
void SEN_SetHwEndIsrCbFunc(uint8_t ubPath, pvSEN_HwEndIsrCbFunc pvCB)
{
	if(ubPath > SENSOR_PATH3)
		return;
	pSEN_HwEndIsrCbFunc[ubPath] = pvCB;
}
//------------------------------------------------------------------------------
void SEN_RegIsrProcCbFunc(SEN_IsrProcType_t tProcType, pSEN_IsrProcCb cb)
{
	if(tProcType > SEN_ISPOUT_PROC)
		return;
	pSEN_IsrProcFunc[tProcType] = cb;
}
//------------------------------------------------------------------------------
void SEN_SetAhdCamResCbFunc(pvSEN_CbAhdSetResFunc pvCB)
{
	if(pSEN_CbAhdSetResFunc == NULL)
		pSEN_CbAhdSetResFunc = pvCB;
}
//------------------------------------------------------------------------------
void SEN_SetAhdCamBufCbFunc(pvSEN_CbAhdSetBufFunc pvCB)
{
	if(pSEN_CbAhdSetBufFunc == NULL)
		pSEN_CbAhdSetBufFunc = pvCB;
}
//------------------------------------------------------------------------------
uint8_t SEN_GetAhdCamType(void)
{
	//printf("sensor_state.ubInputType = %d\n",sensor_state.ubInputType);
	return sensor_state.ubInputType;
}	
//------------------------------------------------------------------------------
uint32_t SEN_Get16MCvbsAddress(void)
{
	return ulSEN_Cvbs16MDataAddr;
}
//------------------------------------------------------------------------------
void SEN_SetYuvBufNub(uint8_t ubNb)
{
	ubSEN_BufNub = ubNb;
}
//------------------------------------------------------------------------------
void SEN_SetAhdCamRes(uint16_t uwH, uint16_t uwV)
{
	if(pSEN_CbAhdSetResFunc != NULL)
		pSEN_CbAhdSetResFunc(uwH,uwV);
}	
//------------------------------------------------------------------------------
void SEN_SetAhdCamBuf(uint16_t uwH, uint16_t uwV)
{
	if(pSEN_CbAhdSetBufFunc != NULL)
		pSEN_CbAhdSetBufFunc(uwH,uwV);
}
void SEN_CaptureScaleUp(uint8_t ubEn, uint32_t ulAddr)
{
    ubSEN_CapScaleUp = ubEn;
    ulSEN_CapAddr = ulAddr;
    ubSEN_CapFrmCnt = 0;    
    if(ubEn)
    {
        ubISP_SetScaleMode(ISP_SCALE_SAME_RATIO);
        ubSEN_IspRate_Backup = GLB->ISP_RATE;
        ubSEN_AxiRate_Backup = GLB->ISP_AXI_RATE;
        GLB->ISP_RATE = 3;
        GLB->ISP_AXI_RATE = 3;
    }
    else
    {
        ubISP_SetScaleMode(ISP_SCALE_NORMAL_MODE);  	
        GLB->ISP_RATE = ubSEN_IspRate_Backup;
        GLB->ISP_AXI_RATE = ubSEN_AxiRate_Backup;
    }
}
uint8_t ubSEN_CaptureScaleUpGet(void)
{
    return ubSEN_CapScaleUp;
}


/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		Buzzer.c
	\brief		Buzzer function
	\author		Ocean
	\version	0.4
	\date		2020/05/04
	\copyright	Copyright(C) 2020 SONiX Technology Co., Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------
#include <math.h>
#include <stdio.h>
#include "Buzzer.h"
#include "ADO_API.h"
#include "APP_CFG_SEL.h"
#include "KNL.h"
#include "UI_BUCCU[WSVGA].h"


typedef struct
{
	uint8_t ubWeight;
	BUZ_ToneParam_t *pToneParam;
	uint8_t ubToneGroupSize;
	uint8_t ubChannelCnt;
}BUZ_META_INFO_t;

osMessageQId BUZ_MesgQ = NULL;
//osThreadId BUZ_PlayId = NULL;
BUZ_META_INFO_t BUZ_Meta = {0};
uint8_t ubBUZ_PlyStopFlg = 0;

osThreadId BUZ_PlayId = NULL;
osThreadDef(BUZ_PlayThread, BUZ_PlayThread, osPriorityAboveNormal, 1, 512);
osSemaphoreId BUZ_Sema;
osSemaphoreDef(BUZ_Sema);


//------------------------------------------------------------------------------
void BUZ_PlayStart(uint8_t ubChCnt, uint8_t ubWeight, BUZ_ToneParam_t *pToneParam, uint8_t ubToneGroupSize)
{
#if APP_ADO_FUNC_ENABLE
	KNL_StoreDbgInfo(__FUNCTION__,__LINE__);
	
	if(BUZ_MesgQ==NULL)
	{
		osMessageQDef(BUZ_MesgQ, 1, BUZ_Meta);
		BUZ_MesgQ = osMessageCreate(osMessageQ(BUZ_MesgQ), NULL);
	}
	
	if(BUZ_PlayId == NULL)
	{
		osThreadDef(BUZ_PlayThread, BUZ_PlayThread, osPriorityAboveNormal, 1, 512);
		BUZ_PlayId = osThreadCreate(osThread(BUZ_PlayThread), NULL);
		if(BUZ_PlayId == NULL)
		{
			printd(DBG_ErrorLvl, "BUZ->create BUZ_PlayThread fail!\n");
			return;
		}
	}
	
	
	KNL_StoreDbgInfo(__FUNCTION__,__LINE__);
	
	printd(DBG_InfoLvl, "BUZ->BuzSize=%d\r\n",ubToneGroupSize);
	BUZ_Meta.ubWeight        = ubWeight;
	BUZ_Meta.pToneParam      = pToneParam;
	BUZ_Meta.ubToneGroupSize = ubToneGroupSize;
	BUZ_Meta.ubChannelCnt    = ubChCnt;
	
	ubBUZ_PlyStopFlg = 0;
	if(osMessagePut(BUZ_MesgQ, &BUZ_Meta, 0)!=osOK)
	{
		printd(DBG_ErrorLvl, "BUZ->Q full\n");
	}
	
	
	KNL_StoreDbgInfo(__FUNCTION__,__LINE__);
#endif
}

void BUZ_PlayThread(void const *argument)
{
	BUZ_META_INFO_t BuzPara;
	uint8_t ubWeight;
	uint8_t ubBUZ_ToneGroupSize;
	uint8_t ubBUZ_ChannelCnt;
	BUZ_ToneParam_t *pBUZ_ToneParam;
	uint8_t ubLoop;
    while(1)
	{
		if( ADO_GetIpReadyStatus()==ADO_IP_READY )
		{
			osMessageGet(BUZ_MesgQ, &BuzPara, osWaitForever);
			
			ubWeight            = BuzPara.ubWeight;
			ubBUZ_ToneGroupSize = BuzPara.ubToneGroupSize;
			ubBUZ_ChannelCnt    = BuzPara.ubChannelCnt;
			pBUZ_ToneParam      = BuzPara.pToneParam;
			
			//initial
			ADO->BUZ_EN = 0;
			ADO->BUZ_WEIGHT = (ubWeight > 8) ? 8 : ubWeight;
			ADO->AUD_WEIGHT = 8 - ADO->BUZ_WEIGHT;
			ADO->BUZ0_LEVEL   = 0;
			ADO->BUZ0_LEV_TRG = 1;
			ADO->BUZ1_LEVEL   = 0;
			ADO->BUZ1_LEV_TRG = 1;
			ADO->BUZ2_LEVEL   = 0;
			ADO->BUZ2_LEV_TRG = 1;
			ADO->BUZ3_LEVEL   = 0;
			ADO->BUZ3_LEV_TRG = 1;
			ADO->BUZ0_CNT = 0;
			ADO->BUZ1_CNT = 0;
			ADO->BUZ2_CNT = 0;
			ADO->BUZ3_CNT = 0;
			
			KNL_StoreDbgInfo(__FUNCTION__,__LINE__);
			
			printd(DBG_InfoLvl, "BUZ->ply start\n");
			
			//play
			ADO_SetDacAutoMute(ADO_OFF);					// Automute
			ADO->BUZ_EN = 1;
			for(ubLoop = 0; ubLoop < ubBUZ_ToneGroupSize; ubLoop++)
			{
				if(ubBUZ_PlyStopFlg==1)
					break;
				
				switch(ubBUZ_ChannelCnt)
				{
					case 4:
						ADO->BUZ3_CNT = round(pBUZ_ToneParam[ubLoop].BUZ_Freq3 * 32 * 128 / 16000);
						ADO->BUZ3_TAB_SEL = pBUZ_ToneParam[ubLoop].ubBUZ_TabSel3;
						ADO->BUZ3_LEV_INDEX = pBUZ_ToneParam[ubLoop].ubBUZ_LevIndex3;
						ADO->BUZ3_INDEX_TRG = 1;
						ADO->BUZ3_LEVEL = pBUZ_ToneParam[ubLoop].ubBUZ_Volume3;
						ADO->BUZ3_LEV_TRG = 1;
					case 3:
						ADO->BUZ2_CNT = round(pBUZ_ToneParam[ubLoop].BUZ_Freq2 * 32 * 128 / 16000);
						ADO->BUZ2_TAB_SEL = pBUZ_ToneParam[ubLoop].ubBUZ_TabSel2;
						ADO->BUZ2_LEV_INDEX = pBUZ_ToneParam[ubLoop].ubBUZ_LevIndex2;
						ADO->BUZ2_INDEX_TRG = 1;
						ADO->BUZ2_LEVEL = pBUZ_ToneParam[ubLoop].ubBUZ_Volume2;
						ADO->BUZ2_LEV_TRG = 1;
					case 2:
						ADO->BUZ1_CNT = round(pBUZ_ToneParam[ubLoop].BUZ_Freq1 * 32 * 128 / 16000);
						ADO->BUZ1_TAB_SEL = pBUZ_ToneParam[ubLoop].ubBUZ_TabSel1;
						ADO->BUZ1_LEV_INDEX = pBUZ_ToneParam[ubLoop].ubBUZ_LevIndex1;
						ADO->BUZ1_INDEX_TRG = 1;
						ADO->BUZ1_LEVEL = pBUZ_ToneParam[ubLoop].ubBUZ_Volume1;
						ADO->BUZ1_LEV_TRG = 1;
					case 1:
						ADO->BUZ0_CNT = round(pBUZ_ToneParam[ubLoop].BUZ_Freq0 * 32 * 128 / 16000);
						ADO->BUZ0_TAB_SEL = pBUZ_ToneParam[ubLoop].ubBUZ_TabSel0;
						ADO->BUZ0_LEV_INDEX = pBUZ_ToneParam[ubLoop].ubBUZ_LevIndex0;
						ADO->BUZ0_INDEX_TRG = 1;
						ADO->BUZ0_LEVEL = pBUZ_ToneParam[ubLoop].ubBUZ_Volume0;
						ADO->BUZ0_LEV_TRG = 1;
						break;
				}
				osDelay(pBUZ_ToneParam[ubLoop].uwDelay);
			}
			ADO->BUZ_EN = 0;
			ADO_SetDacAutoMute(ADO_ON); 					// Automute

			printd(DBG_InfoLvl, "BUZ->ply over\n");
			osDelay(10);
		}
		else
			osDelay(10);
    }
}

void BUZ_PlayStop(void) {
	ubBUZ_PlyStopFlg = 1;
}

void BUZ_PlayPowerOnSound(void) {   
	const static BUZ_ToneParam_t BUZ_PowerOnSound[] = {
	//	delay	Freq_0	Tab_0	LvIdx_0	Vol_0
		{50,	M_DO_1, 3,		0,		63},
		{50,	M_RE_2, 3,		0,		63},
		{50,	M_MI_3, 3,		0,		63},
		{50,	M_FA_4, 3,		0,		63},
		{50,	M_SOL_5,3,		0,		63},
		{50,	M_LA_6, 3,		0,		63},
		{50,	M_SI_7, 3,		0,		63},
		{50,	H_DO_1, 3,		0,		63},
	};
	
	BUZ_PlayStart(1, 8, (BUZ_ToneParam_t*)BUZ_PowerOnSound, sizeof(BUZ_PowerOnSound) / sizeof(BUZ_ToneParam_t));
}

void BUZ_PlayPowerOffSound(void) {   
	const static BUZ_ToneParam_t BUZ_PowerOffSound[] = {
	//	delay	Freq_0	Tab_0	LvIdx_0	Vol_0
		{50,	H_DO_1, 3,		0,		63},
		{50,	M_SI_7, 3,		0,		63},
		{50,	M_LA_6, 3,		0,		63},
		{50,	M_SOL_5,3,		0,		63},
		{50,	M_FA_4, 3,		0,		63},
		{50,	M_MI_3, 3,		0,		63},
		{50,	M_RE_2, 3,		0,		63},
		{50,	M_DO_1, 3,		0,		63},
	};
	
	BUZ_PlayStart(1, 8, (BUZ_ToneParam_t*)BUZ_PowerOffSound, sizeof(BUZ_PowerOffSound) / sizeof(BUZ_ToneParam_t));
}
//------------------------------------------------------------------------------
void BUZ_PlaySingleSound(void) {   
	const static BUZ_ToneParam_t BUZ_SingleSound[] = {
	//	delay	Freq_0	Tab_0	LvIdx_0	Vol_0
		{30,	H_DO_1, 3,		0,		63},
	};
	
	BUZ_PlayStart(1, 8, (BUZ_ToneParam_t*)BUZ_SingleSound, sizeof(BUZ_SingleSound) / sizeof(BUZ_ToneParam_t));
}
//------------------------------------------------------------------------------
void BUZ_PlayFaceSound(void) {   
	const static BUZ_ToneParam_t BUZ_SingleSound[] = {
	//	delay	Freq_0	Tab_0	LvIdx_0	Vol_0
		{20,	H_DO_1, 3,		0,		63},
		{10,	ZERO_0, 3,		0,		63},
	};
	
	BUZ_PlayStart(1, 8, (BUZ_ToneParam_t*)BUZ_SingleSound, sizeof(BUZ_SingleSound) / sizeof(BUZ_ToneParam_t));
}
//------------------------------------------------------------------------------
void BUZ_PlayDoneSound(void) {   
	const static BUZ_ToneParam_t BUZ_SingleSound[] = {
	//	delay	Freq_0	Tab_0	LvIdx_0	Vol_0
		{100,	H_DO_1, 3,		0,		63},
		{80,	H_SI_7, 3,		0,		63},
		{10,	ZERO_0, 3,		0,		63},
		{100,	H_DO_1, 3,		0,		63},
		{80,	H_SI_7, 3,		0,		63},
	};
	
	BUZ_PlayStart(1, 8, (BUZ_ToneParam_t*)BUZ_SingleSound, sizeof(BUZ_SingleSound) / sizeof(BUZ_ToneParam_t));
}
//------------------------------------------------------------------------------
void BUZ_PlayOkSound(void) {   
	const static BUZ_ToneParam_t BUZ_SingleSound[] = {
	//	delay	Freq_0	Tab_0	LvIdx_0	Vol_0
		{100,	H_DO_1, 3,		0,		63},
		{20,	ZERO_0, 3,		0,		63},
		{100,	H_DO_1, 3,		0,		63},
	};
	
	BUZ_PlayStart(1, 8, (BUZ_ToneParam_t*)BUZ_SingleSound, sizeof(BUZ_SingleSound) / sizeof(BUZ_ToneParam_t));
}
//------------------------------------------------------------------------------
void BUZ_PlayFailSound(void) {   
	const static BUZ_ToneParam_t BUZ_SingleSound[] = {
	//	delay	Freq_0	Tab_0	LvIdx_0	Vol_0
		{150,	H_SI_7, 3,		0,		63},
		{80,	L_FA_4, 3,		0,		63},
		{80,	M_LA_6, 3,		0,		63},
	};
	
	BUZ_PlayStart(1, 8, (BUZ_ToneParam_t*)BUZ_SingleSound, sizeof(BUZ_SingleSound) / sizeof(BUZ_ToneParam_t));
}
//------------------------------------------------------------------------------
void BUZ_PlayLowBatSound(void) {   
	const static BUZ_ToneParam_t BUZ_LowBatSound[] = {
	//	delay	Freq_0	Tab_0	LvIdx_0	Vol_0
		{30,	H_DO_1, 3,		0,		63},
		{50,	H_DO_1, 3,		15,		63},
		{20,	ZERO_0, 3,		0,		63},
		{30,	M_DO_1, 3,		0,		63},
		{50,	M_DO_1, 3,		15,		63},
		{150,	ZERO_0,	3,		0,		63},
		{30,	H_DO_1, 3,		0,		63},
		{50,	H_DO_1, 3,		15,		63},
		{20,	ZERO_0, 3,		0,		63},
		{30,	M_DO_1, 3,		0,		63},
		{50,	M_DO_1, 3,		15,		63},
		{150,	ZERO_0, 3,		0,		63},
		{30,	H_DO_1, 3,		0,		63},
		{50,	H_DO_1, 3,		15,		63},
		{20,	ZERO_0, 3,		0,		63},
		{30,	M_DO_1, 3,		0,		63},
		{50,	M_DO_1, 3,		15,		63},
		{150,	ZERO_0, 3,		0,		63},
	};
	
	BUZ_PlayStart(1, 8, (BUZ_ToneParam_t*)BUZ_LowBatSound, sizeof(BUZ_LowBatSound) / sizeof(BUZ_ToneParam_t));
}

void BUZ_PlayTestSound(void) {   
	#define BUZ_SMOOTH	1
	const static BUZ_ToneParam_t BUZ_TestSound[] = {
	//	delay	Freq_0	Tab_0	LvIdx_0	Vol_0	Freq_1	Tab_1	LvIdx_1	Vol_1	Freq_2	Tab_2	LvIdx_2	Vol_2	Freq_3	Tab_3	LvIdx_3	Vol_3
		{200,	M_DO_1, 0,		0,		32, 	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32},
		{200,	M_DO_1, 1,		0,		32, 	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32},
		{200,	M_DO_1, 2,		0,		32, 	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32},
		{200,	M_DO_1, 3,		0,		32, 	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32},
#if BUZ_SMOOTH
		{200,	M_DO_1, 3,		15,		32, 	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32},
#endif
		{200,	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32},

		{200,	M_DO_1, 0,		0,		32, 	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32},
		{200,	M_DO_1,	0,		0,		24, 	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32},
		{200,	M_DO_1, 0,		0,		16, 	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32},
		{200,	M_DO_1, 0,		0,		8, 		ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32},

		{500,	M_DO_1, 0,		0,		32, 	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32},
		{500,	M_DO_1, 0,		1,		32, 	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32},
#if BUZ_SMOOTH
		{500,	M_DO_1, 0,		15,		32, 	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32},
#endif
		{200,	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32},

		{500,	M_DO_1, 0,		0,		32, 	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32},
		{500,	M_DO_1, 0,		2,		32, 	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32},
#if BUZ_SMOOTH
		{500,	M_DO_1, 0,		15,		32, 	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32},
#endif
		{200,	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32},

		{500,	M_DO_1, 0,		0,		32, 	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32},
		{500,	M_DO_1, 0,		3,		32, 	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32},
#if BUZ_SMOOTH
		{500,	M_DO_1, 0,		15,		32, 	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32},
#endif
		{200,	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32},

		{500,	M_DO_1,	0,		0,		32, 	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32},
		{500,	M_DO_1, 0,		4,		32, 	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32},
#if BUZ_SMOOTH
		{500,	M_DO_1, 0,		15,		32, 	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32},
#endif
		{200,	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32},

		{500,	M_DO_1, 0,		0,		32, 	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32},
		{500,	M_DO_1, 0,		5,		32, 	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32},
#if BUZ_SMOOTH
		{500,	M_DO_1, 0,		15,		32, 	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32},
#endif
		{200,	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32},

		{500,	M_DO_1, 0,		0,		32, 	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32},
		{500,	M_DO_1, 0,		6,		32, 	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32},
#if BUZ_SMOOTH
		{500,	M_DO_1, 0,		15,		32, 	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32},
#endif
		{200,	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32},

		{500,	M_DO_1, 0,		0,		32, 	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32},
		{500,	M_DO_1,	0,		7,		32, 	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32},
#if BUZ_SMOOTH
		{500,	M_DO_1, 0,		15,		32, 	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32},
#endif
		{200,	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32},

		{500,	M_DO_1, 0,		0,		32, 	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32},
		{500,	M_DO_1, 0,		8,		32, 	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32},
#if BUZ_SMOOTH
		{500,	M_DO_1, 0,		15,		32, 	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32},
#endif
		{200,	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32},

		{500,	M_DO_1, 0,		0,		32, 	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32},
		{500,	M_DO_1, 0,		9,		32, 	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32},

		{500,	M_DO_1, 0,		0,		32, 	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32},
		{500,	M_DO_1, 0,		10,		32, 	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32},

		{500,	M_DO_1, 0,		0,		32, 	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32},
		{500,	M_DO_1, 0,		11,		32, 	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32},

		{500,	M_DO_1, 0,		0,		32, 	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32},
		{500,	M_DO_1, 0,		12,		32, 	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32},

		{500,	M_DO_1, 0,		0,		32, 	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32},
		{500,	M_DO_1, 0,		13,		32, 	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32},

		{500,	M_DO_1, 0,		0,		32, 	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32},
		{500,	M_DO_1, 0,		14,		32, 	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32},

		{500,	M_DO_1, 0,		0,		32, 	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32},
		{500,	M_DO_1, 0,		15,		32, 	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32},
	};
	
	BUZ_PlayStart(4, 8, (BUZ_ToneParam_t*)BUZ_TestSound, sizeof(BUZ_TestSound) / sizeof(BUZ_ToneParam_t));
}

osMessageQId tIRBuzzer_recvQueue = NULL;

static void IR_BuzzerPolicy_Thread(void)
{
    uint8_t tim, ch;

    while(1)
    {
        if (osMessageGet(tIRBuzzer_recvQueue, (void*)&ch, osWaitForever))
        {
            tim = ch;
            if (tim >= 200)
            {
                tim = 200;
            }
            else if (tim <= 50)
            {
                tim = 50;
            }
            BUZZER_DI(tim);
        }
        //osDelay(20);
    }
}


void BUZ_Init(void)
{
	osMessageQDef(tIRBuzzer_recvQueue, 16, sizeof(char));
    tIRBuzzer_recvQueue = osMessageCreate(osMessageQ(tIRBuzzer_recvQueue), NULL);

    osThreadDef(IR_BuzzerPolicy_Thread, IR_BuzzerPolicy_Thread, osPriorityNormal, 1, 8192);
	osThreadCreate(osThread(IR_BuzzerPolicy_Thread), NULL);
}

void BUZ_Call_DI(uint8_t ch)
{
	if((osMessagePut(tIRBuzzer_recvQueue, (void*) &ch, 0)) != osOK) {
        printf("tIRBuzzer Q full\r\n");
        osMessageReset(tIRBuzzer_recvQueue);
    }
}




#include "UI_BUCCU[WSVGA]_MENU.h"
#ifdef BSP_D_SNCC71_GM8285C_RX_V2
/*************************************************************Home Menu**********************************************************************************/
#define UI_MENUICON_NUM		7

extern uint8_t Drawing_BoxFlag[4];
extern uint8_t ubDrawBSDRange;

//------------------------------------------------------------------------------
void UI_DrawMenuPage(void)
{
	uint8_t i;
	OSD_IMG_INFO tOsdImgInfo[UI_MENUICON_NUM*2];
	MenuOnFlag = TRUE;
	//清理一下框
	UI_ClearOsdImageNoUpdate();
	ubDrawBSDRange = 0;
	UI_DrawDesktopIcon();
	ubDrawBSDRange = 1;
	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_ADO_MODENOR_MENUICON, (UI_MENUICON_NUM*2), &tOsdImgInfo[0]);

	for(i = 0; i < UI_MENUICON_NUM; i++)
		tOSD_Img2(&tOsdImgInfo[2*i], OSD_QUEUE);
	tOSD_Img2(&tOsdImgInfo[2*tUI_MenuItem.ubItemIdx + UI_ICON_HIGHLIGHT], OSD_UPDATE);
	OSD_Weight(OSD_WEIGHT_8DIV8);
}
//------------------------------------------------------------------------------

static void UI_MenuDrawMenuItem(void)
{
	uint16_t uwMenuItemOsdImg[UI_MENUICON_NUM] = {
													OSD2IMG_ADO_MODENOR_MENUICON,
													OSD2IMG_VDO_MODENOR_MENUICON,
													OSD2IMG_PAIRNOR_MENUICON,
													OSD2IMG_RECNOR_MENUICON,
													OSD2IMG_PLYNOR_MENUICON,
													OSD2IMG_SETTINGNOR_MENUICON,
													OSD2IMG_POWERNOR_MENUICON,
													};
	UI_DrawHLandNormalIcon(uwMenuItemOsdImg[tUI_MenuItem.ubItemPreIdx], 
		(uwMenuItemOsdImg[tUI_MenuItem.ubItemIdx] + UI_ICON_HIGHLIGHT));

}

void UI_HomeMenu_Key(UI_ArrowKey_t tArrowKey)
{
	tUI_MenuItem.ubItemPreIdx = tUI_MenuItem.ubItemIdx;
	switch(tArrowKey)
	{
		case LEFT_ARROW:
			if(tUI_MenuItem.ubItemIdx == ADO_MODE_ITEM)
				tUI_MenuItem.ubItemIdx = POWER_ITEM;
			else	
				tUI_MenuItem.ubItemIdx--;
			break;
		case RIGHT_ARROW:
			if(tUI_MenuItem.ubItemIdx == POWER_ITEM)
				tUI_MenuItem.ubItemIdx = ADO_MODE_ITEM;
			else
				tUI_MenuItem.ubItemIdx++;
			break;
		case EXIT_ARROW:
			UI_ClearOsdImageNoUpdate();
			MenuOnFlag = FALSE;
			tUI_MenuItem.ubItemPreIdx = ADO_MODE_ITEM;
			tUI_MenuItem.ubItemIdx 	  = ADO_MODE_ITEM;
			tUI_State = UI_DISPLAY_STATE;
			//ubDrawBSDRange = 0;
			UI_DrawDesktopIcon();
			//ubDrawBSDRange = 1;
			return;
		case ENTER_ARROW:
			if(tUI_MenuItem.ubItemIdx != RECORD_ITEM)
				UI_SubKeyMenu(ENTER_ARROW);
			else//控制录像
			{
				UI_MenuKey();
				UI_EnterKey();
			}
			
		default:
			return;
	}
	UI_MenuDrawMenuItem();
}

/*************************************************************Volume Menu**********************************************************************************/
uint8_t UI_GetCurrentVolumeLevel(void)
{
	UI_CamSource tAudioSrc = UI_GetAudioSrc();

	if(tAudioSrc == CamSource_AI)
		return tUI_CuSetting.AI_ubVolumeLvl;

	if(tAudioSrc <= CamSource_CAM4)
		return tUI_CamStatus[tAudioSrc].ubVolumeLvl;

	return tUI_CamStatus[CamSource_CAM1].ubVolumeLvl;
}

uint8_t *UI_GetCurrentVolumeLevelPtr(void)
{
	UI_CamSource tAudioSrc = UI_GetAudioSrc();

	if(tAudioSrc == CamSource_AI)
		return &tUI_CuSetting.AI_ubVolumeLvl;

	if(tAudioSrc <= CamSource_CAM4)
		return &tUI_CamStatus[tAudioSrc].ubVolumeLvl;

	return &tUI_CamStatus[CamSource_CAM1].ubVolumeLvl;
}
//------------------------------------------------------------------------------
void UI_DrawVolumeMenuPage(void)
{
	OSD_IMG_INFO tOsdImgInfo[4];
	uint8_t DrawboxCnt =0;

	osMutexWait(osEnterVolumeFlag, osWaitForever);
	while(1)
	{
		if((Drawing_BoxFlag[0] == 0 && Drawing_BoxFlag[1] == 0 && Drawing_BoxFlag[2] == 0 && Drawing_BoxFlag[3] == 0)\
			|| DrawboxCnt >= 5)
		{
			DrawboxCnt = 0;
			break;

		}
		else
		{
			osDelay(100);
			DrawboxCnt++;
			printf("wait draw box!!!!!!\n");
		}
		
	}
	DrawboxCnt = 0;
	
	tUI_State = UI_SET_VOLUME_STATE;
	MenuOnFlag = TRUE;
//	OSD_ClearImg1Buf();
	UI_ClearOsdImageNoUpdate();
	ubDrawBSDRange = 0;
	UI_DrawDesktopIcon();
	ubDrawBSDRange = 1;


	SendIrCodeFlag = 1;
	if(TriggerLock && DeskTopShowView <= CAM4 && tUI_CuSetting.tParkingLineEnable[DeskTopShowView] != PARKING_LINE_OFF)//清除倒车光标
	{
		//UI_EraseParkingLine(DeskTopShowView,OSD_QUEUE);

	}
//		UI_ClearOsdImageNoUpdate();
//		UI_DrawDesktopIcon();	
	
	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_SOUND_SLIDER, 4, &tOsdImgInfo[0]);
	
	tOSD_Img2(&tOsdImgInfo[0], OSD_QUEUE);
	if(tUI_CuSetting.ubMuteFlag)
		tOSD_Img2(&tOsdImgInfo[1], OSD_QUEUE);
	else
		tOSD_Img2(&tOsdImgInfo[2], OSD_QUEUE);
	
	
	//滑块
	tOsdImgInfo[3].uwXStart = UI_Map(UI_GetCurrentVolumeLevel(),0,99,
		tOsdImgInfo[0].uwXStart,tOsdImgInfo[0].uwXStart + tOsdImgInfo[0].uwHSize - tOsdImgInfo[3].uwHSize);
	tOSD_Img2(&tOsdImgInfo[3], OSD_QUEUE);
	//数值
	if(UI_GetCurrentVolumeLevel() < 10)
	{
		UI_ShowButtonValueHighLight(0,tOsdImgInfo[3].uwXStart + 4,tOsdImgInfo[3].uwYStart + 9,OSD_QUEUE);
		UI_ShowButtonValueHighLight(UI_GetCurrentVolumeLevel(),tOsdImgInfo[3].uwXStart + 24,tOsdImgInfo[3].uwYStart + 9,OSD_UPDATE);
	}
	else
		UI_ShowButtonValueHighLight(UI_GetCurrentVolumeLevel(),tOsdImgInfo[3].uwXStart + 4,tOsdImgInfo[3].uwYStart + 9,OSD_UPDATE);

	osMutexRelease(osEnterVolumeFlag);
}
//------------------------------------------------------------------------------
void UI_VolumeMenuDrawMenuItem(uint8_t ubPreVolumeLvl)
{
	MenuOnFlag = TRUE;
	OSD_IMG_INFO tOsdImgInfo[5];
	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_SOUND_SLIDER, 5, &tOsdImgInfo[0]);

	if(tUI_CuSetting.ubMuteFlag)
		tOSD_Img2(&tOsdImgInfo[1], OSD_QUEUE);
	else
		tOSD_Img2(&tOsdImgInfo[2], OSD_QUEUE);
	//滑块
	tOsdImgInfo[4].uwXStart = UI_Map(ubPreVolumeLvl,0,99,
		tOsdImgInfo[0].uwXStart,tOsdImgInfo[0].uwXStart + tOsdImgInfo[0].uwHSize - tOsdImgInfo[4].uwHSize);
	tOSD_Img2(&tOsdImgInfo[4], OSD_QUEUE);
	
	tOsdImgInfo[3].uwXStart = UI_Map(UI_GetCurrentVolumeLevel(),0,99,
		tOsdImgInfo[0].uwXStart,tOsdImgInfo[0].uwXStart + tOsdImgInfo[0].uwHSize - tOsdImgInfo[3].uwHSize);
	tOSD_Img2(&tOsdImgInfo[3], OSD_QUEUE);

	//数值
	if(UI_GetCurrentVolumeLevel() < 10)
	{
		UI_ShowButtonValueHighLight(0,tOsdImgInfo[3].uwXStart + 4,tOsdImgInfo[3].uwYStart + 9,OSD_QUEUE);
		UI_ShowButtonValueHighLight(UI_GetCurrentVolumeLevel(),tOsdImgInfo[3].uwXStart + 24,tOsdImgInfo[3].uwYStart + 9,OSD_UPDATE);
	}
	else
		UI_ShowButtonValueHighLight(UI_GetCurrentVolumeLevel(),tOsdImgInfo[3].uwXStart + 4,tOsdImgInfo[3].uwYStart + 9,OSD_UPDATE);

}
//------------------------------------------------------------------------------

void UI_VolumeMenuPage(UI_ArrowKey_t tArrowKey)
{
	uint8_t ubPreVolumeLvl = UI_GetCurrentVolumeLevel();
//	MenuOnFlag = TRUE;
	switch(tArrowKey)
	{
		case LEFT_ARROW:
			if(UI_GetCurrentVolumeLevel() > 0)
				*UI_GetCurrentVolumeLevelPtr() = UI_GetCurrentVolumeLevel()-1;
			else 
				return;
			break;
		case RIGHT_ARROW:
			if(UI_GetCurrentVolumeLevel() < 99)
				*UI_GetCurrentVolumeLevelPtr() = UI_GetCurrentVolumeLevel()+1;
			else 
				return;
			break;
		case EXIT_ARROW:
			SendIrCodeFlag = 0;
			UI_UpdateDevStatusInfo();
			UI_ClearOsdImageNoUpdate();
			MenuOnFlag = FALSE;
			tUI_State = UI_DISPLAY_STATE;
			//ubDrawBSDRange = 0;
			UI_DrawDesktopIcon();
			//ubDrawBSDRange = 1;
			if(TriggerLock && DeskTopShowView <= CAM4 && tUI_CuSetting.tParkingLineEnable[DeskTopShowView] != PARKING_LINE_OFF)
			{
				if( tUI_CuSetting.ubIsEnableBSDRANGE[DeskTopShowView] == 0)
					UI_DrawParkingLine(DeskTopShowView,OSD_UPDATE);
			}
			return;			
		case MUTE_ARROW:
			tUI_CuSetting.ubMuteFlag = 1 - tUI_CuSetting.ubMuteFlag;
			break;
		case ENTER_ARROW:
			tUI_CuSetting.ubMuteFlag = 1 - tUI_CuSetting.ubMuteFlag;
			break;
		default:
			return;
	}

	UI_VolumeMenuDrawMenuItem(ubPreVolumeLvl);
	UI_AudioOn();
}
//------------------------------------------------------------------------------
void UI_ShowSdCardInfo(UI_ArrowKey_t tArrowKey)
{
	switch(tArrowKey)
	{
		case EXIT_ARROW:
			UI_ClearOsdImageNoUpdate();
			MenuOnFlag = FALSE;
			tUI_State = UI_DISPLAY_STATE;
			//ubDrawBSDRange = 0;
			UI_DrawDesktopIcon();
			//ubDrawBSDRange = 1;
			break;
		default:
			break;
	}
}
//------------------------------------------------------------------------------
void UI_ShowTxVersion(UI_ArrowKey_t tArrowKey)
{
	switch(tArrowKey)
	{
		case EXIT_ARROW:
			UI_ClearOsdImageNoUpdate();
			MenuOnFlag = FALSE;
			tUI_State = UI_DISPLAY_STATE;
			//ubDrawBSDRange = 0;
			UI_DrawDesktopIcon();
			//ubDrawBSDRange = 1;
			break;
		default:
			break;
	}
}


/********************************************************************TOUCH*********************************************************************************************/
/********************************************************************TOUCH*********************************************************************************************/
/********************************************************************TOUCH*********************************************************************************************/

/*************************************************************Home Menu**********************************************************************************/

void UI_HomeMenuTouchFunc(TOUCH_EVENT_t *Touch_Info)
{
	uint16_t i = 0;
	OSD_IMG_INFO tOsdImgInfo;
	tUI_MenuItem.ubItemPreIdx = tUI_MenuItem.ubItemIdx;
	switch(Touch_Info->Gesture)
	{
		case TOUCH_PRESS:	

			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_ADO_MODENOR_MENUICON, 1, &tOsdImgInfo);	
			if(Touch_Info->startY > tOsdImgInfo.uwYStart)
			{
				for(i = 0; i < UI_MENUICON_NUM;i++)//判断具体点击在哪个图标上
				{
					tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_ADO_MODENOR_MENUICON + 2*i, 1, &tOsdImgInfo);	
					
					if(Touch_Info->startX > tOsdImgInfo.uwXStart
						&& Touch_Info->startX < tOsdImgInfo.uwXStart + tOsdImgInfo.uwHSize
						&& Touch_Info->startY > tOsdImgInfo.uwYStart 
						&& Touch_Info->startY < tOsdImgInfo.uwYStart + tOsdImgInfo.uwVSize)
					{
						tUI_MenuItem.ubItemIdx = i;
						break;
					}
				}
				if(i == UI_MENUICON_NUM)
					return;
				//当tUI_MenuItem.ubItemIdx和tUI_MenuItem.ubItemPreIdx不相等时，就代表焦点变了
				if(tUI_MenuItem.ubItemIdx != tUI_MenuItem.ubItemPreIdx)
				{
					UI_MenuDrawMenuItem();
				}
				if(tUI_MenuItem.ubItemIdx != RECORD_ITEM)
				{
					UI_EnterKey();
				}
				else
				{
					UI_MenuKey();
					UI_EnterKey();
				}
			}
			else if(Touch_Info->startY < tOsdImgInfo.uwYStart - 30)//触摸到ui上方就退出menu
			{
				UI_MenuKey();
			}
			
			break;

		default :

			break;
	}
}



/*************************************************************Volume Menu**********************************************************************************/
void UI_VolumeMenuTouchFunc(TOUCH_EVENT_t *Touch_Info)
{
	OSD_IMG_INFO tOsdImgInfo;
	uint8_t ubPreVolumeLvl = UI_GetCurrentVolumeLevel();
	//MenuOnFlag = TRUE;
	switch(Touch_Info->Gesture)
	{
		case TOUCH_PRESS:
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_VOLUME_MUTE, 1, &tOsdImgInfo);	//控制静音
			if(Touch_Info->startX > tOsdImgInfo.uwXStart - 10
					&& Touch_Info->startX < tOsdImgInfo.uwXStart + tOsdImgInfo.uwHSize + 10
					&& Touch_Info->startY > tOsdImgInfo.uwYStart - 10
					&& Touch_Info->startY < tOsdImgInfo.uwYStart + tOsdImgInfo.uwVSize + 10)
			{
				tUI_CuSetting.ubMuteFlag = 1 - tUI_CuSetting.ubMuteFlag;
				UI_VolumeMenuDrawMenuItem(ubPreVolumeLvl);
				UI_AudioOn();
				return;
			}
			else
			{
				tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_SOUND_SLIDER, 1, &tOsdImgInfo);	//点击在滑动条上
				if(Touch_Info->startX > tOsdImgInfo.uwXStart - 10
						&& Touch_Info->startX < tOsdImgInfo.uwXStart + tOsdImgInfo.uwHSize + 10
						&& Touch_Info->startY > tOsdImgInfo.uwYStart - 40 
						&& Touch_Info->startY < tOsdImgInfo.uwYStart + tOsdImgInfo.uwVSize + 40)
				{
					return;
				}
				else
					UI_MenuKey();
			}
			break;
		case TOUCH_PRESSDOWN:
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_SOUND_SLIDER,1,&tOsdImgInfo);
			if(Touch_Info->endX >= tOsdImgInfo.uwXStart - 10 
				&& Touch_Info->endX <= tOsdImgInfo.uwXStart + tOsdImgInfo.uwHSize + 10
				&& Touch_Info->endY >= tOsdImgInfo.uwYStart - 40
				&& Touch_Info->endY <= tOsdImgInfo.uwYStart + tOsdImgInfo.uwVSize + 40)
			{
				if(Touch_Info->endX < tOsdImgInfo.uwXStart)
					Touch_Info->endX = tOsdImgInfo.uwXStart;
				if(Touch_Info->endX > tOsdImgInfo.uwXStart + tOsdImgInfo.uwHSize)
					Touch_Info->endX = tOsdImgInfo.uwXStart + tOsdImgInfo.uwHSize;


				//坐标位置转成进度条的值
				uint8_t ubValue = 
					(uint8_t)UI_Map(Touch_Info->endX,tOsdImgInfo.uwXStart,tOsdImgInfo.uwXStart + tOsdImgInfo.uwHSize,0,99);
					
				// tUI_CamStatus[UI_GetAudioSrc()].ubVolumeLvl = ubValue;		
				if(UI_GetAudioSrc() == CamSource_AI)
					tUI_CuSetting.AI_ubVolumeLvl = ubValue;
				else if(UI_GetAudioSrc() <= CamSource_CAM4)
					tUI_CamStatus[UI_GetAudioSrc()].ubVolumeLvl = ubValue;
				else
					tUI_CuSetting.AI_ubVolumeLvl = ubValue;
				
				UI_VolumeMenuDrawMenuItem(ubPreVolumeLvl);
				UI_AudioOn();
			}
			
			break;
		default:
	
			return;
	}
	
	

}
/*************************************************************SdCard Info**********************************************************************************/

//------------------------------------------------------------------------------
void UI_ShowSdCardInfoTouchFunc(TOUCH_EVENT_t *Touch_Info)
{
	OSD_IMG_INFO tOsdImgInfo;
	switch(Touch_Info->Gesture)
	{
		case TOUCH_PRESS:
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_RETURN_NOBACKGROUND_HL, 1, &tOsdImgInfo);	
			if(Touch_Info->startX > tOsdImgInfo.uwXStart 
				&& Touch_Info->startX < tOsdImgInfo.uwXStart + tOsdImgInfo.uwHSize
				&& Touch_Info->startY > tOsdImgInfo.uwYStart 
				&& Touch_Info->startY < tOsdImgInfo.uwYStart + tOsdImgInfo.uwVSize)
			{
				tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
				TIMER_Delay_ms(30);
				UI_MenuKey();
			}
			break;
		
		default:
	
			return;
	}
	
	

}
/*************************************************************Tx Version**********************************************************************************/

//------------------------------------------------------------------------------
void UI_ShowTxVersionTouchFunc(TOUCH_EVENT_t *Touch_Info)
{
	OSD_IMG_INFO tOsdImgInfo;
	switch(Touch_Info->Gesture)
	{
		case TOUCH_PRESS:
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_RETURN_HL, 1, &tOsdImgInfo);	
			if(Touch_Info->startX > tOsdImgInfo.uwXStart 
				&& Touch_Info->startX < tOsdImgInfo.uwXStart + tOsdImgInfo.uwHSize
				&& Touch_Info->startY > tOsdImgInfo.uwYStart 
				&& Touch_Info->startY < tOsdImgInfo.uwYStart + tOsdImgInfo.uwVSize)
			{
				tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
				TIMER_Delay_ms(30);
				UI_MenuKey();
			}
			break;
		
		default:
	
			return;
	}

}


#endif


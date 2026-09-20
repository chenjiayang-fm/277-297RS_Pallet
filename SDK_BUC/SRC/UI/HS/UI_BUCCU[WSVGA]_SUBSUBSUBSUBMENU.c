#include "UI_BUCCU[WSVGA]_SUBSUBSUBSUBMENU.h"

#ifdef BSP_D_SNCC71_GM8285C_RX_V2


void UI_PlaybackSubSubSubSubMenuPage(UI_ArrowKey_t tArrowKey);
void UI_PlaybackSubSubSubSubTouchMenu(TOUCH_EVENT_t *Touch_Info);

void UI_DateTimeSubSubSubSubMenuPage(UI_ArrowKey_t tArrowKey);
void UI_DateTimeSubSubSubSubTouchMenu(TOUCH_EVENT_t *Touch_Info);

//----------------------------------------------------------------------
void UI_SubSubSubSubKeyMenu(UI_ArrowKey_t tArrowKey)
{
	switch(tUI_MenuItem.ubItemIdx)
	{	
		case PLAYBACK_ITEM:
			UI_PlaybackSubSubSubSubMenuPage(tArrowKey);
			break;
		case SETTING_ITEM:
			{
				UI_SettingSubMenuItemList_t tSubMenuItem = 
					(UI_SettingSubMenuItemList_t)tUI_SubMenuItem[SETTING_ITEM].tSubMenuInfo.ubItemIdx;//sub
				uint16_t uwSubSubMenuItemIdx = 
					(uint16_t)tSystemSubSubMenuItem.tSubMenuInfo.ubItemIdx;//sub sub
				printf("tSubMenuItem = %d, uwSubSubMenuItemIdx = %d \n", tSubMenuItem, uwSubSubMenuItemIdx);
				if(tSubMenuItem == SYSTEMSET_ITEM && uwSubSubMenuItemIdx == DATETIME_ITEM)
					UI_DateTimeSubSubSubSubMenuPage(tArrowKey);
			
				break;
			}
		default:
			break;
	}
}
//------------------------------------------------------------------------------------playback------------------------------------------------------------------------------
//------------------------------------------------------------------------------------playback------------------------------------------------------------------------------
//------------------------------------------------------------------------------------playback------------------------------------------------------------------------------
//----------------------------------------------------------------------

void UI_PlayBackUpdateVoiceIcon(void)
{	
	KNL_DISP_TYPE tPlayDispTye; 
	OSD_IMG_INFO tVoiceOsdImgInfo,tEraseOsdImgInfo;
	KNL_DISP_LOCATION tDispLocate;
	tPlayDispTye = (KNL_DISP_TYPE)tUI_RecFilesInfo.tRecFilesInfo[uwCurVideoPlayIdx[0]].HidnFileInfo.SubHidnInfo.ubPreviewMode;
	//擦除声音图标
	if(tPlayDispTye == KNL_DISP_DUAL_U || tPlayDispTye == KNL_DISP_QUAD)
	{
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_DESKTOP_VOICE_MUTE + 1 - tUI_CuSetting.ubMuteFlag, 1, &tEraseOsdImgInfo);
		tEraseOsdImgInfo.uwYStart = 5;
		OSD_EraserImg2(&tEraseOsdImgInfo);

		tEraseOsdImgInfo.uwXStart += OSD_WIDTH/2;
		OSD_EraserImg2(&tEraseOsdImgInfo);

		tEraseOsdImgInfo.uwYStart = 5 + OSD_HEIGHT/2;
		OSD_EraserImg2(&tEraseOsdImgInfo);

		tEraseOsdImgInfo.uwXStart -= OSD_WIDTH/2;
		OSD_EraserImg2(&tEraseOsdImgInfo);
	}
	else if(tPlayDispTye == KNL_DISP_H)
	{
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_DESKTOP_VOICE_MUTE + 1 - tUI_CuSetting.ubMuteFlag, 1, &tEraseOsdImgInfo);
		tEraseOsdImgInfo.uwYStart = 5;
		tEraseOsdImgInfo.uwXStart -= OSD_WIDTH/4;
		OSD_EraserImg2(&tEraseOsdImgInfo);

		tEraseOsdImgInfo.uwXStart += OSD_WIDTH/4*3;
		OSD_EraserImg2(&tEraseOsdImgInfo);

		tEraseOsdImgInfo.uwXStart -= OSD_WIDTH/4;
		OSD_EraserImg2(&tEraseOsdImgInfo);

		tEraseOsdImgInfo.uwYStart += OSD_HEIGHT/2;
		OSD_EraserImg2(&tEraseOsdImgInfo);

	}
	//重新显示声音图标
	if(tPlayDispTye == KNL_DISP_DUAL_U)
	{
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_DESKTOP_VOICE_MUTE + 1 - tUI_CuSetting.ubMuteFlag, 1, &tVoiceOsdImgInfo);//voice
		for(tDispLocate = KNL_DISP_LOCATION1;tDispLocate <= KNL_DISP_LOCATION2;tDispLocate ++)
		{
			UI_CamNum_t tCamNum = tUI_RecFilesInfo.tRecFilesInfo[uwCurVideoPlayIdx[0]].HidnFileInfo.SubHidnInfo.ubSrcLocate[tDispLocate];
			if(tCamNum == tUI_RecPlayAct.tAdoPlayCam)
			{
				tVoiceOsdImgInfo.uwYStart = 5;
				if(tDispLocate == KNL_DISP_LOCATION2)
					tVoiceOsdImgInfo.uwXStart += OSD_WIDTH/2;
				tOSD_Img2(&tVoiceOsdImgInfo, OSD_QUEUE);
			}
		}

	}
	else if(tPlayDispTye == KNL_DISP_QUAD)
	{
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_DESKTOP_VOICE_MUTE + 1 - tUI_CuSetting.ubMuteFlag, 1, &tVoiceOsdImgInfo);//voice
		for(tDispLocate = KNL_DISP_LOCATION1;tDispLocate <= KNL_DISP_LOCATION4;tDispLocate ++)
		{
			UI_CamNum_t tCamNum = tUI_RecFilesInfo.tRecFilesInfo[uwCurVideoPlayIdx[0]].HidnFileInfo.SubHidnInfo.ubSrcLocate[tDispLocate];
			if(tCamNum == tUI_RecPlayAct.tAdoPlayCam)
			{
				if(tDispLocate == 0)
				{
					tVoiceOsdImgInfo.uwYStart = 5;
				}					
				else if(tDispLocate == 1)
				{
					tVoiceOsdImgInfo.uwYStart = 5;
					tVoiceOsdImgInfo.uwXStart += OSD_WIDTH/2;
				}
				else if(tDispLocate == 2)
				{
					tVoiceOsdImgInfo.uwYStart = 5 + OSD_HEIGHT/2;
				}
				else
				{
					tVoiceOsdImgInfo.uwYStart = 5 + OSD_HEIGHT/2;
					tVoiceOsdImgInfo.uwXStart += OSD_WIDTH/2;
				}
				tOSD_Img2(&tVoiceOsdImgInfo, OSD_QUEUE);
			}
		}		

	}
	else if(tPlayDispTye == KNL_DISP_H)
	{
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_DESKTOP_VOICE_MUTE + 1 - tUI_CuSetting.ubMuteFlag, 1, &tVoiceOsdImgInfo);//voice
		for(tDispLocate = KNL_DISP_LOCATION1;tDispLocate <= KNL_DISP_LOCATION4;tDispLocate ++)
		{
			UI_CamNum_t tCamNum = tUI_RecFilesInfo.tRecFilesInfo[uwCurVideoPlayIdx[0]].HidnFileInfo.SubHidnInfo.ubSrcLocate[tDispLocate];
			if(tCamNum == tUI_RecPlayAct.tAdoPlayCam)
			{
				if(tDispLocate == KNL_DISP_LOCATION4)
				{
					tVoiceOsdImgInfo.uwYStart = 5;
					tVoiceOsdImgInfo.uwXStart -= OSD_WIDTH/4;
				}					
				else if(tDispLocate == KNL_DISP_LOCATION1)
				{
					tVoiceOsdImgInfo.uwYStart = 5;
					tVoiceOsdImgInfo.uwXStart += OSD_WIDTH/2;
				}
				else if(tDispLocate == KNL_DISP_LOCATION2)
				{
					tVoiceOsdImgInfo.uwYStart = 5;
					tVoiceOsdImgInfo.uwXStart += OSD_WIDTH/4;
				}
				else if(tDispLocate == KNL_DISP_LOCATION3)
				{
					tVoiceOsdImgInfo.uwYStart = 5 + OSD_HEIGHT/2;
					tVoiceOsdImgInfo.uwXStart += OSD_WIDTH/4;
				}
				tOSD_Img2(&tVoiceOsdImgInfo, OSD_QUEUE);
			}
		}		

	}

}
//----------------------------------------------------------------------

static uint16_t uwPlayAdoSrcImgIdx[UI_PLAYADOSRC_MAX] = {
													OSD2IMG_SELADOCAM1_NOR_ICON,
													OSD2IMG_SELADOCAM2_NOR_ICON,
													OSD2IMG_SELADOCAM3_NOR_ICON,
													OSD2IMG_SELADOCAM4_NOR_ICON, 
							                      };

void UI_PlaybackSubSubSubSubMenuPage(UI_ArrowKey_t tArrowKey)
{
	tUI_RecPlayAdoSrcItem.ubItemPreIdx = tUI_RecPlayAdoSrcItem.ubItemIdx;
	switch(tArrowKey)
	{
		case LEFT_ARROW:
			if(tUI_RecPlayAdoSrcItem.ubItemIdx > UI_PLAYADOSRC1_ITEM)
				tUI_RecPlayAdoSrcItem.ubItemIdx --;
			else
				tUI_RecPlayAdoSrcItem.ubItemIdx = UI_PLAYADOSRC4_ITEM;
			break;
		case RIGHT_ARROW:
			if(tUI_RecPlayAdoSrcItem.ubItemIdx < UI_PLAYADOSRC4_ITEM)
				tUI_RecPlayAdoSrcItem.ubItemIdx ++;
			else
				tUI_RecPlayAdoSrcItem.ubItemIdx = UI_PLAYADOSRC1_ITEM;
			break;
		case ENTER_ARROW:
			tUI_RecPlayAct.tAdoPlayCam = tUI_RecPlayAdoSrcItem.ubItemIdx;
			ubPLY_AdoChannelSet(tUI_RecPlayAct.tAdoPlayCam);
			UI_PlayBackUpdateVoiceIcon();
		case EXIT_ARROW:
			if(ubPLY_Pause(PLY_PAUSE_OFF) == 0)//继续播放
	           return;
			OSD_IMG_INFO tOsdImgInfo[8];
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_SELADOCAM1_NOR_ICON, 8, &tOsdImgInfo[0]);
			for(uint8_t i = 0;i < 4;i++)
				OSD_EraserImg2(&tOsdImgInfo[2*i]);
			
			tUI_State = UI_SUBSUBSUBMENU_STATE;
			return;			
		default:
			return;
	}
	
	UI_DrawHLandNormalIcon(uwPlayAdoSrcImgIdx[tUI_RecPlayAdoSrcItem.ubItemPreIdx], (uwPlayAdoSrcImgIdx[tUI_RecPlayAdoSrcItem.ubItemIdx] + UI_ICON_HIGHLIGHT));
}
//------------------------------------------------------------------------------------setting------------------------------------------------------------------------------
//------------------------------------------------------------------------------------setting------------------------------------------------------------------------------
//------------------------------------------------------------------------------------setting------------------------------------------------------------------------------
uint8_t ubUI_DateTimeSubSubSubSubStsUpdateFlag = FALSE;
static uint16_t TempDateTimeValue;

static void UI_DateTimeSubSubSubSubUpdateItemIndex(uint16_t uwsubsubsubItemIdx)
{
	
	if(!ubUI_DateTimeSubSubSubSubStsUpdateFlag)
	{	
		memset(&(tDateTimeSubSubSubSubMenuItem.tSubMenuInfo), 0, sizeof(UI_MenuItem_t));

		if(uwsubsubsubItemIdx == DATE_TIME_YEAR)
			TempDateTimeValue = tUI_CuSetting.tSysCalendar.uwYear;
		else if(uwsubsubsubItemIdx == DATE_TIME_MONTH)
			 TempDateTimeValue = tUI_CuSetting.tSysCalendar.ubMonth;
		else if(uwsubsubsubItemIdx == DATE_TIME_DAY)
			 TempDateTimeValue = tUI_CuSetting.tSysCalendar.ubDate;
		else if(uwsubsubsubItemIdx == DATE_TIME_HOUR)
			 TempDateTimeValue = tUI_CuSetting.tSysCalendar.ubHour;
		else if(uwsubsubsubItemIdx == DATE_TIME_MINUTE)
			 TempDateTimeValue = tUI_CuSetting.tSysCalendar.ubMin;
		else if(uwsubsubsubItemIdx == DATE_TIME_SECOND)
			 TempDateTimeValue = tUI_CuSetting.tSysCalendar.ubSec;

		ubUI_DateTimeSubSubSubSubStsUpdateFlag = TRUE;
	}
}
//----------------------------------------------------------------------
void UI_DateTimeDrawSubSubSubSubMenuItem(void)
{
	uint16_t uwKeyBoardItemOsdImg[12] = {
										OSD2IMG_KEYBOARD_BUTTON1_NOR, 
										OSD2IMG_KEYBOARD_BUTTON2_NOR,
										OSD2IMG_KEYBOARD_BUTTON3_NOR, 
										OSD2IMG_KEYBOARD_BUTTON4_NOR,
										OSD2IMG_KEYBOARD_BUTTON5_NOR, 
										OSD2IMG_KEYBOARD_BUTTON6_NOR,
										OSD2IMG_KEYBOARD_BUTTON7_NOR,
										OSD2IMG_KEYBOARD_BUTTON8_NOR, 
										OSD2IMG_KEYBOARD_BUTTON9_NOR,
										OSD2IMG_KEYBOARD_BUTTON0_NOR,
										OSD2IMG_KEYBOARD_BUTTON_ENTER_NOR,
										OSD2IMG_KEYBOARD_BUTTON_DELETE_NOR
									   };
	uint8_t ubItemPreIdx = tDateTimeSubSubSubSubMenuItem.tSubMenuInfo.ubItemPreIdx;
	uint8_t ubItemIdx = tDateTimeSubSubSubSubMenuItem.tSubMenuInfo.ubItemIdx;
	
	UI_DrawHLandNormalIcon(uwKeyBoardItemOsdImg[ubItemPreIdx],(uwKeyBoardItemOsdImg[ubItemIdx] + UI_ICON_HIGHLIGHT));
}
//----------------------------------------------------------------------
static void UI_DateTimeSubSubSubSubMenuExit(uint16_t uwsubsubsubItemIdx)
{
	OSD_IMG_INFO tOsdImgInfo[12],tMenuOsdImgInfo;
	uint16_t uwTemp[6];
	tOSD_GetOsdImgInfor(1, OSD_IMG1, OSD1IMG_SUBMENU2, 1, &tMenuOsdImgInfo);
	tOSD_Img1(&tMenuOsdImgInfo, OSD_QUEUE);
	if(!ubUI_TouchPanelSts)
	{
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_RETURN_BLACK, 1, &tMenuOsdImgInfo);
		tOSD_Img2(&tMenuOsdImgInfo, OSD_QUEUE);
	}
	
	//WORD
	tOSD_GetOsdImgInfor(1, OSD_IMG2, (tUI_CuSetting.tLanguage == LANGUAGE_ENGLISH)?OSD2IMG_DATE_TIME_DATE_WORD:
									(tUI_CuSetting.tLanguage == LANGUAGE_GERMAN)?OSD2IMG_DATE_TIME_DATE_WORD_GER:
									(tUI_CuSetting.tLanguage == LANGUAGE_FRENCH)?OSD2IMG_DATE_TIME_DATE_WORD_FR:OSD2IMG_DATE_TIME_DATE_WORD_CHN, 3, &tOsdImgInfo[0]);
	for(uint8_t i = 0;i < 3;i++)
		tOSD_Img2(&tOsdImgInfo[i], OSD_QUEUE);


	// UTC Slider
    tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_TIME_ZONE_SLIDER, 6, &tOsdImgInfo[0]);
	tOSD_Img2(&tOsdImgInfo[0], OSD_QUEUE);
	tOsdImgInfo[2].uwXStart = UI_Map(tUI_CuSetting.ubtimezone,0,26,tOsdImgInfo[0].uwXStart,tOsdImgInfo[0].uwXStart + tOsdImgInfo[0].uwHSize - tOsdImgInfo[2].uwHSize);
	tOSD_Img2(&tOsdImgInfo[2], OSD_QUEUE);
	
	UI_ShowUTCVal();
	// UTC comfirm button
	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_DATE_TIME_UTC_COMFIRM_NOR, 1, &tOsdImgInfo[0]);
	tOSD_Img2(&tOsdImgInfo[0], OSD_QUEUE);

	UI_SystemDrawDateTimeYearButton(uwsubsubsubItemIdx);
	//BUTTON
//	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_DATE_TIME_YEAR_NOR, 12, &tOsdImgInfo[0]);
//	for(uint8_t i = 0;i < 6;i++)
//		tOSD_Img2(&tOsdImgInfo[2*i], OSD_QUEUE);
//	//HIGHLIGHT BUTTON
//	tOSD_Img2(&tOsdImgInfo[2*uwsubsubsubItemIdx + UI_ICON_HIGHLIGHT], OSD_QUEUE);
//		
//	uwTemp[0] = tUI_CuSetting.tSysCalendar.uwYear;
//	uwTemp[1] = tUI_CuSetting.tSysCalendar.ubMonth;
//	uwTemp[2] = tUI_CuSetting.tSysCalendar.ubDate;
//	uwTemp[3] = tUI_CuSetting.tSysCalendar.ubHour;
//	uwTemp[4] = tUI_CuSetting.tSysCalendar.ubMin;
//	uwTemp[5] = tUI_CuSetting.tSysCalendar.ubSec;

//	for(uint8_t i = 0;i < 6;i++)
//	{
//		if(uwTemp[i] < 10)
//		{
//			UI_ShowButtonValueNormal(0,tOsdImgInfo[2*i].uwXStart + 30,tOsdImgInfo[2*i].uwYStart + 15,OSD_QUEUE);
//			UI_ShowButtonValueNormal(uwTemp[i],tOsdImgInfo[2*i].uwXStart + 50,tOsdImgInfo[2*i].uwYStart + 15,OSD_QUEUE);
//		}
//		else if(uwTemp[i] < 100)
//			UI_ShowButtonValueNormal(uwTemp[i],tOsdImgInfo[2*i].uwXStart + 30,tOsdImgInfo[2*i].uwYStart + 15,OSD_QUEUE);
//		else
//			UI_ShowButtonValueNormal(tUI_CuSetting.tSysCalendar.uwYear,tOsdImgInfo[2*i].uwXStart + 10,tOsdImgInfo[2*i].uwYStart + 15,OSD_QUEUE);
//	}

//	//hight button value
//	uint8_t i = uwsubsubsubItemIdx;
//	if(uwTemp[i] < 10)
//	{
//		UI_ShowButtonValueHighLight(0,tOsdImgInfo[2*i].uwXStart + 30,tOsdImgInfo[2*i].uwYStart + 15,OSD_UPDATE);
//		UI_ShowButtonValueHighLight(uwTemp[i],tOsdImgInfo[2*i].uwXStart + 50,tOsdImgInfo[2*i].uwYStart + 15,OSD_UPDATE);
//	}
//	else if(uwTemp[i] < 100)
//		UI_ShowButtonValueHighLight(uwTemp[i],tOsdImgInfo[2*i].uwXStart + 30,tOsdImgInfo[2*i].uwYStart + 15,OSD_UPDATE);
//	else
//		UI_ShowButtonValueHighLight(tUI_CuSetting.tSysCalendar.uwYear,tOsdImgInfo[2*i].uwXStart + 10,tOsdImgInfo[2*i].uwYStart + 15,OSD_UPDATE);
	ubUI_DateTimeSubSubSubSubStsUpdateFlag = FALSE;
	tUI_State = UI_SUBSUBSUBMENU_STATE;
}


//---------------------------------------------------------------------                                                     -
static void UI_DateTimeSubSubSubSubMenuExecute(uint16_t uwsubsubsubItemIdx)
{
	uint16_t uwsubsubsubSubItemIdx = tDateTimeSubSubSubSubMenuItem.tSubMenuInfo.ubItemIdx;

	uint16_t uwYear = TempDateTimeValue;
	
	if(uwsubsubsubSubItemIdx <= KEYBOARD_KEY0)
	{
		if(uwsubsubsubSubItemIdx <= KEYBOARD_KEY9)
		{
			TempDateTimeValue = TempDateTimeValue*10 + uwsubsubsubSubItemIdx + 1;
			
		}
		else if(uwsubsubsubSubItemIdx == KEYBOARD_KEY0)
		{
			TempDateTimeValue = TempDateTimeValue*10;
		}

		if(uwsubsubsubItemIdx == DATE_TIME_YEAR)
		{
			if(TempDateTimeValue > 9999)
				TempDateTimeValue = uwYear;
		}
		else if(uwsubsubsubItemIdx == DATE_TIME_MONTH)
		{
			if(TempDateTimeValue > 12)
				TempDateTimeValue = 12;
		}
		else if(uwsubsubsubItemIdx == DATE_TIME_DAY)
		{
			if(tUI_CuSetting.tSysCalendar.ubMonth == 1 || tUI_CuSetting.tSysCalendar.ubMonth == 3 || tUI_CuSetting.tSysCalendar.ubMonth == 5
				|| tUI_CuSetting.tSysCalendar.ubMonth == 7 || tUI_CuSetting.tSysCalendar.ubMonth == 8 || tUI_CuSetting.tSysCalendar.ubMonth == 10
				 || tUI_CuSetting.tSysCalendar.ubMonth == 12)
			{
				if(TempDateTimeValue > 31)
					TempDateTimeValue = 31;
			}
			else if(tUI_CuSetting.tSysCalendar.ubMonth == 4 || tUI_CuSetting.tSysCalendar.ubMonth == 6
				|| tUI_CuSetting.tSysCalendar.ubMonth == 9 || tUI_CuSetting.tSysCalendar.ubMonth == 11)
			{
				if(TempDateTimeValue > 30)
					TempDateTimeValue = 30;
			}
			else
			{
				if(tUI_CuSetting.tSysCalendar.uwYear % 100)//不是世纪年
				{
					if(tUI_CuSetting.tSysCalendar.uwYear % 4)//平年
					{
						if(TempDateTimeValue > 28)
							TempDateTimeValue = 28;
					}
					else
					{
						if(TempDateTimeValue > 29)
							TempDateTimeValue = 29;

					}
				}
				else
				{
					if(tUI_CuSetting.tSysCalendar.uwYear % 400)//平年
					{
						if(TempDateTimeValue > 28)
							TempDateTimeValue = 28;
					}
					else
					{
						if(TempDateTimeValue > 29)
							TempDateTimeValue = 29;

					}
				}
			}
				
		}
		else if(uwsubsubsubItemIdx == DATE_TIME_HOUR)
		{
			if(TempDateTimeValue > 23)
				TempDateTimeValue = 23;
		}
		else if(uwsubsubsubItemIdx == DATE_TIME_MINUTE)
		{
			if(TempDateTimeValue > 59)
				TempDateTimeValue = 59;
		}
		else if(uwsubsubsubItemIdx == DATE_TIME_SECOND)
		{
			if(TempDateTimeValue > 59)
				TempDateTimeValue = 59;
	
		}
	}
	else if(uwsubsubsubSubItemIdx == KEYBOARD_KEY_ENTER)
	{
		if(uwsubsubsubItemIdx == DATE_TIME_YEAR)
			 tUI_CuSetting.tSysCalendar.uwYear = TempDateTimeValue;
		else if(uwsubsubsubItemIdx == DATE_TIME_MONTH)
			 tUI_CuSetting.tSysCalendar.ubMonth = TempDateTimeValue;
		else if(uwsubsubsubItemIdx == DATE_TIME_DAY)
			 tUI_CuSetting.tSysCalendar.ubDate = TempDateTimeValue;
		else if(uwsubsubsubItemIdx == DATE_TIME_HOUR)
			 tUI_CuSetting.tSysCalendar.ubHour = TempDateTimeValue;
		else if(uwsubsubsubItemIdx == DATE_TIME_MINUTE)
			tUI_CuSetting.tSysCalendar.ubMin = TempDateTimeValue;
		else if(uwsubsubsubItemIdx == DATE_TIME_SECOND)
			tUI_CuSetting.tSysCalendar.ubSec = TempDateTimeValue;
		RTC_SetCalendar((RTC_Calendar_t *)(&tUI_CuSetting.tSysCalendar));
		UI_DateTimeSubSubSubSubMenuExit(uwsubsubsubItemIdx);
		return;
	}
	else if(uwsubsubsubSubItemIdx == KEYBOARD_KEY_DELETE)
	{
		TempDateTimeValue  = TempDateTimeValue / 10;
	}
	
	
	UI_ShowKeyBoardValue(TempDateTimeValue,OSD_UPDATE);

}

//----------------------------------------------------------------------
void UI_DateTimeSubSubSubSubMenuPage(UI_ArrowKey_t tArrowKey)
{
	uint16_t uwsubsubsubItemIdx = 
		tSystemSubSubSubMenuItem[DATETIME_ITEM].tSubMenuInfo.ubItemIdx;//sub sub sub
	
	UI_DateTimeSubSubSubSubUpdateItemIndex(uwsubsubsubItemIdx);
	tDateTimeSubSubSubSubMenuItem.tSubMenuInfo.ubItemPreIdx = tDateTimeSubSubSubSubMenuItem.tSubMenuInfo.ubItemIdx;
	switch(tArrowKey)
	{
		case LEFT_ARROW:
			if(tDateTimeSubSubSubSubMenuItem.tSubMenuInfo.ubItemIdx == tDateTimeSubSubSubSubMenuItem.ubFirstItem)
				tDateTimeSubSubSubSubMenuItem.tSubMenuInfo.ubItemIdx = tDateTimeSubSubSubSubMenuItem.ubItemCount - 1;
			else
				tDateTimeSubSubSubSubMenuItem.tSubMenuInfo.ubItemIdx --;
			break;
		case RIGHT_ARROW:
			if(tDateTimeSubSubSubSubMenuItem.tSubMenuInfo.ubItemIdx == tDateTimeSubSubSubSubMenuItem.ubItemCount - 1)
				tDateTimeSubSubSubSubMenuItem.tSubMenuInfo.ubItemIdx = tDateTimeSubSubSubSubMenuItem.ubFirstItem;
			else
				tDateTimeSubSubSubSubMenuItem.tSubMenuInfo.ubItemIdx ++;
			break;
		case ENTER_ARROW:
			UI_DateTimeSubSubSubSubMenuExecute(uwsubsubsubItemIdx);
			return;
		case EXIT_ARROW:
			UI_DateTimeSubSubSubSubMenuExit(uwsubsubsubItemIdx);
			return;
		default:
			return;
	}
	UI_DateTimeDrawSubSubSubSubMenuItem();
	
}



/********************************************************************TOUCH*********************************************************************************************/
/********************************************************************TOUCH*********************************************************************************************/
/********************************************************************TOUCH*********************************************************************************************/


//----------------------------------------------------------------------
void UI_SubSubSubSubTouchMenu(TOUCH_EVENT_t *Touch_Info)
{
	switch(tUI_MenuItem.ubItemIdx)
	{
		case PLAYBACK_ITEM:
			UI_PlaybackSubSubSubSubTouchMenu(Touch_Info);
			break;
			case SETTING_ITEM:
			{
				UI_SettingSubMenuItemList_t tSubMenuItem = 
					(UI_SettingSubMenuItemList_t)tUI_SubMenuItem[SETTING_ITEM].tSubMenuInfo.ubItemIdx;//sub

				uint16_t uwSubSubMenuItemIdx = 
					(uint16_t)tSystemSubSubMenuItem.tSubMenuInfo.ubItemIdx;//sub sub

				if(tSubMenuItem == SYSTEMSET_ITEM && uwSubSubMenuItemIdx == DATETIME_ITEM)
					UI_DateTimeSubSubSubSubTouchMenu(Touch_Info);
			
			break;
			}
		default:
			break;
	}
}

//------------------------------------------------------------------------------------playback------------------------------------------------------------------------------
//------------------------------------------------------------------------------------playback------------------------------------------------------------------------------
//------------------------------------------------------------------------------------playback------------------------------------------------------------------------------

//----------------------------------------------------------------------

void UI_PlaybackSubSubSubSubTouchMenu(TOUCH_EVENT_t *Touch_Info)
{

	OSD_IMG_INFO tOsdImgInfo;
	uint16_t i;
	tUI_RecPlayAdoSrcItem.ubItemPreIdx = tUI_RecPlayAdoSrcItem.ubItemIdx;

	switch(Touch_Info->Gesture)
	{
		case TOUCH_PRESS:
			//解析手指点击在哪个图标上
			for(i = 0; i < UI_PLAYADOSRC_MAX;i++)//判断具体点击在哪个图标上
			{
				//读取图片信息
				tOSD_GetOsdImgInfor(1, OSD_IMG2, uwPlayAdoSrcImgIdx[i],1, &tOsdImgInfo);
			
				if(Touch_Info->startX > tOsdImgInfo.uwXStart && Touch_Info->startX < tOsdImgInfo.uwXStart + tOsdImgInfo.uwHSize
					&& Touch_Info->startY > tOsdImgInfo.uwYStart && Touch_Info->startY < tOsdImgInfo.uwYStart + tOsdImgInfo.uwVSize)
				{
					tUI_RecPlayAdoSrcItem.ubItemIdx = i;
					break;
				}
			}
			if(UI_PLAYADOSRC_MAX == i)//没有按到有效的控件
			{
				UI_MenuKey();
				return;
			}
			
			UI_DrawHLandNormalIcon(uwPlayAdoSrcImgIdx[tUI_RecPlayAdoSrcItem.ubItemPreIdx], 
				(uwPlayAdoSrcImgIdx[tUI_RecPlayAdoSrcItem.ubItemIdx] + UI_ICON_HIGHLIGHT));
	
			UI_EnterKey();
			break;
			
		case TOUCH_TURNDOWN:
			break;

		default:

			return;
	}

}

//------------------------------------------------------------------------------------date time------------------------------------------------------------------------------
//------------------------------------------------------------------------------------date time------------------------------------------------------------------------------
//------------------------------------------------------------------------------------date time------------------------------------------------------------------------------

//----------------------------------------------------------------------


void UI_DateTimeSubSubSubSubTouchMenu(TOUCH_EVENT_t *Touch_Info)
{

	OSD_IMG_INFO tOsdImgInfo;
	uint16_t i;
	uint16_t uwsubsubsubItemIdx = tSystemSubSubSubMenuItem[DATETIME_ITEM].tSubMenuInfo.ubItemIdx;
	UI_DateTimeSubSubSubSubUpdateItemIndex(uwsubsubsubItemIdx);
	tDateTimeSubSubSubSubMenuItem.tSubMenuInfo.ubItemPreIdx = tDateTimeSubSubSubSubMenuItem.tSubMenuInfo.ubItemIdx;

	switch(Touch_Info->Gesture)
	{
		case TOUCH_PRESS:
			//解析手指点击在哪个图标上
			for(i = 0; i < KEYBOARD_KEY_MAX;i++)//判断具体点击在哪个图标上
			{
				//读取图片信息
				tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_KEYBOARD_BUTTON1_NOR + 2*i,1, &tOsdImgInfo);
			
				if(Touch_Info->startX > tOsdImgInfo.uwXStart && Touch_Info->startX < tOsdImgInfo.uwXStart + tOsdImgInfo.uwHSize
					&& Touch_Info->startY > tOsdImgInfo.uwYStart && Touch_Info->startY < tOsdImgInfo.uwYStart + tOsdImgInfo.uwVSize)
				{
					tDateTimeSubSubSubSubMenuItem.tSubMenuInfo.ubItemIdx = i;
					break;
				}
			}
			if(i < KEYBOARD_KEY_MAX)
			{
				if(tDateTimeSubSubSubSubMenuItem.tSubMenuInfo.ubItemPreIdx != 
							tDateTimeSubSubSubSubMenuItem.tSubMenuInfo.ubItemIdx)
				{
					UI_DateTimeDrawSubSubSubSubMenuItem();
				}
				UI_EnterKey();
			}
			else//return
			{
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
			}
			break;
			
		case TOUCH_TURNDOWN:
			break;

		default:

			return;
	}

}

#endif

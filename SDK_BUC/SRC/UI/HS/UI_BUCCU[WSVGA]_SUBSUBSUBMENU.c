
#include "UI_BUCCU[WSVGA]_SUBSUBSUBMENU.h"
#include "math.h"
#ifdef BSP_D_SNCC71_GM8285C_RX_V2

void UI_CameraSubSubSubMenuPage(UI_ArrowKey_t tArrowKey);
void UI_CameraSubSubSubTouchMenu(TOUCH_EVENT_t *Touch_Info);
void UI_SystemSubSubSubMenuPage(UI_ArrowKey_t tArrowKey);
void UI_SystemSubSubSubTouchMenu(TOUCH_EVENT_t *Touch_Info);

void UI_ParkingLineSubSubSubMenuPage(UI_ArrowKey_t tArrowKey);
void UI_ParkingLIneSubSubSubTouchMenu(TOUCH_EVENT_t *Touch_Info);

void UI_GuideLineSubSubSubMenuPage(UI_ArrowKey_t tArrowKey);
void UI_GuideLineSubSubSubTouchMenu(TOUCH_EVENT_t *Touch_Info);

void AISubSubSubMenuPage(UI_ArrowKey_t tArrowKey);
void UI_AISubSubSubTouchMenu(TOUCH_EVENT_t *Touch_Info);

void UI_AISubSubSubMenuPage(UI_ArrowKey_t tArrowKey);

void UI_SubSubSubKeyMenu(UI_ArrowKey_t tArrowKey)
{
	switch(tUI_MenuItem.ubItemIdx)
	{	
		case PLAYBACK_ITEM:
			UI_PlaybackSubSubSubMenuPage(tArrowKey);
			break;
		case SETTING_ITEM:
			{
				UI_SettingSubMenuItemList_t tSubMenuItem = 
					(UI_SettingSubMenuItemList_t)tUI_SubMenuItem[SETTING_ITEM].tSubMenuInfo.ubItemIdx;//sub
				 switch(tSubMenuItem)
				 {
				 	case CAMERASET_ITEM:
						UI_CameraSubSubSubMenuPage(tArrowKey);
						break;
					case SYSTEMSET_ITEM:
						UI_SystemSubSubSubMenuPage(tArrowKey);
						break;
					case PARKINGLINE_ITEM:
						UI_ParkingLineSubSubSubMenuPage(tArrowKey);
						break;
					case GUIDELINE_ITEM:
						UI_GuideLineSubSubSubMenuPage(tArrowKey);
						break;
					case AI_ITEM:
						UI_AISubSubSubMenuPage(tArrowKey);
						break;
					default:
						UI_SettingSubSubSubMenuPage(tArrowKey);
						break;
				 }
					
			}
			
			break;
		default:
			break;
	}
}

//------------------------------------------------------------------------------------playback------------------------------------------------------------------------------
//------------------------------------------------------------------------------------playback------------------------------------------------------------------------------
//------------------------------------------------------------------------------------playback------------------------------------------------------------------------------

//------------------------------------------------------------------------------------
static UI_RecPlayStatus_t tUI_RecPlaySts = UI_RECFILE_PLAY;
static uint16_t uwRecPlayListImgIdx[UI_RECPLAYLISTITEM_MAX] = {
																	OSD2IMG_REC_ADOSRCSELNOR_ICON,
																	OSD2IMG_REC_SKIPBACKWARDNOR_ICON,
																	OSD2IMG_REC_PAUSENOR_ICON,
																	OSD2IMG_REC_SKIPFORWARDNOR_ICON, 
																	OSD2IMG_REC_RETURNNOR_ICON, 
											                      };

void UI_PlaybackSubSubSubMenuPage(UI_ArrowKey_t tArrowKey)
{
	
	OSD_IMG_INFO tRecPlayOsdImgInfo[2];
	tUI_RecPlayListItem.ubItemPreIdx = tUI_RecPlayListItem.ubItemIdx;

	if(UI_SUBSUBSUBMENU_STATE != tUI_State)
		return;
	
	switch(tArrowKey)
	{
		case LEFT_ARROW:
			if(UI_RECADOSRCSEL_ITEM == tUI_RecPlayListItem.ubItemIdx)
				tUI_RecPlayListItem.ubItemIdx = UI_RECRETURN_ITEM;
			else
				tUI_RecPlayListItem.ubItemIdx--;
			break;
		case RIGHT_ARROW:
			if(tUI_RecPlayListItem.ubItemIdx  == UI_RECRETURN_ITEM)
				tUI_RecPlayListItem.ubItemIdx = UI_RECADOSRCSEL_ITEM;
			else
				tUI_RecPlayListItem.ubItemIdx++;
			break;
		case ENTER_ARROW:
			if(UI_RECFILE_PLAY != tUI_RecPlayAct.tPlaySts)//当前已停止播放
			{
				if(UI_RECPLAYPAUSE_ITEM == tUI_RecPlayListItem.ubItemIdx)
				{
					uwRecPlayListImgIdx[UI_RECPLAYPAUSE_ITEM] = OSD2IMG_REC_PAUSENOR_ICON;
					UI_StartPlayRecordFile((UI_RecPlayDispType_t)tUI_RecPlayAct.ubPlayMode);
				}
				else{
					uwRecPlayListImgIdx[UI_RECPLAYPAUSE_ITEM] = OSD2IMG_REC_PLAYNOR_ICON;
				}
			}
			else//当前正在播放或者暂停
			{
				if(UI_RECPLAYPAUSE_ITEM == tUI_RecPlayListItem.ubItemIdx)//播放停止按钮
				{
					if(UI_RECFILE_PLAY == tUI_RecPlaySts)
					{ 
						if(ubPLY_Pause(PLY_PAUSE_ON) == 0)
	                    	return;
						uwRecPlayListImgIdx[UI_RECPLAYPAUSE_ITEM] = OSD2IMG_REC_PLAYNOR_ICON;
						tUI_RecPlaySts = UI_RECFILE_PAUSE;
					}
					else if(UI_RECFILE_PAUSE == tUI_RecPlaySts)
					{
						if(ubPLY_Pause(PLY_PAUSE_OFF) == 0)
	                    	return;
						uwRecPlayListImgIdx[UI_RECPLAYPAUSE_ITEM] = OSD2IMG_REC_PAUSENOR_ICON;
						tUI_RecPlaySts = UI_RECFILE_PLAY;                   
					}
				}
				if(UI_RECFILE_PAUSE != tUI_RecPlaySts)//只有在播放状态才可以进行进度调节和音源选择
				{
		            if(UI_RECSKIPBKFWD_ITEM == tUI_RecPlayListItem.ubItemIdx)
					{
						ubPLY_Jump(PLY_JUMP_BWD);
					}
		            else if(UI_RECSKIPFRFWD_ITEM == tUI_RecPlayListItem.ubItemIdx)
					{
						ubPLY_Jump(PLY_JUMP_FWD);
					}
					else if(UI_RECADOSRCSEL_ITEM == tUI_RecPlayListItem.ubItemIdx)
					{
						if(tUI_RecPlayAct.ubPlayMode == UI_RECPLAY_SINGLEVIEW)
							return;
						if(ubPLY_Pause(PLY_PAUSE_ON) == 0)//停止播放，选完音频后再继续播放
	                    	return;
						OSD_IMG_INFO tOsdImgInfo[8];
						tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_SELADOCAM1_NOR_ICON, 8, &tOsdImgInfo[0]);
						
						for(uint8_t i = 0;i < 4;i++)
							tOSD_Img2(&tOsdImgInfo[2*i], OSD_QUEUE);	
						tOSD_Img2(&tOsdImgInfo[2*tUI_RecPlayAct.tAdoPlayCam + UI_ICON_HIGHLIGHT], OSD_UPDATE);

						tUI_RecPlayAdoSrcItem.ubItemIdx = tUI_RecPlayAct.tAdoPlayCam;
						tUI_State = UI_SUBSUBSUBSUBMENU_STATE;
					}
				}
			}
			UI_DrawHLandNormalIcon(uwRecPlayListImgIdx[tUI_RecPlayListItem.ubItemPreIdx], (uwRecPlayListImgIdx[tUI_RecPlayListItem.ubItemIdx] + UI_ICON_HIGHLIGHT));
			if(tUI_RecPlayListItem.ubItemIdx != UI_RECRETURN_ITEM)//return
				return;
		case EXIT_ARROW:
			tUI_RecPlayAct.tPlaySts = UI_RECFILE_STOP;
			KNL_VideoPlayStop();
			tUI_RecPlaySts      = UI_RECFILE_PLAY;
			tUI_RecPlayListItem.ubItemIdx = UI_RECPLAYPAUSE_ITEM;
			uwRecPlayListImgIdx[UI_RECPLAYPAUSE_ITEM] = OSD2IMG_REC_PAUSENOR_ICON;
				
			osSemaphoreWait(osUI_CuUiCtr, osWaitForever);
			tOSD_GetOsdImgInfor(1, OSD_IMG1, OSD1IMG_SUBMENU2, 1, &tRecPlayOsdImgInfo[0]);
			tOSD_Img1(&tRecPlayOsdImgInfo[0], OSD_UPDATE);
			if(!ubUI_TouchPanelSts)
			{
				tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_RETURN_BLACK, 1, &tRecPlayOsdImgInfo[0]);
				tOSD_Img2(&tRecPlayOsdImgInfo[0], OSD_QUEUE);
			}
			UI_DrawRecordFileMenu();
			osSemaphoreRelease(osUI_CuUiCtr);
			return;
		case DOWN_ARROW:
			if(UI_RECFILE_PLAY != tUI_RecPlayAct.tPlaySts)
			{
				tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_REC_PLAYHL_ICON, 1, &tRecPlayOsdImgInfo[0]);
				if(UI_RECPLAYPAUSE_ITEM != tUI_RecPlayListItem.ubItemIdx)
				{
					tUI_RecPlayListItem.ubItemPreIdx = tUI_RecPlayListItem.ubItemIdx;
					tUI_RecPlayListItem.ubItemIdx = UI_RECPLAYPAUSE_ITEM;
					uwRecPlayListImgIdx[UI_RECPLAYPAUSE_ITEM] = OSD2IMG_REC_PLAYNOR_ICON;
					tOSD_Img2(&tRecPlayOsdImgInfo[0], OSD_QUEUE);
					break;
				}
				else
				{
					uwRecPlayListImgIdx[UI_RECPLAYPAUSE_ITEM] = OSD2IMG_REC_PLAYNOR_ICON;
					tOSD_Img2(&tRecPlayOsdImgInfo[0], OSD_UPDATE);
				}
			}
		default:
			return;
	}
	UI_DrawHLandNormalIcon(uwRecPlayListImgIdx[tUI_RecPlayListItem.ubItemPreIdx], (uwRecPlayListImgIdx[tUI_RecPlayListItem.ubItemIdx] + UI_ICON_HIGHLIGHT));
}


//------------------------------------------------------------------------------------setting------------------------------------------------------------------------------
//------------------------------------------------------------------------------------setting------------------------------------------------------------------------------
//------------------------------------------------------------------------------------setting------------------------------------------------------------------------------

//------------------------------------------------------------------------------------camera------------------------------------------------------------------------------


//------------------------------------------------------------------------------------
static void UI_CameraSubSubSubMenuExecute()
{
	uint16_t uwSubSubMenuItemIdx = 
		(uint16_t)tSettingSubSubMenuItem.tSettingS[CAMERASET_ITEM].tSubMenuInfo.ubItemIdx;
	uint8_t ubItemIdx = tCameraSubSubSubMenuItem[uwSubSubMenuItemIdx]->tSubMenuInfo.ubItemIdx;
	uint8_t ubValue;
	UI_CamNum_t tCamNum = (UI_CamNum_t)uwSubSubMenuItemIdx;
	UI_CUReqCmd_t tCamSetCmd;
	tCamSetCmd.tDS_CamNum				= tCamNum;
	tCamSetCmd.ubCmd[UI_TWC_TYPE]		= UI_SETTING;
	tCamSetCmd.ubCmd[UI_SETTING_ITEM]	= UI_IMGPROC_SETTING;
	tCamSetCmd.ubCmd_Len				 = 4;
	

	if(ubItemIdx == CAMERA_BRIGHTNESS)
	{
		if(tUI_CamStatus[tCamNum].tCamColorParam.ubColorBL + tUI_CuSetting.bColorBLGain[tCamNum] > 99)
			return;
		ubValue = UI_Map(tUI_CamStatus[tCamNum].tCamColorParam.ubColorBL + tUI_CuSetting.bColorBLGain[tCamNum],0,99,0,127);
		tCamSetCmd.ubCmd[UI_SETTING_DATA]	 = UI_IMGBL_SETTING ;
		tCamSetCmd.ubCmd[UI_SETTING_DATA+1] = ubValue;
	}
	else if(ubItemIdx == CAMERA_CONTRAST)
	{
		if(tUI_CamStatus[tCamNum].tCamColorParam.ubColorContrast + tUI_CuSetting.bColorContrastGain[tCamNum] < 0)
			return;
		ubValue = UI_Map(tUI_CamStatus[tCamNum].tCamColorParam.ubColorContrast + tUI_CuSetting.bColorContrastGain[tCamNum],0,99,0,127);
		tCamSetCmd.ubCmd[UI_SETTING_DATA]	 = UI_IMGCONTRAST_SETTING ;
		tCamSetCmd.ubCmd[UI_SETTING_DATA+1] = ubValue;
	}
	else if(ubItemIdx == CAMERA_SATURATION)
	{
		if(tUI_CamStatus[tCamNum].tCamColorParam.ubColorSaturation + tUI_CuSetting.bColorSaturationGain[tCamNum] < 0)
			return;
		ubValue = UI_Map(tUI_CamStatus[tCamNum].tCamColorParam.ubColorSaturation + tUI_CuSetting.bColorSaturationGain[tCamNum],0,99,0,127);
		tCamSetCmd.ubCmd[UI_SETTING_DATA]	 = UI_IMGSATURATION_SETTING ;
		tCamSetCmd.ubCmd[UI_SETTING_DATA+1] = ubValue;
	}
	else if(ubItemIdx == CAMERA_HUE)
	{
		if(tUI_CamStatus[tCamNum].tCamColorParam.ubColorHue + tUI_CuSetting.bColorHueGain[tCamNum] > 99)
			return;
		ubValue = UI_Map(tUI_CamStatus[tCamNum].tCamColorParam.ubColorHue + tUI_CuSetting.bColorHueGain[tCamNum],0,99,0,127);
		tCamSetCmd.ubCmd[UI_SETTING_DATA]	 = UI_IMGHUE_SETTING ;
		tCamSetCmd.ubCmd[UI_SETTING_DATA+1] = ubValue;
	}
	else if(ubItemIdx == CAMERA_H_MIRROR)
	{
		tCamSetCmd.ubCmd[UI_SETTING_DATA]	= UI_IMGMIRROR_SETTING;
		tCamSetCmd.ubCmd[UI_SETTING_DATA+1] = tUI_CamStatus[tCamNum].tCamImgMirror;
	}
	else if(ubItemIdx == CAMERA_V_MIRROR)
	{
		tCamSetCmd.ubCmd[UI_SETTING_DATA]	= UI_IMGFLIP_SETTING;
		tCamSetCmd.ubCmd[UI_SETTING_DATA+1] = tUI_CamStatus[tCamNum].tCamImgFlip;
	}
	else if(ubItemIdx == CAMERA_LASER)  //Alan
	{
		tCamSetCmd.ubCmd[UI_SETTING_DATA+1] = tUI_CamStatus[tCamNum].tCamLaser;
	}
	else if(ubItemIdx == CAMERA_LED)
	{
		tCamSetCmd.ubCmd[UI_SETTING_DATA+1] = tUI_CamStatus[tCamNum].tCamLed;
	}

	if(ubItemIdx != CAMERA_VOLUME)
	{
		if(ubItemIdx != CAMERA_LASER && ubItemIdx != CAMERA_LED)
		{
			if(UI_SendRequestToCAM(osThreadGetId(), &tCamSetCmd) != rUI_SUCCESS)
			{
				printf("Camera Setting Fail !\n");
				return;
			}
		}
		else if(ubItemIdx == CAMERA_LASER)
		{
			// if(ubKNL_TwcSend((TWC_TAG)tCamNum,TWC_Laser_CTRL,&tCamSetCmd.ubCmd[UI_SETTING_DATA+1],1,10) != rUI_SUCCESS)
			if(UI_SendLaserorLedToCAM(osThreadGetId(), &tCamSetCmd, TWC_Laser_CTRL) != rUI_SUCCESS)
			{
				printf("CAMERA_LASER Setting Fail!!! !\n");
				return;
			}
		}
		else
		{
			// if(ubKNL_TwcSend((TWC_TAG)tCamNum,TWC_Led_CTRL,&tCamSetCmd.ubCmd[UI_SETTING_DATA+1],1,10) != rUI_SUCCESS)
			if(UI_SendLaserorLedToCAM(osThreadGetId(), &tCamSetCmd, TWC_Led_CTRL) != rUI_SUCCESS)
			{
				printf("CAMERA_LED Setting Fail!!! !\n");
				return;
			}
		}
	}
	else
	{
		uint16_t uwValue = (uint16_t)UI_Map(tUI_CamStatus[tCamNum].ubVolumeLvl,0,99,VOLUME_PWM_MIN,VOLUME_PWM_MAX);
		SET_VOLUME_PWM((uwValue == VOLUME_PWM_MIN)?0:uwValue);
		//osDelay(50);
		if(uwValue == VOLUME_PWM_MIN)
		{
			SET_VOLUME_IO = 0;
		}
		else
		{
			SET_VOLUME_IO = 1;
		}
	}
}

//----------------------------------------------------------------------
static void UI_CameraDrawSubSubSubMenuItem()
{
	uint16_t uwSubSubMenuItemIdx = 
		(uint16_t)tSettingSubSubMenuItem.tSettingS[CAMERASET_ITEM].tSubMenuInfo.ubItemIdx;
	uint8_t ubItemPreIdx = tCameraSubSubSubMenuItem[uwSubSubMenuItemIdx]->tSubMenuInfo.ubItemPreIdx;
	uint8_t ubItemCurIdx = tCameraSubSubSubMenuItem[uwSubSubMenuItemIdx]->tSubMenuInfo.ubItemIdx;
	uint8_t *pUI_ColorParm[5] = {
								(uint8_t *)&tUI_CamStatus[uwSubSubMenuItemIdx].tCamColorParam.ubColorBL,
								(uint8_t *)&tUI_CamStatus[uwSubSubMenuItemIdx].tCamColorParam.ubColorContrast,
								(uint8_t *)&tUI_CamStatus[uwSubSubMenuItemIdx].tCamColorParam.ubColorSaturation,
								(uint8_t *)&tUI_CamStatus[uwSubSubMenuItemIdx].tCamColorParam.ubColorHue,
								(uint8_t *)&tUI_CamStatus[uwSubSubMenuItemIdx].ubVolumeLvl
								 };
	OSD_IMG_INFO tOsdImgInfo[16];
	//-----------------------------------------preIdx------------------------------------------
	if(ubItemPreIdx != ubItemCurIdx)//只有当控件出现跳转才需要把前一个控件恢复为normal状态
	{
		if(ubItemPreIdx <= CAMERA_VOLUME)
		{
			//先擦除选中框
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_CAMS_BRIGHTNESS_GREEN_HL, 1, &tOsdImgInfo[0]);
			tOsdImgInfo[0].uwYStart = CAMERA_HL_Y + CAMERA_Y_STEP*ubItemPreIdx;
			OSD_EraserImg2_NoUpdate(&tOsdImgInfo[0]);
			//word
			tOSD_GetOsdImgInfor(1, OSD_IMG2, ((tUI_CuSetting.tLanguage == LANGUAGE_ENGLISH)?OSD2IMG_CAMS_BRIGHTNESS_WORD:
											(tUI_CuSetting.tLanguage == LANGUAGE_GERMAN)?OSD2IMG_CAMS_BRIGHTNESS_WORD_GER:
											(tUI_CuSetting.tLanguage == LANGUAGE_FRENCH)?OSD2IMG_CAMS_BRIGHTNESS_WORD_FR:OSD2IMG_CAMS_BRIGHTNESS_WORD_CHN) + ubItemPreIdx, 1, &tOsdImgInfo[0]);
			tOSD_Img2(&tOsdImgInfo[0],OSD_QUEUE);
			//slider
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_CAMS_BRIGHTNESS_SLIDER, 1, &tOsdImgInfo[0]);
			tOsdImgInfo[0].uwYStart = CAMERA_SLIDER_Y + CAMERA_Y_STEP*ubItemPreIdx;
			tOSD_Img2(&tOsdImgInfo[0],OSD_QUEUE);
			//round
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_CAMS_ROUND_HL, 1, &tOsdImgInfo[1]);
			tOsdImgInfo[1].uwXStart = UI_Map(*pUI_ColorParm[ubItemPreIdx],0,99,
											tOsdImgInfo[0].uwXStart,tOsdImgInfo[0].uwXStart + tOsdImgInfo[0].uwHSize - tOsdImgInfo[1].uwHSize);
			tOsdImgInfo[1].uwYStart = tOsdImgInfo[0].uwYStart;
			tOSD_Img2(&tOsdImgInfo[1], OSD_QUEUE);
			//数值
			if(*pUI_ColorParm[ubItemPreIdx] < 10)
			{
				UI_ShowButtonValueHighLight(0,tOsdImgInfo[1].uwXStart + 4,tOsdImgInfo[1].uwYStart + 9,OSD_QUEUE);
				UI_ShowButtonValueHighLight(*pUI_ColorParm[ubItemPreIdx],tOsdImgInfo[1].uwXStart + 24,tOsdImgInfo[1].uwYStart + 9,OSD_UPDATE);
			}
			else
				UI_ShowButtonValueHighLight(*pUI_ColorParm[ubItemPreIdx],tOsdImgInfo[1].uwXStart + 4,tOsdImgInfo[1].uwYStart + 9,OSD_UPDATE);
				
		}
		else
		{
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_CAMS_H_MIRROR_OFF_NOR, 16, &tOsdImgInfo[0]);
			if(ubItemPreIdx == CAMERA_H_MIRROR)
			{
				if(tUI_CamStatus[uwSubSubMenuItemIdx].tCamImgMirror)
					tOSD_Img2(&tOsdImgInfo[2], OSD_UPDATE);
				else
					tOSD_Img2(&tOsdImgInfo[0], OSD_UPDATE);
			}
			else if(ubItemPreIdx == CAMERA_V_MIRROR)
			{
				if(tUI_CamStatus[uwSubSubMenuItemIdx].tCamImgFlip)
					tOSD_Img2(&tOsdImgInfo[4 + 2], OSD_UPDATE);
				else
					tOSD_Img2(&tOsdImgInfo[4 + 0], OSD_UPDATE);

			}
			else if(ubItemPreIdx == CAMERA_LASER)
			{
				if(tUI_CamStatus[uwSubSubMenuItemIdx].tCamLaser && (UI_CheckTxVersion(tUI_CamStatus[uwSubSubMenuItemIdx].cCamVersion,30,Laserchar,3)))
					tOSD_Img2(&tOsdImgInfo[8 + 2], OSD_UPDATE);
				else
					tOSD_Img2(&tOsdImgInfo[8 + 0], OSD_UPDATE);
			}
			else
			{
				if(tUI_CamStatus[uwSubSubMenuItemIdx].tCamLed && (UI_CheckTxVersion(tUI_CamStatus[uwSubSubMenuItemIdx].cCamVersion,30,Laserchar,3)))
					tOSD_Img2(&tOsdImgInfo[12 + 2], OSD_UPDATE);
				else
					tOSD_Img2(&tOsdImgInfo[12 + 0], OSD_UPDATE);
			}
		}
	}
	//--------------------------------CurIdx----------------------------------------------------
	if(ubItemCurIdx <= CAMERA_VOLUME)
	{
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_CAMS_BRIGHTNESS_GREEN_HL + CameraSubSubSubRemoteMode, 1, &tOsdImgInfo[0]);
		tOsdImgInfo[0].uwYStart = CAMERA_HL_Y + CAMERA_Y_STEP*ubItemCurIdx;
		tOSD_Img2(&tOsdImgInfo[0],OSD_QUEUE);
		//word
		tOSD_GetOsdImgInfor(1, OSD_IMG2, ((tUI_CuSetting.tLanguage == LANGUAGE_ENGLISH)?OSD2IMG_CAMS_BRIGHTNESS_WORD:
										(tUI_CuSetting.tLanguage == LANGUAGE_GERMAN)?OSD2IMG_CAMS_BRIGHTNESS_WORD_GER:
										(tUI_CuSetting.tLanguage == LANGUAGE_FRENCH)?OSD2IMG_CAMS_BRIGHTNESS_WORD_FR:OSD2IMG_CAMS_BRIGHTNESS_WORD_CHN) + ubItemCurIdx, 1, &tOsdImgInfo[0]);
		tOSD_Img2(&tOsdImgInfo[0],OSD_QUEUE);
		//slider
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_CAMS_BRIGHTNESS_SLIDER, 1, &tOsdImgInfo[0]);
		tOsdImgInfo[0].uwYStart = CAMERA_SLIDER_Y + CAMERA_Y_STEP*ubItemCurIdx;
		tOSD_Img2(&tOsdImgInfo[0],OSD_QUEUE);
		//round
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_CAMS_ROUND_HL, 1, &tOsdImgInfo[1]);
		tOsdImgInfo[1].uwXStart = UI_Map(*pUI_ColorParm[ubItemCurIdx],0,99,
										tOsdImgInfo[0].uwXStart,tOsdImgInfo[0].uwXStart + tOsdImgInfo[0].uwHSize - tOsdImgInfo[1].uwHSize);
		tOsdImgInfo[1].uwYStart = tOsdImgInfo[0].uwYStart;
		tOSD_Img2(&tOsdImgInfo[1], OSD_QUEUE);
		//数值
		if(*pUI_ColorParm[ubItemCurIdx] < 10)
		{
			UI_ShowButtonValueHighLight(0,tOsdImgInfo[1].uwXStart + 4,tOsdImgInfo[1].uwYStart + 9,OSD_QUEUE);
			UI_ShowButtonValueHighLight(*pUI_ColorParm[ubItemCurIdx],tOsdImgInfo[1].uwXStart + 24,tOsdImgInfo[1].uwYStart + 9,OSD_UPDATE);
		}
		else
			UI_ShowButtonValueHighLight(*pUI_ColorParm[ubItemCurIdx],tOsdImgInfo[1].uwXStart + 4,tOsdImgInfo[1].uwYStart + 9,OSD_UPDATE);
	}
	else
	{
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_CAMS_H_MIRROR_OFF_NOR, 16, &tOsdImgInfo[0]);
		if(ubItemCurIdx == CAMERA_H_MIRROR)
		{
			if(tUI_CamStatus[uwSubSubMenuItemIdx].tCamImgMirror)
				tOSD_Img2(&tOsdImgInfo[3], OSD_UPDATE);
			else
				tOSD_Img2(&tOsdImgInfo[1], OSD_UPDATE);
		}
		else if(ubItemCurIdx == CAMERA_V_MIRROR)
		{
			if(tUI_CamStatus[uwSubSubMenuItemIdx].tCamImgFlip)
				tOSD_Img2(&tOsdImgInfo[4 + 3], OSD_UPDATE);
			else
				tOSD_Img2(&tOsdImgInfo[4 + 1], OSD_UPDATE);

		}
		else if(ubItemCurIdx == CAMERA_LASER)
		{
			if(tUI_CamStatus[uwSubSubMenuItemIdx].tCamLaser && (UI_CheckTxVersion(tUI_CamStatus[uwSubSubMenuItemIdx].cCamVersion,30,Laserchar,3)))
				tOSD_Img2(&tOsdImgInfo[8 + 3], OSD_UPDATE);
			else
				tOSD_Img2(&tOsdImgInfo[8 + 1], OSD_UPDATE);
		}
		else
		{
			if(tUI_CamStatus[uwSubSubMenuItemIdx].tCamLed && (UI_CheckTxVersion(tUI_CamStatus[uwSubSubMenuItemIdx].cCamVersion,30,Laserchar,3)))
				tOSD_Img2(&tOsdImgInfo[12 + 3], OSD_UPDATE);
			else
				tOSD_Img2(&tOsdImgInfo[12 + 1], OSD_UPDATE);
		}

	}

}
//----------------------------------------------------------------------
void UI_CameraSubSubSubMenuExitToSubSub(uint16_t uwSubSubMenuItemIdx)
{
	OSD_IMG_INFO tOsdImgInfo;

	tOSD_GetOsdImgInfor(1, OSD_IMG2, ((tUI_CuSetting.tLanguage != LANGUAGE_GERMAN)?OSD2IMG_CAMSCAM1_SELECTED_HL_ICON:OSD2IMG_CAMSCAM1_SELECTED_HL_ICON_GER) + 2*uwSubSubMenuItemIdx, 1, &tOsdImgInfo);
	tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
	tUI_State = UI_SUBSUBMENU_STATE;
	tCameraSubSubSubMenuItem[uwSubSubMenuItemIdx]->tSubMenuInfo.ubItemIdx = CAMERA_BRIGHTNESS;
	for(uint8_t i = 0;i < 4;i++)
		UI_UpdateTxDevStatusInfo((UI_CamNum_t)i);
}
//----------------------------------------------------------------------
void UI_CameraSubSubSubMenuExitToSub()
{
	OSD_IMG_INFO tOsdImgInfo;
	UI_CamNum_t tCamNum = tSettingSubSubMenuItem.tSettingS[CAMERASET_ITEM].tSubMenuInfo.ubItemIdx;
	tOSD_GetOsdImgInfor(1, OSD_IMG1, OSD1IMG_SUBMENU2, 1, &tOsdImgInfo);
	tOSD_Img1(&tOsdImgInfo, OSD_QUEUE);
	if(!ubUI_TouchPanelSts)
	{
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_RETURN_BLACK, 1, &tOsdImgInfo);
		tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
	}
	OSD_IMG_INFO tOsdImgInfosub[22];
	tOSD_GetOsdImgInfor(1, OSD_IMG2, (tUI_CuSetting.tLanguage == LANGUAGE_ENGLISH)?OSD2IMG_CAMERA_SETTING_WORD:
									(tUI_CuSetting.tLanguage == LANGUAGE_GERMAN)?OSD2IMG_CAMERA_SETTING_WORD_GER:
									(tUI_CuSetting.tLanguage == LANGUAGE_FRENCH)?OSD2IMG_CAMERA_SETTING_WORD_FR:OSD2IMG_CAMERA_SETTING_WORD_CHN, SETTINGITEM_MAX-1, &tOsdImgInfosub[0]);
	//WORD
	for(uint8_t i = 0;i < SETTINGITEM_MAX-1;i++)
		tOSD_Img2(&tOsdImgInfosub[i], OSD_QUEUE);
	tOSD_GetOsdImgInfor(1, OSD_IMG2, (tUI_CuSetting.tLanguage == LANGUAGE_ENGLISH)?OSD2IMG_AI_WORD:
									(tUI_CuSetting.tLanguage == LANGUAGE_GERMAN)?OSD2IMG_AI_WORD:
									(tUI_CuSetting.tLanguage == LANGUAGE_FRENCH)?OSD2IMG_AI_WORD:OSD2IMG_AI_WORD_CHN,1, &tOsdImgInfo);
	tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);

	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_CAMERA_SETTING_NOR_ICON, SETTINGITEM_MAX*2, &tOsdImgInfosub[0]);

	for(uint16_t i = 0;i < SETTINGITEM_MAX;i++)
		tOSD_Img2(&tOsdImgInfosub[2*i], OSD_QUEUE);
	tOSD_Img2(&tOsdImgInfosub[2*CAMERASET_ITEM + UI_ICON_HIGHLIGHT], OSD_UPDATE);//highlight
	
	memset(&tSettingSubSubMenuItem.tSettingS[CAMERASET_ITEM].tSubMenuInfo, 0, sizeof(UI_MenuItem_t));
	ubUI_SubSubStsUpdateFlag = FALSE;
	for(uint8_t i = 0;i < 4;i++)
		tCameraSubSubSubMenuItem[i]->tSubMenuInfo.ubItemIdx = CAMERA_BRIGHTNESS;
	tUI_State = UI_SUBMENU_STATE;
	
	UI_UpdateDevStatusInfo();
	UI_UpdateTxDevStatusInfo(tCamNum);
			
	UI_SwitchViewType(DeskTopShowView,FALSE);
}
//----------------------------------------------------------------------
void UI_CameraSubSubSubMenuPage(UI_ArrowKey_t tArrowKey)
{
	uint16_t uwSubSubMenuItemIdx = 
		(uint16_t)tSettingSubSubMenuItem.tSettingS[CAMERASET_ITEM].tSubMenuInfo.ubItemIdx;
	
	uint8_t *pUI_ColorParm[5] = {
								(uint8_t *)&tUI_CamStatus[uwSubSubMenuItemIdx].tCamColorParam.ubColorBL,
								(uint8_t *)&tUI_CamStatus[uwSubSubMenuItemIdx].tCamColorParam.ubColorContrast,
								(uint8_t *)&tUI_CamStatus[uwSubSubMenuItemIdx].tCamColorParam.ubColorSaturation,
								(uint8_t *)&tUI_CamStatus[uwSubSubMenuItemIdx].tCamColorParam.ubColorHue,
								(uint8_t *)&tUI_CamStatus[uwSubSubMenuItemIdx].ubVolumeLvl
								 };

	tCameraSubSubSubMenuItem[uwSubSubMenuItemIdx]->tSubMenuInfo.ubItemPreIdx = 
		tCameraSubSubSubMenuItem[uwSubSubMenuItemIdx]->tSubMenuInfo.ubItemIdx;

	
	if(tUI_CamStatus[uwSubSubMenuItemIdx].tCamConnSts == CAM_ONLINE || tArrowKey == EXIT_ARROW)
	{
		//当前进度条的高亮颜色为绿色，遥控的左右键是切换不同的选项
		if(CameraSubSubSubRemoteMode == REMOTE_SELECT_ITEM)
		{
				switch(tArrowKey)
				{
					case LEFT_ARROW:
						if(tCameraSubSubSubMenuItem[uwSubSubMenuItemIdx]->tSubMenuInfo.ubItemIdx ==
							tCameraSubSubSubMenuItem[uwSubSubMenuItemIdx]->ubFirstItem)
						{
							tCameraSubSubSubMenuItem[uwSubSubMenuItemIdx]->tSubMenuInfo.ubItemIdx = 
								tCameraSubSubSubMenuItem[uwSubSubMenuItemIdx]->ubItemCount - 1;
						}
						else
							tCameraSubSubSubMenuItem[uwSubSubMenuItemIdx]->tSubMenuInfo.ubItemIdx --;
						break;
					case RIGHT_ARROW:
						if(tCameraSubSubSubMenuItem[uwSubSubMenuItemIdx]->tSubMenuInfo.ubItemIdx == 
							tCameraSubSubSubMenuItem[uwSubSubMenuItemIdx]->ubItemCount - 1)
						{
							tCameraSubSubSubMenuItem[uwSubSubMenuItemIdx]->tSubMenuInfo.ubItemIdx = 
								tCameraSubSubSubMenuItem[uwSubSubMenuItemIdx]->ubFirstItem;
						}
						else
							tCameraSubSubSubMenuItem[uwSubSubMenuItemIdx]->tSubMenuInfo.ubItemIdx ++;
						break;
					case ENTER_ARROW:
						{	
							uint8_t ubItemCurIdx = tCameraSubSubSubMenuItem[uwSubSubMenuItemIdx]->tSubMenuInfo.ubItemIdx;
							if(tCameraSubSubSubMenuItem[uwSubSubMenuItemIdx]->tSubMenuInfo.ubItemIdx <= CAMERA_VOLUME)//GREEN->RED
							{
								SendIrCodeFlag = 1;//支持连发
								CameraSubSubSubRemoteMode = 1- CameraSubSubSubRemoteMode;
								UI_CameraDrawSubSubSubMenuItem();
							}
							else//mirror flip
							{
								if(ubItemCurIdx == CAMERA_H_MIRROR)
									tUI_CamStatus[uwSubSubMenuItemIdx].tCamImgMirror = 1- tUI_CamStatus[uwSubSubMenuItemIdx].tCamImgMirror;
								else if(ubItemCurIdx == CAMERA_V_MIRROR)
									tUI_CamStatus[uwSubSubMenuItemIdx].tCamImgFlip = 1- tUI_CamStatus[uwSubSubMenuItemIdx].tCamImgFlip;
								else if(ubItemCurIdx == CAMERA_LASER)
								{
									if(UI_CheckTxVersion(tUI_CamStatus[uwSubSubMenuItemIdx].cCamVersion,30,Laserchar,3))
										tUI_CamStatus[uwSubSubMenuItemIdx].tCamLaser = 1- tUI_CamStatus[uwSubSubMenuItemIdx].tCamLaser;
								}
								else
								{
									if(UI_CheckTxVersion(tUI_CamStatus[uwSubSubMenuItemIdx].cCamVersion,30,Laserchar,3))
										tUI_CamStatus[uwSubSubMenuItemIdx].tCamLed = 1- tUI_CamStatus[uwSubSubMenuItemIdx].tCamLed;
								}
								UI_CameraDrawSubSubSubMenuItem();
								UI_CameraSubSubSubMenuExecute();

							}
							
						}
						return;
					case EXIT_ARROW:
						UI_CameraSubSubSubMenuExitToSubSub(uwSubSubMenuItemIdx);
						return;
					default:
						return;
				}
			
			
			UI_CameraDrawSubSubSubMenuItem();
		}//进度条的高亮变为红色，左右键可以调节进度条的数值
		else
		{	
			uint8_t ubItemCurIdx = tCameraSubSubSubMenuItem[uwSubSubMenuItemIdx]->tSubMenuInfo.ubItemIdx;
			switch(tArrowKey)
			{
				case LEFT_ARROW:
					if(*pUI_ColorParm[ubItemCurIdx] > 0)
						(*pUI_ColorParm[ubItemCurIdx]) --;
					else
						return;
					break;
				case RIGHT_ARROW:
					if(*pUI_ColorParm[ubItemCurIdx] < 99)
						(*pUI_ColorParm[ubItemCurIdx]) ++;
					else
						return;
					break;
				case ENTER_ARROW:
					SendIrCodeFlag = 0;//禁止连发
					CameraSubSubSubRemoteMode = 1 - CameraSubSubSubRemoteMode;
					UI_CameraDrawSubSubSubMenuItem();
					return;
				case EXIT_ARROW:
					UI_CameraSubSubSubMenuExitToSubSub(uwSubSubMenuItemIdx);
					return;
				default:
					return;
			}
			UI_CameraDrawSubSubSubMenuItem();
			UI_CameraSubSubSubMenuExecute();
		}
	}
}

//------------------------------------------------------------------------------------system------------------------------------------------------------------------------

static void UI_SystemSubSubSubMenuExit(void)
{
	OSD_IMG_INFO tMenuOsdImgInfo,tOsdImgInfo[10];
	uint8_t ubSubSubMenuItem = tSystemSubSubMenuItem.tSubMenuInfo.ubItemIdx;

//	UI_UART2_PutChar(0XFF);
//	UI_UART2_PutChar(0XDD);	
	
	tOSD_GetOsdImgInfor(1, OSD_IMG1, OSD1IMG_SUBMENU2, 1, &tMenuOsdImgInfo);
	tOSD_Img1(&tMenuOsdImgInfo, OSD_QUEUE);
	
	tOSD_GetOsdImgInfor(1, OSD_IMG2, (tUI_CuSetting.tLanguage == LANGUAGE_ENGLISH)?OSD2IMG_SYSTEM_TITLE:
									(tUI_CuSetting.tLanguage == LANGUAGE_GERMAN)?OSD2IMG_SYSTEM_TITLE_GER:
									(tUI_CuSetting.tLanguage == LANGUAGE_FRENCH)?OSD2IMG_SYSTEM_TITLE_FR:OSD2IMG_SYSTEM_TITLE_CHN, 1, &tOsdImgInfo[0]);
	//文字
	for(uint8_t i = 0;i < 1;i ++)
		tOSD_Img2(&tOsdImgInfo[i], OSD_QUEUE);
									
	if(!ubUI_TouchPanelSts)
	{
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_RETURN_BLACK, 1, &tMenuOsdImgInfo);
		tOSD_Img2(&tMenuOsdImgInfo, OSD_QUEUE);
	}
	
	//滑块滑杆
	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_SYSTEM_DIMMER_SLIDER, 6, &tOsdImgInfo[0]);
	
	//滑杆
	tOSD_Img2(&tOsdImgInfo[0], OSD_QUEUE);
	//滑块
	
	tOsdImgInfo[2].uwXStart = UI_Map(tUI_CuSetting.BriLvL.ulBL_UpdateLvL,0,99,
		tOsdImgInfo[0].uwXStart,tOsdImgInfo[0].uwXStart + tOsdImgInfo[0].uwHSize - tOsdImgInfo[2].uwHSize);
	tOSD_Img2(&tOsdImgInfo[2], OSD_QUEUE);
	//数值
	if(tUI_CuSetting.BriLvL.ulBL_UpdateLvL < 10)
	{
		UI_ShowButtonValueHighLight(0,tOsdImgInfo[2].uwXStart + 4,tOsdImgInfo[2].uwYStart + 9,OSD_QUEUE);
		UI_ShowButtonValueHighLight(tUI_CuSetting.BriLvL.ulBL_UpdateLvL,tOsdImgInfo[2].uwXStart + 24,tOsdImgInfo[2].uwYStart + 9,OSD_QUEUE);
	}
	else
		UI_ShowButtonValueHighLight(tUI_CuSetting.BriLvL.ulBL_UpdateLvL,tOsdImgInfo[2].uwXStart + 4,tOsdImgInfo[2].uwYStart + 9,OSD_QUEUE);
			
	//auto dimmer
	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_SYSTEM_AUTO_OFF_NOR, 4, &tOsdImgInfo[0]);
	if(tUI_CuSetting.ubAutoDimmer)
		tOSD_Img2(&tOsdImgInfo[2], OSD_QUEUE);
	else
		tOSD_Img2(&tOsdImgInfo[0], OSD_QUEUE);
	
	if(ubSubSubMenuItem == AUTO_SET_ITEM)
		tOSD_GetOsdImgInfor(1, OSD_IMG2,OSD2IMG_SYSTEM_AUTO_SETTING_HL, 1, &tOsdImgInfo[0]);
	else
		tOSD_GetOsdImgInfor(1, OSD_IMG2,OSD2IMG_SYSTEM_AUTO_SETTING_NOR, 1, &tOsdImgInfo[0]);
	tOSD_Img2(&tOsdImgInfo[0], OSD_QUEUE);

	
	//menu lock
	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_SYSTEM_MENULOCK_OFF_NOR, 4, &tOsdImgInfo[0]);
	if(tUI_CuSetting.ubMenuLock)
		tOSD_Img2(&tOsdImgInfo[2], OSD_QUEUE);
	else
		tOSD_Img2(&tOsdImgInfo[0], OSD_QUEUE);

	//upgrade
	if(ubSubSubMenuItem == UPGRADE_ITEM)
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_SYSTEM_UPGRADE_HL, 1, &tOsdImgInfo[0]);
	else
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_SYSTEM_UPGRADE_NOR, 1, &tOsdImgInfo[0]);
    tOSD_Img2(&tOsdImgInfo[0], OSD_QUEUE);

	//buzzer
	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_SYSTEM_BUZZER_OFF_NOR, 4, &tOsdImgInfo[0]);
	if(tUI_CuSetting.ubBuzzer)
		tOSD_Img2(&tOsdImgInfo[2], OSD_QUEUE);
	else
		tOSD_Img2(&tOsdImgInfo[0], OSD_QUEUE);
	
	//language
	if(ubSubSubMenuItem == LANGUAGE_ITEM)
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_SYSTEM_LANGUAGE_HL, 1, &tOsdImgInfo[0]);
	else
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_SYSTEM_LANGUAGE_NOR, 1, &tOsdImgInfo[0]);
    tOSD_Img2(&tOsdImgInfo[0], OSD_QUEUE);

	// reset
	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_SYSTEM_RESET_NOR, 1, &tOsdImgInfo[0]);
	tOSD_Img2(&tOsdImgInfo[0], OSD_QUEUE);

	//SD Switch
	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_SYSTEM_SD_SWITCH_OFF_NOR, 4, &tOsdImgInfo[0]);
	if(tUI_CuSetting.ubSD_Switch)
		tOSD_Img2(&tOsdImgInfo[2], OSD_QUEUE);
	else
		tOSD_Img2(&tOsdImgInfo[0], OSD_QUEUE);

	//hide channel name
	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_SYSTEM_HIDE_CHANNEL_NAME_OFF_NOR, 4, &tOsdImgInfo[0]);
	if(tUI_CuSetting.ubHideChannelName)
		tOSD_Img2(&tOsdImgInfo[2], OSD_QUEUE);
	else
		tOSD_Img2(&tOsdImgInfo[0], OSD_QUEUE);

	// date & time
	if(ubSubSubMenuItem == DATETIME_ITEM)
	{
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_SYSTEM_DATE_TIME_HL, 1, &tOsdImgInfo[0]);
	}
	else
	{
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_SYSTEM_DATE_TIME_NOR, 1, &tOsdImgInfo[0]);
	}
	tOSD_Img2(&tOsdImgInfo[0], OSD_QUEUE);

	OSD_ImagePrintf(OSD_IMG_ROTATION_0, 450, 543, tUI_CharOsdImgInfo, OSD_QUEUE, systemAIVersion);
	OSD_ImagePrintf(OSD_IMG_ROTATION_0, 450, 570, tUI_CharOsdImgInfo, OSD_UPDATE, SOFEWARE_VERSION);

	tUI_State = UI_SUBSUBMENU_STATE;

}

//------------------------------------------------------------------------------------
static void UI_DrawDateTimeYearButtonItem(bool isButtonHL, uint8_t DateTimeYearButtonIndex)
{
	OSD_IMG_INFO tOsdImgInfo[2];

	uint16_t uwTemp[6];
	uwTemp[0] = tUI_CuSetting.tSysCalendar.uwYear;
	uwTemp[1] = tUI_CuSetting.tSysCalendar.ubMonth;
	uwTemp[2] = tUI_CuSetting.tSysCalendar.ubDate;
	uwTemp[3] = tUI_CuSetting.tSysCalendar.ubHour;
	uwTemp[4] = tUI_CuSetting.tSysCalendar.ubMin;
	uwTemp[5] = tUI_CuSetting.tSysCalendar.ubSec; 

//	printf("UI_DrawDateTimeYearButtonItem isButtonHL = %d DateTimeYearButtonIndex = %d\n", isButtonHL, DateTimeYearButtonIndex);
	
	//BUTTON
	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_DATE_TIME_YEAR_NOR, 2, &tOsdImgInfo[0]);
	if(isButtonHL)
	{
		//HIGHLIGHT BUTTON
		tOsdImgInfo[1].uwXStart +=  (DateTimeYearButtonIndex % 3) * DATE_TIME_YEAR_DIST_X;
		if(DateTimeYearButtonIndex > 2)
		{
			tOsdImgInfo[1].uwYStart += DATE_TIME_YEAR_DIST_Y;
		}
		tOSD_Img2(&tOsdImgInfo[1], OSD_QUEUE);

//		printf("tOsdImgInfo[1].uwXStart = %d, uwYStart = %d\n", tOsdImgInfo[1].uwXStart, tOsdImgInfo[1].uwYStart);

		if(uwTemp[DateTimeYearButtonIndex] < 10)
		{
			UI_ShowButtonValueHighLight(0,tOsdImgInfo[1].uwXStart + 30,tOsdImgInfo[1].uwYStart + 15,OSD_QUEUE);
			UI_ShowButtonValueHighLight(uwTemp[DateTimeYearButtonIndex],tOsdImgInfo[1].uwXStart + 50,tOsdImgInfo[1].uwYStart + 15,OSD_UPDATE);
		}
		else if(uwTemp[DateTimeYearButtonIndex] < 100)
			UI_ShowButtonValueHighLight(uwTemp[DateTimeYearButtonIndex],tOsdImgInfo[1].uwXStart + 30,tOsdImgInfo[1].uwYStart + 15,OSD_UPDATE);
		else
			UI_ShowButtonValueHighLight(tUI_CuSetting.tSysCalendar.uwYear,tOsdImgInfo[1].uwXStart + 10,tOsdImgInfo[1].uwYStart + 15,OSD_UPDATE);
	}
	else
	{
		tOsdImgInfo[0].uwXStart +=  (DateTimeYearButtonIndex % 3) * DATE_TIME_YEAR_DIST_X;
		if(DateTimeYearButtonIndex > 2)
		{
			tOsdImgInfo[0].uwYStart += DATE_TIME_YEAR_DIST_Y;
		}
		tOSD_Img2(&tOsdImgInfo[0], OSD_QUEUE);

//		printf("tOsdImgInfo[1].uwXStart = %d, uwYStart = %d\n", tOsdImgInfo[0].uwXStart, tOsdImgInfo[0].uwYStart);
		if(uwTemp[DateTimeYearButtonIndex] < 10)
		{
			UI_ShowButtonValueNormal(0,tOsdImgInfo[0].uwXStart + 30,tOsdImgInfo[0].uwYStart + 15,OSD_QUEUE);
			UI_ShowButtonValueNormal(uwTemp[DateTimeYearButtonIndex],tOsdImgInfo[0].uwXStart + 50,tOsdImgInfo[0].uwYStart + 15,OSD_QUEUE);
		}
		else if(uwTemp[DateTimeYearButtonIndex] < 100)
			UI_ShowButtonValueNormal(uwTemp[DateTimeYearButtonIndex],tOsdImgInfo[0].uwXStart + 30,tOsdImgInfo[0].uwYStart + 15,OSD_QUEUE);
		else
			UI_ShowButtonValueNormal(tUI_CuSetting.tSysCalendar.uwYear,tOsdImgInfo[0].uwXStart + 10,tOsdImgInfo[0].uwYStart + 15,OSD_QUEUE);
	}
}


//------------------------------------------------------------------------------------
static void UI_SystemDrawSubSubSubMenuItem(uint8_t ubDrawBox)
{
	uint8_t ubSubSubMenuItem = tSystemSubSubMenuItem.tSubMenuInfo.ubItemIdx;
	uint8_t ubItemPreIdx = tSystemSubSubSubMenuItem[ubSubSubMenuItem].tSubMenuInfo.ubItemPreIdx;
	uint8_t ubItemCurIdx = tSystemSubSubSubMenuItem[ubSubSubMenuItem].tSubMenuInfo.ubItemIdx;
	OSD_IMG_INFO tOsdImgInfo[8];
	uint16_t uwSettingItemOsdImg[16] = {0};
	switch(ubSubSubMenuItem)
	{
		case AUTO_SET_ITEM:
			uint8_t *pUI_TempVal[2] = {
										&tUI_CuSetting.ubAutoDimmer_MaxVal,
										&tUI_CuSetting.ubAutoDimmer_MinVal
										};
			//-----------------------------------------preIdx------------------------------------------
			if(ubItemPreIdx != ubItemCurIdx)//只有当控件出现跳转才需要把前一个控件恢复为normal状态
			{
				//先擦除选中框
				tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_SYSTEM_DAY_GREEN_HL + 2*ubItemPreIdx, 1, &tOsdImgInfo[0]);
				OSD_EraserImg2_NoUpdate(&tOsdImgInfo[0]);
			
				//显示滑杆
				tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_SYSTEM_DAY_SLIDER + ubItemPreIdx, 1, &tOsdImgInfo[0]);
				tOSD_Img2(&tOsdImgInfo[0],OSD_QUEUE);
				//显示滑块
				tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_SYSTEM_DAY_ROUND_HL + ubItemPreIdx, 1, &tOsdImgInfo[1]);
				
				tOsdImgInfo[1].uwXStart = UI_Map(*pUI_TempVal[ubItemPreIdx],0,99,
					tOsdImgInfo[0].uwXStart,tOsdImgInfo[0].uwXStart + tOsdImgInfo[0].uwHSize - tOsdImgInfo[1].uwHSize);	
				tOSD_Img2(&tOsdImgInfo[1], OSD_QUEUE);
				//数值
				if(*pUI_TempVal[ubItemPreIdx] < 10)
				{
					UI_ShowButtonValueHighLight(0,tOsdImgInfo[1].uwXStart + 4,tOsdImgInfo[1].uwYStart + 9,OSD_QUEUE);
					UI_ShowButtonValueHighLight(*pUI_TempVal[ubItemPreIdx],tOsdImgInfo[1].uwXStart + 24,tOsdImgInfo[1].uwYStart + 9,OSD_QUEUE);
				}
				else
					UI_ShowButtonValueHighLight(*pUI_TempVal[ubItemPreIdx],tOsdImgInfo[1].uwXStart + 4,tOsdImgInfo[1].uwYStart + 9,OSD_UPDATE);
						
			
			}
			//--------------------------------CurIdx----------------------------------------------------
			//显示框
			if(ubDrawBox)
			{
				tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_SYSTEM_DAY_GREEN_HL + 2*ubItemCurIdx + SystemSubSubSubRemoteMode, 1, &tOsdImgInfo[0]);
				tOSD_Img2(&tOsdImgInfo[0],OSD_QUEUE);
			}
			//slider
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_SYSTEM_DAY_SLIDER + ubItemCurIdx, 1, &tOsdImgInfo[0]);
			tOSD_Img2(&tOsdImgInfo[0],OSD_QUEUE);
			//round
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_SYSTEM_DAY_ROUND_HL + ubItemCurIdx, 1, &tOsdImgInfo[1]);
			tOsdImgInfo[1].uwXStart = UI_Map(*pUI_TempVal[ubItemCurIdx],0,99,
											tOsdImgInfo[0].uwXStart,tOsdImgInfo[0].uwXStart + tOsdImgInfo[0].uwHSize - tOsdImgInfo[1].uwHSize);
			tOSD_Img2(&tOsdImgInfo[1], OSD_QUEUE);
			//数值
			if(*pUI_TempVal[ubItemCurIdx] < 10)
			{
				UI_ShowButtonValueHighLight(0,tOsdImgInfo[1].uwXStart + 4,tOsdImgInfo[1].uwYStart + 9,OSD_QUEUE);
				UI_ShowButtonValueHighLight(*pUI_TempVal[ubItemCurIdx],tOsdImgInfo[1].uwXStart + 24,tOsdImgInfo[1].uwYStart + 9,OSD_UPDATE);
			}
			else
				UI_ShowButtonValueHighLight(*pUI_TempVal[ubItemCurIdx],tOsdImgInfo[1].uwXStart + 4,tOsdImgInfo[1].uwYStart + 9,OSD_UPDATE);
			break;
			
		case UPGRADE_ITEM:
			for(uint8_t i = 0;i < 2;i ++)
			{
				uwSettingItemOsdImg[i] = ((tUI_CuSetting.tLanguage == LANGUAGE_ENGLISH)?OSD2IMG_SDCARD_YES_NOR:
										(tUI_CuSetting.tLanguage == LANGUAGE_GERMAN)?OSD2IMG_SDCARD_YES_NOR_GER:
										(tUI_CuSetting.tLanguage == LANGUAGE_FRENCH)?OSD2IMG_SDCARD_YES_NOR_FR:OSD2IMG_SDCARD_YES_NOR_CHN) + 2*i;
			}
			UI_DrawHLandNormalIcon(uwSettingItemOsdImg[ubItemPreIdx],(uwSettingItemOsdImg[ubItemCurIdx] + UI_ICON_HIGHLIGHT));
			break;	
			
		case LANGUAGE_ITEM:
			for(uint8_t i = 0;i < LANGUAGE_MAX;i ++)
			{
				uwSettingItemOsdImg[i] = OSD2IMG_SYSTEM_ENGLISH_NOR + 2*i;
				if(tUI_CuSetting.tLanguage == i)
					uwSettingItemOsdImg[i] = OSD2IMG_SYSTEM_ENGLISH_SELECT_NOR + 2*i;
		
			}
			UI_DrawHLandNormalIcon(uwSettingItemOsdImg[ubItemPreIdx],(uwSettingItemOsdImg[ubItemCurIdx] + UI_ICON_HIGHLIGHT));
			break;
		case DATETIME_ITEM:
			//pre
			if(ubItemPreIdx <= DATE_TIME_SECOND)
			{
				UI_DrawDateTimeYearButtonItem(FALSE, ubItemPreIdx);
			}
			else if(ubItemPreIdx == DATE_TIME_UTC_SELECT)
			{
				tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_TIME_ZONE_SLIDER, 6, &tOsdImgInfo[0]);
				if(SystemSubSubSubRemoteMode == REMOTE_SELECT_ITEM)
				{
	                if (ubItemCurIdx != DATE_TIME_UTC_SELECT)
	                {
						OSD_EraserImg2_NoUpdate(&tOsdImgInfo[4]);
					}
				}
				else
				{
					if (ubItemCurIdx != DATE_TIME_UTC_SELECT)
	                {
						OSD_EraserImg2_NoUpdate(&tOsdImgInfo[4]);
					}
					else
					{
						tOSD_Img2(&tOsdImgInfo[5], OSD_QUEUE);
					}
				}

				tOSD_Img2(&tOsdImgInfo[0], OSD_QUEUE);

				tOsdImgInfo[2].uwXStart = UI_Map(tUI_CuSetting.ubtimezone,0,26,
					tOsdImgInfo[0].uwXStart,tOsdImgInfo[0].uwXStart + tOsdImgInfo[0].uwHSize - tOsdImgInfo[2].uwHSize);
				tOSD_Img2(&tOsdImgInfo[2], OSD_UPDATE);
				UI_ShowUTCVal();

			}
			else if(ubItemPreIdx == DATE_TIME_UTC_COMFIRM)
			{
				tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_DATE_TIME_UTC_COMFIRM_NOR, 2, &tOsdImgInfo[0]);
				tOSD_Img2(&tOsdImgInfo[0], OSD_UPDATE);
			}

			//cur
			if(ubItemCurIdx <= DATE_TIME_SECOND)
			{
				UI_DrawDateTimeYearButtonItem(TRUE, ubItemCurIdx);
			}
			else if(ubItemCurIdx == DATE_TIME_UTC_SELECT)
			{
				tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_TIME_ZONE_SLIDER, 6, &tOsdImgInfo[0]);
				if (SystemSubSubSubRemoteMode == REMOTE_SELECT_ITEM)
					tOSD_Img2(&tOsdImgInfo[4], OSD_QUEUE);
				else
					tOSD_Img2(&tOsdImgInfo[5], OSD_QUEUE);
				tOSD_Img2(&tOsdImgInfo[0], OSD_QUEUE);

				tOsdImgInfo[2].uwXStart = UI_Map(tUI_CuSetting.ubtimezone,0,26,
					tOsdImgInfo[0].uwXStart,tOsdImgInfo[0].uwXStart + tOsdImgInfo[0].uwHSize - tOsdImgInfo[2].uwHSize);
				tOSD_Img2(&tOsdImgInfo[2], OSD_UPDATE);
				UI_ShowUTCVal();
			}
			else if(ubItemCurIdx == DATE_TIME_UTC_COMFIRM)
			{
				tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_DATE_TIME_UTC_COMFIRM_NOR, 2, &tOsdImgInfo[0]);
				tOSD_Img2(&tOsdImgInfo[1], OSD_UPDATE);

			}
			break;
		default:
			break;
	}
	

}
//------------------------------------------------------------------------------------
void UI_SystemSubSubSubMenuPage(UI_ArrowKey_t tArrowKey)
{
	
	uint8_t ubSubSubMenuItem = tSystemSubSubMenuItem.tSubMenuInfo.ubItemIdx;
	tSystemSubSubSubMenuItem[ubSubSubMenuItem].tSubMenuInfo.ubItemPreIdx = 
		tSystemSubSubSubMenuItem[ubSubSubMenuItem].tSubMenuInfo.ubItemIdx;

	//当前进度条的高亮颜色为绿色，遥控的左右键是切换不同的选项
	if(SystemSubSubSubRemoteMode == REMOTE_SELECT_ITEM)
	{
		switch(tArrowKey)
		{
			case LEFT_ARROW:
				if(tSystemSubSubSubMenuItem[ubSubSubMenuItem].tSubMenuInfo.ubItemIdx ==
					tSystemSubSubSubMenuItem[ubSubSubMenuItem].ubFirstItem)
				{
					tSystemSubSubSubMenuItem[ubSubSubMenuItem].tSubMenuInfo.ubItemIdx = 
						tSystemSubSubSubMenuItem[ubSubSubMenuItem].ubItemCount - 1;
				}
				else
					tSystemSubSubSubMenuItem[ubSubSubMenuItem].tSubMenuInfo.ubItemIdx --;
				break;
			case RIGHT_ARROW:
				if(tSystemSubSubSubMenuItem[ubSubSubMenuItem].tSubMenuInfo.ubItemIdx ==
					tSystemSubSubSubMenuItem[ubSubSubMenuItem].ubItemCount - 1)
				{
					tSystemSubSubSubMenuItem[ubSubSubMenuItem].tSubMenuInfo.ubItemIdx = 
						tSystemSubSubSubMenuItem[ubSubSubMenuItem].ubFirstItem;
				}
				else
					tSystemSubSubSubMenuItem[ubSubSubMenuItem].tSubMenuInfo.ubItemIdx ++;
				break;
			case ENTER_ARROW:
				if(ubSubSubMenuItem == AUTO_SET_ITEM)
				{
					SendIrCodeFlag = 1;//支持连发
					SystemSubSubSubRemoteMode = 1- SystemSubSubSubRemoteMode;
					UI_SystemDrawSubSubSubMenuItem(TRUE);
					return;
				}
				else if(ubSubSubMenuItem == LANGUAGE_ITEM)
				{
					if(tUI_CuSetting.tLanguage != (UI_Language_t)tSystemSubSubSubMenuItem[ubSubSubMenuItem].tSubMenuInfo.ubItemIdx)
					{
						tUI_CuSetting.tLanguage = (UI_Language_t)tSystemSubSubSubMenuItem[ubSubSubMenuItem].tSubMenuInfo.ubItemIdx;
							//根据语言修改时区
						if(tUI_CuSetting.tLanguage == LANGUAGE_ENGLISH)//西六
							tUI_CuSetting.ubtimezone = 6;
						else if(tUI_CuSetting.tLanguage == LANGUAGE_GERMAN)//东一
							tUI_CuSetting.ubtimezone = 14;
						else if(tUI_CuSetting.tLanguage == LANGUAGE_FRENCH)//东一
							tUI_CuSetting.ubtimezone = 14;
						else if(tUI_CuSetting.tLanguage == LANGUAGE_CHINESE)//东八
							tUI_CuSetting.ubtimezone = 21;
						
						UI_UpdateDevStatusInfo();
						SYS_Reboot();
					}
					else
					{
						UI_SystemDrawSubSubSubMenuItem(TRUE);
						osDelay(100);
					}

				}
				else if(ubSubSubMenuItem == UPGRADE_ITEM)
				{
					if(tSystemSubSubSubMenuItem[ubSubSubMenuItem].tSubMenuInfo.ubItemIdx == SYSTEM_UPGRADE_YES)
					{
						UI_DisconnectTx();
						osDelay(200);
						UI_ClearOsdImage();
						MenuOnFlag = FALSE;
						SendIrCodeFlag = FALSE;
						tUI_State = UI_DISPLAY_STATE;

						UI_Event_t tSdFwUpgEvent;
						osMessageQId *pUI_EventQH = NULL;

						tSdFwUpgEvent.tEventType = FWUPG_EVENT;
						tSdFwUpgEvent.pvEvent  	 = NULL;
						pUI_EventQH 	  	 	 = pUI_GetEventQueueHandle();
						osMessagePut(*pUI_EventQH, &tSdFwUpgEvent, 0);
						
						return;
					}
				}
				else if (ubSubSubMenuItem == DATETIME_ITEM)
				{
					uint8_t uwsubsubsubItemIdx = tSystemSubSubSubMenuItem[ubSubSubMenuItem].tSubMenuInfo.ubItemIdx;
					OSD_IMG_INFO tOsdImgInfo[25],tMenuOsdImgInfo;
					tOSD_GetOsdImgInfor(1, OSD_IMG1, OSD1IMG_SUBMENU2, 1, &tMenuOsdImgInfo);

			        if(uwsubsubsubItemIdx != DATE_TIME_UTC_SELECT)
			        {
						tOSD_Img1(&tMenuOsdImgInfo, OSD_QUEUE);
					}

					if(!ubUI_TouchPanelSts)
					{
						tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_RETURN_BLACK, 1, &tMenuOsdImgInfo);
						tOSD_Img2(&tMenuOsdImgInfo, OSD_QUEUE);
					}
					tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_KEYBOARD_BACKGROUND, 25, &tOsdImgInfo[0]);
					
					if(uwsubsubsubItemIdx < DATE_TIME_UTC_SELECT)
					{
						tOSD_Img2(&tOsdImgInfo[0], OSD_QUEUE);
						for(uint8_t i = 0;i < 12;i++)
							tOSD_Img2(&tOsdImgInfo[1 + 2*i], OSD_QUEUE);
						tOSD_Img2(&tOsdImgInfo[2], OSD_QUEUE);
					}

					if(uwsubsubsubItemIdx == DATE_TIME_YEAR)
					{
						UI_ShowKeyBoardValue(tUI_CuSetting.tSysCalendar.uwYear,OSD_UPDATE);
					}
					else if((uwsubsubsubItemIdx == DATE_TIME_MONTH))
					{
						UI_ShowKeyBoardValue(tUI_CuSetting.tSysCalendar.ubMonth,OSD_UPDATE);
					}
					else if((uwsubsubsubItemIdx == DATE_TIME_DAY))
					{
						UI_ShowKeyBoardValue(tUI_CuSetting.tSysCalendar.ubDate,OSD_UPDATE);
					}
					else if((uwsubsubsubItemIdx == DATE_TIME_HOUR))
					{
						UI_ShowKeyBoardValue(tUI_CuSetting.tSysCalendar.ubHour,OSD_UPDATE);
					}
					else if((uwsubsubsubItemIdx == DATE_TIME_MINUTE))
					{
						UI_ShowKeyBoardValue(tUI_CuSetting.tSysCalendar.ubMin,OSD_UPDATE);	
					}
					else if((uwsubsubsubItemIdx == DATE_TIME_SECOND))
					{
						UI_ShowKeyBoardValue(tUI_CuSetting.tSysCalendar.ubSec,OSD_UPDATE);
					}
					else if((uwsubsubsubItemIdx == DATE_TIME_UTC_SELECT))
					{
						tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_TIME_ZONE_SLIDER, 6, &tOsdImgInfo[0]);
			            //SettingSubSubSubRemoteMode = 1 - SettingSubSubSubRemoteMode;
						SystemSubSubSubRemoteMode = 1- SystemSubSubSubRemoteMode;
			            if(SystemSubSubSubRemoteMode == REMOTE_SELECT_ITEM)
			            	tOSD_Img2(&tOsdImgInfo[4], OSD_QUEUE);
						else
							tOSD_Img2(&tOsdImgInfo[5], OSD_QUEUE);
						tOSD_Img2(&tOsdImgInfo[0], OSD_QUEUE);
						tOsdImgInfo[2].uwXStart = UI_Map(tUI_CuSetting.ubtimezone,0,26,
							tOsdImgInfo[0].uwXStart,tOsdImgInfo[0].uwXStart + tOsdImgInfo[0].uwHSize - tOsdImgInfo[2].uwHSize);
						tOSD_Img2(&tOsdImgInfo[2], OSD_UPDATE);
						
						return;
					}
					else if(uwsubsubsubItemIdx == DATE_TIME_UTC_COMFIRM)
					{
			            tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_DATE_TIME_UTC_COMFIRM_NOR, 2, &tOsdImgInfo[0]);
				   	    tOSD_Img2(&tOsdImgInfo[1], OSD_UPDATE);
			            UI_UpdateDevStatusInfo();
						SYS_Reboot();
						return;
					}
					ubUI_DateTimeSubSubSubSubStsUpdateFlag = FALSE;
					tUI_State = UI_SUBSUBSUBSUBMENU_STATE;
					return;
				}
			case EXIT_ARROW:
				UI_SystemSubSubSubMenuExit();
				return;
			default:
				return;
		}
		
		UI_SystemDrawSubSubSubMenuItem(TRUE);
	}//进度条的高亮变为红色，左右键可以调节进度条的数值
	else
	{	
		uint8_t ubItemCurIdx = tSystemSubSubSubMenuItem[ubSubSubMenuItem].tSubMenuInfo.ubItemIdx;
		switch(tArrowKey)
		{
			case LEFT_ARROW:
				if (tSystemSubSubMenuItem.tSubMenuInfo.ubItemIdx == AUTO_SET_ITEM)	
				{
					if(ubItemCurIdx == DAY_SLIDER)
					{
						if(tUI_CuSetting.ubAutoDimmer_MaxVal > 0 && tUI_CuSetting.ubAutoDimmer_MaxVal > tUI_CuSetting.ubAutoDimmer_MinVal)
							tUI_CuSetting.ubAutoDimmer_MaxVal --;
						else 
							return;
					}
					else if (ubItemCurIdx == NIGHT_SLIDER)
					{
						if(tUI_CuSetting.ubAutoDimmer_MinVal > 0)
							tUI_CuSetting.ubAutoDimmer_MinVal --;
						else 
							return;
					}
				}
				else if (tSystemSubSubMenuItem.tSubMenuInfo.ubItemIdx == DATETIME_ITEM)
				{
					if(ubItemCurIdx == DATE_TIME_UTC_SELECT)
					{
						if (tUI_CuSetting.ubtimezone > 0)
						{
							tUI_CuSetting.ubtimezone--;
						}
						else
						{
							tUI_CuSetting.ubtimezone = 26;
						}
					}
				}
				break;
			case RIGHT_ARROW:
				if (tSystemSubSubMenuItem.tSubMenuInfo.ubItemIdx == AUTO_SET_ITEM)	
				{
					if(ubItemCurIdx == DAY_SLIDER)
					{
						if(tUI_CuSetting.ubAutoDimmer_MaxVal < 99)
							tUI_CuSetting.ubAutoDimmer_MaxVal ++;
						else 
							return;
					}
					else if (ubItemCurIdx == NIGHT_SLIDER)
					{
						if(tUI_CuSetting.ubAutoDimmer_MinVal < 99 && tUI_CuSetting.ubAutoDimmer_MinVal < tUI_CuSetting.ubAutoDimmer_MaxVal)
							tUI_CuSetting.ubAutoDimmer_MinVal ++;
						else 
							return;
					}
				}
				else if (tSystemSubSubMenuItem.tSubMenuInfo.ubItemIdx == DATETIME_ITEM)
				{
					if(ubItemCurIdx == DATE_TIME_UTC_SELECT)
					{
						if (tUI_CuSetting.ubtimezone < 26)
						{
							tUI_CuSetting.ubtimezone++;
						}
						else
						{
							tUI_CuSetting.ubtimezone = 0;
						}
					}
				}
				break;
			case ENTER_ARROW:
				SendIrCodeFlag = 0;//禁止连发
				SystemSubSubSubRemoteMode = 1 - SystemSubSubSubRemoteMode;
				UI_SystemDrawSubSubSubMenuItem(TRUE);
				return;
			case EXIT_ARROW:
				UI_SystemSubSubSubMenuExit();
				return;
			default:
				return;
		}
		UI_SystemDrawSubSubSubMenuItem(FALSE);
	}


}



//------------------------------------------------------------------------------------parkingLine------------------------------------------------------------------------------
#define LEFT_RIGHT_MODE 0
#define UP_DOWN_MODE    1
#define SIZE_MODE       2

uint8_t ParkingLineRemoteMode = LEFT_RIGHT_MODE;


void UI_ParkingLineSubSubSubMenuPageExit()
{
	OSD_IMG_INFO tOsdImgInfo[24],tMenuOsdImgInfo;
	uint16_t uwSubSubMenuItemIdx = 
			(uint16_t)tSettingSubSubMenuItem.tSettingS[PARKINGLINE_ITEM].tSubMenuInfo.ubItemIdx;//sub sub

	tOSD_GetOsdImgInfor(1, OSD_IMG1, OSD1IMG_SUBMENU2, 1, &tMenuOsdImgInfo);
	tOSD_Img1(&tMenuOsdImgInfo, OSD_QUEUE);
	if(!ubUI_TouchPanelSts)
	{
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_RETURN_BLACK, 1, &tMenuOsdImgInfo);
		tOSD_Img2(&tMenuOsdImgInfo, OSD_QUEUE);
	}
	//WORD
	tOSD_GetOsdImgInfor(1, OSD_IMG2, (tUI_CuSetting.tLanguage != LANGUAGE_GERMAN)?OSD2IMG_PARKINGLINE_TITLE:OSD2IMG_PARKINGLINE_TITLE_GER, 5, &tOsdImgInfo[0]);
	if(tUI_CuSetting.tLanguage == LANGUAGE_FRENCH)
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_PARKINGLINE_TITLE_FR, 1, &tOsdImgInfo[0]);
	else if(tUI_CuSetting.tLanguage == LANGUAGE_CHINESE)
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_PARKINGLINE_TITLE_CHN, 1, &tOsdImgInfo[0]);
	
	for(uint8_t i = 0;i < 5;i++)
		tOSD_Img2(&tOsdImgInfo[i], OSD_QUEUE);
	
	//enablr or disable
	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_AUTOSCAN_OFF_CAM1_NOR, 4, &tOsdImgInfo[0]);
	for(uint8_t i = 0;i < 4;i++)
	{
		for(uint8_t j = 0;j < 4;j++)
		{
			tOsdImgInfo[6*i + j] = tOsdImgInfo[j];
			tOsdImgInfo[6*i + j].uwXStart = PARKINGLINE_SWITCH_X + PARKINGLINE_X_STEP*(i%2);
			tOsdImgInfo[6*i + j].uwYStart = PARKINGLINE_SWITCH_Y + PARKINGLINE_Y_STEP*(i/2);
		}
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_PARKINGLINE_CAM1_AUTO_NOR + 2*i, 2, &tOsdImgInfo[6*i + 4]);
	}
	for(uint8_t i = 0;i < 4;i++)
		tOSD_Img2(&tOsdImgInfo[6*i + 2*tUI_CuSetting.tParkingLineEnable[i]], OSD_QUEUE); 
	//CALIBRATE
	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_PARKINGLINE_CAM1_CAL_NOR, 8, &tOsdImgInfo[0]);
	for(uint8_t i = 0;i < 4;i++)
		tOSD_Img2(&tOsdImgInfo[2*i], OSD_QUEUE);
	//high light
	tOSD_Img2(&tOsdImgInfo[2*(uwSubSubMenuItemIdx/2) + UI_ICON_HIGHLIGHT], OSD_UPDATE);
	ParkingLineRemoteMode = LEFT_RIGHT_MODE;
	tUI_State = UI_SUBSUBMENU_STATE;

}

//-----------------------------------------------------------------------------------
void UI_ParkingLineDrawSubSubSubMenuItem()
{
	OSD_IMG_INFO tParkingLineOsdImgInfo,tOsdImgInfo;
	uint16_t uwHSize = 0,uwVSize = 0;
	uint16_t uwImgIdx;
	uint16_t uwSubSubMenuItemIdx = 
	(uint16_t)tSettingSubSubMenuItem.tSettingS[PARKINGLINE_ITEM].tSubMenuInfo.ubItemIdx;
	uint16_t CamIdx = uwSubSubMenuItemIdx/2;	
//	tOSD_GetOsdImgInfor(1, OSD_IMG1, OSD1IMG_SUBMENU2, 1, &tOsdImgInfo);
//	OSD_EraserImg2(&tOsdImgInfo);
	//防止改变大小时越界
	tOSD_GetOsdImgInfor(1, OSD_IMG1, OSD1IMG_PARKINGLINE_UPPER1 + tUI_CuSetting.ParkingLineSize[CamIdx], 1, &tParkingLineOsdImgInfo);
	uwHSize += tParkingLineOsdImgInfo.uwHSize;
	tOSD_GetOsdImgInfor(1, OSD_IMG1, OSD1IMG_PARKINGLINE_LEFT, 1, &tParkingLineOsdImgInfo);
	uwHSize += 2*tParkingLineOsdImgInfo.uwHSize;
	uwVSize = tParkingLineOsdImgInfo.uwVSize;
	
	
	if((tUI_CuSetting.ParkingLine_XY[CamIdx][0] << 8) + tUI_CuSetting.ParkingLine_XY[CamIdx][1] + uwHSize > OSD_WIDTH)
	{
		tUI_CuSetting.ParkingLine_XY[CamIdx][0] = (OSD_WIDTH - uwHSize) >> 8;
		tUI_CuSetting.ParkingLine_XY[CamIdx][1] = (uint8_t)(OSD_WIDTH - uwHSize);
	}
	UI_DrawParkingLine(CamIdx,OSD_QUEUE);	//显示返回图标
	if(ubUI_TouchPanelSts)
	{
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_RETURN_NOBACKGROUND_NOR, 1, &tOsdImgInfo);	
		tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
	}
	//显示遥控操作状态
	if(ParkingLineRemoteMode == LEFT_RIGHT_MODE)
		uwImgIdx = OSD2IMG_PARKINGLINE_L_R_FLAG;
	else if(ParkingLineRemoteMode == UP_DOWN_MODE)
		uwImgIdx = OSD2IMG_PARKINGLINE_U_D_FLAG;
	else 
		uwImgIdx = OSD2IMG_PARKINGLINE_SIZE_FLAG;
	
	tOSD_GetOsdImgInfor(1, OSD_IMG2,uwImgIdx , 1, &tOsdImgInfo);
	tOsdImgInfo.uwXStart = (tUI_CuSetting.ParkingLine_XY[CamIdx][0] << 8) + tUI_CuSetting.ParkingLine_XY[CamIdx][1] + (uwHSize - tOsdImgInfo.uwHSize)/2;
	tOsdImgInfo.uwYStart = (tUI_CuSetting.ParkingLine_XY[CamIdx][2] << 8) + tUI_CuSetting.ParkingLine_XY[CamIdx][3] + (uwVSize - tOsdImgInfo.uwVSize)/2;
	tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);


	//OSD_UpdateImg1();


}
//-----------------------------------------------------------------------------------

void UI_ParkingLineSubSubSubMenuPage(UI_ArrowKey_t tArrowKey)
{
	OSD_IMG_INFO tParkingLineOsdImgInfo,tOsdImgInfo;
	uint16_t uwSubSubMenuItemIdx = 
		(uint16_t)tSettingSubSubMenuItem.tSettingS[PARKINGLINE_ITEM].tSubMenuInfo.ubItemIdx;
	uint16_t CamIdx = uwSubSubMenuItemIdx/2;	
	uint16_t uwTempVal;
	uint16_t uwHSize = 0,uwVSize = 0; 
	//鎿﹂櫎涔嬪墠鐨勫�掕溅鍏夋爣
	UI_EraseParkingLine(CamIdx,OSD_QUEUE);
	
	//鑾峰彇鍊掕溅鍏夋爣鐨勫ぇ灏?
	tOSD_GetOsdImgInfor(1, OSD_IMG1, OSD1IMG_PARKINGLINE_LEFT, 1, &tParkingLineOsdImgInfo);
	uwHSize += 2*tParkingLineOsdImgInfo.uwHSize;
	uwVSize = tParkingLineOsdImgInfo.uwVSize;
	tOSD_GetOsdImgInfor(1, OSD_IMG1, OSD1IMG_PARKINGLINE_UPPER1 + tUI_CuSetting.ParkingLineSize[CamIdx], 1, &tParkingLineOsdImgInfo);
	uwHSize += tParkingLineOsdImgInfo.uwHSize;

	//鎿﹂櫎鎿嶄綔鐘舵�?
	tOSD_GetOsdImgInfor(1, OSD_IMG2,OSD2IMG_PARKINGLINE_L_R_FLAG, 1, &tOsdImgInfo);
	tOsdImgInfo.uwXStart = (tUI_CuSetting.ParkingLine_XY[CamIdx][0] << 8) + tUI_CuSetting.ParkingLine_XY[CamIdx][1] + (uwHSize - tOsdImgInfo.uwHSize)/2;
	tOsdImgInfo.uwYStart = (tUI_CuSetting.ParkingLine_XY[CamIdx][2] << 8) + tUI_CuSetting.ParkingLine_XY[CamIdx][3] + (uwVSize - tOsdImgInfo.uwVSize)/2;
	OSD_EraserImg2_NoUpdate(&tOsdImgInfo);
	
	switch(tArrowKey)
	{
		case LEFT_ARROW:
			//宸﹀彸
			if(ParkingLineRemoteMode == LEFT_RIGHT_MODE)
			{
				if(((tUI_CuSetting.ParkingLine_XY[CamIdx][0] << 8) + tUI_CuSetting.ParkingLine_XY[CamIdx][1]) >= 5)
				{
					uwTempVal = (tUI_CuSetting.ParkingLine_XY[CamIdx][0] << 8) + tUI_CuSetting.ParkingLine_XY[CamIdx][1];
					uwTempVal -= 5;
					tUI_CuSetting.ParkingLine_XY[CamIdx][0] = uwTempVal >> 8;
					tUI_CuSetting.ParkingLine_XY[CamIdx][1] = (uint8_t)uwTempVal;
				}
				else
				{
					tUI_CuSetting.ParkingLine_XY[CamIdx][0] = 0;
					tUI_CuSetting.ParkingLine_XY[CamIdx][1] = 0;
				}
			}//涓婁笅
			else if(ParkingLineRemoteMode == UP_DOWN_MODE)
			{
				if(((tUI_CuSetting.ParkingLine_XY[CamIdx][2] << 8) + tUI_CuSetting.ParkingLine_XY[CamIdx][3]) >= 5)
				{
					uwTempVal = (tUI_CuSetting.ParkingLine_XY[CamIdx][2] << 8) + tUI_CuSetting.ParkingLine_XY[CamIdx][3];
					uwTempVal -= 5;
					tUI_CuSetting.ParkingLine_XY[CamIdx][2] = uwTempVal >> 8;
					tUI_CuSetting.ParkingLine_XY[CamIdx][3] = (uint8_t)uwTempVal;

				}
				else
				{
					tUI_CuSetting.ParkingLine_XY[CamIdx][2] = 0;
					tUI_CuSetting.ParkingLine_XY[CamIdx][3] = 0;

				}
			}//澶у皬
			else
			{
				if(tUI_CuSetting.ParkingLineSize[CamIdx] > 0)
					tUI_CuSetting.ParkingLineSize[CamIdx]--;
			}
			break;
		case RIGHT_ARROW:
			//宸﹀彸
			if(ParkingLineRemoteMode == LEFT_RIGHT_MODE)
			{
				if(((tUI_CuSetting.ParkingLine_XY[CamIdx][0] << 8) + tUI_CuSetting.ParkingLine_XY[CamIdx][1])
					<= (OSD_WIDTH - uwHSize - 5))
				{
					uwTempVal = (tUI_CuSetting.ParkingLine_XY[CamIdx][0] << 8) + tUI_CuSetting.ParkingLine_XY[CamIdx][1];
					uwTempVal += 5;
					tUI_CuSetting.ParkingLine_XY[CamIdx][0] = uwTempVal >> 8;
					tUI_CuSetting.ParkingLine_XY[CamIdx][1] = (uint8_t)uwTempVal;

				}
				else
				{
					tUI_CuSetting.ParkingLine_XY[CamIdx][0] = (OSD_WIDTH - uwHSize) >> 8;
					tUI_CuSetting.ParkingLine_XY[CamIdx][1] = (uint8_t)(OSD_WIDTH - uwHSize);

				}

			}//涓婁笅
			else if(ParkingLineRemoteMode == UP_DOWN_MODE)
			{
				if(((tUI_CuSetting.ParkingLine_XY[CamIdx][2] << 8) + tUI_CuSetting.ParkingLine_XY[CamIdx][3]) 
					<= (OSD_HEIGHT - uwVSize - 5))
				{
					uwTempVal = (tUI_CuSetting.ParkingLine_XY[CamIdx][2] << 8) + tUI_CuSetting.ParkingLine_XY[CamIdx][3];
					uwTempVal += 5;
					tUI_CuSetting.ParkingLine_XY[CamIdx][2] = uwTempVal >> 8;
					tUI_CuSetting.ParkingLine_XY[CamIdx][3] = (uint8_t)uwTempVal;

				}
				else
				{
					tUI_CuSetting.ParkingLine_XY[CamIdx][2] = (OSD_HEIGHT - uwVSize) >> 8;
					tUI_CuSetting.ParkingLine_XY[CamIdx][3] = (uint8_t)(OSD_HEIGHT - uwVSize);

				}

			}//澶у皬
			else
			{
				if(tUI_CuSetting.ParkingLineSize[CamIdx] < 11)
					tUI_CuSetting.ParkingLineSize[CamIdx] ++;
			}
			break;
		case ENTER_ARROW:
			if(ParkingLineRemoteMode == LEFT_RIGHT_MODE)
				ParkingLineRemoteMode = UP_DOWN_MODE;
			else if(ParkingLineRemoteMode == UP_DOWN_MODE)
				ParkingLineRemoteMode = SIZE_MODE;
			else
				ParkingLineRemoteMode = LEFT_RIGHT_MODE;
			break;
		case EXIT_ARROW:
			UI_ParkingLineSubSubSubMenuPageExit();
			SendIrCodeFlag = 0;
			return;
		default:
			return;
	}
	
	UI_ParkingLineDrawSubSubSubMenuItem();


		
}
//------------------------------------------------------------------------------------GuideLine--------------------------------------------------------------------------
//-----------------------------------------------------------------------------------
void UI_GuideLineSubSubSubMenuPageExit(void)
{
	OSD_IMG_INFO tOsdImgInfo[24],tMenuOsdImgInfo;
	uint16_t uwSubSubMenuItemIdx = 
			(uint16_t)tSettingSubSubMenuItem.tSettingS[GUIDELINE_ITEM].tSubMenuInfo.ubItemIdx;//sub sub

	tOSD_GetOsdImgInfor(1, OSD_IMG1, OSD1IMG_SUBMENU2, 1, &tMenuOsdImgInfo);
	tOSD_Img1(&tMenuOsdImgInfo, OSD_QUEUE);
	if(!ubUI_TouchPanelSts)
	{
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_RETURN_BLACK, 1, &tMenuOsdImgInfo);
		tOSD_Img2(&tMenuOsdImgInfo, OSD_QUEUE);
	}
	tOSD_GetOsdImgInfor(1, OSD_IMG2, (tUI_CuSetting.tLanguage == LANGUAGE_ENGLISH)?OSD2IMG_GUIDELINE_TITLE:
									(tUI_CuSetting.tLanguage == LANGUAGE_GERMAN)?OSD2IMG_GUIDELINE_TITLE_GER:
									(tUI_CuSetting.tLanguage == LANGUAGE_FRENCH)?OSD2IMG_GUIDELINE_TITLE_FR:OSD2IMG_GUIDELINE_TITLE_CHN, 1, &tOsdImgInfo[0]);
	tOSD_Img2(&tOsdImgInfo[0], OSD_QUEUE);
	
	//WORD
	tOSD_GetOsdImgInfor(1, OSD_IMG2, (tUI_CuSetting.tLanguage != LANGUAGE_GERMAN)?OSD2IMG_PARKINGLINE_CAM1_WD:OSD2IMG_PARKINGLINE_CAM1_WD_GER, 4, &tOsdImgInfo[0]);
	for(uint8_t i = 0;i < 4;i++)
		tOSD_Img2(&tOsdImgInfo[i], OSD_QUEUE);

	
	//enablr or disable
	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_AUTOSCAN_OFF_CAM1_NOR, 4, &tOsdImgInfo[0]);
	for(uint8_t i = 0;i < 4;i++)
	{
		for(uint8_t j = 0;j < 4;j++)
		{
			tOsdImgInfo[6*i + j] = tOsdImgInfo[j];
			tOsdImgInfo[6*i + j].uwXStart = PARKINGLINE_SWITCH_X + PARKINGLINE_X_STEP*(i%2);
			tOsdImgInfo[6*i + j].uwYStart = PARKINGLINE_SWITCH_Y + PARKINGLINE_Y_STEP*(i/2);
		}
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_PARKINGLINE_CAM1_AUTO_NOR + 2*i, 2, &tOsdImgInfo[6*i + 4]);
	}
	for(uint8_t i = 0;i < 4;i++)
		tOSD_Img2(&tOsdImgInfo[6*i + 2*tUI_CuSetting.GuideLineEnable[i]], OSD_QUEUE); 
	//CALIBRATE
	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_PARKINGLINE_CAM1_CAL_NOR, 8, &tOsdImgInfo[0]);
	for(uint8_t i = 0;i < 4;i++)
		tOSD_Img2(&tOsdImgInfo[2*i], OSD_QUEUE);
	//high light
	tOSD_Img2(&tOsdImgInfo[2*(uwSubSubMenuItemIdx/2) + UI_ICON_HIGHLIGHT], OSD_UPDATE);
	ParkingLineRemoteMode = LEFT_RIGHT_MODE;
	tUI_State = UI_SUBSUBMENU_STATE;

}


//-----------------------------------------------------------------------------------
void UI_GuideLineDrawSubSubSubMenuItem(void)
{
	OSD_IMG_INFO tOsdImgInfo[2];
	uint16_t uwImgIdx;
	uint16_t uwSubSubMenuItemIdx = 
		(uint16_t)tSettingSubSubMenuItem.tSettingS[GUIDELINE_ITEM].tSubMenuInfo.ubItemIdx;
	uint16_t CamIdx = uwSubSubMenuItemIdx/2;	
	//显示新的倒车光标
	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_GUIDELINE_SIZE1 + tUI_CuSetting.GuideLineSize[CamIdx], 1, &tOsdImgInfo[0]);
	//防止改变大小时越界
	if((tUI_CuSetting.GuideLine_XY[CamIdx][0] << 8) + tUI_CuSetting.GuideLine_XY[CamIdx][1] + tOsdImgInfo[0].uwHSize > OSD_WIDTH)
	{
		tUI_CuSetting.GuideLine_XY[CamIdx][0] = (OSD_WIDTH - tOsdImgInfo[0].uwHSize) >> 8;
		tUI_CuSetting.GuideLine_XY[CamIdx][1] = (uint8_t)(OSD_WIDTH - tOsdImgInfo[0].uwHSize);
	}
		
	tOsdImgInfo[0].uwXStart = (tUI_CuSetting.GuideLine_XY[CamIdx][0] << 8) + tUI_CuSetting.GuideLine_XY[CamIdx][1];
	tOsdImgInfo[0].uwYStart = (tUI_CuSetting.GuideLine_XY[CamIdx][2] << 8) + tUI_CuSetting.GuideLine_XY[CamIdx][3];
	tOSD_Img2(&tOsdImgInfo[0], OSD_QUEUE);
	
	//显示遥控操作状态
	if(ParkingLineRemoteMode == LEFT_RIGHT_MODE)
		uwImgIdx = OSD2IMG_PARKINGLINE_L_R_FLAG;
	else if(ParkingLineRemoteMode == UP_DOWN_MODE)
		uwImgIdx = OSD2IMG_PARKINGLINE_U_D_FLAG;
	else 
		uwImgIdx = OSD2IMG_PARKINGLINE_SIZE_FLAG;
	
	tOSD_GetOsdImgInfor(1, OSD_IMG2,uwImgIdx, 1, &tOsdImgInfo[1]);
	tOsdImgInfo[1].uwXStart = tOsdImgInfo[0].uwXStart + (tOsdImgInfo[0].uwHSize - tOsdImgInfo[1].uwHSize)/2;
	if(tOsdImgInfo[0].uwYStart > tOsdImgInfo[1].uwVSize + 20)
		tOsdImgInfo[1].uwYStart = tOsdImgInfo[0].uwYStart - tOsdImgInfo[1].uwVSize - 20;
	else
		tOsdImgInfo[1].uwYStart = tOsdImgInfo[0].uwYStart + tOsdImgInfo[0].uwVSize + 20;
	tOSD_Img2(&tOsdImgInfo[1], ubUI_TouchPanelSts?OSD_QUEUE:OSD_UPDATE);
	//显示返回图标
	if(ubUI_TouchPanelSts)
	{
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_RETURN_NOBACKGROUND_NOR, 1, &tOsdImgInfo[0]);	
		tOSD_Img2(&tOsdImgInfo[0], OSD_UPDATE);
	}
}

void UI_GuideLineSubSubSubMenuPage(UI_ArrowKey_t tArrowKey)
{
	OSD_IMG_INFO tOsdImgInfo[2];
	uint16_t uwSubSubMenuItemIdx = 
		(uint16_t)tSettingSubSubMenuItem.tSettingS[GUIDELINE_ITEM].tSubMenuInfo.ubItemIdx;
	uint16_t CamIdx = uwSubSubMenuItemIdx/2;	
	uint16_t uwPreVal_X = (tUI_CuSetting.GuideLine_XY[CamIdx][0] << 8) + tUI_CuSetting.GuideLine_XY[CamIdx][1];
	uint16_t uwPreVal_Y = (tUI_CuSetting.GuideLine_XY[CamIdx][2] << 8) + tUI_CuSetting.GuideLine_XY[CamIdx][3];
	uint16_t uwTempVal;
	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_GUIDELINE_SIZE1 + tUI_CuSetting.GuideLineSize[CamIdx], 1, &tOsdImgInfo[0]);
	//锟斤拷锟斤拷之前锟侥碉拷锟斤拷锟斤拷锟斤拷遥锟截诧拷锟斤拷状态
	tOsdImgInfo[0].uwXStart = uwPreVal_X;
	tOsdImgInfo[0].uwYStart = uwPreVal_Y;
	OSD_EraserImg2_NoUpdate(&tOsdImgInfo[0]);

	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_PARKINGLINE_L_R_FLAG, 1, &tOsdImgInfo[1]);
	tOsdImgInfo[1].uwXStart = tOsdImgInfo[0].uwXStart + (tOsdImgInfo[0].uwHSize - tOsdImgInfo[1].uwHSize)/2;
	if(tOsdImgInfo[0].uwYStart > tOsdImgInfo[1].uwVSize + 20)
		tOsdImgInfo[1].uwYStart = tOsdImgInfo[0].uwYStart - tOsdImgInfo[1].uwVSize - 20;
	else
		tOsdImgInfo[1].uwYStart = tOsdImgInfo[0].uwYStart + tOsdImgInfo[0].uwVSize + 20;
	OSD_EraserImg2_NoUpdate(&tOsdImgInfo[1]);


	
	switch(tArrowKey)
	{
		case LEFT_ARROW:
			//锟斤拷锟斤拷
			if(ParkingLineRemoteMode == LEFT_RIGHT_MODE)
			{
				if(((tUI_CuSetting.GuideLine_XY[CamIdx][0] << 8) + tUI_CuSetting.GuideLine_XY[CamIdx][1]) >= 5)
				{
					uwTempVal = (tUI_CuSetting.GuideLine_XY[CamIdx][0] << 8) + tUI_CuSetting.GuideLine_XY[CamIdx][1];
					uwTempVal -= 5;
					tUI_CuSetting.GuideLine_XY[CamIdx][0] = uwTempVal >> 8;
					tUI_CuSetting.GuideLine_XY[CamIdx][1] = (uint8_t)uwTempVal;
				}
				else
				{
					tUI_CuSetting.GuideLine_XY[CamIdx][0] = 0;
					tUI_CuSetting.GuideLine_XY[CamIdx][1] = 0;
				}
			}//锟斤拷锟斤拷
			else if(ParkingLineRemoteMode == UP_DOWN_MODE)
			{
				if(((tUI_CuSetting.GuideLine_XY[CamIdx][2] << 8) + tUI_CuSetting.GuideLine_XY[CamIdx][3]) >= 5)
				{
					uwTempVal = (tUI_CuSetting.GuideLine_XY[CamIdx][2] << 8) + tUI_CuSetting.GuideLine_XY[CamIdx][3];
					uwTempVal -= 5;
					tUI_CuSetting.GuideLine_XY[CamIdx][2] = uwTempVal >> 8;
					tUI_CuSetting.GuideLine_XY[CamIdx][3] = (uint8_t)uwTempVal;

				}
				else
				{
					tUI_CuSetting.GuideLine_XY[CamIdx][2] = 0;
					tUI_CuSetting.GuideLine_XY[CamIdx][3] = 0;

				}
			}//锟斤拷小
			else
			{
				if(tUI_CuSetting.GuideLineSize[CamIdx] > 0)
					tUI_CuSetting.GuideLineSize[CamIdx]--;
			}
			break;
		case RIGHT_ARROW:
			//锟斤拷锟斤拷
			if(ParkingLineRemoteMode == LEFT_RIGHT_MODE)
			{
				if(((tUI_CuSetting.GuideLine_XY[CamIdx][0] << 8) + tUI_CuSetting.GuideLine_XY[CamIdx][1])
					<= (OSD_WIDTH - tOsdImgInfo[0].uwHSize - 5))
				{
					uwTempVal = (tUI_CuSetting.GuideLine_XY[CamIdx][0] << 8) + tUI_CuSetting.GuideLine_XY[CamIdx][1];
					uwTempVal += 5;
					tUI_CuSetting.GuideLine_XY[CamIdx][0] = uwTempVal >> 8;
					tUI_CuSetting.GuideLine_XY[CamIdx][1] = (uint8_t)uwTempVal;

				}
				else
				{
					tUI_CuSetting.GuideLine_XY[CamIdx][0] = (OSD_WIDTH - tOsdImgInfo[0].uwHSize) >> 8;
					tUI_CuSetting.GuideLine_XY[CamIdx][1] = (uint8_t)(OSD_WIDTH - tOsdImgInfo[0].uwHSize);

				}

			}//锟斤拷锟斤拷
			else if(ParkingLineRemoteMode == UP_DOWN_MODE)
			{
				if(((tUI_CuSetting.GuideLine_XY[CamIdx][2] << 8) + tUI_CuSetting.GuideLine_XY[CamIdx][3]) 
					<= (OSD_HEIGHT - tOsdImgInfo[0].uwVSize - 5))
				{
					uwTempVal = (tUI_CuSetting.GuideLine_XY[CamIdx][2] << 8) + tUI_CuSetting.GuideLine_XY[CamIdx][3];
					uwTempVal += 5;
					tUI_CuSetting.GuideLine_XY[CamIdx][2] = uwTempVal >> 8;
					tUI_CuSetting.GuideLine_XY[CamIdx][3] = (uint8_t)uwTempVal;

				}
				else
				{
					tUI_CuSetting.GuideLine_XY[CamIdx][2] = (OSD_HEIGHT - tOsdImgInfo[0].uwVSize) >> 8;
					tUI_CuSetting.GuideLine_XY[CamIdx][3] = (uint8_t)(OSD_HEIGHT - tOsdImgInfo[0].uwVSize);

				}

			}//锟斤拷小
			else
			{
				if(tUI_CuSetting.GuideLineSize[CamIdx] < 17)
					tUI_CuSetting.GuideLineSize[CamIdx] ++;
			}
			break;
		case ENTER_ARROW:
			if(ParkingLineRemoteMode == LEFT_RIGHT_MODE)
				ParkingLineRemoteMode = UP_DOWN_MODE;
			else if(ParkingLineRemoteMode == UP_DOWN_MODE)
				ParkingLineRemoteMode = SIZE_MODE;
			else
				ParkingLineRemoteMode = LEFT_RIGHT_MODE;
			break;
		case EXIT_ARROW:
			UI_GuideLineSubSubSubMenuPageExit();
			SendIrCodeFlag = 0;
			return;
		default:
			return;
	}
		
	UI_GuideLineDrawSubSubSubMenuItem();

}

//-------------------------------------------AI----------------------------------------
uint8_t AILineRemoteMode = UnSelect;
uint8_t AILinePointIndex = 0;
//UI_ParkinglinePoint_t UI_AIDetectlinePoint = {{300,100},{800,100},{250,200},{850,200},{200,300},{900,300},{150,550},{950,550}};
//UI_ParkinglinePoint_t UI_AIDetectlinePoint[4] = {
//    {
//        {300, 100}, {800, 100}, {250, 200}, {850, 200},
//        {200, 300}, {900, 300}, {150, 550}, {950, 550}
//    },
//    {
//        {300, 100}, {800, 100}, {250, 200}, {850, 200},
//        {200, 300}, {900, 300}, {150, 550}, {950, 550}
//    },
//    {
//        {300, 100}, {800, 100}, {250, 200}, {850, 200},
//        {200, 300}, {900, 300}, {150, 550}, {950, 550}
//    },
//    {
//        {300, 100}, {800, 100}, {250, 200}, {850, 200},
//        {200, 300}, {900, 300}, {150, 550}, {950, 550}
//    }
//};
//uint8_t Location1[4] = {4,4,4,4}; // 绗簩鏉＄嚎鍦ㄦ暣涓涓殑姣斾緥
//uint8_t Location2[4] = {8,8,8,8}; // 绗笁鏉＄嚎鍦ㄦ暣涓涓殑姣斾緥
uint16_t LINE2_Mid_XPoint,LINE2_Mid_YPoint,LINE3_Mid_XPoint,LINE3_Mid_YPoint;

void UI_AISubSubSubMenuExit()
{
	tAISubSubSubMenuItem.tSubMenuInfo.ubItemIdx = 0;
	tAISubSubSubMenuItem.tSubMenuInfo.ubItemPreIdx = 0;
	AILineRemoteMode = UnSelect;
	AILinePointIndex = PonitUpLeft;
	UI_AISetupReturn();
	UI_SwitchViewType(DeskTopShowView,FALSE);
}
//-------------------------------------------------------------------------------------
void UI_AIDrawSubSubSubMenuItem(uint8_t ubDrawBox)
{
	uint8_t ubSubSubMenuItem = tSettingSubSubMenuItem.tSettingS[10].tSubMenuInfo.ubItemIdx;
	uint8_t CAMIdx = ubSubSubMenuItem%4;
	uint8_t ubItemPreIdx = tAISubSubSubMenuItem.tSubMenuInfo.ubItemPreIdx;
	uint8_t ubItemCurIdx = tAISubSubSubMenuItem.tSubMenuInfo.ubItemIdx;
	OSD_IMG_INFO tOsdImgInfo[8];
	//-------------------------------------pre------------------------------------------
	if(ubItemPreIdx != ubItemCurIdx)
	{
//		if(ubItemPreIdx == BSDRANGE_TYPE1_ITEM)//
//		{
//			tOSD_GetOsdImgInfor(1, OSD_IMG2,OSD2IMG_AI_FRAME_TYPE1_NOR , 4, &tOsdImgInfo[0]);//绫诲瀷涓�鍒ゆ柇
//			if(tUI_CuSetting.ubBSDBoxType)//鍊间负1 绫诲瀷浜?
//				tOSD_Img2(&tOsdImgInfo[0], OSD_QUEUE);//娌￠�変腑鍐呴儴瀛椾綋涓虹櫧鑹?
//			else
//				tOSD_Img2(&tOsdImgInfo[2], OSD_QUEUE);//閫変腑鍐呴儴瀛椾綋涓虹豢鑹?
//		}
//		else if(ubItemPreIdx == BSDRANGE_TYPE2_ITEM)
//		{
//			tOSD_GetOsdImgInfor(1, OSD_IMG2,OSD2IMG_AI_FRAME_TYPE2_NOR , 4, &tOsdImgInfo[0]);//绫诲瀷浜屽垽鏂?
//			if(tUI_CuSetting.ubBSDBoxType)//鍊间负1 绫诲瀷浜?
//				tOSD_Img2(&tOsdImgInfo[2], OSD_QUEUE);//閫変腑鍐呴儴瀛椾綋涓虹豢鑹?
//			else
//				tOSD_Img2(&tOsdImgInfo[0], OSD_QUEUE);//娌￠�変腑鍐呴儴瀛椾綋涓虹櫧鑹?
//
//		}
		if(ubItemPreIdx == BSDRANGE_ENABLE_ITEM)
		{
			tOSD_GetOsdImgInfor(1, OSD_IMG2,OSD2IMG_CAMS_H_MIRROR_OFF_NOR , 4, &tOsdImgInfo[0]);
			if(tUI_CuSetting.ubIsEnableBSDRANGE[CAMIdx])
			{
				tOsdImgInfo[2].uwXStart = 30;
				tOsdImgInfo[2].uwYStart = 20;
				tOSD_Img2(&tOsdImgInfo[2], OSD_QUEUE);
			}
			else
			{
				tOsdImgInfo[0].uwXStart = 30;
				tOsdImgInfo[0].uwYStart = 20;
				tOSD_Img2(&tOsdImgInfo[0], OSD_QUEUE); 		
			}
		}
		else if(ubItemPreIdx == BSDRANGE_RESET_ITEM)
		{
			tOSD_GetOsdImgInfor(1, OSD_IMG2,OSD2IMG_AI_RESET_NOR , 2, &tOsdImgInfo[0]);
			tOSD_Img2(&tOsdImgInfo[0], OSD_QUEUE); 		
		}		
		#if 0
		else if(ubItemPreIdx == PERSON_DET_ITEM)
		{
			tOSD_GetOsdImgInfor(1, OSD_IMG2,OSD2IMG_AI_OFF_NOR , 4, &tOsdImgInfo[0]);
			if(tUI_CuSetting.ubDetectPeopleFlag[0])
			{
				tOsdImgInfo[2].uwXStart = 173 + 720;
				tOsdImgInfo[2].uwYStart = 20;
//					tOnOffOsdImgInfo[0].uwXStart = +(i%2)*720;	
//				tOnOffOsdImgInfo[0].uwYStart = 20+(i%3)*50;
				tOSD_Img2(&tOsdImgInfo[2], OSD_UPDATE);
			}
			else
			{
				tOsdImgInfo[0].uwXStart = 173 + 720;
				tOsdImgInfo[0].uwYStart = 20;
				tOSD_Img2(&tOsdImgInfo[0], OSD_UPDATE);			
			}
		}	
		else if(ubItemPreIdx == BSDRANGE_ENABLE_ITEM)
		{
			tOSD_GetOsdImgInfor(1, OSD_IMG2,OSD2IMG_AI_OFF_NOR , 4, &tOsdImgInfo[0]);
			if(tUI_CuSetting.ubIsEnableBSDRANGE[0])
			{
				tOsdImgInfo[2].uwXStart = 173;
				tOsdImgInfo[2].uwYStart = 20 + 50;
	//					tOnOffOsdImgInfo[0].uwXStart = +(i%2)*720;	
	//				tOnOffOsdImgInfo[0].uwYStart = 20+(i%3)*50;
				tOSD_Img2(&tOsdImgInfo[2], OSD_UPDATE);
			}
			else
			{
				tOsdImgInfo[0].uwXStart = 173;
				tOsdImgInfo[0].uwYStart = 20 + 50;
				tOSD_Img2(&tOsdImgInfo[0], OSD_UPDATE); 		
			}
		}
		else if(ubItemPreIdx == VEHICLES_DET_ITEM)
		{

			tOSD_GetOsdImgInfor(1, OSD_IMG2,OSD2IMG_AI_OFF_NOR , 4, &tOsdImgInfo[0]);
			if(tUI_CuSetting.ubDetectCarFlag[0])
			{
				tOsdImgInfo[2].uwXStart = 173 +720;
				tOsdImgInfo[2].uwYStart = 20 + 50;
	//					tOnOffOsdImgInfo[0].uwXStart = +(i%2)*720;	
	//				tOnOffOsdImgInfo[0].uwYStart = 20+(i%3)*50;
				tOSD_Img2(&tOsdImgInfo[2], OSD_UPDATE);
			}
			else
			{
				tOsdImgInfo[0].uwXStart = 173 + 720;
				tOsdImgInfo[0].uwYStart = 20 + 50;
				tOSD_Img2(&tOsdImgInfo[0], OSD_UPDATE); 		
			}

		}
		else if(ubItemPreIdx == BSDALARM_ENABLE_ITEM)
		{
			tOSD_GetOsdImgInfor(1, OSD_IMG2,OSD2IMG_AI_OFF_NOR , 4, &tOsdImgInfo[0]);
			if(tUI_CuSetting.ubIsEnableBSDALARM[0])
			{
				tOsdImgInfo[2].uwXStart = 173 ;
				tOsdImgInfo[2].uwYStart = 20 + 50*2;
	//					tOnOffOsdImgInfo[0].uwXStart = +(i%2)*720;	
	//				tOnOffOsdImgInfo[0].uwYStart = 20+(i%3)*50;
				tOSD_Img2(&tOsdImgInfo[2], OSD_UPDATE);
			}
			else
			{
				tOsdImgInfo[0].uwXStart = 173;
				tOsdImgInfo[0].uwYStart = 20 + 50*2;
				tOSD_Img2(&tOsdImgInfo[0], OSD_UPDATE); 		
			}


		}
		else if(ubItemPreIdx == BSDTRIGGER_OUT_ITEM)
		{
			tOSD_GetOsdImgInfor(1, OSD_IMG2,OSD2IMG_AI_OFF_NOR , 4, &tOsdImgInfo[0]);
			if(tUI_CuSetting.ubBSDTriggerOut)
			{
				tOsdImgInfo[2].uwXStart = 173 +720;
				tOsdImgInfo[2].uwYStart = 20 + 50*2;
	//					tOnOffOsdImgInfo[0].uwXStart = +(i%2)*720;	
	//				tOnOffOsdImgInfo[0].uwYStart = 20+(i%3)*50;
				tOSD_Img2(&tOsdImgInfo[2], OSD_UPDATE);
			}
			else
			{
				tOsdImgInfo[0].uwXStart = 173 + 720;
				tOsdImgInfo[0].uwYStart = 20 + 50*2;
				tOSD_Img2(&tOsdImgInfo[0], OSD_UPDATE); 		
			}

		}		
		#endif
		else if(ubItemPreIdx == BSDRANGE_UP_ITEM)
		{
			tOSD_GetOsdImgInfor(1, OSD_IMG2,OSD2IMG_AI_CHOOSE_UP_NOR, 1, &tOsdImgInfo[0]);	
			tOSD_Img2(&tOsdImgInfo[0], OSD_QUEUE); 			
		}
		else if(ubItemPreIdx == BSDRANGE_LEFT_ITEM)
		{
			tOSD_GetOsdImgInfor(1, OSD_IMG2,OSD2IMG_AI_CHOOSE_LEFT_NOR, 1, &tOsdImgInfo[0]);	
			tOSD_Img2(&tOsdImgInfo[0], OSD_QUEUE); 			
		}
		else if(ubItemPreIdx == BSDRANGE_OK_ITEM)
		{
			tOSD_GetOsdImgInfor(1, OSD_IMG2,OSD2IMG_AI_CHOOSE_OK_NOR, 1, &tOsdImgInfo[0]);	
			tOSD_Img2(&tOsdImgInfo[0], OSD_QUEUE); 			
		}
		else if(ubItemPreIdx == BSDRANGE_RIGHT_ITEM)
		{
			tOSD_GetOsdImgInfor(1, OSD_IMG2,OSD2IMG_AI_CHOOSE_RIGHT_NOR, 1, &tOsdImgInfo[0]);	
			tOSD_Img2(&tOsdImgInfo[0], OSD_QUEUE); 			
		}
		else if(ubItemPreIdx == BSDRANGE_DOWN_ITEM)
		{
			tOSD_GetOsdImgInfor(1, OSD_IMG2,OSD2IMG_AI_CHOOSE_DOWN_NOR, 1, &tOsdImgInfo[0]);	
			tOSD_Img2(&tOsdImgInfo[0], OSD_QUEUE); 			
		}

	//------------------------------------cur------------------------------------------------
	}
//	if(ubItemCurIdx == BSDRANGE_TYPE1_ITEM)//
//	{
//		tOSD_GetOsdImgInfor(1, OSD_IMG2,OSD2IMG_AI_FRAME_TYPE1_NOR , 4, &tOsdImgInfo[0]);
////		if(ubDrawBox)
////		{
//			if(tUI_CuSetting.ubBSDBoxType)
//				tOSD_Img2(&tOsdImgInfo[1], OSD_UPDATE);//閫変腑鍐呴儴瀛椾綋涓虹豢鑹?
//			else
//				tOSD_Img2(&tOsdImgInfo[3], OSD_UPDATE);//娌￠�変腑鍐呴儴瀛椾綋涓虹櫧鑹?
////		}
////		else
////		{	
////			if(tUI_CuSetting.ubBSDBoxType)
////				tOSD_Img2(&tOsdImgInfo[0], OSD_UPDATE);//閫変腑鍐呴儴瀛椾綋涓虹豢鑹?
////			else
////				tOSD_Img2(&tOsdImgInfo[2], OSD_UPDATE);//娌￠�変腑鍐呴儴瀛椾綋涓虹櫧鑹?
////		}
//	}
//	else if(ubItemCurIdx == BSDRANGE_TYPE2_ITEM)
//	{
////		if(ubDrawBox)
////		{
//			tOSD_GetOsdImgInfor(1, OSD_IMG2,OSD2IMG_AI_FRAME_TYPE2_NOR , 4, &tOsdImgInfo[0]);
//			if(tUI_CuSetting.ubBSDBoxType)
//				tOSD_Img2(&tOsdImgInfo[3], OSD_UPDATE);
//			else
//				tOSD_Img2(&tOsdImgInfo[1], OSD_UPDATE);
////		}
////		else
////		{
////			tOSD_GetOsdImgInfor(1, OSD_IMG2,OSD2IMG_AI_FRAME_TYPE2_NOR , 4, &tOsdImgInfo[0]);
////			if(tUI_CuSetting.ubBSDBoxType)
////				tOSD_Img2(&tOsdImgInfo[2], OSD_UPDATE);
////			else
////				tOSD_Img2(&tOsdImgInfo[0], OSD_UPDATE);
////		}		
//
//	}
	if(ubItemCurIdx == BSDRANGE_ENABLE_ITEM)
	{
		tOSD_GetOsdImgInfor(1, OSD_IMG2,OSD2IMG_CAMS_H_MIRROR_OFF_NOR , 4, &tOsdImgInfo[0]);
		if(tUI_CuSetting.ubIsEnableBSDRANGE[CAMIdx])
		{
			tOsdImgInfo[3].uwXStart = 30;
			tOsdImgInfo[3].uwYStart = 20;
			tOSD_Img2(&tOsdImgInfo[3], OSD_UPDATE);
		}
		else
		{
			tOsdImgInfo[1].uwXStart = 30;
			tOsdImgInfo[1].uwYStart = 20;
			tOSD_Img2(&tOsdImgInfo[1], OSD_UPDATE); 		
		}
	}
	else if(ubItemCurIdx == BSDRANGE_RESET_ITEM)
	{
		tOSD_GetOsdImgInfor(1, OSD_IMG2,OSD2IMG_AI_RESET_NOR, 2, &tOsdImgInfo[0]);
		tOSD_Img2(&tOsdImgInfo[1], OSD_UPDATE);		
	}	

	#if 0
	else if(ubItemCurIdx == PERSON_DET_ITEM)
	{
		tOSD_GetOsdImgInfor(1, OSD_IMG2,OSD2IMG_AI_OFF_NOR , 4, &tOsdImgInfo[0]);
		if(tUI_CuSetting.ubDetectPeopleFlag)
		{
			tOsdImgInfo[3].uwXStart = 173 + 720;
			tOsdImgInfo[3].uwYStart = 20;
//					tOnOffOsdImgInfo[0].uwXStart = +(i%2)*720;	
//				tOnOffOsdImgInfo[0].uwYStart = 20+(i%3)*50;
			tOSD_Img2(&tOsdImgInfo[3], OSD_UPDATE);
		}
		else
		{
			tOsdImgInfo[1].uwXStart = 173 + 720;
			tOsdImgInfo[1].uwYStart = 20;
			tOSD_Img2(&tOsdImgInfo[1], OSD_UPDATE); 		
		}

	}	
	else if(ubItemCurIdx == BSDRANGE_ENABLE_ITEM)
	{
		tOSD_GetOsdImgInfor(1, OSD_IMG2,OSD2IMG_AI_OFF_NOR , 4, &tOsdImgInfo[0]);
		if(tUI_CuSetting.ubIsEnableBSDRANGE[0])
		{
			tOsdImgInfo[3].uwXStart = 173;
			tOsdImgInfo[3].uwYStart = 20 + 50;
//					tOnOffOsdImgInfo[0].uwXStart = +(i%2)*720;	
//				tOnOffOsdImgInfo[0].uwYStart = 20+(i%3)*50;
			tOSD_Img2(&tOsdImgInfo[3], OSD_UPDATE);
		}
		else
		{
			tOsdImgInfo[1].uwXStart = 173;
			tOsdImgInfo[1].uwYStart = 20 + 50;
			tOSD_Img2(&tOsdImgInfo[1], OSD_UPDATE); 		
		}

	}
	else if(ubItemCurIdx == VEHICLES_DET_ITEM)
	{
		tOSD_GetOsdImgInfor(1, OSD_IMG2,OSD2IMG_AI_OFF_NOR , 4, &tOsdImgInfo[0]);
		if(tUI_CuSetting.ubDetectCarFlag[0])
		{
			tOsdImgInfo[3].uwXStart = 173 +720;
			tOsdImgInfo[3].uwYStart = 20 + 50;
//					tOnOffOsdImgInfo[0].uwXStart = +(i%2)*720;	
//				tOnOffOsdImgInfo[0].uwYStart = 20+(i%3)*50;
			tOSD_Img2(&tOsdImgInfo[3], OSD_UPDATE);
		}
		else
		{
			tOsdImgInfo[1].uwXStart = 173 + 720;
			tOsdImgInfo[1].uwYStart = 20 + 50;
			tOSD_Img2(&tOsdImgInfo[1], OSD_UPDATE); 		
		}

	}
	else if(ubItemCurIdx == BSDALARM_ENABLE_ITEM)
	{
		tOSD_GetOsdImgInfor(1, OSD_IMG2,OSD2IMG_AI_OFF_NOR , 4, &tOsdImgInfo[0]);
		if(tUI_CuSetting.ubIsEnableBSDALARM[0])
		{
			tOsdImgInfo[3].uwXStart = 173 ;
			tOsdImgInfo[3].uwYStart = 20 + 50*2;
//					tOnOffOsdImgInfo[0].uwXStart = +(i%2)*720;	
//				tOnOffOsdImgInfo[0].uwYStart = 20+(i%3)*50;
			tOSD_Img2(&tOsdImgInfo[3], OSD_UPDATE);
		}
		else
		{
			tOsdImgInfo[1].uwXStart = 173;
			tOsdImgInfo[1].uwYStart = 20 + 50*2;
			tOSD_Img2(&tOsdImgInfo[1], OSD_UPDATE); 		
		}


	}
	else if(ubItemCurIdx == BSDTRIGGER_OUT_ITEM)
	{
		tOSD_GetOsdImgInfor(1, OSD_IMG2,OSD2IMG_AI_OFF_NOR , 4, &tOsdImgInfo[0]);
		if(tUI_CuSetting.ubBSDTriggerOut)
		{
			tOsdImgInfo[3].uwXStart = 173 + 720;
			tOsdImgInfo[3].uwYStart = 20 + 50*2;
//					tOnOffOsdImgInfo[0].uwXStart = +(i%2)*720;	
//				tOnOffOsdImgInfo[0].uwYStart = 20+(i%3)*50;
			tOSD_Img2(&tOsdImgInfo[3], OSD_UPDATE);
		}
		else
		{
			tOsdImgInfo[1].uwXStart = 173 + 720;
			tOsdImgInfo[1].uwYStart = 20 + 50*2;
			tOSD_Img2(&tOsdImgInfo[1], OSD_UPDATE); 		
		}

	}
	#endif
	else if(ubItemCurIdx == BSDRANGE_UP_ITEM)
	{
		tOSD_GetOsdImgInfor(1, OSD_IMG2,OSD2IMG_AI_CHOOSE_UP_HL, 1, &tOsdImgInfo[0]);	
		tOSD_Img2(&tOsdImgInfo[0], OSD_UPDATE); 			
	}
	else if(ubItemCurIdx == BSDRANGE_LEFT_ITEM)
	{
		tOSD_GetOsdImgInfor(1, OSD_IMG2,OSD2IMG_AI_CHOOSE_LEFT_HL, 1, &tOsdImgInfo[0]);	
		tOSD_Img2(&tOsdImgInfo[0], OSD_UPDATE); 			
	}
	else if(ubItemCurIdx == BSDRANGE_OK_ITEM)
	{
		tOSD_GetOsdImgInfor(1, OSD_IMG2,OSD2IMG_AI_CHOOSE_OK_HL, 1, &tOsdImgInfo[0]);	
		tOSD_Img2(&tOsdImgInfo[0], OSD_UPDATE); 			
	}
	else if(ubItemCurIdx == BSDRANGE_RIGHT_ITEM)
	{
		tOSD_GetOsdImgInfor(1, OSD_IMG2,OSD2IMG_AI_CHOOSE_RIGHT_HL, 1, &tOsdImgInfo[0]);	
		tOSD_Img2(&tOsdImgInfo[0], OSD_UPDATE); 			
	}
	else if(ubItemCurIdx == BSDRANGE_DOWN_ITEM)
	{
		tOSD_GetOsdImgInfor(1, OSD_IMG2,OSD2IMG_AI_CHOOSE_DOWN_HL, 1, &tOsdImgInfo[0]);	
		tOSD_Img2(&tOsdImgInfo[0], OSD_UPDATE); 			
	}


}
//------------------------------------------------------------------------------------

void UI_AISubSubSubMenuPage(UI_ArrowKey_t tArrowKey)
{
	OSD_IMG_INFO tOsdImgInfo;
	uint8_t ubSubSubMenuItem = tAISubSubSubMenuItem.tSubMenuInfo.ubItemIdx;
	uint8_t AILinePointPreIndex = AILinePointIndex;
	uint8_t CamIdx = tSettingSubSubMenuItem.tSettingS[10].tSubMenuInfo.ubItemIdx % 4;//璁＄畻鐜板湪鏄鍑犱釜閫氶亾
	tAISubSubSubMenuItem.tSubMenuInfo.ubItemPreIdx = tAISubSubSubMenuItem.tSubMenuInfo.ubItemIdx;
	if(AISubSubSubRemoteMode == REMOTE_SELECT_ITEM)
	{
		switch(tArrowKey)
		{
			case LEFT_ARROW:
				if(tAISubSubSubMenuItem.tSubMenuInfo.ubItemIdx ==
					tAISubSubSubMenuItem.ubFirstItem)
				{
					tAISubSubSubMenuItem.tSubMenuInfo.ubItemIdx = 
						tAISubSubSubMenuItem.ubItemCount - 1;
				}
				else
					tAISubSubSubMenuItem.tSubMenuInfo.ubItemIdx --;
				break;
			case RIGHT_ARROW:
				if(tAISubSubSubMenuItem.tSubMenuInfo.ubItemIdx ==
					tAISubSubSubMenuItem.ubItemCount - 1)
				{
					tAISubSubSubMenuItem.tSubMenuInfo.ubItemIdx = 
						tAISubSubSubMenuItem.ubFirstItem;
				}
				else
					tAISubSubSubMenuItem.tSubMenuInfo.ubItemIdx ++;
				break;
			case ENTER_ARROW:
//				if(tAISubSubSubMenuItem.tSubMenuInfo.ubItemIdx == BSDRANGE_TYPE1_ITEM)
//				{
//					tUI_CuSetting.ubBSDBoxType = 0;
//					//澶嶇敤鍒峰浘鐨勫嚱鏁帮紝鍒峰浘
//					tAISubSubSubMenuItem.tSubMenuInfo.ubItemPreIdx = BSDRANGE_TYPE2_ITEM;
//				}
//				else if(tAISubSubSubMenuItem.tSubMenuInfo.ubItemIdx == BSDRANGE_TYPE2_ITEM)
//				{
//					tUI_CuSetting.ubBSDBoxType = 1;
//					//澶嶇敤鍒峰浘鐨勫嚱鏁帮紝鍒峰浘
//					tAISubSubSubMenuItem.tSubMenuInfo.ubItemPreIdx = BSDRANGE_TYPE1_ITEM;
//				}
				if(tAISubSubSubMenuItem.tSubMenuInfo.ubItemIdx == BSDRANGE_ENABLE_ITEM)
				{
					tUI_CuSetting.ubIsEnableBSDRANGE[CamIdx] = 1 - tUI_CuSetting.ubIsEnableBSDRANGE[CamIdx];
				}
				else if(tAISubSubSubMenuItem.tSubMenuInfo.ubItemIdx == BSDRANGE_RESET_ITEM)
				{
					//reset
					tUI_CuSetting.tUI_AIDetectlinePoint[CamIdx] = (UI_ParkinglinePoint_t) 
					{
					    .tupLeft = {400, 100},
					    .tupRight = {700, 100},
					    .tmid_upLeft = {350, 250},
					    .tmid_upRight = {650, 250},
					    .tmid_downLeft = {250, 350},
					    .tmid_downRight = {750, 350},
					    .tdownLeft = {160, 580},
					    .tdownRight = {940, 580}
					};
						
					AILineRemoteMode = UnSelect;
					AILinePointIndex = PonitUpLeft;
					tUI_CuSetting.tLocation1[CamIdx] = 4;
					tUI_CuSetting.tLocation2[CamIdx] = 8;
					UI_AISubSubMenuExecute(CamIdx,0);	
					
				}
				else
				{
					if(AILineRemoteMode == UnSelect)
					{
						printf("111111111111111111\n");
						if(tAISubSubSubMenuItem.tSubMenuInfo.ubItemIdx == BSDRANGE_UP_ITEM)//up
						{
//							if(AILinePointIndex == 0)
//								AILinePointIndex = POINT_INDEX_MAX - 2;
//							else if(AILinePointIndex == 1)
//								AILinePointIndex = POINT_INDEX_MAX - 1;
//							else
//								AILinePointIndex -= 2;
							if(AILinePointIndex == 0)
								AILinePointIndex = POINT_INDEX_MAX - 1;
							else
								AILinePointIndex -= 1;
						}
						else if(tAISubSubSubMenuItem.tSubMenuInfo.ubItemIdx == BSDRANGE_DOWN_ITEM)//down
						{
//							if(AILinePointIndex == POINT_INDEX_MAX - 1)
//								AILinePointIndex = 1;
//							else if(AILinePointIndex == POINT_INDEX_MAX - 2)
//								AILinePointIndex = 0;
//							else
//								AILinePointIndex += 2;
							if(AILinePointIndex == POINT_INDEX_MAX - 1)
								AILinePointIndex = 0;
							else
								AILinePointIndex += 1;

						}				
						else if(tAISubSubSubMenuItem.tSubMenuInfo.ubItemIdx == BSDRANGE_OK_ITEM)//ok
						{
							AILineRemoteMode = 1 - AILineRemoteMode;
						}
						else if(tAISubSubSubMenuItem.tSubMenuInfo.ubItemIdx == BSDRANGE_LEFT_ITEM)//left
						{
							if(AILinePointIndex == 0)
								AILinePointIndex = POINT_INDEX_MAX - 1;
							else
								AILinePointIndex -= 1;
						}				
						else if(tAISubSubSubMenuItem.tSubMenuInfo.ubItemIdx == BSDRANGE_RIGHT_ITEM)//right
						{
							if(AILinePointIndex == POINT_INDEX_MAX - 1)
								AILinePointIndex = 0;
							else
								AILinePointIndex += 1;
						}					
					}
					else if(AILineRemoteMode == Selected)
					{
						if(tAISubSubSubMenuItem.tSubMenuInfo.ubItemIdx == BSDRANGE_UP_ITEM)
						{
							switch(AILinePointIndex)
							{
								case PonitUpLeft:
								case PonitUpRight:
									tUI_CuSetting.tUI_AIDetectlinePoint[CamIdx].tupLeft.yPoint  -= ZoomV_Pixels;
									tUI_CuSetting.tUI_AIDetectlinePoint[CamIdx].tupRight.yPoint -= ZoomV_Pixels;
									break;
								case PonitDownLeft:
								case PonitDownRight:
									tUI_CuSetting.tUI_AIDetectlinePoint[CamIdx].tdownLeft.yPoint  -= ZoomV_Pixels;
									tUI_CuSetting.tUI_AIDetectlinePoint[CamIdx].tdownRight.yPoint -= ZoomV_Pixels;
									break;
								case SecondLine:
									tUI_CuSetting.tLocation1[CamIdx] --;
									if(tUI_CuSetting.tLocation1[CamIdx] <= 0)
										tUI_CuSetting.tLocation1[CamIdx] = 1;
									break;
								case ThirdLine:
									tUI_CuSetting.tLocation2[CamIdx] --;									
									if(tUI_CuSetting.tLocation2[CamIdx] <= tUI_CuSetting.tLocation1[CamIdx])
										tUI_CuSetting.tLocation2[CamIdx] = tUI_CuSetting.tLocation1[CamIdx] + 1;
									break;
								case WholeArea:
									tUI_CuSetting.tUI_AIDetectlinePoint[CamIdx].tupRight.yPoint -= ZoomV_Pixels;
									tUI_CuSetting.tUI_AIDetectlinePoint[CamIdx].tupLeft.yPoint  -= ZoomV_Pixels;	
							        tUI_CuSetting.tUI_AIDetectlinePoint[CamIdx].tdownRight.yPoint -= ZoomV_Pixels; 
									tUI_CuSetting.tUI_AIDetectlinePoint[CamIdx].tdownLeft.yPoint  -= ZoomV_Pixels;	
									break;

							}
							if(tUI_CuSetting.tUI_AIDetectlinePoint[CamIdx].tupLeft.yPoint < 0)
							{
								tUI_CuSetting.tUI_AIDetectlinePoint[CamIdx].tupLeft.yPoint  += ZoomV_Pixels;
								tUI_CuSetting.tUI_AIDetectlinePoint[CamIdx].tupRight.yPoint += ZoomV_Pixels;
								if(AILinePointIndex == WholeArea)
								{
									tUI_CuSetting.tUI_AIDetectlinePoint[CamIdx].tdownRight.yPoint += ZoomV_Pixels;
									tUI_CuSetting.tUI_AIDetectlinePoint[CamIdx].tdownLeft.yPoint  += ZoomV_Pixels;	
								}
							}
							if(tUI_CuSetting.tUI_AIDetectlinePoint[CamIdx].tdownLeft.yPoint < tUI_CuSetting.tUI_AIDetectlinePoint[CamIdx].tupLeft.yPoint )
							{
								tUI_CuSetting.tUI_AIDetectlinePoint[CamIdx].tdownRight.yPoint += ZoomV_Pixels;
								tUI_CuSetting.tUI_AIDetectlinePoint[CamIdx].tdownLeft.yPoint  += ZoomV_Pixels;							
							}
						}
						else if(tAISubSubSubMenuItem.tSubMenuInfo.ubItemIdx == BSDRANGE_LEFT_ITEM)
						{
							switch(AILinePointIndex)
							{
								case PonitUpLeft:
									tUI_CuSetting.tUI_AIDetectlinePoint[CamIdx].tupLeft.xPoint    -= ZoomH_Pixels;
									break;
								case PonitDownLeft:
									tUI_CuSetting.tUI_AIDetectlinePoint[CamIdx].tdownLeft.xPoint  -= ZoomH_Pixels;
									break;
								case PonitUpRight:
									tUI_CuSetting.tUI_AIDetectlinePoint[CamIdx].tupRight.xPoint   -= ZoomH_Pixels;
									break;
								case PonitDownRight:
									tUI_CuSetting.tUI_AIDetectlinePoint[CamIdx].tdownRight.xPoint -= ZoomH_Pixels;
									break;
								case SecondLine:
									tUI_CuSetting.tLocation1[CamIdx] --;
									if(tUI_CuSetting.tLocation1[CamIdx] <= 0)
										tUI_CuSetting.tLocation1[CamIdx] = 1;
									break;
								case ThirdLine:
									tUI_CuSetting.tLocation2[CamIdx] --;									
									if(tUI_CuSetting.tLocation2[CamIdx] <= tUI_CuSetting.tLocation1[CamIdx])
										tUI_CuSetting.tLocation2[CamIdx] = tUI_CuSetting.tLocation1[CamIdx] + 1;
									break;
								case WholeArea:
									if(tUI_CuSetting.tUI_AIDetectlinePoint[CamIdx].tupRight.xPoint > ZoomH_Pixels && tUI_CuSetting.tUI_AIDetectlinePoint[CamIdx].tupLeft.xPoint > ZoomH_Pixels && tUI_CuSetting.tUI_AIDetectlinePoint[CamIdx].tdownRight.xPoint > ZoomH_Pixels && tUI_CuSetting.tUI_AIDetectlinePoint[CamIdx].tdownLeft.xPoint > ZoomH_Pixels)
									{
										tUI_CuSetting.tUI_AIDetectlinePoint[CamIdx].tupRight.xPoint -= ZoomH_Pixels;
										tUI_CuSetting.tUI_AIDetectlinePoint[CamIdx].tupLeft.xPoint  -= ZoomH_Pixels;	
								       	tUI_CuSetting.tUI_AIDetectlinePoint[CamIdx].tdownRight.xPoint -= ZoomH_Pixels; 
										tUI_CuSetting.tUI_AIDetectlinePoint[CamIdx].tdownLeft.xPoint  -= ZoomH_Pixels;	
									}
									break;

							}
							if(tUI_CuSetting.tUI_AIDetectlinePoint[CamIdx].tupLeft.xPoint < 0)
							{
								tUI_CuSetting.tUI_AIDetectlinePoint[CamIdx].tupLeft.xPoint  += ZoomH_Pixels;
							}
							if(tUI_CuSetting.tUI_AIDetectlinePoint[CamIdx].tdownLeft.xPoint < 0)
							{
								tUI_CuSetting.tUI_AIDetectlinePoint[CamIdx].tdownLeft.xPoint  += ZoomH_Pixels;							
							}
							if(tUI_CuSetting.tUI_AIDetectlinePoint[CamIdx].tupRight.xPoint < 0)
							{
								tUI_CuSetting.tUI_AIDetectlinePoint[CamIdx].tupRight.xPoint  += ZoomH_Pixels;							
							}
							if(tUI_CuSetting.tUI_AIDetectlinePoint[CamIdx].tdownRight.xPoint < 0)
							{
								tUI_CuSetting.tUI_AIDetectlinePoint[CamIdx].tdownRight.xPoint  += ZoomH_Pixels;							
							}
						}
						else if(tAISubSubSubMenuItem.tSubMenuInfo.ubItemIdx == BSDRANGE_OK_ITEM)//ok
						{
							AILineRemoteMode = 1 - AILineRemoteMode;
						}
						else if(tAISubSubSubMenuItem.tSubMenuInfo.ubItemIdx == BSDRANGE_RIGHT_ITEM)
						{
							switch(AILinePointIndex)
							{
								case PonitUpLeft:
									tUI_CuSetting.tUI_AIDetectlinePoint[CamIdx].tupLeft.xPoint    += ZoomH_Pixels;
									break;
								case PonitDownLeft:
									tUI_CuSetting.tUI_AIDetectlinePoint[CamIdx].tdownLeft.xPoint  += ZoomH_Pixels;
									break;
								case PonitUpRight:
									tUI_CuSetting.tUI_AIDetectlinePoint[CamIdx].tupRight.xPoint   += ZoomH_Pixels;
									break;
								case PonitDownRight:
									tUI_CuSetting.tUI_AIDetectlinePoint[CamIdx].tdownRight.xPoint += ZoomH_Pixels;
									break;
								case SecondLine:
									tUI_CuSetting.tLocation1[CamIdx] ++;
									if(tUI_CuSetting.tLocation1[CamIdx] >= tUI_CuSetting.tLocation2[CamIdx])
										tUI_CuSetting.tLocation1[CamIdx] = tUI_CuSetting.tLocation2[CamIdx] - 1;

									break;
								case ThirdLine:
									tUI_CuSetting.tLocation2[CamIdx] ++;									
									if(tUI_CuSetting.tLocation2[CamIdx]>=12)
										tUI_CuSetting.tLocation2[CamIdx] = 11;	
									break;
								case WholeArea:
									if(tUI_CuSetting.tUI_AIDetectlinePoint[CamIdx].tupRight.xPoint < 1024 - ZoomH_Pixels && tUI_CuSetting.tUI_AIDetectlinePoint[CamIdx].tupLeft.xPoint < 1024 - ZoomH_Pixels && tUI_CuSetting.tUI_AIDetectlinePoint[CamIdx].tdownRight.xPoint < 1024 - ZoomH_Pixels && tUI_CuSetting.tUI_AIDetectlinePoint[CamIdx].tdownLeft.xPoint < 1024 - ZoomH_Pixels)
									{
										tUI_CuSetting.tUI_AIDetectlinePoint[CamIdx].tupRight.xPoint += ZoomH_Pixels;
										tUI_CuSetting.tUI_AIDetectlinePoint[CamIdx].tupLeft.xPoint  += ZoomH_Pixels;	
								        tUI_CuSetting.tUI_AIDetectlinePoint[CamIdx].tdownRight.xPoint += ZoomH_Pixels; 
										tUI_CuSetting.tUI_AIDetectlinePoint[CamIdx].tdownLeft.xPoint  += ZoomH_Pixels;	
									}
									break;
							}
							if(tUI_CuSetting.tUI_AIDetectlinePoint[CamIdx].tupLeft.xPoint > 1024 - ZoomH_Pixels)
							{
								tUI_CuSetting.tUI_AIDetectlinePoint[CamIdx].tupLeft.xPoint  = 1024 - ZoomH_Pixels;
							}
							if(tUI_CuSetting.tUI_AIDetectlinePoint[CamIdx].tdownLeft.xPoint > 1024 - ZoomH_Pixels)
							{
								tUI_CuSetting.tUI_AIDetectlinePoint[CamIdx].tdownLeft.xPoint  = 1024 - ZoomH_Pixels;							
							}
							if(tUI_CuSetting.tUI_AIDetectlinePoint[CamIdx].tupRight.xPoint > 1024 - ZoomH_Pixels)
							{
								tUI_CuSetting.tUI_AIDetectlinePoint[CamIdx].tupRight.xPoint  = 1024 - ZoomH_Pixels;							
							}
							if(tUI_CuSetting.tUI_AIDetectlinePoint[CamIdx].tdownRight.xPoint > 1024 - ZoomH_Pixels)
							{
								tUI_CuSetting.tUI_AIDetectlinePoint[CamIdx].tdownRight.xPoint  = 1024 - ZoomH_Pixels;							
							}							
						}
						else if(tAISubSubSubMenuItem.tSubMenuInfo.ubItemIdx == BSDRANGE_DOWN_ITEM)
						{
							switch(AILinePointIndex)
							{
								case PonitUpLeft:
								case PonitUpRight:
									tUI_CuSetting.tUI_AIDetectlinePoint[CamIdx].tupLeft.yPoint  += ZoomV_Pixels;
									tUI_CuSetting.tUI_AIDetectlinePoint[CamIdx].tupRight.yPoint += ZoomV_Pixels;
									break;
								case PonitDownLeft:
								case PonitDownRight:
									tUI_CuSetting.tUI_AIDetectlinePoint[CamIdx].tdownLeft.yPoint  += ZoomV_Pixels;
									tUI_CuSetting.tUI_AIDetectlinePoint[CamIdx].tdownRight.yPoint += ZoomV_Pixels;
									break;
								case SecondLine:
									tUI_CuSetting.tLocation1[CamIdx] ++;
									if(tUI_CuSetting.tLocation1[CamIdx] >= tUI_CuSetting.tLocation2[CamIdx])
										tUI_CuSetting.tLocation1[CamIdx] = tUI_CuSetting.tLocation2[CamIdx] - 1;
									
									break;
								case ThirdLine:
									tUI_CuSetting.tLocation2[CamIdx] ++;									
									if(tUI_CuSetting.tLocation2[CamIdx] >= 12)
										tUI_CuSetting.tLocation2[CamIdx] = 11;	
									break;
								case WholeArea:
									tUI_CuSetting.tUI_AIDetectlinePoint[CamIdx].tupRight.yPoint += ZoomV_Pixels;
									tUI_CuSetting.tUI_AIDetectlinePoint[CamIdx].tupLeft.yPoint  += ZoomV_Pixels;	
							        tUI_CuSetting.tUI_AIDetectlinePoint[CamIdx].tdownRight.yPoint += ZoomV_Pixels; 
									tUI_CuSetting.tUI_AIDetectlinePoint[CamIdx].tdownLeft.yPoint  += ZoomV_Pixels;	
									break;
							}
							if(tUI_CuSetting.tUI_AIDetectlinePoint[CamIdx].tupLeft.yPoint > tUI_CuSetting.tUI_AIDetectlinePoint[CamIdx].tdownLeft.yPoint)
							{
								tUI_CuSetting.tUI_AIDetectlinePoint[CamIdx].tupLeft.yPoint  -= ZoomV_Pixels;
								tUI_CuSetting.tUI_AIDetectlinePoint[CamIdx].tupRight.yPoint -= ZoomV_Pixels;		
							}
							if(tUI_CuSetting.tUI_AIDetectlinePoint[CamIdx].tdownLeft.yPoint > 596)
							{
								tUI_CuSetting.tUI_AIDetectlinePoint[CamIdx].tdownRight.yPoint  -= ZoomV_Pixels;
								tUI_CuSetting.tUI_AIDetectlinePoint[CamIdx].tdownLeft.yPoint   -= ZoomV_Pixels;		
								if(AILinePointIndex == WholeArea)
								{
									tUI_CuSetting.tUI_AIDetectlinePoint[CamIdx].tupLeft.yPoint -= ZoomV_Pixels;
									tUI_CuSetting.tUI_AIDetectlinePoint[CamIdx].tupRight.yPoint  -= ZoomV_Pixels;
								}
							}

						}
					}
					UI_AISubSubMenuExecute(CamIdx,0);
				}
				
				break;
			case EXIT_ARROW:
				UI_SendBSDRangeTo1126(tUI_CuSetting.tUI_AIDetectlinePoint[CamIdx],CamIdx);
				UI_AISubSubSubMenuExit();
				return;
				break;
		}
		UI_AIDrawSubSubSubMenuItem(TRUE);	
	}
}

//setting的大部分界面的操作逻辑都差不多，所以没有特殊需求的界面都使用同一套遥控解析代码
//------------------------------------------------------------------------------------universal------------------------------------------------------------------------------
/****************record************************/
static UI_SubMenuItem_t tRecordFmtSdMenuItem = 
{
	0,2,{0,0}
};
static UI_SubMenuItem_t tSystemDateTimeMenuItem = 
{
	DATE_TIME_YEAR,DATE_TIME_MAX,{0,0}
};


static UI_SubMenuItem_t *tRecordSetMenuItem[] = 
{
	NULL,
	NULL,
	NULL,
	&tRecordFmtSdMenuItem,
	NULL,
};

static UI_SubMenuItem_t *tSystemSetMenuItem[] = 
{
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	&tSystemDateTimeMenuItem,
};



/****************TRIGGER************************/
static UI_SubMenuItem_t tTriggerCam1DelayMenuItem = 
{
	0,12,{0,0}
};
static UI_SubMenuItem_t tTriggerCam2DelayMenuItem = 
{
	0,12,{0,0}
};
static UI_SubMenuItem_t tTriggerCam3DelayMenuItem = 
{
	0,12,{0,0}
};
static UI_SubMenuItem_t tTriggerCam4DelayMenuItem = 
{
	0,12,{0,0}
};
static UI_SubMenuItem_t tTriggerCamSplitDelayMenuItem = 
{
	0,12,{0,0}
};
static UI_SubMenuItem_t tTriggerCam1PriorityMenuItem = 
{
	0,5,{0,0}
};
static UI_SubMenuItem_t tTriggerCam2PriorityMenuItem = 
{
	0,5,{0,0}
};
static UI_SubMenuItem_t tTriggerCam3PriorityMenuItem = 
{
	0,5,{0,0}
};
static UI_SubMenuItem_t tTriggerCam4PriorityMenuItem = 
{
	0,5,{0,0}
};
static UI_SubMenuItem_t tTriggerCamSplitPriorityMenuItem = 
{
	0,5,{0,0}
};
static UI_SubMenuItem_t tTriggerDisplaySetUpMenuItem = 
{
	0,POWERON_QUAD + 1,{0,0}
};

static UI_SubMenuItem_t *tTriggerMenuItem[] = 
{
	&tTriggerCam1DelayMenuItem,
	&tTriggerCam2DelayMenuItem,
	&tTriggerCam3DelayMenuItem,
	&tTriggerCam4DelayMenuItem,
	&tTriggerCamSplitDelayMenuItem,
	&tTriggerCam1PriorityMenuItem,
	&tTriggerCam2PriorityMenuItem,
	&tTriggerCam3PriorityMenuItem,
	&tTriggerCam4PriorityMenuItem,
	&tTriggerCamSplitPriorityMenuItem,
	&tTriggerDisplaySetUpMenuItem
};

/****************AUTOSCAN************************/
static UI_SubMenuItem_t tAutoScanCam1DelayMenuItem = 
{
	0,12,{0,0}
};
static UI_SubMenuItem_t tAutoScanCam2DelayMenuItem = 
{
	0,12,{0,0}
};
static UI_SubMenuItem_t tAutoScanCam3DelayMenuItem = 
{
	0,12,{0,0}
};
static UI_SubMenuItem_t tAutoScanCam4DelayMenuItem = 
{
	0,12,{0,0}
};
static UI_SubMenuItem_t tAutoScanDualDelayMenuItem = 
{
	0,12,{0,0}
};
static UI_SubMenuItem_t tAutoScanHDelayMenuItem = 
{
	0,12,{0,0}
};
static UI_SubMenuItem_t tAutoScanQuadDelayMenuItem = 
{
	0,12,{0,0}
};
static UI_SubMenuItem_t *tAutoScanMenuItem[] = 
{
	NULL,
	&tAutoScanCam1DelayMenuItem,
	NULL,
	&tAutoScanCam2DelayMenuItem,
	NULL,
	&tAutoScanCam3DelayMenuItem,
	NULL,
	&tAutoScanCam4DelayMenuItem,
	NULL,
	&tAutoScanDualDelayMenuItem,
	NULL,
	&tAutoScanHDelayMenuItem,
	NULL,
	&tAutoScanQuadDelayMenuItem,
};

//----------------------------------------------------------------------
UI_SubMenuItem_t **tSettingSubSubSubMenuItem[] = 
{
	NULL,	//camera
	tRecordSetMenuItem,	//record
	tSystemSetMenuItem, //system
	NULL,	//dual
	NULL,	//quad
	tTriggerMenuItem,	//trigger
	tAutoScanMenuItem,	//auto scan
	NULL,				//power on
	NULL,//parkingLine
};


static uint8_t TempTriggerDelay;
static uint8_t TempAutoScanDelay;

//每次进入相应subsubsub菜单需要更新相应ubItemIdx的初始值
uint8_t ubUI_SubSubSubStsUpdateFlag = FALSE;

//----------------------------------------------------------------------
static void UI_SettingUpdateSubSubSubMenuItemIndex(UI_SettingSubMenuItemList_t tSubMenuItem,uint16_t uwSubSubMenuItemIdx)
{
	if(!ubUI_SubSubSubStsUpdateFlag)
	{	
		memset(&(tSettingSubSubSubMenuItem[tSubMenuItem][uwSubSubMenuItemIdx]->tSubMenuInfo), 0, sizeof(UI_MenuItem_t));
		if(tSubMenuItem == TRIGGERSET_ITEM)
		{
			if(uwSubSubMenuItemIdx <= TRIGGER_DELAY_CAMSPLIT)
			{
				TempTriggerDelay = tUI_CuSetting.TriggerDelay[uwSubSubMenuItemIdx];
			}	
			else if(uwSubSubMenuItemIdx <= TRIGGER_PRIORITY_CAMSPLIT)
			{
				tSettingSubSubSubMenuItem[tSubMenuItem][uwSubSubMenuItemIdx]->tSubMenuInfo.ubItemIdx = 
					tUI_CuSetting.TriggerPriority[uwSubSubMenuItemIdx - 5];
			}
			else
			{
				tSettingSubSubSubMenuItem[tSubMenuItem][uwSubSubMenuItemIdx]->tSubMenuInfo.ubItemIdx = 
					tUI_CuSetting.TriggerSplitView;
			}
				
		}
		else if(tSubMenuItem == AUTOSCANSET_ITEM)
		{
			TempAutoScanDelay = tUI_CuSetting.AutoScanDuty[uwSubSubMenuItemIdx/2];
		}
		ubUI_SubSubSubStsUpdateFlag = TRUE;
	}
}
//------------------------------------------------------------------
static void UI_RecordSubSubSubMenuExit(uint16_t uwSubSubMenuItemIdx)
{
	if(uwSubSubMenuItemIdx == SDCARD_ITEM)
	{
		OSD_IMG_INFO tMenuOsdImgInfo,tOsdImgInfo[18];
		tOSD_GetOsdImgInfor(1, OSD_IMG1, OSD1IMG_SUBMENU2, 1, &tMenuOsdImgInfo);
		tOSD_Img1(&tMenuOsdImgInfo, OSD_QUEUE);
		if(!ubUI_TouchPanelSts)
		{
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_RETURN_BLACK, 1, &tMenuOsdImgInfo);
			tOSD_Img2(&tMenuOsdImgInfo, OSD_QUEUE);
		}
		tOSD_GetOsdImgInfor(1, OSD_IMG2, (tUI_CuSetting.tLanguage == LANGUAGE_ENGLISH)?OSD2IMG_RECORD_TITLE:
										(tUI_CuSetting.tLanguage == LANGUAGE_GERMAN)?OSD2IMG_RECORD_TITLE_GER:
										(tUI_CuSetting.tLanguage == LANGUAGE_FRENCH)?OSD2IMG_RECORD_TITLE_FR:OSD2IMG_RECORD_TITLE_CHN, 5, &tOsdImgInfo[0]);
		//文字
		for(uint8_t i = 0;i < 5;i++)
			tOSD_Img2(&tOsdImgInfo[i], OSD_QUEUE);
		
		//record time
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_RECTIME_1MIN_NOR, 12, &tOsdImgInfo[0]);
		for(uint8_t i = 0;i < 3;i++)
		{
			if(tUI_CuSetting.RecInfo.tREC_Time == i)
				tOSD_Img2(&tOsdImgInfo[2*i + 6], OSD_QUEUE);//高亮图标上的文字
			else
				tOSD_Img2(&tOsdImgInfo[2*i], OSD_QUEUE);
		}
		
		//format sd card
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_FORMATSD_NOR, 4, &tOsdImgInfo[0]);
		if(uwSubSubMenuItemIdx == SDCARD_ITEM)
			tOSD_Img2(&tOsdImgInfo[1], OSD_QUEUE);
		else
			tOSD_Img2(&tOsdImgInfo[0], OSD_QUEUE);

		//power on record
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_POWERON_REC_OFF_NOR, 4, &tOsdImgInfo[0]);
		if(tUI_CuSetting.ubPowerOnRecord)
			tOSD_Img2(&tOsdImgInfo[2], OSD_UPDATE);
		else
			tOSD_Img2(&tOsdImgInfo[0], OSD_UPDATE);
		
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_TIME_STAMP_OFF_NOR, 4, &tOsdImgInfo[0]);
		if(tUI_CuSetting.ubTimeStame)
			tOSD_Img2(&tOsdImgInfo[2], OSD_UPDATE);
		else
			tOSD_Img2(&tOsdImgInfo[0], OSD_UPDATE);

	}
}

//------------------------------------------------------------------
static void UI_TriggerSubSubSubMenuExit(uint16_t uwSubSubMenuItemIdx)
{
		
	OSD_IMG_INFO tOsdImgInfo_Button[22],tOsdImgInfo_Word[9],tMenuOsdImgInfo;
	
	tOSD_GetOsdImgInfor(1, OSD_IMG1, OSD1IMG_SUBMENU2, 1, &tMenuOsdImgInfo);
	tOSD_Img1(&tMenuOsdImgInfo, OSD_QUEUE);
	if(!ubUI_TouchPanelSts)
	{
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_RETURN_BLACK, 1, &tMenuOsdImgInfo);
		tOSD_Img2(&tMenuOsdImgInfo, OSD_QUEUE);
	}

	//word
	tOSD_GetOsdImgInfor(1, OSD_IMG2, (tUI_CuSetting.tLanguage != LANGUAGE_GERMAN)?OSD2IMG_TRIGGER_TITLE:OSD2IMG_TRIGGER_TITLE_GER, 9, &tOsdImgInfo_Word[0]);
	if(tUI_CuSetting.tLanguage == LANGUAGE_FRENCH)
	{
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_TRIGGER_TITLE_FR, 4, &tOsdImgInfo_Word[0]);
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_TRIGGER_SPLIT_WORD_FR, 1, &tOsdImgInfo_Word[8]);
	}
	else if(tUI_CuSetting.tLanguage == LANGUAGE_CHINESE)
	{
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_TRIGGER_TITLE_CHN, 4, &tOsdImgInfo_Word[0]);
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_TRIGGER_SPLIT_WORD_CHN, 1, &tOsdImgInfo_Word[8]);
	}
	for(uint8_t i = 0;i < 9;i++)
		tOSD_Img2(&tOsdImgInfo_Word[i], OSD_QUEUE);

	//button
	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_DATE_TIME_YEAR_NOR, 2, &tOsdImgInfo_Button[0]);
	for(uint8_t i = 0;i < 10;i++)
	{
		tOsdImgInfo_Button[2*i] = tOsdImgInfo_Button[0];
		tOsdImgInfo_Button[2*i + 1] = tOsdImgInfo_Button[1];
		tOsdImgInfo_Button[2*i].uwXStart = tOsdImgInfo_Button[2*i + 1].uwXStart = TRIGGER_X + TRIGGER_X_STEP*(i%5);
		tOsdImgInfo_Button[2*i].uwYStart = tOsdImgInfo_Button[2*i + 1].uwYStart = TRIGGER_Y + TRIGGER_Y_STEP*(i/5);
	}
	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_TRIGGER_DISPLAY_SETUP_NOR,2, &tOsdImgInfo_Button[20]);
//	tOSD_GetOsdImgInfor(1, OSD_IMG2, (tUI_CuSetting.tLanguage == LANGUAGE_ENGLISH)?OSD2IMG_TRIGGER_DISPLAY_SETUP_NOR:
//									(tUI_CuSetting.tLanguage == LANGUAGE_GERMAN)?OSD2IMG_TRIGGER_DISPLAY_SETUP_NOR_GER:
//									(tUI_CuSetting.tLanguage == LANGUAGE_FRENCH)?OSD2IMG_TRIGGER_DISPLAY_SETUP_NOR_FR:OSD2IMG_TRIGGER_DISPLAY_SETUP_NOR_CHN, 2, &tOsdImgInfo_Button[20]);	
	for(uint8_t i = 0;i < 11;i++)
		tOSD_Img2(&tOsdImgInfo_Button[2*i], OSD_QUEUE);
	tOSD_Img2(&tOsdImgInfo_Button[2*uwSubSubMenuItemIdx + UI_ICON_HIGHLIGHT], OSD_QUEUE);
	//value
	for(uint8_t i = 0;i < 5;i++)
	{
		//delay
		UI_ShowButtonValueNormal(tUI_CuSetting.TriggerDelay[i],
			tOsdImgInfo_Button[2*i].uwXStart + ((tUI_CuSetting.TriggerDelay[i] < 10)?40:30),tOsdImgInfo_Button[2*i].uwYStart + 15,OSD_QUEUE);		
		//priority
		UI_ShowButtonValueNormal(tUI_CuSetting.TriggerPriority[i] + 1,
			tOsdImgInfo_Button[10 + 2*i].uwXStart + 40,tOsdImgInfo_Button[10 + 2*i].uwYStart + 15,OSD_QUEUE);		
	}
	//highlight
	uint16_t i = uwSubSubMenuItemIdx;
	if(uwSubSubMenuItemIdx <= TRIGGER_DELAY_CAMSPLIT)
	{
		UI_ShowButtonValueHighLight(tUI_CuSetting.TriggerDelay[i],
			tOsdImgInfo_Button[2*i].uwXStart + ((tUI_CuSetting.TriggerDelay[i] < 10)?40:30),tOsdImgInfo_Button[2*i].uwYStart + 15,OSD_UPDATE);
	}
	else if(uwSubSubMenuItemIdx <= TRIGGER_PRIORITY_CAMSPLIT)
	{
		UI_ShowButtonValueHighLight(tUI_CuSetting.TriggerPriority[i - 5] + 1,
			tOsdImgInfo_Button[2*i].uwXStart + 40,tOsdImgInfo_Button[2*i].uwYStart + 15,OSD_UPDATE);

	}
	else
		tOSD_Img2(&tOsdImgInfo_Button[21], OSD_UPDATE);


}
//------------------------------------------------------------------
static void UI_AutoScanSubSubSubMenuExit(uint16_t uwSubSubMenuItemIdx)
{
	OSD_IMG_INFO tOsdImgInfo[28],tMenuOsdImgInfo;
	uint8_t CamIdx = uwSubSubMenuItemIdx/2;
	tOSD_GetOsdImgInfor(1, OSD_IMG1, OSD1IMG_SUBMENU2, 1, &tMenuOsdImgInfo);
	tOSD_Img1(&tMenuOsdImgInfo, OSD_QUEUE);
	if(!ubUI_TouchPanelSts)
	{
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_RETURN_BLACK, 1, &tMenuOsdImgInfo);
		tOSD_Img2(&tMenuOsdImgInfo, OSD_QUEUE);
	}
	//word
	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_AUTOSCAN_TITLE, 7, &tOsdImgInfo[0]);
	if(tUI_CuSetting.tLanguage == LANGUAGE_GERMAN)
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_AUTOSCAN_CAM1_WORD_GER, 4, &tOsdImgInfo[1]);
	else if(tUI_CuSetting.tLanguage == LANGUAGE_CHINESE)
	{
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_AUTOSCAN_TITLE_CHN, 1, &tOsdImgInfo[0]);
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_AUTOSCAN_DUAL_WORD_CHN, 2, &tOsdImgInfo[5]);
	}
	
	
	for(uint8_t i = 0;i < 7;i++)
		tOSD_Img2(&tOsdImgInfo[i], OSD_QUEUE);

	//delay
	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_DATE_TIME_YEAR_NOR, 2, &tOsdImgInfo[0]);
	for(uint8_t i = 0;i < 6;i++)
	{
		tOsdImgInfo[2*i] = tOsdImgInfo[0];
		tOsdImgInfo[2*i + 1] = tOsdImgInfo[1];
		tOsdImgInfo[2*i].uwXStart = tOsdImgInfo[2*i + 1].uwXStart = AUTOSCAN_DELAY_X + AUTOSCAN_X_STEP*(i%2);
		tOsdImgInfo[2*i].uwYStart = tOsdImgInfo[2*i + 1].uwYStart = AUTOSCAN_DELAY_Y + AUTOSCAN_Y_STEP*(i/2);
	}
	for(uint8_t i = 0;i < 6;i++)
		tOSD_Img2(&tOsdImgInfo[2*i], OSD_QUEUE);

	//value
	for(uint8_t i = 0;i < 6;i++)
	{
		UI_ShowButtonValueNormal(tUI_CuSetting.AutoScanDuty[i],
			tOsdImgInfo[2*i].uwXStart + ((tUI_CuSetting.AutoScanDuty[i] < 10)?40:30),tOsdImgInfo[2*i].uwYStart + 15,OSD_QUEUE);		
	}
	
	//OnOff
	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_AUTOSCAN_OFF_CAM1_NOR, 4, &tOsdImgInfo[0]);
	for(uint8_t i = 0;i < 6;i++)
	{
		for(uint8_t j = 0;j < 4;j++)
		{
			tOsdImgInfo[4*i + j] = tOsdImgInfo[j];
			tOsdImgInfo[4*i + j].uwXStart = AUTOSCAN_SWITCH_X + AUTOSCAN_X_STEP*(i%2);
			tOsdImgInfo[4*i + j].uwYStart = AUTOSCAN_SWITCH_Y + AUTOSCAN_Y_STEP*(i/2);
		}
	}
	for(uint8_t i = 0;i < 6;i++)
	{
		if(!tUI_CuSetting.AutoScanEnable[i])//off
			tOSD_Img2(&tOsdImgInfo[4*i], OSD_QUEUE);
		else
			tOSD_Img2(&tOsdImgInfo[2 + 4*i],OSD_QUEUE);
	}
	//highLight
	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_DATE_TIME_YEAR_NOR, 2, &tOsdImgInfo[0]);
	for(uint8_t i = 0;i < 6;i++)
	{
		tOsdImgInfo[2*i] = tOsdImgInfo[0];
		tOsdImgInfo[2*i + 1] = tOsdImgInfo[1];
		tOsdImgInfo[2*i].uwXStart = tOsdImgInfo[2*i + 1].uwXStart = AUTOSCAN_DELAY_X + AUTOSCAN_X_STEP*(i%2);
		tOsdImgInfo[2*i].uwYStart = tOsdImgInfo[2*i + 1].uwYStart = AUTOSCAN_DELAY_Y + AUTOSCAN_Y_STEP*(i/2);
	}
	tOSD_Img2(&tOsdImgInfo[2*CamIdx + UI_ICON_HIGHLIGHT], OSD_QUEUE);
	UI_ShowButtonValueHighLight(tUI_CuSetting.AutoScanDuty[CamIdx],
		tOsdImgInfo[2*CamIdx].uwXStart + ((tUI_CuSetting.AutoScanDuty[CamIdx] < 10)?40:30),tOsdImgInfo[2*CamIdx].uwYStart + 15,OSD_UPDATE);		


		
}
//------------------------------------------------------------------
static void UI_SettingSubSubSubMenuExit(void)
{
	uint16_t uwSubMenuItemIdx = tUI_SubMenuItem[SETTING_ITEM].tSubMenuInfo.ubItemIdx;//sub
	uint16_t uwSubSubMenuItemIdx = 
		(uint16_t)tSettingSubSubMenuItem.tSettingS[uwSubMenuItemIdx].tSubMenuInfo.ubItemIdx;//sub sub
		
	if(uwSubMenuItemIdx == RECORDSET_ITEM)
	{
		UI_RecordSubSubSubMenuExit(uwSubSubMenuItemIdx);
	}
	else if(uwSubMenuItemIdx == TRIGGERSET_ITEM)
	{
		UI_TriggerSubSubSubMenuExit(uwSubSubMenuItemIdx);
	}
	else if(uwSubMenuItemIdx == AUTOSCANSET_ITEM)
	{	
		UI_AutoScanSubSubSubMenuExit(uwSubSubMenuItemIdx);
	}
	
	ubUI_SubSubSubStsUpdateFlag = FALSE;
	SettingSubSubSubRemoteMode = REMOTE_SELECT_ITEM;
	tUI_State = UI_SUBSUBMENU_STATE;

}
//----------------------------------------------------------------------
static void UI_RecordSubSubSubMenuExecute(uint16_t uwSubSubMenuItemIdx,uint16_t uwsubsubsubItemIdx)
{
	if(uwSubSubMenuItemIdx == SDCARD_ITEM)
	{
		if(uwsubsubsubItemIdx == SDCARD_FORMAT_YES)
		{
			if(UI_REC_START == tUI_RecPlayAct.tRecAct)
				UI_VideoRecordingExec(UI_REC_STOP);
			
			OSD_IMG_INFO tOsdImgInfo[3];
			UI_FuncExecMsg_t tWorkAct;
			uint8_t ubSdFmtRet;
			FS_FMT_STATUS tSDF_Ret;
			
			tWorkAct.uwFunc = UI_SDCARDFMT_ACT;
			osMessagePut(osUI_FuncsExecQue, &tWorkAct, 0);
			osMessageGet(osUI_FuncsFinExecQue, &ubSdFmtRet, osWaitForever);
			tSDF_Ret = (rUI_SUCCESS == ubSdFmtRet)?FORMAT_OK:FORMAT_FAIL;
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_SDCARD_FORMAT_RETURN_BG, 3, &tOsdImgInfo[0]);
			tOSD_Img2(&tOsdImgInfo[0], OSD_QUEUE);
			tOSD_Img2(&tOsdImgInfo[1 + tSDF_Ret], OSD_UPDATE);
			osDelay(1000);
			UI_SettingSubSubSubMenuExit();
		}
		else
		{
			UI_SettingSubSubSubMenuExit();
		}
	}
}
//----------------------------------------------------------------------
static void UI_SystemSubSubSubMenuExecute(uint16_t uwSubSubMenuItemIdx,uint16_t uwsubsubsubItemIdx)
{
	/*if(uwSubSubMenuItemIdx == DATETIME_ITEM)
	{
		OSD_IMG_INFO tOsdImgInfo[25],tMenuOsdImgInfo;
		tOSD_GetOsdImgInfor(1, OSD_IMG1, OSD1IMG_SUBMENU2, 1, &tMenuOsdImgInfo);
        if(uwsubsubsubItemIdx != DATE_TIME_UTC_SELECT)
        {
			tOSD_Img1(&tMenuOsdImgInfo, OSD_QUEUE);
		}
		if(!ubUI_TouchPanelSts)
		{
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_RETURN_BLACK, 1, &tMenuOsdImgInfo);
			tOSD_Img2(&tMenuOsdImgInfo, OSD_QUEUE);
		}
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_KEYBOARD_BACKGROUND, 25, &tOsdImgInfo[0]);
		
		if(uwsubsubsubItemIdx < DATE_TIME_UTC_SELECT)
		{
			tOSD_Img2(&tOsdImgInfo[0], OSD_QUEUE);
			for(uint8_t i = 0;i < 12;i++)
				tOSD_Img2(&tOsdImgInfo[1 + 2*i], OSD_QUEUE);
			tOSD_Img2(&tOsdImgInfo[2], OSD_QUEUE);
		}

		if(uwsubsubsubItemIdx == DATE_TIME_YEAR)
		{
			UI_ShowKeyBoardValue(tUI_CuSetting.tSysCalendar.uwYear,OSD_UPDATE);
		}
		else if((uwsubsubsubItemIdx == DATE_TIME_MONTH))
		{
			UI_ShowKeyBoardValue(tUI_CuSetting.tSysCalendar.ubMonth,OSD_UPDATE);
		}
		else if((uwsubsubsubItemIdx == DATE_TIME_DAY))
		{
			UI_ShowKeyBoardValue(tUI_CuSetting.tSysCalendar.ubDate,OSD_UPDATE);
		}
		else if((uwsubsubsubItemIdx == DATE_TIME_HOUR))
		{
			UI_ShowKeyBoardValue(tUI_CuSetting.tSysCalendar.ubHour,OSD_UPDATE);
		}
		else if((uwsubsubsubItemIdx == DATE_TIME_MINUTE))
		{
			UI_ShowKeyBoardValue(tUI_CuSetting.tSysCalendar.ubMin,OSD_UPDATE);	
		}
		else if((uwsubsubsubItemIdx == DATE_TIME_SECOND))
		{
			UI_ShowKeyBoardValue(tUI_CuSetting.tSysCalendar.ubSec,OSD_UPDATE);
		}
		else if((uwsubsubsubItemIdx == DATE_TIME_UTC_SELECT))
		{
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_TIME_ZONE_SLIDER, 6, &tOsdImgInfo[0]);
            SettingSubSubSubRemoteMode = 1 - SettingSubSubSubRemoteMode;
            if(SettingSubSubSubRemoteMode == REMOTE_SELECT_ITEM)
            	tOSD_Img2(&tOsdImgInfo[4], OSD_QUEUE);
			else
				tOSD_Img2(&tOsdImgInfo[5], OSD_QUEUE);
			tOSD_Img2(&tOsdImgInfo[0], OSD_QUEUE);
			tOsdImgInfo[2].uwXStart = UI_Map(tUI_CuSetting.ubtimezone,0,26,
				tOsdImgInfo[0].uwXStart,tOsdImgInfo[0].uwXStart + tOsdImgInfo[0].uwHSize - tOsdImgInfo[2].uwHSize);
			tOSD_Img2(&tOsdImgInfo[2], OSD_UPDATE);
			
			return;
		}
		else if(uwsubsubsubItemIdx == DATE_TIME_UTC_COMFIRM)
		{
            tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_DATE_TIME_UTC_COMFIRM_NOR, 2, &tOsdImgInfo[0]);
	   	    tOSD_Img2(&tOsdImgInfo[1], OSD_UPDATE);
            UI_UpdateDevStatusInfo();
			SYS_Reboot();
			return;
		}
		ubUI_DateTimeSubSubSubSubStsUpdateFlag = FALSE;
		tUI_State = UI_SUBSUBSUBSUBMENU_STATE;

	}
*/
}
//----------------------------------------------------------------------
static void UI_TriggerSubSubSubSubMenuExecute(uint16_t uwSubSubMenuItemIdx,uint16_t uwsubsubsubItemIdx)
{
	if(uwSubSubMenuItemIdx <= TRIGGER_DELAY_CAMSPLIT)//delay
	{
		if(uwsubsubsubItemIdx <= KEYBOARD_KEY9)
		{
			if((TempTriggerDelay*10 + uwsubsubsubItemIdx + 1) < 60)
				TempTriggerDelay = TempTriggerDelay*10 + uwsubsubsubItemIdx + 1;
			else 
				TempTriggerDelay = 60;
		}
		else if(uwsubsubsubItemIdx == KEYBOARD_KEY0)
		{
			if((TempTriggerDelay*10) <= 60)
				TempTriggerDelay = TempTriggerDelay*10;
			else
				TempTriggerDelay = 60;
		}
		else if(uwsubsubsubItemIdx == KEYBOARD_KEY_ENTER)
		{
			if(TempTriggerDelay > 0)
				tUI_CuSetting.TriggerDelay[uwSubSubMenuItemIdx] = TempTriggerDelay;
			else
				tUI_CuSetting.TriggerDelay[uwSubSubMenuItemIdx] = 1;
			UI_SettingSubSubSubMenuExit();
			return;
		}
		else if(uwsubsubsubItemIdx == KEYBOARD_KEY_DELETE)
			TempTriggerDelay  = TempTriggerDelay / 10;
		
		UI_ShowKeyBoardValue(TempTriggerDelay,OSD_UPDATE);
			
	}
	else if(uwSubSubMenuItemIdx <= TRIGGER_PRIORITY_CAMSPLIT)//priority
	{
		for(uint8_t i = 0;i < 5;i++)
		{
			if(tUI_CuSetting.TriggerPriority[i] == uwsubsubsubItemIdx)
			{
				tUI_CuSetting.TriggerPriority[i] = tUI_CuSetting.TriggerPriority[uwSubSubMenuItemIdx - 5];
				tUI_CuSetting.TriggerPriority[uwSubSubMenuItemIdx - 5] = uwsubsubsubItemIdx;
				UI_SettingSubSubSubMenuExit();
			}
		}
	}
	else//display setup
	{
		tUI_CuSetting.TriggerSplitView = uwsubsubsubItemIdx;
		UI_SettingSubSubSubMenuExit();
	}
}
//----------------------------------------------------------------------
static void UI_AutoScanSubSubSubSubMenuExecute(uint16_t uwSubSubMenuItemIdx,uint16_t uwsubsubsubItemIdx)
{
	uint16_t CamIdx = uwSubSubMenuItemIdx/2;
	if(uwsubsubsubItemIdx <= KEYBOARD_KEY9)
	{
		if((TempAutoScanDelay*10 + uwsubsubsubItemIdx + 1) < 60)
			TempAutoScanDelay = TempAutoScanDelay*10 + uwsubsubsubItemIdx + 1;
		else 
			TempAutoScanDelay = 60;
	}
	else if(uwsubsubsubItemIdx == KEYBOARD_KEY0)
	{
		if((TempAutoScanDelay*10) <= 60)
			TempAutoScanDelay = TempAutoScanDelay*10;
		else
			TempAutoScanDelay = 60;
	}
	else if(uwsubsubsubItemIdx == KEYBOARD_KEY_ENTER)
	{
		if(TempAutoScanDelay > 5)
			tUI_CuSetting.AutoScanDuty[CamIdx] = TempAutoScanDelay;
		else
			tUI_CuSetting.AutoScanDuty[CamIdx] = 5;
			
		UI_SettingSubSubSubMenuExit();
		return;
	}
	else if(uwsubsubsubItemIdx == KEYBOARD_KEY_DELETE)
		TempAutoScanDelay  = TempAutoScanDelay / 10;
	
	UI_ShowKeyBoardValue(TempAutoScanDelay,OSD_UPDATE);

}
//----------------------------------------------------------------------

static void UI_SettingSubSubSubMenuExecute(void)
{
	uint16_t uwSubMenuItem = tUI_SubMenuItem[SETTING_ITEM].tSubMenuInfo.ubItemIdx;//sub
	uint16_t uwSubSubMenuItemIdx = 
		(uint16_t)tSettingSubSubMenuItem.tSettingS[uwSubMenuItem].tSubMenuInfo.ubItemIdx;//sub sub
	uint16_t uwsubsubsubItemIdx = 
		tSettingSubSubSubMenuItem[uwSubMenuItem][uwSubSubMenuItemIdx]->tSubMenuInfo.ubItemIdx;//sub sub sub
	if(uwSubMenuItem == RECORDSET_ITEM)
	{
		UI_RecordSubSubSubMenuExecute(uwSubSubMenuItemIdx,uwsubsubsubItemIdx);
	}
	else if(uwSubMenuItem == SYSTEMSET_ITEM)
	{
		UI_SystemSubSubSubMenuExecute(uwSubSubMenuItemIdx,uwsubsubsubItemIdx);
	}
	else if(uwSubMenuItem == TRIGGERSET_ITEM)
	{
		UI_TriggerSubSubSubSubMenuExecute(uwSubSubMenuItemIdx,uwsubsubsubItemIdx);
	}
	else if(uwSubMenuItem == AUTOSCANSET_ITEM)
	{
		UI_AutoScanSubSubSubSubMenuExecute(uwSubSubMenuItemIdx,uwsubsubsubItemIdx);
	}
	
}
//----------------------------------------------------------------------
static void UI_SettingingDrawSubSubSubMenuItem(uint16_t uwSubMenuItem,uint16_t uwSubSubMenuItemIdx)
{
	uint8_t ubItemPreIdx = tSettingSubSubSubMenuItem[uwSubMenuItem][uwSubSubMenuItemIdx]->tSubMenuInfo.ubItemPreIdx;
	uint8_t ubItemIdx = tSettingSubSubSubMenuItem[uwSubMenuItem][uwSubSubMenuItemIdx]->tSubMenuInfo.ubItemIdx;

	uint16_t uwSettingItemOsdImg[12] = {0};

	if(uwSubMenuItem == SYSTEMSET_ITEM && uwSubSubMenuItemIdx == DATETIME_ITEM)
	{
		OSD_IMG_INFO tOsdImgInfo[12];
		uint16_t uwTemp[6];
		uwTemp[0] = tUI_CuSetting.tSysCalendar.uwYear;
		uwTemp[1] = tUI_CuSetting.tSysCalendar.ubMonth;
		uwTemp[2] = tUI_CuSetting.tSysCalendar.ubDate;
		uwTemp[3] = tUI_CuSetting.tSysCalendar.ubHour;
		uwTemp[4] = tUI_CuSetting.tSysCalendar.ubMin;
		uwTemp[5] = tUI_CuSetting.tSysCalendar.ubSec; 
		//pre
		if(ubItemPreIdx <= DATE_TIME_SECOND)
		{
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_DATE_TIME_YEAR_NOR, 2, &tOsdImgInfo[0]);
			tOsdImgInfo[0].uwXStart += (ubItemPreIdx%3)*DATE_TIME_YEAR_DIST_X;
			tOsdImgInfo[0].uwYStart += (ubItemPreIdx/3)*DATE_TIME_YEAR_DIST_Y;
			tOsdImgInfo[1].uwXStart = tOsdImgInfo[0].uwXStart;
			tOsdImgInfo[1].uwYStart = tOsdImgInfo[0].uwYStart;
			tOSD_Img2(&tOsdImgInfo[0], OSD_QUEUE);
			if(uwTemp[ubItemPreIdx] < 10)
			{
				UI_ShowButtonValueNormal(0,tOsdImgInfo[0].uwXStart + 30,tOsdImgInfo[0].uwYStart + 15,OSD_QUEUE);
				UI_ShowButtonValueNormal(uwTemp[ubItemPreIdx],tOsdImgInfo[0].uwXStart + 50,tOsdImgInfo[0].uwYStart + 15,OSD_QUEUE);
			}
			else if(uwTemp[ubItemPreIdx] < 100)
				UI_ShowButtonValueNormal(uwTemp[ubItemPreIdx],tOsdImgInfo[0].uwXStart + 30,tOsdImgInfo[0].uwYStart + 15,OSD_QUEUE);
			else
				UI_ShowButtonValueNormal(tUI_CuSetting.tSysCalendar.uwYear,tOsdImgInfo[0].uwXStart + 10,tOsdImgInfo[0].uwYStart + 15,OSD_QUEUE);
			
		}
		else if(ubItemPreIdx == DATE_TIME_UTC_SELECT)
		{
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_TIME_ZONE_SLIDER, 6, &tOsdImgInfo[0]);
			if(SettingSubSubSubRemoteMode == REMOTE_SELECT_ITEM)
			{
                if (ubItemIdx != DATE_TIME_UTC_SELECT)
                {
					OSD_EraserImg2_NoUpdate(&tOsdImgInfo[4]);
				}
			}
			else
				tOSD_Img2(&tOsdImgInfo[5], OSD_QUEUE);

			tOSD_Img2(&tOsdImgInfo[0], OSD_QUEUE);

			tOsdImgInfo[2].uwXStart = UI_Map(tUI_CuSetting.ubtimezone,0,26,
				tOsdImgInfo[0].uwXStart,tOsdImgInfo[0].uwXStart + tOsdImgInfo[0].uwHSize - tOsdImgInfo[2].uwHSize);
			tOSD_Img2(&tOsdImgInfo[2], OSD_UPDATE);
			UI_ShowUTCVal();

		}
		else if(ubItemPreIdx == DATE_TIME_UTC_COMFIRM)
		{
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_DATE_TIME_UTC_COMFIRM_NOR, 2, &tOsdImgInfo[0]);
			tOSD_Img2(&tOsdImgInfo[0], OSD_UPDATE);
		}
		//cur
		if(ubItemIdx <= DATE_TIME_SECOND)
		{
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_DATE_TIME_YEAR_NOR, 2, &tOsdImgInfo[0]);
			tOsdImgInfo[0].uwXStart += (ubItemIdx%3)*DATE_TIME_YEAR_DIST_X;
			tOsdImgInfo[0].uwYStart += (ubItemIdx/3)*DATE_TIME_YEAR_DIST_Y;
			tOsdImgInfo[1].uwXStart = tOsdImgInfo[0].uwXStart;
			tOsdImgInfo[1].uwYStart = tOsdImgInfo[0].uwYStart;
			tOSD_Img2(&tOsdImgInfo[1], OSD_QUEUE);
			if(uwTemp[ubItemIdx] < 10)
			{
				UI_ShowButtonValueHighLight(0,tOsdImgInfo[0].uwXStart + 30,tOsdImgInfo[0].uwYStart + 15,OSD_UPDATE);
				UI_ShowButtonValueHighLight(uwTemp[ubItemIdx],tOsdImgInfo[0].uwXStart + 50,tOsdImgInfo[0].uwYStart + 15,OSD_UPDATE);
			}
			else if(uwTemp[ubItemIdx] < 100)
				UI_ShowButtonValueHighLight(uwTemp[ubItemIdx],tOsdImgInfo[0].uwXStart + 30,tOsdImgInfo[0].uwYStart + 15,OSD_UPDATE);
			else
				UI_ShowButtonValueHighLight(tUI_CuSetting.tSysCalendar.uwYear,tOsdImgInfo[0].uwXStart + 10,tOsdImgInfo[0].uwYStart + 15,OSD_UPDATE);

		}
		else if(ubItemIdx == DATE_TIME_UTC_SELECT)
		{
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_TIME_ZONE_SLIDER, 6, &tOsdImgInfo[0]);
			if (SettingSubSubSubRemoteMode == REMOTE_SELECT_ITEM)
				tOSD_Img2(&tOsdImgInfo[4], OSD_QUEUE);
			else
				tOSD_Img2(&tOsdImgInfo[5], OSD_QUEUE);
			tOSD_Img2(&tOsdImgInfo[0], OSD_QUEUE);

			tOsdImgInfo[2].uwXStart = UI_Map(tUI_CuSetting.ubtimezone,0,26,
				tOsdImgInfo[0].uwXStart,tOsdImgInfo[0].uwXStart + tOsdImgInfo[0].uwHSize - tOsdImgInfo[2].uwHSize);
			tOSD_Img2(&tOsdImgInfo[2], OSD_UPDATE);
			UI_ShowUTCVal();
		}
		else if(ubItemIdx == DATE_TIME_UTC_COMFIRM)
		{
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_DATE_TIME_UTC_COMFIRM_NOR, 2, &tOsdImgInfo[0]);
			tOSD_Img2(&tOsdImgInfo[1], OSD_UPDATE);

		}
	}
	else
	{
		if(uwSubMenuItem == RECORDSET_ITEM)
		{
			if(uwSubSubMenuItemIdx == SDCARD_ITEM)
			{
				for(uint8_t i = 0;i < 2;i ++)
					uwSettingItemOsdImg[i] = ((tUI_CuSetting.tLanguage == LANGUAGE_ENGLISH)?OSD2IMG_SDCARD_YES_NOR:
											(tUI_CuSetting.tLanguage == LANGUAGE_GERMAN)?OSD2IMG_SDCARD_YES_NOR_GER:
											(tUI_CuSetting.tLanguage == LANGUAGE_FRENCH)?OSD2IMG_SDCARD_YES_NOR_FR:OSD2IMG_SDCARD_YES_NOR_CHN) + 2*i;
			}
		}
		else if(uwSubMenuItem == TRIGGERSET_ITEM)
		{
			if(uwSubSubMenuItemIdx <= TRIGGER_DELAY_CAMSPLIT)
			{
				for(uint8_t i = 0;i < 12;i ++)
					uwSettingItemOsdImg[i] = OSD2IMG_KEYBOARD_BUTTON1_NOR + 2*i;
			}
			else if(uwSubSubMenuItemIdx <= TRIGGER_PRIORITY_CAMSPLIT)
			{
				for(uint8_t i = 0;i < 5;i ++)
					uwSettingItemOsdImg[i] = OSD2IMG_TRIGGER_PRIORITY1_NOR + 2*i;
			}
			else
			{
				for(uint8_t i = 0;i < POWERON_QUAD + 1;i ++)
					uwSettingItemOsdImg[i] = OSD2IMG_POWERON_CAM1_NOR + 2*i;
			}
		}
		else if(uwSubMenuItem == AUTOSCANSET_ITEM)
		{
			for(uint8_t i = 0;i < 12;i ++)
				uwSettingItemOsdImg[i] = OSD2IMG_KEYBOARD_BUTTON1_NOR + 2*i;
		}

		UI_DrawHLandNormalIcon(uwSettingItemOsdImg[ubItemPreIdx],uwSettingItemOsdImg[ubItemIdx] + UI_ICON_HIGHLIGHT);
	}
	
}


//----------------------------------------------------------------------
void UI_SettingSubSubSubMenuPage(UI_ArrowKey_t tArrowKey)
{
	uint16_t uwSubMenuItem = tUI_SubMenuItem[SETTING_ITEM].tSubMenuInfo.ubItemIdx;//sub
	uint16_t uwSubSubMenuItemIdx = 
		(uint16_t)tSettingSubSubMenuItem.tSettingS[uwSubMenuItem].tSubMenuInfo.ubItemIdx;//sub sub
	UI_SettingUpdateSubSubSubMenuItemIndex(uwSubMenuItem,uwSubSubMenuItemIdx);
	tSettingSubSubSubMenuItem[uwSubMenuItem][uwSubSubMenuItemIdx]->tSubMenuInfo.ubItemPreIdx = 
		tSettingSubSubSubMenuItem[uwSubMenuItem][uwSubSubMenuItemIdx]->tSubMenuInfo.ubItemIdx;

	switch(tArrowKey)
	{
		case LEFT_ARROW:
            if(SettingSubSubSubRemoteMode == REMOTE_SELECT_ITEM)
            {
				if(tSettingSubSubSubMenuItem[uwSubMenuItem][uwSubSubMenuItemIdx]->tSubMenuInfo.ubItemIdx ==
					tSettingSubSubSubMenuItem[uwSubMenuItem][uwSubSubMenuItemIdx]->ubFirstItem)
				{
					tSettingSubSubSubMenuItem[uwSubMenuItem][uwSubSubMenuItemIdx]->tSubMenuInfo.ubItemIdx = 
						tSettingSubSubSubMenuItem[uwSubMenuItem][uwSubSubMenuItemIdx]->ubItemCount - 1;
				}
				else
					tSettingSubSubSubMenuItem[uwSubMenuItem][uwSubSubMenuItemIdx]->tSubMenuInfo.ubItemIdx --;
			}
			else
			{
				if(uwSubMenuItem == RECORDSET_ITEM)
				{
					if (tUI_CuSetting.ubtimezone > 0)
					{
						tUI_CuSetting.ubtimezone--;
					}
					else
					{
						tUI_CuSetting.ubtimezone = 26;
					}
				}
			}
			break;
		case RIGHT_ARROW:
			if(SettingSubSubSubRemoteMode == REMOTE_SELECT_ITEM)
            {
				if(tSettingSubSubSubMenuItem[uwSubMenuItem][uwSubSubMenuItemIdx]->tSubMenuInfo.ubItemIdx == 
					tSettingSubSubSubMenuItem[uwSubMenuItem][uwSubSubMenuItemIdx]->ubItemCount - 1)
				{
					tSettingSubSubSubMenuItem[uwSubMenuItem][uwSubSubMenuItemIdx]->tSubMenuInfo.ubItemIdx = 
						tSettingSubSubSubMenuItem[uwSubMenuItem][uwSubSubMenuItemIdx]->ubFirstItem;
				}
				else
					tSettingSubSubSubMenuItem[uwSubMenuItem][uwSubSubMenuItemIdx]->tSubMenuInfo.ubItemIdx ++;
			}
			else
			{
				if(uwSubMenuItem == RECORDSET_ITEM)
				{
					if (tUI_CuSetting.ubtimezone < 26)
					{
						tUI_CuSetting.ubtimezone++;
					}
					else
					{
						tUI_CuSetting.ubtimezone = 0;
					}
				}
			}
			break;
		case ENTER_ARROW:
			UI_SettingSubSubSubMenuExecute();
			return;
		case EXIT_ARROW:
			UI_SettingSubSubSubMenuExit();
			return;
		default:
			return;
	}
	UI_SettingingDrawSubSubSubMenuItem(uwSubMenuItem,uwSubSubMenuItemIdx);
	
}








/********************************************************************TOUCH*********************************************************************************************/
/********************************************************************TOUCH*********************************************************************************************/
/********************************************************************TOUCH*********************************************************************************************/

void UI_SubSubSubTouchMenu(TOUCH_EVENT_t *Touch_Info)
{
	switch(tUI_MenuItem.ubItemIdx)
	{
		case PLAYBACK_ITEM:
			UI_PlaybackSubSubSubTouchMenu(Touch_Info);
			break;
		case SETTING_ITEM:
			{
				UI_SettingSubMenuItemList_t tSubMenuItem = 
					(UI_SettingSubMenuItemList_t)tUI_SubMenuItem[SETTING_ITEM].tSubMenuInfo.ubItemIdx;//sub
				 switch(tSubMenuItem)
				 {
				 	case CAMERASET_ITEM:
						UI_CameraSubSubSubTouchMenu(Touch_Info);
						break;
					case SYSTEMSET_ITEM:
						UI_SystemSubSubSubTouchMenu(Touch_Info);
						break;
					case PARKINGLINE_ITEM:
						UI_ParkingLIneSubSubSubTouchMenu(Touch_Info);
						break;
					case GUIDELINE_ITEM:
						UI_GuideLineSubSubSubTouchMenu(Touch_Info);
						break;
					case AI_ITEM:
						UI_AISubSubSubTouchMenu(Touch_Info);
						break;					
					default:
						UI_SettingSubSubSubTouchMenu(Touch_Info);
						break;
				 }
					
			}
			break;
		default:
			break;
	}
}

//------------------------------------------------------------------------------playback-----------------------------------------------------------------------
//------------------------------------------------------------------------------playback-----------------------------------------------------------------------
//------------------------------------------------------------------------------playback-----------------------------------------------------------------------

void UI_PlaybackSubSubSubTouchMenu(TOUCH_EVENT_t *Touch_Info)
{

	OSD_IMG_INFO tOsdImgInfo;
	uint16_t i;
	tUI_RecPlayListItem.ubItemPreIdx = tUI_RecPlayListItem.ubItemIdx;
	
	if(UI_SUBSUBSUBMENU_STATE != tUI_State)
		return;

	switch(Touch_Info->Gesture)
	{
		case TOUCH_PRESS:

			//解析手指点击在哪个图标上
			for(i = 0; i < UI_RECPLAYLISTITEM_MAX;i++)//判断具体点击在哪个图标上
			{
				//读取图片信息
				tOSD_GetOsdImgInfor(1, OSD_IMG2, uwRecPlayListImgIdx[i],1, &tOsdImgInfo);
			
				if(Touch_Info->startX > tOsdImgInfo.uwXStart && Touch_Info->startX < tOsdImgInfo.uwXStart + tOsdImgInfo.uwHSize
					&& Touch_Info->startY > tOsdImgInfo.uwYStart && Touch_Info->startY < tOsdImgInfo.uwYStart + tOsdImgInfo.uwVSize)
				{
					tUI_RecPlayListItem.ubItemIdx = i;
					break;
				}
			}
			if(UI_RECPLAYLISTITEM_MAX == i)//没有按到有效的控件
				return;
			
			UI_DrawHLandNormalIcon(uwRecPlayListImgIdx[tUI_RecPlayListItem.ubItemPreIdx], 
				(uwRecPlayListImgIdx[tUI_RecPlayListItem.ubItemIdx] + UI_ICON_HIGHLIGHT));
	
			UI_EnterKey();
			break;
			
		case TOUCH_TURNDOWN:
			break;

		default:

			return;
	}

}


//------------------------------------------------------------------------------setting-----------------------------------------------------------------------
//------------------------------------------------------------------------------setting-----------------------------------------------------------------------
//------------------------------------------------------------------------------setting-----------------------------------------------------------------------

//------------------------------------------------------------------------------------CAMERA------------------------------------------------------------------------------

void UI_CameraSubSubSubTouchMenu(TOUCH_EVENT_t *Touch_Info)
{
	OSD_IMG_INFO tOsdImgInfo;
	static uint8_t ubTouchPressDownCount = 0;//该变量是给TOUCH_PRESSDOWN降速用的
	uint8_t i;
	uint16_t uwSubSubMenuItemIdx = 
		(uint16_t)tSettingSubSubMenuItem.tSettingS[CAMERASET_ITEM].tSubMenuInfo.ubItemIdx;
	
	tCameraSubSubSubMenuItem[uwSubSubMenuItemIdx]->tSubMenuInfo.ubItemPreIdx = 
			tCameraSubSubSubMenuItem[uwSubSubMenuItemIdx]->tSubMenuInfo.ubItemIdx;

	switch(Touch_Info->Gesture)
	{
		case TOUCH_PRESS:
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_CAMSCAM1NOR_ICON,1, &tOsdImgInfo);
			if(Touch_Info->startY < tOsdImgInfo.uwYStart + tOsdImgInfo.uwVSize)//sub sub界面
			{
				for(i = 0;i < 4;i++)
				{
					//读取图片信息
					tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_CAMSCAM1NOR_ICON + 2*i,1, &tOsdImgInfo);
					//解析手指点击在哪个图标上
					if(Touch_Info->startX > tOsdImgInfo.uwXStart 
						&& Touch_Info->startX < tOsdImgInfo.uwXStart + tOsdImgInfo.uwHSize
						&& Touch_Info->startY > tOsdImgInfo.uwYStart 
						&& Touch_Info->startY < tOsdImgInfo.uwYStart + tOsdImgInfo.uwVSize)
					{
						UI_CameraSubSubSubMenuExitToSubSub(i);
						tSettingSubSubMenuItem.tSettingS[CAMERASET_ITEM].tSubMenuInfo.ubItemIdx = i;
						UI_EnterKey();
						break;
					}
				}
			}
			else//sub sub sub界面
			{
				if(tUI_CamStatus[uwSubSubMenuItemIdx].tCamConnSts == CAM_ONLINE)
				{
					for(i = 0;i < 4;i++)
					{
						//读取图片信息
						tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_CAMS_H_MIRROR_OFF_NOR + 4*i,1, &tOsdImgInfo);
						//解析手指点击在哪个图标上
						if(Touch_Info->startX > tOsdImgInfo.uwXStart 
							&& Touch_Info->startX < tOsdImgInfo.uwXStart + tOsdImgInfo.uwHSize
							&& Touch_Info->startY > tOsdImgInfo.uwYStart 
							&& Touch_Info->startY < tOsdImgInfo.uwYStart + tOsdImgInfo.uwVSize)
						{
							tCameraSubSubSubMenuItem[uwSubSubMenuItemIdx]->tSubMenuInfo.ubItemIdx = CAMERA_H_MIRROR + i;
							CameraSubSubSubRemoteMode = REMOTE_SELECT_ITEM;
							break;
						}
					}
					if(i < 4)
					{
						UI_CameraDrawSubSubSubMenuItem();
						//执行mirror或flip的相关操作
						OSD_IMG_INFO tOsdImgInfo[4];
						if(tCameraSubSubSubMenuItem[uwSubSubMenuItemIdx]->tSubMenuInfo.ubItemIdx == CAMERA_H_MIRROR)
							tUI_CamStatus[uwSubSubMenuItemIdx].tCamImgMirror = 1- tUI_CamStatus[uwSubSubMenuItemIdx].tCamImgMirror;
						else if(tCameraSubSubSubMenuItem[uwSubSubMenuItemIdx]->tSubMenuInfo.ubItemIdx == CAMERA_V_MIRROR)
							tUI_CamStatus[uwSubSubMenuItemIdx].tCamImgFlip = 1- tUI_CamStatus[uwSubSubMenuItemIdx].tCamImgFlip;
						else if(tCameraSubSubSubMenuItem[uwSubSubMenuItemIdx]->tSubMenuInfo.ubItemIdx == CAMERA_LASER)
						{
							if(UI_CheckTxVersion(tUI_CamStatus[uwSubSubMenuItemIdx].cCamVersion,30,Laserchar,3))
								tUI_CamStatus[uwSubSubMenuItemIdx].tCamLaser = 1- tUI_CamStatus[uwSubSubMenuItemIdx].tCamLaser;
						}
						else
						{
							if(UI_CheckTxVersion(tUI_CamStatus[uwSubSubMenuItemIdx].cCamVersion,30,Laserchar,3))
								tUI_CamStatus[uwSubSubMenuItemIdx].tCamLed = 1- tUI_CamStatus[uwSubSubMenuItemIdx].tCamLed;
						}
						UI_CameraDrawSubSubSubMenuItem();
						UI_CameraSubSubSubMenuExecute();
						return;
					}
				}
			}
			//return
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_RETURN_NOBACKGROUND_HL, 1, &tOsdImgInfo);	
			if(Touch_Info->startX > tOsdImgInfo.uwXStart 
				&& Touch_Info->startX < tOsdImgInfo.uwXStart + tOsdImgInfo.uwHSize
				&& Touch_Info->startY > tOsdImgInfo.uwYStart 
				&& Touch_Info->startY < tOsdImgInfo.uwYStart + tOsdImgInfo.uwVSize)
			{
				tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
				TIMER_Delay_ms(30);
				UI_CameraSubSubSubMenuExitToSub();
			}
			
			break;
		case TOUCH_PRESSDOWN:
		//滑动条
			if(tUI_CamStatus[uwSubSubMenuItemIdx].tCamConnSts == CAM_ONLINE)
			{
				if(++ ubTouchPressDownCount == 0xff)
					ubTouchPressDownCount = 0;
				if(ubTouchPressDownCount%3 != 0)
					return;
				uint8_t *pUI_ColorParm[5] = {
											(uint8_t *)&tUI_CamStatus[uwSubSubMenuItemIdx].tCamColorParam.ubColorBL,
											(uint8_t *)&tUI_CamStatus[uwSubSubMenuItemIdx].tCamColorParam.ubColorContrast,
											(uint8_t *)&tUI_CamStatus[uwSubSubMenuItemIdx].tCamColorParam.ubColorSaturation,
											(uint8_t *)&tUI_CamStatus[uwSubSubMenuItemIdx].tCamColorParam.ubColorHue,
											(uint8_t *)&tUI_CamStatus[uwSubSubMenuItemIdx].ubVolumeLvl
											 };
	
				for(i = 0;i < 5;i++)
				{
					tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_CAMS_BRIGHTNESS_SLIDER,1,&tOsdImgInfo);
					tOsdImgInfo.uwYStart = CAMERA_SLIDER_Y + CAMERA_Y_STEP*i;
					if(Touch_Info->endX >= tOsdImgInfo.uwXStart - 50 
						&& Touch_Info->endX <= tOsdImgInfo.uwXStart + tOsdImgInfo.uwHSize + 50
						&& Touch_Info->endY >= tOsdImgInfo.uwYStart - 10
						&& Touch_Info->endY <= tOsdImgInfo.uwYStart + tOsdImgInfo.uwVSize + 10)
					{
						if(Touch_Info->endX < tOsdImgInfo.uwXStart)
							Touch_Info->endX = tOsdImgInfo.uwXStart;
						if(Touch_Info->endX > tOsdImgInfo.uwXStart + tOsdImgInfo.uwHSize)
							Touch_Info->endX = tOsdImgInfo.uwXStart + tOsdImgInfo.uwHSize;
	
						tCameraSubSubSubMenuItem[uwSubSubMenuItemIdx]->tSubMenuInfo.ubItemIdx = CAMERA_BRIGHTNESS + i;

						//坐标位置转成进度条的值
						uint8_t ubValue = 
							(uint8_t)UI_Map(Touch_Info->endX,tOsdImgInfo.uwXStart,tOsdImgInfo.uwXStart + tOsdImgInfo.uwHSize,0,99);
	 					uint8_t ubItemIdx = tCameraSubSubSubMenuItem[uwSubSubMenuItemIdx]->tSubMenuInfo.ubItemIdx;
	 					(*pUI_ColorParm[ubItemIdx]) = ubValue;
						
						UI_CameraDrawSubSubSubMenuItem();

						UI_CameraSubSubSubMenuExecute();
						break;
					}
				}
			}
			break;
		default:
	
			return;
	}

}
//------------------------------------------------------------------------------------SYSTEM------------------------------------------------------------------------------
void UI_SystemSubSubSubTouchMenu(TOUCH_EVENT_t *Touch_Info)
{
	OSD_IMG_INFO tOsdImgInfo;
	uint8_t i;
	uint8_t ubSubSubMenuItem = tSystemSubSubMenuItem.tSubMenuInfo.ubItemIdx;
	tSystemSubSubSubMenuItem[ubSubSubMenuItem].tSubMenuInfo.ubItemPreIdx = 
			tSystemSubSubSubMenuItem[ubSubSubMenuItem].tSubMenuInfo.ubItemIdx;
	uint8_t ubMenuItemCount = tSystemSubSubSubMenuItem[ubSubSubMenuItem].ubItemCount;
	switch(Touch_Info->Gesture)
	{
		case TOUCH_PRESS:
			if (ubSubSubMenuItem == UPGRADE_ITEM || ubSubSubMenuItem == LANGUAGE_ITEM || ubSubSubMenuItem == DATETIME_ITEM)
			{
				for (i = 0; i < ubMenuItemCount; i ++)
				{
                    if (ubSubSubMenuItem == DATETIME_ITEM)
                    {
						if(i > DATE_TIME_SECOND)//璺宠繃婊戝姩鏉?
						{						
							i = DATE_TIME_UTC_COMFIRM;
							tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_DATE_TIME_UTC_COMFIRM_NOR, 1, &tOsdImgInfo);
						}
						else
						{
							tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_DATE_TIME_YEAR_NOR, 1, &tOsdImgInfo);
							tOsdImgInfo.uwXStart +=  (i % 3) * DATE_TIME_YEAR_DIST_X;
							if(i > 2)
							{
								tOsdImgInfo.uwYStart += DATE_TIME_YEAR_DIST_Y;
							}
						}
                    }
					else
					{
						//读取图片信息
						tOSD_GetOsdImgInfor(1, OSD_IMG2,((ubSubSubMenuItem == UPGRADE_ITEM)?OSD2IMG_SDCARD_YES_NOR : \
													      OSD2IMG_SYSTEM_ENGLISH_NOR) + 2*i,1, &tOsdImgInfo);
					}


					//解析手指点击在哪个图标上
					if(Touch_Info->startX > tOsdImgInfo.uwXStart 
						&& Touch_Info->startX < tOsdImgInfo.uwXStart + tOsdImgInfo.uwHSize
						&& Touch_Info->startY > tOsdImgInfo.uwYStart 
						&& Touch_Info->startY < tOsdImgInfo.uwYStart + tOsdImgInfo.uwVSize)
					{
						SystemSubSubSubRemoteMode = REMOTE_SELECT_ITEM;
						tSystemSubSubSubMenuItem[ubSubSubMenuItem].tSubMenuInfo.ubItemIdx = i;
						UI_SystemDrawSubSubSubMenuItem(FALSE);
						UI_EnterKey();
						break;
					}
				}
				
			}
			
			//return
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
		case TOUCH_PRESSDOWN:
			if(ubSubSubMenuItem == DATETIME_ITEM)
			{
				tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_TIME_ZONE_SLIDER, 1,&tOsdImgInfo);

				if(Touch_Info->endX >= tOsdImgInfo.uwXStart - 10 
					&& Touch_Info->endX <= tOsdImgInfo.uwXStart + tOsdImgInfo.uwHSize + 10
					&& Touch_Info->endY >= tOsdImgInfo.uwYStart - 10
					&& Touch_Info->endY <= tOsdImgInfo.uwYStart + tOsdImgInfo.uwVSize + 10)
				{
					if(Touch_Info->endX < tOsdImgInfo.uwXStart)
						Touch_Info->endX = tOsdImgInfo.uwXStart;
					if(Touch_Info->endX > tOsdImgInfo.uwXStart + tOsdImgInfo.uwHSize)
						Touch_Info->endX = tOsdImgInfo.uwXStart + tOsdImgInfo.uwHSize;

					uint8_t ubValue = 
						(uint8_t)UI_Map(Touch_Info->endX, tOsdImgInfo.uwXStart, tOsdImgInfo.uwXStart + tOsdImgInfo.uwHSize, 0, 26);

                    tUI_CuSetting.ubtimezone = ubValue;

					tSystemSubSubSubMenuItem[ubSubSubMenuItem].tSubMenuInfo.ubItemIdx = DATE_TIME_UTC_SELECT;
					//tSystemSubSubSubMenuItem[ubSubSubMenuItem][uwSubSubMenuItemIdx]->tSubMenuInfo.ubItemIdx = DATE_TIME_UTC_SELECT;
					UI_SystemDrawSubSubSubMenuItem(FALSE);
				}
			}
		
			if(ubSubSubMenuItem != AUTO_SET_ITEM)
				return;
			for(i = 0;i < 2;i++)
			{
				tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_SYSTEM_DAY_SLIDER + i,1,&tOsdImgInfo);
				if(Touch_Info->endX >= tOsdImgInfo.uwXStart - 50 
					&& Touch_Info->endX <= tOsdImgInfo.uwXStart + tOsdImgInfo.uwHSize + 50
					&& Touch_Info->endY >= tOsdImgInfo.uwYStart - 10
					&& Touch_Info->endY <= tOsdImgInfo.uwYStart + tOsdImgInfo.uwVSize + 10)
				{
					if(Touch_Info->endX < tOsdImgInfo.uwXStart)
						Touch_Info->endX = tOsdImgInfo.uwXStart;
					if(Touch_Info->endX > tOsdImgInfo.uwXStart + tOsdImgInfo.uwHSize)
						Touch_Info->endX = tOsdImgInfo.uwXStart + tOsdImgInfo.uwHSize;

					//坐标位置转成进度条的值
					uint8_t ubValue = 
						(uint8_t)UI_Map(Touch_Info->endX,tOsdImgInfo.uwXStart,tOsdImgInfo.uwXStart + tOsdImgInfo.uwHSize,0,99);
					
					uint8_t ubItemIdx = DAY_SLIDER + i;
					
	 				if(ubItemIdx == DAY_SLIDER)
					{
						if(ubValue >= tUI_CuSetting.ubAutoDimmer_MinVal)
							tUI_CuSetting.ubAutoDimmer_MaxVal = ubValue;
						else 
							return;
					}
					else
					{
						if(ubValue <= tUI_CuSetting.ubAutoDimmer_MaxVal)
							tUI_CuSetting.ubAutoDimmer_MinVal = ubValue;
						else 
							return;
					}
					tSystemSubSubSubMenuItem[ubSubSubMenuItem].tSubMenuInfo.ubItemIdx = DAY_SLIDER + i;
					if(tSystemSubSubSubMenuItem[ubSubSubMenuItem].tSubMenuInfo.ubItemPreIdx != tSystemSubSubSubMenuItem[ubSubSubMenuItem].tSubMenuInfo.ubItemIdx)
						UI_SystemDrawSubSubSubMenuItem(TRUE);
					else
						UI_SystemDrawSubSubSubMenuItem(FALSE);

					break;
				}
			}
		
			break;
		default:
	
			return;
	}

}

//------------------------------------------------------------------------------------parkingLine------------------------------------------------------------------------------

void UI_ParkingLIneSubSubSubTouchMenu(TOUCH_EVENT_t *Touch_Info)
{
	OSD_IMG_INFO tParkingLineOsdImgInfo,tOsdImgInfo;
	static uint8_t ubTouchPressDownCount = 0;//该变量是给TOUCH_PRESSDOWN降速用的
	uint16_t uwSubSubMenuItemIdx = 
		(uint16_t)tSettingSubSubMenuItem.tSettingS[PARKINGLINE_ITEM].tSubMenuInfo.ubItemIdx;
	uint16_t CamIdx = uwSubSubMenuItemIdx/2;	
	uint16_t uwHSize = 0,uwVSize = 0; 

	
	//擦除之前的倒车光标
	UI_EraseParkingLine(CamIdx,OSD_QUEUE);
	
	//获取倒车光标的大小
	tOSD_GetOsdImgInfor(1, OSD_IMG1, OSD1IMG_PARKINGLINE_LEFT, 1, &tParkingLineOsdImgInfo);
	uwHSize += 2*tParkingLineOsdImgInfo.uwHSize;
	uwVSize = tParkingLineOsdImgInfo.uwVSize;
	tOSD_GetOsdImgInfor(1, OSD_IMG1, OSD1IMG_PARKINGLINE_UPPER1 + tUI_CuSetting.ParkingLineSize[CamIdx], 1, &tParkingLineOsdImgInfo);
	uwHSize += tParkingLineOsdImgInfo.uwHSize;
	
	//擦除操作状态
	tOSD_GetOsdImgInfor(1, OSD_IMG2,OSD2IMG_PARKINGLINE_L_R_FLAG, 1, &tOsdImgInfo);
	tOsdImgInfo.uwXStart = (tUI_CuSetting.ParkingLine_XY[CamIdx][0] << 8) + tUI_CuSetting.ParkingLine_XY[CamIdx][1] + (uwHSize - tOsdImgInfo.uwHSize)/2;
	tOsdImgInfo.uwYStart = (tUI_CuSetting.ParkingLine_XY[CamIdx][2] << 8) + tUI_CuSetting.ParkingLine_XY[CamIdx][3] + (uwVSize - tOsdImgInfo.uwVSize)/2;
	OSD_EraserImg2_NoUpdate(&tOsdImgInfo);

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
			return;
		case TOUCH_TURNDOWN:
			break;
		case TOUCH_PRESSDOWN:
			//如果点击在返回图标上就不执行pressDown动作
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_RETURN_NOBACKGROUND_HL, 1, &tOsdImgInfo);	
			if(Touch_Info->startX > tOsdImgInfo.uwXStart 
				&& Touch_Info->startX < tOsdImgInfo.uwXStart + tOsdImgInfo.uwHSize
				&& Touch_Info->startY > tOsdImgInfo.uwYStart 
				&& Touch_Info->startY < tOsdImgInfo.uwYStart + tOsdImgInfo.uwVSize)
			{
				return;
			}
			//降低pressDown速度
			if(++ ubTouchPressDownCount == 0xff)
				ubTouchPressDownCount = 0;
			if(ubTouchPressDownCount%3 != 0)
				return;
			if(Touch_Info->pressPoints == 1)//单指
			{
				//左右
				if((Touch_Info->endX - uwHSize/2 > 0) && (Touch_Info->endX + uwHSize/2 < OSD_WIDTH))
				{
					tUI_CuSetting.ParkingLine_XY[CamIdx][0] = (Touch_Info->endX - uwHSize/2) >> 8;
					tUI_CuSetting.ParkingLine_XY[CamIdx][1] = (uint8_t)(Touch_Info->endX - uwHSize/2);

				}
				else if((Touch_Info->endX - uwHSize/2 < 0))
				{
					tUI_CuSetting.ParkingLine_XY[CamIdx][0] = 0;
					tUI_CuSetting.ParkingLine_XY[CamIdx][1] = 0;

				}
				else if(Touch_Info->endX + uwHSize/2 > OSD_WIDTH)
				{
					tUI_CuSetting.ParkingLine_XY[CamIdx][0] = (OSD_WIDTH - uwHSize) >> 8;
					tUI_CuSetting.ParkingLine_XY[CamIdx][1] = (uint8_t)(OSD_WIDTH - uwHSize);

				}
				//上下
				if((Touch_Info->endY - uwVSize/2 > 0) && (Touch_Info->endY + uwVSize/2 < OSD_HEIGHT))
				{
					tUI_CuSetting.ParkingLine_XY[CamIdx][2] = (Touch_Info->endY - uwVSize/2) >> 8;
					tUI_CuSetting.ParkingLine_XY[CamIdx][3] = (uint8_t)(Touch_Info->endY - uwVSize/2);

				}
				else if((Touch_Info->endY - uwVSize/2 < 0))
				{
					tUI_CuSetting.ParkingLine_XY[CamIdx][2] = 0;
					tUI_CuSetting.ParkingLine_XY[CamIdx][3] = 0;

				}
				else if(Touch_Info->endY + uwVSize/2 > OSD_HEIGHT)
				{
					tUI_CuSetting.ParkingLine_XY[CamIdx][2] = (OSD_HEIGHT - uwVSize) >> 8;
					tUI_CuSetting.ParkingLine_XY[CamIdx][3] = (uint8_t)(OSD_HEIGHT - uwVSize);

				}

			}//双指
			else if(Touch_Info->pressPoints == 2)
			{
				double X[2],Y[2];
				X[0] = Touch_Info->endX;
				X[1] = Touch_Info->readX[1];
				Y[0] = Touch_Info->endY;
				Y[1] = Touch_Info->readY[1];
				uint8_t i;
				for(i = 0;i < 11; i ++)
				{
					uwHSize = 0;
					tOSD_GetOsdImgInfor(1, OSD_IMG1, OSD1IMG_PARKINGLINE_LEFT, 1, &tParkingLineOsdImgInfo);
					uwHSize += 2*tParkingLineOsdImgInfo.uwHSize;
					tOSD_GetOsdImgInfor(1, OSD_IMG1, OSD1IMG_PARKINGLINE_UPPER1 + i, 1, &tParkingLineOsdImgInfo);
					uwHSize += tParkingLineOsdImgInfo.uwHSize;
					if(sqrt((X[0] - X[1])*(X[0] - X[1]) + (Y[0] - Y[1])*(Y[0] - Y[1])) <  uwHSize /2 + 50)
						break;
				}
				tUI_CuSetting.ParkingLineSize[CamIdx] = i;
			}
			break;
		default:

			return;
	}

	UI_ParkingLineDrawSubSubSubMenuItem();
	

}

//------------------------------------------------------------------------------------GuideLine------------------------------------------------------------------------------
void UI_GuideLineSubSubSubTouchMenu(TOUCH_EVENT_t *Touch_Info)
{
	OSD_IMG_INFO tOsdImgInfo[2],tOsdImgInfo_Touch;
	uint16_t uwSubSubMenuItemIdx = 
		(uint16_t)tSettingSubSubMenuItem.tSettingS[GUIDELINE_ITEM].tSubMenuInfo.ubItemIdx;
	uint16_t CamIdx = uwSubSubMenuItemIdx/2;	
	uint16_t uwPreVal_X = (tUI_CuSetting.GuideLine_XY[CamIdx][0] << 8) + tUI_CuSetting.GuideLine_XY[CamIdx][1];
	uint16_t uwPreVal_Y = (tUI_CuSetting.GuideLine_XY[CamIdx][2] << 8) + tUI_CuSetting.GuideLine_XY[CamIdx][3];
	uint16_t uwTempVal;
	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_GUIDELINE_SIZE1 + tUI_CuSetting.GuideLineSize[CamIdx], 1, &tOsdImgInfo[0]);
	//擦除之前的倒车光标和遥控操作状态
	tOsdImgInfo[0].uwXStart = uwPreVal_X;
	tOsdImgInfo[0].uwYStart = uwPreVal_Y;
	OSD_EraserImg2_NoUpdate(&tOsdImgInfo[0]);

	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_PARKINGLINE_L_R_FLAG, 1, &tOsdImgInfo[1]);
	tOsdImgInfo[1].uwXStart = tOsdImgInfo[0].uwXStart + (tOsdImgInfo[0].uwHSize - tOsdImgInfo[1].uwHSize)/2;
	if(tOsdImgInfo[0].uwYStart > tOsdImgInfo[1].uwVSize + 20)
		tOsdImgInfo[1].uwYStart = tOsdImgInfo[0].uwYStart - tOsdImgInfo[1].uwVSize - 20;
	else
		tOsdImgInfo[1].uwYStart = tOsdImgInfo[0].uwYStart + tOsdImgInfo[0].uwVSize + 20;
	OSD_EraserImg2_NoUpdate(&tOsdImgInfo[1]);
	
	switch(Touch_Info->Gesture)
	{
		case TOUCH_PRESS:
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_RETURN_NOBACKGROUND_HL, 1, &tOsdImgInfo_Touch);	
			if(Touch_Info->startX > tOsdImgInfo_Touch.uwXStart 
				&& Touch_Info->startX < tOsdImgInfo_Touch.uwXStart + tOsdImgInfo_Touch.uwHSize
				&& Touch_Info->startY > tOsdImgInfo_Touch.uwYStart 
				&& Touch_Info->startY < tOsdImgInfo_Touch.uwYStart + tOsdImgInfo_Touch.uwVSize)
			{
				tOSD_Img2(&tOsdImgInfo_Touch, OSD_UPDATE);
				TIMER_Delay_ms(30);
				UI_MenuKey();
			}
			return;
		case TOUCH_TURNDOWN:
			break;
		case TOUCH_PRESSDOWN:
			//如果点击在返回图标上就不执行pressDown动作
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_RETURN_NOBACKGROUND_HL, 1, &tOsdImgInfo_Touch);	
			if(Touch_Info->startX > tOsdImgInfo_Touch.uwXStart 
				&& Touch_Info->startX < tOsdImgInfo_Touch.uwXStart + tOsdImgInfo_Touch.uwHSize
				&& Touch_Info->startY > tOsdImgInfo_Touch.uwYStart 
				&& Touch_Info->startY < tOsdImgInfo_Touch.uwYStart + tOsdImgInfo_Touch.uwVSize)
			{
				return;
			}
			printf("$$$$$$$$$$$$$$$$$$$$$Touch_Info->pressPoints is %d******************\n",Touch_Info->pressPoints);
			if(Touch_Info->pressPoints == 1)//单指
			{
				//左右
				if((Touch_Info->endX - tOsdImgInfo[0].uwHSize/2 > 0) && (Touch_Info->endX + tOsdImgInfo[0].uwHSize/2 < OSD_WIDTH))
				{
					tUI_CuSetting.GuideLine_XY[CamIdx][0] = (Touch_Info->endX - tOsdImgInfo[0].uwHSize/2) >> 8;
					tUI_CuSetting.GuideLine_XY[CamIdx][1] = (uint8_t)(Touch_Info->endX - tOsdImgInfo[0].uwHSize/2);

				}
				else if((Touch_Info->endX - tOsdImgInfo[0].uwHSize/2 < 0))
				{
					tUI_CuSetting.GuideLine_XY[CamIdx][0] = 0;
					tUI_CuSetting.GuideLine_XY[CamIdx][1] = 0;

				}
				else if(Touch_Info->endX + tOsdImgInfo[0].uwHSize/2 > OSD_WIDTH)
				{
					tUI_CuSetting.GuideLine_XY[CamIdx][0] = (OSD_WIDTH - tOsdImgInfo[0].uwHSize) >> 8;
					tUI_CuSetting.GuideLine_XY[CamIdx][1] = (uint8_t)(OSD_WIDTH - tOsdImgInfo[0].uwHSize);

				}
				//上下
				if((Touch_Info->endY - tOsdImgInfo[0].uwVSize/2 > 0) && (Touch_Info->endY + tOsdImgInfo[0].uwVSize/2 < OSD_HEIGHT))
				{
					tUI_CuSetting.GuideLine_XY[CamIdx][2] = (Touch_Info->endY - tOsdImgInfo[0].uwVSize/2) >> 8;
					tUI_CuSetting.GuideLine_XY[CamIdx][3] = (uint8_t)(Touch_Info->endY - tOsdImgInfo[0].uwVSize/2);

				}
				else if((Touch_Info->endY - tOsdImgInfo[0].uwVSize/2 < 0))
				{
					tUI_CuSetting.GuideLine_XY[CamIdx][2] = 0;
					tUI_CuSetting.GuideLine_XY[CamIdx][3] = 0;

				}
				else if(Touch_Info->endY + tOsdImgInfo[0].uwVSize/2 > OSD_HEIGHT)
				{
					tUI_CuSetting.GuideLine_XY[CamIdx][2] = (OSD_HEIGHT - tOsdImgInfo[0].uwVSize) >> 8;
					tUI_CuSetting.GuideLine_XY[CamIdx][3] = (uint8_t)(OSD_HEIGHT - tOsdImgInfo[0].uwVSize);

				}

			}//双指
			else if(Touch_Info->pressPoints == 2)
			{
				double X[2],Y[2];
				X[0] = Touch_Info->endX;
				X[1] = Touch_Info->readX[1];
				Y[0] = Touch_Info->endY;
				Y[1] = Touch_Info->readY[1];
				uint16_t i;
				for(i = 0;i < 17;i++)
				{
					tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_GUIDELINE_SIZE1 + i, 1, &tOsdImgInfo_Touch);
					if(sqrt((X[0] - X[1])*(X[0] - X[1]) + (Y[0] - Y[1])*(Y[0] - Y[1])) <  tOsdImgInfo_Touch.uwHSize /2 + 50)
						break;
				}
				tUI_CuSetting.GuideLineSize[CamIdx] = i;
			}
			break;
		default:

			return;
	}

	UI_GuideLineDrawSubSubSubMenuItem();
	

}

//------------------------------------------------------------------------------------AI------------------------------------------------------------------------------
void UI_AISubSubSubTouchMenu(TOUCH_EVENT_t *Touch_Info)
{
	OSD_IMG_INFO tOsdImgInfo,tOsdImgInfo_Touch;
	uint8_t i;
	uint16_t uwSubSubMenuItemIdx = 
		(uint16_t)tSettingSubSubMenuItem.tSettingS[AI_ITEM].tSubMenuInfo.ubItemIdx;
	uint16_t CamIdx = uwSubSubMenuItemIdx%4;	
	uint16_t uwAISubSubSubMenuItemOsdImg[AIITEM_MAX] = {
												OSD2IMG_CAMS_H_MIRROR_OFF_NOR,												
												OSD2IMG_AI_CHOOSE_UP_NOR,
												OSD2IMG_AI_CHOOSE_LEFT_NOR,
												OSD2IMG_AI_CHOOSE_OK_NOR,
												OSD2IMG_AI_CHOOSE_RIGHT_NOR,
												OSD2IMG_AI_CHOOSE_DOWN_NOR,
												OSD2IMG_AI_RESET_NOR,
												};	
	tAISubSubSubMenuItem.tSubMenuInfo.ubItemPreIdx = tAISubSubSubMenuItem.tSubMenuInfo.ubItemIdx;

	switch(Touch_Info->Gesture)
	{
		case TOUCH_PRESS:				
			for(i = 0; i < AIITEM_MAX;i++)//判断具体点击在哪个图标上
			{	

				tOSD_GetOsdImgInfor(1, OSD_IMG2, uwAISubSubSubMenuItemOsdImg[i], 1, &tOsdImgInfo);	
				if(i == BSDRANGE_ENABLE_ITEM)//BSDRANGE寮�鍏冲浘鏍囧鐢℉ Mirror锛岄渶瑕佸崟鐙鐞嗕綅缃?
				{	
					tOsdImgInfo.uwXStart = 30;
					tOsdImgInfo.uwYStart = 20;
				}
				if(Touch_Info->startX > tOsdImgInfo.uwXStart 
						&& Touch_Info->startX < tOsdImgInfo.uwXStart + tOsdImgInfo.uwHSize
						&& Touch_Info->startY > tOsdImgInfo.uwYStart 
						&& Touch_Info->startY < tOsdImgInfo.uwYStart + tOsdImgInfo.uwVSize)
				{
					tAISubSubSubMenuItem.tSubMenuInfo.ubItemIdx = i;
					break;
				}
					
			}

			if(i < AIITEM_MAX)
			{
				UI_EnterKey();
				UI_AIDrawSubSubSubMenuItem(FALSE);
			}
			else
			{
				tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_RETURN_NOBACKGROUND_HL, 1, &tOsdImgInfo_Touch);	
				if(Touch_Info->startX > tOsdImgInfo_Touch.uwXStart 
					&& Touch_Info->startX < tOsdImgInfo_Touch.uwXStart + tOsdImgInfo_Touch.uwHSize
					&& Touch_Info->startY > tOsdImgInfo_Touch.uwYStart 
					&& Touch_Info->startY < tOsdImgInfo_Touch.uwYStart + tOsdImgInfo_Touch.uwVSize)
				{
					tOSD_Img2(&tOsdImgInfo_Touch, OSD_UPDATE);
					TIMER_Delay_ms(30);
					UI_SendBSDRangeTo1126(tUI_CuSetting.tUI_AIDetectlinePoint[CamIdx],CamIdx);
					UI_AISubSubSubMenuExit();
				}		
			}			
				
			return;
		default:

			return;
	}

	//UI_GuideLineDrawSubSubSubMenuItem();
	

}


//setting的大部分界面的操作逻辑都差不多，所以没有特殊需求的界面都使用同一套触摸解析代码
//------------------------------------------------------------------------------------universal------------------------------------------------------------------------------
static uint16_t uwFirstImageIndexOfRecord[] =  //record
{
	0,
	0,
	0,
	OSD2IMG_SDCARD_YES_NOR,
	0,
};

//----------------------------------------------------------------------
static uint16_t uwFirstImageIndexOfTrigger[] =  //trigger
{
	//delay
	OSD2IMG_KEYBOARD_BUTTON1_NOR,
	OSD2IMG_KEYBOARD_BUTTON1_NOR,
	OSD2IMG_KEYBOARD_BUTTON1_NOR,
	OSD2IMG_KEYBOARD_BUTTON1_NOR,
	OSD2IMG_KEYBOARD_BUTTON1_NOR,
	//priority
	OSD2IMG_TRIGGER_PRIORITY1_NOR,
	OSD2IMG_TRIGGER_PRIORITY1_NOR,
	OSD2IMG_TRIGGER_PRIORITY1_NOR,
	OSD2IMG_TRIGGER_PRIORITY1_NOR,
	OSD2IMG_TRIGGER_PRIORITY1_NOR,
	//display setup
	OSD2IMG_SELCAM1NOR_ICON
};
//----------------------------------------------------------------------
static uint16_t uwFirstImageIndexOfAutoScan[] =  //AutoScan
{
	0,//OnOff
	OSD2IMG_KEYBOARD_BUTTON1_NOR,//delay
	0,
	OSD2IMG_KEYBOARD_BUTTON1_NOR,
	0,
	OSD2IMG_KEYBOARD_BUTTON1_NOR,
	0,
	OSD2IMG_KEYBOARD_BUTTON1_NOR,
	0,
	OSD2IMG_KEYBOARD_BUTTON1_NOR,
	0,
	OSD2IMG_KEYBOARD_BUTTON1_NOR,
	0,
	OSD2IMG_KEYBOARD_BUTTON1_NOR,
};
//uwFirstImageIndexOfSubSubSub[][]存的是subsubsub界面的第一张图片
static uint16_t *uwFirstImageIndexOfSubSubSubSetting[] = 
{
	NULL,		//camera
	uwFirstImageIndexOfRecord,//record
	NULL,		//system
	NULL,		//dual
	NULL,		//quad
	uwFirstImageIndexOfTrigger,		//trigger
	uwFirstImageIndexOfAutoScan,	//auto scan
	NULL,							//power on
	NULL,	//parkingLine
};

//----------------------------------------------------------------------

void UI_SettingSubSubSubTouchMenu(TOUCH_EVENT_t *Touch_Info)
{
	UI_SettingSubMenuItemList_t tSubMenuItem = 
		(UI_SettingSubMenuItemList_t)tUI_SubMenuItem[SETTING_ITEM].tSubMenuInfo.ubItemIdx;//sub
	uint16_t uwSubSubMenuItemIdx = 
		(uint16_t)tSettingSubSubMenuItem.tSettingS[tSubMenuItem].tSubMenuInfo.ubItemIdx;//sub sub
	OSD_IMG_INFO tOsdImgInfo;
	uint16_t i = 0;
	
	uint16_t ubSubSubSubMenuItemCount = 
		tSettingSubSubSubMenuItem[tSubMenuItem][uwSubSubMenuItemIdx]->ubItemCount;
	
	UI_SettingUpdateSubSubSubMenuItemIndex(tSubMenuItem,uwSubSubMenuItemIdx);
	
	tSettingSubSubSubMenuItem[tSubMenuItem][uwSubSubMenuItemIdx]->tSubMenuInfo.ubItemPreIdx = 
		tSettingSubSubSubMenuItem[tSubMenuItem][uwSubSubMenuItemIdx]->tSubMenuInfo.ubItemIdx;
	
	switch(Touch_Info->Gesture)
	{
		case TOUCH_PRESS:
			if(uwFirstImageIndexOfSubSubSubSetting[tSubMenuItem][uwSubSubMenuItemIdx] == 0)
				return;
			for(i = 0; i < ubSubSubSubMenuItemCount;i++)//判断具体点击在哪个图标上
			{
				//读取图片信息
				if(tSubMenuItem == RECORDSET_ITEM && uwSubSubMenuItemIdx == DATETIME_ITEM && i > DATE_TIME_SECOND)//跳过滑动条
				{						
					i = DATE_TIME_UTC_COMFIRM;
					tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_DATE_TIME_UTC_COMFIRM_NOR,1, &tOsdImgInfo);
				}
				else
					tOSD_GetOsdImgInfor(1, OSD_IMG2, uwFirstImageIndexOfSubSubSubSetting[tSubMenuItem][uwSubSubMenuItemIdx] + 2*i,1, &tOsdImgInfo);
				
				//解析手指点击在哪个图标上
				if(Touch_Info->startX > tOsdImgInfo.uwXStart 
					&& Touch_Info->startX < tOsdImgInfo.uwXStart + tOsdImgInfo.uwHSize
					&& Touch_Info->startY > tOsdImgInfo.uwYStart 
					&& Touch_Info->startY < tOsdImgInfo.uwYStart + tOsdImgInfo.uwVSize)
				{
					tSettingSubSubSubMenuItem[tSubMenuItem][uwSubSubMenuItemIdx]->tSubMenuInfo.ubItemIdx = i;
					SettingSubSubSubRemoteMode = REMOTE_SELECT_ITEM;
					break;
				}
			}
			if(i < ubSubSubSubMenuItemCount)
			{
				if(tSettingSubSubSubMenuItem[tSubMenuItem][uwSubSubMenuItemIdx]->tSubMenuInfo.ubItemPreIdx != 
							tSettingSubSubSubMenuItem[tSubMenuItem][uwSubSubMenuItemIdx]->tSubMenuInfo.ubItemIdx)
				{
					UI_SettingingDrawSubSubSubMenuItem(tSubMenuItem,uwSubSubMenuItemIdx);
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
		case TOUCH_PRESSDOWN:
			if(tSubMenuItem == RECORDSET_ITEM && uwSubSubMenuItemIdx == DATETIME_ITEM)
			{
				tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_TIME_ZONE_SLIDER, 1,&tOsdImgInfo);

				if(Touch_Info->endX >= tOsdImgInfo.uwXStart - 10 
					&& Touch_Info->endX <= tOsdImgInfo.uwXStart + tOsdImgInfo.uwHSize + 10
					&& Touch_Info->endY >= tOsdImgInfo.uwYStart - 10
					&& Touch_Info->endY <= tOsdImgInfo.uwYStart + tOsdImgInfo.uwVSize + 10)
				{
					if(Touch_Info->endX < tOsdImgInfo.uwXStart)
						Touch_Info->endX = tOsdImgInfo.uwXStart;
					if(Touch_Info->endX > tOsdImgInfo.uwXStart + tOsdImgInfo.uwHSize)
						Touch_Info->endX = tOsdImgInfo.uwXStart + tOsdImgInfo.uwHSize;
                    

					uint8_t ubValue = 
						(uint8_t)UI_Map(Touch_Info->endX, tOsdImgInfo.uwXStart, tOsdImgInfo.uwXStart + tOsdImgInfo.uwHSize, 0, 26);

                    tUI_CuSetting.ubtimezone = ubValue;

					tSettingSubSubSubMenuItem[tSubMenuItem][uwSubSubMenuItemIdx]->tSubMenuInfo.ubItemIdx = DATE_TIME_UTC_SELECT;
					UI_SettingingDrawSubSubSubMenuItem(RECORDSET_ITEM, DATETIME_ITEM);
				}
			}
			break;
		default:

			return;
	}

}

#endif

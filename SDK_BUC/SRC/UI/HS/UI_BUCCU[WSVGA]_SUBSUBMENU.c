
#include "UI_BUCCU[WSVGA]_SUBSUBMENU.h"
#ifdef BSP_D_SNCC71_GM8285C_RX_V2

void UI_ParkingLineSubSubTouchMenuPage(TOUCH_EVENT_t *Touch_Info);

void UI_SettingSubSubMenuPage(UI_ArrowKey_t tArrowKey);
void UI_SettingSubSubTouchMenuPage(TOUCH_EVENT_t *Touch_Info);

void UI_AutoScanSubSubTouchMenuPage(TOUCH_EVENT_t *Touch_Info);

void UI_SystemSubSubMenuPage(UI_ArrowKey_t tArrowKey);
void UI_SystemSubSubTouchMenuPage(TOUCH_EVENT_t *Touch_Info);

void UI_RecordSubSubTouchMenuPage(TOUCH_EVENT_t *Touch_Info);

void UI_DualSubSubMenuPage(UI_ArrowKey_t tArrowKey);
void UI_DualSubSubTouchMenuPage(TOUCH_EVENT_t *Touch_Info);
void UI_QuadSubSubMenuPage(UI_ArrowKey_t tArrowKey);
void UI_QuadSubSubTouchMenuPage(TOUCH_EVENT_t *Touch_Info);
void UI_PowerOnSubSubMenuPage(UI_ArrowKey_t tArrowKey);
void UI_PowerOnSubSubTouchMenuPage(TOUCH_EVENT_t *Touch_Info);

void UI_AISubSubMenuPage(UI_ArrowKey_t tArrowKey);
void UI_AISubSubMenuExit(void);
void UI_AISubSubTouchMenuPage(TOUCH_EVENT_t *Touch_Info);
void UI_AISubSubMenuExecute(uint8_t ubItemCurIdx,uint8_t Switch_FLAG);


//------------------------------------------------------------------------------
UI_MenuAct_t UI_KeyEventMap2SubSubMenuInfo(UI_ArrowKey_t *ptArrowKey, UI_SubMenuItem_t *ptSubSubMenuItem)
{
	uint8_t ubCurrentSubSubMenuItemIdx = 0;
	uint8_t ubNextSubSubMenuItemIdx = 0;

	ubCurrentSubSubMenuItemIdx = ptSubSubMenuItem->tSubMenuInfo.ubItemIdx;
	switch(*ptArrowKey)
	{
		case LEFT_ARROW:
			if(ubCurrentSubSubMenuItemIdx <= ptSubSubMenuItem->ubFirstItem)
				ubNextSubSubMenuItemIdx = ptSubSubMenuItem->ubItemCount -1;
			else
				ubNextSubSubMenuItemIdx = ubCurrentSubSubMenuItemIdx - 1;
			break;
		case RIGHT_ARROW:
			if((ubCurrentSubSubMenuItemIdx+1) >= ptSubSubMenuItem->ubItemCount)
				ubNextSubSubMenuItemIdx = ptSubSubMenuItem->ubFirstItem;
			else
				ubNextSubSubMenuItemIdx = ubCurrentSubSubMenuItemIdx + 1;
			break;
		case ENTER_ARROW:
			return EXECUTE_MENUFUNC;
		case EXIT_ARROW:
			return EXIT_MENUFUNC;
		case VERSION_ARROW:
			OSD_ImagePrintf(OSD_IMG_ROTATION_0, 420, 250, tUI_CharOsdImgInfo, OSD_QUEUE, "AI VERSON!!");		
			return NOT_ACTION;
		default:
			return NOT_ACTION;
	}
	ptSubSubMenuItem->tSubMenuInfo.ubItemPreIdx = ubCurrentSubSubMenuItemIdx;
	ptSubSubMenuItem->tSubMenuInfo.ubItemIdx    = ubNextSubSubMenuItemIdx;
	return DRAW_HIGHLIGHT_MENUICON;
}
//------------------------------------------------------------------------------

void UI_SubSubKeyMenu(UI_ArrowKey_t tArrowKey)
{
	switch(tUI_MenuItem.ubItemIdx)
	{	
		case PLAYBACK_ITEM:
			UI_PlaybackSubSubMenuPage(tArrowKey);
			break;
		case SETTING_ITEM:
			{
				UI_SettingSubMenuItemList_t tSubMenuItem = 
						(UI_SettingSubMenuItemList_t)tUI_SubMenuItem[SETTING_ITEM].tSubMenuInfo.ubItemIdx;
				switch(tSubMenuItem)
				{
					case SYSTEMSET_ITEM:
						UI_SystemSubSubMenuPage(tArrowKey);
						break;
					case DUALSET_ITEM:
						UI_DualSubSubMenuPage(tArrowKey);
						break;
					case QUADSET_ITEM:
						UI_QuadSubSubMenuPage(tArrowKey);
						break;
					case POWERONSET_ITEM:
						UI_PowerOnSubSubMenuPage(tArrowKey);
						break;
					case AI_ITEM:
						UI_AISubSubMenuPage(tArrowKey);
						break;
					default:
						UI_SettingSubSubMenuPage(tArrowKey);
					break;

				}

			}
			
			break;
		default:
			break;
	}
}


//------------------------------------------------------------------------------playback-------------------------------------------------------------------------------------
//------------------------------------------------------------------------------playback-------------------------------------------------------------------------------------
//------------------------------------------------------------------------------playback-------------------------------------------------------------------------------------
void UI_StopPlayRecordFile(uint8_t ubPlayRet)
{
	KNL_RecordAct_t tUI_PlayAct = {KNL_RECORDFUNC_DISABLE, NULL,};
	switch(ubPlayRet)
	{
		case KNL_VDOPLAY_STOP:
            if(tUI_RecPlayAct.tPlaySts != UI_RECFILE_STOP)
            {
    			tUI_RecPlayAct.tPlaySts = UI_RECFILE_STOP;
    			UI_PlaybackSubSubSubMenuPage(DOWN_ARROW);
            }
			break;
		default:
			if(UI_RECFILE_PLAY == tUI_RecPlayAct.tPlaySts)
			{
				printd(DBG_ErrorLvl, "Video Play Err !\n");
				tUI_PlayAct.tRecordFunc = KNL_RECORDFUNC_DISABLE;
                tUI_PlayAct.pRecordStsNtyCb = NULL;
    			tKNL_ExecRecordFunc(tUI_PlayAct);
				UI_PlaybackSubSubSubMenuPage(EXIT_ARROW);
			}
			else if((UI_RECPLAYDISPTYPE_SEL_STATE == tUI_State) || (UI_RECFILES_SEL_STATE == tUI_State))
			{
				OSD_IMG_INFO tRecPlayOsdImgInfo;

				tUI_PlayAct.tRecordFunc = KNL_RECORDFUNC_DISABLE;
                tUI_PlayAct.pRecordStsNtyCb = NULL;
    			tKNL_ExecRecordFunc(tUI_PlayAct);
				tRecPlayOsdImgInfo.uwXStart = 50;
				tRecPlayOsdImgInfo.uwYStart = 150;
				tRecPlayOsdImgInfo.uwHSize  = 892;
				tRecPlayOsdImgInfo.uwVSize  = 435;
				OSD_EraserImg2(&tRecPlayOsdImgInfo);
				UI_DrawRecordFileMenu();
			}
			break;
	}
}
//------------------------------------------------------------------------------
void UI_DrawPlayBackIcon(KNL_RecordAct_t tUI_PlayAct)
{
	OSD_IMG_INFO tOsdImgInfo[4],tVoiceOsdImgInfo;
	KNL_DISP_LOCATION tDispLocate;
	uint16_t uwCam1WordOsdIndex = (tUI_CuSetting.tLanguage != LANGUAGE_GERMAN)?OSD2IMG_DESKTOP_CAM1_WORD:OSD2IMG_DESKTOP_CAM1_WORD_GER;
	
	if(tUI_PlayAct.tPlayDispTye == KNL_DISP_DUAL_U)
	{
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_DESKTOP_LINE_DUAL, 1, &tOsdImgInfo[0]);//line
		tOSD_Img2(&tOsdImgInfo[0], OSD_QUEUE);
		
		tOSD_GetOsdImgInfor(1, OSD_IMG2, uwCam1WordOsdIndex, 4, &tOsdImgInfo[0]);//word
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_DESKTOP_VOICE_MUTE + 1 - tUI_CuSetting.ubMuteFlag, 1, &tVoiceOsdImgInfo);//voice
		for(tDispLocate = KNL_DISP_LOCATION1;tDispLocate <= KNL_DISP_LOCATION2;tDispLocate ++)
		{
			UI_CamNum_t tCamNum = tUI_RecFilesInfo.tRecFilesInfo[tUI_PlayAct.ulVideoPlayIdx[0]].HidnFileInfo.SubHidnInfo.ubSrcLocate[tDispLocate];
			tOsdImgInfo[tCamNum].uwYStart = 15;
			if(tDispLocate == KNL_DISP_LOCATION1)
				tOsdImgInfo[tCamNum].uwXStart -= OSD_WIDTH/4;
			else
				tOsdImgInfo[tCamNum].uwXStart += OSD_WIDTH/4;
			tOSD_Img2(&tOsdImgInfo[tCamNum], OSD_QUEUE);

			if(tCamNum == tUI_RecPlayAct.tAdoPlayCam)
			{
				tVoiceOsdImgInfo.uwYStart = 5;
				if(tDispLocate == KNL_DISP_LOCATION2)
					tVoiceOsdImgInfo.uwXStart += OSD_WIDTH/2;
				tOSD_Img2(&tVoiceOsdImgInfo, OSD_QUEUE);
			}
		}
	}
	else if(tUI_PlayAct.tPlayDispTye == KNL_DISP_QUAD)
	{
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_DESKTOP_LINE_QUAD_H, 2, &tOsdImgInfo[0]);
		tOSD_Img2(&tOsdImgInfo[0], OSD_QUEUE);
		tOSD_Img2(&tOsdImgInfo[1], OSD_QUEUE);

		tOSD_GetOsdImgInfor(1, OSD_IMG2, uwCam1WordOsdIndex, 4, &tOsdImgInfo[0]);
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_DESKTOP_VOICE_MUTE + 1 - tUI_CuSetting.ubMuteFlag, 1, &tVoiceOsdImgInfo);//voice
		for(tDispLocate = KNL_DISP_LOCATION1;tDispLocate <= KNL_DISP_LOCATION4;tDispLocate ++)
		{
			UI_CamNum_t tCamNum = tUI_RecFilesInfo.tRecFilesInfo[tUI_PlayAct.ulVideoPlayIdx[0]].HidnFileInfo.SubHidnInfo.ubSrcLocate[tDispLocate];
			if(tDispLocate == KNL_DISP_LOCATION1)
			{
				tOsdImgInfo[tCamNum].uwYStart = 15;
				tOsdImgInfo[tCamNum].uwXStart -= OSD_WIDTH/4;
			}					
			else if(tDispLocate == KNL_DISP_LOCATION2)
			{
				tOsdImgInfo[tCamNum].uwYStart = 15;
				tOsdImgInfo[tCamNum].uwXStart += OSD_WIDTH/4;
			}
			else if(tDispLocate == KNL_DISP_LOCATION3)
			{
				tOsdImgInfo[tCamNum].uwYStart = 15 + OSD_HEIGHT/2;
				tOsdImgInfo[tCamNum].uwXStart -= OSD_WIDTH/4;
			}
			else
			{
				tOsdImgInfo[tCamNum].uwYStart = 15 + OSD_HEIGHT/2;
				tOsdImgInfo[tCamNum].uwXStart += OSD_WIDTH/4;
			}
			tOSD_Img2(&tOsdImgInfo[tCamNum], OSD_QUEUE);

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

}
//------------------------------------------------------------------------------

void UI_StartPlayRecordFile(UI_RecPlayDispType_t tPlayDispType)
{
	OSD_IMG_INFO tRecPlayOsdImgInfo[12],tOsdImgInfo[4],tVoiceOsdImgInfo;
	KNL_RecordAct_t tUI_PlayAct = {KNL_RECORDFUNC_DISABLE, NULL,};
	KNL_ROLE tRecRoleNum;
	UI_CamNum_t tRecCamNum;
	uint16_t uwRecFileIndex, uwIdx;
	uint8_t ubImgIdx = 0, ubPlayIdx = 0;

	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_REC_ADOSRCSELNOR_ICON, 12, &tRecPlayOsdImgInfo[0]);

	tUI_RecPlayAct.ubPlayMode = tPlayDispType;
	if(UI_SUBSUBSUBMENU_STATE == tUI_State)
	{
		tUI_RecPlayAct.tPlaySts = UI_RECFILE_PLAY;
		ubPLY_Jump(PLY_JUMP_RESTART);
		KNL_ResetLcdChannel();
		tOSD_Img2(&tRecPlayOsdImgInfo[5], OSD_UPDATE);
	}
	else
	{
		uwRecFileIndex 				= tUI_RecFilesInfo.uwRecFileSelIdx;
		tUI_PlayAct.tRecordFunc  	= KNL_VIDEO_PLAY;
		tUI_PlayAct.pRecordStsNtyCb = UI_StopPlayRecordFile;
		if(UI_RECPLAY_SINGLEVIEW == tPlayDispType)
		{
			tUI_PlayAct.tPlayDispTye = KNL_DISP_SINGLE;
			tUI_PlayAct.ulVideoPlayIdx[0] = uwRecFileIndex;
			tUI_PlayAct.ubPlayFileNum = 1;
			tRecRoleNum = VDO_KNLSrcNumMap2KNLRoleNum((KNL_SRC)tUI_RecFilesInfo.tRecFilesInfo[uwRecFileIndex].HidnFileInfo.SrcNum);
			APP_KNLRoleMap2CamNum(tRecRoleNum, tRecCamNum);
			ubPLY_AdoChannelSet(tRecCamNum);
		}
		else
		{
			tUI_RecPlayListItem.ubItemIdx = UI_RECPLAYPAUSE_ITEM;
			tUI_PlayAct.tPlayDispTye = (KNL_DISP_TYPE)tUI_RecFilesInfo.tRecFilesInfo[uwRecFileIndex].HidnFileInfo.SubHidnInfo.ubPreviewMode;
			for(tRecCamNum = CAM1; tRecCamNum <= CAM4; tRecCamNum++)
				tUI_RecPlayAct.tVdoPlayCam[tRecCamNum] = NO_CAM;
			
			tRecRoleNum = VDO_KNLSrcNumMap2KNLRoleNum((KNL_SRC)tUI_RecFilesInfo.tRecFilesInfo[uwRecFileIndex].HidnFileInfo.SrcNum);
			if(KNL_NONE != tRecRoleNum)
			{
				APP_KNLRoleMap2CamNum(tRecRoleNum, tRecCamNum);
				tUI_RecPlayAct.tVdoPlayCam[tRecCamNum] = tRecCamNum;
				tUI_RecPlayAct.tAdoPlayCam = tRecCamNum;
				printf("---------tUI_RecPlayAct.tAdoPlayCam = %d\n",tUI_RecPlayAct.tAdoPlayCam);
				ubPLY_AdoChannelSet(tUI_RecPlayAct.tAdoPlayCam);
			}
			tUI_PlayAct.ubPlayFileNum = 0;
			for(uwIdx = 0; uwIdx < tUI_RecFilesInfo.uwTotalRecFileNum; uwIdx++)
			{
				if(tUI_PlayAct.ubPlayFileNum)
				{
					 if(++ubPlayIdx > 4)
						break;
				}
				if(tUI_RecFilesInfo.tRecFilesInfo[uwIdx].HidnFileInfo.uwGroupIdx == tUI_RecFilesInfo.tRecFilesInfo[uwRecFileIndex].HidnFileInfo.uwGroupIdx)
				{
					tRecRoleNum = VDO_KNLSrcNumMap2KNLRoleNum((KNL_SRC)tUI_RecFilesInfo.tRecFilesInfo[uwIdx].HidnFileInfo.SrcNum);
					if(KNL_NONE != tRecRoleNum)
					{
						APP_KNLRoleMap2CamNum(tRecRoleNum, tRecCamNum);
						tUI_RecPlayAct.tVdoPlayCam[tRecCamNum] = tRecCamNum;
					}
					tUI_PlayAct.ulVideoPlayIdx[tUI_PlayAct.ubPlayFileNum++] = uwIdx;
					uwCurVideoPlayIdx[tUI_PlayAct.ubPlayFileNum - 1] = uwIdx;
				}
			}
		}
		if(KNL_OK == tKNL_ExecRecordFunc(tUI_PlayAct))
		{
			tUI_RecPlayAct.tPlaySts = UI_RECFILE_PLAY;
			tUI_State = UI_SUBSUBSUBMENU_STATE;
			UI_ClearOsdImageNoUpdate();
			UI_DrawPlayBackIcon(tUI_PlayAct);
			//播放控制按键
			for(ubImgIdx = 0; ubImgIdx < 6; ubImgIdx ++)
				tOSD_Img2(&tRecPlayOsdImgInfo[2*ubImgIdx], OSD_QUEUE);
			tOSD_Img2(&tRecPlayOsdImgInfo[5], OSD_UPDATE);

			
		}
		else
			tUI_RecPlayAct.tPlaySts = UI_RECFILE_STOP;
	}
}
//------------------------------------------------------------------------------
static void UI_PlaybackDrawSubSubMenuItemFocus(uint16_t uwPreIdx,uint16_t uwIdx)
{
	OSD_IMG_INFO tFileSelOsdImgInfo[4];
	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_FILE_FILEBOX_LEFT, 4, &tFileSelOsdImgInfo[0]);
	for(uint8_t i = 0; i < 4; i++)
	{
		tFileSelOsdImgInfo[i].uwYStart += ((uwPreIdx % REC_FILE_LIST_MAXNUM) * 50);
		OSD_EraserImg2_NoUpdate(&tFileSelOsdImgInfo[i]);
	}
	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_FILE_FILEBOX_LEFT, 4, &tFileSelOsdImgInfo[0]);
	for(uint8_t i = 0; i < 4; i++)
	{
		tFileSelOsdImgInfo[i].uwYStart += ((uwIdx % REC_FILE_LIST_MAXNUM) * 50);
		tOSD_Img2(&tFileSelOsdImgInfo[i], (i == 3)?OSD_UPDATE:OSD_QUEUE);
	}

}
//------------------------------------------------------------------------------


void UI_PlayDispTypeSelection(UI_ArrowKey_t tArrowKey)
{
	uint16_t uwRecPlayDispTypeImgIdx[2] = {
											OSD2IMG_RECFILEMULTIVIEWNOR_ICON, 
											OSD2IMG_RECFILESIGNLEVIEWNOR_ICON
											};
	uint16_t uwFileStartIdx,uwFileEndIdx;
	uint8_t ubUI_RecFileIdx,ubUI_PrevRecFileIdx;
	tUI_PlyDispTypeSelItem.ubItemPreIdx = tUI_PlyDispTypeSelItem.ubItemIdx;

	switch(tArrowKey)
	{
		case LEFT_ARROW:
			if(UI_RECPLAY_MULTIVIEW == tUI_PlyDispTypeSelItem.ubItemIdx)
				tUI_PlyDispTypeSelItem.ubItemIdx = UI_RECPLAY_SINGLEVIEW;
			else
				tUI_PlyDispTypeSelItem.ubItemIdx  = UI_RECPLAY_MULTIVIEW;
			break;
		case RIGHT_ARROW:
			if(UI_RECPLAY_SINGLEVIEW == tUI_PlyDispTypeSelItem.ubItemIdx)
				tUI_PlyDispTypeSelItem.ubItemIdx = UI_RECPLAY_MULTIVIEW;
			else
				tUI_PlyDispTypeSelItem.ubItemIdx  = UI_RECPLAY_SINGLEVIEW;
			break;
		case ENTER_ARROW:
			UI_StartPlayRecordFile(tUI_PlyDispTypeSelItem.ubItemIdx);
			tUI_PlyDispTypeSelItem.ubItemIdx = UI_RECPLAY_MULTIVIEW;
			return;
		case EXIT_ARROW:
		{
			uwFileStartIdx = (tUI_RecFilesInfo.uwRecFileSelIdx / REC_FILE_LIST_MAXNUM) * REC_FILE_LIST_MAXNUM;
			uwFileEndIdx   = tUI_RecFilesInfo.uwTotalRecFileNum - tUI_RecFilesInfo.uwRecFileSelIdx;
			uwFileEndIdx   = (uwFileEndIdx < REC_FILE_LIST_MAXNUM)?tUI_RecFilesInfo.uwTotalRecFileNum:(uwFileStartIdx + REC_FILE_LIST_MAXNUM);
			UI_ListRecFileInfo(uwFileStartIdx, uwFileEndIdx, OSD_QUEUE);
			
			ubUI_RecFileIdx = (tUI_RecFilesInfo.uwRecFileSelIdx % REC_FILE_LIST_MAXNUM);
			ubUI_PrevRecFileIdx = ubUI_RecFileIdx;
			UI_PlaybackDrawSubSubMenuItemFocus(ubUI_PrevRecFileIdx,ubUI_RecFileIdx);
			
			tUI_PlyDispTypeSelItem.ubItemIdx = UI_RECPLAY_MULTIVIEW;
			tUI_State = UI_SUBSUBMENU_STATE;
			return;
		}
		default:
			return;
	}
	UI_DrawHLandNormalIcon(uwRecPlayDispTypeImgIdx[tUI_PlyDispTypeSelItem.ubItemPreIdx], 
		(uwRecPlayDispTypeImgIdx[tUI_PlyDispTypeSelItem.ubItemIdx] + UI_ICON_HIGHLIGHT));
}
//------------------------------------------------------------------------------
void UI_PlayRecordFile(uint16_t uwRecFileIndex)
{
    if(ubPLY_GetOpMode() == PLY_MODE_R)
    {
        printd(DBG_ErrorLvl, "(Now Recording) \n");
		return;
    }
	if(!memcmp(tUI_RecFilesInfo.tRecFilesInfo[uwRecFileIndex].HidnFileInfo.FileName.chExt, "MP4", 3))
	{
		if(KNL_DISP_SINGLE == tUI_RecFilesInfo.tRecFilesInfo[uwRecFileIndex].HidnFileInfo.SubHidnInfo.ubPreviewMode)
		{
			UI_StartPlayRecordFile(UI_RECPLAY_SINGLEVIEW);
		}
		else
		{
			OSD_IMG_INFO tRecPlayDispOsdImgInfo[5];

			if(tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_RECFILEDISPTYPE_BG, 5, &tRecPlayDispOsdImgInfo[0]) != OSD_OK)
			{
				printd(DBG_ErrorLvl, "Load OSD Image FAIL, pls check (%d) !\n", __LINE__);
				return;
			}
			tOSD_Img2(&tRecPlayDispOsdImgInfo[0], OSD_QUEUE);
			tOSD_Img2(&tRecPlayDispOsdImgInfo[2], OSD_QUEUE);
			tOSD_Img2(&tRecPlayDispOsdImgInfo[3], OSD_UPDATE);
			tUI_PlyDispTypeSelItem.ubItemIdx = UI_RECPLAY_MULTIVIEW;
			tUI_State = UI_RECPLAYDISPTYPE_SEL_STATE;
		}
	}
}
//------------------------------------------------------------------------------
uint8_t UI_DeleteRecordFile(uint16_t uwRecFileIndex)
{
	FS_KNL_MANUAL_DEL_PROCESS_t tUI_DelAct; 
	uint8_t ubFsTimeout = 150;
	FS_MEDIA_SEL MediaSel;

	MediaSel = KNL_GetFsMedia();
	
	tUI_DelAct.MediaSel = MediaSel;
	memset(&tUI_DelAct.FldName, 0, sizeof(tUI_DelAct.FldName));
	memset(&tUI_DelAct.FileName, 0, sizeof(tUI_DelAct.FileName));
	memcpy(&tUI_DelAct.FldName, &tUI_RecFoldersInfo.tRecFolderInfo[tUI_RecFoldersInfo.uwRecFolderSelIdx].FldName, sizeof(tUI_RecFoldersInfo.tRecFolderInfo[tUI_RecFoldersInfo.uwRecFolderSelIdx].FldName));
	memcpy(&tUI_DelAct.FileName, &tUI_RecFilesInfo.tRecFilesInfo[uwRecFileIndex].HidnFileInfo.FileName, sizeof(tUI_RecFilesInfo.tRecFilesInfo[uwRecFileIndex].HidnFileInfo.FileName));
	tUI_DelAct.FilePath = tUI_RecFilesInfo.tRecFilesInfo[uwRecFileIndex].HidnFileInfo.FilePath;
	FS_ManualDeleteFile(&tUI_DelAct);
	while(FS_ChkManualDelStatus(MediaSel) != FS_MANUAL_DEL_OK)
	{
		osDelay(20);
		if(!--ubFsTimeout)
		{
			printd(DBG_ErrorLvl, "Delete File Err !!\n");
			return FALSE;
		}
	}
	return TRUE;
}



//------------------------------------------------------------------------------

void UI_PlaybackSubSubMenuPage(UI_ArrowKey_t tArrowKey)
{
	OSD_IMG_INFO tOsdImgInfo;
	uint16_t ubUI_RecFileIdx, ubUI_PrevRecFileIdx = 0;
	KNL_RecordAct_t tUI_PlayAct = {KNL_RECORDFUNC_DISABLE, NULL,};

	if((!tUI_RecFilesInfo.uwTotalRecFileNum) && (EXIT_ARROW != tArrowKey))
		return;

	switch(tArrowKey)
	{
		case LEFT_ARROW:
			if(!tUI_RecFilesInfo.uwRecFileSelIdx)
				return;
			ubUI_PrevRecFileIdx = (tUI_RecFilesInfo.uwRecFileSelIdx % REC_FILE_LIST_MAXNUM);
			ubUI_RecFileIdx 	= (--tUI_RecFilesInfo.uwRecFileSelIdx % REC_FILE_LIST_MAXNUM);
			break;
		case RIGHT_ARROW:
			if((tUI_RecFilesInfo.uwRecFileSelIdx + 1) >= tUI_RecFilesInfo.uwTotalRecFileNum)
				return;
			ubUI_PrevRecFileIdx = (tUI_RecFilesInfo.uwRecFileSelIdx % REC_FILE_LIST_MAXNUM);
			ubUI_RecFileIdx 	= (++tUI_RecFilesInfo.uwRecFileSelIdx % REC_FILE_LIST_MAXNUM);
			break;
		case ENTER_ARROW:

			UI_PlayRecordFile(tUI_RecFilesInfo.uwRecFileSelIdx);
			return;
			
		case EXIT_ARROW:
			tUI_State = UI_SUBMENU_STATE;
			KNL_RevertDisplayMode();
			if(tKNL_GetRecordFunc() != KNL_RECORDFUNC_LOOP && tKNL_GetRecordFunc() != KNL_RECORDFUNC_MANU)
			{
				tUI_PlayAct.tRecordFunc = KNL_RECORDFUNC_DISABLE;
				tUI_PlayAct.pRecordStsNtyCb = NULL;
				tKNL_ExecRecordFunc(tUI_PlayAct);
			}
			tUI_RecFilesInfo.uwRecFileSelIdx = 0;
			UI_DrawDCIMFolderMenu();
			tUI_CuSetting.IconSts.ubShowLostLogoFlag = FALSE;
			SendIrCodeFlag = 0;
			
			return;
		default:
			return;
	}
	//换页
	if((((!(tUI_RecFilesInfo.uwRecFileSelIdx % REC_FILE_LIST_MAXNUM)) && (tUI_RecFilesInfo.uwRecFileSelIdx) && (ubUI_PrevRecFileIdx == REC_FILE_LIST_MAXNUM - 1))) ||//下一页
	   ((ubUI_PrevRecFileIdx == 0) && (ubUI_RecFileIdx == REC_FILE_LIST_MAXNUM - 1)))//上一页
	{
		uint16_t uwFileStartIdx = 0, uwFileEndIdx = 0;
		if(RIGHT_ARROW == tArrowKey)
		{
			uwFileStartIdx = tUI_RecFilesInfo.uwRecFileSelIdx;
			uwFileEndIdx   = tUI_RecFilesInfo.uwTotalRecFileNum - tUI_RecFilesInfo.uwRecFileSelIdx;
			uwFileEndIdx   = (uwFileEndIdx < REC_FILE_LIST_MAXNUM)?tUI_RecFilesInfo.uwTotalRecFileNum:(uwFileStartIdx + REC_FILE_LIST_MAXNUM);
		}
		else
		{
			uwFileStartIdx = (tUI_RecFilesInfo.uwRecFileSelIdx / REC_FILE_LIST_MAXNUM) * REC_FILE_LIST_MAXNUM;
			uwFileEndIdx   = tUI_RecFilesInfo.uwRecFileSelIdx + 1;
		}
		UI_ListRecFileInfo(uwFileStartIdx, uwFileEndIdx, OSD_QUEUE);
	}
	UI_PlaybackDrawSubSubMenuItemFocus(ubUI_PrevRecFileIdx,ubUI_RecFileIdx);
}





//------------------------------------------------------------------------------setting-------------------------------------------------------------------------------------
//------------------------------------------------------------------------------setting-------------------------------------------------------------------------------------
//------------------------------------------------------------------------------setting-------------------------------------------------------------------------------------


void UI_SystemDrawDateTimeYearButton(uint8_t DateTimeYearButtonHLIndex)
{
	OSD_IMG_INFO tOsdImgInfo[2];
	uint16_t uwTemp[6];

	
	uwTemp[0] = tUI_CuSetting.tSysCalendar.uwYear;
	uwTemp[1] = tUI_CuSetting.tSysCalendar.ubMonth;
	uwTemp[2] = tUI_CuSetting.tSysCalendar.ubDate;
	uwTemp[3] = tUI_CuSetting.tSysCalendar.ubHour;
	uwTemp[4] = tUI_CuSetting.tSysCalendar.ubMin;
	uwTemp[5] = tUI_CuSetting.tSysCalendar.ubSec;

	//BUTTON
	for(uint8_t i = 0;i < 3;i++)
	{
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_DATE_TIME_YEAR_NOR, 2, &tOsdImgInfo[0]);
		tOsdImgInfo[0].uwXStart += DATE_TIME_YEAR_DIST_X * i;
		tOSD_Img2(&tOsdImgInfo[0], OSD_QUEUE);

		//数值
		if(uwTemp[i] < 10)
		{
			UI_ShowButtonValueNormal(0,tOsdImgInfo[0].uwXStart + 30,tOsdImgInfo[0].uwYStart + 15,OSD_QUEUE);
			UI_ShowButtonValueNormal(uwTemp[i],tOsdImgInfo[0].uwXStart + 50,tOsdImgInfo[0].uwYStart + 15,OSD_QUEUE);
		}
		else if(uwTemp[i] < 100)
			UI_ShowButtonValueNormal(uwTemp[i],tOsdImgInfo[0].uwXStart + 30,tOsdImgInfo[0].uwYStart + 15,OSD_QUEUE);
		else
			UI_ShowButtonValueNormal(tUI_CuSetting.tSysCalendar.uwYear,tOsdImgInfo[0].uwXStart + 10,tOsdImgInfo[0].uwYStart + 15,OSD_QUEUE);

		tOsdImgInfo[0].uwYStart += DATE_TIME_YEAR_DIST_Y;
		tOSD_Img2(&tOsdImgInfo[0], OSD_QUEUE);

		//数值
		if(uwTemp[i + 3] < 10)
		{
			UI_ShowButtonValueNormal(0,tOsdImgInfo[0].uwXStart + 30,tOsdImgInfo[0].uwYStart + 15,OSD_QUEUE);
			UI_ShowButtonValueNormal(uwTemp[i +3],tOsdImgInfo[0].uwXStart + 50,tOsdImgInfo[0].uwYStart + 15,OSD_QUEUE);
		}
		else if(uwTemp[i + 3] < 100)
			UI_ShowButtonValueNormal(uwTemp[i + 3],tOsdImgInfo[0].uwXStart + 30,tOsdImgInfo[0].uwYStart + 15,OSD_QUEUE);
		else
			UI_ShowButtonValueNormal(tUI_CuSetting.tSysCalendar.uwYear,tOsdImgInfo[0].uwXStart + 10,tOsdImgInfo[0].uwYStart + 15,OSD_QUEUE);
	}
	
	//HIGHLIGHT BUTTON
	tOsdImgInfo[1].uwXStart +=  (DateTimeYearButtonHLIndex % 3) * DATE_TIME_YEAR_DIST_X;
	if(DateTimeYearButtonHLIndex > 2)
	{
		tOsdImgInfo[1].uwYStart += DATE_TIME_YEAR_DIST_Y;
	}
	tOSD_Img2(&tOsdImgInfo[1], OSD_QUEUE);

	//数值高亮
	if(uwTemp[DateTimeYearButtonHLIndex] < 10)
	{
		UI_ShowButtonValueHighLight(0,tOsdImgInfo[1].uwXStart + 30,tOsdImgInfo[1].uwYStart + 15,OSD_QUEUE);
		UI_ShowButtonValueHighLight(uwTemp[DateTimeYearButtonHLIndex],tOsdImgInfo[1].uwXStart + 50,tOsdImgInfo[1].uwYStart + 15,OSD_UPDATE);
	}
	else if(uwTemp[DateTimeYearButtonHLIndex] < 100)
		UI_ShowButtonValueHighLight(uwTemp[DateTimeYearButtonHLIndex],tOsdImgInfo[1].uwXStart + 30,tOsdImgInfo[1].uwYStart + 15,OSD_UPDATE);
	else
		UI_ShowButtonValueHighLight(tUI_CuSetting.tSysCalendar.uwYear,tOsdImgInfo[1].uwXStart + 10,tOsdImgInfo[1].uwYStart + 15,OSD_UPDATE);
	
}


//------------------------------------------------------------------------------------system------------------------------------------------------------------------------


static uint8_t SystemSubSubRemoteMode = REMOTE_SELECT_ITEM;


//------------------------------------------------------------------------------------
void UI_SystemSubSubMenuExecute()
{
	uint8_t ubItemIdx = tSystemSubSubMenuItem.tSubMenuInfo.ubItemIdx;
	if(ubItemIdx == DIMMER_ITEM  || ubItemIdx == AUTO_ONOFF_ITEM)
	{
		UI_PanelOn();
	}
	else if(ubItemIdx == AUTO_SET_ITEM)
	{
		OSD_IMG_INFO tOsdImgInfo[10];
		
		tOSD_GetOsdImgInfor(1, OSD_IMG1, OSD1IMG_SUBMENU2, 1, &tOsdImgInfo[0]);
		tOSD_Img1(&tOsdImgInfo[0], OSD_QUEUE);
		if(!ubUI_TouchPanelSts)
		{
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_RETURN_BLACK, 1, &tOsdImgInfo[0]);
			tOSD_Img2(&tOsdImgInfo[0], OSD_QUEUE);
		}
		tOSD_GetOsdImgInfor(1, OSD_IMG2, (tUI_CuSetting.tLanguage == LANGUAGE_ENGLISH)?OSD2IMG_SYSTEM_DAY_WORD:
										(tUI_CuSetting.tLanguage == LANGUAGE_GERMAN)?OSD2IMG_SYSTEM_DAY_WORD_GER:
										(tUI_CuSetting.tLanguage == LANGUAGE_FRENCH)?OSD2IMG_SYSTEM_DAY_WORD_FR:OSD2IMG_SYSTEM_DAY_WORD_CHN, 2, &tOsdImgInfo[0]);
		//文字
		tOSD_Img2(&tOsdImgInfo[0], OSD_QUEUE);
		tOSD_Img2(&tOsdImgInfo[1], OSD_QUEUE);
		//滑块滑杆
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_SYSTEM_DAY_SLIDER, 8, &tOsdImgInfo[0]);
		//绿色高亮
		tOSD_Img2(&tOsdImgInfo[2], OSD_QUEUE);	

		//滑杆
		tOSD_Img2(&tOsdImgInfo[0], OSD_QUEUE);
		tOSD_Img2(&tOsdImgInfo[1], OSD_QUEUE);
		
		//滑块
		tOsdImgInfo[6].uwXStart = UI_Map(tUI_CuSetting.ubAutoDimmer_MaxVal,0,99,
			tOsdImgInfo[0].uwXStart,tOsdImgInfo[0].uwXStart + tOsdImgInfo[0].uwHSize - tOsdImgInfo[6].uwHSize);
		tOSD_Img2(&tOsdImgInfo[6], OSD_QUEUE);
			
		if(tUI_CuSetting.ubAutoDimmer_MaxVal < 10)
		{
			UI_ShowButtonValueHighLight(0,tOsdImgInfo[6].uwXStart + 4,tOsdImgInfo[6].uwYStart + 9,OSD_QUEUE);
			UI_ShowButtonValueHighLight(tUI_CuSetting.ubAutoDimmer_MaxVal,tOsdImgInfo[6].uwXStart + 24,tOsdImgInfo[6].uwYStart + 9,OSD_QUEUE);
		}
		else
			UI_ShowButtonValueHighLight(tUI_CuSetting.ubAutoDimmer_MaxVal,tOsdImgInfo[6].uwXStart + 4,tOsdImgInfo[6].uwYStart + 9,OSD_QUEUE);
		//滑块
		tOsdImgInfo[7].uwXStart = UI_Map(tUI_CuSetting.ubAutoDimmer_MinVal,0,99,
			tOsdImgInfo[1].uwXStart,tOsdImgInfo[1].uwXStart + tOsdImgInfo[1].uwHSize - tOsdImgInfo[7].uwHSize);
		tOSD_Img2(&tOsdImgInfo[7], OSD_QUEUE);
		
		if(tUI_CuSetting.ubAutoDimmer_MinVal < 10)
		{
			UI_ShowButtonValueHighLight(0,tOsdImgInfo[7].uwXStart + 4,tOsdImgInfo[7].uwYStart + 9,OSD_UPDATE);
			UI_ShowButtonValueHighLight(tUI_CuSetting.ubAutoDimmer_MinVal,tOsdImgInfo[7].uwXStart + 24,tOsdImgInfo[7].uwYStart + 9,OSD_UPDATE);
		}
		else
			UI_ShowButtonValueHighLight(tUI_CuSetting.ubAutoDimmer_MinVal,tOsdImgInfo[7].uwXStart + 4,tOsdImgInfo[7].uwYStart + 9,OSD_UPDATE);

		
		SystemSubSubSubRemoteMode = REMOTE_SELECT_ITEM;
		memset(&(tSystemSubSubSubMenuItem[ubItemIdx].tSubMenuInfo), 0, sizeof(UI_MenuItem_t));
		tUI_State = UI_SUBSUBSUBMENU_STATE;
			
	}
	else if(ubItemIdx == UPGRADE_ITEM)
	{
		OSD_IMG_INFO tOsdImgInfo[4];
		tOSD_GetOsdImgInfor(1, OSD_IMG2,(tUI_CuSetting.tLanguage == LANGUAGE_ENGLISH)?OSD2IMG_SYSTEM_UPGRADE_BG:
										(tUI_CuSetting.tLanguage == LANGUAGE_GERMAN)?OSD2IMG_SYSTEM_UPGRADE_BG_GER:
										(tUI_CuSetting.tLanguage == LANGUAGE_FRENCH)?OSD2IMG_SYSTEM_UPGRADE_BG_FR:OSD2IMG_SYSTEM_UPGRADE_BG_CHN, 1, &tOsdImgInfo[0]);
		tOSD_Img2(&tOsdImgInfo[0], OSD_QUEUE);

		tOSD_GetOsdImgInfor(1, OSD_IMG2,(tUI_CuSetting.tLanguage == LANGUAGE_ENGLISH)?OSD2IMG_SDCARD_FORMAT_BG:
										(tUI_CuSetting.tLanguage == LANGUAGE_GERMAN)?OSD2IMG_SDCARD_FORMAT_BG_GER:
										(tUI_CuSetting.tLanguage == LANGUAGE_FRENCH)?OSD2IMG_SDCARD_FORMAT_BG_FR:OSD2IMG_SDCARD_FORMAT_BG_CHN, 5, &tOsdImgInfo[0]);
		tOSD_Img2(&tOsdImgInfo[2], OSD_QUEUE);
		tOSD_Img2(&tOsdImgInfo[3], OSD_UPDATE);

		SystemSubSubSubRemoteMode = REMOTE_SELECT_ITEM;
		memset(&(tSystemSubSubSubMenuItem[ubItemIdx].tSubMenuInfo), 0, sizeof(UI_MenuItem_t));
		tUI_State = UI_SUBSUBSUBMENU_STATE;
	}
	else if(ubItemIdx == LANGUAGE_ITEM)
	{
		OSD_IMG_INFO tOsdImgInfo[LANGUAGE_MAX*4];
		
		tOSD_GetOsdImgInfor(1, OSD_IMG1, OSD1IMG_SUBMENU2, 1, &tOsdImgInfo[0]);
		tOSD_Img1(&tOsdImgInfo[0], OSD_QUEUE);
		if(!ubUI_TouchPanelSts)
		{
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_RETURN_BLACK, 1, &tOsdImgInfo[0]);
			tOSD_Img2(&tOsdImgInfo[0], OSD_QUEUE);
		}
		
		tOSD_GetOsdImgInfor(1, OSD_IMG2,  OSD2IMG_SYSTEM_ENGLISH_NOR, 4*LANGUAGE_MAX, &tOsdImgInfo[0]);
		for(uint8_t i = 0;i < LANGUAGE_MAX;i ++)
		{
			if(tUI_CuSetting.tLanguage == i)
				tOSD_Img2(&tOsdImgInfo[2*i + 2*LANGUAGE_MAX], OSD_QUEUE);//高亮图标上的文字
			else
				tOSD_Img2(&tOsdImgInfo[2*i], OSD_QUEUE);
		}
		//highLight
		if(tUI_CuSetting.tLanguage == LANGUAGE_ENGLISH)
			tOSD_Img2(&tOsdImgInfo[LANGUAGE_MAX*2 + 1], OSD_UPDATE);
		else
			tOSD_Img2(&tOsdImgInfo[1], OSD_UPDATE);
		

		SystemSubSubSubRemoteMode = REMOTE_SELECT_ITEM;
		memset(&(tSystemSubSubSubMenuItem[ubItemIdx].tSubMenuInfo), 0, sizeof(UI_MenuItem_t));
		tUI_State = UI_SUBSUBSUBMENU_STATE;
	}
	else if(ubItemIdx == SD_SWITCH_ITEM)
	{
		UI_SwitchViewType(DeskTopShowView,FALSE);
	}
	else if(ubItemIdx == DATETIME_ITEM)
	{
		OSD_IMG_INFO tMenuOsdImgInfo;
		OSD_IMG_INFO tOsdImgInfo[12];
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

		//timezone Slider
        tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_TIME_ZONE_SLIDER, 6, &tOsdImgInfo[0]);
		tOSD_Img2(&tOsdImgInfo[0], OSD_QUEUE);
		tOsdImgInfo[2].uwXStart = UI_Map(tUI_CuSetting.ubtimezone,0,26,tOsdImgInfo[0].uwXStart,tOsdImgInfo[0].uwXStart + tOsdImgInfo[0].uwHSize - tOsdImgInfo[2].uwHSize);
		tOSD_Img2(&tOsdImgInfo[2], OSD_QUEUE);      

		UI_ShowUTCVal();

		// timezone comfirm button
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_DATE_TIME_UTC_COMFIRM_NOR, 1, &tOsdImgInfo[0]);
		tOSD_Img2(&tOsdImgInfo[0], OSD_QUEUE);
		
		//BUTTON
//		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_DATE_TIME_YEAR_NOR, 12, &tOsdImgInfo[0]);
//		for(uint8_t i = 0;i < 6;i++)
//			tOSD_Img2(&tOsdImgInfo[2*i], OSD_QUEUE);
//		//HIGHLIGHT BUTTON
//		tOSD_Img2(&tOsdImgInfo[1], OSD_QUEUE);
		
		RTC_GetCalendar((RTC_Calendar_t *)(&(tUI_CuSetting.tSysCalendar)));
		UI_SystemDrawDateTimeYearButton(0);

		SystemSubSubSubRemoteMode = REMOTE_SELECT_ITEM;
		memset(&(tSystemSubSubSubMenuItem[ubItemIdx].tSubMenuInfo), 0, sizeof(UI_MenuItem_t));
		tUI_State = UI_SUBSUBSUBMENU_STATE;
	}
	else if(ubItemIdx == RESET_ITEM)
	{
		OSD_IMG_INFO tOsdImgInfo;
		
		tOSD_GetOsdImgInfor(1, OSD_IMG1, OSD1IMG_RESET_LOGO, 1, &tOsdImgInfo);
		tOSD_Img1(&tOsdImgInfo, OSD_QUEUE);
		
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_RETURN_BLACK, 1, &tOsdImgInfo);
		tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);		
		UI_ResetUIParameter();
		UI_PanelOn();

	    UI_SwitchViewType(QUALVIEW_ITEM, FALSE);
		osDelay(2000);
		
		for(UI_CamNum_t tCamNum = CAM1;tCamNum <= CAM4;tCamNum ++)
		{
			UI_UpdateLaserAndLedCtrl(tCamNum);
			UI_SetTxImgProc(tCamNum);
			UI_UpdateTxDevStatusInfo(tCamNum);
		}
		SYS_Reboot();
	}	
}
//------------------------------------------------------------------------------------
void UI_AIDrawSubSubMenuItem(uint8_t ubDrawBox)
{
#if 1
	OSD_IMG_INFO tOsdImgInfo[8];
	uint8_t ubItemPreIdx = tSettingSubSubMenuItem.tSettingS[10].tSubMenuInfo.ubItemPreIdx;
	uint8_t ubItemCurIdx = tSettingSubSubMenuItem.tSettingS[10].tSubMenuInfo.ubItemIdx;
	uint8_t ubCamNumPre = ubItemPreIdx%4;
	uint8_t ubCamNumCur = ubItemCurIdx%4;
	uint16_t ubXpos = AI_Config_Xpos,ubYpos = AI_Config_Ypos,ubXstep = AI_Config_Xstep,ubYstep = AI_Config_Ystep;
	printf("ubItemPreIdx=%d",ubItemPreIdx);
	printf("ubItemCurIdx=%d",ubItemCurIdx);
	//----------------------------pre----------------------------------------------
	if(ubItemPreIdx != ubItemCurIdx)
	{
		//pre
		tOSD_GetOsdImgInfor(1, OSD_IMG2,OSD2IMG_AI_OFF_NOR , 4, &tOsdImgInfo[0]);
//		if(ubItemPreIdx <= AI_BSD_CAM4)//BSD
//		{	
//			//tOSD_GetOsdImgInfor(1, OSD_IMG2,OSD2IMG_AI_OFF_NOR , 4, &tOsdImgInfo[0]);
//			if(tUI_CuSetting.ubIsEnableBSD[ubCamNumPre])
//			{
//				tOsdImgInfo[2].uwXStart = ubXpos + ubXstep * ubCamNumPre;
//				tOsdImgInfo[2].uwYStart = ubYpos;
//				tOSD_Img2(&tOsdImgInfo[2], OSD_UPDATE);
//			}
//			else
//			{
//				tOsdImgInfo[0].uwXStart = ubXpos + ubXstep * ubCamNumPre;
//				tOsdImgInfo[0].uwYStart = ubYpos;
//				tOSD_Img2(&tOsdImgInfo[0], OSD_UPDATE);
//			}
//		}
		if(ubItemPreIdx <= AI_PD_CAM4)//PERSON_DET_ITEM
		{	
			if(tUI_CuSetting.ubDetectPeopleFlag[ubCamNumPre])
			{
				tOsdImgInfo[2].uwXStart = ubXpos + ubXstep * ubCamNumPre;
				tOsdImgInfo[2].uwYStart = ubYpos;
				tOSD_Img2(&tOsdImgInfo[2], OSD_UPDATE);
			}
			else
			{
				tOsdImgInfo[0].uwXStart = ubXpos + ubXstep * ubCamNumPre;
				tOsdImgInfo[0].uwYStart = ubYpos;
				tOSD_Img2(&tOsdImgInfo[0], OSD_UPDATE);
			}

		}
		else if(ubItemPreIdx > AI_PD_CAM4 && ubItemPreIdx <= AI_CD_CAM4)//VEHICLES_DET_ITEM
		{	
			if(tUI_CuSetting.ubDetectCarFlag[ubCamNumPre])
			{
				tOsdImgInfo[2].uwXStart = ubXpos + ubXstep * ubCamNumPre;
				tOsdImgInfo[2].uwYStart = ubYpos + ubYstep*1;
				tOSD_Img2(&tOsdImgInfo[2], OSD_UPDATE);
			}
			else
			{
				tOsdImgInfo[0].uwXStart = ubXpos + ubXstep * ubCamNumPre;
				tOsdImgInfo[0].uwYStart = ubYpos + ubYstep*1;
				tOSD_Img2(&tOsdImgInfo[0], OSD_UPDATE);
			}

		}
		else if(ubItemPreIdx > AI_CD_CAM4 && ubItemPreIdx <= AI_ALARM_CAM4)//ALARM
		{	
			if(tUI_CuSetting.ubIsEnableBSDALARM[ubCamNumPre])
			{
				tOsdImgInfo[2].uwXStart = ubXpos + ubXstep * ubCamNumPre;
				tOsdImgInfo[2].uwYStart = ubYpos + ubYstep*2;
				tOSD_Img2(&tOsdImgInfo[2], OSD_UPDATE);
			}
			else
			{
				tOsdImgInfo[0].uwXStart = ubXpos + ubXstep * ubCamNumPre;
				tOsdImgInfo[0].uwYStart = ubYpos + ubYstep*2;
				tOSD_Img2(&tOsdImgInfo[0], OSD_UPDATE);
			}

		}
		else if(ubItemPreIdx > AI_ALARM_CAM4 && ubItemPreIdx <= AI_TRIGGER_CAM4)
		{	
			if(tUI_CuSetting.ubBSDTriggerOut[ubCamNumPre])
			{
				tOsdImgInfo[2].uwXStart = ubXpos + ubXstep * ubCamNumPre;
				tOsdImgInfo[2].uwYStart = ubYpos + ubYstep*3;
				tOSD_Img2(&tOsdImgInfo[2], OSD_UPDATE);
			}
			else
			{
				tOsdImgInfo[0].uwXStart = ubXpos + ubXstep * ubCamNumPre;
				tOsdImgInfo[0].uwYStart = ubYpos + ubYstep*3;
				tOSD_Img2(&tOsdImgInfo[0], OSD_UPDATE);
			}
		}
		else if(ubItemPreIdx > AI_TRIGGER_CAM4 && ubItemPreIdx <= AI_BSDRANGE_CAM4)
		{	
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_AI_CONFIG_SETUP_NOR, 2, &tOsdImgInfo[0]);
			tOsdImgInfo[0].uwXStart = ubXpos + ubXstep * ubCamNumPre;
			tOsdImgInfo[0].uwYStart = ubYpos + ubYstep*4;
			tOSD_Img2(&tOsdImgInfo[0], OSD_UPDATE); 
		//棰勭暀
		}
	}
	//----------------------------cur----------------------------------------------
	//cur
	tOSD_GetOsdImgInfor(1, OSD_IMG2,OSD2IMG_AI_OFF_NOR , 4, &tOsdImgInfo[0]);
//	if(ubItemCurIdx <= AI_BSD_CAM4)//BSD
//	{	
//		//tOSD_GetOsdImgInfor(1, OSD_IMG2,OSD2IMG_AI_OFF_NOR , 4, &tOsdImgInfo[0]);
//		if(tUI_CuSetting.ubIsEnableBSD[ubCamNumCur])
//		{
//			tOsdImgInfo[3].uwXStart = ubXpos + ubXstep * ubCamNumCur;
//			tOsdImgInfo[3].uwYStart = ubYpos;
//			tOSD_Img2(&tOsdImgInfo[3], OSD_UPDATE);
//		}
//		else
//		{
//			tOsdImgInfo[1].uwXStart = ubXpos + ubXstep * ubCamNumCur;
//			tOsdImgInfo[1].uwYStart = ubYpos;
//			tOSD_Img2(&tOsdImgInfo[1], OSD_UPDATE);
//		}
//	}
	if(ubItemCurIdx <= AI_PD_CAM4)//PERSON_DET_ITEM
	{	
		if(tUI_CuSetting.ubDetectPeopleFlag[ubCamNumCur])
		{
			tOsdImgInfo[3].uwXStart = ubXpos + ubXstep * ubCamNumCur;
			tOsdImgInfo[3].uwYStart = ubYpos;

			tOSD_Img2(&tOsdImgInfo[3], OSD_UPDATE);
		}
		else
		{
			tOsdImgInfo[1].uwXStart = ubXpos + ubXstep * ubCamNumCur;
			tOsdImgInfo[1].uwYStart = ubYpos;
			tOSD_Img2(&tOsdImgInfo[1], OSD_UPDATE);
		}
	
	}
	else if(ubItemCurIdx > AI_PD_CAM4 && ubItemCurIdx <= AI_CD_CAM4)//VEHICLES_DET_ITEM
	{	
		if(tUI_CuSetting.ubDetectCarFlag[ubCamNumCur])
		{
			tOsdImgInfo[3].uwXStart = ubXpos + ubXstep * ubCamNumCur;
			tOsdImgInfo[3].uwYStart = ubYpos + ubYstep * 1;
			tOSD_Img2(&tOsdImgInfo[3], OSD_UPDATE);
		}
		else
		{
			tOsdImgInfo[1].uwXStart = ubXpos + ubXstep * ubCamNumCur;
			tOsdImgInfo[1].uwYStart = ubYpos + ubYstep * 1;
			tOSD_Img2(&tOsdImgInfo[1], OSD_UPDATE);
		}
	
	}
	else if(ubItemCurIdx > AI_CD_CAM4 && ubItemCurIdx <= AI_ALARM_CAM4)//ALARM
	{	
		if(tUI_CuSetting.ubIsEnableBSDALARM[ubCamNumCur])
		{
			tOsdImgInfo[3].uwXStart = ubXpos + ubXstep * ubCamNumCur;
			tOsdImgInfo[3].uwYStart = ubYpos + ubYstep * 2;
			tOSD_Img2(&tOsdImgInfo[3], OSD_UPDATE);
		}
		else
		{
			tOsdImgInfo[1].uwXStart =  ubXpos + ubXstep * ubCamNumCur;
			tOsdImgInfo[1].uwYStart =  ubYpos + ubYstep * 2;
			tOSD_Img2(&tOsdImgInfo[1], OSD_UPDATE);
		}
	
	}
	else if(ubItemCurIdx > AI_ALARM_CAM4 && ubItemCurIdx <= AI_TRIGGER_CAM4)
	{	
		if(tUI_CuSetting.ubBSDTriggerOut[ubCamNumCur])
		{
			tOsdImgInfo[3].uwXStart = ubXpos + ubXstep * ubCamNumCur;
			tOsdImgInfo[3].uwYStart = ubYpos + ubYstep * 3;
			tOSD_Img2(&tOsdImgInfo[3], OSD_UPDATE);
		}
		else
		{
			tOsdImgInfo[1].uwXStart = ubXpos + ubXstep * ubCamNumCur;
			tOsdImgInfo[1].uwYStart = ubYpos + ubYstep * 3;
			tOSD_Img2(&tOsdImgInfo[1], OSD_UPDATE);
		}
	}
	else if(ubItemCurIdx > AI_TRIGGER_CAM4 && ubItemCurIdx <= AI_BSDRANGE_CAM4)
	{	
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_AI_CONFIG_SETUP_NOR, 2, &tOsdImgInfo[0]);
		tOsdImgInfo[1].uwXStart = ubXpos + ubXstep * ubCamNumCur;
		tOsdImgInfo[1].uwYStart = ubYpos + ubYstep*4;
		tOSD_Img2(&tOsdImgInfo[1], OSD_UPDATE); 
	}


#endif
}
//------------------------------------------------------------------------------------
void UI_SystemDrawSubSubMenuItem(uint8_t ubDrawBox)
{
	OSD_IMG_INFO tOsdImgInfo[8];
	uint8_t ubItemPreIdx = tSystemSubSubMenuItem.tSubMenuInfo.ubItemPreIdx;
	uint8_t ubItemCurIdx = tSystemSubSubMenuItem.tSubMenuInfo.ubItemIdx;
	
	//----------------------------pre----------------------------------------------
	if(ubItemPreIdx != ubItemCurIdx)
	{
		if(ubItemPreIdx == DIMMER_ITEM)//slider
		{
			//先擦除选中框
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_SYSTEM_DIMMER_GREEN_HL, 1, &tOsdImgInfo[0]);
			OSD_EraserImg2_NoUpdate(&tOsdImgInfo[0]);
			//显示滑杆
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_SYSTEM_DIMMER_SLIDER, 1, &tOsdImgInfo[0]);
			tOSD_Img2(&tOsdImgInfo[0],OSD_QUEUE);
			//显示滑块
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_SYSTEM_DIMMER_ROUND_HL, 1, &tOsdImgInfo[1]);
			
			tOsdImgInfo[1].uwXStart = UI_Map(tUI_CuSetting.BriLvL.ulBL_UpdateLvL,0,99,
				tOsdImgInfo[0].uwXStart,tOsdImgInfo[0].uwXStart + tOsdImgInfo[0].uwHSize - tOsdImgInfo[1].uwHSize);	
			tOSD_Img2(&tOsdImgInfo[1], OSD_QUEUE);
			//数值
			if(tUI_CuSetting.BriLvL.ulBL_UpdateLvL < 10)
			{
				UI_ShowButtonValueHighLight(0,tOsdImgInfo[1].uwXStart + 4,tOsdImgInfo[1].uwYStart + 9,OSD_QUEUE);
				UI_ShowButtonValueHighLight(tUI_CuSetting.BriLvL.ulBL_UpdateLvL,tOsdImgInfo[1].uwXStart + 24,tOsdImgInfo[1].uwYStart + 9,OSD_QUEUE);
			}
			else
				UI_ShowButtonValueHighLight(tUI_CuSetting.BriLvL.ulBL_UpdateLvL,tOsdImgInfo[1].uwXStart + 4,tOsdImgInfo[1].uwYStart + 9,OSD_UPDATE);
			
		
		}
		else if(ubItemPreIdx == AUTO_ONOFF_ITEM)
		{
			tOSD_GetOsdImgInfor(1, OSD_IMG2,OSD2IMG_SYSTEM_AUTO_OFF_NOR , 4, &tOsdImgInfo[0]);
			if(tUI_CuSetting.ubAutoDimmer)
				tOSD_Img2(&tOsdImgInfo[2], OSD_UPDATE);
			else
				tOSD_Img2(&tOsdImgInfo[0], OSD_UPDATE);

		}
		else if(ubItemPreIdx == AUTO_SET_ITEM)
		{
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_SYSTEM_AUTO_SETTING_NOR , 1, &tOsdImgInfo[0]);
			tOSD_Img2(&tOsdImgInfo[0], OSD_UPDATE);
		}
		else if(ubItemPreIdx == MENULOCK_ITEM)
		{
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_SYSTEM_MENULOCK_OFF_NOR , 4, &tOsdImgInfo[0]);
			if(tUI_CuSetting.ubMenuLock)
				tOSD_Img2(&tOsdImgInfo[2], OSD_UPDATE);
			else
				tOSD_Img2(&tOsdImgInfo[0], OSD_UPDATE);
		}
		else if (ubItemPreIdx == BUZZER_ITEM)
		{
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_SYSTEM_BUZZER_OFF_NOR, 4, &tOsdImgInfo[0]);
			if(tUI_CuSetting.ubBuzzer)
				tOSD_Img2(&tOsdImgInfo[2], OSD_UPDATE);
			else
				tOSD_Img2(&tOsdImgInfo[0], OSD_UPDATE);
		}
		else if(ubItemPreIdx == UPGRADE_ITEM)
		{
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_SYSTEM_UPGRADE_NOR, 1, &tOsdImgInfo[0]);
			tOSD_Img2(&tOsdImgInfo[0], OSD_UPDATE);
		}
		else if(ubItemPreIdx == LANGUAGE_ITEM)
		{
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_SYSTEM_LANGUAGE_NOR, 1, &tOsdImgInfo[0]);
			tOSD_Img2(&tOsdImgInfo[0], OSD_UPDATE);
		}
		else if(ubItemPreIdx == RESET_ITEM)
		{
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_SYSTEM_RESET_NOR, 1, &tOsdImgInfo[0]);
			tOSD_Img2(&tOsdImgInfo[0], OSD_UPDATE);
		}
		else if(ubItemPreIdx == SD_SWITCH_ITEM)
		{
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_SYSTEM_SD_SWITCH_OFF_NOR , 4, &tOsdImgInfo[0]);
			if(tUI_CuSetting.ubSD_Switch)
				tOSD_Img2(&tOsdImgInfo[2], OSD_UPDATE);
			else
				tOSD_Img2(&tOsdImgInfo[0], OSD_UPDATE);
		}
		else if(ubItemPreIdx == HIDE_CHANNEL_NAME_ITEM)
		{
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_SYSTEM_HIDE_CHANNEL_NAME_OFF_NOR , 4, &tOsdImgInfo[0]);
			if(tUI_CuSetting.ubHideChannelName)
				tOSD_Img2(&tOsdImgInfo[2], OSD_UPDATE);
			else
				tOSD_Img2(&tOsdImgInfo[0], OSD_UPDATE);
		}
		else if(ubItemPreIdx == DATETIME_ITEM)
		{
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_SYSTEM_DATE_TIME_NOR , 1, &tOsdImgInfo[0]);
			tOSD_Img2(&tOsdImgInfo[0], OSD_UPDATE);
		}
	}
	//----------------------------cur----------------------------------------------
	if(ubItemCurIdx == DIMMER_ITEM)//slider
	{
		//显示框
		if(ubDrawBox)
		{
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_SYSTEM_DIMMER_GREEN_HL  + SystemSubSubRemoteMode, 1, &tOsdImgInfo[0]);
			tOSD_Img2(&tOsdImgInfo[0],OSD_QUEUE);
		}
		//显示滑杆
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_SYSTEM_DIMMER_SLIDER, 1, &tOsdImgInfo[0]);
		tOSD_Img2(&tOsdImgInfo[0],OSD_QUEUE);
		//显示滑块
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_SYSTEM_DIMMER_ROUND_HL, 1, &tOsdImgInfo[1]);
	
		tOsdImgInfo[1].uwXStart = UI_Map(tUI_CuSetting.BriLvL.ulBL_UpdateLvL,0,99,
			tOsdImgInfo[0].uwXStart,tOsdImgInfo[0].uwXStart + tOsdImgInfo[0].uwHSize - tOsdImgInfo[1].uwHSize);	
		tOSD_Img2(&tOsdImgInfo[1], OSD_QUEUE);
		//数值
		if(tUI_CuSetting.BriLvL.ulBL_UpdateLvL < 10)
		{
			UI_ShowButtonValueHighLight(0,tOsdImgInfo[1].uwXStart + 4,tOsdImgInfo[1].uwYStart + 9,OSD_QUEUE);
			UI_ShowButtonValueHighLight(tUI_CuSetting.BriLvL.ulBL_UpdateLvL,tOsdImgInfo[1].uwXStart + 24,tOsdImgInfo[1].uwYStart + 9,OSD_UPDATE);
		}
		else
			UI_ShowButtonValueHighLight(tUI_CuSetting.BriLvL.ulBL_UpdateLvL,tOsdImgInfo[1].uwXStart + 4,tOsdImgInfo[1].uwYStart + 9,OSD_UPDATE);
	


	}
	else if(ubItemCurIdx == AUTO_ONOFF_ITEM)
	{
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_SYSTEM_AUTO_OFF_NOR , 4, &tOsdImgInfo[0]);
		if(tUI_CuSetting.ubAutoDimmer)
			tOSD_Img2(&tOsdImgInfo[3], OSD_UPDATE);
		else
			tOSD_Img2(&tOsdImgInfo[1], OSD_UPDATE);

	}
	else if(ubItemCurIdx == AUTO_SET_ITEM)
	{
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_SYSTEM_AUTO_SETTING_HL , 1, &tOsdImgInfo[0]);
		tOSD_Img2(&tOsdImgInfo[0], OSD_UPDATE);
	}
	else if(ubItemCurIdx == MENULOCK_ITEM)
	{
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_SYSTEM_MENULOCK_OFF_NOR , 4, &tOsdImgInfo[0]);
		if(tUI_CuSetting.ubMenuLock)
			tOSD_Img2(&tOsdImgInfo[3], OSD_UPDATE);
		else
			tOSD_Img2(&tOsdImgInfo[1], OSD_UPDATE);
	}
	else if(ubItemCurIdx == BUZZER_ITEM)
	{
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_SYSTEM_BUZZER_OFF_NOR, 4, &tOsdImgInfo[0]);
		if(tUI_CuSetting.ubBuzzer)
			tOSD_Img2(&tOsdImgInfo[3], OSD_UPDATE);
		else
			tOSD_Img2(&tOsdImgInfo[1], OSD_UPDATE);
	}
	else if(ubItemCurIdx == UPGRADE_ITEM)
	{
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_SYSTEM_UPGRADE_HL, 1, &tOsdImgInfo[0]);
		tOSD_Img2(&tOsdImgInfo[0], OSD_UPDATE);
	}
	else if(ubItemCurIdx == LANGUAGE_ITEM)
	{
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_SYSTEM_LANGUAGE_HL, 1, &tOsdImgInfo[0]);
		tOSD_Img2(&tOsdImgInfo[0], OSD_UPDATE);
	}
	else if(ubItemCurIdx == RESET_ITEM)
	{
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_SYSTEM_RESET_HL, 1, &tOsdImgInfo[0]);
		tOSD_Img2(&tOsdImgInfo[0], OSD_UPDATE);
	}
	else if(ubItemCurIdx == SD_SWITCH_ITEM)
	{
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_SYSTEM_SD_SWITCH_OFF_NOR , 4, &tOsdImgInfo[0]);
		if(tUI_CuSetting.ubSD_Switch)
			tOSD_Img2(&tOsdImgInfo[3], OSD_UPDATE);
		else
			tOSD_Img2(&tOsdImgInfo[1], OSD_UPDATE);
	}
	else if(ubItemCurIdx == HIDE_CHANNEL_NAME_ITEM)
	{
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_SYSTEM_HIDE_CHANNEL_NAME_OFF_NOR , 4, &tOsdImgInfo[0]);
		if(tUI_CuSetting.ubHideChannelName)
			tOSD_Img2(&tOsdImgInfo[3], OSD_UPDATE);
		else
			tOSD_Img2(&tOsdImgInfo[1], OSD_UPDATE);
	}
	else if(ubItemCurIdx == DATETIME_ITEM)
	{
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_SYSTEM_DATE_TIME_HL , 1, &tOsdImgInfo[0]);
		tOSD_Img2(&tOsdImgInfo[0], OSD_UPDATE);
	}
}
//------------------------------------------------------------------------------------
void UI_SystemSubSubMenuExit()
{
	OSD_IMG_INFO tOsdImgInfo;
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
	tOSD_Img2(&tOsdImgInfosub[2*SYSTEMSET_ITEM + UI_ICON_HIGHLIGHT], OSD_UPDATE);//highlight
	
	SystemSubSubRemoteMode = REMOTE_SELECT_ITEM;
	tUI_State = UI_SUBMENU_STATE;
	
	UI_UpdateDevStatusInfo();

}
//------------------------------------------------------------------------------------
void UI_AISubSubMenuExit()
{
	OSD_IMG_INFO tOsdImgInfo;
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
	tOSD_Img2(&tOsdImgInfosub[2*AI_ITEM + UI_ICON_HIGHLIGHT], OSD_UPDATE);//highlight
	
	tUI_State = UI_SUBMENU_STATE;
	
	UI_UpdateDevStatusInfo();
}

//------------------------------------------------------------------------------------
extern uint8_t ISPlaying_wav;
void UI_AISubSubMenuPage(UI_ArrowKey_t tArrowKey)
{
	UI_MenuAct_t tMenuAct;
	OSD_IMG_INFO tOsdImgInfo[12],tOnOffOsdImgInfo[4],tEraserOsdImgInfo;
	tSettingSubSubMenuItem.tSettingS[10].tSubMenuInfo.ubItemPreIdx = tSettingSubSubMenuItem.tSettingS[10].tSubMenuInfo.ubItemIdx;

	if(SystemSubSubRemoteMode == REMOTE_SELECT_ITEM)
	{
		tMenuAct = UI_KeyEventMap2SubSubMenuInfo(&tArrowKey, &tSettingSubSubMenuItem.tSettingS[10]);//AI
		switch(tMenuAct)
		{
			case DRAW_HIGHLIGHT_MENUICON:
				UI_AIDrawSubSubMenuItem(TRUE);
				break;
			case EXECUTE_MENUFUNC:
				uint8_t ubItemCurIdx = tSettingSubSubMenuItem.tSettingS[10].tSubMenuInfo.ubItemIdx;
				printf("**************************************************%d\n",ubItemCurIdx);
//				if(ubItemCurIdx <= AI_BSD_CAM4)
//				{
//					tUI_CuSetting.ubIsEnableBSD[ubItemCurIdx%4] = 1- tUI_CuSetting.ubIsEnableBSD[ubItemCurIdx%4];
//				}
				if(ubItemCurIdx <= AI_PD_CAM4)
				{
					tUI_CuSetting.ubDetectPeopleFlag[ubItemCurIdx%4] = 1- tUI_CuSetting.ubDetectPeopleFlag[ubItemCurIdx%4];
				}
				else if(ubItemCurIdx <= AI_CD_CAM4)
				{
					tUI_CuSetting.ubDetectCarFlag[ubItemCurIdx%4] = 1- tUI_CuSetting.ubDetectCarFlag[ubItemCurIdx%4];

				}
				else if(ubItemCurIdx <= AI_ALARM_CAM4)
				{
					tUI_CuSetting.ubIsEnableBSDALARM[ubItemCurIdx%4] = 1- tUI_CuSetting.ubIsEnableBSDALARM[ubItemCurIdx%4];
				}
				else if(ubItemCurIdx <= AI_TRIGGER_CAM4)
				{
					tUI_CuSetting.ubBSDTriggerOut[ubItemCurIdx%4] = 1- tUI_CuSetting.ubBSDTriggerOut[ubItemCurIdx%4];
				}
				else if(ubItemCurIdx <= AI_BSDRANGE_CAM4)
				{	
					UI_AISubSubMenuExecute(ubItemCurIdx,1);
					UI_AIDrawSubSubSubMenuItem(TRUE);
			 		break;
				}
				UI_AIDrawSubSubMenuItem(TRUE);	
				break;
			case EXIT_MENUFUNC:
				UI_AISubSubMenuExit();
				break;
			default:
				break;
		}

	}

}
//------------------------------------------------------------------------------------
void UI_SystemSubSubMenuPage(UI_ArrowKey_t tArrowKey)
{
	UI_MenuAct_t tMenuAct;
	tSystemSubSubMenuItem.tSubMenuInfo.ubItemPreIdx = tSystemSubSubMenuItem.tSubMenuInfo.ubItemIdx;
	if(SystemSubSubRemoteMode == REMOTE_SELECT_ITEM)
	{
		tMenuAct = UI_KeyEventMap2SubSubMenuInfo(&tArrowKey, &tSystemSubSubMenuItem);
		switch(tMenuAct)
		{
			case DRAW_HIGHLIGHT_MENUICON:
				UI_SystemDrawSubSubMenuItem(TRUE);
				break;
			case EXECUTE_MENUFUNC:
				{
					uint8_t ubItemCurIdx = tSystemSubSubMenuItem.tSubMenuInfo.ubItemIdx;
					if(ubItemCurIdx == DIMMER_ITEM)
					{
						SendIrCodeFlag = 1;//支持连发
						SystemSubSubRemoteMode = 1- SystemSubSubRemoteMode;
						UI_SystemDrawSubSubMenuItem(TRUE);
					}
					else
					{
						if(ubItemCurIdx == AUTO_ONOFF_ITEM)
							tUI_CuSetting.ubAutoDimmer = 1- tUI_CuSetting.ubAutoDimmer;
						else if(ubItemCurIdx == MENULOCK_ITEM)
						{
							tUI_CuSetting.ubMenuLock = 1- tUI_CuSetting.ubMenuLock;
						}
						else if(ubItemCurIdx == BUZZER_ITEM)
						{
							tUI_CuSetting.ubBuzzer = 1 - tUI_CuSetting.ubBuzzer;
						}
						else if(ubItemCurIdx == SD_SWITCH_ITEM)
							tUI_CuSetting.ubSD_Switch = 1- tUI_CuSetting.ubSD_Switch;
						else if(ubItemCurIdx == HIDE_CHANNEL_NAME_ITEM)
							tUI_CuSetting.ubHideChannelName = 1- tUI_CuSetting.ubHideChannelName;
						
						UI_SystemDrawSubSubMenuItem(TRUE);
						UI_SystemSubSubMenuExecute();
					}
				}
				break;
			case EXIT_MENUFUNC:
				UI_SystemSubSubMenuExit();
				break;
			default:
				break;
		}

	}
	else
	{
		uint8_t ubItemCurIdx = tSystemSubSubMenuItem.tSubMenuInfo.ubItemIdx;
		switch(tArrowKey)
		{
			case LEFT_ARROW:
				if(ubItemCurIdx == DIMMER_ITEM)
				{
					if(tUI_CuSetting.BriLvL.ulBL_UpdateLvL > 0)
						tUI_CuSetting.BriLvL.ulBL_UpdateLvL --;
					else
						return;
				}
				break;
			case RIGHT_ARROW:
				if(ubItemCurIdx == DIMMER_ITEM)
				{
					if(tUI_CuSetting.BriLvL.ulBL_UpdateLvL < 99)
						tUI_CuSetting.BriLvL.ulBL_UpdateLvL ++;
					else
						return;
				}
				break;
			case ENTER_ARROW:
				SendIrCodeFlag = 0;//禁止连发
				SystemSubSubRemoteMode = 1 - SystemSubSubRemoteMode;
				UI_SystemDrawSubSubMenuItem(TRUE);
				return;
			case EXIT_ARROW:
				UI_SystemSubSubMenuExit();
				return;
			default:
				return;
		}
		UI_SystemDrawSubSubMenuItem(FALSE);
		UI_SystemSubSubMenuExecute();
	}
}

//------------------------------------------------------------------------------------dual------------------------------------------------------------------------------

//------------------------------------------------------------------------------
void UI_DualSubSubMenuExit()
{
	OSD_IMG_INFO tOsdImgInfo;
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
	tOSD_Img2(&tOsdImgInfosub[2*DUALSET_ITEM + UI_ICON_HIGHLIGHT], OSD_UPDATE);//highlight
	
	tUI_State = UI_SUBMENU_STATE;
	
	UI_UpdateDevStatusInfo();
	UI_SwitchViewType(DeskTopShowView,FALSE);

}
//------------------------------------------------------------------------------
void UI_DualDrawSubSubMenuItem()
{
	OSD_IMG_INFO tCamOsdImgInfo[16];
	uint8_t ubItemPreIdx = tDualSubSubMenuItem.tSubMenuInfo.ubItemPreIdx;
	uint8_t ubItemCurIdx = tDualSubSubMenuItem.tSubMenuInfo.ubItemIdx;
	uint16_t uwLcd_HSize = uwLCD_GetLcdHoSize();
	tOSD_GetOsdImgInfor(1, OSD_IMG2, (tUI_CuSetting.tLanguage != LANGUAGE_GERMAN)?OSD2IMG_DUAL_LEFT_CAM1NOR:OSD2IMG_DUAL_LEFT_CAM1NOR_GER, 16, &tCamOsdImgInfo[0]);
	//---------------------------------------pre------------------------------------------
	if(ubItemPreIdx != ubItemCurIdx)
	{
		if(ubItemPreIdx <= DUAL_LEFT_CAM4)//left
		{
			if(tUI_CamStatus[ubItemPreIdx].tCamDispLocation_Dual == DISP_LEFT)//文字高亮
				tOSD_Img2(&tCamOsdImgInfo[8 + 2*ubItemPreIdx], OSD_QUEUE);
			else
				tOSD_Img2(&tCamOsdImgInfo[2*ubItemPreIdx], OSD_QUEUE);
		}
		else//right
		{
			ubItemPreIdx -= 4;
			if(tUI_CamStatus[ubItemPreIdx].tCamDispLocation_Dual == DISP_RIGHT)//文字高亮
			{
				tCamOsdImgInfo[8 + 2*ubItemPreIdx].uwXStart += uwLcd_HSize/2;
				tOSD_Img2(&tCamOsdImgInfo[8 + 2*ubItemPreIdx], OSD_QUEUE);
			}
			else
			{
				tCamOsdImgInfo[2*ubItemPreIdx].uwXStart += uwLcd_HSize/2;
				tOSD_Img2(&tCamOsdImgInfo[2*ubItemPreIdx], OSD_QUEUE);
			}
		}
	}
	//---------------------------------------cur------------------------------------------
	if(ubItemCurIdx <= DUAL_LEFT_CAM4)//left
	{
		if(tUI_CamStatus[ubItemCurIdx].tCamDispLocation_Dual == DISP_LEFT)//文字高亮
			tOSD_Img2(&tCamOsdImgInfo[8 + 2*ubItemCurIdx + UI_ICON_HIGHLIGHT], OSD_UPDATE);
		else
			tOSD_Img2(&tCamOsdImgInfo[2*ubItemCurIdx + UI_ICON_HIGHLIGHT], OSD_UPDATE);
	}
	else//right
	{
		ubItemCurIdx -= 4;
		if(tUI_CamStatus[ubItemCurIdx].tCamDispLocation_Dual == DISP_RIGHT)//文字高亮
		{
			tCamOsdImgInfo[8 + 2*ubItemCurIdx + UI_ICON_HIGHLIGHT].uwXStart += uwLcd_HSize/2;;
			tOSD_Img2(&tCamOsdImgInfo[8 + 2*ubItemCurIdx + UI_ICON_HIGHLIGHT], OSD_UPDATE);
		}
		else
		{
			tCamOsdImgInfo[2*ubItemCurIdx + UI_ICON_HIGHLIGHT].uwXStart += uwLcd_HSize/2;;
			tOSD_Img2(&tCamOsdImgInfo[2*ubItemCurIdx + UI_ICON_HIGHLIGHT], OSD_UPDATE);
		}
	}
	
}
//------------------------------------------------------------------------------
void UI_DualSubSubMenuExecute()
{
	uint16_t uwLcd_HSize = uwLCD_GetLcdHoSize();
	uint8_t ubItemCurIdx = tDualSubSubMenuItem.tSubMenuInfo.ubItemIdx;
	if(ubItemCurIdx <= DUAL_LEFT_CAM4)//left
	{
		/*交换位置
		* 找到位置为DISP_LEFT的tx，把该tx的位置改为当前选中的tx的位置
		* 然后当前选中的tx的位置变为DISP_LEFT
		*/
		for(uint8_t i = 0;i < 4;i++)
		{
			if(tUI_CamStatus[i].tCamDispLocation_Dual == DISP_LEFT)
				tUI_CamStatus[i].tCamDispLocation_Dual = tUI_CamStatus[ubItemCurIdx].tCamDispLocation_Dual;
		}
		tUI_CamStatus[ubItemCurIdx].tCamDispLocation_Dual = DISP_LEFT;
	}
	else//right
	{
		for(uint8_t i = 0;i < 4;i++)
		{
			if(tUI_CamStatus[i].tCamDispLocation_Dual == DISP_RIGHT)
				tUI_CamStatus[i].tCamDispLocation_Dual = tUI_CamStatus[ubItemCurIdx - 4].tCamDispLocation_Dual;
		}
		tUI_CamStatus[ubItemCurIdx - 4].tCamDispLocation_Dual = DISP_RIGHT;

	}


	//刷新界面
	OSD_IMG_INFO tCamOsdImgInfo[16];
	tOSD_GetOsdImgInfor(1, OSD_IMG2, (tUI_CuSetting.tLanguage != LANGUAGE_GERMAN)?OSD2IMG_DUAL_LEFT_CAM1NOR:OSD2IMG_DUAL_LEFT_CAM1NOR_GER, 16, &tCamOsdImgInfo[0]);
	for(uint8_t i = 0;i < 4;i++)
	{
		//left
		if(tUI_CamStatus[i].tCamDispLocation_Dual == DISP_LEFT)
		{
			tOSD_Img2(&tCamOsdImgInfo[8 + 2*i], OSD_QUEUE);
		}
		else
		{
			tOSD_Img2(&tCamOsdImgInfo[2*i], OSD_QUEUE);
		}
		//right
		if(tUI_CamStatus[i].tCamDispLocation_Dual == DISP_RIGHT)
		{
			tCamOsdImgInfo[8 + 2*i].uwXStart += uwLcd_HSize/2;
			tOSD_Img2(&tCamOsdImgInfo[8 + 2*i], OSD_QUEUE);
		}
		else
		{
			tCamOsdImgInfo[2*i].uwXStart += uwLcd_HSize/2;
			tOSD_Img2(&tCamOsdImgInfo[2*i], OSD_QUEUE);
		}
	}
	//highLight
	if(ubItemCurIdx <= DUAL_LEFT_CAM4)
		tOSD_Img2(&tCamOsdImgInfo[8 + 2*ubItemCurIdx + UI_ICON_HIGHLIGHT], OSD_UPDATE);
	else
	{
		tCamOsdImgInfo[8 + 2*(ubItemCurIdx - 4) + UI_ICON_HIGHLIGHT].uwXStart += uwLcd_HSize/2;
		tOSD_Img2(&tCamOsdImgInfo[8 + 2*(ubItemCurIdx - 4) + UI_ICON_HIGHLIGHT], OSD_UPDATE);
	}
	
	UI_SwitchViewType(DUALVIEW_ITEM,TRUE);

	
}

//------------------------------------------------------------------------------
void UI_DualSubSubMenuPage(UI_ArrowKey_t tArrowKey)
{
	UI_MenuAct_t tMenuAct;
	tDualSubSubMenuItem.tSubMenuInfo.ubItemPreIdx = tDualSubSubMenuItem.tSubMenuInfo.ubItemIdx;
	tMenuAct = UI_KeyEventMap2SubSubMenuInfo(&tArrowKey, &tDualSubSubMenuItem);
	switch(tMenuAct)
	{
		case DRAW_HIGHLIGHT_MENUICON:
			UI_DualDrawSubSubMenuItem();
			break;
		case EXECUTE_MENUFUNC:
			UI_DualSubSubMenuExecute();
			break;
		case EXIT_MENUFUNC:
			UI_DualSubSubMenuExit();
			break;
		default:
			break;
	}

}
//------------------------------------------------------------------------------------quad------------------------------------------------------------------------------

//------------------------------------------------------------------------------
void UI_QuadSubSubMenuExit()
{
	OSD_IMG_INFO tOsdImgInfo;
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
	tOSD_Img2(&tOsdImgInfosub[2*QUADSET_ITEM + UI_ICON_HIGHLIGHT], OSD_UPDATE);//highlight
	
	tUI_State = UI_SUBMENU_STATE;
	
	UI_UpdateDevStatusInfo();
	UI_SwitchViewType(DeskTopShowView,FALSE);

}
//------------------------------------------------------------------------------
void UI_QuadDrawSubSubMenuItem()
{
	OSD_IMG_INFO tCamOsdImgInfo[16];
	uint16_t uwLcd_HSize = uwLCD_GetLcdHoSize();
	uint16_t uwLcd_VSize = uwLCD_GetLcdVoSize();
	uint8_t ubItemIdx[2];
	ubItemIdx[0] = tQuadSubSubMenuItem.tSubMenuInfo.ubItemPreIdx;
	ubItemIdx[1] = tQuadSubSubMenuItem.tSubMenuInfo.ubItemIdx;
	for(uint8_t i = 0;i < 2;i++)
	{
		tOSD_GetOsdImgInfor(1, OSD_IMG2, (tUI_CuSetting.tLanguage != LANGUAGE_GERMAN)?OSD2IMG_DUAL_LEFT_CAM1NOR:OSD2IMG_DUAL_LEFT_CAM1NOR_GER, 16, &tCamOsdImgInfo[0]);
	
		if(ubItemIdx[i] <= QUAD_UPPER_LEFT_CAM4)//upper left
		{
			if(tUI_CamStatus[ubItemIdx[i]].tCamDispLocation_Quad == DISP_UPPER_LEFT)//文字高亮
			{
				tCamOsdImgInfo[8 + 2*ubItemIdx[i] + i].uwYStart -= uwLcd_VSize/4;
				tOSD_Img2(&tCamOsdImgInfo[8 + 2*ubItemIdx[i] + i], (i == 1)?OSD_UPDATE:OSD_QUEUE);
			}
			else
			{
				tCamOsdImgInfo[2*ubItemIdx[i] + i].uwYStart -= uwLcd_VSize/4;
				tOSD_Img2(&tCamOsdImgInfo[2*ubItemIdx[i] + i], (i == 1)?OSD_UPDATE:OSD_QUEUE);
			}
		}
		else if(ubItemIdx[i] <= QUAD_UPPER_RIGHT_CAM4)//upper right
		{
			ubItemIdx[i] -= 4;
			if(tUI_CamStatus[ubItemIdx[i]].tCamDispLocation_Quad == DISP_UPPER_RIGHT)//文字高亮
			{
				tCamOsdImgInfo[8 + 2*ubItemIdx[i] + i].uwXStart += uwLcd_HSize/2;
				tCamOsdImgInfo[8 + 2*ubItemIdx[i] + i].uwYStart -= uwLcd_VSize/4;
				tOSD_Img2(&tCamOsdImgInfo[8 + 2*ubItemIdx[i] + i], (i == 1)?OSD_UPDATE:OSD_QUEUE);
			}
			else
			{
				tCamOsdImgInfo[2*ubItemIdx[i] + i].uwXStart += uwLcd_HSize/2;
				tCamOsdImgInfo[2*ubItemIdx[i] + i].uwYStart -= uwLcd_VSize/4;
				tOSD_Img2(&tCamOsdImgInfo[2*ubItemIdx[i] + i], (i == 1)?OSD_UPDATE:OSD_QUEUE);
			}
		}
		else if(ubItemIdx[i] <= QUAD_LOWER_LEFT_CAM4)//lower left
		{
			ubItemIdx[i] -= 8;
			if(tUI_CamStatus[ubItemIdx[i]].tCamDispLocation_Quad == DISP_LOWER_LEFT)//文字高亮
			{
				tCamOsdImgInfo[8 + 2*ubItemIdx[i] + i].uwYStart += uwLcd_VSize/4;
				tOSD_Img2(&tCamOsdImgInfo[8 + 2*ubItemIdx[i] + i], (i == 1)?OSD_UPDATE:OSD_QUEUE);
			}
			else
			{
				tCamOsdImgInfo[2*ubItemIdx[i] + i].uwYStart += uwLcd_VSize/4;
				tOSD_Img2(&tCamOsdImgInfo[2*ubItemIdx[i] + i], (i == 1)?OSD_UPDATE:OSD_QUEUE);
			}
		}
		else										//lower right
		{
			ubItemIdx[i] -= 12;
			if(tUI_CamStatus[ubItemIdx[i]].tCamDispLocation_Quad == DISP_LOWER_RIGHT)//文字高亮
			{
				tCamOsdImgInfo[8 + 2*ubItemIdx[i] + i].uwXStart += uwLcd_HSize/2;
				tCamOsdImgInfo[8 + 2*ubItemIdx[i] + i].uwYStart += uwLcd_VSize/4;
				tOSD_Img2(&tCamOsdImgInfo[8 + 2*ubItemIdx[i] + i], (i == 1)?OSD_UPDATE:OSD_QUEUE);
			}
			else
			{
				tCamOsdImgInfo[2*ubItemIdx[i] + i].uwXStart += uwLcd_HSize/2;
				tCamOsdImgInfo[2*ubItemIdx[i] + i].uwYStart += uwLcd_VSize/4;
				tOSD_Img2(&tCamOsdImgInfo[2*ubItemIdx[i] + i], (i == 1)?OSD_UPDATE:OSD_QUEUE);
			}

		}
	
	}
	
}
//------------------------------------------------------------------------------
void UI_QuadSubSubMenuExecute(void)
{
	uint16_t uwLcd_HSize = uwLCD_GetLcdHoSize();
	uint16_t uwLcd_VSize = uwLCD_GetLcdVoSize();
	
	uint8_t ubItemCurIdx = tQuadSubSubMenuItem.tSubMenuInfo.ubItemIdx;
	//画面位置设置
	if(ubItemCurIdx <= QUAD_UPPER_LEFT_CAM4)
	{
		for(uint8_t i = 0;i < 4;i++)
		{
			if(tUI_CamStatus[i].tCamDispLocation_Quad == DISP_UPPER_LEFT)
				tUI_CamStatus[i].tCamDispLocation_Quad = tUI_CamStatus[ubItemCurIdx].tCamDispLocation_Quad;
		}
		tUI_CamStatus[ubItemCurIdx].tCamDispLocation_Quad = DISP_UPPER_LEFT;
	}
	else if(ubItemCurIdx <= QUAD_UPPER_RIGHT_CAM4)
	{
		for(uint8_t i = 0;i < 4;i++)
		{
			if(tUI_CamStatus[i].tCamDispLocation_Quad == DISP_UPPER_RIGHT)
				tUI_CamStatus[i].tCamDispLocation_Quad = tUI_CamStatus[ubItemCurIdx - 4].tCamDispLocation_Quad;
		}
		tUI_CamStatus[ubItemCurIdx - 4].tCamDispLocation_Quad = DISP_UPPER_RIGHT;
	}
	else if(ubItemCurIdx <= QUAD_LOWER_LEFT_CAM4)
	{
		for(uint8_t i = 0;i < 4;i++)
		{
			if(tUI_CamStatus[i].tCamDispLocation_Quad == DISP_LOWER_LEFT)
				tUI_CamStatus[i].tCamDispLocation_Quad = tUI_CamStatus[ubItemCurIdx - 8].tCamDispLocation_Quad;
		}
		tUI_CamStatus[ubItemCurIdx - 8].tCamDispLocation_Quad = DISP_LOWER_LEFT;
	}
	else if(ubItemCurIdx <= QUAD_LOWER_RIGHT_CAM4)
	{
		for(uint8_t i = 0;i < 4;i++)
		{
			if(tUI_CamStatus[i].tCamDispLocation_Quad == DISP_LOWER_RIGHT)
				tUI_CamStatus[i].tCamDispLocation_Quad = tUI_CamStatus[ubItemCurIdx - 12].tCamDispLocation_Quad;
		}
		tUI_CamStatus[ubItemCurIdx - 12].tCamDispLocation_Quad = DISP_LOWER_RIGHT;
	}

	//刷新界面
	OSD_IMG_INFO tCamOsdImgInfo[16];
	
	for(uint8_t i = 0;i < 4;i++)
	{
		//upper left
		tOSD_GetOsdImgInfor(1, OSD_IMG2, (tUI_CuSetting.tLanguage != LANGUAGE_GERMAN)?OSD2IMG_DUAL_LEFT_CAM1NOR:OSD2IMG_DUAL_LEFT_CAM1NOR_GER, 16, &tCamOsdImgInfo[0]);
		if(tUI_CamStatus[i].tCamDispLocation_Quad == DISP_UPPER_LEFT)
		{
			tCamOsdImgInfo[8 + 2*i].uwYStart -= uwLcd_VSize/4;
			tOSD_Img2(&tCamOsdImgInfo[8 + 2*i], OSD_QUEUE);
		}
		else
		{
			tCamOsdImgInfo[2*i].uwYStart -= uwLcd_VSize/4;
			tOSD_Img2(&tCamOsdImgInfo[2*i], OSD_QUEUE);
		}
		//upper right
		tOSD_GetOsdImgInfor(1, OSD_IMG2, (tUI_CuSetting.tLanguage != LANGUAGE_GERMAN)?OSD2IMG_DUAL_LEFT_CAM1NOR:OSD2IMG_DUAL_LEFT_CAM1NOR_GER, 16, &tCamOsdImgInfo[0]);
		if(tUI_CamStatus[i].tCamDispLocation_Quad == DISP_UPPER_RIGHT)
		{
			tCamOsdImgInfo[8 + 2*i].uwXStart += uwLcd_HSize/2;
			tCamOsdImgInfo[8 + 2*i].uwYStart -= uwLcd_VSize/4;
			tOSD_Img2(&tCamOsdImgInfo[8 + 2*i], OSD_QUEUE);
		}
		else
		{
			tCamOsdImgInfo[2*i].uwXStart += uwLcd_HSize/2;
			tCamOsdImgInfo[2*i].uwYStart -= uwLcd_VSize/4;
			tOSD_Img2(&tCamOsdImgInfo[2*i], OSD_QUEUE);
		}
		//lower left
		tOSD_GetOsdImgInfor(1, OSD_IMG2, (tUI_CuSetting.tLanguage != LANGUAGE_GERMAN)?OSD2IMG_DUAL_LEFT_CAM1NOR:OSD2IMG_DUAL_LEFT_CAM1NOR_GER, 16, &tCamOsdImgInfo[0]);
		if(tUI_CamStatus[i].tCamDispLocation_Quad == DISP_LOWER_LEFT)
		{
			tCamOsdImgInfo[8 + 2*i].uwYStart += uwLcd_VSize/4;
			tOSD_Img2(&tCamOsdImgInfo[8 + 2*i], OSD_QUEUE);
		}
		else
		{
			tCamOsdImgInfo[2*i].uwYStart += uwLcd_VSize/4;
			tOSD_Img2(&tCamOsdImgInfo[2*i], OSD_QUEUE);
		}
		//lower right
		tOSD_GetOsdImgInfor(1, OSD_IMG2, (tUI_CuSetting.tLanguage != LANGUAGE_GERMAN)?OSD2IMG_DUAL_LEFT_CAM1NOR:OSD2IMG_DUAL_LEFT_CAM1NOR_GER, 16, &tCamOsdImgInfo[0]);
		if(tUI_CamStatus[i].tCamDispLocation_Quad == DISP_LOWER_RIGHT)
		{
			tCamOsdImgInfo[8 + 2*i].uwXStart += uwLcd_HSize/2;
			tCamOsdImgInfo[8 + 2*i].uwYStart += uwLcd_VSize/4;
			tOSD_Img2(&tCamOsdImgInfo[8 + 2*i], OSD_QUEUE);
		}
		else
		{
			tCamOsdImgInfo[2*i].uwXStart += uwLcd_HSize/2;
			tCamOsdImgInfo[2*i].uwYStart += uwLcd_VSize/4;
			tOSD_Img2(&tCamOsdImgInfo[2*i], OSD_QUEUE);
		}
	}
	//highLight
	tOSD_GetOsdImgInfor(1, OSD_IMG2, (tUI_CuSetting.tLanguage != LANGUAGE_GERMAN)?OSD2IMG_DUAL_LEFT_CAM1NOR:OSD2IMG_DUAL_LEFT_CAM1NOR_GER, 16, &tCamOsdImgInfo[0]);
	if(ubItemCurIdx <= QUAD_UPPER_LEFT_CAM4)
	{
		tCamOsdImgInfo[8 + 2*ubItemCurIdx + UI_ICON_HIGHLIGHT].uwYStart -= uwLcd_VSize/4;
		tOSD_Img2(&tCamOsdImgInfo[8 + 2*ubItemCurIdx + UI_ICON_HIGHLIGHT], OSD_UPDATE);
	}
	else if(ubItemCurIdx <= QUAD_UPPER_RIGHT_CAM4)
	{
		tCamOsdImgInfo[8 + 2*(ubItemCurIdx - 4) + UI_ICON_HIGHLIGHT].uwYStart -= uwLcd_VSize/4;
		tCamOsdImgInfo[8 + 2*(ubItemCurIdx - 4) + UI_ICON_HIGHLIGHT].uwXStart += uwLcd_HSize/2;
		tOSD_Img2(&tCamOsdImgInfo[8 + 2*(ubItemCurIdx - 4) + UI_ICON_HIGHLIGHT], OSD_UPDATE);
	}
	else if(ubItemCurIdx <= QUAD_LOWER_LEFT_CAM4)
	{
		tCamOsdImgInfo[8 + 2*(ubItemCurIdx - 8) + UI_ICON_HIGHLIGHT].uwYStart += uwLcd_VSize/4;
		tOSD_Img2(&tCamOsdImgInfo[8 + 2*(ubItemCurIdx - 8) + UI_ICON_HIGHLIGHT], OSD_UPDATE);
	}
	else
	{
		tCamOsdImgInfo[8 + 2*(ubItemCurIdx - 12) + UI_ICON_HIGHLIGHT].uwXStart += uwLcd_HSize/2;
		tCamOsdImgInfo[8 + 2*(ubItemCurIdx - 12) + UI_ICON_HIGHLIGHT].uwYStart += uwLcd_VSize/4;
		tOSD_Img2(&tCamOsdImgInfo[8 + 2*(ubItemCurIdx - 12) + UI_ICON_HIGHLIGHT], OSD_UPDATE);
	}
	
	UI_SwitchViewType(QUALVIEW_ITEM,TRUE);
}


//------------------------------------------------------------------------------
void UI_QuadSubSubMenuPage(UI_ArrowKey_t tArrowKey)
{
	UI_MenuAct_t tMenuAct;
	tMenuAct = UI_KeyEventMap2SubSubMenuInfo(&tArrowKey, &tQuadSubSubMenuItem);
	switch(tMenuAct)
	{
		case DRAW_HIGHLIGHT_MENUICON:
			UI_QuadDrawSubSubMenuItem();
			break;
		case EXECUTE_MENUFUNC:
			UI_QuadSubSubMenuExecute();
			break;
		case EXIT_MENUFUNC:
			UI_QuadSubSubMenuExit();
			break;
		default:
			break;
	}

}

//------------------------------------------------------------------------------------Power On------------------------------------------------------------------------------
uint8_t ubUI_PowerOnSubSubStsUpdateFlag = FALSE;

void UI_PowerOnDrawSubSubMenuItem(void)
{
	uint16_t uwPowerOnMenuItemOsdImg[POWERON_MAX - 1] = {
														OSD2IMG_POWERON_CAM1_NOR,
														OSD2IMG_POWERON_CAM2_NOR,
														OSD2IMG_POWERON_CAM3_NOR,
														OSD2IMG_POWERON_CAM4_NOR,
														OSD2IMG_POWERON_DUAL_NOR,
														OSD2IMG_POWERON_QUAD_NOR,
														OSD2IMG_POWERON_SCAN_NOR,
														OSD2IMG_POWERON_STANDBY_NOR,
														};
	uint8_t ubPreIdx = tPowerOnSubSubMenuItem.tSubMenuInfo.ubItemPreIdx;
	uint8_t ubCurIdx = tPowerOnSubSubMenuItem.tSubMenuInfo.ubItemIdx;
	if(tUI_CuSetting.tPowerOnMode != POWERON_LAST)
		UI_DrawHLandNormalIcon(uwPowerOnMenuItemOsdImg[ubPreIdx], (uwPowerOnMenuItemOsdImg[ubCurIdx] + UI_ICON_HIGHLIGHT));
	else
		UI_DrawHLandNormalIcon(uwPowerOnMenuItemOsdImg[ubPreIdx], uwPowerOnMenuItemOsdImg[ubCurIdx]);

}

//------------------------------------------------------------------------------
void UI_PowerOnSubSubMenuExit(void)
{
	OSD_IMG_INFO tOsdImgInfo;
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
	tOSD_Img2(&tOsdImgInfosub[2*POWERONSET_ITEM + UI_ICON_HIGHLIGHT], OSD_UPDATE);//highlight
	
	tUI_State = UI_SUBMENU_STATE;
	
	UI_UpdateDevStatusInfo();
	ubUI_PowerOnSubSubStsUpdateFlag = FALSE;
}


//------------------------------------------------------------------------------
void UI_PowerOnSubSubMenuPage(UI_ArrowKey_t tArrowKey)
{
	if(!ubUI_PowerOnSubSubStsUpdateFlag)
	{
		ubUI_PowerOnSubSubStsUpdateFlag = TRUE;
		tPowerOnSubSubMenuItem.tSubMenuInfo.ubItemIdx = tUI_CuSetting.tOldPowerOnMode;
	}
	tPowerOnSubSubMenuItem.tSubMenuInfo.ubItemPreIdx = tPowerOnSubSubMenuItem.tSubMenuInfo.ubItemIdx;
	switch(tArrowKey)
	{
		case LEFT_ARROW:
			if(tUI_CuSetting.tPowerOnMode == POWERON_LAST)
				tUI_CuSetting.tPowerOnMode = tUI_CuSetting.tOldPowerOnMode;
			
			if(tPowerOnSubSubMenuItem.tSubMenuInfo.ubItemIdx > POWERON_CAM1)	
				tPowerOnSubSubMenuItem.tSubMenuInfo.ubItemIdx --;
			else
				tPowerOnSubSubMenuItem.tSubMenuInfo.ubItemIdx = POWERON_STANDBY;
			tUI_CuSetting.tPowerOnMode = tUI_CuSetting.tOldPowerOnMode  = tPowerOnSubSubMenuItem.tSubMenuInfo.ubItemIdx;
			break;
		case RIGHT_ARROW:
			if(tUI_CuSetting.tPowerOnMode == POWERON_LAST)
				tUI_CuSetting.tPowerOnMode = tUI_CuSetting.tOldPowerOnMode;
			
			if(tPowerOnSubSubMenuItem.tSubMenuInfo.ubItemIdx < POWERON_STANDBY)	
				tPowerOnSubSubMenuItem.tSubMenuInfo.ubItemIdx ++;
			else
				tPowerOnSubSubMenuItem.tSubMenuInfo.ubItemIdx = POWERON_CAM1;
			
			tUI_CuSetting.tPowerOnMode = tUI_CuSetting.tOldPowerOnMode  = tPowerOnSubSubMenuItem.tSubMenuInfo.ubItemIdx;
			break;
			
		case ENTER_ARROW:
			if(tUI_CuSetting.tPowerOnMode == POWERON_LAST)
			{
				tUI_CuSetting.tPowerOnMode = tUI_CuSetting.tOldPowerOnMode;
			}
			else
			{
				tUI_CuSetting.tPowerOnMode = POWERON_LAST;
			}
			break;
			
		case EXIT_ARROW:
			UI_PowerOnSubSubMenuExit();

			return;
		default:
			return;
	}
	UI_PowerOnDrawSubSubMenuItem();


}


//setting的大部分界面的操作逻辑都差不多，所以没有特殊需求的界面都使用同一套遥控解析代码
//------------------------------------------------------------------------------------universal------------------------------------------------------------------------------

//------------------------------------------------------------------------------
static void UI_SettingingDrawSubSubMenuItemValue(uint16_t uwSubMenuItem)
{
	uint8_t ubItemPreIdx = tSettingSubSubMenuItem.tSettingS[uwSubMenuItem].tSubMenuInfo.ubItemPreIdx;
	uint8_t ubItemIdx	 = tSettingSubSubMenuItem.tSettingS[uwSubMenuItem].tSubMenuInfo.ubItemIdx;

	if(uwSubMenuItem == TRIGGERSET_ITEM)
	{
		OSD_IMG_INFO tOsdImgInfo_Button[22];
		//button
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_TRIGGER_DELAY_CAM1_NOR, 22, &tOsdImgInfo_Button[0]);
		//show normal number
		uint8_t i = ubItemPreIdx;
		if(ubItemPreIdx <= TRIGGER_DELAY_CAMSPLIT)
		{
			UI_ShowButtonValueNormal(tUI_CuSetting.TriggerDelay[i],
				tOsdImgInfo_Button[2*i].uwXStart + ((tUI_CuSetting.TriggerDelay[i] < 10)?40:30),tOsdImgInfo_Button[2*i].uwYStart + 15,OSD_UPDATE);
		}
		else if(ubItemPreIdx <= TRIGGER_PRIORITY_CAMSPLIT)
		{
			UI_ShowButtonValueNormal(tUI_CuSetting.TriggerPriority[i - 5] + 1,
				tOsdImgInfo_Button[2*i].uwXStart + 40,tOsdImgInfo_Button[2*i].uwYStart + 15,OSD_UPDATE);
		}
		//show highLight Number	
		i = ubItemIdx;
		if(ubItemIdx <= TRIGGER_DELAY_CAMSPLIT)
		{
			UI_ShowButtonValueHighLight(tUI_CuSetting.TriggerDelay[i],
				tOsdImgInfo_Button[2*i].uwXStart + ((tUI_CuSetting.TriggerDelay[i] < 10)?40:30),tOsdImgInfo_Button[2*i].uwYStart + 15,OSD_UPDATE);
		}
		else if(ubItemIdx <= TRIGGER_PRIORITY_CAMSPLIT)
		{
			UI_ShowButtonValueHighLight(tUI_CuSetting.TriggerPriority[i - 5] + 1,
				tOsdImgInfo_Button[2*i].uwXStart + 40,tOsdImgInfo_Button[2*i].uwYStart + 15,OSD_UPDATE);
		}
		
	}
	else if(uwSubMenuItem == AUTOSCANSET_ITEM)
	{
		OSD_IMG_INFO tOsdImgInfo[14];
		uint8_t i;
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_AUTOSCAN_DELAY_CAM1_NOR, 14, &tOsdImgInfo[0]);
		if(ubItemPreIdx %2)
		{
			i = ubItemPreIdx / 2;
			UI_ShowButtonValueNormal(tUI_CuSetting.AutoScanDuty[i],
				tOsdImgInfo[2*i].uwXStart + ((tUI_CuSetting.AutoScanDuty[i] < 10)?40:30),tOsdImgInfo[2*i].uwYStart + 15,OSD_UPDATE);	
		}
		if(ubItemIdx %2)
		{
			i = ubItemIdx / 2;
			UI_ShowButtonValueHighLight(tUI_CuSetting.AutoScanDuty[i],
				tOsdImgInfo[2*i].uwXStart + ((tUI_CuSetting.AutoScanDuty[i] < 10)?40:30),tOsdImgInfo[2*i].uwYStart + 15,OSD_UPDATE);
		}
	}

}
//------------------------------------------------------------------------------
static void UI_SettingingDrawSubSubMenuItem(uint16_t uwSubMenuItem)
{
//	OSD_IMG_INFO tOsdImgInfo;

	uint8_t ubItemPreIdx = tSettingSubSubMenuItem.tSettingS[uwSubMenuItem].tSubMenuInfo.ubItemPreIdx;
	uint8_t ubItemIdx    = tSettingSubSubMenuItem.tSettingS[uwSubMenuItem].tSubMenuInfo.ubItemIdx;

	uint16_t uwSettingItemOsdImg[16] = {0};

	//ai item
	OSD_IMG_INFO tOsdImgInfo[4];
	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_PARKINGLINE_CAM1_DIS_NOR, 4, &tOsdImgInfo[0]);

	
	if(uwSubMenuItem == CAMERASET_ITEM)
	{
		for(uint8_t i = 0;i < 4;i ++)
		{
			uwSettingItemOsdImg[i] = ((tUI_CuSetting.tLanguage != LANGUAGE_GERMAN)?OSD2IMG_CAMSCAM1NOR_ICON:OSD2IMG_CAMSCAM1NOR_ICON_GER) + 2*i;
		}
	}
	else if(uwSubMenuItem == RECORDSET_ITEM)
	{
		for(uint8_t i = 0;i < RECITEM_MAX;i ++)
		{
			if(i <= RECTIME_5MIN_ITEM)
			{
				uwSettingItemOsdImg[i] = OSD2IMG_RECTIME_1MIN_NOR + 2*i;
				if(tUI_CuSetting.RecInfo.tREC_Time == i)
					uwSettingItemOsdImg[i] = OSD2IMG_RECTIME_SELECT_1MIN_NOR + 2*i;
			}
			else if(i == SDCARD_ITEM)
			{
				uwSettingItemOsdImg[i] = OSD2IMG_FORMATSD_NOR;
			}
			else if(i == POWERONREC_ITEM)
			{
				if(tUI_CuSetting.ubPowerOnRecord)
					uwSettingItemOsdImg[i] = OSD2IMG_POWERON_REC_ON_NOR;
				else
					uwSettingItemOsdImg[i] = OSD2IMG_POWERON_REC_OFF_NOR;
			}
			else if(i == TIMESTAMP_ITEM)
			{
				if(tUI_CuSetting.ubTimeStame)
					uwSettingItemOsdImg[i] = OSD2IMG_TIME_STAMP_ON_NOR;
				else
					uwSettingItemOsdImg[i] = OSD2IMG_TIME_STAMP_OFF_NOR;
			}
		}
	}
	else if(uwSubMenuItem == TRIGGERSET_ITEM)
	{
		for(uint8_t i = 0;i < TRIGGER_MAX;i ++)
		{
			uwSettingItemOsdImg[i] = OSD2IMG_TRIGGER_DELAY_CAM1_NOR + 2*i;
			if(i == TRIGGER_DISPLAY_SETUP)
//				uwSettingItemOsdImg[i] = (tUI_CuSetting.tLanguage == LANGUAGE_ENGLISH)?OSD2IMG_TRIGGER_DISPLAY_SETUP_NOR:
//										(tUI_CuSetting.tLanguage == LANGUAGE_GERMAN)?OSD2IMG_TRIGGER_DISPLAY_SETUP_NOR_GER:
//										(tUI_CuSetting.tLanguage == LANGUAGE_FRENCH)?OSD2IMG_TRIGGER_DISPLAY_SETUP_NOR_FR:OSD2IMG_TRIGGER_DISPLAY_SETUP_NOR_CHN;
				uwSettingItemOsdImg[i] = OSD2IMG_TRIGGER_DISPLAY_SETUP_NOR;

		}
	}
	else if(uwSubMenuItem == AUTOSCANSET_ITEM)
	{
		for(uint8_t i = 0;i < AUTOSCAN_MAX;i ++)
		{
			if(i%2)
				uwSettingItemOsdImg[i] = OSD2IMG_AUTOSCAN_DELAY_CAM1_NOR + (i/2)*2;
			else
			{
				if(tUI_CuSetting.AutoScanEnable[i/2])
					uwSettingItemOsdImg[i] = OSD2IMG_AUTOSCAN_ON_CAM1_NOR + (i/2)*4;
				else
					uwSettingItemOsdImg[i] = OSD2IMG_AUTOSCAN_OFF_CAM1_NOR + (i/2)*4;
			}
		}
	}
	else if(uwSubMenuItem == PARKINGLINE_ITEM)
	{
		for(uint8_t i = 0;i < PARKINGLINE_MAX;i ++)
		{
			if(i%2)
				uwSettingItemOsdImg[i] = OSD2IMG_PARKINGLINE_CAM1_CAL_NOR + (i/2)*2;
			else
			{
				if(tUI_CuSetting.tParkingLineEnable[i/2] == PARKING_LINE_OFF)
					uwSettingItemOsdImg[i] = OSD2IMG_PARKINGLINE_CAM1_DIS_NOR + (i/2)*6;
				else if(tUI_CuSetting.tParkingLineEnable[i/2] == PARKING_LINE_ON)
					uwSettingItemOsdImg[i] = OSD2IMG_PARKINGLINE_CAM1_EN_NOR + (i/2)*6;
				else if(tUI_CuSetting.tParkingLineEnable[i/2] == PARKING_LINE_AUTO)
					uwSettingItemOsdImg[i] = OSD2IMG_PARKINGLINE_CAM1_AUTO_NOR + (i/2)*6;
					
			}
		}

	}
	else if(uwSubMenuItem == GUIDELINE_ITEM)
	{
		for(uint8_t i = 0;i < PARKINGLINE_MAX;i ++)
		{
			if(i%2)
				uwSettingItemOsdImg[i] = OSD2IMG_PARKINGLINE_CAM1_CAL_NOR + (i/2)*2;
			else
			{
				if(tUI_CuSetting.GuideLineEnable[i/2])
					uwSettingItemOsdImg[i] = OSD2IMG_PARKINGLINE_CAM1_EN_NOR + (i/2)*6;
				else
					uwSettingItemOsdImg[i] = OSD2IMG_PARKINGLINE_CAM1_DIS_NOR + (i/2)*6;
			}
		}
	}
//	else if(uwSubMenuItem == AI_ITEM)
//	{
//		for(uint8_t i = 0;i < GUIDELINE_ITEM;i ++)
//		{
//			if(i%2)
//				uwSettingItemOsdImg[i] = OSD2IMG_PARKINGLINE_CAM1_CAL_NOR + (i/2)*2;
//			else
//			{
//				if(tUI_CuSetting.ubIsEnableBSD[i/2] == PARKING_LINE_OFF)
//					uwSettingItemOsdImg[i] = OSD2IMG_PARKINGLINE_CAM1_DIS_NOR + (i/2)*6;
//				else if(tUI_CuSetting.ubIsEnableBSD[i/2] == PARKING_LINE_ON)
//					uwSettingItemOsdImg[i] = OSD2IMG_PARKINGLINE_CAM1_EN_NOR + (i/2)*6;
////				else if(tUI_CuSetting.ubIsEnableBSD[i/2] == PARKING_LINE_AUTO)
////					uwSettingItemOsdImg[i] = OSD2IMG_PARKINGLINE_CAM1_AUTO_NOR + (i/2)*6;
//					
//			}
//		}
//
//	}







//	if(uwSubMenuItem != AI_ITEM)
//	{	
		UI_DrawHLandNormalIcon(uwSettingItemOsdImg[ubItemPreIdx],uwSettingItemOsdImg[ubItemIdx] + UI_ICON_HIGHLIGHT);
		/**************************************显示数值*********************************************/	
		UI_SettingingDrawSubSubMenuItemValue(uwSubMenuItem);
//	}

}


//------------------------------------------------------------------------------

static void UI_CameraSubSubMenuExecute(uint16_t uwSubSubMenuItemIdx)
{	
	
	OSD_IMG_INFO tOsdImgInfo[16],tEraseOsdImgInfo;
	uint8_t *pUI_ColorParm[5] = {
								(uint8_t *)&tUI_CamStatus[uwSubSubMenuItemIdx].tCamColorParam.ubColorBL,
								(uint8_t *)&tUI_CamStatus[uwSubSubMenuItemIdx].tCamColorParam.ubColorContrast,
								(uint8_t *)&tUI_CamStatus[uwSubSubMenuItemIdx].tCamColorParam.ubColorSaturation,
								(uint8_t *)&tUI_CamStatus[uwSubSubMenuItemIdx].tCamColorParam.ubColorHue,
								(uint8_t *)&tUI_CamStatus[uwSubSubMenuItemIdx].ubVolumeLvl
								 };
	//切换到选中的摄像头画面
	UI_SwitchViewType((UI_CamNum_t)uwSubSubMenuItemIdx,TRUE);
	
	//相应的cam文字高亮
	tOSD_GetOsdImgInfor(1, OSD_IMG2, (tUI_CuSetting.tLanguage != LANGUAGE_GERMAN)?OSD2IMG_CAMSCAM1NOR_ICON:OSD2IMG_CAMSCAM1NOR_ICON_GER, 8, &tOsdImgInfo[0]);
	for(uint8_t i = 0;i < 4;i++)
		tOSD_Img2(&tOsdImgInfo[2*i], OSD_QUEUE);//先把所以cam文字都刷为normal状态
	tOSD_GetOsdImgInfor(1, OSD_IMG2, (tUI_CuSetting.tLanguage != LANGUAGE_GERMAN)?OSD2IMG_CAMSCAM1_SELECTED_NOR_ICON:OSD2IMG_CAMSCAM1_SELECTED_NOR_ICON_GER, 8, &tOsdImgInfo[0]);
	tOSD_Img2(&tOsdImgInfo[2*uwSubSubMenuItemIdx], OSD_QUEUE);	
	
	//擦除Cam1~Cam4以下的区域
	tEraseOsdImgInfo.uwHSize = 1024;
	tEraseOsdImgInfo.uwVSize = 600 - tOsdImgInfo[0].uwVSize + tOsdImgInfo[0].uwYStart;
	tEraseOsdImgInfo.uwXStart = 0;
	tEraseOsdImgInfo.uwYStart = tOsdImgInfo[0].uwVSize + tOsdImgInfo[0].uwYStart;
	OSD_EraserImg2_NoUpdate(&tEraseOsdImgInfo);
	
	//高亮brightness
	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_CAMS_BRIGHTNESS_GREEN_HL, 1, &tOsdImgInfo[0]);
	tOSD_Img2(&tOsdImgInfo[0], OSD_QUEUE);	
	//文字
	tOSD_GetOsdImgInfor(1, OSD_IMG2, (tUI_CuSetting.tLanguage == LANGUAGE_ENGLISH)?OSD2IMG_CAMS_BRIGHTNESS_WORD:
									(tUI_CuSetting.tLanguage == LANGUAGE_GERMAN)?OSD2IMG_CAMS_BRIGHTNESS_WORD_GER:
									(tUI_CuSetting.tLanguage == LANGUAGE_FRENCH)?OSD2IMG_CAMS_BRIGHTNESS_WORD_FR:OSD2IMG_CAMS_BRIGHTNESS_WORD_CHN, 7, &tOsdImgInfo[0]);
	for(uint16_t i = 0;i < 7;i++)
		tOSD_Img2(&tOsdImgInfo[i], OSD_QUEUE);

	tOSD_GetOsdImgInfor(1, OSD_IMG2, (tUI_CuSetting.tLanguage == LANGUAGE_CHINESE)?OSD2IMG_CAMS_LASER_WORD_CHN:OSD2IMG_CAMS_LASER_WORD, 1, &tOsdImgInfo[0]);
	tOSD_Img2(&tOsdImgInfo[0], OSD_QUEUE);
	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_CAMS_LED_WORD, 1, &tOsdImgInfo[0]);
	tOSD_Img2(&tOsdImgInfo[0], OSD_QUEUE);
	
	//滑杆和滑块
	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_CAMS_BRIGHTNESS_SLIDER, 8, &tOsdImgInfo[0]);
	for(uint8_t i = 0;i < 5;i++)
	{
		//滑杆
		tOSD_Img2(&tOsdImgInfo[i], OSD_QUEUE);
		//滑块
		tOsdImgInfo[6].uwXStart = UI_Map(*pUI_ColorParm[i],0,99,
			tOsdImgInfo[i].uwXStart,tOsdImgInfo[i].uwXStart + tOsdImgInfo[i].uwHSize - tOsdImgInfo[6].uwHSize);
		tOsdImgInfo[6].uwYStart = tOsdImgInfo[i].uwYStart;
		tOSD_Img2(&tOsdImgInfo[6], OSD_QUEUE);
		//数值
		if(*pUI_ColorParm[i] < 10)
		{
			UI_ShowButtonValueHighLight(0,tOsdImgInfo[6].uwXStart + 4,tOsdImgInfo[6].uwYStart + 9,OSD_QUEUE);
			UI_ShowButtonValueHighLight(*pUI_ColorParm[i],tOsdImgInfo[6].uwXStart + 24,tOsdImgInfo[6].uwYStart + 9,OSD_QUEUE);
		}
		else
			UI_ShowButtonValueHighLight(*pUI_ColorParm[i],tOsdImgInfo[6].uwXStart + 4,tOsdImgInfo[6].uwYStart + 9,OSD_QUEUE);
	}
	//镜像
	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_CAMS_H_MIRROR_OFF_NOR, 16, &tOsdImgInfo[0]);
		//mirrow
	if(tUI_CamStatus[uwSubSubMenuItemIdx].tCamImgMirror)
		tOSD_Img2(&tOsdImgInfo[2], OSD_QUEUE);
	else
		tOSD_Img2(&tOsdImgInfo[0], OSD_QUEUE);
		//flip
	if(tUI_CamStatus[uwSubSubMenuItemIdx].tCamImgFlip)
		tOSD_Img2(&tOsdImgInfo[6], OSD_QUEUE);
	else
		tOSD_Img2(&tOsdImgInfo[4], OSD_QUEUE);
	//laser
	if(tUI_CamStatus[uwSubSubMenuItemIdx].tCamLaser && (UI_CheckTxVersion(tUI_CamStatus[uwSubSubMenuItemIdx].cCamVersion,30,Laserchar,3)))  //Alan
		tOSD_Img2(&tOsdImgInfo[10], OSD_QUEUE);
	else
		tOSD_Img2(&tOsdImgInfo[8], OSD_QUEUE);
		//led
	if(tUI_CamStatus[uwSubSubMenuItemIdx].tCamLed && (UI_CheckTxVersion(tUI_CamStatus[uwSubSubMenuItemIdx].cCamVersion,30,Laserchar,3)))
		tOSD_Img2(&tOsdImgInfo[14], OSD_UPDATE);
	else
		tOSD_Img2(&tOsdImgInfo[12], OSD_UPDATE);
	
	//return
	if(ubUI_TouchPanelSts)
	{
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_RETURN_NOBACKGROUND_NOR, 1, &tOsdImgInfo[0]);
		tOSD_Img2(&tOsdImgInfo[0], OSD_UPDATE);
	}

	if(UI_CheckTxVersion(tUI_CamStatus[uwSubSubMenuItemIdx].cCamVersion,30,Laserchar,3))
	{
		UI_UpdateLaserAndLedCtrl((UI_CamNum_t)uwSubSubMenuItemIdx);
		UI_UpdateTxDevStatusInfo((UI_CamNum_t)uwSubSubMenuItemIdx);
	}
	
	CameraSubSubSubRemoteMode = REMOTE_SELECT_ITEM;
	memset(&(tCameraSubSubSubMenuItem[uwSubSubMenuItemIdx]->tSubMenuInfo), 0, sizeof(UI_MenuItem_t));
}
//------------------------------------------------------------------------------
void UI_ShowUTCVal(void)
{
		
    //  0  FS_TZ_DST       = 0xD0,             //!< UTC-12:00, Dateline Standard Time
	//  1  FS_TZ_SST       = 0xD4,             //!< UTC-11:00, Samoa Standard Time
	//  2  FS_TZ_HST       = 0xD8,             //!< UTC-10:00, Hawaii Standard Time
	//  3  FS_TZ_ALASKAST  = 0xDC,             //!< UTC-09:00, Alaska Standard Time
	//  4  FS_TZ_PST       = 0xE0,             //!< UTC-08:00, Pacific Standard Time
	//  5  FS_TZ_MOUNTST   = 0xE4,             //!< UTC-07:00, Mountain Standard Time
	//  6  FS_TZ_CST       = 0xE8,             //!< UTC-06:00, Central Standard Time
	//  7  FS_TZ_EST       = 0xEC,             //!< UTC-05:00, Eastern Standard Time
	//  8  FS_TZ_ATLANTST  = 0xF0,             //!< UTC-04:00, Atlantic Standard time
	//  9  FS_TZ_NST       = 0xF2,             //!< UTC-03:30, Newfoundland Standard Time
	// 10  FS_TZ_GREENLANDST = 0xF4,           //!< UTC-03:00, Greenland Standard Time
	// 11  FS_TZ_MAST      = 0xF8,             //!< UTC-02:00, Mid-Atlantic Standard Time
	// 12  FS_TZ_AZORESST  = 0xFC              //!< UTC-01:00, Azores Standard Time
	// 13  FS_TZ_GMT       = 0x80,             //!< UTC=GMT, Greenwich Standard Time
	// 14  FS_TZ_CET       = 0x84,             //!< UTC+01:00, Central Europe Time
	// 15  FS_TZ_EEST      = 0x88,             //!< UTC+02:00, Eastern Europe Standard Time
	// 16  FS_TZ_MST       = 0x8C,             //!< UTC+03:00, Moscow Standard Time
	// 17  FS_TZ_AST       = 0x90,             //!< UTC+04:00, Arabian Standard Time
	// 18  FS_TZ_WAST      = 0x94,             //!< UTC+05:00, West Asia Standard Time
	// 19  FS_TZ_CAST      = 0x98,             //!< UTC+06:00, Central Asia Standard Time
	// 20  FS_TZ_NAST      = 0x9C,             //!< UTC+07:00, North Asia Standard Time
	// 21  FS_TZ_NAEST     = 0xA0,             //!< UTC+08:00, North Asia East Standard Time,GMT+08:00
	// 22  FS_TZ_TOKYOST   = 0xA4,             //!< UTC+09:00, Tokyo Standard Time
	// 23  FS_TZ_WPST      = 0xA8,             //!< UTC+10:00, West Pacific Standard Time
	// 24  FS_TZ_CPST      = 0xAC,             //!< UTC+11:00, Central Pacific Standard Time
	// 25  FS_TZ_NZST      = 0xB0,             //!< UTC+12:00, New Zealand Standard Time
	// 26  FS_TZ_TONGAST   = 0xB4,             //!< UTC+13:00, Tonga Standard Time
	
	OSD_IMG_INFO tOsdImgInfo[12];
	uint8_t ubUTC_Hour[27] =   {12,11,10,9,8,7,6,5,4,3, 3,2,1,0,1,2,3,4,5,6,7,8,9,10,11,12,13};
	uint8_t ubUTC_Minute[27] = {0, 0, 0, 0,0,0,0,0,0,30,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0, 0, 0};
	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_TIME_ZONE_UTC_MINUS, 2, &tOsdImgInfo[0]);
	if(tUI_CuSetting.ubtimezone >= 13)	//UTC +
		tOSD_Img2(&tOsdImgInfo[1], OSD_QUEUE);
	else 								//UTC -
		tOSD_Img2(&tOsdImgInfo[0], OSD_QUEUE);
	//hour			
	if(ubUTC_Hour[tUI_CuSetting.ubtimezone] < 10)
	{
		UI_ShowNumber(0,tOsdImgInfo[0].uwXStart + tOsdImgInfo[0].uwHSize,tOsdImgInfo[0].uwYStart,OSD2IMG_NORMAL_NUM0,OSD_QUEUE);
		UI_ShowNumber(ubUTC_Hour[tUI_CuSetting.ubtimezone],tOsdImgInfo[0].uwXStart + tOsdImgInfo[0].uwHSize + 20,tOsdImgInfo[0].uwYStart,OSD2IMG_NORMAL_NUM0,OSD_QUEUE);
	}
	else
		UI_ShowNumber(ubUTC_Hour[tUI_CuSetting.ubtimezone],tOsdImgInfo[0].uwXStart + tOsdImgInfo[0].uwHSize,tOsdImgInfo[0].uwYStart,OSD2IMG_NORMAL_NUM0,OSD_QUEUE);
	//colon
	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_NORMAL_COLON, 1, &tOsdImgInfo[2]);
	tOsdImgInfo[2].uwXStart = tOsdImgInfo[0].uwXStart + tOsdImgInfo[0].uwHSize + 40;
	tOsdImgInfo[2].uwYStart = tOsdImgInfo[0].uwYStart;
	tOSD_Img2(&tOsdImgInfo[2], OSD_QUEUE);

	//minute
	if(ubUTC_Minute[tUI_CuSetting.ubtimezone] < 10)
	{
		UI_ShowNumber(0,tOsdImgInfo[0].uwXStart + tOsdImgInfo[0].uwHSize + 60,tOsdImgInfo[0].uwYStart,OSD2IMG_NORMAL_NUM0,OSD_QUEUE);
		UI_ShowNumber(ubUTC_Minute[tUI_CuSetting.ubtimezone],tOsdImgInfo[0].uwXStart + tOsdImgInfo[0].uwHSize + 80,tOsdImgInfo[0].uwYStart,OSD2IMG_NORMAL_NUM0,OSD_UPDATE);
	}
	else
		UI_ShowNumber(ubUTC_Minute[tUI_CuSetting.ubtimezone],tOsdImgInfo[0].uwXStart + tOsdImgInfo[0].uwHSize + 60,tOsdImgInfo[0].uwYStart,OSD2IMG_NORMAL_NUM0,OSD_UPDATE);
}
//------------------------------------------------------------------------------
static void UI_RecordSubSubMenuExecute(uint16_t uwSubSubMenuItemIdx)
{
	OSD_IMG_INFO tOsdImgInfo[12],tMenuOsdImgInfo;

	if(uwSubSubMenuItemIdx <= RECTIME_5MIN_ITEM)
	{
		UI_RecordingAct_t tCurRecAct;

		tCurRecAct = tUI_RecPlayAct.tRecAct;
		if(UI_REC_START == tUI_RecPlayAct.tRecAct)
			UI_VideoRecordingExec(UI_REC_STOP);
		tUI_CuSetting.RecInfo.tREC_Time = (UI_RecordTime_t)uwSubSubMenuItemIdx;
		if(tUI_CuSetting.RecInfo.tREC_Time == RECTIME_1MIN)
			REC_TimeSet(0,60);
		else if(tUI_CuSetting.RecInfo.tREC_Time == RECTIME_3MIN)
			REC_TimeSet(0,180);
		else if(tUI_CuSetting.RecInfo.tREC_Time == RECTIME_5MIN)
			REC_TimeSet(0,300);
		if(UI_REC_START == tCurRecAct)
			UI_VideoRecordingExec(UI_REC_START);

		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_RECTIME_1MIN_NOR, 12, &tOsdImgInfo[0]);
		for(uint8_t i = 0;i < 3;i++)
			tOSD_Img2(&tOsdImgInfo[2*i], OSD_QUEUE);
		
		tOSD_Img2(&tOsdImgInfo[7 + 2*uwSubSubMenuItemIdx], OSD_UPDATE);

	}
	else if(uwSubSubMenuItemIdx == SDCARD_ITEM)
	{
		tOSD_GetOsdImgInfor(1, OSD_IMG2, (tUI_CuSetting.tLanguage == LANGUAGE_ENGLISH)?OSD2IMG_SDCARD_FORMAT_BG:
										(tUI_CuSetting.tLanguage == LANGUAGE_GERMAN)?OSD2IMG_SDCARD_FORMAT_BG_GER:
										(tUI_CuSetting.tLanguage == LANGUAGE_FRENCH)?OSD2IMG_SDCARD_FORMAT_BG_FR:OSD2IMG_SDCARD_FORMAT_BG_CHN, 5, &tOsdImgInfo[0]);
		tOSD_Img2(&tOsdImgInfo[0], OSD_QUEUE);
		tOSD_Img2(&tOsdImgInfo[2], OSD_QUEUE);
		tOSD_Img2(&tOsdImgInfo[3], OSD_UPDATE);
		tUI_State = UI_SUBSUBSUBMENU_STATE;
	}
	else if(uwSubSubMenuItemIdx == POWERONREC_ITEM)
	{
		tUI_CuSetting.ubPowerOnRecord = 1 - tUI_CuSetting.ubPowerOnRecord ;
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_POWERON_REC_OFF_NOR, 4, &tOsdImgInfo[0]);
		tOSD_Img2(&tOsdImgInfo[1 + 2*tUI_CuSetting.ubPowerOnRecord], OSD_UPDATE);
	}
	else if(uwSubSubMenuItemIdx == TIMESTAMP_ITEM)
	{
		tUI_CuSetting.ubTimeStame = 1 - tUI_CuSetting.ubTimeStame ;
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_TIME_STAMP_OFF_NOR, 4, &tOsdImgInfo[0]);
		tOSD_Img2(&tOsdImgInfo[1 + 2*tUI_CuSetting.ubTimeStame], OSD_UPDATE);
	}
		
}


//------------------------------------------------------------------------------
static void UI_TriggerSubSubMenuExecute(uint16_t uwSubSubMenuItemIdx)
{
	OSD_IMG_INFO tOsdImgInfo[25],tMenuOsdImgInfo;
	tOSD_GetOsdImgInfor(1, OSD_IMG1, OSD1IMG_SUBMENU2, 1, &tMenuOsdImgInfo);
	tOSD_Img1(&tMenuOsdImgInfo, OSD_QUEUE);
	if(!ubUI_TouchPanelSts)
	{
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_RETURN_BLACK, 1, &tMenuOsdImgInfo);
		tOSD_Img2(&tMenuOsdImgInfo, OSD_QUEUE);
	}
	//delay
	if(uwSubSubMenuItemIdx <= TRIGGER_DELAY_CAMSPLIT)
	{
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_KEYBOARD_BACKGROUND, 25, &tOsdImgInfo[0]);
		tOSD_Img2(&tOsdImgInfo[0], OSD_QUEUE);
		for(uint8_t i = 0;i < 12;i++)
			tOSD_Img2(&tOsdImgInfo[1 + 2*i], OSD_QUEUE);
		tOSD_Img2(&tOsdImgInfo[2], OSD_QUEUE);
		UI_ShowKeyBoardValue(tUI_CuSetting.TriggerDelay[uwSubSubMenuItemIdx],OSD_UPDATE);
		
	}//priority
	else if(uwSubSubMenuItemIdx <= TRIGGER_PRIORITY_CAMSPLIT)
	{
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_TRIGGER_PRIORITY1_NOR, 10, &tOsdImgInfo[0]);
		for(uint8_t i = 0;i < 5;i++)
			tOSD_Img2(&tOsdImgInfo[2*i], OSD_QUEUE);
		tOSD_Img2(&tOsdImgInfo[2*tUI_CuSetting.TriggerPriority[uwSubSubMenuItemIdx - 5] + UI_ICON_HIGHLIGHT], OSD_UPDATE);

	}//display setup
	else
	{
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_POWERON_CAM1_NOR, 12, &tOsdImgInfo[0]);
		for(uint8_t i = 0;i < 7;i++)
			tOSD_Img2(&tOsdImgInfo[2*i], OSD_QUEUE);

		tOSD_Img2(&tOsdImgInfo[2*tUI_CuSetting.TriggerSplitView + UI_ICON_HIGHLIGHT], OSD_UPDATE);

	}
}
//------------------------------------------------------------------------------
static void UI_AutoScanSubSubMenuExecute(uint16_t uwSubSubMenuItemIdx)
{
	OSD_IMG_INFO tOsdImgInfo[28],tMenuOsdImgInfo;
	uint16_t CamIdx = uwSubSubMenuItemIdx/2;
	uint8_t ubCount = 0;
	//OnOff
	if(uwSubSubMenuItemIdx%2 == 0)
	{
		tUI_CuSetting.AutoScanEnable[CamIdx] = 1 - tUI_CuSetting.AutoScanEnable[CamIdx];
		for(uint8_t i = 0;i < 6;i ++)
		{
			if(tUI_CuSetting.AutoScanEnable[i])
				ubCount ++;
		}
		if(ubCount == 0)//自动扫描至少需要一个画面参与
		{
			tUI_CuSetting.AutoScanEnable[CamIdx] = 1 - tUI_CuSetting.AutoScanEnable[CamIdx];
			return;
		}

		
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_AUTOSCAN_OFF_CAM1_NOR, 24, &tOsdImgInfo[0]);
		uint16_t ImgIdx = 4*(CamIdx) + 2*tUI_CuSetting.AutoScanEnable[CamIdx] + UI_ICON_HIGHLIGHT;
		tOSD_Img2(&tOsdImgInfo[ImgIdx], OSD_UPDATE);
	}//delay
	else
	{
		tOSD_GetOsdImgInfor(1, OSD_IMG1, OSD1IMG_SUBMENU2, 1, &tMenuOsdImgInfo);
		tOSD_Img1(&tMenuOsdImgInfo, OSD_QUEUE);
		if(!ubUI_TouchPanelSts)
		{
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_RETURN_BLACK, 1, &tMenuOsdImgInfo);
			tOSD_Img2(&tMenuOsdImgInfo, OSD_QUEUE);
		}
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_KEYBOARD_BACKGROUND, 25, &tOsdImgInfo[0]);
		tOSD_Img2(&tOsdImgInfo[0], OSD_QUEUE);
		for(uint8_t i = 0;i < 12;i++)
			tOSD_Img2(&tOsdImgInfo[1 + 2*i], OSD_QUEUE);
		tOSD_Img2(&tOsdImgInfo[2], OSD_QUEUE);
		UI_ShowKeyBoardValue(tUI_CuSetting.AutoScanDuty[CamIdx],OSD_UPDATE);
		
		tUI_State = UI_SUBSUBSUBMENU_STATE;

	}

}


//------------------------------------------------------------------------------
//extern UI_ParkinglinePoint_t tUI_ParkinglinePoint[4];
static void UI_ParkingLineSubSubMenuExecute(uint16_t uwSubSubMenuItemIdx)
{
	OSD_IMG_INFO tOsdImgInfo[24],tEraserOsdImgInfo,tOsdImgInfo1;
	UI_CamNum_t CamIdx = (UI_CamNum_t)(uwSubSubMenuItemIdx/2);
	if(uwSubSubMenuItemIdx%2 == 0)//enable/disable
	{
		if(tUI_CuSetting.tParkingLineEnable[CamIdx] == PARKING_LINE_AUTO)
			tUI_CuSetting.tParkingLineEnable[CamIdx] = PARKING_LINE_OFF;
		else
			tUI_CuSetting.tParkingLineEnable[CamIdx] ++;
		//刷新图片
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_PARKINGLINE_CAM1_DIS_NOR, 24, &tOsdImgInfo[0]);
		uint16_t ImgIdx = 6*(CamIdx) + 2*tUI_CuSetting.tParkingLineEnable[CamIdx] + UI_ICON_HIGHLIGHT;
		tOSD_Img2(&tOsdImgInfo[ImgIdx], OSD_UPDATE);	
	}
	else//calibrate
	{
		
		uint16_t uwHSize = 0,uwVSize = 0;
		UI_PanelOff();
		tOSD_GetOsdImgInfor(1, OSD_IMG1, OSD1IMG_SUBMENU2, 1, &tEraserOsdImgInfo);
		OSD_EraserImg1(&tEraserOsdImgInfo);
		UI_SwitchViewType(CamIdx,TRUE);	
 		UI_DrawParkingLine(CamIdx,OSD_QUEUE);
 		
 		tOSD_GetOsdImgInfor(1, OSD_IMG1, OSD1IMG_PARKINGLINE_UPPER1 + tUI_CuSetting.ParkingLineSize[CamIdx], 1, &tOsdImgInfo[0]);
 		uwHSize += tOsdImgInfo[0].uwHSize;
 		tOSD_GetOsdImgInfor(1, OSD_IMG1, OSD1IMG_PARKINGLINE_LEFT, 1, &tOsdImgInfo[0]);
 		uwHSize += 2*tOsdImgInfo[0].uwHSize;
 		uwVSize = tOsdImgInfo[0].uwVSize;
 	//	tOSD_Img1(&tOsdImgInfo[0], OSD_UPDATE);
 		//遥控操作状态
 		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_PARKINGLINE_L_R_FLAG, 1, &tOsdImgInfo[1]);
 		tOsdImgInfo[1].uwXStart = (tUI_CuSetting.ParkingLine_XY[CamIdx][0] << 8) + tUI_CuSetting.ParkingLine_XY[CamIdx][1] + (uwHSize - tOsdImgInfo[1].uwHSize)/2;
 		tOsdImgInfo[1].uwYStart = (tUI_CuSetting.ParkingLine_XY[CamIdx][2] << 8) + tUI_CuSetting.ParkingLine_XY[CamIdx][3] + (uwVSize - tOsdImgInfo[1].uwVSize)/2;
 		tOSD_Img2(&tOsdImgInfo[1], OSD_UPDATE);
 		//return
 		if(ubUI_TouchPanelSts)
 		{
 			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_RETURN_NOBACKGROUND_NOR, 1, &tOsdImgInfo[0]);
 			tOSD_Img2(&tOsdImgInfo[0], OSD_UPDATE);
 		}
 		tUI_State = UI_SUBSUBSUBMENU_STATE;
 		ParkingLineRemoteMode = 0;
 		SendIrCodeFlag = 1;

	}

}

//------------------------------------------------------------------------------

static void UI_GuideLineSubSubMenuExecute(uint16_t uwSubSubMenuItemIdx)
{
	OSD_IMG_INFO tOsdImgInfo[24],tEraserOsdImgInfo;
	UI_CamNum_t CamIdx = (UI_CamNum_t)(uwSubSubMenuItemIdx/2);
	if(uwSubSubMenuItemIdx%2 == 0)//enable/disable
	{
		tUI_CuSetting.GuideLineEnable[CamIdx] = 1 - tUI_CuSetting.GuideLineEnable[CamIdx];
		//刷新图片
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_PARKINGLINE_CAM1_DIS_NOR, 24, &tOsdImgInfo[0]);
		uint16_t ImgIdx = 6*(CamIdx) + 2*tUI_CuSetting.GuideLineEnable[CamIdx] + UI_ICON_HIGHLIGHT;
		tOSD_Img2(&tOsdImgInfo[ImgIdx], OSD_UPDATE);	
	}
	else//calibrate
	{
		UI_PanelOff();
		tOSD_GetOsdImgInfor(1, OSD_IMG1, OSD1IMG_SUBMENU2, 1, &tEraserOsdImgInfo);
		OSD_EraserImg1(&tEraserOsdImgInfo);
		UI_SwitchViewType(CamIdx,TRUE);
//		//引导线
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_GUIDELINE_SIZE1 + tUI_CuSetting.GuideLineSize[CamIdx], 1, &tOsdImgInfo[0]);
	
		tOsdImgInfo[0].uwXStart = (tUI_CuSetting.GuideLine_XY[CamIdx][0] << 8) + tUI_CuSetting.GuideLine_XY[CamIdx][1];
		tOsdImgInfo[0].uwYStart = (tUI_CuSetting.GuideLine_XY[CamIdx][2] << 8) + tUI_CuSetting.GuideLine_XY[CamIdx][3];
		tOSD_Img2(&tOsdImgInfo[0], OSD_QUEUE);
		
		//遥控操作状态
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_PARKINGLINE_L_R_FLAG, 1, &tOsdImgInfo[1]);
		tOsdImgInfo[1].uwXStart = tOsdImgInfo[0].uwXStart + (tOsdImgInfo[0].uwHSize - tOsdImgInfo[1].uwHSize)/2;
		if(tOsdImgInfo[0].uwYStart > tOsdImgInfo[1].uwVSize + 20)
			tOsdImgInfo[1].uwYStart = tOsdImgInfo[0].uwYStart - tOsdImgInfo[1].uwVSize - 20;
		else
			tOsdImgInfo[1].uwYStart = tOsdImgInfo[0].uwYStart + tOsdImgInfo[0].uwVSize + 20;
		tOSD_Img2(&tOsdImgInfo[1], OSD_UPDATE);
		

		//return
		if(ubUI_TouchPanelSts)
		{
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_RETURN_NOBACKGROUND_NOR, 1, &tOsdImgInfo[0]);
			tOSD_Img2(&tOsdImgInfo[0], OSD_UPDATE);
		}
		tUI_State = UI_SUBSUBSUBMENU_STATE;
		ParkingLineRemoteMode = 0;
		SendIrCodeFlag = 1;

	}

}
//------------------------------------------------------------------------------
void UI_AISubSubMenuExecute(uint8_t ubItemCurIdx,uint8_t Switch_FLAG)
{
	OSD_IMG_INFO tOsdImgInfo[12];
	uint8_t CamIdx;
	CamIdx = ubItemCurIdx%4; 
	
	tOSD_GetOsdImgInfor(1, OSD_IMG1, OSD1IMG_SUBMENU2, 1, &tOsdImgInfo[0]);
	OSD_EraserImg1_NOUpdate(&tOsdImgInfo[0]);
	if(Switch_FLAG)
	{
		//闃叉鑷姩閫�鍑鸿彍鍗曡�岀储寮曟湭娓?
		tAISubSubSubMenuItem.tSubMenuInfo.ubItemIdx = 0;
		tAISubSubSubMenuItem.tSubMenuInfo.ubItemPreIdx = 0;
		AILineRemoteMode = UnSelect;
		AILinePointIndex = PonitUpLeft;	
		UI_PanelOff();
		UI_SwitchViewType(CamIdx,TRUE); 
		UI_DRAW_BSDRANGE(tUI_CuSetting.tUI_AIDetectlinePoint[CamIdx],tUI_CuSetting.tLocation1[CamIdx],tUI_CuSetting.tLocation2[CamIdx],OSD_UPDATE,OSD_IMG2);
		
		//AI WORD
//		tOSD_GetOsdImgInfor(1, OSD_IMG2, (tUI_CuSetting.tLanguage != LANGUAGE_GERMAN)?OSD2IMG_AI_CONFIG_WORD:OSD2IMG_AI_CONFIG_WORD_GEN, 1, &tOsdImgInfo[0]);
//		if(tUI_CuSetting.tLanguage == LANGUAGE_FRENCH)
//			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_AI_CONFIG_WORD_FR, 1, &tOsdImgInfo[0]);
//		else if(tUI_CuSetting.tLanguage == LANGUAGE_CHINESE)
//			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_AI_CONFIG_WORD_CHN, 1, &tOsdImgInfo[0]);
//		tOSD_Img2(&tOsdImgInfo[0], OSD_QUEUE);
		//AI CONFIG 
//		//鍚庨潰闇�瑕佸姞鍒ゆ柇锛岄�夋嫨浜嗕粈涔堢被鍨嬬殑鐢绘
//		if(tUI_CuSetting.ubBSDBoxType)//閫夋嫨浜嗙被鍨嬩簩 1
//		{
//			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_AI_FRAME_TYPE1_NOR, 4, &tOsdImgInfo[0]);
//			tOSD_Img2(&tOsdImgInfo[1], OSD_QUEUE);//hl
//			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_AI_FRAME_TYPE2_NOR, 4, &tOsdImgInfo[0]);
//			tOSD_Img2(&tOsdImgInfo[2], OSD_QUEUE);
//		}
//		else//閫夋嫨浜嗙被鍨嬩竴 0 
//		{
//			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_AI_FRAME_TYPE1_NOR, 4, &tOsdImgInfo[0]);
//			tOSD_Img2(&tOsdImgInfo[3], OSD_QUEUE);//hl
//			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_AI_FRAME_TYPE2_NOR, 4, &tOsdImgInfo[0]);
//			tOSD_Img2(&tOsdImgInfo[0], OSD_QUEUE);
//		}
		//UI_DRAW_BSDRANGE(tUI_CuSetting.tUI_AIDetectlinePoint[CamIdx],tUI_CuSetting.tLocation1[CamIdx],tUI_CuSetting.tLocation2[CamIdx]);
	}
	else
	{
		UI_DRAW_BSDRANGE(tUI_CuSetting.tUI_AIDetectlinePoint[CamIdx],tUI_CuSetting.tLocation1[CamIdx],tUI_CuSetting.tLocation2[CamIdx],OSD_UPDATE,OSD_IMG2);
//		//AI WORD		
//		tOSD_GetOsdImgInfor(1, OSD_IMG2, (tUI_CuSetting.tLanguage != LANGUAGE_GERMAN)?OSD2IMG_AI_CONFIG_WORD:OSD2IMG_AI_CONFIG_WORD_GEN, 1, &tOsdImgInfo[0]);
//		if(tUI_CuSetting.tLanguage == LANGUAGE_FRENCH)
//			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_AI_CONFIG_WORD_FR, 1, &tOsdImgInfo[0]);
//		else if(tUI_CuSetting.tLanguage == LANGUAGE_CHINESE)
//			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_AI_CONFIG_WORD_CHN, 1, &tOsdImgInfo[0]);
//		tOSD_Img2(&tOsdImgInfo[0], OSD_QUEUE);
//		if(tUI_CuSetting.ubBSDBoxType)//閫夋嫨浜嗙被鍨嬩簩 1
//		{
//			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_AI_FRAME_TYPE1_NOR, 4, &tOsdImgInfo[0]);
//			tOSD_Img2(&tOsdImgInfo[0], OSD_QUEUE);
//			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_AI_FRAME_TYPE2_NOR, 4, &tOsdImgInfo[0]);
//			tOSD_Img2(&tOsdImgInfo[2], OSD_QUEUE);
//		}
//		else//閫夋嫨浜嗙被鍨嬩竴 0 
//		{
//			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_AI_FRAME_TYPE1_NOR, 4, &tOsdImgInfo[0]);
//			tOSD_Img2(&tOsdImgInfo[2], OSD_QUEUE);
//			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_AI_FRAME_TYPE2_NOR, 4, &tOsdImgInfo[0]);
//			tOSD_Img2(&tOsdImgInfo[0], OSD_QUEUE);
//		}
	}	
		// BSD 鐢绘鍑芥暟API棰勭暀
		//UI_AutoDrawBSD(ubItemCurIdx%4); 
		//UI_AIDrawPoint(AILinePointIndex,CamIdx);
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_CAMS_H_MIRROR_OFF_NOR, 4, &tOsdImgInfo[0]);//澶嶇敤H mirror鐨凮N/OFF鍥?
		tOsdImgInfo[2*tUI_CuSetting.ubIsEnableBSDRANGE[CamIdx]].uwXStart = 30; 
		tOsdImgInfo[2*tUI_CuSetting.ubIsEnableBSDRANGE[CamIdx]].uwYStart = 20;
		tOSD_Img2(&tOsdImgInfo[2*tUI_CuSetting.ubIsEnableBSDRANGE[CamIdx]], OSD_QUEUE); 

		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_AI_RESET_NOR, 2, &tOsdImgInfo[0]);
		tOSD_Img2(&tOsdImgInfo[0], OSD_QUEUE); 


		
		//铏氭嫙閿洏
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_AI_CHOOSE_UP_NOR, 10, &tOsdImgInfo[0]);
		for(uint8_t i = 0; i < 5;i++)
		{	
			tOSD_Img2(&tOsdImgInfo[i*2], OSD_QUEUE);		
		}	


		
		//return
		if(ubUI_TouchPanelSts)
		{
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_RETURN_NOBACKGROUND_NOR, 1, &tOsdImgInfo[0]);
			tOSD_Img2(&tOsdImgInfo[0], OSD_QUEUE);
			UI_AIDrawPoint(AILinePointIndex,CamIdx);//update
//			if(!Switch_FLAG)
//			{
//				UI_DRAW_BSDRANGE(tUI_CuSetting.tUI_AIDetectlinePoint[CamIdx],tUI_CuSetting.tLocation1[CamIdx],tUI_CuSetting.tLocation2[CamIdx],OSD_UPDATE);
//			}
		}
		else
		{
			UI_AIDrawPoint(AILinePointIndex,CamIdx);//update
			//OSD_UpdateImg2();
		}

	tUI_State = UI_SUBSUBSUBMENU_STATE;
	SendIrCodeFlag = 1;//杩涘叆瀛愮骇鑿滃崟

}
//------------------------------------------------------------------------------

static void UI_SettingSubSubMenuExecute(void)
{
	uint16_t uwSubMenuItem = 
		(UI_SettingSubMenuItemList_t)tUI_SubMenuItem[SETTING_ITEM].tSubMenuInfo.ubItemIdx;//sub
	uint16_t uwSubSubMenuItemIdx = 
		(uint16_t)tSettingSubSubMenuItem.tSettingS[uwSubMenuItem].tSubMenuInfo.ubItemIdx;//sub sub
	ubUI_SubSubSubStsUpdateFlag = FALSE;
	SettingSubSubSubRemoteMode = REMOTE_SELECT_ITEM;
	switch(uwSubMenuItem)
	{
		case CAMERASET_ITEM:
			UI_CameraSubSubMenuExecute(uwSubSubMenuItemIdx);
			tUI_State = UI_SUBSUBSUBMENU_STATE;
			break;
		case RECORDSET_ITEM:
			UI_RecordSubSubMenuExecute(uwSubSubMenuItemIdx);
			break;
		case TRIGGERSET_ITEM:
			UI_TriggerSubSubMenuExecute(uwSubSubMenuItemIdx);
			tUI_State = UI_SUBSUBSUBMENU_STATE;
			break;
		case AUTOSCANSET_ITEM:
			UI_AutoScanSubSubMenuExecute(uwSubSubMenuItemIdx);
			break;
		case PARKINGLINE_ITEM:
			UI_ParkingLineSubSubMenuExecute(uwSubSubMenuItemIdx);
			break;
		case GUIDELINE_ITEM:
			UI_GuideLineSubSubMenuExecute(uwSubSubMenuItemIdx);
			break;
//		case AI_ITEM:
//			UI_AISubSubMenuExecute(uwSubSubMenuItemIdx);	
//			break;
		default:
			break;
	}
}
//------------------------------------------------------------------------------
uint8_t ubUI_SubSubStsUpdateFlag = FALSE;

static void UI_SettingUpdateSubSubMenuItemIndex(uint16_t uwSubMenuItem)
{
	if(!ubUI_SubSubStsUpdateFlag)
	{
		ubUI_SubSubStsUpdateFlag = TRUE;
	}	

}
//------------------------------------------------------------------------------

void UI_SettingSubSubMenuPage(UI_ArrowKey_t tArrowKey)
{
	UI_MenuAct_t tMenuAct;
	OSD_IMG_INFO tOsdImgInfo;
	uint16_t uwSubMenuItem = tUI_SubMenuItem[SETTING_ITEM].tSubMenuInfo.ubItemIdx;
	
	UI_SettingUpdateSubSubMenuItemIndex(uwSubMenuItem);
	tMenuAct = UI_KeyEventMap2SubSubMenuInfo(&tArrowKey, &tSettingSubSubMenuItem.tSettingS[uwSubMenuItem]);
	switch(tMenuAct)
	{
		case DRAW_HIGHLIGHT_MENUICON:
			UI_SettingingDrawSubSubMenuItem(uwSubMenuItem);
			break;
		case EXECUTE_MENUFUNC:
			UI_SettingSubSubMenuExecute();
			break;
		case EXIT_MENUFUNC:
		{
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
			tOSD_Img2(&tOsdImgInfosub[2*uwSubMenuItem + UI_ICON_HIGHLIGHT], OSD_UPDATE);//highlight
			
			ubUI_SubSubStsUpdateFlag = FALSE;
			tUI_State = UI_SUBMENU_STATE;
			
			UI_UpdateDevStatusInfo();

			if(uwSubMenuItem == PARKINGLINE_ITEM || uwSubMenuItem == CAMERASET_ITEM || uwSubMenuItem == GUIDELINE_ITEM)
			{
				UI_SwitchViewType(DeskTopShowView,FALSE);
			}

			break;
		}
		default:
			break;
	}
	
}


/********************************************************************TOUCH*********************************************************************************************/
/********************************************************************TOUCH*********************************************************************************************/
/********************************************************************TOUCH*********************************************************************************************/
void UI_SubSubTouchMenu(TOUCH_EVENT_t *Touch_Info)
{
	switch(tUI_MenuItem.ubItemIdx)
	{
		case PLAYBACK_ITEM:
			UI_PlaybackSubSubTouchMenuPage(Touch_Info);
			break;
		case SETTING_ITEM:
			{
				
				UI_SettingSubMenuItemList_t tSubMenuItem = 
						(UI_SettingSubMenuItemList_t)tUI_SubMenuItem[SETTING_ITEM].tSubMenuInfo.ubItemIdx;
				switch(tSubMenuItem)
				{	//以下界面无法用函数UI_SettingSubSubTouchMenuPage来进行触摸解析，所以重新用一个函数
					case RECORDSET_ITEM:
						UI_RecordSubSubTouchMenuPage(Touch_Info);
						break;
					case SYSTEMSET_ITEM:
						UI_SystemSubSubTouchMenuPage(Touch_Info);
						break;
					case DUALSET_ITEM:
						UI_DualSubSubTouchMenuPage(Touch_Info);
						break;
					case QUADSET_ITEM:
						UI_QuadSubSubTouchMenuPage(Touch_Info);
						break;
					case AUTOSCANSET_ITEM:
						UI_AutoScanSubSubTouchMenuPage(Touch_Info);
						break;
					case POWERONSET_ITEM:
						UI_PowerOnSubSubTouchMenuPage(Touch_Info);
						break;
					case PARKINGLINE_ITEM:
						UI_ParkingLineSubSubTouchMenuPage(Touch_Info);
						break;
					case GUIDELINE_ITEM:
						UI_ParkingLineSubSubTouchMenuPage(Touch_Info);
						break;
					case AI_ITEM:
						UI_AISubSubTouchMenuPage(Touch_Info);
						break;
					default:
						UI_SettingSubSubTouchMenuPage(Touch_Info);
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
void UI_PlayDispTypeTouchSelection(TOUCH_EVENT_t *Touch_Info)
{
	OSD_IMG_INFO tOsdImgInfo[2];
	uint16_t uwRecPlayDispTypeImgIdx[2] = {OSD2IMG_RECFILEMULTIVIEWNOR_ICON, OSD2IMG_RECFILESIGNLEVIEWNOR_ICON};
	uint16_t i = 0;
	tUI_PlyDispTypeSelItem.ubItemPreIdx = tUI_PlyDispTypeSelItem.ubItemIdx;
	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_RECFILEMULTIVIEWNOR_ICON, 1, &tOsdImgInfo[0]);
	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_RECFILESIGNLEVIEWNOR_ICON, 1, &tOsdImgInfo[1]);

	switch(Touch_Info->Gesture)
	{
		case TOUCH_PRESS:	
			for(i = 0;i < 2;i++)
			{
				if(Touch_Info->startY > tOsdImgInfo[i].uwYStart 
				   && Touch_Info->startY < tOsdImgInfo[i].uwYStart + tOsdImgInfo[i].uwVSize 
				   &&Touch_Info->startX > tOsdImgInfo[i].uwXStart
				   &&Touch_Info->startX < tOsdImgInfo[i].uwXStart + tOsdImgInfo[i].uwHSize)
				{
					tUI_PlyDispTypeSelItem.ubItemIdx = i;
					break;
				}
			}
			if(i == 2)
			{
				UI_MenuKey();
				return;
			}
			UI_DrawHLandNormalIcon(uwRecPlayDispTypeImgIdx[tUI_PlyDispTypeSelItem.ubItemPreIdx], 
				(uwRecPlayDispTypeImgIdx[tUI_PlyDispTypeSelItem.ubItemIdx]+UI_ICON_HIGHLIGHT));
			
			UI_EnterKey();	
			break;
			
		case TOUCH_TURNDOWN:
			break;

		default :

			break;
	}	

}
//------------------------------------------------------------------------------

void UI_PlaybackSubSubTouchMenuPage(TOUCH_EVENT_t *Touch_Info)
{
	uint16_t uwHSize,uwVSize,uwXStart,uwYStart;
	OSD_IMG_INFO tFileSelOsdImgInfo[4],tOsdImgInfo;
	uint16_t ubUI_RecFileIdx, ubUI_PrevRecFileIdx = 0,i;
	uint16_t uwFileStartIdx = 0, uwFileEndIdx = 0;
	
	ubUI_RecFileIdx = (tUI_RecFilesInfo.uwRecFileSelIdx % REC_FILE_LIST_MAXNUM);
	ubUI_PrevRecFileIdx = ubUI_RecFileIdx;
	
	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_FILE_FILEBOX_LEFT, 1, &tFileSelOsdImgInfo[0]);

	switch(Touch_Info->Gesture)
	{
		case TOUCH_PRESS:	
			for(i = 0;i < REC_FILE_LIST_MAXNUM;i++)
			{
				//计算该界面中每行播放文件的位置
				uwHSize = 900;
				uwVSize = tFileSelOsdImgInfo[0].uwVSize;
				uwXStart = tFileSelOsdImgInfo[0].uwXStart;
				uwYStart = tFileSelOsdImgInfo[0].uwYStart + i*50;

				//判断点击在那个播放文件上
				if(Touch_Info->startX > uwXStart && Touch_Info->startX < uwXStart + uwHSize
					&& Touch_Info->startY > uwYStart && Touch_Info->startY < uwYStart + uwVSize)
				{
					break;
				}
			}
			if(i < REC_FILE_LIST_MAXNUM)//点击在文件列表区域
			{
				if((tUI_RecFilesInfo.uwRecFileSelIdx + i - ubUI_RecFileIdx) >= tUI_RecFilesInfo.uwTotalRecFileNum)//当前点击的位置没有文件
					return;
				
				ubUI_RecFileIdx = i;
				tUI_RecFilesInfo.uwRecFileSelIdx += i - ubUI_PrevRecFileIdx;
				UI_PlaybackDrawSubSubMenuItemFocus(ubUI_PrevRecFileIdx,ubUI_RecFileIdx);
				UI_EnterKey();
				
			}
			else
			{
				tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_RETURN_HL, 1, &tOsdImgInfo);	//return
				if(Touch_Info->startX > tOsdImgInfo.uwXStart 
					&& Touch_Info->startX < tOsdImgInfo.uwXStart + tOsdImgInfo.uwHSize
					&& Touch_Info->startY > tOsdImgInfo.uwYStart 
					&& Touch_Info->startY < tOsdImgInfo.uwYStart + tOsdImgInfo.uwVSize)
				{
					tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
					UI_MenuKey();
					return;
				}

				for(i = 0;i < 2;i ++)//换页按钮
				{
					tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_PAGE_BACKWARD_HL + 2*i, 1, &tOsdImgInfo);
					if(Touch_Info->startX > tOsdImgInfo.uwXStart 
					&& Touch_Info->startX < tOsdImgInfo.uwXStart + tOsdImgInfo.uwHSize
					&& Touch_Info->startY > tOsdImgInfo.uwYStart 
					&& Touch_Info->startY < tOsdImgInfo.uwYStart + tOsdImgInfo.uwVSize)
					{						
						if(i == 0)//backward
						{
							//大于0就说明上一页还有内容
							if(tUI_RecFilesInfo.uwRecFileSelIdx / REC_FILE_LIST_MAXNUM * REC_FILE_LIST_MAXNUM)
							{
								tUI_RecFilesInfo.uwRecFileSelIdx = tUI_RecFilesInfo.uwRecFileSelIdx / REC_FILE_LIST_MAXNUM * REC_FILE_LIST_MAXNUM;
								tUI_RecFilesInfo.uwRecFileSelIdx --;
								uwFileStartIdx = (tUI_RecFilesInfo.uwRecFileSelIdx / REC_FILE_LIST_MAXNUM) * REC_FILE_LIST_MAXNUM;
								uwFileEndIdx   = tUI_RecFilesInfo.uwRecFileSelIdx + 1;
							}
							else
								return;

						}
						else//forward
						{
							//满足该条件表示下一页还有文件
							if((tUI_RecFilesInfo.uwRecFileSelIdx / REC_FILE_LIST_MAXNUM + 1) * REC_FILE_LIST_MAXNUM < tUI_RecFilesInfo.uwTotalRecFileNum)
							{
								tUI_RecFilesInfo.uwRecFileSelIdx = (tUI_RecFilesInfo.uwRecFileSelIdx / REC_FILE_LIST_MAXNUM + 1) * REC_FILE_LIST_MAXNUM;
								uwFileStartIdx = tUI_RecFilesInfo.uwRecFileSelIdx;
								uwFileEndIdx   = tUI_RecFilesInfo.uwTotalRecFileNum - tUI_RecFilesInfo.uwRecFileSelIdx;
								uwFileEndIdx   = (uwFileEndIdx < REC_FILE_LIST_MAXNUM)?tUI_RecFilesInfo.uwTotalRecFileNum:(uwFileStartIdx + REC_FILE_LIST_MAXNUM);
							}
							else 
								return;

						}
						tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
						UI_ListRecFileInfo(uwFileStartIdx, uwFileEndIdx, OSD_QUEUE);
	
						ubUI_RecFileIdx = (tUI_RecFilesInfo.uwRecFileSelIdx % REC_FILE_LIST_MAXNUM);
						UI_PlaybackDrawSubSubMenuItemFocus(ubUI_PrevRecFileIdx,ubUI_RecFileIdx);

						break;
					}	
				}

				
			}
			return;
		
		case TOUCH_TURNDOWN:
			return;
			
		case TOUCH_TURNLEFT: 
			
			break;
		case TOUCH_TURNRIGHT:	
			
			
			break;

		default:
			return;
	}
	
	
	



}



//------------------------------------------------------------------------------setting-----------------------------------------------------------------------
//------------------------------------------------------------------------------setting-----------------------------------------------------------------------
//------------------------------------------------------------------------------setting-----------------------------------------------------------------------


//------------------------------------------------------------------------------record-----------------------------------------------------------------------

void UI_RecordSubSubTouchMenuPage(TOUCH_EVENT_t *Touch_Info)
{
	OSD_IMG_INFO tOsdImgInfo;
	uint8_t i;
	uint16_t uwRecordTouchOsdImg[RECITEM_MAX] = {
												OSD2IMG_RECTIME_1MIN_NOR,
												OSD2IMG_RECTIME_3MIN_NOR,
												OSD2IMG_RECTIME_5MIN_NOR,
												OSD2IMG_FORMATSD_NOR,
												OSD2IMG_POWERON_REC_OFF_NOR,
												OSD2IMG_TIME_STAMP_OFF_NOR,
												};
	uint16_t uwSubMenuItem = RECORDSET_ITEM;
	tSettingSubSubMenuItem.tSettingS[uwSubMenuItem].tSubMenuInfo.ubItemPreIdx = 
		tSettingSubSubMenuItem.tSettingS[uwSubMenuItem].tSubMenuInfo.ubItemIdx;
	switch(Touch_Info->Gesture)
	{
		case TOUCH_PRESS:
			for(i = 0;i < RECITEM_MAX;i++)
			{
				//读取图片信息
				tOSD_GetOsdImgInfor(1, OSD_IMG2, uwRecordTouchOsdImg[i],1, &tOsdImgInfo);
				//解析手指点击在哪个图标上
				if(Touch_Info->startX > tOsdImgInfo.uwXStart 
					&& Touch_Info->startX < tOsdImgInfo.uwXStart + tOsdImgInfo.uwHSize
					&& Touch_Info->startY > tOsdImgInfo.uwYStart 
					&& Touch_Info->startY < tOsdImgInfo.uwYStart + tOsdImgInfo.uwVSize)
				{
					tSettingSubSubMenuItem.tSettingS[uwSubMenuItem].tSubMenuInfo.ubItemIdx = i;
					break;
				}
			}
			if(i < RECITEM_MAX)
			{	
				if(tSettingSubSubMenuItem.tSettingS[uwSubMenuItem].tSubMenuInfo.ubItemPreIdx != 
							tSettingSubSubMenuItem.tSettingS[uwSubMenuItem].tSubMenuInfo.ubItemIdx)
				{
					UI_SettingingDrawSubSubMenuItem(uwSubMenuItem);
				}
				UI_EnterKey();
				
				return;
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
	case TOUCH_PRESSDOWN:
			break;
		default:
			return;
	}

	

}
//------------------------------------------------------------------------------ai-----------------------------------------------------------------------
void UI_AISubSubTouchMenuPage(TOUCH_EVENT_t *Touch_Info)
{
#if 1
	OSD_IMG_INFO tOsdImgInfo[12],tOnOffOsdImgInfo[4],tEraserOsdImgInfo;

	uint8_t i,j,index,break_flag;
	uint16_t X_Pos = AI_Config_Xpos,Y_Pos = AI_Config_Ypos,X_Step = AI_Config_Xstep,Y_Step = AI_Config_Ystep;
	tSettingSubSubMenuItem.tSettingS[10].tSubMenuInfo.ubItemPreIdx = tSettingSubSubMenuItem.tSettingS[10].tSubMenuInfo.ubItemIdx;
	switch(Touch_Info->Gesture)
	{
		case TOUCH_PRESS:
			tOSD_GetOsdImgInfor(1, OSD_IMG2,OSD2IMG_AI_OFF_NOR ,1, &tOsdImgInfo[0]);
			for(i = 0;i < 6;i++)
			{
				for(j = 0;j < 4;j++)
				{		
					if(Touch_Info->startX > X_Pos + X_Step*j
						&& Touch_Info->startX < X_Pos + X_Step*j + tOsdImgInfo[0].uwHSize
						&& Touch_Info->startY > Y_Pos + Y_Step*i
						&& Touch_Info->startY < Y_Pos + Y_Step*i + tOsdImgInfo[0].uwVSize )
					{
						tSettingSubSubMenuItem.tSettingS[10].tSubMenuInfo.ubItemIdx = AI_PD_CAM1 + 4*i + j;
						SystemSubSubRemoteMode = REMOTE_SELECT_ITEM;
						break_flag = 1;
						break;
					}

				}
				if(break_flag == 1)
				{
					break_flag = 0;
					break;
				}
			}
			index = 4*i + j; 
			if(index < AI_MAX)
			{
				switch(i%6)
				{
//					case 0:
//						tUI_CuSetting.ubIsEnableBSD[j] = 1 - tUI_CuSetting.ubIsEnableBSD[j];	
//						break;
					case 0:
						tUI_CuSetting.ubDetectPeopleFlag[j] = 1 - tUI_CuSetting.ubDetectPeopleFlag[j];	
						break;
					case 1:
						tUI_CuSetting.ubDetectCarFlag[j] = 1 - tUI_CuSetting.ubDetectCarFlag[j];	
						break;
					case 2:
						tUI_CuSetting.ubIsEnableBSDALARM[j] = 1 - tUI_CuSetting.ubIsEnableBSDALARM[j];	
						break;
					case 3:
						tUI_CuSetting.ubBSDTriggerOut[j] = 1 - tUI_CuSetting.ubBSDTriggerOut[j];	
						break;
					case 4:
						{	
							UI_AISubSubMenuExecute(j,1);
							UI_AIDrawSubSubSubMenuItem(TRUE);
							//瀛愮骇鑿滃崟鎻愬墠杩斿洖
							return;
						}
					default:
						break;
				}
				UI_AIDrawSubSubMenuItem(TRUE);
			}
			else//return
			{
				tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_RETURN_HL, 1, &tOsdImgInfo[0]);	
				if(Touch_Info->startX > tOsdImgInfo[0].uwXStart 
					&& Touch_Info->startX < tOsdImgInfo[0].uwXStart + tOsdImgInfo[0].uwHSize
					&& Touch_Info->startY > tOsdImgInfo[0].uwYStart 
					&& Touch_Info->startY < tOsdImgInfo[0].uwYStart + tOsdImgInfo[0].uwVSize)
				{
					tOSD_Img2(&tOsdImgInfo[0], OSD_UPDATE);
					TIMER_Delay_ms(30);
					UI_MenuKey();
				}
			}
			break;
		default:
			return;
	}
#endif
}

//------------------------------------------------------------------------------system-----------------------------------------------------------------------
void UI_SystemSubSubTouchMenuPage(TOUCH_EVENT_t *Touch_Info)
{
	OSD_IMG_INFO tOsdImgInfo;
	uint8_t i;
	uint16_t uwSystemTouchOsdImg[SYSTEMITEM_MAX - 1] = {
										OSD2IMG_SYSTEM_AUTO_OFF_NOR,
										OSD2IMG_SYSTEM_AUTO_SETTING_NOR,
										OSD2IMG_SYSTEM_MENULOCK_OFF_NOR,
										OSD2IMG_SYSTEM_UPGRADE_NOR,
										OSD2IMG_SYSTEM_LANGUAGE_NOR,
										OSD2IMG_SYSTEM_RESET_NOR,
										OSD2IMG_SYSTEM_SD_SWITCH_OFF_NOR,
										OSD2IMG_SYSTEM_HIDE_CHANNEL_NAME_OFF_NOR,
										OSD2IMG_SYSTEM_BUZZER_OFF_NOR,
										OSD2IMG_SYSTEM_DATE_TIME_NOR,
										};
	tSystemSubSubMenuItem.tSubMenuInfo.ubItemPreIdx = tSystemSubSubMenuItem.tSubMenuInfo.ubItemIdx;
	switch(Touch_Info->Gesture)
	{
		case TOUCH_PRESS:
			for(i = 0;i < SYSTEMITEM_MAX - 1;i++)
			{
				//读取图片信息
				tOSD_GetOsdImgInfor(1, OSD_IMG2, uwSystemTouchOsdImg[i],1, &tOsdImgInfo);
				//解析手指点击在哪个图标上
				if(Touch_Info->startX > tOsdImgInfo.uwXStart 
					&& Touch_Info->startX < tOsdImgInfo.uwXStart + tOsdImgInfo.uwHSize
					&& Touch_Info->startY > tOsdImgInfo.uwYStart 
					&& Touch_Info->startY < tOsdImgInfo.uwYStart + tOsdImgInfo.uwVSize)
				{
					tSystemSubSubMenuItem.tSubMenuInfo.ubItemIdx = AUTO_ONOFF_ITEM + i;
					SystemSubSubRemoteMode = REMOTE_SELECT_ITEM;
					break;
				}
			}
			if(i < SYSTEMITEM_MAX - 1)
			{
				if(tSystemSubSubMenuItem.tSubMenuInfo.ubItemIdx == AUTO_ONOFF_ITEM)
					tUI_CuSetting.ubAutoDimmer = 1- tUI_CuSetting.ubAutoDimmer;
				else if(tSystemSubSubMenuItem.tSubMenuInfo.ubItemIdx == MENULOCK_ITEM)
					tUI_CuSetting.ubMenuLock = 1- tUI_CuSetting.ubMenuLock;
				else if(tSystemSubSubMenuItem.tSubMenuInfo.ubItemIdx == SD_SWITCH_ITEM)
					tUI_CuSetting.ubSD_Switch = 1- tUI_CuSetting.ubSD_Switch;
				else if(tSystemSubSubMenuItem.tSubMenuInfo.ubItemIdx == BUZZER_ITEM)
					tUI_CuSetting.ubBuzzer = 1- tUI_CuSetting.ubBuzzer;
				else if(tSystemSubSubMenuItem.tSubMenuInfo.ubItemIdx == HIDE_CHANNEL_NAME_ITEM)
					tUI_CuSetting.ubHideChannelName = 1- tUI_CuSetting.ubHideChannelName;

				UI_SystemDrawSubSubMenuItem(TRUE);
				UI_SystemSubSubMenuExecute();
				return;
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
		case TOUCH_PRESSDOWN:
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_SYSTEM_DIMMER_SLIDER,1,&tOsdImgInfo);
			if(Touch_Info->endX >= tOsdImgInfo.uwXStart - 10 
				&& Touch_Info->endX <= tOsdImgInfo.uwXStart + tOsdImgInfo.uwHSize + 10
				&& Touch_Info->endY >= tOsdImgInfo.uwYStart - 10
				&& Touch_Info->endY <= tOsdImgInfo.uwYStart + tOsdImgInfo.uwVSize + 10)
			{
				if(Touch_Info->endX < tOsdImgInfo.uwXStart)
					Touch_Info->endX = tOsdImgInfo.uwXStart;
				if(Touch_Info->endX > tOsdImgInfo.uwXStart + tOsdImgInfo.uwHSize)
					Touch_Info->endX = tOsdImgInfo.uwXStart + tOsdImgInfo.uwHSize;

				tSystemSubSubMenuItem.tSubMenuInfo.ubItemIdx = DIMMER_ITEM;

				//坐标位置转成进度条的值
				uint8_t ubValue = 
					(uint8_t)UI_Map(Touch_Info->endX,tOsdImgInfo.uwXStart,tOsdImgInfo.uwXStart + tOsdImgInfo.uwHSize,0,99);
					
				tUI_CuSetting.BriLvL.ulBL_UpdateLvL = ubValue;

				
				if(tSystemSubSubMenuItem.tSubMenuInfo.ubItemPreIdx != tSystemSubSubMenuItem.tSubMenuInfo.ubItemIdx)
					UI_SystemDrawSubSubMenuItem(TRUE);
				else
					UI_SystemDrawSubSubMenuItem(FALSE);
				
				UI_SystemSubSubMenuExecute();
			}
			
			break;
		default:
	
			return;
	}

}
//------------------------------------------------------------------------------Dual-----------------------------------------------------------------------

void UI_DualSubSubTouchMenuPage(TOUCH_EVENT_t *Touch_Info)
{
	uint16_t i = 0;
	OSD_IMG_INFO tOsdImgInfo;
	uint16_t uwLcd_HSize = uwLCD_GetLcdHoSize();
	uint16_t uwDualMenuItemOsdImg[DUAL_MAX] = {
												OSD2IMG_DUAL_LEFT_CAM1NOR,
												OSD2IMG_DUAL_LEFT_CAM2NOR,
												OSD2IMG_DUAL_LEFT_CAM3NOR,
												OSD2IMG_DUAL_LEFT_CAM4NOR,
												OSD2IMG_DUAL_LEFT_CAM1NOR,
												OSD2IMG_DUAL_LEFT_CAM2NOR,
												OSD2IMG_DUAL_LEFT_CAM3NOR,
												OSD2IMG_DUAL_LEFT_CAM4NOR,
												};
	
	tDualSubSubMenuItem.tSubMenuInfo.ubItemPreIdx = tDualSubSubMenuItem.tSubMenuInfo.ubItemIdx;
	switch(Touch_Info->Gesture)
	{
		case TOUCH_PRESS:	
			for(i = 0; i < DUAL_MAX;i++)//判断具体点击在哪个图标上
			{	

				tOSD_GetOsdImgInfor(1, OSD_IMG2, uwDualMenuItemOsdImg[i], 1, &tOsdImgInfo);	
				if(i > DUAL_LEFT_CAM4)//right
					tOsdImgInfo.uwXStart += uwLcd_HSize/2;
				if(Touch_Info->startX > tOsdImgInfo.uwXStart 
						&& Touch_Info->startX < tOsdImgInfo.uwXStart + tOsdImgInfo.uwHSize
						&& Touch_Info->startY > tOsdImgInfo.uwYStart 
						&& Touch_Info->startY < tOsdImgInfo.uwYStart + tOsdImgInfo.uwVSize)
				{
					tDualSubSubMenuItem.tSubMenuInfo.ubItemIdx = i;
					break;
				}
					
			}
			
			if(i < DUAL_MAX)
			{
				UI_DualDrawSubSubMenuItem();
				UI_EnterKey();
			}
			else//return
			{
				OSD_IMG_INFO tOsdImgInfo;
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
			}
			break;
			
		case TOUCH_TURNDOWN:
			break;

		default:

			return;
	}

}
//------------------------------------------------------------------------------quad-----------------------------------------------------------------------

void UI_QuadSubSubTouchMenuPage(TOUCH_EVENT_t *Touch_Info)
{
	uint16_t i = 0;
	OSD_IMG_INFO tOsdImgInfo;
	uint16_t uwLcd_HSize = uwLCD_GetLcdHoSize();
	uint16_t uwLcd_VSize = uwLCD_GetLcdVoSize();
	uint16_t uwQuadMenuItemOsdImg[QUAD_MAX] = {
												OSD2IMG_DUAL_LEFT_CAM1NOR,
												OSD2IMG_DUAL_LEFT_CAM2NOR,
												OSD2IMG_DUAL_LEFT_CAM3NOR,
												OSD2IMG_DUAL_LEFT_CAM4NOR,
												
												OSD2IMG_DUAL_LEFT_CAM1NOR,
												OSD2IMG_DUAL_LEFT_CAM2NOR,
												OSD2IMG_DUAL_LEFT_CAM3NOR,
												OSD2IMG_DUAL_LEFT_CAM4NOR,

												OSD2IMG_DUAL_LEFT_CAM1NOR,
												OSD2IMG_DUAL_LEFT_CAM2NOR,
												OSD2IMG_DUAL_LEFT_CAM3NOR,
												OSD2IMG_DUAL_LEFT_CAM4NOR,
												
												OSD2IMG_DUAL_LEFT_CAM1NOR,
												OSD2IMG_DUAL_LEFT_CAM2NOR,
												OSD2IMG_DUAL_LEFT_CAM3NOR,
												OSD2IMG_DUAL_LEFT_CAM4NOR,
												};
	tQuadSubSubMenuItem.tSubMenuInfo.ubItemPreIdx = tQuadSubSubMenuItem.tSubMenuInfo.ubItemIdx;
	switch(Touch_Info->Gesture)
	{
		case TOUCH_PRESS:	
			for(i = 0; i < QUAD_MAX;i++)//判断具体点击在哪个图标上
			{	

				tOSD_GetOsdImgInfor(1, OSD_IMG2, uwQuadMenuItemOsdImg[i], 1, &tOsdImgInfo);	
				if(i <= QUAD_UPPER_LEFT_CAM4)
				{
					tOsdImgInfo.uwYStart -= uwLcd_VSize/4;
				}
				else if(i <= QUAD_UPPER_RIGHT_CAM4)//upper right
				{
					tOsdImgInfo.uwYStart -= uwLcd_VSize/4;
					tOsdImgInfo.uwXStart += uwLcd_HSize/2;
				}
				else if(i <= QUAD_LOWER_LEFT_CAM4)//lower left
				{
					tOsdImgInfo.uwYStart += uwLcd_VSize/4;
				}
				else//lower right
				{
					tOsdImgInfo.uwXStart += uwLcd_HSize/2;
					tOsdImgInfo.uwYStart += uwLcd_VSize/4;
				}
				if(Touch_Info->startX > tOsdImgInfo.uwXStart 
					&& Touch_Info->startX < tOsdImgInfo.uwXStart + tOsdImgInfo.uwHSize
					&& Touch_Info->startY > tOsdImgInfo.uwYStart 
					&& Touch_Info->startY < tOsdImgInfo.uwYStart + tOsdImgInfo.uwVSize)
				{
					tQuadSubSubMenuItem.tSubMenuInfo.ubItemIdx = i;
					break;
				}
					
			}
			
			if(i < QUAD_MAX)
			{
				UI_QuadDrawSubSubMenuItem();
				UI_EnterKey();
			}
			else//return
			{
				OSD_IMG_INFO tOsdImgInfo;
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
			}
			break;
			
		case TOUCH_TURNDOWN:
			break;

		default:

			return;
	}

}
//------------------------------------------------------------------------------AutoScan-----------------------------------------------------------------------
void UI_AutoScanSubSubTouchMenuPage(TOUCH_EVENT_t *Touch_Info)
{
	uint16_t uwAutoScanMenuItemOsdImg[AUTOSCAN_MAX] = {
														OSD2IMG_AUTOSCAN_OFF_CAM1_NOR,
														OSD2IMG_AUTOSCAN_DELAY_CAM1_NOR,
														OSD2IMG_AUTOSCAN_OFF_CAM2_NOR,
														OSD2IMG_AUTOSCAN_DELAY_CAM2_NOR,
														OSD2IMG_AUTOSCAN_OFF_CAM3_NOR,
														OSD2IMG_AUTOSCAN_DELAY_CAM3_NOR,
														OSD2IMG_AUTOSCAN_OFF_CAM4_NOR,
														OSD2IMG_AUTOSCAN_DELAY_CAM4_NOR,
														OSD2IMG_AUTOSCAN_OFF_DUAL_NOR,
														OSD2IMG_AUTOSCAN_DELAY_DUAL_NOR,
														OSD2IMG_AUTOSCAN_OFF_QUAD_NOR,
														OSD2IMG_AUTOSCAN_DELAY_QUAD_NOR,
														};
	uint16_t i = 0;
	OSD_IMG_INFO tOsdImgInfo;
	uint16_t uwSubMenuItem = AUTOSCANSET_ITEM;
	tSettingSubSubMenuItem.tSettingS[uwSubMenuItem].tSubMenuInfo.ubItemPreIdx = 
		tSettingSubSubMenuItem.tSettingS[uwSubMenuItem].tSubMenuInfo.ubItemIdx;
	switch(Touch_Info->Gesture)
	{
		case TOUCH_PRESS:	
			for(i = 0; i < tSettingSubSubMenuItem.tSettingS[uwSubMenuItem].ubItemCount;i++)//判断具体点击在哪个图标上
			{	
				//读取当前操作界面的全部图标信息
				tOSD_GetOsdImgInfor(1, OSD_IMG2, uwAutoScanMenuItemOsdImg[i], 1, &tOsdImgInfo);	
				if(Touch_Info->startX > tOsdImgInfo.uwXStart 
					&& Touch_Info->startX < tOsdImgInfo.uwXStart + tOsdImgInfo.uwHSize
					&& Touch_Info->startY > tOsdImgInfo.uwYStart 
					&& Touch_Info->startY < tOsdImgInfo.uwYStart + tOsdImgInfo.uwVSize)
				{
					tSettingSubSubMenuItem.tSettingS[uwSubMenuItem].tSubMenuInfo.ubItemIdx = i;
					break;
				}
			}
			
			if(i < tSettingSubSubMenuItem.tSettingS[uwSubMenuItem].ubItemCount)
			{
				
				if(tSettingSubSubMenuItem.tSettingS[uwSubMenuItem].tSubMenuInfo.ubItemPreIdx != 
							tSettingSubSubMenuItem.tSettingS[uwSubMenuItem].tSubMenuInfo.ubItemIdx)
				{
					UI_SettingingDrawSubSubMenuItem(uwSubMenuItem);
				}
				UI_EnterKey();
			}
			else//return
			{
				OSD_IMG_INFO tOsdImgInfo;
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


//------------------------------------------------------------------------------parkingLine-----------------------------------------------------------------------
//该界面无法使用函数UI_SettingSubSubTouchMenuPage进行触摸识别，所以单独使用一个函数进行解析
void UI_ParkingLineSubSubTouchMenuPage(TOUCH_EVENT_t *Touch_Info)
{
	uint16_t uwParkingLineMenuItemOsdImg[PARKINGLINE_MAX] = {
																OSD2IMG_PARKINGLINE_CAM1_DIS_NOR,
																OSD2IMG_PARKINGLINE_CAM1_CAL_NOR,
																OSD2IMG_PARKINGLINE_CAM2_DIS_NOR,
																OSD2IMG_PARKINGLINE_CAM2_CAL_NOR,
																OSD2IMG_PARKINGLINE_CAM3_DIS_NOR,
																OSD2IMG_PARKINGLINE_CAM3_CAL_NOR,
																OSD2IMG_PARKINGLINE_CAM4_DIS_NOR,
																OSD2IMG_PARKINGLINE_CAM4_CAL_NOR,
																};
	uint16_t i = 0;
	OSD_IMG_INFO tOsdImgInfo;
	uint16_t uwSubMenuItem = tUI_SubMenuItem[SETTING_ITEM].tSubMenuInfo.ubItemIdx;
	tSettingSubSubMenuItem.tSettingS[uwSubMenuItem].tSubMenuInfo.ubItemPreIdx = 
		tSettingSubSubMenuItem.tSettingS[uwSubMenuItem].tSubMenuInfo.ubItemIdx;
	switch(Touch_Info->Gesture)
	{
		case TOUCH_PRESS:	
			for(i = 0; i < tSettingSubSubMenuItem.tSettingS[uwSubMenuItem].ubItemCount;i++)//判断具体点击在哪个图标上
			{	
				//读取当前操作界面的全部图标信息
				tOSD_GetOsdImgInfor(1, OSD_IMG2, uwParkingLineMenuItemOsdImg[i], 1, &tOsdImgInfo);	
				if(Touch_Info->startX > tOsdImgInfo.uwXStart 
					&& Touch_Info->startX < tOsdImgInfo.uwXStart + tOsdImgInfo.uwHSize
					&& Touch_Info->startY > tOsdImgInfo.uwYStart 
					&& Touch_Info->startY < tOsdImgInfo.uwYStart + tOsdImgInfo.uwVSize)
				{
					tSettingSubSubMenuItem.tSettingS[uwSubMenuItem].tSubMenuInfo.ubItemIdx = i;
					break;
				}
			}
			
			if(i < tSettingSubSubMenuItem.tSettingS[uwSubMenuItem].ubItemCount)
			{
				if(tSettingSubSubMenuItem.tSettingS[uwSubMenuItem].tSubMenuInfo.ubItemPreIdx != 
							tSettingSubSubMenuItem.tSettingS[uwSubMenuItem].tSubMenuInfo.ubItemIdx)
				{
					UI_SettingingDrawSubSubMenuItem(uwSubMenuItem);
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
//------------------------------------------------------------------------------Power On-----------------------------------------------------------------------
void UI_PowerOnSubSubTouchMenuPage(TOUCH_EVENT_t *Touch_Info)
{
	uint16_t uwPowerOnMenuItemOsdImg[POWERON_MAX - 1] = {
															OSD2IMG_POWERON_CAM1_NOR,
															OSD2IMG_POWERON_CAM2_NOR,
															OSD2IMG_POWERON_CAM3_NOR,
															OSD2IMG_POWERON_CAM4_NOR,
															OSD2IMG_POWERON_DUAL_NOR,
															OSD2IMG_POWERON_QUAD_NOR,
															OSD2IMG_POWERON_SCAN_NOR,
															OSD2IMG_POWERON_STANDBY_NOR,
															};
	OSD_IMG_INFO tOsdImgInfo;
	uint8_t i;
	if(!ubUI_PowerOnSubSubStsUpdateFlag)
	{
		ubUI_PowerOnSubSubStsUpdateFlag = TRUE;
		tPowerOnSubSubMenuItem.tSubMenuInfo.ubItemIdx = tUI_CuSetting.tOldPowerOnMode;
	}
	
	tPowerOnSubSubMenuItem.tSubMenuInfo.ubItemPreIdx = tPowerOnSubSubMenuItem.tSubMenuInfo.ubItemIdx;
	switch(Touch_Info->Gesture)
	{
		case TOUCH_PRESS:	
			for(i = 0; i < POWERON_MAX - 1;i++)
			{	
				tOSD_GetOsdImgInfor(1, OSD_IMG2, uwPowerOnMenuItemOsdImg[i], 1, &tOsdImgInfo);	
				if(Touch_Info->startX > tOsdImgInfo.uwXStart 
					&& Touch_Info->startX < tOsdImgInfo.uwXStart + tOsdImgInfo.uwHSize
					&& Touch_Info->startY > tOsdImgInfo.uwYStart 
					&& Touch_Info->startY < tOsdImgInfo.uwYStart + tOsdImgInfo.uwVSize)
				{
					tPowerOnSubSubMenuItem.tSubMenuInfo.ubItemIdx = i;
					break;
				}
			}
			if(i < POWERON_MAX - 1)
			{
				if(tUI_CuSetting.tPowerOnMode == POWERON_LAST)
				{
					tUI_CuSetting.tOldPowerOnMode = tUI_CuSetting.tPowerOnMode = tPowerOnSubSubMenuItem.tSubMenuInfo.ubItemIdx;
				}
				else
				{
					tUI_CuSetting.tOldPowerOnMode = tPowerOnSubSubMenuItem.tSubMenuInfo.ubItemIdx;
					
					if(tPowerOnSubSubMenuItem.tSubMenuInfo.ubItemIdx == tUI_CuSetting.tPowerOnMode)
						tUI_CuSetting.tPowerOnMode = POWERON_LAST;
					else
						tUI_CuSetting.tPowerOnMode = tPowerOnSubSubMenuItem.tSubMenuInfo.ubItemIdx;
					
				}
				UI_PowerOnDrawSubSubMenuItem();
			}
			else
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

		default:

			return;
	}

	
}



//非特殊界面都可以在下面函数中进行触摸解析
//------------------------------------------------------------------------------universal-----------------------------------------------------------------------

//该数组存的是每个界面的起始图片序号
static uint16_t uwFirstImageIndexOfSettingMenu[SETTINGITEM_MAX] = 
{
	OSD2IMG_CAMSCAM1NOR_ICON,//camera
	0,//record
	0,//system
	0,//dual
	0,//quad
	OSD2IMG_TRIGGER_DELAY_CAM1_NOR,//trigger
	0,//autoScan
	OSD2IMG_POWERON_CAM1_NOR,//poweron
	0,//parkingLine
	0,//guideLine
	0,//AI
};
//------------------------------------------------------------------------------

void UI_SettingSubSubTouchMenuPage(TOUCH_EVENT_t *Touch_Info)
{
	uint16_t uwSubMenuItem = tUI_SubMenuItem[SETTING_ITEM].tSubMenuInfo.ubItemIdx;//sub menu

	UI_SettingUpdateSubSubMenuItemIndex(uwSubMenuItem);
	
	tSettingSubSubMenuItem.tSettingS[uwSubMenuItem].tSubMenuInfo.ubItemPreIdx = 
		tSettingSubSubMenuItem.tSettingS[uwSubMenuItem].tSubMenuInfo.ubItemIdx;//sub sub menu	
		
	uint16_t i = 0;
	OSD_IMG_INFO tOsdImgInfo;
	
	switch(Touch_Info->Gesture)
	{
		case TOUCH_PRESS:	
			//值为0就返回
			if(!uwFirstImageIndexOfSettingMenu[uwSubMenuItem])
				return;
			for(i = 0; i < tSettingSubSubMenuItem.tSettingS[uwSubMenuItem].ubItemCount;i++)//判断具体点击在哪个图标上
			{	
				//读取当前操作界面的全部图标信息
				tOSD_GetOsdImgInfor(1, OSD_IMG2, uwFirstImageIndexOfSettingMenu[uwSubMenuItem] + 2*i, 1, &tOsdImgInfo);	
				if(Touch_Info->startX > tOsdImgInfo.uwXStart 
					&& Touch_Info->startX < tOsdImgInfo.uwXStart + tOsdImgInfo.uwHSize
					&& Touch_Info->startY > tOsdImgInfo.uwYStart 
					&& Touch_Info->startY < tOsdImgInfo.uwYStart + tOsdImgInfo.uwVSize)
				{
					tSettingSubSubMenuItem.tSettingS[uwSubMenuItem].tSubMenuInfo.ubItemIdx = i;
					break;
				}
			}
			
			if(i < tSettingSubSubMenuItem.tSettingS[uwSubMenuItem].ubItemCount)
			{
				if(tSettingSubSubMenuItem.tSettingS[uwSubMenuItem].tSubMenuInfo.ubItemPreIdx != 
							tSettingSubSubMenuItem.tSettingS[uwSubMenuItem].tSubMenuInfo.ubItemIdx)
				{
					UI_SettingingDrawSubSubMenuItem(uwSubMenuItem);
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

#include "UI_BUCCU[WSVGA]_SUBMENU.h"
#ifdef BSP_D_SNCC71_GM8285C_RX_V2

void UI_AdoSelSubTouchMenu(TOUCH_EVENT_t *Touch_Info);
void UI_CamSelSubTouchMenu(TOUCH_EVENT_t *Touch_Info);
void UI_PairingSubTouchMenu(TOUCH_EVENT_t *Touch_Info);
void UI_PlaybackSubTouchMenu(TOUCH_EVENT_t *Touch_Info);
void UI_SettingSubTouchMenu(TOUCH_EVENT_t *Touch_Info);
void UI_PowerSubTouchMenu(TOUCH_EVENT_t *Touch_Info);
void UI_SettingSubMenuPage(UI_ArrowKey_t tArrowKey);
extern uint8_t ubDrawBSDRange;

//------------------------------------------------------------------------------
UI_MenuAct_t UI_KeyEventMap2SubMenuInfo(UI_ArrowKey_t *ptArrowKey, UI_SubMenuItem_t *ptSubMenu)
{
	uint8_t ubCurrentSubMenuItemIdx = ptSubMenu->tSubMenuInfo.ubItemIdx;
	uint8_t ubNextSubMenuItemIdx = 0;

	switch(*ptArrowKey)
	{
		case LEFT_ARROW:
			if(ubCurrentSubMenuItemIdx <= ptSubMenu->ubFirstItem)
				ubNextSubMenuItemIdx = ptSubMenu->ubItemCount -1;
			else
				ubNextSubMenuItemIdx = ubCurrentSubMenuItemIdx - 1;
			break;
		case RIGHT_ARROW:
			if((ubCurrentSubMenuItemIdx+1) >= ptSubMenu->ubItemCount)
				ubNextSubMenuItemIdx = ptSubMenu->ubFirstItem;
			else
				ubNextSubMenuItemIdx = ubCurrentSubMenuItemIdx + 1;
			break;
		case ENTER_ARROW:
			return DRAW_MENUPAGE;
		case EXIT_ARROW:
			return EXIT_MENUFUNC;
		default:
			return NOT_ACTION;
	}
	ptSubMenu->tSubMenuInfo.ubItemPreIdx = ubCurrentSubMenuItemIdx;
	ptSubMenu->tSubMenuInfo.ubItemIdx    = ubNextSubMenuItemIdx;
	return DRAW_HIGHLIGHT_MENUICON;
}
//------------------------------------------------------------------------------
static const uint16_t ado_button_x[3][5] = {
	{411, 411+80+42, 0, 0, 0},
	{352, 352+80+40, 352+80+40+80+40, 0, 0},
	{(1024-5*80-4*42)/2, (1024-5*80-4*42)/2 + 80 + 42, (1024-5*80-4*42)/2 + 2*(80 + 42), (1024-5*80-4*42)/2 + 3*(80 + 42), (1024-5*80-4*42)/2 + 4*(80 + 42)},
};

void UI_DrawAdoSelSubMenuPage()
{
	OSD_IMG_INFO tOsdImgInfo[10];
	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_SELADOCAM1_NOR_ICON, 10, &tOsdImgInfo[0]);
	
	if(SINGLE_VIEW == tCamViewSel.tCamViewType) {
		tOsdImgInfo[2 * tCamViewSel.tCamViewPool[0]].uwXStart = ado_button_x[0][0];
		tOsdImgInfo[8].uwXStart = ado_button_x[0][1];
		tOSD_Img2(&tOsdImgInfo[2 * tCamViewSel.tCamViewPool[0]], OSD_QUEUE);
		tOSD_Img2(&tOsdImgInfo[8], OSD_QUEUE);
	} else if(DUAL_VIEW == tCamViewSel.tCamViewType) {
		tOsdImgInfo[2 * tCamViewSel.tCamViewPool[0]].uwXStart = ado_button_x[1][0];
		tOsdImgInfo[2 * tCamViewSel.tCamViewPool[1]].uwXStart = ado_button_x[1][1];
		tOsdImgInfo[8].uwXStart = ado_button_x[1][2];
		tOSD_Img2(&tOsdImgInfo[2 * tCamViewSel.tCamViewPool[0]], OSD_QUEUE);
		tOSD_Img2(&tOsdImgInfo[2 * tCamViewSel.tCamViewPool[1]], OSD_QUEUE);
		tOSD_Img2(&tOsdImgInfo[8], OSD_QUEUE);
		
		if(UI_GetAudioSrc() != tCamViewSel.tCamViewPool[0] && 
			UI_GetAudioSrc() != tCamViewSel.tCamViewPool[1] &&
			UI_GetAudioSrc() != 4) {
			UI_SetAudioSrc((UI_CamSource)tCamViewSel.tCamViewPool[0]);
		}
		
	} else if(QUAD_VIEW == tCamViewSel.tCamViewType) {
		tOsdImgInfo[0].uwXStart = ado_button_x[2][0];
		tOsdImgInfo[2].uwXStart = ado_button_x[2][1];
		tOsdImgInfo[4].uwXStart = ado_button_x[2][2];
		tOsdImgInfo[6].uwXStart = ado_button_x[2][3];
		tOsdImgInfo[8].uwXStart = ado_button_x[2][4];
		for(uint8_t i = 0;i < 5;i++)
			tOSD_Img2(&tOsdImgInfo[2*i], OSD_QUEUE);
	}

	if(DUAL_VIEW == tCamViewSel.tCamViewType) {
		if(UI_GetAudioSrc() == tCamViewSel.tCamViewPool[0]) {
			tOsdImgInfo[2*UI_GetAudioSrc() + UI_ICON_HIGHLIGHT].uwXStart = ado_button_x[1][0];
		} else if(UI_GetAudioSrc() == tCamViewSel.tCamViewPool[1]) {
			tOsdImgInfo[2*UI_GetAudioSrc() + UI_ICON_HIGHLIGHT].uwXStart = ado_button_x[1][1];
		} else {
			tOsdImgInfo[2*UI_GetAudioSrc() + UI_ICON_HIGHLIGHT].uwXStart = ado_button_x[1][2];
		}
	} else {
		tOsdImgInfo[2*UI_GetAudioSrc() + UI_ICON_HIGHLIGHT].uwXStart = tOsdImgInfo[2*UI_GetAudioSrc()].uwXStart;
	}
	
	tOSD_Img2(&tOsdImgInfo[2*UI_GetAudioSrc() + UI_ICON_HIGHLIGHT], OSD_UPDATE);
	tUI_SubMenuItem[ADO_MODE_ITEM].tSubMenuInfo.ubItemIdx = UI_GetAudioSrc();		
}

//------------------------------------------------------------------------------

void UI_DrawCamSelSubMenuPage(void)
{
	OSD_IMG_INFO tOsdImgInfo[VDO_VIEW_MAX*2];
	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_SELCAM1NOR_ICON, VDO_VIEW_MAX*2, &tOsdImgInfo[0]);
	for(uint8_t i = 0;i < VDO_VIEW_MAX;i++)
		tOSD_Img2(&tOsdImgInfo[2*i], OSD_QUEUE);
					
	tUI_SubMenuItem[VDO_MODE_ITEM].tSubMenuInfo.ubItemIdx = tUI_ViewModeSel;			
	tOSD_Img2(&tOsdImgInfo[2*tUI_ViewModeSel + UI_ICON_HIGHLIGHT], OSD_UPDATE);
}
//------------------------------------------------------------------------------

void UI_DrawPairingSubMenuPage(void)
{
	OSD_IMG_INFO tOsdImgInfo[16],tCamRdyOsdImgInfo;
	
	tOSD_GetOsdImgInfor(1, OSD_IMG2, (tUI_CuSetting.tLanguage == LANGUAGE_ENGLISH)?OSD2IMG_PAIRING_TITLE:
									(tUI_CuSetting.tLanguage == LANGUAGE_GERMAN)?OSD2IMG_PAIRING_TITLE_GER:
									(tUI_CuSetting.tLanguage == LANGUAGE_FRENCH)?OSD2IMG_PAIRING_TITLE_FR:OSD2IMG_PAIRING_TITLE_CHN, 3, &tOsdImgInfo[0]);
	//word
	for(uint8_t i = 0;i < 3;i++)
		tOSD_Img2(&tOsdImgInfo[i], OSD_QUEUE);

	//cam1 ~cam4
	tOSD_GetOsdImgInfor(1, OSD_IMG2, (tUI_CuSetting.tLanguage != LANGUAGE_GERMAN)?OSD2IMG_PAIRCAM1NOR_ICON:OSD2IMG_PAIRCAM1NOR_ICON_GER, 8, &tOsdImgInfo[0]);
	for(uint8_t i = 0;i < 8;i++)
	{
		tOsdImgInfo[8 + i] = tOsdImgInfo[i];
		tOsdImgInfo[8 + i].uwYStart = PAIRING_Y + PAIRING_Y_STEP;
	}
	for(uint8_t i = 0;i < 8;i++)
		tOSD_Img2(&tOsdImgInfo[2*i], OSD_QUEUE);
	//pairing success flag
	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_PAIRRDYMASK_ICON, 1, &tCamRdyOsdImgInfo);
	for(uint16_t i = 0;i < 4;i++)
	{
		tOSD_Img2(&tOsdImgInfo[2*i], OSD_QUEUE);
		if(tUI_CamStatus[i].ulCAM_ID != INVALID_ID)
		{
			tCamRdyOsdImgInfo.uwXStart = tOsdImgInfo[2*i].uwXStart + (tOsdImgInfo[2*i].uwHSize - tCamRdyOsdImgInfo.uwHSize)/2;
			tOSD_Img2(&tCamRdyOsdImgInfo, OSD_QUEUE);
		}
	}	
	//highLight
	tOSD_Img2(&tOsdImgInfo[1], OSD_UPDATE);
}



//------------------------------------------------------------------------------
void UI_SearchDCIMFolder(void)
{
	uint16_t uwLdState = UI_SEARCH_DCIMFOLDER;

	UI_SetLdDispStatus(UI_OSDLDDISP_ON);
	osMessagePut(osUI_OsdLdDispQueue, &uwLdState, 0);
	osDelay(200);
	tUI_RecFoldersInfo.uwTotalRecFolderNum = ulKNL_GetSortingFolders(KNL_REAL_FLD,SORT_BY_NAME_DESCENDING,&tUI_RecFoldersInfo.tRecFolderInfo[0]);
	UI_SetLdDispStatus(UI_OSDLDDISP_OFF);
	osDelay(350);
}

//------------------------------------------------------------------------------
void UI_ListDCIMFolderInfo(uint16_t uwStartFileIdx, uint16_t uwEndFileIdx, OSD_UPDATE_TYP tUpdateMode)
{
	OSD_IMG_INFO tRecOsdImgInfo[5],tEraseOsdImgInfo;
	uint16_t uwRecXStart[2] = {0, 0}, uwRecYStart[2] = {0, 0};
	uint8_t ubRecFolderIdx, ubOpenOsdIdx = 0;
	uint16_t uwIdx;

	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_REC_FOLDERCLOSE_ICON, 2, &tRecOsdImgInfo[0]);

	uwRecXStart[0] = tRecOsdImgInfo[0].uwXStart;
	uwRecYStart[0] = tRecOsdImgInfo[0].uwYStart;
	uwRecXStart[1] = tRecOsdImgInfo[1].uwXStart;
	uwRecYStart[1] = tRecOsdImgInfo[1].uwYStart;
	for(uwIdx = uwStartFileIdx; uwIdx < uwEndFileIdx; uwIdx++)
	{
		UI_SetRecImgColor(UI_RECIMG_COLOR1);
		ubOpenOsdIdx = (tUI_RecFoldersInfo.uwRecFolderSelIdx == uwIdx)?1:0;
		ubRecFolderIdx = uwIdx % REC_FOLDER_LIST_MAXNUM;
		
		tRecOsdImgInfo[ubOpenOsdIdx].uwXStart += ((ubRecFolderIdx % 5) * 180);
		tRecOsdImgInfo[ubOpenOsdIdx].uwYStart += ((ubRecFolderIdx / 5) * 220);
		tOSD_Img2(&tRecOsdImgInfo[ubOpenOsdIdx], OSD_QUEUE);
		
		OSD_ImagePrintf(OSD_IMG_ROTATION_0, (72 + ((ubRecFolderIdx % 5) * 180)), (225 + ((ubRecFolderIdx / 5) * 220)), 
			tUI_RecOsdImgInfo, OSD_QUEUE, tUI_RecFoldersInfo.tRecFolderInfo[uwIdx].FldName.chName);
		
		tRecOsdImgInfo[ubOpenOsdIdx].uwXStart = uwRecXStart[ubOpenOsdIdx];
		tRecOsdImgInfo[ubOpenOsdIdx].uwYStart = uwRecYStart[ubOpenOsdIdx];
	}
	//页码和换页按钮
	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_PAGE_SLASH, 5, &tRecOsdImgInfo[0]);
	
	tOSD_Img2(&tRecOsdImgInfo[0], OSD_QUEUE);
	
	uint8_t ubTotalPageNum,ubCurrentPageNum;
	ubTotalPageNum = tUI_RecFoldersInfo.uwTotalRecFolderNum / REC_FOLDER_LIST_MAXNUM + ((tUI_RecFoldersInfo.uwTotalRecFolderNum % REC_FOLDER_LIST_MAXNUM)?1:0);
	ubCurrentPageNum = (tUI_RecFoldersInfo.uwRecFolderSelIdx + 1) / REC_FOLDER_LIST_MAXNUM + (((tUI_RecFoldersInfo.uwRecFolderSelIdx + 1) % REC_FOLDER_LIST_MAXNUM)?1:0);
	
	tEraseOsdImgInfo.uwXStart = tRecOsdImgInfo[0].uwXStart - 42;//擦除数值再重新显示，避免有残留
	tEraseOsdImgInfo.uwYStart = tRecOsdImgInfo[0].uwYStart + 16;
	tEraseOsdImgInfo.uwHSize  = 14 * 3;
	tEraseOsdImgInfo.uwVSize  = 28;
	OSD_EraserImg2_NoUpdate(&tEraseOsdImgInfo);
	
	if(ubCurrentPageNum < 10)
		UI_ShowPlaybackPageNum(ubCurrentPageNum,tRecOsdImgInfo[0].uwXStart - 14,tRecOsdImgInfo[0].uwYStart + 16,OSD_QUEUE);
	else if(ubCurrentPageNum < 100)
		UI_ShowPlaybackPageNum(ubCurrentPageNum,tRecOsdImgInfo[0].uwXStart - 28,tRecOsdImgInfo[0].uwYStart + 16,OSD_QUEUE);
	else if(ubCurrentPageNum < 1000)
		UI_ShowPlaybackPageNum(ubCurrentPageNum,tRecOsdImgInfo[0].uwXStart - 42,tRecOsdImgInfo[0].uwYStart + 16,OSD_QUEUE);

	UI_ShowPlaybackPageNum(ubTotalPageNum,tRecOsdImgInfo[0].uwXStart + 24,tRecOsdImgInfo[0].uwYStart + 16,tUpdateMode);
	if(ubUI_TouchPanelSts)
	{
		tOSD_Img2(&tRecOsdImgInfo[1], OSD_QUEUE);
		tOSD_Img2(&tRecOsdImgInfo[3], tUpdateMode);
	}
}
//------------------------------------------------------------------------------

void UI_DrawDCIMFolderMenu(void)
{
	OSD_IMG_INFO tRecBgOsdImgInfo;

	tOSD_GetOsdImgInfor(1, OSD_IMG1, OSD1IMG_SUBMENU2, 1, &tRecBgOsdImgInfo);
	OSD_Weight(OSD_WEIGHT_8DIV8);
	tOSD_Img1(&tRecBgOsdImgInfo, OSD_QUEUE);
	
	if(!ubUI_TouchPanelSts)
	{
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_RETURN_BLACK, 1, &tRecBgOsdImgInfo);
		tOSD_Img2(&tRecBgOsdImgInfo, OSD_QUEUE);
	}
	UI_SearchDCIMFolder();
	
	//显示文件夹信息
	if(tUI_RecFoldersInfo.uwTotalRecFolderNum)
	{
		uint8_t ubFldStartIdx = 0, ubFldEndIdx = 0;
		if(tUI_RecFoldersInfo.uwRecFolderSelIdx == 0)
		{
			ubFldStartIdx = 0;
			ubFldEndIdx = (tUI_RecFoldersInfo.uwTotalRecFolderNum < REC_FOLDER_LIST_MAXNUM)?tUI_RecFoldersInfo.uwTotalRecFolderNum:REC_FOLDER_LIST_MAXNUM;
		}
		else
		{
			ubFldStartIdx = (tUI_RecFoldersInfo.uwRecFolderSelIdx / REC_FOLDER_LIST_MAXNUM) * REC_FOLDER_LIST_MAXNUM;//该界面第一个文件夹的序号
			
			ubFldEndIdx = tUI_RecFoldersInfo.uwTotalRecFolderNum - ubFldStartIdx;//ubFldStartIdx之后总剩余文件夹数量
			ubFldEndIdx = (ubFldEndIdx < REC_FOLDER_LIST_MAXNUM)?tUI_RecFoldersInfo.uwTotalRecFolderNum:(ubFldStartIdx + REC_FOLDER_LIST_MAXNUM);

		}
		UI_ListDCIMFolderInfo(ubFldStartIdx, ubFldEndIdx, OSD_UPDATE);
	}
		
}

void UI_DrawPlaybackSubMenuPage(void)
{
	UI_DrawDCIMFolderMenu();
}
//------------------------------------------------------------------------------
void UI_DrawSettingSubMenuPage(void)
{
	OSD_IMG_INFO tOsdImgInfo[22];

	tOSD_GetOsdImgInfor(1, OSD_IMG2, (tUI_CuSetting.tLanguage == LANGUAGE_ENGLISH)?OSD2IMG_CAMERA_SETTING_WORD:
									(tUI_CuSetting.tLanguage == LANGUAGE_GERMAN)?OSD2IMG_CAMERA_SETTING_WORD_GER:
									(tUI_CuSetting.tLanguage == LANGUAGE_FRENCH)?OSD2IMG_CAMERA_SETTING_WORD_FR:OSD2IMG_CAMERA_SETTING_WORD_CHN, SETTINGITEM_MAX-1, &tOsdImgInfo[0]);
	//WORD
	for(uint8_t i = 0;i < SETTINGITEM_MAX-1;i++)
		tOSD_Img2(&tOsdImgInfo[i], OSD_QUEUE);
	//ai_word
//	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_AI_WORD, 1, &tOsdImgInfo[0]);
		
	tOSD_GetOsdImgInfor(1, OSD_IMG2, (tUI_CuSetting.tLanguage == LANGUAGE_ENGLISH)?OSD2IMG_AI_WORD:
									(tUI_CuSetting.tLanguage == LANGUAGE_GERMAN)?OSD2IMG_AI_WORD:
									(tUI_CuSetting.tLanguage == LANGUAGE_FRENCH)?OSD2IMG_AI_WORD:OSD2IMG_AI_WORD_CHN,1, &tOsdImgInfo[0]);
	tOSD_Img2(&tOsdImgInfo[0], OSD_QUEUE);


	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_CAMERA_SETTING_NOR_ICON, SETTINGITEM_MAX*2, &tOsdImgInfo[0]);
	for(uint8_t i = 0;i < SETTINGITEM_MAX;i++)
		tOSD_Img2(&tOsdImgInfo[2*i], OSD_QUEUE);
	tOSD_Img2(&tOsdImgInfo[1], OSD_UPDATE);
	

	tUI_SubMenuItem[SETTING_ITEM].tSubMenuInfo.ubItemIdx = CAMERASET_ITEM;

}
//------------------------------------------------------------------------------
void UI_DrawPowerSubMenuPage(void)
{
	OSD_IMG_INFO tOsdImgInfo[5];
	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_POWER_BG, 5, &tOsdImgInfo[0]);
	tOSD_Img2(&tOsdImgInfo[0], OSD_QUEUE);
	tOSD_Img2(&tOsdImgInfo[2], OSD_QUEUE);
	tOSD_Img2(&tOsdImgInfo[3], OSD_QUEUE);
	//WORD
	tOSD_GetOsdImgInfor(1, OSD_IMG2, (tUI_CuSetting.tLanguage == LANGUAGE_ENGLISH)?OSD2IMG_POWER_SCREENOFF_WORD:
									(tUI_CuSetting.tLanguage == LANGUAGE_GERMAN)?OSD2IMG_POWER_SCREENOFF_WORD_GER:
									(tUI_CuSetting.tLanguage == LANGUAGE_FRENCH)?OSD2IMG_POWER_SCREENOFF_WORD_FR:OSD2IMG_POWER_SCREENOFF_WORD_CHN, 2, &tOsdImgInfo[0]);	
									
	tOSD_Img2(&tOsdImgInfo[0], OSD_QUEUE);
	tOSD_Img2(&tOsdImgInfo[1], OSD_UPDATE);
}

//------------------------------------------------------------------------------

void UI_DrawSubMenuPage(UI_MenuItemList_t MenuItem)
{
	static UI_DrawSubMenuFuncPtr_t DrawSubMenuFunc[MENUITEM_MAX] =
	{
		UI_DrawAdoSelSubMenuPage,
		UI_DrawCamSelSubMenuPage,
		UI_DrawPairingSubMenuPage,
		NULL,
		UI_DrawPlaybackSubMenuPage,
		UI_DrawSettingSubMenuPage,
		UI_DrawPowerSubMenuPage,
	};
	OSD_IMG_INFO tOsdImgInfo;

	if(NULL == DrawSubMenuFunc[MenuItem].pvFuncPtr)
		return;
	printf("osSemaphoreWait Submenu!!!!\n");
	osSemaphoreWait(osUI_CuUiCtr, osWaitForever);
	if(MenuItem != ADO_MODE_ITEM && MenuItem != VDO_MODE_ITEM && MenuItem != POWER_ITEM)	
	{
		tOSD_GetOsdImgInfor(1, OSD_IMG1, OSD1IMG_SUBMENU2, 1, &tOsdImgInfo);
		tOSD_Img1(&tOsdImgInfo, OSD_QUEUE);
		if(!ubUI_TouchPanelSts)
		{
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_RETURN_BLACK, 1, &tOsdImgInfo);
			tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
		}
	}
	DrawSubMenuFunc[MenuItem].pvFuncPtr();
	osSemaphoreRelease(osUI_CuUiCtr);
	printf("osSemaphoreRelease Submenu!!!!\n");
}
//------------------------------------------------------------------------------

void UI_SubKeyMenu(UI_ArrowKey_t tArrowKey)
{
	static UI_MenuFuncPtr_t SubMenuFunc[MENUITEM_MAX] =
	{
		UI_AdoSelSubMenuPage,
		UI_CamSelSubMenuPage,  //画面选择
		UI_PairingSubMenuPage,
		NULL,
		UI_PlaybackSubMenuPage,		
		UI_SettingSubMenuPage,
		UI_PowerSubMenuPage,
	};
	if(tArrowKey == EXIT_ARROW)
	{
		tUI_State = UI_MAINMENU_STATE;
		UI_ClearOsdImageNoUpdate();
		ubDrawBSDRange = 0;
		UI_DrawDesktopIcon();
		ubDrawBSDRange = 1;
		UI_DrawMenuPage();
		for(int i = 0; i < MENUITEM_MAX; i++)
			memset(&tUI_SubMenuItem[i].tSubMenuInfo, 0, sizeof(UI_MenuItem_t));
		
		if(ubUI_RestartRec && ubUI_FinishViewSwitch)//重新启动录像
		{
			UI_VideoRecordingExec(UI_REC_START);
			ubUI_RestartRec = FALSE;
		}
		
		return;

	}
	if(SubMenuFunc[tUI_MenuItem.ubItemIdx].pvFuncPtr)
		SubMenuFunc[tUI_MenuItem.ubItemIdx].pvFuncPtr(tArrowKey);
}
//------------------------------------------------------------------------------adoSel-----------------------------------------------------------------------------------------
//------------------------------------------------------------------------------adoSel-----------------------------------------------------------------------------------------
//------------------------------------------------------------------------------adoSel-----------------------------------------------------------------------------------------
static void UI_AdoSelSubMenuExecute(void)
{
	UI_CamSource tAdoCamNumSel = (UI_CamSource)tUI_SubMenuItem[ADO_MODE_ITEM].tSubMenuInfo.ubItemIdx;
	UI_SetAudioSrc(tAdoCamNumSel);
	UI_SwitchAudioSource(tAdoCamNumSel);
	UI_AudioOn();
	UI_UpdateDevStatusInfo();
}
//------------------------------------------------------------------------------
static void UI_AdoSelDrawSubMenuItem(void)
{
	uint16_t pre_x, now_x;
	OSD_IMG_INFO tOsdImgInfo;
	
	uint16_t uwSubMenuItemOsdImg[5] = {
										OSD2IMG_SELADOCAM1_NOR_ICON, 
										OSD2IMG_SELADOCAM2_NOR_ICON,
										OSD2IMG_SELADOCAM3_NOR_ICON,
										OSD2IMG_SELADOCAM4_NOR_ICON,
										OSD2IMG_SELADOAI_NOR_ICON,
										};
			
	uint8_t ubSubMenuItemPreIdx = tUI_SubMenuItem[ADO_MODE_ITEM].tSubMenuInfo.ubItemPreIdx;
	uint8_t ubSubMenuItemIdx = tUI_SubMenuItem[ADO_MODE_ITEM].tSubMenuInfo.ubItemIdx;
											
	if(SINGLE_VIEW == tCamViewSel.tCamViewType) {		
		if(ubSubMenuItemPreIdx == 4) {
			pre_x = ado_button_x[0][1];
			now_x = ado_button_x[0][0];
		} else {
			pre_x = ado_button_x[0][0];
			now_x = ado_button_x[0][1];
		}
	} else if(DUAL_VIEW == tCamViewSel.tCamViewType) {
		if(ubSubMenuItemPreIdx == tCamViewSel.tCamViewPool[0]) {
			pre_x = ado_button_x[1][0];
		} else if(ubSubMenuItemPreIdx == tCamViewSel.tCamViewPool[1]) {
			pre_x = ado_button_x[1][1];
		} else {
			pre_x = ado_button_x[1][2];
		}
		
		if(ubSubMenuItemIdx == tCamViewSel.tCamViewPool[0]) {
			now_x = ado_button_x[1][0];
		} else if(ubSubMenuItemIdx == tCamViewSel.tCamViewPool[1]) {
			now_x = ado_button_x[1][1];
		} else {
			now_x = ado_button_x[1][2];
		}
	} else if(QUAD_VIEW == tCamViewSel.tCamViewType) {
		pre_x = ado_button_x[2][ubSubMenuItemPreIdx];
		now_x = ado_button_x[2][ubSubMenuItemIdx];
	}
						
	tOSD_GetOsdImgInfor(1, OSD_IMG2, uwSubMenuItemOsdImg[ubSubMenuItemPreIdx], 1, &tOsdImgInfo);
	tOsdImgInfo.uwXStart = pre_x;
	tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
			
	tOSD_GetOsdImgInfor(1, OSD_IMG2, uwSubMenuItemOsdImg[ubSubMenuItemIdx]+UI_ICON_HIGHLIGHT, 1, &tOsdImgInfo);
	tOsdImgInfo.uwXStart = now_x;
	tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
}
//------------------------------------------------------------------------------
UI_MenuAct_t UI_KeyEventMap2SubMenuInfo_ADO(UI_ArrowKey_t *ptArrowKey, UI_SubMenuItem_t *ptSubMenu)
{
	uint8_t ubCurrentSubMenuItemIdx = ptSubMenu->tSubMenuInfo.ubItemIdx;
	uint8_t ubNextSubMenuItemIdx = 0;
	
	switch(*ptArrowKey)
	{
		case LEFT_ARROW:
		case RIGHT_ARROW:
			break;
		case ENTER_ARROW:
			return DRAW_MENUPAGE;
		case EXIT_ARROW:
			return EXIT_MENUFUNC;
		default:
			return NOT_ACTION;
	}
	
	if(SINGLE_VIEW == tCamViewSel.tCamViewType) {		
		if(ubCurrentSubMenuItemIdx == 4) {
			ubNextSubMenuItemIdx = tCamViewSel.tCamViewPool[0];
		} else {
			ubNextSubMenuItemIdx = 4;
		}
	} else if(DUAL_VIEW == tCamViewSel.tCamViewType) {
		if(ubCurrentSubMenuItemIdx == tCamViewSel.tCamViewPool[0]) {
			if(*ptArrowKey == LEFT_ARROW){
				ubNextSubMenuItemIdx = 4;
			}else{
				ubNextSubMenuItemIdx = tCamViewSel.tCamViewPool[1];
			}
		} else if(ubCurrentSubMenuItemIdx == tCamViewSel.tCamViewPool[1]) {
			if(*ptArrowKey == LEFT_ARROW){
				ubNextSubMenuItemIdx = tCamViewSel.tCamViewPool[0];
			}else{
				ubNextSubMenuItemIdx = 4;
			}
		} else {
			if(*ptArrowKey == LEFT_ARROW){
				ubNextSubMenuItemIdx = tCamViewSel.tCamViewPool[1];
			}else{
				ubNextSubMenuItemIdx = tCamViewSel.tCamViewPool[0];
			}
		}
	} else {
		if(*ptArrowKey == LEFT_ARROW){
			if(ubCurrentSubMenuItemIdx <= ptSubMenu->ubFirstItem)
				ubNextSubMenuItemIdx = ptSubMenu->ubItemCount -1;
			else
				ubNextSubMenuItemIdx = ubCurrentSubMenuItemIdx - 1;
		} else {
			if((ubCurrentSubMenuItemIdx+1) >= ptSubMenu->ubItemCount)
				ubNextSubMenuItemIdx = ptSubMenu->ubFirstItem;
			else
				ubNextSubMenuItemIdx = ubCurrentSubMenuItemIdx + 1;
		}
	}
	
	ptSubMenu->tSubMenuInfo.ubItemPreIdx = ubCurrentSubMenuItemIdx;
	ptSubMenu->tSubMenuInfo.ubItemIdx    = ubNextSubMenuItemIdx;
	return DRAW_HIGHLIGHT_MENUICON;
}
void UI_AdoSelSubMenuPage(UI_ArrowKey_t tArrowKey)
{
//	if(tCamViewSel.tCamViewType == SINGLE_VIEW)//锟斤拷锟斤拷锟芥不锟斤拷锟斤拷锟叫伙拷锟斤拷频
//	{
//        UI_MenuKey();
//        UI_LeftArrowKey();
//		return;
//	}
	UI_MenuAct_t tMenuAct;
		
	if(tUI_State == UI_MAINMENU_STATE)
	{
		tUI_State = UI_SUBMENU_STATE;
		memset(&tUI_SubMenuItem[ADO_MODE_ITEM].tSubMenuInfo, 0, sizeof(UI_MenuItem_t));
		UI_DrawSubMenuPage(ADO_MODE_ITEM);
		return;
	}
	
	tMenuAct = UI_KeyEventMap2SubMenuInfo_ADO(&tArrowKey, &tUI_SubMenuItem[ADO_MODE_ITEM]);
	switch(tMenuAct)
	{
		case DRAW_HIGHLIGHT_MENUICON:
		{
			UI_AdoSelDrawSubMenuItem();
			break;
		}
		case DRAW_MENUPAGE:
		{
			UI_AdoSelSubMenuExecute();
			//回到desktop
			UI_ClearOsdImageNoUpdate();
			MenuOnFlag = FALSE;
			tUI_State = UI_DISPLAY_STATE;
			//ubDrawBSDRange = 0;
			UI_DrawDesktopIcon();
			//ubDrawBSDRange = 1;
			break;
		}
		case EXIT_MENUFUNC:
			break;
		default:
			break;
	}
	
	
}



//------------------------------------------------------------------------------camSel-----------------------------------------------------------------------------------------
//------------------------------------------------------------------------------camSel-----------------------------------------------------------------------------------------
//------------------------------------------------------------------------------camSel-----------------------------------------------------------------------------------------


static void UI_CamSelSubMenuExecute(void)
{
	tUI_ViewModeSel = (UI_CamNum_t)tUI_SubMenuItem[VDO_MODE_ITEM].tSubMenuInfo.ubItemIdx;

	if(tUI_ViewModeSel != SCANVIEW_ITEM)
	{
		ulAutoScanCount = 0;
		DeskTopShowView = tUI_ViewModeSel;
		UI_SwitchViewType(tUI_ViewModeSel,TRUE);
		
	}
	else 
	{
		UI_CamNum_t CamNum;
		//找出自动扫描第一个画面
		for(CamNum = CAM1;CamNum < AUTOSCAN_MAX/2;CamNum ++)
		{
			if(tUI_CuSetting.AutoScanEnable[CamNum])
				break;
		}
		DeskTopShowView = CamNum;
		UI_SwitchViewType(CamNum,TRUE);
		
		tUI_AutoScanNumSel = CamNum;
		ulAutoScanCount = tUI_CuSetting.AutoScanDuty[tUI_AutoScanNumSel]* 1000;
	}
	if(tUI_CuSetting.tPowerOnMode == POWERON_LAST)
		UI_SaveLastMode();


}
//------------------------------------------------------------------------------

static void UI_CamSelDrawSubMenuItem(void)
{
	uint16_t uwSubMenuItemOsdImg[8] = {
										OSD2IMG_SELCAM1NOR_ICON, 
										OSD2IMG_SELCAM2NOR_ICON,
										OSD2IMG_SELCAM3NOR_ICON,
										OSD2IMG_SELCAM4NOR_ICON,
										OSD2IMG_SEL4TDUALNOR_ICON,
										OSD2IMG_SELCAM4T_QUAD_NOR_ICON,
										OSD2IMG_SEL4TSCANNOR_ICON
										};
	uint8_t ubSubMenuItemPreIdx = tUI_SubMenuItem[VDO_MODE_ITEM].tSubMenuInfo.ubItemPreIdx;
	uint8_t ubSubMenuItemIdx = tUI_SubMenuItem[VDO_MODE_ITEM].tSubMenuInfo.ubItemIdx;
	UI_DrawHLandNormalIcon(uwSubMenuItemOsdImg[ubSubMenuItemPreIdx], 
		(uwSubMenuItemOsdImg[ubSubMenuItemIdx] + UI_ICON_HIGHLIGHT));

}
//------------------------------------------------------------------------------

void UI_CamSelSubMenuPage(UI_ArrowKey_t tArrowKey)
{
	UI_MenuAct_t tMenuAct;
	if(tUI_State == UI_MAINMENU_STATE)
	{
		tUI_State = UI_SUBMENU_STATE;
		memset(&tUI_SubMenuItem[VDO_MODE_ITEM].tSubMenuInfo, 0, sizeof(UI_MenuItem_t));
		UI_DrawSubMenuPage(VDO_MODE_ITEM);
		return;
	}
	tMenuAct = UI_KeyEventMap2SubMenuInfo(&tArrowKey, &tUI_SubMenuItem[VDO_MODE_ITEM]);
	switch(tMenuAct)
	{
		case DRAW_HIGHLIGHT_MENUICON:
		{
			UI_CamSelDrawSubMenuItem();
			break;
		}
		case DRAW_MENUPAGE:
		{
			UI_CamSelSubMenuExecute();
			//回到desktop
			UI_ClearOsdImageNoUpdate();
			MenuOnFlag = FALSE;
			tUI_State = UI_DISPLAY_STATE;
			
			//ubDrawBSDRange = 0;
			UI_DrawDesktopIcon();
			//ubDrawBSDRange = 1;
			break;
		}
		case EXIT_MENUFUNC:
			break;
		default:
			break;
	}


}

//------------------------------------------------------------------------------Pairing----------------------------------------------------------------------------------------
//------------------------------------------------------------------------------Pairing----------------------------------------------------------------------------------------
//------------------------------------------------------------------------------Pairing----------------------------------------------------------------------------------------

void UI_PairingSubMenuExecute(void)
{
	uint16_t tSubMenuItem = tUI_SubMenuItem[PAIRING_ITEM].tSubMenuInfo.ubItemIdx;
	OSD_IMG_INFO tOsdImgInfo;
	if(tSubMenuItem <= PAIRINGCAM4_ITEM)
	{
		APP_EventMsg_t tUI_PairMessage = {0};
		tPairInfo.tPairSelCam = (UI_CamNum_t)tSubMenuItem;
		tPairInfo.ubDrawFlag = TRUE;
		tUI_State = UI_PAIRING_STATE;
		tUI_PairMessage.ubAPP_Event 	 = APP_PAIRING_START_EVENT;
		tUI_PairMessage.ubAPP_Message[0] = 2;		//! Message Length
		tUI_PairMessage.ubAPP_Message[1] = tPairInfo.tPairSelCam;
		tUI_PairMessage.ubAPP_Message[2] = tUI_CamStatus[tPairInfo.tPairSelCam].tCamDispLocation;
		tUI_PairMessage.ubAPP_Message[3] = FALSE;
		UI_SendMessageToAPP(&tUI_PairMessage);
	}
	else
	{
		OSD_IMG_INFO tDelOsdInfo;

		UI_CamNum_t tUI_DelCam = (UI_CamNum_t)(tSubMenuItem - 4), tSwCamNum;
		if(INVALID_ID == tUI_CamStatus[tUI_DelCam].ulCAM_ID)
		{
			return;
		}
		UI_UnBindCam(tUI_DelCam);
		
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_PAIRRDYMASK_ICON, 1, &tDelOsdInfo);
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_PAIRCAM1NOR_ICON + 2*tUI_DelCam, 1, &tOsdImgInfo);
		
		tDelOsdInfo.uwXStart = tOsdImgInfo.uwXStart + (tOsdImgInfo.uwHSize - tDelOsdInfo.uwHSize)/2;
		OSD_EraserImg2(&tDelOsdInfo);
	}

}

//-----------------------------------------------------------------------------
static void UI_PairingDrawSubMenuItem(void)
{
	uint16_t uwFirstItemOsdImg = (tUI_CuSetting.tLanguage != LANGUAGE_GERMAN)?OSD2IMG_PAIRCAM1NOR_ICON:OSD2IMG_PAIRCAM1NOR_ICON_GER;
	uint8_t ubItemPreIdx = tUI_SubMenuItem[PAIRING_ITEM].tSubMenuInfo.ubItemPreIdx;
	uint8_t ubItemIdx = tUI_SubMenuItem[PAIRING_ITEM].tSubMenuInfo.ubItemIdx;
	OSD_IMG_INFO tOsdImgInfo;
	for(uint8_t i = 0;i < 2;i++)
	{
		uint8_t ubItem = (i == 0)?ubItemPreIdx:ubItemIdx;
		tOSD_GetOsdImgInfor(1, OSD_IMG2, uwFirstItemOsdImg + 2*(ubItem%4) + i, 1, &tOsdImgInfo);
		tOsdImgInfo.uwYStart = PAIRING_Y + PAIRING_Y_STEP*(ubItem/4);
		tOSD_Img2(&tOsdImgInfo, (i == 0)?OSD_QUEUE:OSD_UPDATE);
	}

}

//-----------------------------------------------------------------------------
void UI_PairingSubMenuPage(UI_ArrowKey_t tArrowKey)
{
	UI_MenuAct_t tMenuAct;

	if(tUI_State == UI_MAINMENU_STATE)
	{
		tUI_State = UI_SUBMENU_STATE;
		memset(&tUI_SubMenuItem[PAIRING_ITEM].tSubMenuInfo, 0, sizeof(UI_MenuItem_t));
		UI_DrawSubMenuPage(PAIRING_ITEM);
		if(UI_REC_START == tUI_RecPlayAct.tRecAct)//进入对接界面需要停止录像
		{
			UI_VideoRecordingExec(UI_REC_STOP);
			ubUI_RestartRec = TRUE;
		}
		return;
	}
	tMenuAct = UI_KeyEventMap2SubMenuInfo(&tArrowKey, &tUI_SubMenuItem[PAIRING_ITEM]);
	switch(tMenuAct)
	{
		case DRAW_HIGHLIGHT_MENUICON:
		{
			UI_PairingDrawSubMenuItem();
			break;
		}
		case DRAW_MENUPAGE:
		{
			UI_PairingSubMenuExecute();
			break;
		}
		case EXIT_MENUFUNC:
			break;
		default:
			break;
	}
}

//------------------------------------------------------------------------------playback---------------------------------------------------------------------------
//------------------------------------------------------------------------------playback---------------------------------------------------------------------------
//------------------------------------------------------------------------------playback---------------------------------------------------------------------------
void UI_ListRecFileInfo(uint16_t uwStartFileIdx, uint16_t uwEndFileIdx, OSD_UPDATE_TYP tUpdateMode)
{
	UI_CamNum_t tRecSelCamNum;
	KNL_ROLE tRecRoleNum;
	OSD_IMG_INFO tOsdImgInfo,tEraseOsdImgInfo;
	uint16_t uwIdx;
	int iGrpRIdx = 1;
	char cRecFileName[32], cRecFileCreated[20];
	char cListGrp[9];
	tEraseOsdImgInfo.uwXStart = 0;
	tEraseOsdImgInfo.uwYStart = 62;
	tEraseOsdImgInfo.uwHSize  = 1024;
	tEraseOsdImgInfo.uwVSize  = 450;
	OSD_EraserImg2_NoUpdate(&tEraseOsdImgInfo);

	for(uwIdx = uwStartFileIdx; uwIdx < uwEndFileIdx; uwIdx++)
	{
		
		tRecRoleNum = VDO_KNLSrcNumMap2KNLRoleNum((KNL_SRC)tUI_RecFilesInfo.tRecFilesInfo[uwIdx].HidnFileInfo.SrcNum);
		if(KNL_NONE == tRecRoleNum)
		{
			printd(DBG_ErrorLvl, "Get kernel role info Err!\n");
			tRecRoleNum = KNL_STA1;
		}
		APP_KNLRoleMap2CamNum(tRecRoleNum, tRecSelCamNum);
		tOSD_GetOsdImgInfor(1, OSD_IMG2, ((tUI_CuSetting.tLanguage != LANGUAGE_GERMAN)?OSD2IMG_FILE_CAM1_WORD:OSD2IMG_FILE_CAM1_WORD_GER) + tRecSelCamNum, 1, &tOsdImgInfo);
		tOsdImgInfo.uwYStart = 85 + (uwIdx % REC_FILE_LIST_MAXNUM) * 50;
		tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
		
		UI_SetRecImgColor(UI_RECIMG_COLOR1);
		memset(cRecFileName, 0, sizeof(cRecFileName));
		snprintf(cRecFileName, sizeof(cRecFileName), "%s.%s", tUI_RecFilesInfo.tRecFilesInfo[uwIdx].HidnFileInfo.FileName.chName, tUI_RecFilesInfo.tRecFilesInfo[uwIdx].HidnFileInfo.FileName.chExt);
		cRecFileName[(tUI_RecFilesInfo.tRecFilesInfo[uwIdx].HidnFileInfo.FileName.ubLen + 4)] = 0;
		OSD_ImagePrintf(OSD_IMG_ROTATION_0, 163, (85 + ((uwIdx % REC_FILE_LIST_MAXNUM) * 50)), tUI_RecOsdImgInfo, OSD_QUEUE, cRecFileName);

		snprintf(cListGrp, sizeof(cListGrp), "Grp%d", tUI_RecFilesInfo.tRecFilesInfo[uwIdx].HidnFileInfo.uwGroupIdx);
		OSD_ImagePrintf(OSD_IMG_ROTATION_0, 795, (85 + ((uwIdx % REC_FILE_LIST_MAXNUM) * 50)), tUI_RecOsdImgInfo, OSD_QUEUE, cListGrp);

		memset(cRecFileCreated, 0, sizeof(cRecFileCreated));
		snprintf(cRecFileCreated, sizeof(cRecFileCreated), "%02d-%02d-%02d %02d:%02d", tUI_RecFilesInfo.tRecFilesInfo[uwIdx].HidnFileInfo.CreTime.uwYear, tUI_RecFilesInfo.tRecFilesInfo[uwIdx].HidnFileInfo.CreTime.ubMonth, tUI_RecFilesInfo.tRecFilesInfo[uwIdx].HidnFileInfo.CreTime.ubDay,
																					   tUI_RecFilesInfo.tRecFilesInfo[uwIdx].HidnFileInfo.CreTime.ubHour, tUI_RecFilesInfo.tRecFilesInfo[uwIdx].HidnFileInfo.CreTime.ubMin);
		OSD_ImagePrintf(OSD_IMG_ROTATION_0, 425, (85 + ((uwIdx % REC_FILE_LIST_MAXNUM) * 50)), tUI_RecOsdImgInfo,OSD_QUEUE, cRecFileCreated);
	}

	OSD_IMG_INFO tRecOsdImgInfo[5];
	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_PAGE_SLASH, 5, &tRecOsdImgInfo[0]);
	
	tOSD_Img2(&tRecOsdImgInfo[0], OSD_QUEUE);
	
	uint8_t ubTotalPageNum,ubCurrentPageNum;
	ubTotalPageNum = tUI_RecFilesInfo.uwTotalRecFileNum / REC_FILE_LIST_MAXNUM + ((tUI_RecFilesInfo.uwTotalRecFileNum % REC_FILE_LIST_MAXNUM)?1:0);
	ubCurrentPageNum = (tUI_RecFilesInfo.uwRecFileSelIdx + 1) / REC_FILE_LIST_MAXNUM + (((tUI_RecFilesInfo.uwRecFileSelIdx + 1) % REC_FILE_LIST_MAXNUM)?1:0);

	tEraseOsdImgInfo.uwXStart = tRecOsdImgInfo[0].uwXStart - 42;
	tEraseOsdImgInfo.uwYStart = tRecOsdImgInfo[0].uwYStart + 16;
	tEraseOsdImgInfo.uwHSize  = 14 * 3;
	tEraseOsdImgInfo.uwVSize  = 28;
	OSD_EraserImg2_NoUpdate(&tEraseOsdImgInfo);
	
	if(ubCurrentPageNum < 10)
		UI_ShowPlaybackPageNum(ubCurrentPageNum,tRecOsdImgInfo[0].uwXStart - 14,tRecOsdImgInfo[0].uwYStart + 16,OSD_QUEUE);
	else if(ubCurrentPageNum < 100)
		UI_ShowPlaybackPageNum(ubCurrentPageNum,tRecOsdImgInfo[0].uwXStart - 28,tRecOsdImgInfo[0].uwYStart + 16,OSD_QUEUE);
	else if(ubCurrentPageNum < 1000)
		UI_ShowPlaybackPageNum(ubCurrentPageNum,tRecOsdImgInfo[0].uwXStart - 42,tRecOsdImgInfo[0].uwYStart + 16,OSD_QUEUE);

	UI_ShowPlaybackPageNum(ubTotalPageNum,tRecOsdImgInfo[0].uwXStart + 24,tRecOsdImgInfo[0].uwYStart + 16,tUpdateMode);
	
	if(ubUI_TouchPanelSts)
	{
		tOSD_Img2(&tRecOsdImgInfo[1], OSD_QUEUE);
		tOSD_Img2(&tRecOsdImgInfo[3], tUpdateMode);
	}
}
//------------------------------------------------------------------------------
void UI_SearchRecordFile(uint8_t ubRecFolderIndex)
{
	uint16_t uwLdState = UI_SEARCH_RECFILES;

	ubRecFolderIndex = ubRecFolderIndex;
	UI_SetLdDispStatus(UI_OSDLDDISP_ON);
	osMessagePut(osUI_OsdLdDispQueue, &uwLdState, 0);
	osDelay(200);
	tUI_RecFilesInfo.uwTotalRecFileNum = uwKNL_GetSortingFiles(ubRecFolderIndex, SORT_BY_TIME_DESCENDING, tUI_RecFilesInfo.tRecFilesInfo);
	UI_SetLdDispStatus(UI_OSDLDDISP_OFF);
	osDelay(500);
}
//------------------------------------------------------------------------------

void UI_DrawRecordFileMenu(void)
{
	OSD_IMG_INFO tOsdImgInfo[5];
	uint16_t uwFileStartIdx = 0, uwFileEndIdx = 0;
	uint8_t ubIdx;
	
	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_FILE_HEADLINE, 5, &tOsdImgInfo[0]);
	tOSD_GetOsdImgInfor(1, OSD_IMG2, (tUI_CuSetting.tLanguage == LANGUAGE_ENGLISH)?OSD2IMG_FILE_HEADLINE:
									(tUI_CuSetting.tLanguage == LANGUAGE_GERMAN)?OSD2IMG_FILE_HEADLINE_GER:
									(tUI_CuSetting.tLanguage == LANGUAGE_FRENCH)?OSD2IMG_FILE_HEADLINE_FR:OSD2IMG_FILE_HEADLINE_CHN, 1, &tOsdImgInfo[0]);

	UI_SearchRecordFile(tUI_RecFoldersInfo.uwRecFolderSelIdx);
	tOSD_Img2(&tOsdImgInfo[0], OSD_UPDATE);
	if(tUI_RecFilesInfo.uwTotalRecFileNum)
	{
		uwFileStartIdx = (tUI_RecFilesInfo.uwRecFileSelIdx / REC_FILE_LIST_MAXNUM) * REC_FILE_LIST_MAXNUM;
		uwFileEndIdx   = tUI_RecFilesInfo.uwTotalRecFileNum - uwFileStartIdx;
		uwFileEndIdx   = (uwFileEndIdx < REC_FILE_LIST_MAXNUM)?tUI_RecFilesInfo.uwTotalRecFileNum:(uwFileStartIdx + REC_FILE_LIST_MAXNUM);
	
		UI_ListRecFileInfo(uwFileStartIdx, uwFileEndIdx, OSD_QUEUE);
		//BOX
		for(ubIdx = 0; ubIdx < 4; ubIdx++)
		{
			tOsdImgInfo[1 + ubIdx].uwYStart += ((tUI_RecFilesInfo.uwRecFileSelIdx % REC_FILE_LIST_MAXNUM) * 50);
			tOSD_Img2(&tOsdImgInfo[1 + ubIdx], OSD_UPDATE);
		}
	}
	tUI_State = UI_SUBSUBMENU_STATE;
}
//------------------------------------------------------------------------------

void UI_PlaybackSubMenuPage(UI_ArrowKey_t tArrowKey)
{
	OSD_IMG_INFO tFolderSelOsdImgInfo[2], tOsdImgInfo;
	uint8_t ubUI_RecFolderIdx = 0, ubUI_PrevRecFolderIdx = 0, ubRecSelIdx;
	uint8_t ubRefreshFldIconFlag = FALSE;
	if(UI_MAINMENU_STATE == tUI_State)
	{
		tUI_RecFoldersInfo.uwRecFolderSelIdx = 0;
		tUI_State = UI_SUBMENU_STATE;
		UI_DrawSubMenuPage(PLAYBACK_ITEM);
		if(UI_REC_START == tUI_RecPlayAct.tRecAct)//进入播放器就停止录像
		{
			UI_VideoRecordingExec(UI_REC_STOP);
			ubUI_RestartRec = TRUE;
		}
		return;
	}

	if((!tUI_RecFoldersInfo.uwTotalRecFolderNum) && (EXIT_ARROW != tArrowKey))
		return;

	if(tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_REC_FOLDERCLOSE_ICON, 2, &tFolderSelOsdImgInfo[0]) != OSD_OK)
	{
		printd(DBG_ErrorLvl, "Load OSD Image FAIL, pls check (%d) !\n", __LINE__);
		return;
	}
	ubUI_RecFolderIdx = (tUI_RecFoldersInfo.uwRecFolderSelIdx % REC_FOLDER_LIST_MAXNUM);
	ubUI_PrevRecFolderIdx = ubUI_RecFolderIdx;
	
	switch(tArrowKey)
	{
		case LEFT_ARROW:
			if(!(ubUI_RecFolderIdx % REC_FOLDER_LIST_MAXNUM))//当前在第一个文件夹
			{
				if(tUI_RecFoldersInfo.uwRecFolderSelIdx > 0)//前面还有文件夹，切换到前一页
				{
					ubRefreshFldIconFlag = TRUE;
				}
				else
					return;//已经是第一个文件夹
					
			}
		
			tUI_RecFoldersInfo.uwRecFolderSelIdx --;
			ubUI_RecFolderIdx = (tUI_RecFoldersInfo.uwRecFolderSelIdx % REC_FOLDER_LIST_MAXNUM);
			break;
		case RIGHT_ARROW:
			if(tUI_RecFoldersInfo.uwTotalRecFolderNum / REC_FOLDER_LIST_MAXNUM == tUI_RecFoldersInfo.uwRecFolderSelIdx /REC_FOLDER_LIST_MAXNUM)//当前在最后一页
			{
				if(tUI_RecFoldersInfo.uwTotalRecFolderNum == tUI_RecFoldersInfo.uwRecFolderSelIdx + 1)//当前在最后一个文件夹
					return;
			}
			else
			{
				if(tUI_RecFoldersInfo.uwRecFolderSelIdx % REC_FOLDER_LIST_MAXNUM == REC_FOLDER_LIST_MAXNUM - 1)//当前在最后一个文件夹
					ubRefreshFldIconFlag = TRUE;
			}
			tUI_RecFoldersInfo.uwRecFolderSelIdx ++;
			ubUI_RecFolderIdx = (tUI_RecFoldersInfo.uwRecFolderSelIdx % REC_FOLDER_LIST_MAXNUM);
			break;
		case ENTER_ARROW:
			tOSD_GetOsdImgInfor(1, OSD_IMG1, OSD1IMG_SUBMENU2, 1, &tOsdImgInfo);
			tOSD_Img1(&tOsdImgInfo, OSD_QUEUE);
			if(!ubUI_TouchPanelSts)
			{
				tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_RETURN_BLACK, 1, &tOsdImgInfo);
				printf("x = %d y = %d \n", tOsdImgInfo.uwXStart, tOsdImgInfo.uwYStart);
				tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
			}

			tUI_RecFilesInfo.uwRecFileSelIdx = 0;
			UI_DrawRecordFileMenu();
			SendIrCodeFlag = 1;
			return;
		case EXIT_ARROW:
			
			return;
		default:
			return;
	}
	if(TRUE == ubRefreshFldIconFlag)//翻页
	{
		uint8_t ubFldStartIdx = 0, ubFldEndIdx = 0;

		tOSD_GetOsdImgInfor(1, OSD_IMG1, OSD1IMG_SUBMENU2, 1, &tOsdImgInfo);
		tOSD_Img1(&tOsdImgInfo, OSD_QUEUE);
		if(!ubUI_TouchPanelSts)
		{
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_RETURN_BLACK, 1, &tOsdImgInfo);
			tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
		}
		ubFldStartIdx = (tUI_RecFoldersInfo.uwRecFolderSelIdx / REC_FOLDER_LIST_MAXNUM) * REC_FOLDER_LIST_MAXNUM;//该界面第一个文件夹的序号
		if(tArrowKey == LEFT_ARROW)
		{
			ubFldEndIdx = ubFldStartIdx + REC_FOLDER_LIST_MAXNUM;
		}
		else
		{
			ubFldEndIdx = tUI_RecFoldersInfo.uwTotalRecFolderNum - tUI_RecFoldersInfo.uwRecFolderSelIdx;
			ubFldEndIdx = (ubFldEndIdx < REC_FOLDER_LIST_MAXNUM)?tUI_RecFoldersInfo.uwTotalRecFolderNum:(ubFldStartIdx + REC_FOLDER_LIST_MAXNUM);

		}
		
		UI_ListDCIMFolderInfo(ubFldStartIdx, ubFldEndIdx, OSD_UPDATE);
	}
	if(FALSE == ubRefreshFldIconFlag)//刷新文件夹选中状态
	{
		ubRecSelIdx = (tUI_RecFoldersInfo.uwRecFolderSelIdx / REC_FOLDER_LIST_MAXNUM) * REC_FOLDER_LIST_MAXNUM;//该界面第一个文件夹的序号
		UI_SetRecImgColor(UI_RECIMG_COLOR1);
		tFolderSelOsdImgInfo[0].uwXStart += ((ubUI_PrevRecFolderIdx % 5) * 180);
		tFolderSelOsdImgInfo[0].uwYStart += ((ubUI_PrevRecFolderIdx / 5) * 220);
		tOSD_Img2(&tFolderSelOsdImgInfo[0], OSD_QUEUE);
		OSD_ImagePrintf(OSD_IMG_ROTATION_0, (72 + ((ubUI_PrevRecFolderIdx % 5) * 180)), (225 + ((ubUI_PrevRecFolderIdx / 5) * 220)),
						tUI_RecOsdImgInfo, OSD_QUEUE, tUI_RecFoldersInfo.tRecFolderInfo[ubUI_PrevRecFolderIdx + ubRecSelIdx].FldName.chName);
		UI_SetRecImgColor(UI_RECIMG_COLOR1);
		tFolderSelOsdImgInfo[1].uwXStart += ((ubUI_RecFolderIdx % 5) * 180);
		tFolderSelOsdImgInfo[1].uwYStart += ((ubUI_RecFolderIdx / 5) * 220);
		tOSD_Img2(&tFolderSelOsdImgInfo[1], OSD_QUEUE);
		OSD_ImagePrintf(OSD_IMG_ROTATION_0, (72 + ((ubUI_RecFolderIdx % 5) * 180)), (225 + ((ubUI_RecFolderIdx / 5) * 220)),
						tUI_RecOsdImgInfo, OSD_UPDATE, tUI_RecFoldersInfo.tRecFolderInfo[ubUI_RecFolderIdx + ubRecSelIdx].FldName.chName);
	}

}
//------------------------------------------------------------------------------setting-----------------------------------------------------------------------------------
//------------------------------------------------------------------------------setting-----------------------------------------------------------------------------------
//------------------------------------------------------------------------------setting-----------------------------------------------------------------------------------

static void UI_CameraSubMenuExecute(void)
{
	OSD_IMG_INFO tCamsOsdImgInfo[8],tOsdImgInfo;

	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_CAMSCAM1NOR_ICON, 8, &tCamsOsdImgInfo[0]);
	//background
	tOSD_GetOsdImgInfor(1, OSD_IMG1, OSD1IMG_SUBMENU2, 1, &tOsdImgInfo);
	
	UI_PanelOff();
	OSD_EraserImg1(&tOsdImgInfo);
	//CAM1~CAM4
	for(uint8_t i = 0;i < 4;i++)
		tOSD_Img2(&tCamsOsdImgInfo[2*i], OSD_QUEUE);
	tOSD_Img2(&tCamsOsdImgInfo[1], OSD_QUEUE);
	
	tUI_State = UI_SUBSUBMENU_STATE;
	UI_EnterKey();	
}
//-----------------------------------------------------------------------------
static void UI_RecordSubMenuExecute(void)
{
	#define RECORE_WORD_NUMBER 5
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
									(tUI_CuSetting.tLanguage == LANGUAGE_FRENCH)?OSD2IMG_RECORD_TITLE_FR:OSD2IMG_RECORD_TITLE_CHN, RECORE_WORD_NUMBER, &tOsdImgInfo[0]);
	//文字
	for(uint8_t i = 0;i < RECORE_WORD_NUMBER;i++)
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
	//highLight record time 1 min
	if(tUI_CuSetting.RecInfo.tREC_Time == RECTIME_1MIN)
		tOSD_Img2(&tOsdImgInfo[7], OSD_QUEUE);
	else
		tOSD_Img2(&tOsdImgInfo[1], OSD_QUEUE);
	
	
	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_FORMATSD_NOR, 2, &tOsdImgInfo[0]);
	
	//format sd card
	tOSD_Img2(&tOsdImgInfo[0], OSD_QUEUE);
	//date&time
	//tOSD_Img2(&tOsdImgInfo[2], OSD_QUEUE);
	
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
	tUI_State = UI_SUBSUBMENU_STATE;

}
//-----------------------------------------------------------------------------
static void UI_SystemSubMenuExecute(void)
{
	OSD_IMG_INFO tMenuOsdImgInfo,tOsdImgInfo[10];

//	UI_UART2_PutChar(0XFF);
//	UI_UART2_PutChar(0XDD);
	
	tOSD_GetOsdImgInfor(1, OSD_IMG1, OSD1IMG_SUBMENU2, 1, &tMenuOsdImgInfo);
	tOSD_Img1(&tMenuOsdImgInfo, OSD_QUEUE);

	
	//文字
	tOSD_GetOsdImgInfor(1, OSD_IMG2, (tUI_CuSetting.tLanguage == LANGUAGE_ENGLISH)?OSD2IMG_SYSTEM_TITLE:
									(tUI_CuSetting.tLanguage == LANGUAGE_GERMAN)?OSD2IMG_SYSTEM_TITLE_GER:
									(tUI_CuSetting.tLanguage == LANGUAGE_FRENCH)?OSD2IMG_SYSTEM_TITLE_FR:OSD2IMG_SYSTEM_TITLE_CHN, SYSTEMITEM_MAX, &tOsdImgInfo[0]);

	for(uint8_t i = 0;i < 1;i ++)
		tOSD_Img2(&tOsdImgInfo[i], OSD_QUEUE);

	if(!ubUI_TouchPanelSts)
	{
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_RETURN_BLACK, 1, &tMenuOsdImgInfo);
		tOSD_Img2(&tMenuOsdImgInfo, OSD_QUEUE);
	}
	//滑块滑杆
	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_SYSTEM_DIMMER_SLIDER, 6, &tOsdImgInfo[0]);
	//绿色高亮
	tOSD_Img2(&tOsdImgInfo[4], OSD_QUEUE);		
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
	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_SYSTEM_AUTO_OFF_NOR, 6, &tOsdImgInfo[0]);

	if(tUI_CuSetting.ubAutoDimmer)
		tOSD_Img2(&tOsdImgInfo[2], OSD_QUEUE);
	else
		tOSD_Img2(&tOsdImgInfo[0], OSD_QUEUE);
	tOSD_Img2(&tOsdImgInfo[4], OSD_QUEUE);

	
	//menu lock
	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_SYSTEM_MENULOCK_OFF_NOR, 4, &tOsdImgInfo[0]);

	if(tUI_CuSetting.ubMenuLock)
		tOSD_Img2(&tOsdImgInfo[2], OSD_QUEUE);
	else
		tOSD_Img2(&tOsdImgInfo[0], OSD_QUEUE);

	//upgrade
	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_SYSTEM_UPGRADE_NOR, 1, &tOsdImgInfo[0]);
    tOSD_Img2(&tOsdImgInfo[0], OSD_QUEUE);
	
	//language
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

	//buzzer
	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_SYSTEM_BUZZER_OFF_NOR, 4, &tOsdImgInfo[0]);

	if(tUI_CuSetting.ubBuzzer)
		tOSD_Img2(&tOsdImgInfo[2], OSD_QUEUE);
	else
		tOSD_Img2(&tOsdImgInfo[0], OSD_QUEUE);

	// date & time
    tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_SYSTEM_DATE_TIME_NOR, 1, &tOsdImgInfo[0]);
	tOSD_Img2(&tOsdImgInfo[0], OSD_QUEUE);

	OSD_ImagePrintf(OSD_IMG_ROTATION_0, 450, 543, tUI_CharOsdImgInfo, OSD_QUEUE, systemAIVersion);
	OSD_ImagePrintf(OSD_IMG_ROTATION_0, 450, 570, tUI_CharOsdImgInfo, OSD_UPDATE, SOFEWARE_VERSION);
			
	memset(&tSystemSubSubMenuItem.tSubMenuInfo, 0, sizeof(UI_MenuItem_t));
	tUI_State = UI_SUBSUBMENU_STATE;

}
//-----------------------------------------------------------------------------
static void UI_DualSubMenuExecute(void)
{
	OSD_IMG_INFO tCamOsdImgInfo[16],tOsdImgInfo;
	APP_EventMsg_t tUI_SwitchBuMsg = {0};
	uint16_t uwLcd_HSize = uwLCD_GetLcdHoSize();

	UI_PanelOff();
	tOSD_GetOsdImgInfor(1, OSD_IMG1, OSD1IMG_SUBMENU2, 1, &tOsdImgInfo);
	OSD_EraserImg1(&tOsdImgInfo);
	//切换到dual画面
	UI_SwitchViewType(DUALVIEW_ITEM,TRUE);
	
	//line
	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_DESKTOP_LINE_DUAL, 1, &tCamOsdImgInfo[0]);
	tOSD_Img2(&tCamOsdImgInfo[0], OSD_QUEUE);
	
	
	for(uint8_t i = 0;i < 4;i++)
	{
		//left
		tOSD_GetOsdImgInfor(1, OSD_IMG2, (tUI_CuSetting.tLanguage != LANGUAGE_GERMAN)?OSD2IMG_DUAL_LEFT_CAM1NOR:OSD2IMG_DUAL_LEFT_CAM1NOR_GER, 16, &tCamOsdImgInfo[0]);
		if(tUI_CamStatus[i].tCamDispLocation_Dual == DISP_LEFT)
		{
			tUI_SwitchBuMsg.ubAPP_Message[2] = i;
			tOSD_Img2(&tCamOsdImgInfo[8 + 2*i], OSD_QUEUE);
		}
		else
		{
			tOSD_Img2(&tCamOsdImgInfo[2*i], OSD_QUEUE);
		}
		//right
		tOSD_GetOsdImgInfor(1, OSD_IMG2, (tUI_CuSetting.tLanguage != LANGUAGE_GERMAN)?OSD2IMG_DUAL_LEFT_CAM1NOR:OSD2IMG_DUAL_LEFT_CAM1NOR_GER, 16, &tCamOsdImgInfo[0]);
		if(tUI_CamStatus[i].tCamDispLocation_Dual == DISP_RIGHT)
		{
			tUI_SwitchBuMsg.ubAPP_Message[3] = i;
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
	tOSD_GetOsdImgInfor(1, OSD_IMG2, (tUI_CuSetting.tLanguage != LANGUAGE_GERMAN)?OSD2IMG_DUAL_LEFT_CAM1NOR:OSD2IMG_DUAL_LEFT_CAM1NOR_GER, 16, &tCamOsdImgInfo[0]);
	if(tUI_CamStatus[0].tCamDispLocation_Dual == DISP_LEFT)
		tOSD_Img2(&tCamOsdImgInfo[8 + 1], OSD_UPDATE);
	else
		tOSD_Img2(&tCamOsdImgInfo[1], OSD_UPDATE);
	
	//return
	if(ubUI_TouchPanelSts)
	{
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_RETURN_NOBACKGROUND_NOR, 1, &tOsdImgInfo);
		tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
	}
	
	memset(&tDualSubSubMenuItem.tSubMenuInfo, 0, sizeof(UI_MenuItem_t));
	tUI_State = UI_SUBSUBMENU_STATE;

}
//-----------------------------------------------------------------------------
static void UI_QuadSubMenuExecute(void)
{
	OSD_IMG_INFO tCamOsdImgInfo[16],tOsdImgInfo;
	uint16_t uwLcd_HSize = uwLCD_GetLcdHoSize();
	uint16_t uwLcd_VSize = uwLCD_GetLcdVoSize();

	UI_PanelOff();
	tOSD_GetOsdImgInfor(1, OSD_IMG1, OSD1IMG_SUBMENU2, 1, &tOsdImgInfo);
	OSD_EraserImg1(&tOsdImgInfo);
	
	//切换到quad画面
	UI_SwitchViewType(QUALVIEW_ITEM,TRUE);
	//line
	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_DESKTOP_LINE_QUAD_H, 2, &tCamOsdImgInfo[0]);
	tOSD_Img2(&tCamOsdImgInfo[0], OSD_QUEUE);
	tOSD_Img2(&tCamOsdImgInfo[1], OSD_QUEUE);
	
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
			tCamOsdImgInfo[8 + 2*i].uwYStart -= uwLcd_VSize/4;
			tCamOsdImgInfo[8 + 2*i].uwXStart += uwLcd_HSize/2;
			tOSD_Img2(&tCamOsdImgInfo[8 + 2*i], OSD_QUEUE);
		}
		else
		{
			tCamOsdImgInfo[2*i].uwYStart -= uwLcd_VSize/4;
			tCamOsdImgInfo[2*i].uwXStart += uwLcd_HSize/2;
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
	tOSD_GetOsdImgInfor(1, OSD_IMG2, (tUI_CuSetting.tLanguage != LANGUAGE_GERMAN)?OSD2IMG_DUAL_LEFT_CAM1NOR:OSD2IMG_DUAL_LEFT_CAM1NOR_GER, 16, &tCamOsdImgInfo[0]);
	//highLight
	if(tUI_CamStatus[0].tCamDispLocation_Quad == DISP_UPPER_LEFT)
	{
		tCamOsdImgInfo[8 + 1].uwYStart -= uwLcd_VSize/4;
		tOSD_Img2(&tCamOsdImgInfo[8 + 1], OSD_UPDATE);
	}
	else
	{
		tCamOsdImgInfo[1].uwYStart -= uwLcd_VSize/4;
		tOSD_Img2(&tCamOsdImgInfo[1], OSD_UPDATE);
	}

	//return
	if(ubUI_TouchPanelSts)
	{
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_RETURN_NOBACKGROUND_NOR, 1, &tOsdImgInfo);
		tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
	}

	memset(&tQuadSubSubMenuItem.tSubMenuInfo, 0, sizeof(UI_MenuItem_t));
	tUI_State = UI_SUBSUBMENU_STATE;
}
//-----------------------------------------------------------------------------
static void UI_TriggerSubMenuExecute(void)
{
	
	OSD_IMG_INFO tOsdImgInfo_Button[22],tOsdImgInfo_Word[9],tOsdImgInfo_2,tMenuOsdImgInfo;
	
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
//	tOSD_GetOsdImgInfor(1, OSD_IMG2, (tUI_CuSetting.tLanguage == LANGUAGE_ENGLISH)?OSD2IMG_TRIGGER_DISPLAY_SETUP_NOR:
//									(tUI_CuSetting.tLanguage == LANGUAGE_GERMAN)?OSD2IMG_TRIGGER_DISPLAY_SETUP_NOR_GER:
//									(tUI_CuSetting.tLanguage == LANGUAGE_FRENCH)?OSD2IMG_TRIGGER_DISPLAY_SETUP_NOR_FR:OSD2IMG_TRIGGER_DISPLAY_SETUP_NOR_CHN, 2, &tOsdImgInfo_Button[20]);
	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_TRIGGER_DISPLAY_SETUP_NOR,2, &tOsdImgInfo_Button[20]);


	for(uint8_t i = 0;i < 11;i++)
		tOSD_Img2(&tOsdImgInfo_Button[2*i], OSD_QUEUE);
	tOSD_Img2(&tOsdImgInfo_Button[1], OSD_QUEUE);
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
	UI_ShowButtonValueHighLight(tUI_CuSetting.TriggerDelay[0],
			tOsdImgInfo_Button[0].uwXStart + ((tUI_CuSetting.TriggerDelay[0] < 10)?40:30),tOsdImgInfo_Button[0].uwYStart + 15,OSD_UPDATE);
	
	tUI_State = UI_SUBSUBMENU_STATE;
}
//-----------------------------------------------------------------------------
static void UI_AutoScanSubMenuExecute(void)
{
	
	OSD_IMG_INFO tOsdImgInfo[24],tMenuOsdImgInfo;
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
			tOSD_Img2(&tOsdImgInfo[4*i + ((i == 0)?UI_ICON_HIGHLIGHT:UI_ICON_NORMAL)], ((i == 5)?OSD_UPDATE:OSD_QUEUE));
		else
			tOSD_Img2(&tOsdImgInfo[2 + 4*i + ((i == 0)?UI_ICON_HIGHLIGHT:UI_ICON_NORMAL)], ((i == 5)?OSD_UPDATE:OSD_QUEUE));
	}
	
	tUI_State = UI_SUBSUBMENU_STATE;
}	

//-----------------------------------------------------------------------------
static void UI_PowerOnSubMenuExecute(void)
{
	OSD_IMG_INFO tOsdImgInfo[2*POWERON_MAX],tMenuOsdImgInfo;
	
	tOSD_GetOsdImgInfor(1, OSD_IMG1, OSD1IMG_SUBMENU2, 1, &tMenuOsdImgInfo);
	tOSD_Img1(&tMenuOsdImgInfo, OSD_QUEUE);
	if(!ubUI_TouchPanelSts)
	{
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_RETURN_BLACK, 1, &tMenuOsdImgInfo);
		tOSD_Img2(&tMenuOsdImgInfo, OSD_QUEUE);
	}
	//title
	tOSD_GetOsdImgInfor(1, OSD_IMG2, (tUI_CuSetting.tLanguage == LANGUAGE_ENGLISH)?OSD2IMG_POWERON_TITLE:
									(tUI_CuSetting.tLanguage == LANGUAGE_GERMAN)?OSD2IMG_POWERON_TITLE_GER:
									(tUI_CuSetting.tLanguage == LANGUAGE_FRENCH)?OSD2IMG_POWERON_TITLE_FR:OSD2IMG_POWERON_TITLE_CHN, 1, &tOsdImgInfo[0]);

	tOSD_Img2(&tOsdImgInfo[0], OSD_QUEUE);
	
	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_POWERON_CAM1_NOR, 2*(POWERON_MAX - 1), &tOsdImgInfo[0]);
	for(uint8_t i = 0;i < (POWERON_MAX - 1);i++)
		tOSD_Img2(&tOsdImgInfo[2*i], (tUI_CuSetting.tPowerOnMode == POWERON_LAST)?OSD_UPDATE:OSD_QUEUE);
	
	if(tUI_CuSetting.tPowerOnMode != POWERON_LAST)
		tOSD_Img2(&tOsdImgInfo[2*tUI_CuSetting.tPowerOnMode + UI_ICON_HIGHLIGHT], OSD_UPDATE);
	tUI_State = UI_SUBSUBMENU_STATE;

	
	memset(&tPowerOnSubSubMenuItem.tSubMenuInfo, 0, sizeof(UI_MenuItem_t));
	ubUI_PowerOnSubSubStsUpdateFlag = FALSE;
}

//-----------------------------------------------------------------------------
static void UI_ParkingLineSubMenuExecute(void)
{
	
	OSD_IMG_INFO tOsdImgInfo[24],tMenuOsdImgInfo;
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
	//CALIBRATE
	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_PARKINGLINE_CAM1_CAL_NOR, 8, &tOsdImgInfo[0]);
	for(uint8_t i = 0;i < 4;i++)
		tOSD_Img2(&tOsdImgInfo[2*i], OSD_QUEUE);
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
	//high light
	tOSD_Img2(&tOsdImgInfo[2*tUI_CuSetting.tParkingLineEnable[0] + 1], OSD_UPDATE);
	tUI_State = UI_SUBSUBMENU_STATE;
	
}

//-----------------------------------------------------------------------------
static void UI_GuideLineSubMenuExecute(void)
{
	OSD_IMG_INFO tOsdImgInfo[24],tMenuOsdImgInfo;
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
			
	//CALIBRATE
	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_PARKINGLINE_CAM1_CAL_NOR, 8, &tOsdImgInfo[0]);
	for(uint8_t i = 0;i < 4;i++)
		tOSD_Img2(&tOsdImgInfo[2*i], OSD_QUEUE);
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
	//high light
	tOSD_Img2(&tOsdImgInfo[2*tUI_CuSetting.GuideLineEnable[0] + 1], OSD_UPDATE);
	tUI_State = UI_SUBSUBMENU_STATE;
	
}

//-----------------------------------------------------------------------------
static void UI_AISubMenuExecute(void)
{
	UI_AIEnterMenu();
}

//-----------------------------------------------------------------------------

static void UI_SettingSubMenuExecute(void)
{
	uint8_t ubSubMenuItemIdx = tUI_SubMenuItem[SETTING_ITEM].tSubMenuInfo.ubItemIdx;
	memset(&tSettingSubSubMenuItem.tSettingS[ubSubMenuItemIdx].tSubMenuInfo, 0, sizeof(UI_MenuItem_t));
	
	ubUI_SubSubStsUpdateFlag = FALSE;
	if(ubSubMenuItemIdx == CAMERASET_ITEM)
	{
		UI_CameraSubMenuExecute();
	}
	else if(ubSubMenuItemIdx == RECORDSET_ITEM)
	{
		UI_RecordSubMenuExecute();
	}
	else if(ubSubMenuItemIdx == SYSTEMSET_ITEM)
	{		
		UI_SystemSubMenuExecute();
	}
	else if(ubSubMenuItemIdx == DUALSET_ITEM)
	{
		UI_DualSubMenuExecute();
	}
	else if(ubSubMenuItemIdx == QUADSET_ITEM)
	{
		UI_QuadSubMenuExecute();
	}
	else if(ubSubMenuItemIdx == TRIGGERSET_ITEM)
	{
		UI_TriggerSubMenuExecute();
	}
	else if(ubSubMenuItemIdx == AUTOSCANSET_ITEM)
	{
		UI_AutoScanSubMenuExecute();
	}
	else if(ubSubMenuItemIdx == POWERONSET_ITEM)
	{
		UI_PowerOnSubMenuExecute();
	}
	else if(ubSubMenuItemIdx == PARKINGLINE_ITEM)
	{
		UI_ParkingLineSubMenuExecute();
	}
	else if(ubSubMenuItemIdx == GUIDELINE_ITEM)
	{
		UI_GuideLineSubMenuExecute();
	}
	else if(ubSubMenuItemIdx == AI_ITEM)
	{
		UI_AISubMenuExecute();
	}
	else
		return;
}
//-----------------------------------------------------------------------------
static void UI_SettingingDrawSubMenuItem(void)
{
	uint16_t uwSubMenuItemOsdImg[SETTINGITEM_MAX] = {
													OSD2IMG_CAMERA_SETTING_NOR_ICON, 
													OSD2IMG_RECORD_SETTING_NOR_ICON,
													OSD2IMG_SYSTEM_SETTING_NOR_ICON,
													OSD2IMG_DUAL_SETTING_NOR_ICON,
													OSD2IMG_QUAD_SETTING_NOR_ICON,
													OSD2IMG_TRIGGER_NOR_ICON,
													OSD2IMG_AUTOSCAN_NOR_ICON,
													OSD2IMG_POWERON_NOR_ICON,
													OSD2IMG_PARKINGLINE_NOR_ICON,
													OSD2IMG_GUIDELINE_NOR_ICON,
													OSD2IMG_AI_NOR_ICON,
													};
	uint8_t ubSubMenuItemPreIdx = tUI_SubMenuItem[SETTING_ITEM].tSubMenuInfo.ubItemPreIdx;
	uint8_t ubSubMenuItemIdx = tUI_SubMenuItem[SETTING_ITEM].tSubMenuInfo.ubItemIdx;
	UI_DrawHLandNormalIcon(uwSubMenuItemOsdImg[ubSubMenuItemPreIdx], (uwSubMenuItemOsdImg[ubSubMenuItemIdx] + UI_ICON_HIGHLIGHT));

}
//-----------------------------------------------------------------------------

void UI_SettingSubMenuPage(UI_ArrowKey_t tArrowKey)
{
	UI_MenuAct_t tMenuAct;
	
	if(tUI_State == UI_MAINMENU_STATE)
	{
		tUI_State = UI_SUBMENU_STATE;
		memset(&tUI_SubMenuItem[SETTING_ITEM].tSubMenuInfo, 0, sizeof(UI_MenuItem_t));
		UI_DrawSubMenuPage(SETTING_ITEM);
		return;
	}
	tMenuAct = UI_KeyEventMap2SubMenuInfo(&tArrowKey, &tUI_SubMenuItem[SETTING_ITEM]);
	switch(tMenuAct)
	{
		case DRAW_HIGHLIGHT_MENUICON:
		{
			UI_SettingingDrawSubMenuItem();
			break;
		}
		case DRAW_MENUPAGE:
		{
			UI_SettingSubMenuExecute();
			break;
		}
		case EXIT_MENUFUNC:
			break;
		default:
			break;
	}
}
//------------------------------------------------------------------------------power-----------------------------------------------------------------------------------
//------------------------------------------------------------------------------power-----------------------------------------------------------------------------------
//------------------------------------------------------------------------------power-----------------------------------------------------------------------------------
static void UI_PowerSubMenuExecute(void)
{
	if(tUI_SubMenuItem[POWER_ITEM].tSubMenuInfo.ubItemIdx == POWER_SCREENOFF)
	{
		UI_EnterStandby();
	}
	else
	{
		UI_EnterStandby();
		ubUI_CuPowerOffFlag = TRUE;
	}
}
//------------------------------------------------------------------------------
static void UI_PowerDrawSubMenuItem(void)
{
	uint16_t uwSubMenuItemOsdImg[2] = {
										OSD2IMG_POWER_SCREENOFF_NOR, 
										OSD2IMG_POWER_SHUTDOWN_NOR,
										};
	uint8_t ubSubMenuItemPreIdx = tUI_SubMenuItem[POWER_ITEM].tSubMenuInfo.ubItemPreIdx;
	uint8_t ubSubMenuItemIdx = tUI_SubMenuItem[POWER_ITEM].tSubMenuInfo.ubItemIdx;
	UI_DrawHLandNormalIcon(uwSubMenuItemOsdImg[ubSubMenuItemPreIdx], 
		(uwSubMenuItemOsdImg[ubSubMenuItemIdx] + UI_ICON_HIGHLIGHT));

}

//------------------------------------------------------------------------------
void UI_PowerSubMenuPage(UI_ArrowKey_t tArrowKey)
{
	UI_MenuAct_t tMenuAct;

	if(tUI_State == UI_MAINMENU_STATE)
	{
		tUI_State = UI_SUBMENU_STATE;
		memset(&tUI_SubMenuItem[POWER_ITEM].tSubMenuInfo, 0, sizeof(UI_MenuItem_t));
		UI_DrawSubMenuPage(POWER_ITEM);
		return;
	}
	tMenuAct = UI_KeyEventMap2SubMenuInfo(&tArrowKey, &tUI_SubMenuItem[POWER_ITEM]);
	switch(tMenuAct)
	{
		case DRAW_HIGHLIGHT_MENUICON:
		{
			UI_PowerDrawSubMenuItem();
			break;
		}
		case DRAW_MENUPAGE:
		{
			UI_PowerSubMenuExecute();
			break;
		}
		case EXIT_MENUFUNC:
			break;
		default:
			break;
	}
	
	
}


/********************************************************************TOUCH*********************************************************************************************/
/********************************************************************TOUCH*********************************************************************************************/
/********************************************************************TOUCH*********************************************************************************************/
void UI_SubTouchMenu(TOUCH_EVENT_t *Touch_Info)
{
	static UI_MenuTouchFuncPtr_t SubMenuFunc[MENUITEM_MAX] = 
	{
		UI_AdoSelSubTouchMenu,
		UI_CamSelSubTouchMenu,
		UI_PairingSubTouchMenu,
		NULL,
		UI_PlaybackSubTouchMenu,
		UI_SettingSubTouchMenu,
		UI_PowerSubTouchMenu,
	};
	if(SubMenuFunc[tUI_MenuItem.ubItemIdx].pvFuncPtr)
		SubMenuFunc[tUI_MenuItem.ubItemIdx].pvFuncPtr(Touch_Info);
}
//------------------------------------------------------------------------------adoSel-----------------------------------------------------------------------------------------
//------------------------------------------------------------------------------adoSel-----------------------------------------------------------------------------------------
//------------------------------------------------------------------------------adoSel-----------------------------------------------------------------------------------------
void UI_AdoSelSubTouchMenu(TOUCH_EVENT_t *Touch_Info)
{
	OSD_IMG_INFO tOsdImgInfo;
	uint16_t i = 0;
	tUI_SubMenuItem[ADO_MODE_ITEM].tSubMenuInfo.ubItemPreIdx = tUI_SubMenuItem[ADO_MODE_ITEM].tSubMenuInfo.ubItemIdx;

	uint8_t count;
	uint16_t j;
	if(SINGLE_VIEW == tCamViewSel.tCamViewType) {
		count = 2;
		j = 0;
	} else if(DUAL_VIEW == tCamViewSel.tCamViewType) {
		count = 3;
		j = 1;
	} else {
		count = 5;
		j = 2;
	}
	
	switch(Touch_Info->Gesture)
	{
		case TOUCH_PRESS:	
			for(i = 0; i < count;i++)//判断具体点击在哪个图标上
			{
				//读取当前操作界面的全部图标信息
				tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_SELADOCAM1_NOR_ICON + 2*i, 1, &tOsdImgInfo);	

				if(Touch_Info->startX > ado_button_x[j][i] 
					&& Touch_Info->startX < ado_button_x[j][i] + tOsdImgInfo.uwHSize
					&& Touch_Info->startY > tOsdImgInfo.uwYStart 
					&& Touch_Info->startY < tOsdImgInfo.uwYStart + tOsdImgInfo.uwVSize)
				{
					if(i == (count-1)) {
						tUI_SubMenuItem[ADO_MODE_ITEM].tSubMenuInfo.ubItemIdx = 4;
						break;
					}
					
					if(SINGLE_VIEW == tCamViewSel.tCamViewType || DUAL_VIEW == tCamViewSel.tCamViewType) {
						tUI_SubMenuItem[ADO_MODE_ITEM].tSubMenuInfo.ubItemIdx = tCamViewSel.tCamViewPool[i];
					} else {
						tUI_SubMenuItem[ADO_MODE_ITEM].tSubMenuInfo.ubItemIdx = i;
					}
					break;
				}
			}
			if(i < count)
			{
				if(tUI_SubMenuItem[ADO_MODE_ITEM].tSubMenuInfo.ubItemPreIdx != 
					tUI_SubMenuItem[ADO_MODE_ITEM].tSubMenuInfo.ubItemIdx)
				{
					UI_AdoSelDrawSubMenuItem(); 			
				}
				UI_EnterKey();	
			}
			else
			{
				UI_MenuKey();
				UI_MenuKey();
                UI_LeftArrowKey();
			}	

		break;
		case TOUCH_TURNDOWN:
			break;

		default:
			return;

	}
}


//------------------------------------------------------------------------------camSel-----------------------------------------------------------------------------------------
//------------------------------------------------------------------------------camSel-----------------------------------------------------------------------------------------
//------------------------------------------------------------------------------camSel-----------------------------------------------------------------------------------------
void UI_CamSelSubTouchMenu(TOUCH_EVENT_t *Touch_Info)
{
	OSD_IMG_INFO tOsdImgInfo;
	uint16_t i = 0;
	tUI_SubMenuItem[VDO_MODE_ITEM].tSubMenuInfo.ubItemPreIdx = tUI_SubMenuItem[VDO_MODE_ITEM].tSubMenuInfo.ubItemIdx;
	switch(Touch_Info->Gesture)
	{
		case TOUCH_PRESS:	
				
			for(i = 0; i < tUI_SubMenuItem[VDO_MODE_ITEM].ubItemCount;i++)//判断具体点击在哪个图标上
			{
				//读取当前操作界面的全部图标信息
				tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_SELCAM1NOR_ICON + 2*i, 1, &tOsdImgInfo);	

				if(Touch_Info->startX > tOsdImgInfo.uwXStart 
					&& Touch_Info->startX < tOsdImgInfo.uwXStart + tOsdImgInfo.uwHSize
					&& Touch_Info->startY > tOsdImgInfo.uwYStart 
					&& Touch_Info->startY < tOsdImgInfo.uwYStart + tOsdImgInfo.uwVSize)
				{
					tUI_SubMenuItem[VDO_MODE_ITEM].tSubMenuInfo.ubItemIdx = i;
					break;
				}
			}
			if(i < tUI_SubMenuItem[VDO_MODE_ITEM].ubItemCount)
			{
				UI_CamSelDrawSubMenuItem(); 
				osDelay(100);
				UI_EnterKey();	
			}
			else
			{
				UI_MenuKey();
			}

		break;
		case TOUCH_TURNDOWN:
			break;

		default:
			return;

	}

}
//------------------------------------------------------------------------------pairing-----------------------------------------------------------------------------------------
//------------------------------------------------------------------------------pairing-----------------------------------------------------------------------------------------
//------------------------------------------------------------------------------pairing-----------------------------------------------------------------------------------------
void UI_PairingSubTouchMenu(TOUCH_EVENT_t *Touch_Info)
{
	OSD_IMG_INFO tOsdImgInfo;
	uint16_t i = 0;
	tUI_SubMenuItem[PAIRING_ITEM].tSubMenuInfo.ubItemPreIdx = tUI_SubMenuItem[PAIRING_ITEM].tSubMenuInfo.ubItemIdx;
	switch(Touch_Info->Gesture)
	{
		case TOUCH_PRESS:	
				
			for(i = 0; i < tUI_SubMenuItem[PAIRING_ITEM].ubItemCount;i++)//判断具体点击在哪个图标上
			{
				tOSD_GetOsdImgInfor(1, OSD_IMG2, ((tUI_CuSetting.tLanguage != LANGUAGE_GERMAN)?OSD2IMG_PAIRCAM1NOR_ICON:OSD2IMG_PAIRCAM1NOR_ICON_GER) + 2*(i%4), 1, &tOsdImgInfo);
				tOsdImgInfo.uwYStart = PAIRING_Y + PAIRING_Y_STEP*(i/4);
				if(Touch_Info->startX > tOsdImgInfo.uwXStart 
					&& Touch_Info->startX < tOsdImgInfo.uwXStart + tOsdImgInfo.uwHSize
					&& Touch_Info->startY > tOsdImgInfo.uwYStart 
					&& Touch_Info->startY < tOsdImgInfo.uwYStart + tOsdImgInfo.uwVSize)
				{
					tUI_SubMenuItem[PAIRING_ITEM].tSubMenuInfo.ubItemIdx = i;
					break;
				}
			}
			
			if(i < tUI_SubMenuItem[PAIRING_ITEM].ubItemCount)	
			{
				if(tUI_SubMenuItem[PAIRING_ITEM].tSubMenuInfo.ubItemPreIdx != 
					tUI_SubMenuItem[PAIRING_ITEM].tSubMenuInfo.ubItemIdx)
				{
					UI_PairingDrawSubMenuItem();
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

//------------------------------------------------------------------------------playback-----------------------------------------------------------------------------------------
//------------------------------------------------------------------------------playback-----------------------------------------------------------------------------------------
//------------------------------------------------------------------------------playback-----------------------------------------------------------------------------------------
/* ICON
* 0 1 2 3 4
* 5 6 7 8 9
*/
#define PLAYBACK_SUB_MAX_ICON 10
void UI_PlaybackSubTouchMenu(TOUCH_EVENT_t *Touch_Info)
{
	uint16_t uwHSize[PLAYBACK_SUB_MAX_ICON],uwVSize[PLAYBACK_SUB_MAX_ICON],uwXStart[PLAYBACK_SUB_MAX_ICON],uwYStart[PLAYBACK_SUB_MAX_ICON];
	uint8_t ubUI_RecFolderIdx = 0, ubUI_PrevRecFolderIdx = 0, ubRecSelIdx;
	OSD_IMG_INFO tFolderSelOsdImgInfo[2],tOsdImgInfo;
	uint8_t ubFldStartIdx = 0, ubFldEndIdx = 0;
	uint16_t i = 0;	
	ubRecSelIdx = tUI_RecFoldersInfo.uwRecFolderSelIdx;
	ubUI_RecFolderIdx = (ubRecSelIdx % REC_FOLDER_LIST_MAXNUM);//当前选中的图标在界面中排第几位
	ubUI_PrevRecFolderIdx = ubUI_RecFolderIdx;	
	
	switch(Touch_Info->Gesture)
	{
		case TOUCH_PRESS:	
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_REC_FOLDERCLOSE_ICON, 2, &tFolderSelOsdImgInfo[0]);
			
			for(i = 0;i < PLAYBACK_SUB_MAX_ICON;i ++)
			{
				//计算该界面中每个图标的位置
				uwHSize[i] = tFolderSelOsdImgInfo[0].uwHSize;
				uwVSize[i] = tFolderSelOsdImgInfo[0].uwVSize;
				uwXStart[i] = tFolderSelOsdImgInfo[0].uwXStart + (i % 5) * 180;
				uwYStart[i] = tFolderSelOsdImgInfo[0].uwYStart + (i / 5) * 220;

				//判断点击在哪个图标上
				if(Touch_Info->startX > uwXStart[i] && Touch_Info->startX < uwXStart[i] + uwHSize[i]
					&& Touch_Info->startY > uwYStart[i] && Touch_Info->startY < uwYStart[i] + uwVSize[i])
				{
					break;
				}
			}
			if(i < PLAYBACK_SUB_MAX_ICON)//点击在文件夹上
			{
				if((ubRecSelIdx + i - ubUI_RecFolderIdx) >= tUI_RecFoldersInfo.uwTotalRecFolderNum)//当前点击的位置没有文件夹
					return;
				ubUI_RecFolderIdx = i;
				tUI_RecFoldersInfo.uwRecFolderSelIdx += i - ubUI_PrevRecFolderIdx;

				ubRecSelIdx = (tUI_RecFoldersInfo.uwRecFolderSelIdx / REC_FOLDER_LIST_MAXNUM) * REC_FOLDER_LIST_MAXNUM;//该界面第一个文件夹的序号
				UI_SetRecImgColor(UI_RECIMG_COLOR1);
				tFolderSelOsdImgInfo[0].uwXStart += ((ubUI_PrevRecFolderIdx % 5) * 180);
				tFolderSelOsdImgInfo[0].uwYStart += ((ubUI_PrevRecFolderIdx / 5) * 220);
				tOSD_Img2(&tFolderSelOsdImgInfo[0], OSD_QUEUE);
				OSD_ImagePrintf(OSD_IMG_ROTATION_0, (72 + ((ubUI_PrevRecFolderIdx % 5) * 180)), (225 + ((ubUI_PrevRecFolderIdx / 5) * 220)),
								tUI_RecOsdImgInfo, OSD_QUEUE, tUI_RecFoldersInfo.tRecFolderInfo[ubUI_PrevRecFolderIdx + ubRecSelIdx].FldName.chName);
				UI_SetRecImgColor(UI_RECIMG_COLOR1);
				tFolderSelOsdImgInfo[1].uwXStart += ((ubUI_RecFolderIdx % 5) * 180);
				tFolderSelOsdImgInfo[1].uwYStart += ((ubUI_RecFolderIdx / 5) * 220);
				tOSD_Img2(&tFolderSelOsdImgInfo[1], OSD_QUEUE);
				OSD_ImagePrintf(OSD_IMG_ROTATION_0, (72 + ((ubUI_RecFolderIdx % 5) * 180)), (225 + ((ubUI_RecFolderIdx / 5) * 220)),
								tUI_RecOsdImgInfo, OSD_UPDATE, tUI_RecFoldersInfo.tRecFolderInfo[ubUI_RecFolderIdx + ubRecSelIdx].FldName.chName);
				TIMER_Delay_ms(30);
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
					TIMER_Delay_ms(30);
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
						ubFldStartIdx = (tUI_RecFoldersInfo.uwRecFolderSelIdx / REC_FOLDER_LIST_MAXNUM) * REC_FOLDER_LIST_MAXNUM;//该界面第一个文件夹的序号 
						if(i == 0)//backward
						{
							//如果该文件夹序号大于0就说明上一页还有内容
							if(ubFldStartIdx > 0)
							{
								tUI_RecFoldersInfo.uwRecFolderSelIdx = ubFldStartIdx - 1;
								ubFldStartIdx = (tUI_RecFoldersInfo.uwRecFolderSelIdx / REC_FOLDER_LIST_MAXNUM) * REC_FOLDER_LIST_MAXNUM;
								ubFldEndIdx = ubFldStartIdx + REC_FOLDER_LIST_MAXNUM;
							}
							else
								return;

						}
						else//forward
						{
							//满足该条件表示下一页还有文件夹
							if(ubFldStartIdx + REC_FOLDER_LIST_MAXNUM < tUI_RecFoldersInfo.uwTotalRecFolderNum)
							{
								tUI_RecFoldersInfo.uwRecFolderSelIdx = ubFldStartIdx + REC_FOLDER_LIST_MAXNUM;
								ubFldStartIdx = (tUI_RecFoldersInfo.uwRecFolderSelIdx / REC_FOLDER_LIST_MAXNUM) * REC_FOLDER_LIST_MAXNUM;
								ubFldEndIdx = tUI_RecFoldersInfo.uwTotalRecFolderNum - tUI_RecFoldersInfo.uwRecFolderSelIdx;
								ubFldEndIdx = (ubFldEndIdx < REC_FOLDER_LIST_MAXNUM)?tUI_RecFoldersInfo.uwTotalRecFolderNum:(ubFldStartIdx + REC_FOLDER_LIST_MAXNUM);
							}
							else 
								return;

						}
						tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
						tOSD_GetOsdImgInfor(1, OSD_IMG1, OSD1IMG_SUBMENU2, 1, &tOsdImgInfo);
						tOSD_Img1(&tOsdImgInfo, OSD_QUEUE);
						
						UI_ListDCIMFolderInfo(ubFldStartIdx, ubFldEndIdx, OSD_UPDATE);

						break;
					}	
				}
				
				
				
			}

			break;
		case TOUCH_TURNLEFT:
			break;
		case TOUCH_TURNRIGHT:
			
			break;
		case TOUCH_TURNDOWN:
			break;
		
		default:

			return;
	}

}
//------------------------------------------------------------------------------setting-----------------------------------------------------------------------------------------
//------------------------------------------------------------------------------setting-----------------------------------------------------------------------------------------
//------------------------------------------------------------------------------setting-----------------------------------------------------------------------------------------


void UI_SettingSubTouchMenu(TOUCH_EVENT_t *Touch_Info)
{
	OSD_IMG_INFO tOsdImgInfo;
	uint16_t i = 0;
	tUI_SubMenuItem[SETTING_ITEM].tSubMenuInfo.ubItemPreIdx = tUI_SubMenuItem[SETTING_ITEM].tSubMenuInfo.ubItemIdx;
	switch(Touch_Info->Gesture)
	{
		case TOUCH_PRESS:
			
			for(i = 0; i < tUI_SubMenuItem[SETTING_ITEM].ubItemCount;i++)//判断具体点击在哪个图标上
			{
				//读取当前操作界面的全部图标信息
				tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_CAMERA_SETTING_NOR_ICON + 2*i, 1, &tOsdImgInfo);	
				if(Touch_Info->startX > tOsdImgInfo.uwXStart 
					&& Touch_Info->startX < tOsdImgInfo.uwXStart + tOsdImgInfo.uwHSize
					&& Touch_Info->startY > tOsdImgInfo.uwYStart 
					&& Touch_Info->startY < tOsdImgInfo.uwYStart + tOsdImgInfo.uwVSize)
				{
					tUI_SubMenuItem[SETTING_ITEM].tSubMenuInfo.ubItemIdx = i;
					break;
				}
			}
			
			if(i < tUI_SubMenuItem[SETTING_ITEM].ubItemCount)
			{
				if(tUI_SubMenuItem[SETTING_ITEM].tSubMenuInfo.ubItemPreIdx != 
					tUI_SubMenuItem[SETTING_ITEM].tSubMenuInfo.ubItemIdx)
				{
					UI_SettingingDrawSubMenuItem();				
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
//------------------------------------------------------------------------------power-----------------------------------------------------------------------------------------
//------------------------------------------------------------------------------power-----------------------------------------------------------------------------------------
//------------------------------------------------------------------------------power-----------------------------------------------------------------------------------------
void UI_PowerSubTouchMenu(TOUCH_EVENT_t *Touch_Info)
{
	OSD_IMG_INFO tOsdImgInfo;
	uint16_t i = 0;
	tUI_SubMenuItem[POWER_ITEM].tSubMenuInfo.ubItemPreIdx = tUI_SubMenuItem[POWER_ITEM].tSubMenuInfo.ubItemIdx;
	switch(Touch_Info->Gesture)
	{
		case TOUCH_PRESS:	
				
			for(i = 0; i < tUI_SubMenuItem[POWER_ITEM].ubItemCount;i++)//判断具体点击在哪个图标上
			{
				//读取当前操作界面的全部图标信息
				tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_POWER_SCREENOFF_NOR + 2*i, 1, &tOsdImgInfo);	

				if(Touch_Info->startX > tOsdImgInfo.uwXStart 
					&& Touch_Info->startX < tOsdImgInfo.uwXStart + tOsdImgInfo.uwHSize
					&& Touch_Info->startY > tOsdImgInfo.uwYStart 
					&& Touch_Info->startY < tOsdImgInfo.uwYStart + tOsdImgInfo.uwVSize)
				{
					tUI_SubMenuItem[POWER_ITEM].tSubMenuInfo.ubItemIdx = i;
					break;
				}
			}
			if(i < tUI_SubMenuItem[POWER_ITEM].ubItemCount)
			{
				if(tUI_SubMenuItem[POWER_ITEM].tSubMenuInfo.ubItemPreIdx != 
					tUI_SubMenuItem[POWER_ITEM].tSubMenuInfo.ubItemIdx)
				{
					UI_PowerDrawSubMenuItem(); 			
					osDelay(100);
				}
				UI_EnterKey();	
			}
			else
			{
				UI_MenuKey();
			}	
		break;
		case TOUCH_TURNDOWN:
			break;

		default:
			return;

	}
}

#endif

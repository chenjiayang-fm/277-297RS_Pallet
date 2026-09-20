/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		APP_UVCUACCMD.c
	\brief		UVC UAC Command Application function
	\author		Scott
	\version	1.0
	\date		2020/12/07
	\copyright	Copyright(C) 2020 SONiX Technology Co., Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------
#include <stdio.h>
#include <string.h>
#include "APP_UVCUACCMD.h"
#include "UI_BUCCU.h"

#ifdef BUC_CU
void APP_UvcCtCmd(uint8_t ubCs, uint8_t ubLen, uint8_t * buf)
{
	UI_CUReqCmd_t tCamSetColorCmd;
	printd(DBG_InfoLvl, "APP_UvcCtCmd %x %x %x %x %x %x\n",ubCs,ubLen,buf[0],buf[1],buf[2],buf[3] );
	tCamSetColorCmd.tDS_CamNum 				= CAM1;
	tCamSetColorCmd.ubCmd[UI_TWC_TYPE]		= UI_SETTING;
	tCamSetColorCmd.ubCmd[UI_SETTING_ITEM]  = UI_IMGPROC_SETTING;
	tCamSetColorCmd.ubCmd[UI_SETTING_DATA]   = UI_CT_SETTING;
	tCamSetColorCmd.ubCmd[UI_SETTING_DATA+1] = ubCs;
	tCamSetColorCmd.ubCmd[UI_SETTING_DATA+2] = ubLen;
	tCamSetColorCmd.ubCmd[UI_SETTING_DATA+3] = buf[0];
	tCamSetColorCmd.ubCmd[UI_SETTING_DATA+4] = buf[1];
	tCamSetColorCmd.ubCmd[UI_SETTING_DATA+5] = buf[2];
	tCamSetColorCmd.ubCmd[UI_SETTING_DATA+6] = buf[3];
	tCamSetColorCmd.ubCmd[UI_SETTING_DATA+7] = buf[4];
	tCamSetColorCmd.ubCmd[UI_SETTING_DATA+8] = buf[5];
	tCamSetColorCmd.ubCmd[UI_SETTING_DATA+9] = buf[6];
	tCamSetColorCmd.ubCmd[UI_SETTING_DATA+10]= buf[7];
	tCamSetColorCmd.ubCmd_Len				 = ubLen+5;	
	if(UI_SendRequestToCAM(osThreadGetId(), &tCamSetColorCmd) != rUI_SUCCESS)
		printd(DBG_ErrorLvl, "APP_CT %d \n", tCamSetColorCmd.tDS_CamNum);	
}

void APP_UvcPuCmd(uint8_t ubCs, uint8_t ubLen, uint8_t * buf)
{
	UI_CUReqCmd_t tCamSetColorCmd;
	printd(DBG_InfoLvl, "APP_UvcPuCmd %x %x %x %x %x %x\n",ubCs,ubLen,buf[0],buf[1],buf[2],buf[3] );
	tCamSetColorCmd.tDS_CamNum 				= CAM1;
	tCamSetColorCmd.ubCmd[UI_TWC_TYPE]		= UI_SETTING;
	tCamSetColorCmd.ubCmd[UI_SETTING_ITEM]  = UI_IMGPROC_SETTING;
	tCamSetColorCmd.ubCmd[UI_SETTING_DATA]   = UI_PU_SETTING;
	tCamSetColorCmd.ubCmd[UI_SETTING_DATA+1] = ubCs;
	tCamSetColorCmd.ubCmd[UI_SETTING_DATA+2] = ubLen;
	tCamSetColorCmd.ubCmd[UI_SETTING_DATA+3] = buf[0];
	tCamSetColorCmd.ubCmd[UI_SETTING_DATA+4] = buf[1];
	tCamSetColorCmd.ubCmd[UI_SETTING_DATA+5] = buf[2];
	tCamSetColorCmd.ubCmd[UI_SETTING_DATA+6] = buf[3];
	tCamSetColorCmd.ubCmd_Len				 = ubLen+5;	
	if(UI_SendRequestToCAM(osThreadGetId(), &tCamSetColorCmd) != rUI_SUCCESS)
		printd(DBG_ErrorLvl, "APP_PU %d \n", tCamSetColorCmd.tDS_CamNum);
}

void APP_UacCmd(uint8_t ubCs, uint8_t ubLen, uint8_t * buf)
{
	UI_CUReqCmd_t tCamAdoSet;
	printd(DBG_InfoLvl, "APP_UacCmd %x %x %x %x %x %x\n",ubCs,ubLen,buf[0],buf[1],buf[2],buf[3] );
	tCamAdoSet.tDS_CamNum 				= CAM1;
	tCamAdoSet.ubCmd[UI_TWC_TYPE]		= UI_SETTING;
	tCamAdoSet.ubCmd[UI_SETTING_ITEM]   = UI_ADO_SETTING;
	tCamAdoSet.ubCmd[UI_SETTING_DATA]   = ubCs;
	tCamAdoSet.ubCmd[UI_SETTING_DATA+1] = ubLen;
	tCamAdoSet.ubCmd[UI_SETTING_DATA+2] = buf[0];
	tCamAdoSet.ubCmd[UI_SETTING_DATA+3] = buf[1];
	tCamAdoSet.ubCmd[UI_SETTING_DATA+4] = buf[2];
	tCamAdoSet.ubCmd[UI_SETTING_DATA+5] = buf[3];
	tCamAdoSet.ubCmd_Len				= ubLen+4;	
	if(UI_SendRequestToCAM(osThreadGetId(), &tCamAdoSet) != rUI_SUCCESS)
		printd(DBG_ErrorLvl, "APP_UAC %d \n", tCamAdoSet.tDS_CamNum);
}

#define SNX_MJPG_HEADER_LEN             			1024

struct uvc_image_info UvcMjpgFhd = {
        USB_UVC_VS_FORMAT_MJPEG,                // type
        FHD_WIDTH,                              // width
        FHD_HEIGHT,                             // height
        (FHD_WIDTH * FHD_HEIGHT) + SNX_MJPG_HEADER_LEN, // imagesize
        5,                                      // fps_num
        { 30, 20, 15, 10, 5 },                  // fps
};	

struct uvc_image_info UvcH264Fhd = {
        USB_UVC_VS_FORMAT_FRAME_BASED ,             // type
        VGA_WIDTH,                              // width
        VGA_HEIGHT,                             // height
        VGA_WIDTH * VGA_HEIGHT,                 // imagesize
        5,                                      // fps_num
        { 30, 20, 15, 10, 5 },                  // fps
};

struct uvc_image_info UvcYuvVga = {
        USB_UVC_VS_FORMAT_UNCOMPRESSED ,                                // type
        VGA_WIDTH,                                                      // width
        VGA_HEIGHT,                                                     // height
        (VGA_WIDTH*VGA_HEIGHT*2),                                       // imagesize
        3,                                                              // fps_num
        { 15, 10, 5 },              			                        // fps
};

struct uvc_image_info UvcH264Fhd1 = {
        USB_UVC_VS_FORMAT_FRAME_BASED ,             // type
        1280,                              // width
        720,                             // height
        1280 * 720,                 // imagesize
        5,                                      // fps_num
        { 30, 20, 15, 10, 5 },                  // fps
};

void APP_UvcImageInf(uint8_t *list)
{
	struct uvc_image_info *uvc_img_info_list[USBD_IMG_INF_MAX];

	uvc_img_info_list[0] = &UvcMjpgFhd;
	uvc_img_info_list[1] = &UvcYuvVga;
	uvc_img_info_list[2] = &UvcH264Fhd;
	uvc_img_info_list[3] = &UvcH264Fhd1;
	
	for(uint8_t ubIdx = 4; ubIdx < USBD_IMG_INF_MAX; ubIdx++)
		uvc_img_info_list[ubIdx] = NULL;

	memcpy(list,uvc_img_info_list,sizeof(uvc_img_info_list));		
}
#endif	



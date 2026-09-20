
#include "UART4AI.h"
#include "OSD.h"

osMessageQId tUART4AI_RecvQueue;
extern osSemaphoreId tUI_AIBoxCleanSem;
extern uint8_t ubUI_CuFWU_1126Flag;

//#define DATA_LENGTH_MAX 20
//
//typedef struct
//{
//	short x1; //���Ͻ�x����ֵ
//	short y1; //���Ͻ�y����ֵ
//	short x2; //���½�x����ֵ
//	short y2; //���½�y����ֵ
//} Algo_Pos;
//
//typedef struct
//{
//	uint32_t chn;
//	uint32_t cnt;
//	Algo_Pos pos[DATA_LENGTH_MAX];
//} Algo_Result;


typedef enum
{
	UART4AI_STATE_CHECK_0XFF,
	UART4AI_STATE_CHECK_0XABC,
	UART4AI_STATE_GET_TARGET_CHANNAL_AA,
	UART4AI_STATE_GET_TARGET_CHANNAL_BB,
	UART4AI_STATE_GET_TARGET_P_OR_C,
	UART4AI_STATE_GET_TARGET_NUMBER,
	UART4AI_STATE_AI_DATA,
	UART4AI_STATE_CRC1,
	UART4AI_STATE_CRC2,
	UART4AI_STATE_GET_RSENDI_OR_BSDRANGE,
	UART4AI_STATE_GET_UPGRADE,
	UART4AI_STATE_GET_VERSON,
} eHandleUART4AIState;

static const unsigned short crc16Table[256] = {
0x0000, 0xc0c1, 0xc181, 0x0140, 0xc301, 0x03c0, 0x0280, 0xc241,
0xc601, 0x06c0, 0x0780, 0xc741, 0x0500, 0xc5c1, 0xc481, 0x0440,
0xcc01, 0x0cc0, 0x0d80, 0xcd41, 0x0f00, 0xcfc1, 0xce81, 0x0e40,
0x0a00, 0xcac1, 0xcb81, 0x0b40, 0xc901, 0x09c0, 0x0880, 0xc841,
0xd801, 0x18c0, 0x1980, 0xd941, 0x1b00, 0xdbc1, 0xda81, 0x1a40,
0x1e00, 0xdec1, 0xdf81, 0x1f40, 0xdd01, 0x1dc0, 0x1c80, 0xdc41,
0x1400, 0xd4c1, 0xd581, 0x1540, 0xd701, 0x17c0, 0x1680, 0xd641,
0xd201, 0x12c0, 0x1380, 0xd341, 0x1100, 0xd1c1, 0xd081, 0x1040,
0xf001, 0x30c0, 0x3180, 0xf141, 0x3300, 0xf3c1, 0xf281, 0x3240,
0x3600, 0xf6c1, 0xf781, 0x3740, 0xf501, 0x35c0, 0x3480, 0xf441,
0x3c00, 0xfcc1, 0xfd81, 0x3d40, 0xff01, 0x3fc0, 0x3e80, 0xfe41,
0xfa01, 0x3ac0, 0x3b80, 0xfb41, 0x3900, 0xf9c1, 0xf881, 0x3840,
0x2800, 0xe8c1, 0xe981, 0x2940, 0xeb01, 0x2bc0, 0x2a80, 0xea41,
0xee01, 0x2ec0, 0x2f80, 0xef41, 0x2d00, 0xedc1, 0xec81, 0x2c40,
0xe401, 0x24c0, 0x2580, 0xe541, 0x2700, 0xe7c1, 0xe681, 0x2640,
0x2200, 0xe2c1, 0xe381, 0x2340, 0xe101, 0x21c0, 0x2080, 0xe041,
0xa001, 0x60c0, 0x6180, 0xa141, 0x6300, 0xa3c1, 0xa281, 0x6240,
0x6600, 0xa6c1, 0xa781, 0x6740, 0xa501, 0x65c0, 0x6480, 0xa441,
0x6c00, 0xacc1, 0xad81, 0x6d40, 0xaf01, 0x6fc0, 0x6e80, 0xae41,
0xaa01, 0x6ac0, 0x6b80, 0xab41, 0x6900, 0xa9c1, 0xa881, 0x6840,
0x7800, 0xb8c1, 0xb981, 0x7940, 0xbb01, 0x7bc0, 0x7a80, 0xba41,
0xbe01, 0x7ec0, 0x7f80, 0xbf41, 0x7d00, 0xbdc1, 0xbc81, 0x7c40,
0xb401, 0x74c0, 0x7580, 0xb541, 0x7700, 0xb7c1, 0xb681, 0x7640,
0x7200, 0xb2c1, 0xb381, 0x7340, 0xb101, 0x71c0, 0x7080, 0xb041,
0x5000, 0x90c1, 0x9181, 0x5140, 0x9301, 0x53c0, 0x5280, 0x9241,
0x9601, 0x56c0, 0x5780, 0x9741, 0x5500, 0x95c1, 0x9481, 0x5440,
0x9c01, 0x5cc0, 0x5d80, 0x9d41, 0x5f00, 0x9fc1, 0x9e81, 0x5e40,
0x5a00, 0x9ac1, 0x9b81, 0x5b40, 0x9901, 0x59c0, 0x5880, 0x9841,
0x8801, 0x48c0, 0x4980, 0x8941, 0x4b00, 0x8bc1, 0x8a81, 0x4a40,
0x4e00, 0x8ec1, 0x8f81, 0x4f40, 0x8d01, 0x4dc0, 0x4c80, 0x8c41,
0x4400, 0x84c1, 0x8581, 0x4540, 0x8701, 0x47c0, 0x4680, 0x8641,
0x8201, 0x42c0, 0x4380, 0x8341, 0x4100, 0x81c1, 0x8081, 0x4040,
};

void UART2_PutChar(char ch)
{
	while(!UART2->TX_RDY);
	UART2->RS_DATA = ch;
	//ch = ch;
}

uint16_t crc16_Gen(char *pu8Data, int u32Len)
{
	uint16_t uwCrc = 0, i;
    uint8_t ubByte;
    if (NULL == pu8Data || 0 == u32Len)
    {
        return 0;
    }

    for (i = 0; i < u32Len; i++)
    {
        ubByte = *pu8Data;
        uwCrc = (uwCrc >> 8) ^ crc16Table[(uwCrc ^ ubByte) & 0xff]; 
        pu8Data++;
    }

    return (((uwCrc >> 8) | (uwCrc << 8)) & 0xFFFF);
}



extern uint8_t ul_drawboxflag;
extern uint8_t  star_countdrawflag;
uint8_t ResendI_Flag = 0;
uint8_t ResendI_CHN = 0xff;

void handleUart4ai(char data[], int length)
{
	
	Algo_Result curRet = {0};
	uint32_t targetNum = 0, targetChn,targetP_OR_C,index,targetFrame;
	osMessageQId *pUI_EventQH;
	uint32_t ClearBOX_chn;
	static uint8_t dropFram = 0;	
    UI_Event_t tDrawBoxEvent;

	targetChn = (uint32_t)data[0];
	targetP_OR_C = (uint32_t)data[1];
	targetNum = (uint32_t)data[2];
	
	if(targetChn > 4)
	{
		Drawing_BoxFlag[targetChn] = 0;
		printf("chn recive error!!!!!!!!!!!\n");
		return;
	}
//	if(tUI_CuSetting.ubIsEnableBSD[targetChn] == 0)
//	{
//		return;
//	}
	if(tUI_CuSetting.ubDetectPeopleFlag[targetChn] == 0 && tUI_CuSetting.ubDetectCarFlag[targetChn] == 0 )
	{
		Drawing_BoxFlag[targetChn] = 0;
		return;
	}
	if(targetNum >= 5 && (DeskTopShowView == DUALVIEW_ITEM||DeskTopShowView == QUALVIEW_ITEM))
	{
		targetNum = 5;
	}
	if(targetNum >= 10)
	{
		targetNum = 10;
	}
		
	//printf("***********************targetFrame : %d ********\n",targetFrame);
	if (targetNum > 0) // Ŀ��������0 ˵���л������� ��Ҫ���� ��Ĩȥ��һ�λ��Ŀ�
	{
		curRet.chn = targetChn;
		curRet.P_OR_C = targetP_OR_C;
		curRet.cnt = targetNum;
        if (targetNum > DATA_LENGTH_MAX)
        {
        	targetNum = 10;
			curRet.cnt = targetNum;
        }		
		for (index = 0; index < targetNum; index++)
		{
			curRet.pos[index].x1  = (((short)data[3+index*9]<<0)&0x00FF);
			curRet.pos[index].x1 |= (((short)data[4+index*9]<<8)&0xFF00);

			curRet.pos[index].y1  = (((short)data[5+index*9]<<0)&0x00FF);
			curRet.pos[index].y1 |= (((short)data[6+index*9]<<8)&0xFF00);

			curRet.pos[index].x2  = (((short)data[7+index*9]<<0)&0x00FF);
			curRet.pos[index].x2 |= (((short)data[8+index*9]<<8)&0xFF00);

			curRet.pos[index].y2  = (((short)data[9+index*9]<<0)&0x00FF);
			curRet.pos[index].y2 |= (((short)data[10+index*9]<<8)&0xFF00);

			curRet.pos[index].alarm_type = (short)data[11+index*9];
		}
		pUI_EventQH 	  	 	 = pUI_GetAIBOXQueueHandle(curRet.chn);
//		if(targetFrame == 30)
//		{
////			ul_drawboxflag = 1;
////			star_countdrawflag = 1;
////			ul_drawboxflag = 0;
////			printf("stop count!!!!!!!!!!!!!\n");
//			//printf("curRet.chn is %x!!!!!!!!!!\n",curRet.chn);
////			curRet.chn = 0xf0|curRet.chn ;
//			//printf("curRet.chn is %x!!!!!!!!!!\n",curRet.chn);
//		}
		if(pUI_EventQH == 0)
			return;
		if(dropFram == 0)
		{
			//dropFram ++;
			if(osMessagePut(*pUI_EventQH, &curRet, 0) != osOK)
			{
				printf("Draw box is full !!!!!!!!!!!\n");
				return;
			}

		}
		else
		{
			dropFram = 0;
			return;

		}
	}
    else // �յ����ݳ���С��1 ˵����Ҫ����ǰ���Ŀ�Ĩȥ
    {
		ClearBOX_chn = targetChn;
		pUI_EventQH 	  	 	 = pUI_GetAIBOX_clearQueueHandle();
		osMessagePut(*pUI_EventQH, &ClearBOX_chn, 0);
    }
}

int checkCRCReceiveData(char data[], int length, uint16_t recvCRC)
{
    uint16_t calcCRC = crc16_Gen(data, length);

//	char ch = (char)(calcCRC&0x00FF);
//	//UART2_PutChar(ch);
//
//	ch = (char)((calcCRC&0xFF00) >> 8);
//	//UART2_PutChar(ch);

	if (recvCRC == calcCRC)
	{
		return 1;
	}

	return 0;
}

void ResendI_handle(char CAM_CH)
{
	ResendI_CHN = CAM_CH;
	if(ResendI_Flag == 0)
		ResendI_Flag = 1;
	printf("1126 request resendI !!!!!!!!!!!\n");
	return;
}
uint8_t receivedData[3+DATA_LENGTH_MAX*9] = {0};
uint8_t receivedVersion[VERSION_LEN]={0};
uint8_t systemAIVersion[VERSION_LEN]={0};

static eHandleUART4AIState state = UART4AI_STATE_CHECK_0XFF;
extern uint8_t KNL_UsbdFwuFg;
extern osMessageQId KNL_UsbdFwuQueue;
uint8_t handleEptTarget[4] = {0};
uint8_t Reboot_Flag = 0;
uint8_t md5x01_getflag = 0,sdkx04getflag = 0;
//extern UI_ParkinglinePoint_t tUI_ParkinglinePoint[4];
extern uint8_t Playwav_Flag;
extern uint32_t Playwav_Count;

static void UART4AI_RecvThread(void const *argument)
{
	uint8_t ch,MD5_ErrorCount_Flag;
	static int targetNum = 0, indexRecv = 0;
	uint16_t crcRecv;
	static uint8_t ErrorCount= 0;
	KNL_USBDFWU_PROCESS tProc;
	OSD_IMG_INFO tOsdImgInfo;

	while(1)
	{
		if (osMessageGet(tUART4AI_RecvQueue, (void*)&ch, osWaitForever))
		{
//			printf("tUART4AI_RecvQueue ch = 0x%x\n", ch);
			switch (state)
			{
				case UART4AI_STATE_CHECK_0XFF:
					if (ch != 0xFF)
					{	
						break;
					}
					else
					{
						if(MD5_ErrorCount_Flag)
							ErrorCount ++;
						if(ErrorCount > 5000)
						{
							printf("update fail!!!!!!!!!!\n");
							MD5_ErrorCount_Flag = 0;
							ErrorCount = 0;
							KNL_UsbdFwuFg = 0;
						}
						state = UART4AI_STATE_CHECK_0XABC;
					}
					break;
				case UART4AI_STATE_CHECK_0XABC:
					//state = UART4AI_STATE_CHECK_0XFF;
					if (ch == 0xAA)
					{
						//printf("**********FFAA***********\n");
						state = UART4AI_STATE_GET_TARGET_CHANNAL_AA;
					}
					else if(ch == 0xBB)
					{
						//printf("**********FFBB***********\n");
						state = UART4AI_STATE_GET_TARGET_CHANNAL_BB;
					}
					else if(ch == 0xCC)
					{
						//printf("**********FFCC***********\n");
						state = UART4AI_STATE_GET_UPGRADE;
					}
					else if(ch == 0xDD)
					{
						//printf("**********FFDD***********\n");
						indexRecv = 0;
						state = UART4AI_STATE_GET_VERSON;
					}
					else
					{
						state = UART4AI_STATE_CHECK_0XFF;
						break;
					}
					break;
#if 1 // 包头为FFBB的响应代码
/***************************ffbb******************************************/
				case UART4AI_STATE_GET_TARGET_CHANNAL_BB:
						receivedData[0] = ch;
						//if(ch == CAM1)
							//printf("ch 1:");
						if ((ch > CAM4) || (ch < CAM1))
						{
							state = UART4AI_STATE_CHECK_0XFF;
						}
						state = UART4AI_STATE_GET_RSENDI_OR_BSDRANGE;
						break;
			
					//FFBB0x（x是通道号）添加判断需要重发i帧的通道。
/***************************RSENDI_OR_BSDRANGE*******************************/
				case UART4AI_STATE_GET_RSENDI_OR_BSDRANGE:
					receivedData[1] = ch;
					if(receivedData[1] == 0)
					{				
						//添加重发I帧的操作
						ResendI_handle(receivedData[0]);
						state = UART4AI_STATE_CHECK_0XFF;
					}
					else if(receivedData[1] == 1)
					{
						//添加检测区域发送操作
						switch(receivedData[0])
						{
							case 0:
								UI_SendBSDRangeTo1126(tUI_CuSetting.tUI_AIDetectlinePoint[0],receivedData[0]);
								break;
							case 1:
								UI_SendBSDRangeTo1126(tUI_CuSetting.tUI_AIDetectlinePoint[1],receivedData[0]);
								break;
							case 2:
								UI_SendBSDRangeTo1126(tUI_CuSetting.tUI_AIDetectlinePoint[2],receivedData[0]);
								break;
							case 3:
								UI_SendBSDRangeTo1126(tUI_CuSetting.tUI_AIDetectlinePoint[3],receivedData[0]);
								break;
						}
						state = UART4AI_STATE_CHECK_0XFF;
					}
					break;
/***************************RSENDI_OR_BSDRANGE***********************************/

#endif

#if 1 // 包头为FFCC响应代码
/***************************UPGRADE******************************************/
				case UART4AI_STATE_GET_UPGRADE:
					//发送md5码
					switch(ch)
					{
						case 0x00: //请求MD5
							printf("return md5 ack1111111!!!!!!!!\n");
							UART2_PutChar(0xFF);
							UART2_PutChar(0xCC);
							UART2_PutChar(0x00);
							KNL_UsbdFwuFg = 1;//关闭264码流传输
							md5x01_getflag = 0;
							sdkx04getflag = 0;
							MD5_ErrorCount_Flag = 1;
							printf("return md5 ack0000000000!!!!!!!!\n");
							state = UART4AI_STATE_CHECK_0XFF;
							break;
						case 0x01: //发送Md5
							ErrorCount = 0;
							printf("send md5 !!!!!!!!\n");		
							tProc.ubCmd = KNL_USBD_FWU_MDA;		
							if(md5x01_getflag == 0)
								osMessagePut(KNL_UsbdFwuQueue,&tProc,0);
							md5x01_getflag = 1;
							state = UART4AI_STATE_CHECK_0XFF;
							//发送Md5文件
							break;
						case 0x04: //接收完MD5
							ErrorCount = 0;
							Reboot_Flag = 1;
							printf("send upgrate !!!!!!!!\n");
							tProc.ubCmd = KNL_USBD_FWU_SDK;
							if(sdkx04getflag == 0)
							{
								osMessagePut(KNL_UsbdFwuQueue,&tProc,0);
								ubUI_CuFWU_1126Flag = 1;//关闭UI事件
							}
							sdkx04getflag = 1;
							state = UART4AI_STATE_CHECK_0XFF;	
							break;
						case 0x05: //接收完MD5
							printf("upgrate error!!!!!!!!\n");
							if(Reboot_Flag)
							{
								tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_FWUFAILED_ICON, 1, &tOsdImgInfo);
								tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
							}
							ubUI_CuFWU_1126Flag = 0;//打开UI事件
							MD5_ErrorCount_Flag = 0;
							ErrorCount = 0;
							KNL_UsbdFwuFg = 0;
							//OSD_ImagePrintf(OSD_IMG_ROTATION_0, 420, 310, tUI_CharOsdImgInfo, OSD_QUEUE, "upgrate Error!!");
							//osDelay(300);
							state = UART4AI_STATE_CHECK_0XFF;						
							break;
						case 0x06: //接收完MD5
							break;
						case 0x07: //接收完MD5
							break;
						case 0x98: //接受失败，sd卡没文件或者没插卡或者读取文件失败导致失败
							printf("no upgrate !!!!!!!!\n");
							if(Reboot_Flag)
							{
								tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_FWUFAILED_ICON, 1, &tOsdImgInfo);
								tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
							}
							ubUI_CuFWU_1126Flag = 0;//打开UI事件
							MD5_ErrorCount_Flag = 0;
							ErrorCount = 0;
							KNL_UsbdFwuFg = 0;
							//OSD_ImagePrintf(OSD_IMG_ROTATION_0, 420, 310, tUI_CharOsdImgInfo, OSD_QUEUE, "upgrate Error!!");
							//osDelay(300);
							state = UART4AI_STATE_CHECK_0XFF;
							break;					
						case 0x99: //接收成功(不管是否升级失败)，打开算法
							ubUI_CuFWU_1126Flag = 0;
							//OSD_ImagePrintf(OSD_IMG_ROTATION_0, 410, 240, tUI_CharOsdImgInfo, OSD_UPDATE, "OK!Wait Reboot!");
							//printf("upgrate ok!!!!!!!!\n");
							UART2_PutChar(0xFF);
							UART2_PutChar(0xCC);
							UART2_PutChar(0x99);
							MD5_ErrorCount_Flag = 0;
							ErrorCount = 0;
							KNL_UsbdFwuFg = 0;
							//osDelay(300);
							state = UART4AI_STATE_CHECK_0XFF;
							if(Reboot_Flag)
							{
								SYS_Reboot();
							}
							break;

					}
					break;
					
/***************************UPGRADE******************************************/
#endif

#if 1 // 包头为FFAA的响应代码
				case UART4AI_STATE_GET_TARGET_CHANNAL_AA:
					receivedData[0] = ch;
					//if(ch == CAM1)
						//printf("ch 1:");
					if ((ch > CAM4) || (ch < CAM1))
					{
						state = UART4AI_STATE_CHECK_0XFF;
					}
					state = UART4AI_STATE_GET_TARGET_P_OR_C;
					break;
/***************************person_or_car***********************************/
				case UART4AI_STATE_GET_TARGET_P_OR_C:
					receivedData[1] = ch;
				//	printf("receivedData is :%d\n",receivedData[1]);
					if ((ch > 3) || (ch < 0))//1:仅人 2：仅车 3：人车 
					{
						state = UART4AI_STATE_CHECK_0XFF;
					}
					state = UART4AI_STATE_GET_TARGET_NUMBER;
					break;
/***************************person_or_car***********************************/					
				case UART4AI_STATE_GET_TARGET_NUMBER:
					receivedData[2] = ch;
					//if(receivedData[0] == CAM1)
						//printf("target is : %d",ch);
					targetNum = (int)receivedData[2];
					indexRecv = 3;
					if (targetNum == 0)
                    {
                    	if (handleEptTarget[receivedData[0]] == 0)
                    	{
                    		//printf("into handleUart4ai :ch:%d,targetNum:%d!!!!!!!!!!!\n",receivedData[0],receivedData[2]);
                    		handleUart4ai(receivedData, indexRecv);	
							handleEptTarget[receivedData[0]] = 1;
						}
                    	state = UART4AI_STATE_CHECK_0XFF;
						break;
                    }
					else if (targetNum > DATA_LENGTH_MAX)
					{
						targetNum = 0;
						receivedData[2] = DATA_LENGTH_MAX;
						state = UART4AI_STATE_CHECK_0XFF;
						handleUart4ai(receivedData, indexRecv);
						printf("UART4AI_STATE_GET_TARGET_NUMBER targetNum > DATA_LENGTH_MAX\n");
						break;
					}
					handleEptTarget[receivedData[0]] = 0;
					state = UART4AI_STATE_AI_DATA;
					break;
				case UART4AI_STATE_AI_DATA:
					receivedData[indexRecv] = ch;
					indexRecv++;
					if (indexRecv == targetNum*9 + 3)
					{
						state = UART4AI_STATE_CRC1;
					}
					break;
				#if 0
				case UART4AI_STATE_CRC1:
					handleUart4ai(receivedData, indexRecv);
					state = UART4AI_STATE_CHECK_0XFF;
					break;
				#endif
				case UART4AI_STATE_CRC1:
					state = UART4AI_STATE_CRC2;
					crcRecv = (((uint16_t)ch<<0)&0x00FF);
					break;
				case UART4AI_STATE_CRC2:

					crcRecv |= (((uint16_t)ch<<8)&0xFF00);
					if (checkCRCReceiveData(receivedData, indexRecv, crcRecv))
					{
					
						if(Playwav_Flag == 0 && tUI_CuSetting.ubIsEnableBSDALARM[receivedData[0]] == 1 &&(tUI_CuSetting.ubDetectPeopleFlag[receivedData[0]] == 1 ||tUI_CuSetting.ubDetectCarFlag[receivedData[0]] == 1 ) )//&& tUI_CuSetting.ubIsEnableBSD[receivedData[0]] == 1)
						{
							Playwav_Flag = 1;
							Playwav_Count = 0;
							ADO_WavPlay(0);
						//	printf("play once wav!!!\n");
						}
						if(MenuOnFlag == TRUE)
						{
							;//printf("menu on quit draw box!!!!!!!!\n");
							
						}
						else
						{
							handleUart4ai(receivedData, indexRecv);		
						}
							
					}
					state = UART4AI_STATE_CHECK_0XFF;
					break;
#endif

#if 1 // 包头为FFDD的响应代码
					case UART4AI_STATE_GET_VERSON:
						if(indexRecv < VERSION_LEN)
						{
							receivedVersion[indexRecv] = ch;
							printf("data %d is %c",indexRecv,ch);
							indexRecv++;
						}
						else
						{
							printf("Verson get!!\n");
							indexRecv = 0;
//							OSD_ImagePrintf(OSD_IMG_ROTATION_0, 450, 543, tUI_CharOsdImgInfo, OSD_QUEUE, receivedVersion);
							snprintf(systemAIVersion, sizeof(systemAIVersion), "%s%s", "AI02", &receivedVersion[4]);
							state = UART4AI_STATE_CHECK_0XFF;
						}
						break;

#endif
			}
		}
		//osDelay(2);
	}
}

void UART4AI_Init(void)
{
//	GLB->PADIO49 = 2; //UART2_TX
//	GLB->PADIO50 = 2; //UART2_RX
//	ubPwrUartFlag = TRUE;
//	//延迟2s开启串口
//	ubPwrUartCount = 2000;
//	printf("time reset 2000 UART4AI_Init!!!!!!!");

	osMessageQDef(tUART4AI_RevQueue, 1024, sizeof(char));
	tUART4AI_RecvQueue = osMessageCreate(osMessageQ(tUART4AI_RevQueue), NULL);

	osThreadDef(uart4aiThread, UART4AI_RecvThread, osPriorityNormal, 1, 8192);
	osThreadCreate(osThread(uart4aiThread), NULL);
}

//uint8_t count = 0;
//uint8_t str[255] = {0};
void UART4AI_Recv(char ch)
{
	
//	str[count] = ch;
//	count++;
//	if(count == 20)
//	{
//		while(count != 0)
//		{
//			UART2_PutChar(str[20 - count]);
//			count--;
//		}
//		
//	}

	
	if((osMessagePut(tUART4AI_RecvQueue, (void*) &ch, 0)) != osOK)
	{
		printf("UART4AI Q full\r\n");
	}
}




#include "Trigger.h"

static UI_Event_t Trigger_Event;

TRIGGER_EVENT_t Trigger_View;



//------------------------------------------------------------------------------

static void TriggerCheck_Thread(void const *argument)
{
	uint8_t TriggerCheckQue[5];//存优先级0~4对应的tx序号,
	uint8_t TriggerSta[5];	//触发线状态
	uint8_t TriggerDisplay[5];
	uint16_t i;
	
	while(1)
	{
		osSemaphoreWait(osUI_CuTriggerCtr, osWaitForever);
		if(MenuOnFlag == FALSE && ubUI_CuPowerOffFlag == FALSE && !ubUI_CuPowerDiscFlag)//菜单开启时不响应触发，进入睡眠模式也不响应触发
		{
			#if 0
				printf("uwSADC_GetReport(SADC_CH1) = %d,uwSADC_GetReport(SADC_CH2) = %d\n",uwSADC_GetReport(SADC_CH1),uwSADC_GetReport(SADC_CH2));
			#endif
			//优先级0~4对应的tx序号存在TriggerCheckQue[i]中
			for(i = 0;i < 5;i++)
				TriggerCheckQue[tUI_CuSetting.TriggerPriority[i]] = i;
			
			TriggerSta[0] = TRIGGER_L;
			TriggerSta[1] = TRIGGER_R;
			TriggerSta[2] = TRIGGER_F;
			TriggerSta[3] = TRIGGER_B;
			TriggerSta[4] = TRIGGER_S;

			TriggerDisplay[0] = CAM1;
			TriggerDisplay[1] = CAM2;
			TriggerDisplay[2] = CAM3;
			TriggerDisplay[3] = CAM4;
			TriggerDisplay[4] = tUI_CuSetting.TriggerSplitView;
			
			for(i = 0;i < 5;i++)
			{
				
				if(TriggerSta[TriggerCheckQue[i]])//有触发
				{	
					if(TriggerSource != TriggerCheckQue[i])//触发源不一样,就执行画面切换
					{
						TriggerSource = TriggerCheckQue[i];
						if(TriggerLock == FALSE)
						{
							TriggerBackUp_View = DeskTopShowView;//保存触发前的画面
							TriggerLock = TRUE;
							if(ubUI_CuStandbyFlag)//待机触发
							{
								osSemaphoreWait(osUI_PowerLightCtr, osWaitForever);
								POWER_LED_IO = 0;
								osSemaphoreRelease(osUI_PowerLightCtr);
							}
						}
						printf("*************osMessagePut Trigger event tUI_ViewSel = %d!!\n", Trigger_View.tUI_ViewSel);
						Trigger_View.tUI_ViewSel = TriggerDisplay[TriggerCheckQue[i]];
						Trigger_Event.pvEvent = (void *)(&Trigger_View);
						Trigger_Event.tEventType = TRIGGER_EVENT;
						 if(osMessagePut(UI_EventQueue, &Trigger_Event, 0) != osOK)
								printd(DBG_ErrorLvl, "Trigger Q Full\n");
					
					}
					ulTriggerOverCount = tUI_CuSetting.TriggerDelay[TriggerCheckQue[i]] * 1000;
					break;
				}
			}

		}
		osSemaphoreRelease(osUI_CuTriggerCtr);
		osDelay(50);
	}

}
//-----------------------------------------------------------------------
void Trigger_init(void)
{
	//初始化两个AD
	SADC_SetPinMode(SADC_CH2, SADC_PIN_ANALOG_INPUT_MODE);
	SADC_SetPinMode(SADC_CH1, SADC_PIN_ANALOG_INPUT_MODE);
	SADC_Enable();
	osThreadDef(TriggerCheck_Thread, TriggerCheck_Thread, osPriorityBelowNormal, 1, 1024);
	if(osThreadCreate(osThread(TriggerCheck_Thread), NULL) == NULL)
	{
			printd(DBG_ErrorLvl, "Create TouchCheck_Thread fail!\n");
			while(1);
	}	
	printf("Trigger_init\n");
}



#include "TIMER.h"
#include "GPIO.h"
#include "stdio.h"
#include "IR.h"
#include "KEY.h"
#include "Buzzer.h"
#include "UI_BUCCU[WSVGA].h"



static uint32_t cn;
static uint32_t IRcode;
static uint32_t customcode;			//红外码中的用户码	
static uint8_t datacode;	//数据码
static uint8_t Rdatacode;	//数据码的反码
static uint32_t time_us,IR_Key_Repeat;	
KEY_Event_t tIRKEY_Event;
KEY_Event_t tIRKEY_Event4RepeatDown;
KEY_Event_t tIRKEY_Event4RepeatUp;
static uint8_t Power_longpressFlag = 0;

uint32_t IR_Key_Tick,DataRight;		//收到键值或者重复码后就给该变量赋值，过一段时间该变量就会减为0，如果为0就表示当前遥控的按键已经松开
uint8_t SendIrCodeFlag;//该变量是为标记哪个界面可以使用连发

static void IR_1MSTrigger(void)
{
	static int cn = 0;
	cn++;
	if(cn >17)
	{
		cn = 0;
		if(IR_Key_Tick >0)
			IR_Key_Tick --;
		if(IR_Key_Tick == 0 && DataRight)//松手
		{
			if(datacode == REMO_POWER && Power_longpressFlag == 1)
			{
				Power_longpressFlag = 0;
				printf("release long press\n");
				DataRight = 0;
				tIRKEY_Event.ubKeyAction = KEY_CNT_ACT;
				tIRKEY_Event.ubKeyID = datacode;
				tIRKEY_Event.uwKeyCnt = 20;
				KEY_QueueSend(IRKEY, &tIRKEY_Event);
			}
			else
			{
				printf("release\n");
				DataRight = 0;
				tIRKEY_Event.ubKeyAction = KEY_UP_ACT;
				tIRKEY_Event.ubKeyID = datacode;
				tIRKEY_Event.uwKeyCnt = 0;
				KEY_QueueSend(IRKEY, &tIRKEY_Event);
			}

		}
	}
}
//配置一个1ms的中断
static void IR_1MSCounterInit(void)
{
	TIMER_SETUP_t TmSetup;

	TmSetup.tCLK 		= TIMER_CLK_EXTCLK;
	TmSetup.ulTmLoad 	= 10000;
	TmSetup.ulTmCounter = TmSetup.ulTmLoad;
	TmSetup.ulTmMatch1 	= TmSetup.ulTmLoad + 1;
	TmSetup.ulTmMatch2 	= TmSetup.ulTmLoad + 1;
	TmSetup.tOF 		= TIMER_OF_ENABLE;
	TmSetup.tDIR 		= TIMER_DOWN_CNT;
	TmSetup.tEM 		= TIMER_CB;
	TmSetup.pvEvent 	= IR_1MSTrigger;
	TIMER_Start(TIMER1_3, TmSetup);
}


//time unit 1us
static void IR_IntHandler(void)
{
	if(GPIO->GPIO_INTR_FLAG0)
	{
		//printf("have press ir remote!!!!!!\n");
		//关掉画框
		GPIO->CLR_GPIO_INTR0 = 1;
		INTC_IrqClear(INTC_GPIO_IRQ);
		time_us = TIMER_GetValue_us(TIMER1_2);
		TIMER_SetValue_us(TIMER1_2,0);
		//收到引导码13.5ms
		if(time_us > 12500 && time_us < 14500)
		{	
			cn = 1;
			IRcode = 0;
		}
		if(cn <34)
		{
			if(time_us > 1000 && time_us < 1300) //0	560 + 560 uS
			{
				IRcode = IRcode|0x00000000;
				if(cn < 33)
					IRcode = IRcode >> 1;
			}		  
			else if(time_us > 2100 && time_us < 2400) //1	 560 + 1680 uS
			{
				IRcode = IRcode|0x80000000;
				if(cn < 33) 
					IRcode = IRcode >> 1;
			}

		}
		else if(time_us > 10500 && time_us < 12500)  //遇到重复码，递增  9ms + 2.5ms
		{
			 //正确接收到引导码 用户码 和数据码后才是重复码。
			if(DataRight)
			{

				IR_Key_Repeat ++;
				IR_Key_Tick = 8;
				if(SendIrCodeFlag && (datacode == REMO_LEFT || datacode == REMO_RIGHT) && IR_Key_Repeat > 2)
				{
					//通过发KEY_DOWN_ACT和KEY_UP_ACT来模拟一次按压
					tIRKEY_Event4RepeatDown.ubKeyAction = KEY_DOWN_ACT;
					tIRKEY_Event4RepeatDown.ubKeyID = datacode;
					tIRKEY_Event4RepeatDown.uwKeyCnt = 0;					
					KEY_QueueSend(IRKEY, &tIRKEY_Event4RepeatDown);
					
					tIRKEY_Event4RepeatUp.ubKeyAction = KEY_UP_ACT;
					tIRKEY_Event4RepeatUp.ubKeyID = datacode;
					tIRKEY_Event4RepeatUp.uwKeyCnt = 0;
					KEY_QueueSend(IRKEY, &tIRKEY_Event4RepeatUp);
				}
				else if(datacode == REMO_POWER && IR_Key_Repeat == 20)
				{
					IR_Key_Tick = 0;
					IR_Key_Repeat = 0;
					Power_longpressFlag = 1;
					printf("11111111111111111111\n");
				}
			}
		}

		if(cn < 37)
			cn++; 
		if(cn == 34) 	 //如果读完了32位
		{

			customcode = IRcode & 0xffff;    //取红外码中的用户码	 
			datacode = (IRcode >> 16) & 0xff;	//取数据码
			Rdatacode = (IRcode >> 24) & 0xff; 	//取数据码的反码

			if((customcode == 0xFF80) && ((unsigned char)(~Rdatacode) == datacode)) //不加(unsigned char)会不行!!
			{ 
				uint8_t buzzerflag = 1;
				if (IR_Key_Tick != 0)
				{
					buzzerflag = 0;
				}
				DataRight = 1; /*KeyValue = datacode;*/
				IR_Key_Tick = 8;//如果该值减为0就代表松手了
				IR_Key_Repeat = 0;	
				tIRKEY_Event.ubKeyAction = KEY_DOWN_ACT;
				tIRKEY_Event.ubKeyID = datacode;
				tIRKEY_Event.uwKeyCnt = 0;

//				switch(datacode)
//				{
//					case 0x02:
//						datacode = REMO_POWER;
//						break;
//				}
//
//				tIRKEY_Event.ubKeyID = datacode;
//				if(datacode == 0x02)//鏄犲皠power閿綅鐨勯敭鍊?
//				{
//					tIRKEY_Event.ubKeyID = REMO_POWER;
//				}
				switch(datacode)//5 key 
				{
					case 0x02: datacode = REMO_POWER; break; //PWR
					case 0x05: datacode = REMO_LEFT;  break; //LEFT
					case 0x03: datacode = REMO_RIGHT; break; //RIGHT
					case 0x01:  
						if (tUI_State == UI_DISPLAY_STATE)
							datacode = REMO_SEL;		  
						else
							datacode = REMO_OK;//ENTER	
						break; 	
					case 0x04: datacode = REMO_MENU;  break; //MENU & EXIT
					default: break;
				}

				printf("code = 0x%x\n",datacode);
				tIRKEY_Event.ubKeyID = datacode;
				if((!ubUI_CuStandbyFlag) && buzzerflag)
				{
					printf("-------------IR_Key_Tick = %d----------------\n", IR_Key_Tick);
					BUZ_Call_DI(150);
				}
				KEY_QueueSend(IRKEY, &tIRKEY_Event);
				
				
			}	
			else 
			{ 
				//printf("customcode = %x\t datacode = %x\t Rdatacode = %x\n",customcode,datacode,Rdatacode);
				DataRight = 0; 
				IRcode = 0; 
			}
		}





	}
}

void IR_init(void)
{
	GLB->PADIO14 = 0;
	GPIO->GPIO_OE0 = 0;
	GPIO_Intr_Setup(GPIO_INTR_SEL_0, GPIO_INTR_EDGE_TRG, GPIO_INTR_FALLING_EDGE, IR_IntHandler);
	IR_1MSCounterInit();
	printf("IR_init\n");
}



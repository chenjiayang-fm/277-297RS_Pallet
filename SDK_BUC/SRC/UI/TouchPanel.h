#ifndef _TOUCHPANEL_H
#define _TOUCHPANEL_H
#include "_510PF.h"
#include "UI.h"

typedef enum 
{
	TOUCH_NONE = 0,
	TOUCH_PRESS,
	TOUCH_RELEASE,
	TOUCH_TURNLEFT,
	TOUCH_TURNRIGHT,
	TOUCH_TURNUP,
	TOUCH_TURNDOWN,
	TOUCH_PRESSDOWN
}TouchGesture_t;


typedef struct TOUCH_INF_S
{
	uint16_t startX;
	uint16_t startY;
	uint16_t endX;
	uint16_t endY;
	
	//´Ó´¥ÃþÆÁ¶ÁÈ¡µÄÊý¾Ý
	uint16_t readX[2];
	uint16_t readY[2];
	TouchGesture_t Gesture;   
	TouchGesture_t oldGesture; 
	int8_t PressLastingTime;
	int8_t ReleaseLastingTime;
	uint8_t pressPoints;    
	uint8_t oldpressPoints; 
}TOUCH_EVENT_t;


#define TOUCHPANEL_RST_OUTPUT 		PWM->PWM_EN5
									

#define TOUCHPANEL_INT_OUTPUT  		GPIO->GPIO_O12 
#define TOUCHPANEL_INT_OUTPUT_EN  	GPIO->GPIO_OE12
#define TOUCHPANEL_INT_INPUT  		GPIO->GPIO_I12 

#define TOUCHPANEL_SLAVEADDR 		0x5d

//#define VERSION          0xC4 // 10å¯?

//0-Æß´çÆÁ 1-10´çÆÁ
#if HDW297RS_000000
#define TEN_SCREEN 
#define VERSION          0xC4 // 10å¯?
#elif HDW277RS_000000
#define SEVEN_SCREEN 
#define VERSION          0xA0// 7å¯?
#endif

void TouchPanel_init(void);

extern osMessageQId UI_EventQueue;
extern uint8_t ubUI_TouchPanelSts;




#endif







/*!
	\file		RN6752.c
	\brief		RN6752 header
	\author		BoCun
	\version	3.2
	\date		2020/07/23
	\copyright	Copyright(C) 2020 SONiX Technology Co.,Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------
#ifndef _TP9950_H_
#define _TP9950_H_
#include "_510PF.h"
#include "bsp_config.h"

//==============================================================================
// DEFINITION
//==============================================================================
// Sensor RN6752
// SLAVE_ID is 0x30 [1:7] + [0] r/w
// I2C slave ID can be programmed as 58/59 5A/5B for write and read.
#define SEN_SLAVE_ADDR           (0x44)

/* PINMUX */
//#define	TP9950_RESET_PADIO		GLB->PADIO15
//#define	TP9950_RESET_OUT_EN		GPIO->GPIO_OE14
//#define	TP9950_RESET_OUT		GPIO->GPIO_O1

// system control registers
/* ID */
#define TP9950_CHIP_ID_HIGH_ADDR   (0xFE)
#define TP9950_CHIP_ID_LOW_ADDR    (0xFF)
#define TP9950_CHIP_ID             (0x2850)

enum
{
    TP9950_25fps = 0,
    TP9950_30fps = 1,
};

enum
{
    TP9950_ValidVideo = 0,
	TP9950_NoVideo = 1,
};


enum
{
    TP9950_NTSC_PAL = 0,
    TP9950_720P = 1,
    TP9950_1080P = 2,
};


enum
{
    TP9950_AHD = 0,
    TP9950_TVI = 1,
    TP9950_CVI = 2,
	TP9950_CVBS = 3,
};
//==============================================================================
// FUNCTION
//==============================================================================
//------------------------------------------------------------------------
/*!
\brief Check sensor state.
\retval 1:alive. 
\retval 0:I2C fail.
*/
uint8_t ubSEN_CheckSensorState(void);
//------------------------------------------------------------------------
/*!
\brief Set frame rate and pixel clock.
\param ubPclkIdx		Set frame rate number.
\return(no)
\par [Example]
\code
		SEN_PclkSetting(SEN_FPS30);
\endcode
*/
void SEN_PclkSetting(uint8_t ubPclkIdx);
//------------------------------------------------------------------------
/*!
\brief Setup sensor initial setting and check sensor chip ID.
\param setting 	    Parameter of sensor setting struct.
\retval True		1->sensor chip initial success.
\retval False		0->read sensor chip fail.
\par [Example]
\code
		ubSEN_Start(&sensor_cfg);
\endcode
*/
uint8_t ubSEN_SetSensorInitTable(void);
//------------------------------------------------------------------------
/*!
\brief Setup sensor initial and ISP/sensor rate.
\param setting 	    Parameter of sensor setting struct.
\retval True	    1->Setup sensor setting success.
\retval False	    0->Setup sensor setting fail.
\par [Example]
\code
		ubSEN_Open(&sensor_cfg);
\endcode
*/
uint8_t ubSEN_Open(struct SENSOR_SETTING *setting);
//------------------------------------------------------------------------
/*!
\brief Transfer the exposure line of algorithm to exposure line and dummmy line.
\param ulAlgExpTime 	Exposure time.
\return(no)
\par [Example]
\code
		SEN_CalExpLDmyL(ulExpTime);
\endcode
*/
void SEN_CalExpLDmyL(uint32_t ulAlgExpTime);
//------------------------------------------------------------------------
/*!
\brief Get pixel clock from sensor struct.
\return Pixel clock value.
\par [Example]
\code
		ulSEN_GetPixClk();
\endcode
*/
uint32_t ulSEN_GetPixClk(void);
//------------------------------------------------------------------------
/*!
\brief Get pixel clock per line from sensor struct.
\return Pixel clock per line value.
\par [Example]
\code
		ulSEN_GetPckPerLine();
\endcode
*/
uint32_t ulSEN_GetPckPerLine(void);
//------------------------------------------------------------------------
/*!
\brief Write total line to sensor.
\return (no)
\par [Example]
\code
		SEN_WriteTotalLine(void);
\endcode
*/
void SEN_WriteTotalLine(void);
//------------------------------------------------------------------------
/*!
\brief Write dummy line to sensor.
\param uwDL 	Number of dummy line.
\return (no)
\par [Example]
\code
		SEN_WrDummyLine(xtSENInst.xtSENCtl.uwDmyLine);
\endcode
*/
void SEN_WrDummyLine(uint16_t uwDL);
//------------------------------------------------------------------------
/*!
\brief Write exposure line to sensor.
\param uwExpLine 	The number of exposure line.
\return (no)
\par [Example]
\code
		SEN_WrExpLine(xtSENInst.xtSENCtl.uwExpLine);
\endcode
*/
void SEN_WrExpLine(uint16_t uwExpLine);
//------------------------------------------------------------------------
/*!
\brief Write AGC(auto gain control) gain to sensor.
\param ulGainX1024 	Gain value.
\return (no)
\par [Example]
\code
		SEN_WrGain(xtSENInst.xtSENCtl.uwGain);
\endcode
*/
void SEN_WrGain(uint32_t ulGainX1024);
//------------------------------------------------------------------------
/*!
\brief Turn the group hold ON after vsync.
\return (no)
\par [Example]
\code
		SEN_GroupHoldOnVSync();
\endcode
*/
void SEN_GroupHoldOnVSync(void);
//------------------------------------------------------------------------
/*!
\brief Turn the group hold OFF after vsync.
\return (no)
\par [Example]
\code
		SEN_GroupHoldOffVSync();
\endcode
*/
void SEN_GroupHoldOffVSync(void);
//------------------------------------------------------------------------
/*!
\brief Set sensor output size and image size.
\return (no)
*/
void SEN_SetSensorImageSize(void);
//------------------------------------------------------------------------
/*!
\brief Set sensor type.
\return (no)
\par [Example]
\code
		SEN_SetSensorType();
\endcode
*/
void SEN_SetSensorType(void);
//------------------------------------------------------------------------
/*!
\brief Set sensor mirror/flip.
\param ubMirrorEn 	mirror on/off.
\param ubFlipEn 	flip on/off.
\return (no)
*/
void SEN_SetMirrorFlip(uint8_t ubMirrorEn, uint8_t ubFlipEn);
//------------------------------------------------------------------------
/*!
\brief Detect input source and update information.
\return (no)
*/
uint8_t ubSEN_DetectState(void);
//------------------------------------------------------------------------
/*!
\brief restart RN6752(include HW reset.
\return (no)
*/
void SEN_ReSensorInit(void);
//------------------------------------------------------------------------
/*!
\brief Get field state.
\return 1:odd field 0:even field.
*/
uint8_t ubSEN_GetFieldState(void);
//------------------------------------------------------------------------
/*!
\brief Set sensor brightness.
\param ubBrit 	value. (-128~127, default value is 0)
\return (no)
*/
void SEN_SetBrightness(uint8_t ubBrit);
//------------------------------------------------------------------------
/*!
\brief Set sensor contrast.
\param ubCont 	value. (0~255, default value is 0x80)
\return (no)
*/
void SEN_SetContrast(uint8_t ubCont);
//------------------------------------------------------------------------
/*!
\brief Set sensor saturation.
\param ubSatu 	value. (0~255, default value is 0x80)
\return (no)
*/
void SEN_SetSaturation(uint8_t ubSatu);
//------------------------------------------------------------------------
/*!
\brief Set sensor hue.
\param ubHue 	value. (0~255, default value is 0x80)
\return (no)
*/
void SEN_SetHue(uint8_t ubHue);
//------------------------------------------------------------------------
/*!
\brief AE realtion call back function.
\return (no)
*/
void SEN_AeCbFunc(void);
//------------------------------------------------------------------------
/*!
\brief According environment give different parameter.
\return (no)
*/
void SEN_SetEnvironment(void);
#endif

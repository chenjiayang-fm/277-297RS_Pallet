/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		IMX307.h
	\brief		Sensor IMX307 header
	\author		BoCun
	\version	3.0
	\date		2020-05-05
	\copyright	Copyright(C) 2020 SONiX Technology Co.,Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------
#ifndef _IMX307_H_
#define _IMX307_H_
#include "_510PF.h"

//==============================================================================
// DEFINITION
//==============================================================================
// Sensor IMX307
// SLAVE_ID is 0x1A [1:7] + [0] r/w
#define SEN_SLAVE_ADDR              (0x1A)
// system control registers
#define IMX307_CHIP_MODELID_ADDR    (0x301E)
#define IMX307_CHIP_MODELID         (0xB2)
#define IMX307_FRAME_LENGTH_H		(0x301A)
#define IMX307_FRAME_LENGTH_M		(0x3019)
#define IMX307_FRAME_LENGTH_L		(0x3018)
#define IMX307_DUMMY_LINE_H			(0x3022)
#define IMX307_DUMMY_LINE_M			(0x3021)
#define IMX307_DUMMY_LINE_L			(0x3020)
#define IMX307_GAIN					(0x3014)

#define IMX307_MIRROR		        (0x1 << 0)
#define IMX307_FLIP                 (0x1 << 1)
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
		ubSEN_Open(&sensor_cfg, ubSEN_FrameRate);
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
\par [Example]
\code 
		SEN_SetSensorImageSize(void);
		
		sensor(IMX323) output size is 1984x1152,
			sensor_cfg.HOSize = 1984, sensor_cfg.VOSize = 1152
		Image for 510PF ISP is FHD(1920x1080),
			sensor_cfg.HOSize = 1920, sensor_cfg.VOSize = 1080
\endcode
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
\param ubMirrorEn 	mirror.
\param ubFlipEn 	flip.
\return (no)
*/
void SEN_SetMirrorFlip(uint8_t ubMirrorEn, uint8_t ubFlipEn);
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

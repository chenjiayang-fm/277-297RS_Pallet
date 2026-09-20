/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		GL3004.h
	\brief		Sensor GL3004 header
	\author		BoCun
	\version	1.0
	\date		2020-09-18
	\copyright	Copyright(C) 2020 SONiX Technology Co.,Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------
#ifndef _GL3004_H_
#define _GL3004_H_
#include "_510PF.h"

//==============================================================================
// DEFINITION
//==============================================================================
//GL3004
#define SEN_SLAVE_ADDR_3004         (0x08)
#define GL3004_DISPLAY_MODE         (0x0A42)
//GC4653
#define SEN_SLAVE_ADDR              (0x29)
// system control registers
/* ID */
#define GC4653_CHIP_ID_HIGH_ADDR   (0x03F0)
#define GC4653_CHIP_ID_LOW_ADDR    (0x03F1)
#define GC4653_CHIP_ID             (0x4653)
#define GC4653_GAIN_L              (0x020f)
#define GC4653_GAIN_H              (0x020e)
#define GC4653_EXP_L               (0x0203)
#define GC4653_EXP_H               (0x0202)
#define GC4653_FRAME_L			   (0x0341)
#define GC4653_FRAME_H			   (0x0340)

#define GC4653_MIRROR		       (0x1 << 0)
#define GC4653_FLIP		           (0x1 << 1)

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
\retval True		1->sensor chip initial success.
\retval False		0->read sensor chip fail.
\par [Example]
\code 
		ubSEN_SetSensorInitTable();
\endcode
*/
uint8_t ubSEN_SetSensorInitTable (void);
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
\par [Example]
\code 
		SEN_SetSensorImageSize(void);
		
		sensor(OV9732) output size is 1280x720,
			sensor_cfg.HOSize = 1280, sensor_cfg.VOSize = 720
		Image for 510PF ISP is HD(1280x720),
			sensor_cfg.HOSize = 1280, sensor_cfg.VOSize = 720
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
\param ubMirrorEn 	mirror on/off.
\param ubFlipEn 	flip on/off.
\return (no)
*/
void SEN_SetMirrorFlip(uint8_t ubMirrorEn, uint8_t ubFlipEn);

void SEN_AeCbFunc(void);

void SEN_ChgDisplayMode(uint8_t ubIdx);
#endif

/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		Buzzer.h
	\brief		Buzzer header file
	\author		Ocean
	\version	0.3
	\date		2019/03/18
	\copyright	Copyright(C) 2019 SONiX Technology Co., Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------
#ifndef _BUZZER_H_
#define _BUZZER_H_
#include <stdint.h>

/** \defgroup BUZZER_MODULE Buzzer Modules
 * \ingroup MISCELLANEOUS_MODULE
 * 
 * @{
 */ 
 
#define L_DO_1	261.626		//!< Low DO Tone Frequency
#define L_RE_2	293.665		//!< Low RE Tone Frequency
#define L_MI_3	329.628		//!< Low MI Tone Frequency
#define L_FA_4	349.228		//!< Low FA Tone Frequency
#define L_SOL_5	391.995		//!< Low SOL Tone Frequency
#define L_LA_6	440			//!< Low LA Tone Frequency
#define L_SI_7	493.883		//!< Low SI Tone Frequency

#define M_DO_1	523.251		//!< Middle DO Tone Frequency
#define	M_RE_2	587.330		//!< Middle RE Tone Frequency
#define	M_MI_3	659.255		//!< Middle MI Tone Frequency
#define	M_FA_4	698.456		//!< Middle FA Tone Frequency
#define	M_SOL_5	783.991		//!< Middle SOL Tone Frequency
#define	M_LA_6	880			//!< Middle LA Tone Frequency
#define	M_SI_7	987.767		//!< Middle SI Tone Frequency

#define	H_DO_1	1046.502		//!< High DO Tone Frequency
#define H_RE_2	1174.659		//!< High RE Tone Frequency
#define	H_MI_3	1318.510		//!< High MI Tone Frequency
#define	H_FA_4	1396.913		//!< High FA Tone Frequency
#define	H_SOL_5	1567.982		//!< High SOL Tone Frequency
#define	H_LA_6	1760			//!< High LA Tone Frequency
#define	H_SI_7	1975.533		//!< High SI Tone Frequency

#define	ZERO_0	0				//!< ZERO Tone

typedef struct {
	uint16_t uwDelay;
	
	float BUZ_Freq0;			//!< Buzzer 0 Frequency
	uint8_t ubBUZ_TabSel0;		//!< Buzzer 0 Sine Wave Table
	uint8_t ubBUZ_LevIndex0;  	//!< BUZ0 LEVEL index, indicate decrease or increase value during one period.
	uint8_t ubBUZ_Volume0;		//!< The level of buzzer0 volume. max:63
	
	float BUZ_Freq1;			//!< Buzzer 1 Frequency
	uint8_t ubBUZ_TabSel1;		//!< Buzzer 1 Sine Wave Table
	uint8_t ubBUZ_LevIndex1;	//!< BUZ1 LEVEL index, indicate decrease or increase value during one period.
	uint8_t ubBUZ_Volume1;		//!< The level of buzzer1 volume. max:63
	
	float BUZ_Freq2;			//!< Buzzer 2 Frequency
	uint8_t ubBUZ_TabSel2;		//!< Buzzer 2 Sine Wave Table
	uint8_t ubBUZ_LevIndex2;	//!< BUZ2 LEVEL index, indicate decrease or increase value during one period.
	uint8_t ubBUZ_Volume2;		//!< The level of buzzer2 volume. max:63
	
	float BUZ_Freq3;			//!< Buzzer 3 Frequency
	uint8_t ubBUZ_TabSel3;		//!< Buzzer 3 Sine Wave Table
	uint8_t ubBUZ_LevIndex3;	//!< BUZ3 LEVEL index, indicate decrease or increase value during one period.
	uint8_t ubBUZ_Volume3;		//!< The level of buzzer3 volume. max:63
}BUZ_ToneParam_t;

/*!
\brief Buzzer Play Start 
\param ubChCnt			Buzzer Channel Number
\param ubWeight			Buzzer Weight (0~8)
\param pToneParam		Buzzer Tone Parameter Structure
\param ubToneGroupSize	Buzzer Tone Size
\return(no)
\par [Example]
\code 
			const static BUZ_ToneParam_t BUZ_PowerOnSound[] = {
	//	delay	Freq_0	Tab_0	LvIdx_0	Vol_0
		{50,	M_DO_1, 3,		0,		63},
		{50,	M_RE_2, 3,		0,		63},
		{50,	M_MI_3, 3,		0,		63},
		{50,	M_FA_4, 3,		0,		63},
		{50,	M_SOL_5,3,		0,		63},
		{50,	M_LA_6, 3,		0,		63},
		{50,	M_SI_7, 3,		0,		63},
		{50,	H_DO_1, 3,		0,		63},
	};
	
	BUZ_PlayStart(1, 8, (BUZ_ToneParam_t*)BUZ_PowerOnSound, sizeof(BUZ_PowerOnSound) / sizeof(BUZ_ToneParam_t));
\endcode
*/
void BUZ_PlayStart(uint8_t ubChCnt, uint8_t ubWeight, BUZ_ToneParam_t *pToneParam, uint8_t ubToneGroupSize);
/*!
\brief Buzzer Play Stop 
\return(no)
\par [Example]
\code 
	BUZ_PlayStop();
\endcode
*/
void BUZ_PlayStop(void);
/*!
\brief Buzzer Play Thread
\param argument			Thread argument
\return(no)
\par [Example]
*/
void BUZ_PlayThread(void const *argument);
/*!
\brief Buzzer Play Power on Sound 
\return(no)
\par [Example]
\code 
	BUZ_PlayPowerOnSound();
\endcode
*/
void BUZ_PlayPowerOnSound(void);
/*!
\brief Buzzer Play Power off Sound 
\return(no)
\par [Example]
\code 
	BUZ_PlayPowerOffSound();
\endcode
*/
void BUZ_PlayPowerOffSound(void);
/*!
\brief Buzzer Play Single Sound 
\return(no)
\par [Example]
\code 
	BUZ_PlaySingleSound();
\endcode
*/
void BUZ_PlaySingleSound(void);
/*!
\brief Buzzer Play Low Battery Sound 
\return(no)
\par [Example]
\code 
	BUZ_PlayLowBatSound();
\endcode
*/
void BUZ_PlayLowBatSound(void);
/*!
\brief Buzzer Play Face Sound 
\return(no)
\par [Example]
\code 
	BUZ_PlayFaceSound();
\endcode
*/
void BUZ_PlayFaceSound(void);
/*!
\brief Buzzer Play Done Sound 
\return(no)
\par [Example]
\code 
	BUZ_PlayDoneSound();
\endcode
*/
void BUZ_PlayDoneSound(void);
/*!
\brief Buzzer Play Ok Sound 
\return(no)
\par [Example]
\code 
	BUZ_PlayOkSound();
\endcode
*/
void BUZ_PlayOkSound(void);
/*!
\brief Buzzer Play Fail Sound 
\return(no)
\par [Example]
\code 
	BUZ_PlayFailSound();
\endcode
*/
void BUZ_PlayFailSound(void);
/*!
\brief Buzzer Play Test Sound 
\return(no)
\par [Example]
\code 
	BUZ_PlayTestSound();
\endcode
*/
void BUZ_PlayTestSound(void);

void BUZ_Init(void);
void BUZ_Call_DI(uint8_t ch);

#endif
/** @} */

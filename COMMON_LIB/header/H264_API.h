/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		H264_API.h
	\brief		H264 codec header
	\author		Bruce Hsu	
	\version	0.6
	\date		2018/05/18
	\copyright	Copyright(C) 2018 SONiX Technology Co.,Ltd. All rights reserved.
*/

#ifndef __H264_API__
#define __H264_API__

#include "_510PF.h"

/** \defgroup H264_MODULE H264 Modules
 * \ingroup VIDEO_MODULE
 * 
 * @{
 */ 
 
/*!
\brief H264 Function Type 
*/
typedef enum 
{
	H264_ENCODE = 0,	//!< H264 IP Encode Mode	
	H264_DECODE			//!< H264 IP Decode Mode
}H264_TYPE;						

/*!
\brief H264 Result Index 
*/ 
typedef enum
{
	H264_SUCESS,		//!< Sucess index
	H264_FAIL			//!< Fail index
}H264_BOOL_RESULT;

/*!
\brief H264 Enable Status 
*/ 
typedef enum
{
	H264_DISABLE,		//!< Disable index
	H264_ENABLE			//!< Enable index
}H264_ENABLE_STATUS;

/*!
\brief H264 Rate Control Mode 
*/ 
typedef enum
{
	CQP,					//!< Constant QP Mode of H264 Rate Control 
	CBR,					//!< Constant Bitrate of H264 Rate Control
	LBR						//!< Lower Bitrate of H264 Rate Control
	
}H264_RATE_CONTROL_MODE;
/*!
\brief H264 Encode stream selection 
*/
typedef enum
{
	ENCODE_0,			//!< Encoder Index 0
	ENCODE_1,			//!< Encoder Index 1
	ENCODE_2,			//!< Encoder Index 2
	ENCODE_3			//!< Encoder Index 3
	
}H264_ENCODE_INDEX;
/*!
\brief H264 Decode stream selection 
*/
typedef enum
{
	DECODE_0,			//!< Decoder Index 0	
	DECODE_1,			//!< Decoder Index 1
	DECODE_2,			//!< Decoder Index 2
	DECODE_3,			//!< Decoder Index 3
	DECODE_4,			//!< Decoder Index 4
	DECODE_5			//!< Decoder Index 5
}H264_DECODE_INDEX;
/*!
\brief MROI index selection 
*/
typedef enum
{
	MROI_0,				//!< MROI Index 0	
	MROI_1,				//!< MROI Index 1
	MROI_2,				//!< MROI Index 2
	MROI_3,				//!< MROI Index 3
	MROI_4,				//!< MROI Index 4
	MROI_5,				//!< MROI Index 5
	MROI_6,				//!< MROI Index 6
	MROI_7				//!< MROI Index 7
	
}H264_MROI_INDEX;

/*!
\brief MROI extension size 
*/
typedef enum
{
	SIZE_0,				//!<Extension Size =0
	SIZE_3,				//!<Extension Size =3
	SIZE_7				//!<Extension Size =7	
	
}H264_MROI_EXT_SIZE;
/*!
\brief AROI method 
*/
typedef enum
{
	MV_OR_SKIN,				//!<MV||Skin
	MV,						//!<Only MV	
	SKIN,					//!<Only Skin
	MV_AND_SKIN				//!<MV&&Skin
}AROI_METHOD;

/*!
\brief ROI report method 
*/
typedef enum
{
	MROI,        		 //!< MROI first              	                                                                                                                                                                                                                                              
	AROI,				 //!< AROI first
	QP_LOWER			 //!< QP Lower first
	
}ROI_PRIORITY;

/*!
\brief H264 result structure 
*/
typedef struct 
{
	H264_TYPE Type;									//!< H264 working type (ENCODE/DECODE) 
	H264_ENCODE_INDEX EncodeStream;					//!< Index of encode stream (0~3)
	H264_DECODE_INDEX DecodeStream;					//!< Index of decode stream (0~3)
	H264_BOOL_RESULT Result;						//!< IMG_SUCESS/IMG_FAIL
	uint32_t Size;									//!< Size of encoded/decoded result data
	uint32_t YuvAddr;								//!< Yuv data address
	uint32_t BSAddr;								//!< Bitstream data address
	uint32_t ulFrmIdx;								//!< Frame index
	uint32_t ulGop;									//!< GOP
}H264_RESULT;
/*!
\brief H264 and JPEG and Scaling down result structure 
*/
struct IMG_RESULT
{
	H264_RESULT* H264Result;	//!< H264 Codec Process Result
	uint32_t YuvAddr;			//!< Input Image Address
	uint32_t JPEGDesAddr;		//!< JPEG Output Address
	uint32_t ScalingDesAddr;	//!< Scaling Output Address
	uint32_t JPEGSize;			//!< JPEG Output Size
	uint32_t ScalingSize;		//!< Scaling Output Size
};
/*!
\brief H264 task structure 
*/
struct H264_TASK
{
	H264_TYPE Type;									//!< H264 working type (ENCODE/DECODE) 
	H264_ENCODE_INDEX EncodeStream;					//!< Index of encode stream (0~3)
	H264_DECODE_INDEX DecodeStream;					//!< Index of decode stream (0~3)
	uint32_t DesAddr;								//!< Destination address
};

/*!
\brief MROI setup structure
*/
typedef struct 
{
	H264_ENABLE_STATUS ENABLE;			//!< ENABLE/DISABLE
	H264_MROI_INDEX Num;				//!< Index of MROI(0~6)
	uint8_t ubWeight;					//!< Priority of region,0 is first priority, 7 is last
	int16_t uwQP_Value;					//!< Delata QP Value of each region (range +9 ~ -9)
	uint32_t ulPosX;					//!< X-axis start postion of region(MB unit)
	uint32_t ulWidth;					//!< Width of region
	uint32_t ulPosY;					//!< Y-axis start postion of region(MB unit)
	uint32_t ulHeight;					//!< Height of region
	H264_MROI_EXT_SIZE ExtSize;			//!< Extension size,can be 0,3 and 7
}H264_MROI_SETUP;

/*!
\brief Partition Size Control Parameter
*/

typedef struct
{
	uint8_t ubParQPAdj0;				//!< Partition BS Adjustment multiple 0
	uint8_t ubParQPAdj1;				//!< Partition BS Adjustment multiple 1
	uint8_t ubParQPAdj2;				//!< Partition BS Adjustment multiple 2
	uint8_t ubParQPAdj3;				//!< Partition BS Adjustment multiple 3
	uint8_t ubParQPAdj4;				//!< Partition BS Adjustment multiple 4
	uint8_t ubParQPAdj5;				//!< Partition BS Adjustment multiple 5
	uint8_t ubParQPAdj6;				//!< Partition BS Adjustment multiple 6
	uint8_t ubParQPNum0;				//!< Partition QP Adjustment Value 0
	uint8_t ubParQPNum1;				//!< Partition QP Adjustment Value 1
	uint8_t ubParQPNum2;				//!< Partition QP Adjustment Value 2				
	uint8_t ubParQPNum3;				//!< Partition QP Adjustment Value 3
	uint8_t ubParQPNum4;				//!< Partition QP Adjustment Value 4
	uint8_t ubParQPNum5;				//!< Partition QP Adjustment Value 5
	uint8_t ubParQPNum6;				//!< Partition QP Adjustment Value 6
	uint8_t ubParQPNum7;				//!< Partition QP Adjustment Value 7
	
}H264_PAR_SETUP;

/*!
\brief Micro Block Control Parameter
*/

typedef struct
{
	
	uint8_t MB_MODE;					//!< Micro Block Control Mode
	uint8_t MB_THD;						//!< Micro Block Threshold
	uint8_t MBQPAdj0;					//!< Micro Block AVE Adjustment multiple 0
	uint8_t MBQPAdj1;					//!< Micro Block AVE Adjustment multiple 1
	uint8_t MBQPAdj2;					//!< Micro Block AVE Adjustment multiple 2
	uint8_t MBQPAdj3;					//!< Micro Block AVE Adjustment multiple 3
	uint8_t MBQPAdj4;					//!< Micro Block AVE Adjustment multiple 4
	uint8_t MBQPAdj5;					//!< Micro Block AVE Adjustment multiple 5
	uint8_t MBQPNum0;					//!< Micro Block QP Adjustment Value 0
	uint8_t MBQPNum1;					//!< Micro Block QP Adjustment Value 1
	uint8_t MBQPNum2;					//!< Micro Block QP Adjustment Value 2
	uint8_t MBQPNum3;					//!< Micro Block QP Adjustment Value 3
	uint8_t MBQPNum4;					//!< Micro Block QP Adjustment Value 4
	uint8_t MBQPNum5;					//!< Micro Block QP Adjustment Value 5
	uint8_t MBQPNum6;					//!< Micro Block QP Adjustment Value 6
	
}H264_MB_QP_SETUP;


//------------------------------------------------------------------------
/*!
\brief Get Request Buffer Size Of Stream
\param ulWidth				Image width
\param ulHeight				Image height
\return ulBufferSize
\par [Example]
\code 
			uint32_t ulBufferSize = ulH264_Get_ENC_Buffer_Size(640,480);
\endcode
*/
uint32_t 	ulH264_GetENCBufferSize(uint32_t ulWidth,uint32_t ulHeight);
//------------------------------------------------------------------------
/*!
\brief Initial H264 Encoder Object
\param EncodeIndex			Index of encoder object
\param ulWidth					Image width
\param ulHeight					Image height
\param ulBufferStrAddr	Address of buffer of Kernel offering		
\return (no)
\par [Example]
\code 
			H264_EncodeInit(ENCODE_0,640,480,0x190000);
\endcode
*/
//void H264_EncodeInit(H264_ENCODE_INDEX EncodeIndex,uint32_t ulWidth, uint32_t ulHeight,uint32_t ulBufferStrAddr);
void H264_EncodeInit(H264_ENCODE_INDEX EncodeIndex,uint32_t ulWidth, uint32_t ulHeight,uint32_t ulBufferStrAddr,uint32_t ulFrameRate,uint32_t ulIntraPeriod);
//------------------------------------------------------------------------
/*!
\brief Set H264 Stream Frame Per Second
\param EncodeIndex			Index of encoder object
\param ulFrameRate			Frame Per Second
\return(no)
\par [Example]
\code 
			H264_SetFrameRate(ENCODE_0, 30);
\endcode
*/
void H264_SetFrameRate(H264_ENCODE_INDEX EncodeIndex,uint32_t ulFrameRate);
	
//------------------------------------------------------------------------
/*!
\brief Set QP of Encoder
\param EncodeIndex		Index of encoder object
\param ubIFrameQP						Value of QP(1~51)
\param ubPFrameQP						Value of QP(1~51)
\return(no)
\par [Example]
\code 
			H264_SetQP(ENCODE_0,25,25);
\endcode
*/
void H264_SetQp(H264_ENCODE_INDEX EncodeIndex,uint8_t ubIFrameQP,uint8_t ubPFrameQP);
//------------------------------------------------------------------------
/*!
\brief Set GOP of Encoder
\param EncodeIndex		Index of encoder object
\param ubGOP					Value of GOP
\return(no)
\par [Example]
\code 
			H264_SetGOP(ENCODE_0,30);
\endcode
*/
void H264_SetGOP(H264_ENCODE_INDEX EncodeIndex,uint32_t ulGOP);
//------------------------------------------------------------------------
/*!
\brief Get GOP of Encoder
\param EncodeIndex		Index of encoder object
\return GOP
\endcode
*/
uint32_t H264_GetGOP(H264_ENCODE_INDEX EncodeIndex);
//------------------------------------------------------------------------

/*!
\brief Set Max QP of Encoder
\param EncodeIndex		Index of encoder object
\param ubQP						Value of Max QP (range 1~51)
\return(no)
\par [Example]
\code 
			H264_SetMaxQP(ENCODE_0,30);
\endcode
*/
void H264_SetMaxQP(H264_ENCODE_INDEX EncodeIndex,uint8_t ubQP);
//------------------------------------------------------------------------
/*!
\brief Set Min QP of Encoder
\param EncodeIndex		Index of encoder object
\param ubQP						Value of Min QP (range 1~51)
\return(no)
\par [Example]
\code 
			H264_SetMinQP(ENCODE_0,10);
\endcode
*/
void H264_SetMinQP(H264_ENCODE_INDEX EncodeIndex,uint8_t ubQP);


//------------------------------------------------------------------------
/*!
\brief Force next frame is I frame
\param EncodeIndex	Index of encoder object
\return(no)
\par [Example]
\code 
		H264_ResetIPCnt(ENCODE_0);
\endcode
*/
void H264_ResetIPCnt(H264_ENCODE_INDEX EncodeIndex);
//------------------------------------------------------------------------

/*!
\brief H264 Reset  
\return (no)
*/
void H264_Reset(void);

//------------------------------------------------------------------------
/*!
\brief Set MROI Mode Enable 
\param EncodeIndex			Index of encoder object
\param ubStatus					Enable/Disable
\return(no)
\par [Example]
\code 
		H264_SetMROIModeEnable(ENCODE_0,ENABLE);
\endcode
*/
void H264_SetMROIModeEnable(H264_ENCODE_INDEX EncodeIndex,H264_ENABLE_STATUS ubStatus);

//------------------------------------------------------------------------
/*!
\brief Set MROI Parameter 
\param EncodeIndex			Index of encoder object
\param Setup						Structure of MROISetup
\return(no)
\par [Example]
\code 
	H264_MROI_SETUP MROISetup;
	
	MROISetup.Num = MROI_0;
	MROISetup.ENABLE = ENABLE;
	MROISetup.ubWeight =1;
	MROISetup.uwQP_Value =-1;
	MROISetup.ulPosX = 3;
	MROISetup.ulWidth = 3;
	MROISetup.ulPosY = 1;
	MROISetup.ulHeight = 4;
	MROISetup.ExtSize =SIZE_0;
	
	H264_SetMROI(ENCODE_0,MROISetup);
\endcode
*/
void H264_SetMROI(H264_ENCODE_INDEX EncodeIndex,H264_MROI_SETUP Setup);
//------------------------------------------------------------------------
/*!
\brief Set AROI Paratemer And Status
\param EncodeIndex		Index of encoder object
\param ubStatus				Enable/Disable
\param ubPriority			ROI ouput decision method(0:MROI first ,1:AROI first,2:QP Lower first)
\param ubMethod				AROI generate method (0:MV||Skin,1:MV,2:Skin,3MV&Skin)
\return(no)
\par [Example]
\code 
		H264_SetAROIEN(ENCODE_0,ENABLE,MROI,MV_AND_SKIN);
\endcode
*/
void H264_SetAROIEN(H264_ENCODE_INDEX EncodeIndex,H264_ENABLE_STATUS ubStatus,ROI_PRIORITY ubPriority,AROI_METHOD ubMethod);
//------------------------------------------------------------------------
/*!
\brief Set Skin Paratemer 
\param EncodeIndex			Index of encoder object
\param ubStatus					Enable/Disable
\return(no)
\par [Example]
\code 
		H264_SetSkinMode(ENCODE_0,ENABLE);
\endcode
*/
void H264_SetSkinMode(H264_ENCODE_INDEX EncodeIndex,H264_ENABLE_STATUS ubStatus);
//------------------------------------------------------------------------
/*!
\brief Set AROI Paratemer And Status
\param EncodeIndex		Index of encoder object
\param Status			Enable/Disable
\param ubSR				Search Range (range 0~7)
\return(no)
\par [Example]
\code 
		H264_SetCondensedMode(ENCODE_0,ENABLE,7);
\endcode
*/
void H264_SetCondensedMode(H264_ENCODE_INDEX EncodeIndex,H264_ENABLE_STATUS Status,uint8_t ubSR);
//------------------------------------------------------------------------
/*!
\brief Set Rate Control Enable
\param EncodeIndex		Index of encoder object
\param Status					Enable/Disable
\param Mode						RateControl mode	
\param ulTargetBS			Target bitRate
\return(no)
\par [Example]
\code 
		H264_RcSetEN(ENCODE_0,ENABLE,100000);
\endcode
*/
void H264_RcSetEN(H264_ENCODE_INDEX EncodeIndex,H264_ENABLE_STATUS Status ,H264_RATE_CONTROL_MODE Mode ,uint32_t ulTargetBS);
//------------------------------------------------------------------------
/*!
\brief Set Rate Control Target BitRate
\param EncodeIndex		Index of encoder object
\param ulTargetBS			Target bitRate
\return(no)
\par [Example]
\code 
		H264_RcSetTargetBS(ENCODE_0,100000);
\endcode
*/
void H264_RcSetTargetBS(H264_ENCODE_INDEX EncodeIndex,uint32_t ulTargetBS);
//------------------------------------------------------------------------
/*!
\brief Get Request Buffer Size Of Stream
\param ulWidth				Image width
\param ulHeight				Image height
\return BufferSize
\par [Example]
\code 
	uint32_t ulBufferSize = ulH264_Get_DEC_Buffer_Size(640,480);
\endcode
*/
uint32_t 	ulH264_GetDECBufferSize (uint32_t ulWidth,uint32_t ulHeight);
//------------------------------------------------------------------------
/*!
\brief Initial H264 		Decoder Object
\param DecodeIndex			Index of decoder object
\param ulWidth					Image width
\param ulHeight					Image height
\param ulBufferStrAddr	Address of buffer of Kernel offering
\return (no)
\par [Example]
\code 
	H264_DecodeInit(ENCODE_0,640,480,0x190000);
\endcode
*/
void H264_DecoderInit(H264_DECODE_INDEX DecodeIndex,uint32_t ulWidth, uint32_t ulHeight,uint32_t ulBufferAddr);
//------------------------------------------------------------------------
/*!
\brief Set H264 Decode Mirror Enable
\param DEcodeIndex		Index of decoder object
\param Status 				ENABLE\DISABLE
\return(no)
\par [Example]
\code 
		H264_SetMirrorEn(DECODE_0,ENABLE);
\endcode
*/
void H264_SetMirrorEn(H264_DECODE_INDEX DecodeIndex ,H264_ENABLE_STATUS Status);
//------------------------------------------------------------------------
/*!
\brief Set H264 Decode Flip Enable
\param DEcodeIndex		Index of decoder object
\param Status 				ENABLE\DISABLE
\return(no)
\par [Example]
\code 
		H264_SetFlipEn(DECODE_0,ENABLE);
\endcode
*/
void H264_SetFlipEn(H264_DECODE_INDEX DecodeIndex ,H264_ENABLE_STATUS Status);
//------------------------------------------------------------------------
/*!
\brief Set H264 Decode Rotation Enable
\param DEcodeIndex		Index of decoder object
\param Status 				ENABLE\DISABLE
\return(no)
\par [Example]
\code 
		H264_SetRotationEn(DECODE_0,ENABLE);
\endcode
*/
void H264_SetRotationEn(H264_DECODE_INDEX DecodeIndex ,H264_ENABLE_STATUS Status);
//------------------------------------------------------------------------
/*!
\brief Get H264 Current Frame QP
\return(no)
\code 
		H264_GetCurrentQP();
\endcode
*/
uint8_t H264_GetCurrentQP(void);
//------------------------------------------------------------------------
/*!
\brief Reset Rate Control
\return(no)
\code 
		H264_ResetRateControl();
\endcode
*/
void H264_ResetRateControl(uint8_t ubInitQp);


//------------------------------------------------------------------------
/*!
\brief Reset Rate Control
\return(no)
\code 
		H264_ResetRateControl();
\endcode
*/
void H264_ResetRateControl2(uint8_t ubCodecIdx,uint8_t ubInitQp);

//------------------------------------------------------------------------
/*!
\brief Get H264 Encode Stream Size
\return H264_Encode_Size
\code 
		ulH264_GetStreamSize();
\endcode
*/
uint32_t ulH264_GetStreamSize(void);
//------------------------------------------------------------------------
/*!
\brief Set Rate Control Parameter
\return H264_Encode_Size
\code 
		ulH264_GetStreamSize();
\endcode
*/
void H264_SetRCParameter(H264_ENCODE_INDEX EncodeIndex, uint32_t Bitrate, uint32_t FrameRate);
void H264_UpdateRCParameter(H264_ENCODE_INDEX EncodeIndex);
//------------------------------------------------------------------------
/*!
\brief Get Decode Error Flag
\param DecodeIndex 				Index of decoder object
\return Error=1 
\code 
		ubH264_GetDecErrorFlg();
\endcode
*/
uint8_t ubH264_GetDecErrorFlg(H264_DECODE_INDEX DecodeIndex);
//------------------------------------------------------------------------
/*!
\brief Set Decode Error Flag
\param DecodeIndex 				Index of decoder object
\param ubFlg							If Error flag = 1 else flag = 0
\return(no)
\code 
		H264_SetErrorFlg(DECODE_0,0);
\endcode
*/
void H264_SetErrorFlg(H264_DECODE_INDEX DecodeIndex,uint8_t ubFlg);
//------------------------------------------------------------------------
/*!
\brief 	Get H264 Version	
\return	Version
*/
uint16_t uwH264_GetVersion(void);
//------------------------------------------------------------------------
/*!
\brief Set H264 Clock Rate
\param ulWidth 				Image Resolution Width
\param ulHeight				Image Resolution Height
\param fps					Image frame per second
\return(no)
\code 
		SetH264Rate(1280,720,15);
\endcode
*/
void SetH264Rate(uint32_t ulWidth,uint32_t Height ,uint32_t fps);
//------------------------------------------------------------------------
/*!
\brief Get Current Period
\param EncodeIndex 						Index of eecoder object
\return current period
\code 
		period = H264_GetPeriod(ENCODE_0);
\endcode
*/
uint32_t H264_GetPeriod(H264_ENCODE_INDEX EncodeIndex);
#endif

/** @} */

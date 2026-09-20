#ifndef _AAC_ENC_H_
#define _AAC_ENC_H_
#include "_510PF.h"

#define ADO_AAC_ENC_MAX_NUM		5

//--------------------------------------------------------------------------------------------------------------------------
/*!
\brief AAC LC encode initial
\param ubUseIdx			support 0~(ADO_AAC_ENC_MAX_NUM-1) AAC encode function
\param ubEncType		0->raw format; 1->ADTS format
\param ulSampleRate		sample rate, ex:16000
\param ulChannels		1->mono; 2->stereo
\return 1->success; 0->fail
*/
uint8_t ubAAC_EncInit(uint8_t ubUseIdx, uint8_t ubEncType, uint32_t ulSampleRate, uint32_t ulChannels);
//--------------------------------------------------------------------------------------------------------------------------
/*!
\brief Get AAC LC encode unit size
\param ubUseIdx			support 0~(ADO_AAC_ENC_MAX_NUM-1) AAC encode function
\return encode unit size
*/
uint32_t ulAAC_GetEncUnitSize(uint8_t ubUseIdx);
//--------------------------------------------------------------------------------------------------------------------------
/*!
\brief AAC LC encode
\param ubUseIdx			support 0~(ADO_AAC_ENC_MAX_NUM-1) AAC encode function
\param SrcAddr			source address
\param SrcSize			source size
\param DestAddr			destination address
\param DestSize			destination size
\return 1->success; 0->fail
*/
uint8_t ubAAC_EncEncode(uint8_t ubUseIdx, uint8_t *SrcAddr, uint32_t SrcSize, uint8_t *DestAddr, uint32_t *DestSize);
//--------------------------------------------------------------------------------------------------------------------------
/*!
\brief AAC LC encode flush(encode the remaind data in the aac encode lib)
\param ubUseIdx			support 0~(ADO_AAC_ENC_MAX_NUM-1) AAC encode function
\param DestAddr			destination address
\param DestSize			destination size
\return 1->success; 0->fail
*/
uint8_t ubAAC_EncFlushEncode(uint8_t ubUseIdx, uint8_t *DestAddr, uint32_t *DestSize);
//--------------------------------------------------------------------------------------------------------------------------
/*!
\brief AAC LC encode un-initial
\param ubUseIdx			support 0~(ADO_AAC_ENC_MAX_NUM-1) AAC encode function
\return 1->success; 0->fail
*/
void AAC_EncUnInit(uint8_t ubUseIdx);
//--------------------------------------------------------------------------------------------------------------------------
#endif

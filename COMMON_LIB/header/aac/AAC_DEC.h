#ifndef _AAC_DEC_H_
#define _AAC_DEC_H_
#include "_510PF.h"

#define ADO_AAC_DEC_MAX_NUM		5

//--------------------------------------------------------------------------------------------------------------------------
/*!
\brief AAC LC decode initial
\param ubUseIdx			support 0~(ADO_AAC_DEC_MAX_NUM-1) AAC decode function
\param ubEncType		0->raw format; 1->ADTS format
\param ulSampleRate		sample rate, ex:16000
\param ulChannels		1->mono; 2->stereo
\return 1->success; 0->fail
*/
uint8_t ubAAC_DecInit(uint8_t ubUseIdx, uint8_t ubDecType, uint32_t ulSampleRate, uint32_t ulChannels);
//--------------------------------------------------------------------------------------------------------------------------
/*!
\brief AAC LC decode
\param ubUseIdx			support 0~(ADO_AAC_DEC_MAX_NUM-1) AAC decode function
\param SrcAddr			source address
\param SrcSize			source size
\param DestAddr			destination address
\param DestSize			destination size
\return 1->success; 0->fail
*/
uint8_t ubAAC_DecDecode(uint8_t ubUseIdx, uint8_t *SrcAddr, uint32_t SrcSize, uint8_t *DestAddr, uint32_t *DestSize);
//--------------------------------------------------------------------------------------------------------------------------
/*!
\brief AAC LC decode un-initial
\param ubUseIdx			support 0~(ADO_AAC_DEC_MAX_NUM-1) AAC decode function
\return 1->success; 0->fail
*/
void AAC_DecUnInit(uint8_t ubUseIdx);
//--------------------------------------------------------------------------------------------------------------------------
#endif

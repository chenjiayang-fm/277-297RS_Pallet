#ifndef __PCM_ALAW_H__
#define __PCM_ALAW_H__

//#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

#define AUD_CODEC_SUCCESS			0

#define AUD_CODEC_ERR_BASE			0x100
#define AUD_CODEC_ERR_INVALID_FORMAT		(-(AUD_CODEC_ERR_BASE + 1))
#define AUD_CODEC_ERR_INVALID_PARAMETER		(-(AUD_CODEC_ERR_BASE + 2))
#define AUD_CODEC_ERR_INSUFFICIENT_RESOUCE	(-(AUD_CODEC_ERR_BASE + 3))

#define AUD_CODEC_ENCODER 0
#define AUD_CODEC_DECODER 1

	
typedef struct snx_alaw_st
{
	int32_t type;
	uint32_t samplerate;
}SNX_ALAW_PARA_t;

/* -----------------------------------------
 * Encode the data
 * alaw:	the opened snx_alaw_st
 * p_src:	the stream of source
 * p_dst:	the stream of destination
 * src_size: the size of source
 * dst_size: get the size of destination
 *----------------------------------------*/
int32_t snx_alaw_encode(SNX_ALAW_PARA_t *alaw, int16_t *p_src, uint8_t *p_dst, int32_t src_size, int32_t  *dst_size);


/*-----------------------------------------
 * Decode the data
 * alaw:	the opened snx_alaw_st
 * p_src:	the stream of source
 * p_dst:	the stream of destination
 * src_size: the size of source
 * dst_size: get the size of destination
 *----------------------------------------*/
int32_t snx_alaw_decode(SNX_ALAW_PARA_t *alaw, uint8_t *p_src, int16_t *p_dst, int32_t src_size, int32_t  *dst_size);


#ifdef __cplusplus
}
#endif
#endif

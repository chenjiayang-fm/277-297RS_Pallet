#include <stdint.h>
#include "ADO_API.h"
#include "encoder/encoder.h"
#include "decoder/decoder.h"
//#define int32_t int
//#define int8_t char



#ifdef __cplusplus
extern "C" {
#endif

#define SNX_AUD32_TYPE_ENCODER	1	// type for encoder
#define SNX_AUD32_TYPE_DECODER	2	// type for decoder

typedef struct{
	int32_t type;			// encoder or decoder
	int32_t aud32_bytes_per_frame;	// bytes/frame in encoded data
	int32_t pcm_bytes_per_frame;	// bytes/frame in pcm S16LE data
	int32_t samples_per_frame;	// samples
	AUD32_ENCODE_INFO encode_st;
	AUD32_DECODE_INFO decode_st;
}SNX_AUD32_CONTEXT_t;

int32_t snx_aud32_open (SNX_AUD32_CONTEXT_t *aud32, int32_t type, ADO_SNX_AUD32_FORMAT format);
int32_t snx_aud32_encode (SNX_AUD32_CONTEXT_t *aud32, int8_t *p_src, int8_t *p_dst, int32_t src_bytes, int32_t *p_dst_bytes);
int32_t snx_aud32_decode (SNX_AUD32_CONTEXT_t *aud32, int8_t *p_src, int8_t *p_dst, int32_t src_bytes, int32_t *p_dst_bytes);

#ifdef __cplusplus
}
#endif

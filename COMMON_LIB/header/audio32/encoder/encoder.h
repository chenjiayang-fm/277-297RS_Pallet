/*
 * Copyright 2012 Raylios Technology, Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are NOT permitted without specific written permission
 * from above copyright holder.
 */

#ifndef AUDIO32_ENCODER_H_
#define AUDIO32_ENCODER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "../common/defs.h"

/************************************************************************************
 Local type declarations                                             
*************************************************************************************/


/* This object is used to control the command line input */
typedef struct 
{
    Word16  syntax;
    Word32  bit_rate;
    Word16  bandwidth;
    Word16  number_of_bits_per_frame;
    Word16  number_of_regions;
    Word16  frame_size;
    FILE    *fpin;
    FILE    *fp_bitstream;
} ENCODER_CONTROL;


/************************************************************************************/
typedef struct 
{
    Word16  enc_history[MAX_FRAMESIZE];
    Word16  enc_number_of_16bit_words_per_frame;
	Word16  enc_number_of_bits_per_frame;
    Word16  enc_mag_shift;
    Word16  enc_mlt_coefs[MAX_FRAMESIZE];	
	Word16  enc_frame_size;
	Word16  enc_number_of_regions;
} Subband_aud32_encode_info;
typedef struct 
{
	Word16	g_enc_samplerate;
	Word16	g_enc_special_case;
	Subband_aud32_encode_info g_aud32en_sb1;
	Subband_aud32_encode_info g_aud32en_sb2;
}AUD32_ENCODE_INFO;
/************************************************************************************/

void write_ITU_format(Word16 *out_words,
                      Word16 number_of_bits_per_frame,
                      Word16 number_of_16bit_words_per_frame,
                      FILE   *fp_bitstream);


Word16 endianessT(Word16 Word16num);
void encoder(Word16  number_of_available_bits,
             Word16  number_of_regions,
             Word16  *mlt_coefs,
             Word16  mag_shift,
             Word16  *out_words);

void bits_to_words(UWord32 *region_mlt_bits,
                   Word16  *region_mlt_bit_counts,
                   Word16  *drp_num_bits,
                   UWord16 *drp_code_bits,
                   Word16  *out_words,
                   Word16  categorization_control,
                   Word16  number_of_regions,
                   Word16  num_categorization_control_bits,
                   Word16  number_of_bits_per_frame);

void adjust_abs_region_power_index(Word16 *absolute_region_power_index,Word16 *mlt_coefs,Word16 number_of_regions);
Word16 compute_region_powers(Word16  *mlt_coefs,
                             Word16  mag_shift,
                             Word16  *drp_num_bits,
                             UWord16 *drp_code_bits,
                             Word16  *absolute_region_power_index,
                             Word16  number_of_regions);

void vector_quantize_mlts(Word16 number_of_available_bits,
                          Word16 number_of_regions,
                          Word16 num_categorization_control_possibilities,
                          Word16 *mlt_coefs,
                          Word16 *absolute_region_power_index,
                          Word16 *power_categories,
                          Word16 *category_balances,
                          Word16 *p_categorization_control,
                          Word16 *region_mlt_bit_counts,
                          UWord32 *region_mlt_bits);

Word16 vector_huffman(Word16 category,
                      Word16 power_index,
                      Word16 *raw_mlt_ptr,
                      UWord32 *word_ptr);

Word16 endianessT(Word16 Word16num);

int aud32_encode_init( AUD32_ENCODE_INFO *ptr, Word32 samplerate, Word32 bitrate, Word16 *inbuf_size, Word16 *outbuf_size);
void aud32_encode_frame(AUD32_ENCODE_INFO *ptr, Word16* input,Word16 *output);

#ifdef __cplusplus
}
#endif

#endif /* AUDIO32_ENCODER_H_ */

#ifndef AUDIO32_DECODER_H_
#define AUDIO32_DECODER_H_

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
    FILE    *fpout;
    FILE    *fp_bitstream;
} DECODER_CONTROL;



/************************************************************************************/
typedef struct
{

	Bit_Obj dec_bitobj;
	Rand_Obj dec_randobj;
	Word16 dec_number_of_bits_left;
	Word16 dec_words;
	Word16 dec_frame_error_flag ;//= 0;
	Word16 dec_decoder_mlt_coefs[SUB_AUD32_DCT_LENGTH];    
	Word16 dec_old_mag_shift;//=0;
	Word16 dec_mag_shift;
	Word16 dec_old_decoder_mlt_coefs[SUB_AUD32_DCT_LENGTH];
	Word16 dec_old_samples[SUB_AUD32_DCT_LENGTH>>1];
	Word16 dec_frame_size;
	Word16 dec_number_of_regions;
	Word16 dec_number_of_bits_per_frame;
	Word16 dec_number_of_16bit_words_per_frame;
}Subband_aud32_decode_info;
typedef struct
{
	Word16	g_dec_samplerate;
	Word32 g_dec_frame_cnt;
	Word16 g_output_gain;
	Word16 g_output_gain_frac;
	Word32 g_mbe_totalframelen;
	Word16 g_dec_special_case;
	Subband_aud32_decode_info g_aud32de_sb1;
	Subband_aud32_decode_info g_aud32de_sb2;
} AUD32_DECODE_INFO;
/************************************************************************************/

void decoder(Bit_Obj *bitobj,
             Rand_Obj *randobj,
             Word16 number_of_regions,
	         Word16 *decoder_mlt_coefs,
	         Word16 *p_mag_shift,
             Word16 *p_old_mag_shift,
             Word16 *old_decoder_mlt_coefs,
             Word16 frame_error_flag);
			 
void decode_envelope(Bit_Obj *bitobj,
                     Word16  number_of_regions,
                     Word16  *decoder_region_standard_deviation,
		             Word16  *absolute_region_power_index,
		             Word16  *p_mag_shift);
					 
void rate_adjust_categories(Word16 categorization_control,
			                Word16 *decoder_power_categories,
			                Word16 *decoder_category_balances,
							Word16 number_of_regions);
							
void decode_vector_quantized_mlt_indices(Bit_Obj  *bitobj,
                                         Rand_Obj *randobj,
                                         Word16   number_of_regions,
                                         Word16   *decoder_region_standard_deviation,
					                     Word16   *decoder_power_categories,
					                     Word16   *decoder_mlt_coefs);

Word16 index_to_array(Word16 index,Word16 *array,Word16 category);

void test_4_frame_errors(Bit_Obj *bitobj,
                         Word16 number_of_regions,
                         Word16 num_categorization_control_possibilities,
                         Word16 *frame_error_flag,
                         Word16 categorization_control,
                         Word16 *absolute_region_power_index);
						 
void error_handling(Word16 number_of_coefs,
                    Word16 number_of_valid_coefs,
                    Word16 *frame_error_flag,
                    Word16 *decoder_mlt_coefs,
                    Word16 *old_decoder_mlt_coefs,
                    Word16 *p_mag_shift,
                    Word16 *p_old_mag_shift);
					
void get_next_bit(Bit_Obj *bitobj);

Word16 get_rand(Rand_Obj *randobj);



Word16 read_ITU_format(Word16 *out_words,
                       Word16 *p_frame_error_flag,
                       Word16 number_of_16bit_words_per_frame,
		               FILE   *fp_bitstream);

int aud32_decode_init(AUD32_DECODE_INFO *ptr, Word32 samplerate, Word32 bitrate, Word16 *inbuf_size, Word16 *outbuf_size);
void aud32_decode_frame(AUD32_DECODE_INFO *ptr, Word16* input, Word16 *output);

#ifdef __cplusplus
}
#endif

#endif /* AUDIO32_DECODER_H_ */

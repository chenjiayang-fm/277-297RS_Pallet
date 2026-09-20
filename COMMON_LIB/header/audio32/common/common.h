/***********************************************************************
************************************************************************
**
**   ITU-T 7/14kHz Audio Coder Candidate (G.722.1 Annex C) Source Code
**
**   ?2004 Polycom, Inc.
**
**   All rights reserved.
**
************************************************************************
***********************************************************************/
#ifndef AUDIO32LIB_COMMON_H_
#define AUDIO32LIB_COMMON_H_

#ifdef __cplusplus
extern "C" {
#endif


#include "defs.h"


/***************************************************************************/
/* Function definitions                                                    */
/***************************************************************************/
Word16  compute_region_powers(Word16  *mlt_coefs,
                             Word16  mag_shift,
                             Word16  *drp_num_bits,
                             UWord16 *drp_code_bits,
                             Word16  *absolute_region_power_index,
                             Word16  number_of_regions);

void    vector_quantize_mlts(Word16 number_of_available_bits,
                          Word16 number_of_regions,
                          Word16 num_categorization_control_possibilities,
                          Word16 *mlt_coefs,
                          Word16 *absolute_region_power_index,
                          Word16 *power_categories,
                          Word16 *category_balances,
                          Word16 *p_categorization_control,
                          Word16 *region_mlt_bit_counts,
                          UWord32 *region_mlt_bits);

Word16  vector_huffman(Word16 category,
                      Word16 power_index,
                      Word16 *raw_mlt_ptr,
                      UWord32 *word_ptr);


void    adjust_abs_region_power_index(Word16 *absolute_region_power_index,Word16 *mlt_coefs,Word16 number_of_regions);

void    bits_to_words(UWord32 *region_mlt_bits,Word16 *region_mlt_bit_counts,
                      Word16 *drp_num_bits,UWord16 *drp_code_bits,Word16 *out_words,
                      Word16 categorization_control, Word16  number_of_regions,
                      Word16  num_categorization_control_bits, Word16 number_of_bits_per_frame);

void    encoder(Word16  number_of_available_bits,
                Word16  number_of_regions,
                Word16  *mlt_coefs,
                Word16  mag_shift,
                Word16  *out_words);

void decoder(Bit_Obj *bitobj,
    	     Rand_Obj *randobj,
             Word16 number_of_regions,
             Word16 *decoder_mlt_coefs,
	         Word16 *p_mag_shift,
	         Word16 *p_old_mag_shift,
	         Word16 *old_decoder_mlt_coefs,
	         Word16 frame_error_flag);

Word16  samples_to_rmlt_coefs(Word16 *new_samples,Word16 *history,Word16 *coefs,Word16 dct_length);
void rmlt_coefs_to_samples(Word16 *coefs,     
                           Word16 *old_samples,           
                           Word16 *out_samples,           
                           Word16 dct_length,           
                           Word16 mag_shift);

Word16  index_to_array(Word16 index,Word16 *array,Word16 category);
//void    categorize(Word16 number_of_available_bits,
//                   Word16 number_of_regions,
//				   Word16 num_categorization_control_possibilities,
//		           Word16 *rms_index,
//		           Word16 *power_categories,
//		           Word16 *category_balances);

Word16 calc_offset(Word16 *rms_index,Word16 number_of_regions,Word16 available_bits);    
void   compute_raw_pow_categories(Word16 *power_categories,Word16 *rms_index,Word16 number_of_regions,Word16 offset);
void   comp_powercat_and_catbalance(Word16 *power_categories,
                                    Word16 *category_balances,
                                    Word16 *rms_index,
                                    Word16 number_of_available_bits,
                                    Word16 number_of_regions,
                                    Word16 num_categorization_control_possibilities,
                                    Word16 offset);
 
//void dct_type_iv_a (Word16 *input,Word16 *output,Word16 dct_length);
void dct_type_iv_s(Word16 *input,Word16 *output,Word16 dct_length);
void decode_envelope(Bit_Obj *bitobj,
                     Word16  number_of_regions,
                     Word16  *decoder_region_standard_deviation,
		             Word16  *absolute_region_power_index,
		             Word16  *p_mag_shift);

void decode_vector_quantized_mlt_indices(Bit_Obj  *bitobj,
                                         Rand_Obj *randobj,
                                         Word16   number_of_regions,
                                         Word16   *decoder_region_standard_deviation,
					                     Word16   *dedecoder_power_categories,
					                     Word16   *dedecoder_mlt_coefs);

void rate_adjust_categories(Word16 categorization_control,
			                Word16 *decoder_power_categories,
			                Word16 *decoder_category_balances,
							Word16	number_of_regions);

void get_next_bit(Bit_Obj *bitobj);
Word16 get_rand(Rand_Obj *randobj);

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


#ifdef __cplusplus
}
#endif

#endif /* AUDIO32LIB_COMMON_H_ */

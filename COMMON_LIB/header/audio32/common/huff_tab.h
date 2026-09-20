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
#ifndef AUDIO32LIB_HUFF_TAB_H_
#define AUDIO32LIB_HUFF_TAB_H_

#ifdef __cplusplus
extern "C" {
#endif

extern const Word16  differential_region_power_bits[NUMBER_OF_REGIONS_32KHZ][DIFF_REGION_POWER_LEVELS];
extern const UWord16 differential_region_power_codes[NUMBER_OF_REGIONS_32KHZ][DIFF_REGION_POWER_LEVELS];
extern const Word16  differential_region_power_decoder_tree[NUMBER_OF_REGIONS_32KHZ][DIFF_REGION_POWER_LEVELS-1][2];
extern const Word16  mlt_quant_centroid[NUM_CATEGORIES][MAX_NUM_BINS];
extern const Word16  expected_bits_table[NUM_CATEGORIES];
extern const Word16  mlt_sqvh_bitcount_category_0[196];
extern const UWord16 mlt_sqvh_code_category_0[196];
extern const Word16  mlt_sqvh_bitcount_category_1[100];
extern const UWord16 mlt_sqvh_code_category_1[100];
extern const Word16  mlt_sqvh_bitcount_category_2[49];
extern const UWord16 mlt_sqvh_code_category_2[49];
extern const Word16  mlt_sqvh_bitcount_category_3[625];
extern const UWord16 mlt_sqvh_code_category_3[625];
extern const Word16  mlt_sqvh_bitcount_category_4[256];
extern const UWord16 mlt_sqvh_code_category_4[256];
extern const Word16  mlt_sqvh_bitcount_category_5[243];
extern const UWord16 mlt_sqvh_code_category_5[243];
extern const Word16  mlt_sqvh_bitcount_category_6[32];
extern const UWord16 mlt_sqvh_code_category_6[32];
extern Word16  *table_of_bitcount_tables[NUM_CATEGORIES-1];
extern UWord16 *table_of_code_tables[NUM_CATEGORIES-1];
extern const Word16  mlt_decoder_tree_category_0[180][2];
extern const Word16  mlt_decoder_tree_category_1[93][2];
extern const Word16  mlt_decoder_tree_category_2[47][2];
extern const Word16  mlt_decoder_tree_category_3[519][2];
extern const Word16  mlt_decoder_tree_category_4[208][2];
extern const Word16  mlt_decoder_tree_category_5[191][2];
extern const Word16  mlt_decoder_tree_category_6[31][2];
extern Word16  *table_of_decoder_tables[NUM_CATEGORIES-1];



#ifdef __cplusplus
}
#endif

#endif /* AUDIO32LIB_HUFF_TAB_H_ */    

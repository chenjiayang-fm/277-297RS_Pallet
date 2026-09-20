/***********************************************************************
************************************************************************
**
**   ITU-T 7/14kHz Audio Coder Candidate (G.722.1 Annex C) Source Code
**
**   ?1999 PictureTel Coporation
**          Andover, MA, USA  
**
**	    All rights reserved.
**
************************************************************************
***********************************************************************/
#ifndef AUDIO32LIB_TABLES_H_
#define AUDIO32LIB_TABLES_H_

#ifdef __cplusplus
extern "C" {
#endif

/***********************************************************************
  Filename:    tables.h    

  Purpose:     Contains table definitions used by G.722.1 Annex C
		
  Design Notes:

***********************************************************************/
/***********************************************************************
 Include files                                                           
***********************************************************************/
#define REGION_POWER_TABLE_SIZE 64
#define NUM_CATEGORIES          8
#define DCT_LENGTH                  320
//#define MAX_DCT_LENGTH          640

extern const Word16 int_region_standard_deviation_table[REGION_POWER_TABLE_SIZE];
extern const Word16 standard_deviation_inverse_table[REGION_POWER_TABLE_SIZE];
extern const Word16 step_size_inverse_table[NUM_CATEGORIES];
extern const Word16 vector_dimension[NUM_CATEGORIES];
extern const Word16 number_of_vectors[NUM_CATEGORIES];
/* The last category isn't really coded with scalar quantization. */
extern const Word16 max_bin[NUM_CATEGORIES];
extern const Word16 max_bin_plus_one_inverse[NUM_CATEGORIES];
extern const Word16 int_dead_zone[NUM_CATEGORIES];
extern const Word16 samples_to_rmlt_window_8khz[DCT_LENGTH_8KHZ];
extern const Word16 samples_to_rmlt_window_16khz[DCT_LENGTH_16KHZ];

extern const Word16 rmlt_to_samples_window_8khz[DCT_LENGTH_8KHZ];
extern const Word16 rmlt_to_samples_window_16khz[DCT_LENGTH_16KHZ];


/* Add next line in Release 1.2 */
extern const Word16 int_dead_zone_low_bits[NUM_CATEGORIES];




#ifdef __cplusplus
}
#endif

#endif /* AUDIO32LIB_TABLES_H_ */    

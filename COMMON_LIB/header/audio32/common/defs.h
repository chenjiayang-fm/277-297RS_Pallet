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
//2014/11/11 Jerry test on VC platform (Windows)
//#define AUD32_BUF2BYTE_ALIGNMENT


#ifndef AUDIO32LIB_DEFS_H_
#define AUDIO32LIB_DEFS_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
// SONiX modify - start, <Ricky>
#include "typedef.h"
#include "basop32.h"
// SONiX modify - end, <Ricky>

//#define WAVETOOL_VERSION

#define SUBBAND_AUD32_LOWBAND_BITRATE_RATIO 3  //3 bit/sample for low freq (0 ~  8kHz)
#define SUBBAND_AUD32_HIBAND_BITRATE_RATIO 1   //1 bit/sample for hi  freq (8 ~ 16kHz)

#define  PI             3.141592653589793238462


/* Yishion modify for non-aligned data, 20140310 */
/* For Linux Platform ONLY*/
#ifndef AUD32_BUF2BYTE_ALIGNMENT
#define PUT2BYTES(ptr, data)	\
{\
	Word8 *p = (Word8 *)(ptr);\
	Word16 d = (data);\
	*p++ = (Word8)(d & 0xff);\
	*p++ = (Word8)((d >> 8) & 0xff);\
}
#endif /* AUD32_BUF2BYTE_ALIGNMENT */



	
#define DCT_LENGTH_8KHZ		160
#define DCT_LENGTH_16KHZ    320
#define DCT_LENGTH_32KHZ    640
#define MAX_DCT_LENGTH		DCT_LENGTH_32KHZ	

#define SUB_AUD32_DCT_LENGTH DCT_LENGTH_16KHZ	//Subband Method

//#define DCT_LENGTH          320
#define DCT_LENGTH_DIV_2    160
#define DCT_LENGTH_DIV_4     80
#define DCT_LENGTH_DIV_8     40
#define DCT_LENGTH_DIV_16    20
#define DCT_LENGTH_DIV_32    10
#define DCT_LENGTH_DIV_64     5


#define MAX_t(a,b) (a > b ? a : b)
#define MIN_t(a,b) (a < b ? a : b)

#define NUM_CATEGORIES                  8



#define NUM_CATEGORIZATION_CONTROL_BITS_8KHZ	8 //aud16 8  aud32 4
#define NUM_CATEGORIZATION_CONTROL_BITS_16KHZ	4 //aud16 8  aud32 4
#define NUM_CATEGORIZATION_CONTROL_BITS_32KHZ	5 //aud16 8  aud32 4
	

#define NUM_CATEGORIZATION_CONTROL_POSSIBILITIES_8KHZ	32	//aud16 32   aud32 16
#define NUM_CATEGORIZATION_CONTROL_POSSIBILITIES_16KHZ	16	//aud16 32   aud32 16
#define NUM_CATEGORIZATION_CONTROL_POSSIBILITIES_32KHZ	32	//aud16 32   aud32 16

#define CORE_SIZE       10



#define DCT_LENGTH_LOG_8KHZ	  5					  //aud16 5      aud32   6,7(32kHZ)
#define DCT_LENGTH_LOG_16KHZ  6					  //aud16 5      aud32   6,7(32kHZ)
#define DCT_LENGTH_LOG_32KHZ  7					  //aud16 5      aud32   6,7(32kHZ)


/*  region_size = (BLOCK_SIZE * 0.875)/NUM_REGIONS; */
			
#define NUMBER_OF_REGIONS_8KHZ	7						//aud16 aud32
#define NUMBER_OF_REGIONS_16KHZ	14						//aud16 aud32
#define NUMBER_OF_REGIONS_32KHZ	28						//aud16 aud32

#define NUMBER_OF_REGIONS_8KHZ_ALLBAND	8						//aud16 aud32
#define NUMBER_OF_REGIONS_16KHZ_ALLBAND	16						//aud16 aud32
#define NUMBER_OF_REGIONS_32KHZ_ALLBAND	32						//aud16 aud32


#define REGION_SIZE             20

#define NUMBER_OF_VALID_COEFS_8KHZ   (NUMBER_OF_REGIONS_8KHZ * REGION_SIZE)		//aud16 aud32
#define NUMBER_OF_VALID_COEFS_16KHZ   (NUMBER_OF_REGIONS_16KHZ * REGION_SIZE)	//aud16 aud32
#define NUMBER_OF_VALID_COEFS_32KHZ   (NUMBER_OF_REGIONS_32KHZ * REGION_SIZE)	//aud16 aud32


#define REGION_POWER_TABLE_SIZE 64
#define REGION_POWER_TABLE_NUM_NEGATIVES 24


#define ENCODER_SCALE_FACTOR 18318.0

/* The MLT output is incorrectly scaled by the factor
   product of ENCODER_SCALE_FACTOR and sqrt(160.)
   This is now (9/30/96) 1.0/2^(4.5) or 1/22.627.
   In the current implementation this  
   must be an integer power of sqrt(2). The
   integer power is ESF_ADJUSTMENT_TO_RMS_INDEX.
   The -2 is to conform with the range defined in the spec. */

 
#define ESF_ADJUSTMENT_TO_RMS_INDEX (9-2)
 

#define INTERMEDIATE_FILES_FLAG 0

/* Max bit rate is 48000 bits/sec. */
#define MAX_BITS_PER_FRAME 1280			//aud16 640 aud32 960
#define MAX_BITS_PER_FRAME_A32 1280

/************************************************************************************
 Constant definitions                                                    
*************************************************************************************/
#define MAX_SAMPLE_RATE		32000


#define MAX_FRAMESIZE		SUB_AUD32_DCT_LENGTH  //(MAX_SAMPLE_RATE/50)//aud16 (MAX_SAMPLE_RATE/100)  aud32 (MAX_SAMPLE_RATE/50)

#define MEASURE_WMOPS		0
#define WMOPS			0

/***************************************************************************/
/* Type definitions                                                        */
/***************************************************************************/
typedef struct
{
    Word16 code_bit_count;      /* bit count of the current word */
    Word16 current_word;        /* current word in the bitstream being processed */
    Word16 *code_word_ptr;      /* pointer to the bitstream */
    Word16 number_of_bits_left; /* number of bits left in the current word */
    Word16 next_bit;            /* next bit in the current word */
}Bit_Obj;

typedef struct
{
    Word16 seed0;
    Word16 seed1;
    Word16 seed2;
    Word16 seed3;
}Rand_Obj;

void    categorize(Word16 number_of_available_bits,
                   Word16 number_of_regions,
				   Word16 num_categorization_control_possibilities,
		           Word16 *rms_index,
		           Word16 *power_categories,
		           Word16 *category_balances);

void dct_type_iv_a (Word16 *input,Word16 *output,Word16 dct_length);

#ifdef __cplusplus
}
#endif

#endif /* AUDIO32LIB_DEFS_H_ */

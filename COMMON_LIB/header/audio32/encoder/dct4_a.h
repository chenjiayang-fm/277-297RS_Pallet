/****************************************************************************************
*****************************************************************************************
**
**   ITU-T 7/14kHz Audio Coder Candidate (G.722.1 Annex C) Source Code
**
**   ?2004 Polycom, Inc.
**
**	 All rights reserved.
**
*****************************************************************************************

****************************************************************************************/
/****************************************************************************************
  Filename:    dct4_a.h    

  Purpose:     Contains tables used by dct4_a.c
		
  Design Notes:

****************************************************************************************/
/***************************************************************************
 Include files                                                           
***************************************************************************/
#ifndef AUDIO32LIB_DCT4_A_H_
#define AUDIO32LIB_DCT4_A_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <math.h>
#include "../common/defs.h"

typedef struct 
{
    Word16	cosine;
    Word16	minus_sine;
} cos_msin_t;


//void dct_type_iv_a (Word16 *input,Word16 *output,Word16 dct_length);

#ifdef __cplusplus
}
#endif

#endif /* AUDIO32LIB_COUNT_H_ */

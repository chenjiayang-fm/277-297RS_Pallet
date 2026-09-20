/*
  ===========================================================================
   File: BASOP.H                                         v.1.1 - 05.Jul.2000
  ===========================================================================

		      ITU-T STL  BASIC OPERATORS

 		      GLOBAL FUNCTION PROTOTYPES

   History:
   26.Jan.00	v1.0	Incorporated to the STL from updated G.723.1/G.729 
                        basic operator library (based on basic_op.h) and 
                        G.723.1's basop.h.
   05.Jul.00    v1.1    Added 32-bit shiftless mult/mac/msub operators
  ============================================================================ */
#ifndef AUDIO32LIB_BASOP32_H_
#define AUDIO32LIB_BASOP32_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "defs.h"
#ifndef BASOP_H_DEFINED
#define BASOP_H_DEFINED 110

/*___________________________________________________________________________
 |                                                                           |
 |   Constants and Globals                                                   |
 |                                                                           |
 | $Id $
 |___________________________________________________________________________|
*/

extern Flag Overflow;
extern Flag Carry;

#define MAX_32 0x7fffffffL
#define MIN_32 0x80000000L

#define MAX_16 0x7fff
#define MIN_16 0x8000

#define UMAX_32 0xffffffffL
#define UMIN_32 0x00000000L

/*___________________________________________________________________________
 |                                                                           |
 |   Prototypes for basic arithmetic operators                               |
 |___________________________________________________________________________|
*/



#define saturate(L_var1) (L_var1 > 0X00007fffL) ? MAX_16 : ((L_var1 < (Word32)0xffff8000L) ? MIN_16 : (Word16)L_var1)
#define saturateflag(L_var1) (L_var1 > 0X00007fffL) ? 1 : ((L_var1 < (Word32)0xffff8000L) ? 1 : 0)	


#define BASOP32_OPTIMAL
#ifdef BASOP32_OPTIMAL
	//2014/11/10  Jerry add for Aud16 Optimization
	//相較於原版本Code內有需要修改部分請search Jerry or BASOP32_OPTIMAL
	//16-bit...................................................................................
	// shl, shr, 比較有可能會有bug
	#define add(a,b) (saturate( ( (int)(a)+(int)(b) ) ))
	#define sub(a,b) (saturate( ( (int)(a)-(int)(b) ) ))
	#define mult(a,b)     ((short)saturate(  (  ((int)(a))*((int)(b))>>15  )  )	)	//Q15
	#define shl(a,b) (saturate(	(int)(a)<<(int)(b)    )    )			//((b) < 0 ? (  (a)>> ( -(b)  )  : saturate( (int)(a)<<(int)(b) )    )   
	#define shr(a,b) (		  (	(int)(a)>>(int)(b)    )	   )			//((b) < 0 ? saturate(  (int)(a)<< (int)( -(b)  )  : (a)>>(b)    ) 
	#define negate(a)	( (a) == MIN_16 ) ? MAX_16 :  -(a) 
	//#define abs_s(a) ((a) < 0 ? (-(a)) : (a))  

	//32-bit...................................................................................  
	//L_add L_sub L_shl L_shr   要檢查overflow似乎換不掉
	#define L_shr(a,b) a>>b // only for audio32
	#define L_shl(a,b) a<<b // only for audio32
	#define L_add(a,b) (a)+(b)	
	#define L_sub(a,b) (a)-(b)
	#define L_mult(a,b)   (  ( (int)(a))*( (int)(b) )<<1  )		//Q31  有一個值會有誤差1,不確定
	#define L_mult0(a,b) (   (int)(a)  )*(  (int)(b)   )
	#define L_mac(a,b,c)  (L_add( (a),(L_mult((b),(c))) ) )
	#define L_mac0(a,b,c)  ( L_add((a),L_mult0((b),(c))) )
	#define extract_l(a) (short)( ((a)) & 0x0000FFFF  )
	#define round_audio16(a)  (short)(     ( L_add( (a),((int)0x00008000L))      )>>16 )
	#define L_deposit_l(a)  ( ((int)(a)) )
	//#define L_shl(a,b) ((b) < 0 ? (  (a)>> ( (-1)*(b) ) )  : (a)<<(b)    ) 
	//#define L_shr(a,b) ((b) < 0 ? (  (a)<< ( (-1)*(b) ) )  : (a)>>(b)    ) 
	//#define LU_shr(a,b) (  (b) < 0 ?     (   (a)<< ( abs_s((b)) )    ) :   ( (a)>>(b) )        )

	//useless...................................................................................
	//#define extract_h(a) (short)( ((a)>>16) & 0x0000FFFF  )
	//#define L_msu(a,b,c)  (a)-(L_mult((b),(c)))
	//#define L_negate(a) ((-1)*(a))
	//#define L_deposit_h(a)  ( ((int)(a))<<16 )

#endif



#ifndef add    
Word16 add (Word16 var1, Word16 var2);    
#endif

#ifndef sub    
Word16 sub (Word16 var1, Word16 var2);   
#endif

#ifndef abs_s              
Word16 abs_s (Word16 var1);            
#endif

#ifndef shl   
Word16 shl (Word16 var1, Word16 var2);    
#endif

#ifndef shr   
Word16 shr (Word16 var1, Word16 var2);    
#endif

#ifndef mult  
Word16 mult (Word16 var1, Word16 var2);  
#endif

#ifndef L_mult 
Word32 L_mult (Word16 var1, Word16 var2); 
#endif

#ifndef negate              
Word16 negate (Word16 var1);              
#endif

#ifndef extract_h         
Word16 extract_h (Word32 L_var1);         
#endif

#ifndef extract_l         
Word16 extract_l (Word32 L_var1);         
#endif

#ifndef round_audio16             
Word16 round_audio16 (Word32 L_var1);             
#endif

#ifndef L_mac
Word32 L_mac (Word32 L_var3, Word16 var1, Word16 var2);
#endif

#ifndef L_msu   
Word32 L_msu (Word32 L_var3, Word16 var1, Word16 var2);
#endif

#ifndef L_macNs 
Word32 L_macNs (Word32 L_var3, Word16 var1, Word16 var2);
#endif

#ifndef L_msuNs 
Word32 L_msuNs (Word32 L_var3, Word16 var1, Word16 var2);
#endif

#ifndef L_add   
Word32 L_add (Word32 L_var1, Word32 L_var2);
#endif

#ifndef L_sub   
Word32 L_sub (Word32 L_var1, Word32 L_var2);  
#endif

#ifndef L_add_c  
Word32 L_add_c (Word32 L_var1, Word32 L_var2);
#endif

#ifndef L_sub_c  
Word32 L_sub_c (Word32 L_var1, Word32 L_var2);
#endif

#ifndef L_negate                
Word32 L_negate (Word32 L_var1);  
#endif

#ifndef mult_r       
Word16 mult_r (Word16 var1, Word16 var2);  
#endif

#ifndef L_shl     
Word32 L_shl (Word32 L_var1, Word16 var2);  
#endif

#ifndef L_shr     
Word32 L_shr (Word32 L_var1, Word16 var2);
#endif

#ifndef shr_r       
Word16 shr_r (Word16 var1, Word16 var2); 
#endif

#ifndef mac_r 
Word16 mac_r (Word32 L_var3, Word16 var1, Word16 var2);
#endif

#ifndef msu_r 
Word16 msu_r (Word32 L_var3, Word16 var1, Word16 var2);
#endif

#ifndef L_deposit_h        
Word32 L_deposit_h (Word16 var1); 
#endif

#ifndef L_deposit_l        
Word32 L_deposit_l (Word16 var1);
#endif 

#ifndef L_shr_r 
Word32 L_shr_r (Word32 L_var1, Word16 var2);
#endif
												
#ifndef L_abs           
Word32 L_abs (Word32 L_var1);  
#endif

#ifndef L_sat            
Word32 L_sat (Word32 L_var1); 
#endif

#ifndef norm_s             
Word16 norm_s (Word16 var1); 
#endif

#ifndef div_s 
Word16 div_s (Word16 var1, Word16 var2);
#endif

#ifndef norm_l         
Word16 norm_l (Word32 L_var1); 
#endif
/*
   Additional G.723.1 operators
*/

#ifndef L_mls     /* Weight FFS currently assigned 1 */
Word32 L_mls( Word32, Word16 ) ;  
#endif

#ifndef div_l     /* Weight FFS currently assigned 1 */
Word16 div_l( Word32, Word16 ) ; 
#endif

#ifndef i_mult  /* Weight FFS currently assigned 1 */
Word16 i_mult(Word16 a, Word16 b);
#endif

/* 
    New shiftless operators,not used in G.729/G.723.1
*/

#ifndef L_mult0 /* 32-bit Multiply w/o shift         1 */
Word32 L_mult0(Word16 v1, Word16 v2); 
#endif

#ifndef L_mac0 /* 32-bit Mac w/o shift  1 */
Word32 L_mac0(Word32 L_v3, Word16 v1, Word16 v2);
#endif

#ifndef L_msu0 /* 32-bit Msu w/o shift  1 */
Word32 L_msu0(Word32 L_v3, Word16 v1, Word16 v2);
#endif
/* 
    Additional G.722.1 operators
*/

#ifndef LU_shl
UWord32 LU_shl (UWord32 L_var1, Word16 var2);
#endif

#ifndef LU_shr
UWord32 LU_shr (UWord32 L_var1, Word16 var2);
#endif



#ifndef norm_s
Word16 norm_s (Word16 var1);             /* Short norm,           15  */
#endif












#endif

					
/* ------------------------- End of saturate() ------------------------- */

#ifdef __cplusplus
}
#endif

#endif /* AUDIO32LIB_BASOP32_H_ */

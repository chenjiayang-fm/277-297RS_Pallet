#ifndef _RWERROR_H
#define _RWERROR_H

#define RW_ERROR_BADF        9  /* Bad file number */
#define RW_ERROR_AGAIN      11  /* Try again */
#define RW_ERROR_FAULT      14  /* Bad address */
#define RW_ERROR_INVAL      22  /* Invalid argument */
#define RW_ERROR_MSGSIZE    90  /* Message too long */
#define RW_ERROR_NOPROTOOPT 92  /* Protocol not available */
#define RW_ERROR_ADDRINUSE  98  /* Address already in use */
#define RW_ERROR_ENOTCONN   107 /* Transport endpoint is not connected */
#define RW_ERROR_TIMEDOUT   110 /* Timed out */
#define RW_ERROR_OVERRUN    160 /* Overrun */
#define RW_ERROR_NACK       161 /* No ack */
#define RW_ERROR_SPI        192 /* SPI error */
#define RW_ERROR_VCO        193 /* VCO calibration fail */
#define RW_ERROR_CRC        194 /* CRC or HEC error */

#endif

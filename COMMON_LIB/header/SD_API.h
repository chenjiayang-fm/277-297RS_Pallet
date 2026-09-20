/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		SD_API.h
	\brief		SD API access header file
	\author		Justin Chen
	\version	0.31
	\date		2021/7/14
	\copyright	Copyright(C) 2017 SONiX Technology Co.,Ltd. All rights reserved.
*/

#ifndef __SD_API_h
#define __SD_API_h

#include "_510PF.h"

#define SD_FunctionDisable								\
						{								\
							GLB->SDIO1_FUNC_DIS = 1;	\
							GLB->SDIO2_FUNC_DIS = 1;	\
							GLB->SDIO3_FUNC_DIS = 1;	\
						}
						
#define SD_GET_AU_SIZE_EN	0
//------------------------------------------------------------------------
#pragma pack(push)
#pragma pack(1)
//------------------------------------------------------------------------
typedef enum
{
	SD_1 = 0,
	SD_2,
	SD_3,
}SD_DEVICE_t;

typedef enum
{
	SD_ACC_TRGANDFORGOT = 0,	//Trigger and forgot 	(Interrupt Mode)
	SD_ACC_WAIT					//Trigger and wait		(Normal Mode)
}SD_ACCESS_MODE;

typedef enum
{
	SD_DET_PIN_STANDARD,	//starndard card detect pin
	SD_DET_PIN_CUST			//customerize card detect pin
}SD_DETECT_PIN_MODE;

typedef uint32_t(*SD_Get1msHook)(void);
typedef struct
{
	SD_DEVICE_t DevIF;
	uint32_t *ulDetPinAddr;
	uint8_t ubDetPinOfs;
	SD_DETECT_PIN_MODE DetPinMode;
	uint32_t ulBufStartAddr;
	uint32_t ulWatchDogTimeoutVal;	//second
	SD_Get1msHook Get1msHk;
}SD_INIT_INFO_t;
//------------------------------------------------------------------------
#pragma pack(pop)
//------------------------------------------------------------------------
/*!
\brief get sd buffer size
\return(no)
*/
uint32_t ulSD_GetTotalBufSize(void);
//------------------------------------------------------------------------
/*!
\brief Initial SD
\return(no)
*/
void SD_Init(SD_INIT_INFO_t info);

//------------------------------------------------------------------------
/*!
\brief Get SD interface number
\return SD_1 or SD_2
*/
SD_DEVICE_t tSD_GetDevIF(void);

//------------------------------------------------------------------------
/*!
\brief Check SD interface setup result
\return 0->Not ready
        1->Ready
*/
uint8_t ubSD_ChkIFSetup(void);
//------------------------------------------------------------------------
/*!
\brief Check SD interface setup result
\return 0->Not ready
        1->Ready
*/
void SD_SetClockAPI(uint16_t uwClk);
//------------------------------------------------------------------------
/*!
\brief Identify SD
\return 0->Fail\n
		1->Pass	
*/
uint8_t ubSD_Identify(SD_DEVICE_t tSD_Dev);

//------------------------------------------------------------------------------
#if SD_GET_AU_SIZE_EN
/*!
\brief get AU size
\return AU size
*/
uint32_t ulSD_GetAUSize(SD_DEVICE_t tSD_Dev);
#endif
//------------------------------------------------------------------------------
/*!
\brief 	Get SD Function Version	
\return	Unsigned short value, high byte is the major version and low byte is the minor version
\par [Example]
\code		 
	 uint16_t uwVer;
	 
	 uwVer = uwSD_GetVersion();
	 printf("SD Version = %d.%d\n", uwVer >> 8, uwVer & 0xFF);
\endcode
*/
uint16_t uwSD_GetVersion(void);

//------------------------------------------------------------------------
/*!
\brief Check SD is card-in or card-out status
\return 0->Card-Out\n
		1->Card-In
*/
uint8_t ubSD_ChkCardIn(SD_DEVICE_t tSD_Dev);

//------------------------------------------------------------------------
/*!
\brief Write data to SD
\param ulSrcAddr	Source address
\param ulSdLBA 	Destination SD LBA
\param ulSize 	Access size
\param ubMode 	0->Without wait access done,1->Wait access done
\return 0->Fail\n
		0xFE->timeout\n
		0xFF->ECC err\n
		1->Pass
*/
uint8_t ubSD_Write(SD_DEVICE_t tSD_Dev,uint32_t ulSrcAddr, uint32_t ulSdLBA, uint32_t ulSize, uint8_t ubMode);

//------------------------------------------------------------------------
/*!
\brief Read data from SD
\param ulDestAddr	Destination address
\param ulSdLBA 	Source SD LBA
\param ulSize 	Access size
\param ubMode 	0->Without wait access done,1->Wait access done
\return 0->Fail\n
		0xFE->timeout\n
		0xFF->ECC err\n
		1->Pass
*/
uint8_t ubSD_Read(SD_DEVICE_t tSD_Dev,uint32_t ulDestAddr, uint32_t ulSdLBA, uint32_t ulSize, uint8_t ubMode);

//------------------------------------------------------------------------
/*!
\brief Get SD total sectors
\return Total sectors		
*/
uint32_t ulSD_GetTotalSector(SD_DEVICE_t tSD_Dev);

#endif

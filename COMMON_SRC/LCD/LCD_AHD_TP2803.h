/*!
	\file		AHD TP2803.h
	\brief		AHD TP2803 Funcation Header
	\author		
	\version	
	\date		
	\copyright	
*/
//------------------------------------------------------------------------------
#ifndef _AHD_TP2803_H_
#define _AHD_TP2803_H_
#include <stdbool.h>
#include "_510PF.h"
//------------------------------------------------------------------------------
// Sensor TP2803
// SLAVE_ID is 0x44 [1:7] + [0] r/w
// I2C slave ID can be programmed as 88/89 or 8A/8B for write and read.
#define AHD_SLAVE_ADDR				0x45
//------------------------------------------------------------------------------
#define		TP2803_RESET_PADIO			GLB->PADIO17
#define		TP2803_RESET_OUT_EN			GPIO->GPIO_OE3
#define		TP2803_RESET_OUT			GPIO->GPIO_O3
#define		TP2803_I2C_CLK_PADIO		GLB->PADIO13
#define		TP2803_I2C_SDA_PADIO		GLB->PADIO14

#define		TP2803_CHIP_ID_HIGH_ADDR   (0xFE)
#define 	TP2803_CHIP_ID_LOW_ADDR    (0xFF)
#define 	TP2803_CHIP_ID             (0x2910)
//------------------------------------------------------------------------------
void AHD_TP2803_Start(void);
bool bAHD_TP2803_IturBt656 (void);
#endif

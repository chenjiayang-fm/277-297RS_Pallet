/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		OSD1Image_Table.h
	\brief		OSD1 Image Table
	\author		Hanyi Chiu
	\version	1
	\date		2016/09/20
	\copyright	Copyright (C) 2016 SONiX Technology Co., Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------

//!           Index 	  	  		  SF Address     Pos X     Pos Y
OSD1IMGPOOL(IMG,	  					0	  ,         0,       0)
OSD1IMGPOOL(ANT5,						0x2000,         0,       0)
OSD1IMGPOOL(ANT4,						0x2000,         0,       0)
OSD1IMGPOOL(ANT3,						0x2000,         0,       0)
OSD1IMGPOOL(ANT2,						0x2000,         0,       0)
OSD1IMGPOOL(ANT1,						0x2000,         0,       0)
OSD1IMGPOOL(ANT0,						0x2000,         0,       0)
OSD1IMGPOOL(REC,						0x2000,         0,       0)
OSD1IMGPOOL(NUM0,	        			0,        	   0,        0)
OSD1IMGPOOL(NUM1,        				0x1000,        0,        0)
OSD1IMGPOOL(NUM2,	     				0x2000,        0,        0)
OSD1IMGPOOL(NUM3,						0x2000,        0,        0)
OSD1IMGPOOL(NUM4,		 				0x2000,        0,        0)
OSD1IMGPOOL(NUM5,			 			0x2000,        0,        0)
OSD1IMGPOOL(NUM6,			 			0x2000,        0,        0)
OSD1IMGPOOL(NUM7,			 			0x2000,        0,        0)
OSD1IMGPOOL(NUM8,		 				0x2000,        0,      620)
OSD1IMGPOOL(NUM9,		 				0x2000,        0,      620)
OSD1IMGPOOL(SLASH,						0x2000,         0,       0)
OSD1IMGPOOL(COLON,						0x2000,         0,       0)
OSD1IMGPOOL(INDEX,						0x2000,         0,       0)
OSD1IMGPOOL(AHD,						0x2000,         0,       0)
OSD1IMGPOOL(TVI,						0x2000,         0,       0)
OSD1IMGPOOL(CVI,						0x2000,         0,       0)
OSD1IMGPOOL(PAL,						0x2000,         0,       0)
OSD1IMGPOOL(NTSC,						0x2000,         0,       0)
OSD1IMGPOOL(720P,						0x2000,         0,       0)
OSD1IMGPOOL(1080P,						0x2000,         0,       0)
OSD1IMGPOOL(25,							0x2000,         0,       0)
OSD1IMGPOOL(30,							0x2000,         0,       0)

#undef OSD1IMGPOOL

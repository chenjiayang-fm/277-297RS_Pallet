/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		OSDLogo_Table_AHD.h
	\brief		OSD Logo Table for AHD
	\author		Hanyi Chiu
	\version	1
	\date		2019/11/20
	\copyright	Copyright (C) 2017 SONiX Technology Co., Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------

//!           Index 	  	  		  SF Address     Pos X     Pos Y
OSDLOGOPOOL(MUTE, 					0      )
OSDLOGOPOOL(LOSTLINK,  				0x3000 )
OSDLOGOPOOL(LOSTLINK_VGA,			0x3000 )

#undef OSDLOGOPOOL

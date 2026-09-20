/*!

	\file			RTC676x_SPI.h
	\brief		SPI header file for Richwave RTC676x serial RF chip
	\author		Bruce Cheng
	\version		0.1
	\date		2017/10/30
	\copyright	Copyright (C) 2017 Richwave Co., Ltd. All rights reserved.
*/

#if !defined(RTC676x_SPI_H)
#define		RTC676x_SPI_H

#include "_510PF.h"
#include "RTC676x_API.h"

#if defined(RTC6765_ASIC)
#define	RTC676x_6765
#endif

//------------------------------------------------------------------------------
typedef enum
{
    SPI_SLAVE = 1,
    SPI_MASTER = 3
} SPI_MODE_t;

typedef void (*SPI_SlaveHook) (uint8_t ubDataLen);
typedef void (*SPI_DmaEndHook) (void);

typedef struct
{
    uint8_t         ubSPI_CPOL:1;                       //!< SPI Clock Polarity
    uint8_t         ubSPI_CPHA:1;                       //!< SPI Clock Phase
    SPI_MODE_t      tSPI_Mode;                          //!< Master/Slave Mode
    uint16_t        uwClkDiv;                           //!< SPI Clock Divider, SPI_CLK = PCLK / (2* (uwSPI_ClkDiv+1))
    SPI_SlaveHook   pfSlaveHook;
    SPI_DmaEndHook  pfDmaEndHook;
} SPI_Setup_t;

void RTC676x_SPI_Init(SPI_Setup_t* setup);

#define		RTC676x_DEBUG_MODE

//------------------------------------------------------------------------------

#define		RTC676x_REG_STATUS_LENGTH		1
#define		RTC676x_REG_TIM_STATIS_LENGTH	1
#if defined(RTC676x_6765)
#define		RTC676x_REG_CONFIG3_LENGTH		2
#else
#define		RTC676x_REG_CONFIG3_LENGTH		1
#endif
#define		RTC676x_REG_DBG_LENGTH			4
#define		RTC676x_REG_DBGCONF_LENGTH		1

//------------------------------------------------------------------------------

#define		RTC676x_REG_CONFIG0			0x00
#define		RTC676x_REG_CONFIG2			0x01
#define		RTC676x_REG_STATUS			0x02
#define		RTC676x_REG_W_BB_ADDR		0x0E
#define		RTC676x_REG_R_BB_ADDR		0x0F
#define		RTC676x_REG_D_SLOT			0x07
#define		RTC676x_REG_D_EVEN_SLOT		0x08
#define		RTC676x_REG_OP_MODE			0x09
#define		RTC676x_REG_BB_DATA			0x10
#define		RTC676x_REG_TIMER_EN		0x12
#define		RTC676x_REG_P_LISTEN		0x14
#define		RTC676x_REG_TIM_STATUS		0x13
#define		RTC676x_REG_TX_FIFO			0x1F
#define		RTC676x_REG_RX_FIFO			0x20
#define		RTC676x_REG_W_RF_ADDR		0x22
#define		RTC676x_REG_R_RF_ADDR		0x23
#define		RTC676x_REG_RF_DATA1		0x24
#define		RTC676x_REG_CONFIG5			0x25
#define		RTC676x_REG_PMU_ADDR		0x32
#define		RTC676x_REG_PMU_DATA		0x33
#define		RTC676x_REG_CONFIG3			0x35
#define		RTC676x_REG_CONFIG4			0x36
#define		RTC676x_REG_DBGCONF			0x7C

// RF_Status
#define		RTC676x_REG_STATUS_TSLOT_END		(0x1 << 7)
#define		RTC676x_REG_STATUS_BCPB_SYNC		(0x1 << 6)
#define		RTC676x_REG_STATUS_BB_BUSY			(0x1 << 5)
#define		RTC676x_REG_STATUS_RF_BUSY			(0x1 << 4)
#define		RTC676x_REG_STATUS_RX_DR			(0x1 << 3)
#define		RTC676x_REG_STATUS_TX_DS			(0x1 << 2)
#define		RTC676x_REG_STATUS_MAX_RT			(0x1 << 1)
#define		RTC676x_REG_STATUS_TX_NS			(0x1 << 0)

// TIMER_Enable
#define		RTC676x_REG_TIMER_EN_INQR_TO		(0x1 << 6)
#define		RTC676x_REG_TIMER_EN_PB_TO			(0x1 << 5)
#define		RTC676x_REG_TIMER_EN_BEACON_TO		(0x1 << 3)
#define 	RTC676x_REG_TIMER_EN_NEW_CONN_TO	(0x1 << 2)
#define		RTC676x_REG_TIMER_EN_OCF_TO			(0x1 << 1)
#define		RTC676x_REG_TIMER_EN_LNK_TO			(0x1 << 0)

// Tim_Status
#define		RTC676x_REG_TIM_STATUS_BEACON_EXP	(0x1 << 7)
#define		RTC676x_REG_TIM_STATUS_NEWCONN_EXP	(0x1 << 6)
#define		RTC676x_REG_TIM_STATUS_OCF_EXP		(0x1 << 5)
#define		RTC676x_REG_TIM_STATUS_LNK_EXP		(0x1 << 4)
#define		RTC676x_REG_TIM_STATUS_WAKE			(0x1 << 0)

// Config2
#define		RTC676x_REG_CONFIG2_BB_RESET		(0x1 << 5)
#define		RTC676x_REG_CONFIG2_MAC_RESET		(0x1 << 4)
#define		RTC676x_REG_CONFIG2_SW_HOP_EN		(0x1 << 2)

// Config3
#if defined(RTC676x_6765)
#define		RTC676x_REG_CONFIG3_TXF0_EMPTY		(0x1 << 8)
#define		RTC676x_REG_CONFIG3_TXF1_EMPTY		(0x1 << 7)
#define		RTC676x_REG_CONFIG3_RXF0_EMPTY		(0x1 << 4)
#define		RTC676x_REG_CONFIG3_RXF1_EMPTY		(0x1 << 3)
#else /*6763*/
#define		RTC676x_REG_CONFIG3_SF_EMPTY		(0x1 << 2)
#define		RTC676x_REG_CONFIG3_BF_EMPTY		(0x1 << 1)
#endif
#define		RTC676x_REG_CONFIG3_LOSS_LINK		(0x1 << 0)

// Config4
#define		RTC676x_REG_CONFIG4_BF_TX			(0x1 << 3)

// OP mode
#define		RTC676x_REG_OP_MODE_NORMAL			(0x0)
#define		RTC676x_REG_OP_MODE_4_DIV			(0x4)

// Dbg_Conf
#define		RTC676x_REG_DBG_CONF_SW_SFCLR			(0x1 << 5)
#define		RTC676x_REG_DBG_CONF_SW_BFCLR			(0x1 << 4)

#define		RTC676x_SPI_REG_GET			0x00000008
#define		RTC676x_SPI_REG_SET			0x00000010
#define		RTC676x_SPI_TX_PACKET		0x00000020
#define		RTC676x_SPI_RX_PACKET		0x00000040
#define		RTC676x_SPI_RF_RESET		0x00000080
#define		RTC676x_SPI_RF_INTERRUPT	0x00010000
#define		RTC676x_SPI_RF_CHECK_INT	0x00020000

#define		RTC676x_SPI_RETURN_REG		0x00000100

#if defined(RTC676x_DEBUG_MODE)
#define		RTC676x_SPI_DEBUG			0x00008000
#define			RTC676x_SPI_DEBUG_CS_GET	0x01
#define			RTC676x_SPI_DEBUG_CS_SET	0x02
#define			RTC676x_SPI_DEBUG_INT_GET	0x03
#define			RTC676x_SPI_DEBUG_INT_SET	0x04
#define			RTC676x_SPI_DEBUG_RESET_GET	0x05
#define			RTC676x_SPI_DEBUG_RESET_SET	0x06
#define			RTC676x_SPI_DEBUG_DMA_TRANS_PKT	0x07
#define			RTC676x_SPI_DEBUG_ADD_REQ_TO_NA_WAIT	0x08
#define			RTC676x_SPI_DEBUG_PUT_NA_EVENT			0x09
#define			RTC676x_SPI_DEBUG_DMA_WRITE_TEST		0x0A
#define			RTC676x_SPI_DEBUG_DMA_READ_TEST			0x0B
#endif /* RTC676x_DEBUG_MODE */

#define		RTC676x_SPI_REG_LOWERMAC	0x01
#define		RTC676x_SPI_REG_BASEBAND	0x02
#define		RTC676x_SPI_REG_RF			0x04
#define		RTC676x_SPI_REG_PMU			0x08

#define		RTC676x_Vector_Header_Size	10
#define		RTC676x_Mac_Header_Size		14
#define		RTC676x_Vector_Mac_Size		(RTC676x_Vector_Header_Size + RTC676x_Mac_Header_Size)

typedef enum {
	PKT_TYPE_MGMT = 0,
	PKT_TYPE_SMSG,
	PKT_TYPE_VIDEO,
	PKT_TYPE_AUDIO,
	PKT_TYPE_MAX
}RTC676x_PKT_TYPE;

typedef enum {
	MAC_PKT_BEACON				= 0,
	MAC_PKT_PROBE				= 1,
	MAC_PKT_PBACK				= 2,
	MAC_PKT_CMD					= 5,
	MAC_PKT_CMD_ACK				= 6,
	MAC_PKT_INTR				= 7,
	MAC_PKT_WAKE				= 8,
	MAC_PKT_VIDEO				= 9,
	MAC_PKT_VIDEO_ACK			= 10,
	MAC_PKT_AUDIO				= 11,
	MAC_PKT_AUDIO_ACK			= 12,
	MAC_PKT_SMSG				= 13,
	MAC_PKT_SMS_ACK				= 14,
	MAC_PKT_RESET_ALL_BUFFERS	= 253
}RTC676x_MAC_PACKET_TYPE_LIST;

typedef enum {
    I_AM_SLAVE = 0,
    I_AM_MASTER = 1,
    I_AM_SNIFFER = 2,
    I_AM_UNDEFINED = 3
} DAV676X_MAC_IDENTITY;

//#if !defined(VBM_BU) && !defined(VBM_PU)
//#err No Target Definition
//#else
//#endif

void RTC676x_Get_CS(void);
void RTC676x_Set_CS(uint8_t value);
void RTC676x_Get_Reset(void);
void RTC676x_Set_Reset(uint8_t value);
void RTC676x_Get_Interrupt(void);
void RTC676x_Set_Interrupt(uint8_t value);

uint64_t udlRTC676x_GetIsrCnt(void);


#endif /*RTC676x_SPI_H*/

/*!

	\file			RTC676x_API.h
	\brief		API header file for Richwave RTC676x serial RF chip
	\author		Bruce Cheng
	\version		0.1
	\date		2018/01/25
	\copyright	Copyright (C) 2017 Richwave Co., Ltd. All rights reserved.
*/

#if !defined(RTC676x_API_H)
#define		RTC676x_API_H

#include <stdbool.h>
#include <stdint.h>

#define		RTC676x_CHECK_CRC

typedef enum
{
	RATE_1000_HOPS_PER_SECOND	= 3000,
	RATE_900_HOPS_PER_SECOND	= 3333,
	RATE_600_HOPS_PER_SECOND	= 5000,
} RTC676x_HOPPING_RATE;

/*
 *  16Qam	: Preferred Rate > Balance > Preferred Distance(Qpsk inly, 16Qam is the same as Qpsk)
 *  Qpsk	: Preferred Distance > Balance > Preferred Rate
 */
typedef enum
{
	PREFERRED_RATE_BOTH_16QAM_AND_QPSK,
	BALANCE_BOTH_16QAM_AND_QPSK,
	PREFERRED_DISTANCE_QPSK_ONLY,
    PREFERRED_LONG_DISTANCE_BPSK_ONLY_MORE_BYTES_R2T, /* Make sure Rx to Tx MPSK mode is ok, BUT_LESS_BYTES_T2R */
	PREFERRED_LONG_DISTANCE_BPSK_ONLY, /* Tx to Rx priority, Rx to Tx BPSK mode may be invalid (length 0) */
} RTC676x_BANDWIDTH_MODE;

/*														600 hops		900 hops		1000 hops
 *
 * PREFERRED_RATE_BOTH_16QAM_AND_QPSK					support			not support		not support
 * BALANCE_BOTH_16QAM_AND_QPSK							support			not support		not support
 * PREFERRED_DISTANCE_QPSK_ONLY							support			not support		not support
 * PREFERRED_LONG_DISTANCE_BPSK_ONLY_MORE_BYTES_R2T		support			support			not support
 * PREFERRED_LONG_DISTANCE_BPSK_ONLY					support			support			support
 *
 */

enum {
	TYPE_MGMT,
	TYPE_SMSG,
	TYPE_VIDEO,
	TYPE_AUDIO,
};

typedef enum {
	PRIORITY_HIGH	= 0,
	PRIORITY_MIDDLE	= 1,
	PRIORITY_LOW	= 2,
}RTC676x_PKT_PRIORITY;

#if defined(RTC676x_CHECK_CRC)
typedef enum {
	CRC_DONT_CHECK		= 0,
	CRC_CHECK_AND_REPORT= 1,
	CRC_CHECK_AND_DROP	= 2,
	CRC_CHECK_ONLY		= 3,
}RTC676x_CHECK_CRC_MODE;
#endif

typedef struct
{
	RTC676x_BANDWIDTH_MODE	slBandwidthMode;						/*input*/
	uint32_t				ulWholeSlot;							/*input*/
	int32_t					slMacIdentify;							/*input DAV676X_MAC_IDENTITY*/
	uint32_t				ulEvenSlot;								/*output*/
	uint32_t				ulBpskLength;							/*output*/
	uint32_t				ulBpskLengthHalfOrQuarterBandwidth;		/*output*/
	uint32_t				ulQpskLength;							/*output*/
	uint32_t				ulQpskLengthHalfOrQuarterBandwidth;		/*output*/
	uint32_t				ul16QamLengthFullBandwidth;				/*output*/
	uint32_t				ul16QamLengthHalfOrQuarterBandwidth;	/*output*/
} RTC676x_PKT_SIZE_SUPPORT_CMD;

typedef struct
{
	uint32_t		ulTimeoutMsec;
	uint32_t		ulTxHeaderLen;
	uint32_t		ulTxDataLen;
	uint32_t		ulNeedAck;
	uint32_t		ulPackedLen;
	void*			pbVectorMac;
	void*			pbHeader;
	void*			pbBuffer;
	int32_t			slIsTimeoutFlag;
	int32_t			slIsAck;
	int32_t			slBBR;
	int32_t			slRetry;
	uint32_t		ulAck0_31;
	uint32_t		ulAck32_63;
	uint32_t		ulAckMask0_31;
	uint32_t		ulAckMask32_63;
//	uint8_t			ubReverse;
} RTC676x_Tx_Cmd;

typedef struct
{
	uint32_t		ulTimeoutMsec;
	uint32_t		ulRxDataLen;
	uint8_t*		pbBuffer;
	int32_t			slIsTimeoutFlag;
	int32_t			slIsOverrun;
#if defined(RTC676x_CHECK_CRC)
	int32_t			slIsCrcError; //only used when CRC_CHECK_ONLY mode, 0 : ok, 1 : this packet CRC is wront
#endif
} RTC676x_Rx_Cmd;

void RTC676x_Reset(void);
/*
 * to get the value of even slot and packet sizes for each BBR
 * depend on Identify(Master or Slave), WholeSlot(ex: 5000 for 600 heart beat/s),
 * and BandwidthMode(preferred rate, distance, or balance)
 */
int RTC676x_Packet_Size_Support_Get(RTC676x_PKT_SIZE_SUPPORT_CMD *pkt_size);
int RTC676x_Lowermac_Reg_Write(uint8_t ubRegOffset, uint32_t ulRegData, uint8_t ubRegLength);
int RTC676x_RF_Reg_Write(uint8_t ubRegOffset, uint32_t ulRegData, uint8_t ubRegLength);
int RTC676x_Baseband_Reg_Write(uint8_t ubRegOffset, uint32_t ulRegData, uint8_t ubRegLength);
int RTC676x_PMU_Reg_Write(uint8_t ubRegOffset, uint32_t ulRegData, uint8_t ubRegLength);
uint32_t RTC676x_Lowermac_Reg_Read(uint8_t ubRegOffset, uint8_t ubRegLength);
uint32_t RTC676x_RF_Reg_Read(uint8_t ubRegOffset, uint8_t ubRegLength);
uint32_t RTC676x_Baseband_Reg_Read(uint8_t ubRegOffset, uint8_t ubRegLength);
uint32_t RTC676x_PMU_Reg_Read(uint8_t ubRegOffset, uint8_t ubRegLength);
void RTC676x_Wait_Beacon_Timeout(void);
int RTC676x_Tx_MGMT_Pkt(RTC676x_Tx_Cmd *tx_cmd);
int RTC676x_Tx_SMSG_Pkt(RTC676x_Tx_Cmd *tx_cmd);
int RTC676x_Tx_VIDEO_Pkt(RTC676x_Tx_Cmd *tx_cmd);
int RTC676x_Tx_AUDIO_Pkt(RTC676x_Tx_Cmd *tx_cmd);
int RTC676x_Get_MGMT_Ack_List(uint64_t* plAck);
int RTC676x_Get_SMSG_Ack_List(uint64_t* plAck);
int RTC676x_Get_VIDEO_Ack_List(uint64_t* plAck);
int RTC676x_Get_AUDIO_Ack_List(uint64_t* plAck);
int RTC676x_Rx_MGMT_Pkt(RTC676x_Rx_Cmd *rx_cmd);
int RTC676x_Rx_SMSG_Pkt(RTC676x_Rx_Cmd *rx_cmd);
int RTC676x_Rx_VIDEO_Pkt(RTC676x_Rx_Cmd *rx_cmd);
int RTC676x_Rx_AUDIO_Pkt(RTC676x_Rx_Cmd *rx_cmd);

void RTC676x_Register_MGMT_Pkt_Arrive_Signal(void *thread_id, int32_t signal);

uint64_t RTC676x_Get_Last_ACK_Timestamp(void);
uint64_t RTC676x_GetTXPackets(int type);
uint64_t RTC676x_GetRXPackets(int type);
uint64_t RTC676x_GetACKPackets(int type);
uint64_t RTC676x_GetNACKPackets(int type);
uint64_t RTC676x_GetTXBytes(int type);
uint64_t RTC676x_GetRXBytes(int type);
uint64_t RTC676x_GetACKBytes(int type);
void RTC676x_Stop_Transmit_and_wait_clear(void);

uint32_t RTC676x_DMA_Transmit_Buffer_Get_Total_Number(void);
uint32_t RTC676x_DMA_Transmit_Buffer_Get_Free_Number(void);

void RTC676x_Transmit_Priority_Get(RTC676x_PKT_PRIORITY *messagePriority, RTC676x_PKT_PRIORITY *audioPriority, RTC676x_PKT_PRIORITY *videoPriority);
int RTC676x_Transmit_Priority_Set(RTC676x_PKT_PRIORITY messagePriority, RTC676x_PKT_PRIORITY audioPriority, RTC676x_PKT_PRIORITY videoPriority);

void RTC676x_Debug_Print_Enable(bool enable);

#if defined(RTC676x_CHECK_CRC)
RTC676x_CHECK_CRC_MODE RTC676x_CRC_Check_Mode_Get(void);
int RTC676x_CRC_Check_Mode_Set(RTC676x_CHECK_CRC_MODE mode);
#endif

#endif /*RTC676x_API_H*/

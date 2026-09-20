/*!


	\file			RTC676x_SPI.c
	\brief		SPI (Serial Peripheral Interface) function for Richwave RTC676x serial RF chip
	\author		Bruce Cheng
	\version		0.1
	\date		2017/10/30
	\copyright	Copyright (C) 2017 Richwave Co., Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "RTC676x_SPI.h"
#include "APBC.h"
#include "INTC.h"
#include "TIMER.h"
#include "BUF.h"

#include "WDT.h"
//------------------------------------------------------------------------------

#if defined(RTC676x_6765)
#define	RTC676x_ISR_RESEND_EVENT_WHEN_DMA_DOING
#endif
#define	RTC676x_TRANSMIT_REQUEST_MULTI_Q
#if defined(RTC676x_TRANSMIT_REQUEST_MULTI_Q)
#define	RTC676x_USER_DEFINE_TRANSMIT_QUEUE_PRIORIT
#endif
// #define	RTC676x_WAIT_ACK_BY_EVENT
#define	RTC676x_WAIT_RETURN_BY_EVENT

#if !defined(RTC676x_6765)
#define	RTC676x_CLEAR_RECV_FIFO
#endif

#define	RTC676x_CHECK_INTERRUPT_STOP
//#define	RTC676x_SPI_DMA_RW_ONLY
#define	RTC676x_USING_UNCACHED_MALLOC
#define	RTC676x_DMA_QUEUE_UNCACHED_MALLOC
#define	RTC676x_DMA_QUEUE_OS_MSG_Q_MGMT
#define		RTC676x_CHECK_TIMER 1000
#define		RTC676x_CHECK_TIMER_SLAVE 50
//#define		RTC676x_DEBUG_TIMER 5
#define		RTC676x_DEBUG_CHECK_RUNNING

//#define	RTC676x_SPI_MAC_READ_PRE_DO		23

typedef enum
{
    SPI_WaitReady,
    SPI_DontWait
} SPI_WaitMode_t;

#define RTC676x_SPI_MAJOR_VER    0           // Major version
#define RTC676x_SPI_MINOR_VER    1           // Minor version

#if defined(__CC_ARM)
  //#pragma push
  #pragma anon_unions
#else
  #warning Not supported compiler type
#endif

#define	RTC676x_WAIT_SET_REG_DONE_EVENT		0x02000000
#define	RTC676x_WAIT_GET_REG_RETURN_EVENT	0x04000000
#define	RTC676x_WAIT_TRANSMIT_RETURN_EVENT	0x08000000
#define	RTC676x_WAIT_ACK_MGMT_EVENT			0x10000000
#define	RTC676x_WAIT_ACK_SMSG_EVENT			0x20000000
#define	RTC676x_WAIT_ACK_VIDEO_EVENT		0x40000000
#define	RTC676x_WAIT_ACK_AUDIO_EVENT		0x80000000

#define RTC676x_SPI_SLAVE_GOT_DATA  0x00000002
#define RTC676x_SPI_TX_DMA_END      0x00000004

#define	RTC676x_SPI_REG_OFFSET_READ_BIT	0x80

#define	RTC676x_SPI_PACKET_BIG_SIZE		1792
#define	RTC676x_SPI_PACKET_SMALL_SIZE	640
#define	RTC676x_SPI_PACKET_BIG_NUMBER	16 /*128*/
#define	RTC676x_SPI_PACKET_SMALL_NUMBER	16 /*128*/

//#if defined(VBM_PU)   /*LCD : RX*/
#define	RTC676x_RECV_BIG_PACKET_SIZE		1792
//#else
#define	RTC676x_RECV_SMALL_PACKET_SIZE		896
//#endif
uint32_t RTC676x_RECV_PACKET_SIZE = 0;

typedef enum
{
	RTC676x_RF_Slave = 0,
	RTC676x_RF_Master
}RTC676x_RF_Device;

struct tx_cmd
{
	uint32_t	ulTimeoutMs;
	uint32_t	ulHeaderLen;
	uint32_t	ulDataLen;
	uint8_t*	pbVectorMac;
	uint8_t*	pbHeader;
	uint8_t*	pbData;
	uint8_t		ubNeedAck;
	uint8_t		ubBBR;
	uint8_t		ubRetry;
	uint8_t		ubPktType;

	uint32_t	ulAck0_31;
	uint32_t	ulAck32_63;
	uint32_t	ulAckMask0_31;
	uint32_t	ulAckMask32_63;
};

struct rx_cmd
{
	uint32_t	ulTimeoutMs;
	uint32_t	ulDataLen;
	uint8_t*	pbData;
	uint8_t		ubIsTimeout;
	uint8_t		ubIsOverrun;
	uint8_t		ubReverse[17];
	uint8_t		ubPktType;
};

typedef struct
{
	uint32_t		ulDataLen;
	uint8_t*		pbDataBuf;
	osPoolId		ptDmaPool;
}RTC676x_Recv_Data;

struct tx_return
{
	uint32_t		ulIsTimeout;
};
struct rx_return
{
	uint32_t		ulIsTimeout;
};
struct ack_return
{
	uint32_t		ulACK_0_31;
	uint32_t		ulACL_32_63;
};

struct reg_return
{
	uint32_t		ulRegValue;
};

typedef struct
{
	uint32_t		ulEvent;
	int32_t			ilReturnValue;
	union unionEventReturn{
		struct tx_return	tx_return;
		struct rx_return	rx_return;
		struct ack_return	ack_return;
		struct reg_return	reg_return;
	};
}RTC676x_Return_MEAS;

typedef struct
{
	uint8_t		ubPktData[RTC676x_SPI_PACKET_BIG_SIZE];
}RTC676x_Packet_Big;

typedef struct
{
	uint8_t		ubPktData[RTC676x_SPI_PACKET_SMALL_SIZE];
}RTC676x_Packet_Small;

typedef struct
{
	uint32_t		ulPktType;
	uint32_t		ulTransLen;
	uint8_t*		pbTransBuf;
	osPoolId		ptDmaPool;
	uint32_t		ulNeedAck;
#if defined(RTC676x_WAIT_ACK_BY_EVENT)
	osThreadId		txThreadId;
#endif
	uint32_t		ubPreOffset;
	int32_t			slBBR;
}RTC676x_Transmit_Request_t;

typedef struct
{
	uint8_t			ubDBGEvent;
	uint8_t			ubDBGData;
}RTC676x_Debug_Event_t;

typedef enum
{
	DMA_Trans_D_TX = 0,
	DMA_Trans_D_RX
}RTC676x_DMA_Trans_Direction;

typedef struct
{
	RTC676x_DMA_Trans_Direction		transDirection;
	uint32_t						ulPktType;
	uint32_t						ulPktLen;
	uint8_t*						pbDmaBuf;
	osPoolId						ptDmaPool;
	uint32_t						ubPreOffset;
	uint32_t						ulPreDoOffset;
}RTC676x_DMA_Request_t;

typedef struct
{
	uint32_t		ulEvent;
	union unionEventData{
		uint8_t						ubEventData[32];
		RTC676x_Transmit_Request_t	tx_request;
		RTC676x_Debug_Event_t		pDBGEvent;
	};
}RTC676x_MEAS;

#if defined(RTC676x_DMA_QUEUE_OS_MSG_Q_MGMT)
typedef struct
{
	uint8_t *pubBuffer;
}RTC676x_DMA_BUFFER_MGMT_t;
#endif

//#define	RTC676x_Interrupt_Status_Keep_Q_Max		5

typedef struct
{
	uint8_t Reg_Rf_Status;
	uint8_t Reg_Tim_Status;
#if defined(RTC676x_6765)
	uint16_t Reg_Config3;
#else /*6763*/
	uint8_t Reg_Config3;
#endif

	uint8_t Reg_Config;
	uint8_t Reg_Config2;
	uint16_t Reg_D_SLOT;
	uint16_t Reg_D_EVEN_SLOT;

#if defined(RTC676x_Interrupt_Status_Keep_Q_Max)
	uint8_t	Reg_Interrupt_Log_Index;
	uint8_t Reg_Rf_Status_Log[RTC676x_Interrupt_Status_Keep_Q_Max];
#endif

#if defined(RTC676x_6765)
	uint16_t	Reg_Tx_Fifo_Check_Mask;
	uint16_t	Reg_Rx_Fifo_Check_Mask;
#else /*6763*/
	uint8_t	Reg_Tx_Fifo_Check_Mask;
	uint8_t	Reg_Rx_Fifo_Check_Mask;
#endif
	uint8_t	Reg_Tx_Fifo_Clear_Mask;
	uint8_t	Reg_Rx_Fifo_Clear_Mask;

	uint8_t ubRFTxMode;
	uint8_t ubRFOpMode;

#if defined(RTC676x_CHECK_CRC)
	RTC676x_CHECK_CRC_MODE checkCrcMode;
#endif

	uint8_t ubDmaRunning;
	uint8_t ubNeedGetAck;
	uint8_t ubInqrBeaconMode;

	uint32_t ulCheckTimerInterval;
	RTC676x_DMA_Request_t tDmaRequest;

	uint8_t ubDmaRecvBuf[RTC676x_SPI_PACKET_BIG_SIZE];
	uint8_t	ubDmaDummy[RTC676x_SPI_PACKET_BIG_SIZE];
}RTC676x_Data_t;

static RTC676x_Data_t* pgRtc676xData = NULL;

uint64_t ullRTC676x_ACK_Timestamp = 0;

#if defined(RTC676x_WAIT_ACK_BY_EVENT)
const uint32_t ulWaitEvent[PKT_TYPE_MAX] = {RTC676x_WAIT_ACK_MGMT_EVENT, RTC676x_WAIT_ACK_SMSG_EVENT, RTC676x_WAIT_ACK_VIDEO_EVENT, RTC676x_WAIT_ACK_AUDIO_EVENT};
#endif

uint8_t RTC676x_SPI_TxEndFlag;
uint8_t RTC676x_SPI_RxEndFlag;
uint8_t* pbRTC676x_SPI_RxData;
uint8_t ubRTC676x_SPI_RxDataLen;
uint8_t ubRTC676x_SPI_RxByteWidth;
//SPI_SlaveHook pfRTC676x_SPI_SlaveHook = NULL;
SPI_DmaEndHook pfRTC676x_SPI_DmaEndHook = NULL;
uint8_t ubRTC676x_SPI_WaitEvent = 0;

bool RTC676x_Mac_Wait_Beacon_Timeout_Flag = false;

uint32_t RTC676x_Memory_Poll_Ptr = 0;
uint32_t RTC676x_Memory_Poll_Used = 0;

#if defined(RTC676x_USING_UNCACHED_MALLOC)
uint64_t *RTC676x_Tx_Ack_List;
uint64_t *RTC676x_Tx_Ack_List_Mask;
uint32_t *RTC676x_Tx_Ack_List_Count;

int32_t *RTC676x_Rx_Overrun_State;
#else
uint64_t RTC676x_Tx_Ack_List[PKT_TYPE_MAX] = {0};
uint64_t RTC676x_Tx_Ack_List_Mask[PKT_TYPE_MAX] = {0};
uint32_t RTC676x_Tx_Ack_List_Count[PKT_TYPE_MAX] = {0};

int32_t RTC676x_Rx_Overrun_State[PKT_TYPE_MAX] = {0};
#endif

#if defined(RTC676x_WAIT_RETURN_BY_EVENT)
uint8_t	RTC676x_Reg_Return_Data[12];
#endif

uint8_t *ubRegReadRData, *ubRegReadWData;
uint8_t *ubRegWriteRData, *ubRegWriteWData;

#define	RTC676x_RECV_PRE_HEADER_LEN		7

#if defined(RTC676x_USING_UNCACHED_MALLOC)
uint8_t	*ubRfStatus, *ubRfStatusRead;
uint8_t	*ubTimStatus, *ubTimStatusRead;
#if defined(RTC676x_6765)
uint8_t *ubConfig3, *ubConfig3Read;
#else /*6763*/
uint8_t *ubConfig3, *ubConfig3Read;
#endif
uint8_t	*ubRfStatusClean;
uint8_t	*ubTimStatusClean ;
uint8_t	*ubRegReadDummy;

uint8_t *ubRecvHeader, *ubRecvHeaderRead;
uint8_t *ubDbgConf, *ubDbgConfRead;
#else
uint8_t	ubRfStatus[RTC676x_REG_STATUS_LENGTH + 1] = {RTC676x_REG_STATUS | RTC676x_SPI_REG_OFFSET_READ_BIT, 0}, ubRfStatusRead[2];
uint8_t	ubTimStatus[RTC676x_REG_TIM_STATIS_LENGTH + 1] =  {RTC676x_REG_TIM_STATUS | RTC676x_SPI_REG_OFFSET_READ_BIT, 0}, ubTimStatusRead[2];
#if defined(RTC676x_6765)
uint8_t ubConfig3[RTC676x_REG_CONFIG3_LENGTH + 1] = {RTC676x_REG_CONFIG3 | RTC676x_SPI_REG_OFFSET_READ_BIT, 0, 0}, ubConfig3Read[3];
#else /*6763*/
uint8_t ubConfig3[RTC676x_REG_CONFIG3_LENGTH + 1] = {RTC676x_REG_CONFIG3 | RTC676x_SPI_REG_OFFSET_READ_BIT, 0}, ubConfig3Read[2];
#endif
uint8_t	ubRfStatusClean[RTC676x_REG_STATUS_LENGTH + 1] = {RTC676x_REG_STATUS, 0};
uint8_t	ubTimStatusClean[RTC676x_REG_TIM_STATIS_LENGTH + 1] =  {RTC676x_REG_TIM_STATUS, 0};
uint8_t	ubRegReadDummy[2] = {0};

uint8_t ubRecvHeader[RTC676x_RECV_PRE_HEADER_LEN], ubRecvHeaderRead[RTC676x_RECV_PRE_HEADER_LEN];
uint8_t ubDbgConf[RTC676x_REG_DBGCONF_LENGTH + 1] = {0}, ubDbgConfRead[RTC676x_REG_DBGCONF_LENGTH + 1] = {0};
#endif

osThreadId mgmtPacketArriveSignalThreadId = 0;
uint32_t mgmtPacketArriveSignal = 0;

osThreadId linkStateResetSignalThreadId = 0;
uint32_t linkStateResetSignal = 0;

int RTC676x_Transmit_Request(uint32_t ulPktType, RTC676x_Tx_Cmd *tx_cmd);

void RTC676x_SPI_CpuRW(uint8_t* pbTxData, uint8_t* pbRxData, uint8_t ubDataLen, uint8_t ubDataWidth);
void RTC676x_ISR(void);
void RTC676x_SPI_Thread(void const *argument);
void RTC676x_SPI_DmaEnd(void);
void RTC676x_SPI_DmaEnd2(void);
osThreadId RTC676x_SPI_ThreadId = NULL;
//#if defined(VBM_PU)   /*LCD : RX*/

//osThreadDef(RTC676x_SPI_SlavePolling, RTC676x_SPI_Thread, osPriorityRealtime, 1, 512);
osThreadDef(RTC676x_SPI_SlavePolling, RTC676x_SPI_Thread, osPriorityRealtime, 1, 2048);

//#else
//osThreadDef(RTC676x_SPI_SlavePolling, RTC676x_SPI_Thread, osPriorityRealtime, 1, 512);
//#endif
void RTC676x_GPIO_Init(void);
//void RTC676x_DMA_Buffer_Init(void);
int RTC676x_DMA_Buffer_Init(uint32_t poll_ptr, RTC676x_RF_Device tRFDevice);
void RTC676x_RF_ISR(void);
void RTC676x_SPI_DmaEnd_CallBack(void);
int RTC676x_Recv_Queue_Init(uint32_t poll_str);

// Pool
#if !defined(RTC676x_DMA_QUEUE_UNCACHED_MALLOC)
osPoolId RTC676x_Pkt_Big_Size_Poll = NULL;
osPoolDef(RTC676x_Pkt_Big_Size_Poll, RTC676x_SPI_PACKET_BIG_NUMBER, RTC676x_Packet_Big);

osPoolId RTC676x_Pkt_Small_Size_Poll = NULL;
osPoolDef(RTC676x_Pkt_Small_Size_Poll, RTC676x_SPI_PACKET_SMALL_NUMBER, RTC676x_Packet_Small);

osPoolId RTC676x_Pkt_Recv_Poll = NULL, RTC676x_Pkt_Trans_Poll = NULL;
#endif

// Message Queue
osMessageQId RTC676x_Msg_Q = NULL;
osMessageQDef(RTC676x_Msg_Q, 16, RTC676x_MEAS);

#if !defined(RTC676x_WAIT_RETURN_BY_EVENT)
osMessageQId RTC676x_Reg_Return_Msg_Q = NULL;
osMessageQDef(RTC676x_Reg_Return_Msg_Q, 1, RTC676x_Return_MEAS);
#endif

#if defined(RTC676x_TRANSMIT_REQUEST_MULTI_Q)
#if defined(RTC676x_USER_DEFINE_TRANSMIT_QUEUE_PRIORIT)
osMessageQId RTC676x_Tx_Request_MGMT_Q = NULL;
osMessageQDef(RTC676x_Tx_Request_MGMT_Q, 16, RTC676x_Transmit_Request_t);

osMessageQId RTC676x_Tx_Request_Priority_High_Q = NULL;
osMessageQDef(RTC676x_Tx_Request_Priority_High_Q, 96, RTC676x_Transmit_Request_t);

osMessageQId RTC676x_Tx_Request_Priority_Middle_Q = NULL;
osMessageQDef(RTC676x_Tx_Request_Priority_Middle_Q, 96, RTC676x_Transmit_Request_t);

osMessageQId RTC676x_Tx_Request_Priority_Low_Q = NULL;
osMessageQDef(RTC676x_Tx_Request_Priority_Low_Q, 96, RTC676x_Transmit_Request_t);
#else
osMessageQId RTC676x_Tx_Request_MGMT_Q = NULL;
osMessageQDef(RTC676x_Tx_Request_MGMT_Q, 64, RTC676x_Transmit_Request_t);

osMessageQId RTC676x_Tx_Request_SMSG_Q = NULL;
osMessageQDef(RTC676x_Tx_Request_SMSG_Q, 64, RTC676x_Transmit_Request_t);

osMessageQId RTC676x_Tx_Request_VIDEO_Q = NULL;
osMessageQDef(RTC676x_Tx_Request_VIDEO_Q, 64, RTC676x_Transmit_Request_t);

osMessageQId RTC676x_Tx_Request_AUDIO_Q = NULL;
osMessageQDef(RTC676x_Tx_Request_AUDIO_Q, 64, RTC676x_Transmit_Request_t);
#endif

#if defined(RTC676x_USING_UNCACHED_MALLOC)
osMessageQId *RTC676x_Tx_Request_Q;
#else
osMessageQId RTC676x_Tx_Request_Q[PKT_TYPE_MAX] = {NULL};
#endif
#else /* RTC676x_TRANSMIT_REQUEST_MULTI_Q */
osMessageQId RTC676x_Tx_Request_Q = NULL;
osMessageQDef(RTC676x_Tx_Request_Q, 128, RTC676x_Transmit_Request_t);
#endif

osMessageQId RTC676x_Tx_Wait_Ack_Q = NULL;
osMessageQDef(RTC676x_Tx_Wait_Ack_Q, 64, RTC676x_Transmit_Request_t);

osMessageQId RTC676x_DMA_Request_Q = NULL;
osMessageQDef(RTC676x_DMA_Request_Q, 8, RTC676x_DMA_Request_t);

osMessageQId RTC676x_Recv_MGMT_Q = NULL;
osMessageQDef(RTC676x_Recv_MGMT_Q, 64, RTC676x_Recv_Data);

osMessageQId RTC676x_Recv_SMSG_Q = NULL;
osMessageQDef(RTC676x_Recv_SMSG_Q, 64, RTC676x_Recv_Data);

osMessageQId RTC676x_Recv_VIDEO_Q = NULL;
osMessageQDef(RTC676x_Recv_VIDEO_Q, 64, RTC676x_Recv_Data);

osMessageQId RTC676x_Recv_AUDIO_Q = NULL;
osMessageQDef(RTC676x_Recv_AUDIO_Q, 64, RTC676x_Recv_Data);

// Semaphore
osSemaphoreId RTC676x_Reg_RW_Sema;
osSemaphoreDef(RTC676x_Reg_RW_Sema);

#if !defined(RTC676x_WAIT_ACK_BY_EVENT)
osSemaphoreId RTC676x_Wait_MGMT_Ack_Sem;
osSemaphoreDef(RTC676x_Wait_MGMT_Ack_Sem);

osSemaphoreId RTC676x_Wait_SMSG_Ack_Sem;
osSemaphoreDef(RTC676x_Wait_SMSG_Ack_Sem);

osSemaphoreId RTC676x_Wait_VIDEO_Ack_Sem;
osSemaphoreDef(RTC676x_Wait_VIDEO_Ack_Sem);

osSemaphoreId RTC676x_Wait_AUDIO_Ack_Sem;
osSemaphoreDef(RTC676x_Wait_AUDIO_Ack_Sem);

#if defined(RTC676x_USING_UNCACHED_MALLOC)
osSemaphoreId *RTC676x_Wait_Ack_Sem;
#else
osSemaphoreId RTC676x_Wait_Ack_Sem[PKT_TYPE_MAX] = {NULL};
#endif
#endif

osSemaphoreId RTC676x_Receive_MGMT_Sema;
osSemaphoreDef(RTC676x_Receive_MGMT_Sema);

osSemaphoreId RTC676x_Receive_SMSG_Sema;
osSemaphoreDef(RTC676x_Receive_SMSG_Sema);

osSemaphoreId RTC676x_Receive_VIDEO_Sema;
osSemaphoreDef(RTC676x_Receive_VIDEO_Sema);

osSemaphoreId RTC676x_Receive_AUDIO_Sema;
osSemaphoreDef(RTC676x_Receive_AUDIO_Sema);

osSemaphoreId RTC676x_Mac_Wait_Beacom_Timeout_Sema = NULL;
osSemaphoreDef(RTC676x_Mac_Wait_Beacom_Timeout_Sema);

#if defined(RTC676x_DMA_QUEUE_UNCACHED_MALLOC)
#define		RTC676x_DMA_Queue_MAX	64 /*16*/

#if defined(RTC676x_DMA_QUEUE_OS_MSG_Q_MGMT)

osMessageQId RTC676x_DMA_Big_Size_MSGQ;
//osMessageQDef(RTC676x_DMA_Big_Size_MSGQ, RTC676x_DMA_Queue_MAX, RTC676x_DMA_BUFFER_MGMT_t);

osMessageQId RTC676x_DMA_Small_Size_MSGQ;
//osMessageQDef(RTC676x_DMA_Small_Size_MSGQ, RTC676x_DMA_Queue_MAX, RTC676x_DMA_BUFFER_MGMT_t);

osMessageQId RTC676x_DMA_Transmit_MSGQ, RTC676x_DMA_Receive_MSGQ;

#else

osSemaphoreId RTC676x_DMA_Big_Size_Sema;
osSemaphoreDef(RTC676x_DMA_Big_Size_Sema);

osSemaphoreId RTC676x_DMA_Small_Size_Sema;
osSemaphoreDef(RTC676x_DMA_Small_Size_Sema);

osSemaphoreId RTC676x_DMA_Transmit_Sema, RTC676x_DMA_Receive_Sema;

#endif

osMutexId 	*RTC676x_DMA_Transmit_Mutex, *RTC676x_DMA_Receive_Mutex;
osMutexDef(RTC676x_DMA_Transmit_Mutex);
osMutexDef(RTC676x_DMA_Receive_Mutex);


#if defined(RTC676x_DMA_QUEUE_OS_MSG_Q_MGMT)
#else
uint8_t		RTC676x_DMA_Transmit_Queue_Consumer, RTC676x_DMA_Transmit_Queue_Producer;
uint8_t		RTC676x_DMA_Receive_Queue_Consumer, RTC676x_DMA_Receive_Queue_Producer;
#endif

uint32_t RTC676x_DMA_Queue_Size = RTC676x_DMA_Queue_MAX;

//uint8_t		*RTC676x_DMA_Big_Pkt_Queue[RTC676x_DMA_Queue_MAX + 2], *RTC676x_DMA_Small_Pkt_Queue[RTC676x_DMA_Queue_MAX + 2]; /*extra 1 : under bound, extra 2 : upper bound*/
uint8_t		**RTC676x_DMA_Big_Pkt_Queue, **RTC676x_DMA_Small_Pkt_Queue;
uint32_t	*RTC676x_DMA_Transmit_Queue, *RTC676x_DMA_Receive_Queue;
uint32_t	RTC676x_DMA_Transmit_Buffer_Size, RTC676x_DMA_Receive_Buffer_Size;
#endif

osSemaphoreId RTC676x_Tx_Temp_Stop_Done_Sema;
osSemaphoreDef(RTC676x_Tx_Temp_Stop_Done_Sema);

osSemaphoreId RTC676x_Tx_Clear_Wait_Sema;
osSemaphoreDef(RTC676x_Tx_Clear_Wait_Sema);

osSemaphoreId RTC676x_Timeout_Sema;
osSemaphoreDef(RTC676x_Timeout_Sema);

osSemaphoreId RTC676x_Recv_Queue_Sema[PKT_TYPE_MAX];
osSemaphoreId RTC676x_Recv_Queue_MGMT_Sema;
osSemaphoreId RTC676x_Recv_Queue_SMSG_Sema;
osSemaphoreId RTC676x_Recv_Queue_VIDEO_Sema;
osSemaphoreId RTC676x_Recv_Queue_AUDIO_Sema;
osSemaphoreDef(RTC676x_Recv_Queue_MGMT_Sema);
osSemaphoreDef(RTC676x_Recv_Queue_SMSG_Sema);
osSemaphoreDef(RTC676x_Recv_Queue_VIDEO_Sema);
osSemaphoreDef(RTC676x_Recv_Queue_AUDIO_Sema);

osMutexId 	RTC676x_Recv_Queue_Mutex[PKT_TYPE_MAX] = {NULL};
osMutexId RTC676x_Recv_Queue_MGMT_Mutex;
osMutexId RTC676x_Recv_Queue_SMSG_Mutex;
osMutexId RTC676x_Recv_Queue_VIDEO_Mutex;
osMutexId RTC676x_Recv_Queue_AUDIO_Mutex;
osMutexDef(RTC676x_Recv_Queue_MGMT_Mutex);
osMutexDef(RTC676x_Recv_Queue_SMSG_Mutex);
osMutexDef(RTC676x_Recv_Queue_VIDEO_Mutex);
osMutexDef(RTC676x_Recv_Queue_AUDIO_Mutex);

#define		RTC676x_Recv_Queue_MAX	16
uint8_t		RTC676x_Recv_Queue_Consumer[PKT_TYPE_MAX], RTC676x_Recv_Queue_Producer[PKT_TYPE_MAX], RTC676x_Recv_Queue_Used[PKT_TYPE_MAX];
uint8_t		*RTC676x_Recv_Queue[PKT_TYPE_MAX][RTC676x_Recv_Queue_MAX];
uint32_t	RTC676x_Recv_Queue_Data_Len[PKT_TYPE_MAX][RTC676x_Recv_Queue_MAX];

uint8_t		RTC676x_Recv_Last_Pkt_Type[PKT_TYPE_MAX];
uint32_t	RTC676x_Recv_Last_Beacon_Timestamp = 0;

// Mutex
#if defined(RTC676x_USING_UNCACHED_MALLOC)
osMutexId *RTC676x_Tx_Ack_List_Mutex;
#else
osMutexId RTC676x_Tx_Ack_List_Mutex[PKT_TYPE_MAX] = {NULL};
#endif
osMutexId RTC676x_Tx_Ack_List_MGMT_Mutex = NULL;
osMutexId RTC676x_Tx_Ack_List_SMSG_Mutex = NULL;
osMutexId RTC676x_Tx_Ack_List_VIDEO_Mutex = NULL;
osMutexId RTC676x_Tx_Ack_List_AUDIO_Mutex = NULL;
osMutexDef(RTC676x_Tx_Ack_List_MGMT_Mutex);
osMutexDef(RTC676x_Tx_Ack_List_SMSG_Mutex);
osMutexDef(RTC676x_Tx_Ack_List_VIDEO_Mutex);
osMutexDef(RTC676x_Tx_Ack_List_AUDIO_Mutex);

osMutexId RTC676x_Tx_Global_Mutex = NULL;
osMutexDef(RTC676x_Tx_Global_Mutex);

//Timer
#if defined(RTC676x_CHECK_TIMER)
void RTC676x_Check_Timer_Handler(void const *arg);
osTimerId RTC676x_Check_Timer = NULL;
osTimerDef(RTC676x_Check_Timer, RTC676x_Check_Timer_Handler);
#endif

#if defined(RTC676x_DEBUG_TIMER)
void RTC676x_Debug_Timer_Handler(void const *arg);
osTimerId RTC676x_Debug_Timer = NULL;
osTimerDef(RTC676x_Debug_Timer, RTC676x_Debug_Timer_Handler);
#endif

#if defined(RTC676x_USER_DEFINE_TRANSMIT_QUEUE_PRIORIT)
uint32_t RTC676x_Transmit_user_priority[PKT_TYPE_MAX] = {0};
#endif

static uint32_t	RTC676x_Tx_Temp_Stop = 0;
static uint32_t	RTC676x_Tx_Temp_Stop_Check = 0;
static uint32_t	RTC676x_Tx_Stop = 0;
static uint32_t	RTC676x_Tx_Stop_Waiting = 0;
static uint32_t RTC676x_Recv_FIFO_Not_Empty_Cnt = 0;
static bool RTC676x_Check_Int = false;
static bool RTC676x_ISR_Keep_Going = false;
static bool RTC676x_Debug_Print = false;

void RTC676x_SPI_CS_High(void);
void RTC676x_SPI_CS_Low(void);
void RTC676x_RF_Reset_Pin_High(void);
void RTC676x_RF_Reset_Pin_Low(void);
void RTC676x_IRQ_INTR(uint32_t value);
uint32_t RTC676x_IRQ_INTR_GET(void);
uint32_t RTC676x_IRQ_INPUT_GET(void);
void RTC676x_IRQ_INTR_MASK_SET(uint32_t value);
uint32_t RTC676x_IRQ_INTR_MASK_GET(void);
uint32_t RTC676x_IRQ_INTR_FLAG_GET(void);
void RTC676x_IRQ_CLR_INTR_SET(uint32_t value);
void RTC676x_INTC_IrqClear(void);

// Debug
uint64_t RTC676x_ISR_Cnt = 0;
uint64_t RTC676x_TSLOT_Cnt = 0;
uint64_t RTC676x_RXDR_Cnt = 0;
uint64_t RTC676x_RXDR_Err_Cnt = 0;
uint64_t RTC676x_TX_Done_Cnt = 0;
uint64_t RTC676x_TX_DS_Cnt = 0;
uint64_t RTC676x_TX_NS_Cnt = 0;
uint64_t RTC676x_TX_NS_PAD_Cnt = 0;
uint64_t RTC676x_TX_NS_Q3_PAD_Cnt = 0;
uint64_t RTC676x_MAX_RT_Cnt = 0;
uint64_t RTC676x_None_Cnt = 0;
uint64_t RTC676x_INT_BUT_NON_DONE_Cnt = 0;
uint64_t RTC676x_TX_DATARATE_Cnt = 0;
uint64_t RTC676x_TX_VALIDIRY_DATARATE_Cnt = 0;
uint8_t RTC676x_TX_DATARATE_Cnt_Clear = 0;
uint8_t RTC676x_TX_VALIDIRY_DATARATE_Cnt_Clear = 0;
uint64_t RTC676x_Drop_Consecutive_Beacons_Cnt = 0;

uint64_t RTV676x_TX_NS_SOFT_Cnt = 0;

uint32_t RTC676x_RX_CRC_ERRs = 0;
uint32_t RTC676x_RX_CRC_ERRs_Fix = 0;



uint64_t RTC676x_ISR_Pre_Cnt = 0;
uint64_t RTC676x_TSLOT_Pre_Cnt = 0;
uint64_t RTC676x_TX_Done_Pre_Cnt = 0;
uint64_t RTC676x_RXDR_Pre_Cnt = 0;
uint64_t RTC676x_TX_DS_Pre_Cnt = 0;
uint64_t RTC676x_TX_NS_Pre_Cnt = 0;
uint64_t RTC676x_None_Pre_Cnt = 0;
uint64_t RTC676x_TX_DATARATE_Per_S = 0;
uint64_t RTC676x_VALIDIRY_TX_DATARATE_Per_S = 0;

uint64_t RTC676x_Clear_Transmit_FIFO_Cnt = 0;
uint64_t RTC676x_Clear_Recv_FIFO_Cnt = 0;

uint64_t RTC676x_Thread_Run_Cnt = 0;
uint32_t RTC676x_Thread_State = 0;

uint16_t RTC676x_ISR_Per_S = 0;
uint16_t RTC676x_TSLOT_Per_S = 0;
uint16_t RTC676x_TSLOT_Transmit_Per_S = 0;
uint16_t RTC676x_RXDR_Per_S = 0;
uint16_t RTC676x_TX_DS_Per_S = 0;
uint16_t RTC676x_TX_NS_Per_S = 0;
uint16_t RTC676x_None_Per_S = 0;

uint64_t RTC676x_Transmit_Cnt[PKT_TYPE_MAX] = {0};
uint64_t RTC676x_ACK_Cnt[PKT_TYPE_MAX] = {0};
uint64_t RTC676x_NACK_Cnt[PKT_TYPE_MAX] = {0};
uint64_t RTC676x_NACK_PAD_Cnt[PKT_TYPE_MAX] = {0};
uint64_t RTC676x_Receive_Cnt[PKT_TYPE_MAX] = {0};
uint64_t RTC676x_RXBytes[PKT_TYPE_MAX] = {0};
uint64_t RTC676x_TXBytes[PKT_TYPE_MAX] = {0};
uint64_t RTC676x_ACKedBytes[PKT_TYPE_MAX] = {0};
#if defined(RTC676x_USER_DEFINE_TRANSMIT_QUEUE_PRIORIT)
uint64_t RTC676x_Priority_Q_Transmit_Cnt[PKT_TYPE_MAX] = {0};
#endif

uint64_t RTC676x_REISR_EVENT_Cnt = 0;
uint64_t RTC676x_INT_ISSUE_CHECK_Cnt = 0;
uint64_t RTC676x_INT_ISSUE_CHECK_CLEAR_Cnt = 0;
uint8_t RTC676x_INT_ISSUE_CHECK_CLEAR = 0;
uint32_t RTC676x_PUT_NACK_ERR = 0;
uint64_t RTC676x_TX_Wait_Empty_Cnt = 0;

typedef struct
{
	int32_t			slBandwidthMode;
	uint32_t		ulWholeSlot;
	uint32_t		ulEvenSlot;
	uint32_t		ulSlaveBpskLength;
	uint32_t		ulSlaveBpskLength4DivMode;
	uint32_t		ulSlaveQpskLength;
	uint32_t		ulSlaveQpskLength4DivMode;
	uint32_t		ulSlave16QamLengthFullMode;
	uint32_t		ulSlave16QamLength4DivMode;
	uint32_t		ulMasterBpskLength;
	uint32_t		ulMasterBpskLength4DivMode;
	uint32_t		ulMasterQpskLength;
	uint32_t		ulMasterQpskLength4DivMode;
	uint32_t		ulMaster16QamLengthFullMode;
	uint32_t		ulMaster16QamLength4DivMode;
} RTC676x_PKT_SIZE_SUPPORT;

static const RTC676x_PKT_SIZE_SUPPORT pkt_size_support[] =
{
	{
		.slBandwidthMode = PREFERRED_LONG_DISTANCE_BPSK_ONLY,
		.ulWholeSlot = RATE_1000_HOPS_PER_SECOND,
		.ulEvenSlot = 750,
		.ulSlaveBpskLength = 200,
		.ulSlaveBpskLength4DivMode = 200,
		.ulSlaveQpskLength = 428,
		.ulSlaveQpskLength4DivMode = 428,
		.ulSlave16QamLengthFullMode = 428,
		.ulSlave16QamLength4DivMode = 824,
		.ulMasterBpskLength = 0,
		.ulMasterBpskLength4DivMode = 0,
		.ulMasterQpskLength = 56,
		.ulMasterQpskLength4DivMode = 56,
		.ulMaster16QamLengthFullMode = 136,
		.ulMaster16QamLength4DivMode = 136,
	},
	{
		.slBandwidthMode = PREFERRED_LONG_DISTANCE_BPSK_ONLY,
		.ulWholeSlot = RATE_900_HOPS_PER_SECOND,
		.ulEvenSlot = 833,
		.ulSlaveBpskLength = 236,
		.ulSlaveBpskLength4DivMode = 236,
		.ulSlaveQpskLength = 488,
		.ulSlaveQpskLength4DivMode = 488,
		.ulSlave16QamLengthFullMode = 488,
		.ulSlave16QamLength4DivMode = 952,
		.ulMasterBpskLength = 0,
		.ulMasterBpskLength4DivMode = 0,
		.ulMasterQpskLength = 80,
		.ulMasterQpskLength4DivMode = 80,
		.ulMaster16QamLengthFullMode = 156,
		.ulMaster16QamLength4DivMode = 156,
	},
	{
		.slBandwidthMode = PREFERRED_LONG_DISTANCE_BPSK_ONLY_MORE_BYTES_R2T,
		.ulWholeSlot = RATE_900_HOPS_PER_SECOND,
		.ulEvenSlot = 900,
		.ulSlaveBpskLength = 228,
		.ulSlaveBpskLength4DivMode = 228,
		.ulSlaveQpskLength = 472,
		.ulSlaveQpskLength4DivMode = 472,
		.ulSlave16QamLengthFullMode = 604,
		.ulSlave16QamLength4DivMode = 920,
		.ulMasterBpskLength = 36,
		.ulMasterBpskLength4DivMode = 36,
		.ulMasterQpskLength = 88,
		.ulMasterQpskLength4DivMode = 88,
		.ulMaster16QamLengthFullMode = 164,
		.ulMaster16QamLength4DivMode = 164,
	},
	{
		.slBandwidthMode = PREFERRED_LONG_DISTANCE_BPSK_ONLY,
		.ulWholeSlot = RATE_600_HOPS_PER_SECOND,
		.ulEvenSlot = 880,
		.ulSlaveBpskLength = 440,
		.ulSlaveBpskLength4DivMode = 440,
		.ulSlaveQpskLength = 548,
		.ulSlaveQpskLength4DivMode = 856,
		.ulSlave16QamLengthFullMode = 548,
		.ulSlave16QamLength4DivMode = 1444,
		.ulMasterBpskLength = 32,
		.ulMasterBpskLength4DivMode = 32,
		.ulMasterQpskLength = 80,
		.ulMasterQpskLength4DivMode = 80,
		.ulMaster16QamLengthFullMode = 188,
		.ulMaster16QamLength4DivMode = 188,
	},
    {
        .slBandwidthMode = PREFERRED_LONG_DISTANCE_BPSK_ONLY_MORE_BYTES_R2T,
        .ulWholeSlot = RATE_600_HOPS_PER_SECOND,
        .ulEvenSlot = 900,
        .ulSlaveBpskLength = 436,
        .ulSlaveBpskLength4DivMode = 436,
        .ulSlaveQpskLength = 640,
        .ulSlaveQpskLength4DivMode = 868,
        .ulSlave16QamLengthFullMode = 652,
        .ulSlave16QamLength4DivMode = 1448,
        .ulMasterBpskLength = 36,
        .ulMasterBpskLength4DivMode = 36,
        .ulMasterQpskLength = 88,
        .ulMasterQpskLength4DivMode = 88,
        .ulMaster16QamLengthFullMode = 200,
        .ulMaster16QamLength4DivMode = 200,
    },
	{
		.slBandwidthMode = PREFERRED_DISTANCE_QPSK_ONLY,
		.ulWholeSlot = RATE_600_HOPS_PER_SECOND,
		.ulEvenSlot = 1200,
		.ulSlaveBpskLength = 400,
		.ulSlaveBpskLength4DivMode = 400,
		.ulSlaveQpskLength = 788,
		.ulSlaveQpskLength4DivMode = 816,
		.ulSlave16QamLengthFullMode = 888 /*784*/,
		.ulSlave16QamLength4DivMode = 1440 /*784*/,
		.ulMasterBpskLength = 72,
		.ulMasterBpskLength4DivMode = 72,
		.ulMasterQpskLength = 164,
		.ulMasterQpskLength4DivMode = 164,
		.ulMaster16QamLengthFullMode = 348,
		.ulMaster16QamLength4DivMode = 348,
	},
	{
		.slBandwidthMode = BALANCE_BOTH_16QAM_AND_QPSK,
		.ulWholeSlot = RATE_600_HOPS_PER_SECOND,
		.ulEvenSlot = 1433,
		.ulSlaveBpskLength = 368,
		.ulSlaveBpskLength4DivMode = 368,
		.ulSlaveQpskLength = 752,
		.ulSlaveQpskLength4DivMode = 756,
		.ulSlave16QamLengthFullMode = 1136,
		.ulSlave16QamLength4DivMode = 1432,
		.ulMasterBpskLength = 104,
		.ulMasterBpskLength4DivMode = 104,
		.ulMasterQpskLength = 224,
		.ulMasterQpskLength4DivMode = 224,
		.ulMaster16QamLengthFullMode = 464,
		.ulMaster16QamLength4DivMode = 464,
	},
	{
		.slBandwidthMode = PREFERRED_RATE_BOTH_16QAM_AND_QPSK,
		.ulWholeSlot = RATE_600_HOPS_PER_SECOND,
		.ulEvenSlot = 1667,
		.ulSlaveBpskLength = 340,
		.ulSlaveBpskLength4DivMode = 340,
		.ulSlaveQpskLength = 700,
		.ulSlaveQpskLength4DivMode = 700,
		.ulSlave16QamLengthFullMode = 1352,
		.ulSlave16QamLength4DivMode = 1416,
		.ulMasterBpskLength = 132,
		.ulMasterBpskLength4DivMode = 132,
		.ulMasterQpskLength = 284,
		.ulMasterQpskLength4DivMode = 284,
		.ulMaster16QamLengthFullMode = 584,
		.ulMaster16QamLength4DivMode = 584,
	},
};

void RTC676x_Reset(void);

#define CRC16_CCITT_SEED       0xFFFF

static unsigned short crc16_ccitt_table[256] =
{
    0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50A5, 0x60C6, 0x70E7,
    0x8108, 0x9129, 0xA14A, 0xB16B, 0xC18C, 0xD1AD, 0xE1CE, 0xF1EF,
    0x1231, 0x0210, 0x3273, 0x2252, 0x52B5, 0x4294, 0x72F7, 0x62D6,
    0x9339, 0x8318, 0xB37B, 0xA35A, 0xD3BD, 0xC39C, 0xF3FF, 0xE3DE,
    0x2462, 0x3443, 0x0420, 0x1401, 0x64E6, 0x74C7, 0x44A4, 0x5485,
    0xA56A, 0xB54B, 0x8528, 0x9509, 0xE5EE, 0xF5CF, 0xC5AC, 0xD58D,
    0x3653, 0x2672, 0x1611, 0x0630, 0x76D7, 0x66F6, 0x5695, 0x46B4,
    0xB75B, 0xA77A, 0x9719, 0x8738, 0xF7DF, 0xE7FE, 0xD79D, 0xC7BC,
    0x48C4, 0x58E5, 0x6886, 0x78A7, 0x0840, 0x1861, 0x2802, 0x3823,
    0xC9CC, 0xD9ED, 0xE98E, 0xF9AF, 0x8948, 0x9969, 0xA90A, 0xB92B,
    0x5AF5, 0x4AD4, 0x7AB7, 0x6A96, 0x1A71, 0x0A50, 0x3A33, 0x2A12,
    0xDBFD, 0xCBDC, 0xFBBF, 0xEB9E, 0x9B79, 0x8B58, 0xBB3B, 0xAB1A,
    0x6CA6, 0x7C87, 0x4CE4, 0x5CC5, 0x2C22, 0x3C03, 0x0C60, 0x1C41,
    0xEDAE, 0xFD8F, 0xCDEC, 0xDDCD, 0xAD2A, 0xBD0B, 0x8D68, 0x9D49,
    0x7E97, 0x6EB6, 0x5ED5, 0x4EF4, 0x3E13, 0x2E32, 0x1E51, 0x0E70,
    0xFF9F, 0xEFBE, 0xDFDD, 0xCFFC, 0xBF1B, 0xAF3A, 0x9F59, 0x8F78,
    0x9188, 0x81A9, 0xB1CA, 0xA1EB, 0xD10C, 0xC12D, 0xF14E, 0xE16F,
    0x1080, 0x00A1, 0x30C2, 0x20E3, 0x5004, 0x4025, 0x7046, 0x6067,
    0x83B9, 0x9398, 0xA3FB, 0xB3DA, 0xC33D, 0xD31C, 0xE37F, 0xF35E,
    0x02B1, 0x1290, 0x22F3, 0x32D2, 0x4235, 0x5214, 0x6277, 0x7256,
    0xB5EA, 0xA5CB, 0x95A8, 0x8589, 0xF56E, 0xE54F, 0xD52C, 0xC50D,
    0x34E2, 0x24C3, 0x14A0, 0x0481, 0x7466, 0x6447, 0x5424, 0x4405,
    0xA7DB, 0xB7FA, 0x8799, 0x97B8, 0xE75F, 0xF77E, 0xC71D, 0xD73C,
    0x26D3, 0x36F2, 0x0691, 0x16B0, 0x6657, 0x7676, 0x4615, 0x5634,
    0xD94C, 0xC96D, 0xF90E, 0xE92F, 0x99C8, 0x89E9, 0xB98A, 0xA9AB,
    0x5844, 0x4865, 0x7806, 0x6827, 0x18C0, 0x08E1, 0x3882, 0x28A3,
    0xCB7D, 0xDB5C, 0xEB3F, 0xFB1E, 0x8BF9, 0x9BD8, 0xABBB, 0xBB9A,
    0x4A75, 0x5A54, 0x6A37, 0x7A16, 0x0AF1, 0x1AD0, 0x2AB3, 0x3A92,
    0xFD2E, 0xED0F, 0xDD6C, 0xCD4D, 0xBDAA, 0xAD8B, 0x9DE8, 0x8DC9,
    0x7C26, 0x6C07, 0x5C64, 0x4C45, 0x3CA2, 0x2C83, 0x1CE0, 0x0CC1,
    0xEF1F, 0xFF3E, 0xCF5D, 0xDF7C, 0xAF9B, 0xBFBA, 0x8FD9, 0x9FF8,
    0x6E17, 0x7E36, 0x4E55, 0x5E74, 0x2E93, 0x3EB2, 0x0ED1, 0x1EF0
};

unsigned short crc16_ccitt(unsigned char *pbuf, size_t len)
{
    unsigned short crc = CRC16_CCITT_SEED;

	if(len > 4096)
	{
		printf("Errr[%d] @%s\r\n",len,__func__);
		return crc;
	}
	
    while (len-- > 0)
    {
        crc = crc16_ccitt_table[(crc >> 8 ^ *pbuf++) & 0xff] ^ (crc << 8);
    }

    return crc;

}


//API for main or other thread
//------------------------------------------------------------------------------
uint16_t uwRTC676x_SPI_GetVersion(void)
{
    return ((RTC676x_SPI_MAJOR_VER << 8) + RTC676x_SPI_MINOR_VER);
}
//------------------------------------------------------------------------------
static inline uint64_t RTC676x_Sys_Micro_Second(void)
{
	uint64_t tick = osKernelSysTick();
    return tick * 1000000 / osKernelSysTickFrequency;
}
//------------------------------------------------------------------------------
#if 0
void RTC676x_SPI_Init(SPI_Setup_t* setup)
{
	RTC676x_Data_t* pRtc676xData = NULL;

	RTC676x_GPIO_Init();

    SSP->SSP_SCLKPH     = setup->ubSPI_CPHA;
    SSP->SSP_SCLKPO     = setup->ubSPI_CPOL;
    SSP->SSP_OPM        = setup->tSPI_Mode;
    SSP->SSP_SCLKDIV    = setup->uwClkDiv;
    SSP->SSP_FFMT       = 1;            // Motorola SPI
    //pfRTC676x_SPI_DmaEndHook    = setup->pfDmaEndHook;
    pfRTC676x_SPI_DmaEndHook = RTC676x_SPI_DmaEnd_CallBack;

    if(SSP->SSP_OPM == SPI_SLAVE)
        pfRTC676x_SPI_SlaveHook = setup->pfSlaveHook;

    //! clear rx/tx fifo
    SSP->SSP_RXF_CLR = 1;
    SSP->SSP_TXF_CLR = 1;
    SSP->SSP_RXF_TH = 1;
    SSP->SSP_TXF_TH = 15;
    SSP->SSP_TXDOE = 1;

	SSP->SSP_TXF_TH = 4;

    SSP->SSP_EN = 0;

	RTC676x_Memory_Poll_Ptr = ulBUF_GetRFDriverStartAddr();

	//! create memory poll
	//RTC676x_Msg_Pool = osPoolCreate(osPool(RTC676x_Msg_Pool));
#if defined(RTC676x_DMA_QUEUE_UNCACHED_MALLOC)
	//RTC676x_DMA_Buffer_Init();
	RTC676x_Memory_Poll_Used += RTC676x_DMA_Buffer_Init(RTC676x_Memory_Poll_Ptr + RTC676x_Memory_Poll_Used);
#else
	RTC676x_Pkt_Big_Size_Poll = osPoolCreate(osPool(RTC676x_Pkt_Big_Size_Poll));
	RTC676x_Pkt_Small_Size_Poll = osPoolCreate(osPool(RTC676x_Pkt_Small_Size_Poll));

	RTC676x_Pkt_Trans_Poll = RTC676x_Pkt_Big_Size_Poll;
	RTC676x_Pkt_Recv_Poll = RTC676x_Pkt_Small_Size_Poll;
#endif

	RTC676x_Memory_Poll_Used += RTC676x_Recv_Queue_Init(RTC676x_Memory_Poll_Ptr + RTC676x_Memory_Poll_Used);

	//! create message queue
	RTC676x_Msg_Q = osMessageCreate(osMessageQ(RTC676x_Msg_Q), NULL);
#if !defined(RTC676x_WAIT_RETURN_BY_EVENT)
	RTC676x_Reg_Return_Msg_Q = osMessageCreate(osMessageQ(RTC676x_Reg_Return_Msg_Q), NULL);
#endif
#if defined(RTC676x_TRANSMIT_REQUEST_MULTI_Q)
	RTC676x_Tx_Request_MGMT_Q = osMessageCreate(osMessageQ(RTC676x_Tx_Request_MGMT_Q), NULL);
	RTC676x_Tx_Request_SMSG_Q = osMessageCreate(osMessageQ(RTC676x_Tx_Request_SMSG_Q), NULL);
	RTC676x_Tx_Request_VIDEO_Q = osMessageCreate(osMessageQ(RTC676x_Tx_Request_VIDEO_Q), NULL);
	RTC676x_Tx_Request_AUDIO_Q = osMessageCreate(osMessageQ(RTC676x_Tx_Request_AUDIO_Q), NULL);
#if defined(RTC676x_USING_UNCACHED_MALLOC)
	RTC676x_Tx_Request_Q = (osMessageQId *)osUncachedMalloc(sizeof(osMessageQId) * PKT_TYPE_MAX);
#endif
	RTC676x_Tx_Request_Q[PKT_TYPE_MGMT] = RTC676x_Tx_Request_MGMT_Q;
	RTC676x_Tx_Request_Q[PKT_TYPE_SMSG] = RTC676x_Tx_Request_SMSG_Q;
	RTC676x_Tx_Request_Q[PKT_TYPE_VIDEO] = RTC676x_Tx_Request_VIDEO_Q;
	RTC676x_Tx_Request_Q[PKT_TYPE_AUDIO] = RTC676x_Tx_Request_AUDIO_Q;
#else /* RTC676x_TRANSMIT_REQUEST_MULTI_Q */
	RTC676x_Tx_Request_Q = osMessageCreate(osMessageQ(RTC676x_Tx_Request_Q), NULL);
#endif
	RTC676x_Tx_Wait_Ack_Q = osMessageCreate(osMessageQ(RTC676x_Tx_Wait_Ack_Q), NULL);
	RTC676x_DMA_Request_Q = osMessageCreate(osMessageQ(RTC676x_DMA_Request_Q), NULL);
	RTC676x_Recv_MGMT_Q = osMessageCreate(osMessageQ(RTC676x_Recv_MGMT_Q), NULL);
	RTC676x_Recv_SMSG_Q = osMessageCreate(osMessageQ(RTC676x_Recv_SMSG_Q), NULL);
	RTC676x_Recv_VIDEO_Q = osMessageCreate(osMessageQ(RTC676x_Recv_VIDEO_Q), NULL);
	RTC676x_Recv_AUDIO_Q = osMessageCreate(osMessageQ(RTC676x_Recv_AUDIO_Q), NULL);

	//! create semaphore
	RTC676x_Reg_RW_Sema = osSemaphoreCreate(osSemaphore(RTC676x_Reg_RW_Sema), 1);
	RTC676x_Receive_MGMT_Sema = osSemaphoreCreate(osSemaphore(RTC676x_Receive_MGMT_Sema), 1);
	RTC676x_Receive_SMSG_Sema = osSemaphoreCreate(osSemaphore(RTC676x_Receive_SMSG_Sema), 1);
	RTC676x_Receive_VIDEO_Sema = osSemaphoreCreate(osSemaphore(RTC676x_Receive_VIDEO_Sema), 1);
	RTC676x_Receive_AUDIO_Sema = osSemaphoreCreate(osSemaphore(RTC676x_Receive_AUDIO_Sema), 1);
	RTC676x_Mac_Wait_Beacom_Timeout_Sema = osSemaphoreCreate(osSemaphore(RTC676x_Mac_Wait_Beacom_Timeout_Sema), 1);
	if (osSemaphoreWait(RTC676x_Receive_MGMT_Sema, 3) == -1)
		printf("RTC676x_SPI: RTC676x_Receive_MGMT_Sema fail\n");
	if (osSemaphoreWait(RTC676x_Receive_SMSG_Sema, 3) == -1)
		printf("RTC676x_SPI: RTC676x_Receive_SMSG_Sema fail\n");
	if (osSemaphoreWait(RTC676x_Receive_VIDEO_Sema, 3) == -1)
		printf("RTC676x_SPI: RTC676x_Receive_VIDEO_Sema fail\n");
	if (osSemaphoreWait(RTC676x_Receive_AUDIO_Sema, 3) == -1)
		printf("RTC676x_SPI: RTC676x_Receive_AUDIO_Sema fail\n");
	if (osSemaphoreWait(RTC676x_Mac_Wait_Beacom_Timeout_Sema, 3) == -1)
		printf("RTC676x_SPI: RTC676x_Mac_Wait_Beacom_Timeout_Sema fail\n");
#if !defined(RTC676x_WAIT_ACK_BY_EVENT)
	int ret;
	RTC676x_Wait_MGMT_Ack_Sem = osSemaphoreCreate(osSemaphore(RTC676x_Wait_MGMT_Ack_Sem), 1);
	RTC676x_Wait_SMSG_Ack_Sem = osSemaphoreCreate(osSemaphore(RTC676x_Wait_SMSG_Ack_Sem), 1);
	RTC676x_Wait_VIDEO_Ack_Sem = osSemaphoreCreate(osSemaphore(RTC676x_Wait_VIDEO_Ack_Sem), 1);
	RTC676x_Wait_AUDIO_Ack_Sem = osSemaphoreCreate(osSemaphore(RTC676x_Wait_AUDIO_Ack_Sem), 1);
	ret = osSemaphoreWait(RTC676x_Wait_MGMT_Ack_Sem, 3);
	if (ret == -1)
		printf("RTC676x_SPI: RTC676x_Wait_MGMT_Ack_Sem fail\n");
	else
		printf("RTC676x_SPI: RTC676x_Wait_MGMT_Ack_Sem ret:%d\n", ret);
	osSemaphoreRelease(RTC676x_Wait_MGMT_Ack_Sem);
	ret = osSemaphoreWait(RTC676x_Wait_MGMT_Ack_Sem, 3);
	printf("RTC676x_SPI: RTC676x_Wait_MGMT_Ack_Sem __ret:%d\n", ret);
	if (osSemaphoreWait(RTC676x_Wait_SMSG_Ack_Sem, 3) == -1)
		printf("RTC676x_SPI: RTC676x_Wait_SMSG_Ack_Sem fail\n");
	if (osSemaphoreWait(RTC676x_Wait_VIDEO_Ack_Sem, 3) == -1)
		printf("RTC676x_SPI: RTC676x_Wait_VIDEO_Ack_Sem fail\n");
	if (osSemaphoreWait(RTC676x_Wait_AUDIO_Ack_Sem, 3) == -1)
		printf("RTC676x_SPI: RTC676x_Wait_AUDIO_Ack_Sem fail\n");
#if defined(RTC676x_USING_UNCACHED_MALLOC)
	RTC676x_Wait_Ack_Sem = (osSemaphoreId *)osUncachedMalloc(sizeof(osSemaphoreId) * PKT_TYPE_MAX);
#endif
	RTC676x_Wait_Ack_Sem[PKT_TYPE_MGMT] = RTC676x_Wait_MGMT_Ack_Sem;
	RTC676x_Wait_Ack_Sem[PKT_TYPE_SMSG] = RTC676x_Wait_SMSG_Ack_Sem;
	RTC676x_Wait_Ack_Sem[PKT_TYPE_VIDEO] = RTC676x_Wait_VIDEO_Ack_Sem;
	RTC676x_Wait_Ack_Sem[PKT_TYPE_AUDIO] = RTC676x_Wait_AUDIO_Ack_Sem;
#endif

	//! create mutex
	RTC676x_Tx_Ack_List_MGMT_Mutex = osMutexCreate(osMutex(RTC676x_Tx_Ack_List_MGMT_Mutex));
	RTC676x_Tx_Ack_List_SMSG_Mutex = osMutexCreate(osMutex(RTC676x_Tx_Ack_List_SMSG_Mutex));
	RTC676x_Tx_Ack_List_VIDEO_Mutex = osMutexCreate(osMutex(RTC676x_Tx_Ack_List_VIDEO_Mutex));
	RTC676x_Tx_Ack_List_AUDIO_Mutex = osMutexCreate(osMutex(RTC676x_Tx_Ack_List_AUDIO_Mutex));
#if defined(RTC676x_USING_UNCACHED_MALLOC)
	RTC676x_Tx_Ack_List_Mutex = (osMutexId *)osUncachedMalloc(sizeof(osMutexId) * PKT_TYPE_MAX);
#endif
	RTC676x_Tx_Ack_List_Mutex[PKT_TYPE_MGMT] = RTC676x_Tx_Ack_List_MGMT_Mutex;
	RTC676x_Tx_Ack_List_Mutex[PKT_TYPE_SMSG] = RTC676x_Tx_Ack_List_SMSG_Mutex;
	RTC676x_Tx_Ack_List_Mutex[PKT_TYPE_VIDEO] = RTC676x_Tx_Ack_List_VIDEO_Mutex;
	RTC676x_Tx_Ack_List_Mutex[PKT_TYPE_AUDIO] = RTC676x_Tx_Ack_List_AUDIO_Mutex;

	//! allocate memory
	//pRtc676xData = (RTC676x_Data_t*)malloc(sizeof(RTC676x_Data_t));
	pRtc676xData = (RTC676x_Data_t*)osUncachedMalloc(sizeof(RTC676x_Data_t));
	memset(pRtc676xData, 0, sizeof(RTC676x_Data_t));
	ubRegReadRData = osUncachedMalloc(16);
	ubRegReadWData = osUncachedMalloc(16);
	ubRegWriteRData = osUncachedMalloc(16);
	ubRegWriteWData = osUncachedMalloc(16);

#if defined(RTC676x_USING_UNCACHED_MALLOC)
	RTC676x_Tx_Ack_List = (uint64_t *)osUncachedMalloc(sizeof(uint64_t) * PKT_TYPE_MAX);
	RTC676x_Tx_Ack_List_Mask = (uint64_t *)osUncachedMalloc(sizeof(uint64_t) * PKT_TYPE_MAX);

	RTC676x_Rx_Overrun_State = (int32_t *)osUncachedMalloc(sizeof(int32_t) * PKT_TYPE_MAX);

	memset(RTC676x_Tx_Ack_List, 0x00, sizeof(uint64_t) * PKT_TYPE_MAX);
	memset(RTC676x_Tx_Ack_List_Mask, 0x00, sizeof(uint64_t) * PKT_TYPE_MAX);
	memset(RTC676x_Rx_Overrun_State, 0x00, sizeof(int32_t) * PKT_TYPE_MAX);
#endif

#if defined(RTC676x_USING_UNCACHED_MALLOC)
	ubRfStatus = osUncachedMalloc(RTC676x_REG_STATUS_LENGTH + 1);
	ubRfStatusRead = osUncachedMalloc(RTC676x_REG_STATUS_LENGTH + 1);
	ubTimStatus = osUncachedMalloc(RTC676x_REG_TIM_STATIS_LENGTH + 1);
	ubTimStatusRead = osUncachedMalloc(RTC676x_REG_TIM_STATIS_LENGTH + 1);

	ubRfStatus[0] = RTC676x_REG_STATUS | RTC676x_SPI_REG_OFFSET_READ_BIT;
	ubTimStatus[0] = RTC676x_REG_TIM_STATUS | RTC676x_SPI_REG_OFFSET_READ_BIT;
#if defined(RTC676x_6765)
	ubConfig3 = osUncachedMalloc(RTC676x_REG_CONFIG3_LENGTH + 1);
	ubConfig3Read= osUncachedMalloc(RTC676x_REG_CONFIG3_LENGTH + 1);
#else /*6763*/
	ubConfig3 = osUncachedMalloc(RTC676x_REG_CONFIG3_LENGTH + 1);
	ubConfig3Read = osUncachedMalloc(RTC676x_REG_CONFIG3_LENGTH + 1);
#endif
	ubConfig3[0] = RTC676x_REG_CONFIG3 | RTC676x_SPI_REG_OFFSET_READ_BIT;

	ubRfStatusClean = osUncachedMalloc(RTC676x_REG_STATUS_LENGTH + 1);
	ubTimStatusClean = osUncachedMalloc(RTC676x_REG_STATUS_LENGTH + 1);
	ubRegReadDummy = osUncachedMalloc(RTC676x_REG_CONFIG3_LENGTH + 1);
	ubRfStatusClean[0] = RTC676x_REG_STATUS;
	ubTimStatusClean[0] = RTC676x_REG_TIM_STATUS;

#if defined(RTC676x_SPI_MAC_READ_PRE_DO)
	ubRecvHeader = osUncachedMalloc(64);
	ubRecvHeaderRead = osUncachedMalloc(64);
#else
	ubRecvHeader = osUncachedMalloc(5);
	ubRecvHeaderRead = osUncachedMalloc(5);
#endif
#endif

#if defined(RTC676x_CHECK_TIMER)
	RTC676x_Check_Timer = osTimerCreate(osTimer(RTC676x_Check_Timer), osTimerPeriodic, NULL);
	osTimerStart(RTC676x_Check_Timer, RTC676x_CHECK_TIMER);
#endif

#if defined(RTC676x_DEBUG_TIMER)
	RTC676x_Debug_Timer = osTimerCreate(osTimer(RTC676x_Debug_Timer), osTimerPeriodic, NULL);
	osTimerStart(RTC676x_Debug_Timer, RTC676x_DEBUG_TIMER);
#endif

    if(RTC676x_SPI_ThreadId == NULL)
    {
        RTC676x_SPI_ThreadId = osThreadCreate(osThread(RTC676x_SPI_SlavePolling), pRtc676xData);
        if(RTC676x_SPI_ThreadId == NULL)
        {
            printd(DBG_CriticalLvl, "RTC676x: Create RTC676x_Thread fail !!!!\r\n");
            while(1);
        }
    }
//    INTC_IrqSetup(INTC_SSP_IRQ, INTC_EDGE_TRIG, RTC676x_ISR);

	printf("RTC676x: SPI init done!\r\n");
}
#endif
//void RTC676x_SPI_Init_Step2(RTC676x_Data_t* pRtc676xData, RTC676x_RF_Device tRFDevice)
void RTC676x_SPI_Init_Step2(RTC676x_RF_Device tRFDevice)
{
	RTC676x_Data_t* pRtc676xData = (RTC676x_Data_t*)osUncachedMalloc(sizeof(RTC676x_Data_t));
	memset(pRtc676xData, 0, sizeof(RTC676x_Data_t));

	RTC676x_Memory_Poll_Ptr = ulBUF_GetRFDriverStartAddr();

	pfRTC676x_SPI_DmaEndHook = RTC676x_SPI_DmaEnd_CallBack;

	//! create memory poll
	//RTC676x_Msg_Pool = osPoolCreate(osPool(RTC676x_Msg_Pool));
#if defined(RTC676x_DMA_QUEUE_UNCACHED_MALLOC)
	//RTC676x_DMA_Buffer_Init();
	RTC676x_Memory_Poll_Used += RTC676x_DMA_Buffer_Init(RTC676x_Memory_Poll_Ptr + RTC676x_Memory_Poll_Used, tRFDevice);
#else
	RTC676x_Pkt_Big_Size_Poll = osPoolCreate(osPool(RTC676x_Pkt_Big_Size_Poll));
	RTC676x_Pkt_Small_Size_Poll = osPoolCreate(osPool(RTC676x_Pkt_Small_Size_Poll));

	RTC676x_Pkt_Trans_Poll = RTC676x_Pkt_Big_Size_Poll;
	RTC676x_Pkt_Recv_Poll = RTC676x_Pkt_Small_Size_Poll;
#endif

	if(tRFDevice == RTC676x_RF_Master)
		RTC676x_RECV_PACKET_SIZE = RTC676x_RECV_BIG_PACKET_SIZE;
	else
		RTC676x_RECV_PACKET_SIZE = RTC676x_RECV_SMALL_PACKET_SIZE;

	RTC676x_Memory_Poll_Used += RTC676x_Recv_Queue_Init(RTC676x_Memory_Poll_Ptr + RTC676x_Memory_Poll_Used);

	//! create message queue
	RTC676x_Msg_Q = osMessageCreate(osMessageQ(RTC676x_Msg_Q), NULL);
#if !defined(RTC676x_WAIT_RETURN_BY_EVENT)
	RTC676x_Reg_Return_Msg_Q = osMessageCreate(osMessageQ(RTC676x_Reg_Return_Msg_Q), NULL);
#endif
#if defined(RTC676x_TRANSMIT_REQUEST_MULTI_Q)
	RTC676x_Tx_Request_MGMT_Q = osMessageCreate(osMessageQ(RTC676x_Tx_Request_MGMT_Q), NULL);
#if defined(RTC676x_USER_DEFINE_TRANSMIT_QUEUE_PRIORIT)
	RTC676x_Tx_Request_Priority_High_Q = osMessageCreate(osMessageQ(RTC676x_Tx_Request_Priority_High_Q), NULL);
	RTC676x_Tx_Request_Priority_Middle_Q = osMessageCreate(osMessageQ(RTC676x_Tx_Request_Priority_Middle_Q), NULL);
	RTC676x_Tx_Request_Priority_Low_Q = osMessageCreate(osMessageQ(RTC676x_Tx_Request_Priority_Low_Q), NULL);
#else
	RTC676x_Tx_Request_SMSG_Q = osMessageCreate(osMessageQ(RTC676x_Tx_Request_SMSG_Q), NULL);
	RTC676x_Tx_Request_VIDEO_Q = osMessageCreate(osMessageQ(RTC676x_Tx_Request_VIDEO_Q), NULL);
	RTC676x_Tx_Request_AUDIO_Q = osMessageCreate(osMessageQ(RTC676x_Tx_Request_AUDIO_Q), NULL);
#endif
#if defined(RTC676x_USING_UNCACHED_MALLOC)
	RTC676x_Tx_Request_Q = (osMessageQId *)osUncachedMalloc(sizeof(osMessageQId) * PKT_TYPE_MAX);
#endif
#if defined(RTC676x_USER_DEFINE_TRANSMIT_QUEUE_PRIORIT)
	RTC676x_Transmit_user_priority[PKT_TYPE_SMSG] = PRIORITY_HIGH + 1;
	RTC676x_Transmit_user_priority[PKT_TYPE_VIDEO] = PRIORITY_LOW + 1;
	RTC676x_Transmit_user_priority[PKT_TYPE_AUDIO] = PRIORITY_MIDDLE + 1;
	
	RTC676x_Tx_Request_Q[0] = RTC676x_Tx_Request_MGMT_Q;
	RTC676x_Tx_Request_Q[PRIORITY_HIGH + 1] = RTC676x_Tx_Request_Priority_High_Q;
	RTC676x_Tx_Request_Q[PRIORITY_MIDDLE + 1] = RTC676x_Tx_Request_Priority_Middle_Q;
	RTC676x_Tx_Request_Q[PRIORITY_LOW + 1] = RTC676x_Tx_Request_Priority_Low_Q;
#else
	RTC676x_Tx_Request_Q[PKT_TYPE_MGMT] = RTC676x_Tx_Request_MGMT_Q;
	RTC676x_Tx_Request_Q[PKT_TYPE_SMSG] = RTC676x_Tx_Request_SMSG_Q;
	RTC676x_Tx_Request_Q[PKT_TYPE_VIDEO] = RTC676x_Tx_Request_VIDEO_Q;
	RTC676x_Tx_Request_Q[PKT_TYPE_AUDIO] = RTC676x_Tx_Request_AUDIO_Q;
#endif
#else /* RTC676x_TRANSMIT_REQUEST_MULTI_Q */
	RTC676x_Tx_Request_Q = osMessageCreate(osMessageQ(RTC676x_Tx_Request_Q), NULL);
#endif
	RTC676x_Tx_Wait_Ack_Q = osMessageCreate(osMessageQ(RTC676x_Tx_Wait_Ack_Q), NULL);
	RTC676x_DMA_Request_Q = osMessageCreate(osMessageQ(RTC676x_DMA_Request_Q), NULL);
	RTC676x_Recv_MGMT_Q = osMessageCreate(osMessageQ(RTC676x_Recv_MGMT_Q), NULL);
	RTC676x_Recv_SMSG_Q = osMessageCreate(osMessageQ(RTC676x_Recv_SMSG_Q), NULL);
	RTC676x_Recv_VIDEO_Q = osMessageCreate(osMessageQ(RTC676x_Recv_VIDEO_Q), NULL);
	RTC676x_Recv_AUDIO_Q = osMessageCreate(osMessageQ(RTC676x_Recv_AUDIO_Q), NULL);

	//! create semaphore
	RTC676x_Reg_RW_Sema = osSemaphoreCreate(osSemaphore(RTC676x_Reg_RW_Sema), 1);
	RTC676x_Receive_MGMT_Sema = osSemaphoreCreate(osSemaphore(RTC676x_Receive_MGMT_Sema), 1);
	RTC676x_Receive_SMSG_Sema = osSemaphoreCreate(osSemaphore(RTC676x_Receive_SMSG_Sema), 1);
	RTC676x_Receive_VIDEO_Sema = osSemaphoreCreate(osSemaphore(RTC676x_Receive_VIDEO_Sema), 1);
	RTC676x_Receive_AUDIO_Sema = osSemaphoreCreate(osSemaphore(RTC676x_Receive_AUDIO_Sema), 1);
	RTC676x_Mac_Wait_Beacom_Timeout_Sema = osSemaphoreCreate(osSemaphore(RTC676x_Mac_Wait_Beacom_Timeout_Sema), 1);
	if (osSemaphoreWait(RTC676x_Receive_MGMT_Sema, 3) == -1)
		printf("RTC676x_SPI: RTC676x_Receive_MGMT_Sema fail\n");
	if (osSemaphoreWait(RTC676x_Receive_SMSG_Sema, 3) == -1)
		printf("RTC676x_SPI: RTC676x_Receive_SMSG_Sema fail\n");
	if (osSemaphoreWait(RTC676x_Receive_VIDEO_Sema, 3) == -1)
		printf("RTC676x_SPI: RTC676x_Receive_VIDEO_Sema fail\n");
	if (osSemaphoreWait(RTC676x_Receive_AUDIO_Sema, 3) == -1)
		printf("RTC676x_SPI: RTC676x_Receive_AUDIO_Sema fail\n");
	if (osSemaphoreWait(RTC676x_Mac_Wait_Beacom_Timeout_Sema, 3) == -1)
		printf("RTC676x_SPI: RTC676x_Mac_Wait_Beacom_Timeout_Sema fail\n");
#if !defined(RTC676x_WAIT_ACK_BY_EVENT)
	int ret;
	RTC676x_Wait_MGMT_Ack_Sem = osSemaphoreCreate(osSemaphore(RTC676x_Wait_MGMT_Ack_Sem), 1);
	RTC676x_Wait_SMSG_Ack_Sem = osSemaphoreCreate(osSemaphore(RTC676x_Wait_SMSG_Ack_Sem), 1);
	RTC676x_Wait_VIDEO_Ack_Sem = osSemaphoreCreate(osSemaphore(RTC676x_Wait_VIDEO_Ack_Sem), 1);
	RTC676x_Wait_AUDIO_Ack_Sem = osSemaphoreCreate(osSemaphore(RTC676x_Wait_AUDIO_Ack_Sem), 1);
	ret = osSemaphoreWait(RTC676x_Wait_MGMT_Ack_Sem, 3);
	if (ret == -1)
		printf("RTC676x_SPI: RTC676x_Wait_MGMT_Ack_Sem fail\n");
	else
		printf("RTC676x_SPI: RTC676x_Wait_MGMT_Ack_Sem ret:%d\n", ret);
	osSemaphoreRelease(RTC676x_Wait_MGMT_Ack_Sem);
	ret = osSemaphoreWait(RTC676x_Wait_MGMT_Ack_Sem, 3);
	printf("RTC676x_SPI: RTC676x_Wait_MGMT_Ack_Sem __ret:%d\n", ret);
	if (osSemaphoreWait(RTC676x_Wait_SMSG_Ack_Sem, 3) == -1)
		printf("RTC676x_SPI: RTC676x_Wait_SMSG_Ack_Sem fail\n");
	if (osSemaphoreWait(RTC676x_Wait_VIDEO_Ack_Sem, 3) == -1)
		printf("RTC676x_SPI: RTC676x_Wait_VIDEO_Ack_Sem fail\n");
	if (osSemaphoreWait(RTC676x_Wait_AUDIO_Ack_Sem, 3) == -1)
		printf("RTC676x_SPI: RTC676x_Wait_AUDIO_Ack_Sem fail\n");
#if defined(RTC676x_USING_UNCACHED_MALLOC)
	RTC676x_Wait_Ack_Sem = (osSemaphoreId *)osUncachedMalloc(sizeof(osSemaphoreId) * PKT_TYPE_MAX);
#endif
	RTC676x_Wait_Ack_Sem[PKT_TYPE_MGMT] = RTC676x_Wait_MGMT_Ack_Sem;
	RTC676x_Wait_Ack_Sem[PKT_TYPE_SMSG] = RTC676x_Wait_SMSG_Ack_Sem;
	RTC676x_Wait_Ack_Sem[PKT_TYPE_VIDEO] = RTC676x_Wait_VIDEO_Ack_Sem;
	RTC676x_Wait_Ack_Sem[PKT_TYPE_AUDIO] = RTC676x_Wait_AUDIO_Ack_Sem;
#endif
	RTC676x_Timeout_Sema = osSemaphoreCreate(osSemaphore(RTC676x_Timeout_Sema), 0);
	RTC676x_Tx_Clear_Wait_Sema = osSemaphoreCreate(osSemaphore(RTC676x_Tx_Clear_Wait_Sema), 1);
	if (osSemaphoreWait(RTC676x_Tx_Clear_Wait_Sema, 1) == -1)
		printf("RTC676x_SPI: RTC676x_Tx_Clear_Wait_Sema fail\n");
	RTC676x_Tx_Temp_Stop_Done_Sema = osSemaphoreCreate(osSemaphore(RTC676x_Tx_Temp_Stop_Done_Sema), 1);
	if (osSemaphoreWait(RTC676x_Tx_Temp_Stop_Done_Sema, 1) == -1)
		printf("RTC676x_SPI: RTC676x_Tx_Temp_Stop_Done_Sema fail\n");

	//! create mutex
	RTC676x_Tx_Ack_List_MGMT_Mutex = osMutexCreate(osMutex(RTC676x_Tx_Ack_List_MGMT_Mutex));
	RTC676x_Tx_Ack_List_SMSG_Mutex = osMutexCreate(osMutex(RTC676x_Tx_Ack_List_SMSG_Mutex));
	RTC676x_Tx_Ack_List_VIDEO_Mutex = osMutexCreate(osMutex(RTC676x_Tx_Ack_List_VIDEO_Mutex));
	RTC676x_Tx_Ack_List_AUDIO_Mutex = osMutexCreate(osMutex(RTC676x_Tx_Ack_List_AUDIO_Mutex));
#if defined(RTC676x_USING_UNCACHED_MALLOC)
	RTC676x_Tx_Ack_List_Mutex = (osMutexId *)osUncachedMalloc(sizeof(osMutexId) * PKT_TYPE_MAX);
#endif
	RTC676x_Tx_Ack_List_Mutex[PKT_TYPE_MGMT] = RTC676x_Tx_Ack_List_MGMT_Mutex;
	RTC676x_Tx_Ack_List_Mutex[PKT_TYPE_SMSG] = RTC676x_Tx_Ack_List_SMSG_Mutex;
	RTC676x_Tx_Ack_List_Mutex[PKT_TYPE_VIDEO] = RTC676x_Tx_Ack_List_VIDEO_Mutex;
	RTC676x_Tx_Ack_List_Mutex[PKT_TYPE_AUDIO] = RTC676x_Tx_Ack_List_AUDIO_Mutex;
	RTC676x_Tx_Global_Mutex = osMutexCreate(osMutex(RTC676x_Tx_Global_Mutex));

	//! allocate memory
	ubRegReadRData = osUncachedMalloc(16);
	ubRegReadWData = osUncachedMalloc(16);
	ubRegWriteRData = osUncachedMalloc(16);
	ubRegWriteWData = osUncachedMalloc(16);

#if defined(RTC676x_USING_UNCACHED_MALLOC)
	RTC676x_Tx_Ack_List = (uint64_t *)osUncachedMalloc(sizeof(uint64_t) * PKT_TYPE_MAX);
	RTC676x_Tx_Ack_List_Mask = (uint64_t *)osUncachedMalloc(sizeof(uint64_t) * PKT_TYPE_MAX);
	RTC676x_Tx_Ack_List_Count = (uint32_t *)osUncachedMalloc(sizeof(uint32_t) * PKT_TYPE_MAX);

	RTC676x_Rx_Overrun_State = (int32_t *)osUncachedMalloc(sizeof(int32_t) * PKT_TYPE_MAX);
	
	

	memset(RTC676x_Tx_Ack_List, 0x00, sizeof(uint64_t) * PKT_TYPE_MAX);
	memset(RTC676x_Tx_Ack_List_Mask, 0x00, sizeof(uint64_t) * PKT_TYPE_MAX);
	memset(RTC676x_Tx_Ack_List_Count, 0, sizeof(uint32_t) * PKT_TYPE_MAX);
	memset(RTC676x_Rx_Overrun_State, 0x00, sizeof(int32_t) * PKT_TYPE_MAX);
#endif

#if defined(RTC676x_USING_UNCACHED_MALLOC)
	ubRfStatus = osUncachedMalloc(RTC676x_REG_STATUS_LENGTH + 1);
	ubRfStatusRead = osUncachedMalloc(RTC676x_REG_STATUS_LENGTH + 1);
	ubTimStatus = osUncachedMalloc(RTC676x_REG_TIM_STATIS_LENGTH + 1);
	ubTimStatusRead = osUncachedMalloc(RTC676x_REG_TIM_STATIS_LENGTH + 1);

	memset(ubRfStatus, 0, RTC676x_REG_STATUS_LENGTH + 1);
	memset(ubTimStatus, 0, RTC676x_REG_TIM_STATIS_LENGTH + 1);

	ubRfStatus[0] = RTC676x_REG_STATUS | RTC676x_SPI_REG_OFFSET_READ_BIT;
	ubTimStatus[0] = RTC676x_REG_TIM_STATUS | RTC676x_SPI_REG_OFFSET_READ_BIT;
#if defined(RTC676x_6765)
	ubConfig3 = osUncachedMalloc(RTC676x_REG_CONFIG3_LENGTH + 1);
	ubConfig3Read= osUncachedMalloc(RTC676x_REG_CONFIG3_LENGTH + 1);
#else /*6763*/
	ubConfig3 = osUncachedMalloc(RTC676x_REG_CONFIG3_LENGTH + 1);
	ubConfig3Read = osUncachedMalloc(RTC676x_REG_CONFIG3_LENGTH + 1);
#endif
	memset(ubConfig3, 0, RTC676x_REG_CONFIG3_LENGTH + 1);
	ubConfig3[0] = RTC676x_REG_CONFIG3 | RTC676x_SPI_REG_OFFSET_READ_BIT;

	ubRfStatusClean = osUncachedMalloc(RTC676x_REG_STATUS_LENGTH + 1);
	ubTimStatusClean = osUncachedMalloc(RTC676x_REG_STATUS_LENGTH + 1);
	ubRegReadDummy = osUncachedMalloc(RTC676x_REG_CONFIG3_LENGTH + 1);
	ubRfStatusClean[0] = RTC676x_REG_STATUS;
	ubTimStatusClean[0] = RTC676x_REG_TIM_STATUS;

#if defined(RTC676x_SPI_MAC_READ_PRE_DO)
	ubRecvHeader = osUncachedMalloc(64);
	ubRecvHeaderRead = osUncachedMalloc(64);
#else
	ubRecvHeader = osUncachedMalloc(5);
	ubRecvHeaderRead = osUncachedMalloc(5);
#endif

	ubDbgConf = osUncachedMalloc(RTC676x_REG_DBGCONF_LENGTH + 1);
	ubDbgConfRead = osUncachedMalloc(RTC676x_REG_DBGCONF_LENGTH + 1);
#endif

	pgRtc676xData = pRtc676xData;

#if defined(RTC676x_CHECK_TIMER)
	RTC676x_Check_Timer = osTimerCreate(osTimer(RTC676x_Check_Timer), osTimerPeriodic, NULL);
//	osTimerStart(RTC676x_Check_Timer, RTC676x_CHECK_TIMER);
	if (tRFDevice == RTC676x_RF_Master)
		pRtc676xData->ulCheckTimerInterval = RTC676x_CHECK_TIMER;
	else
		pRtc676xData->ulCheckTimerInterval = RTC676x_CHECK_TIMER_SLAVE;
	osTimerStart(RTC676x_Check_Timer, pRtc676xData->ulCheckTimerInterval);
#endif

#if defined(RTC676x_DEBUG_TIMER)
	RTC676x_Debug_Timer = osTimerCreate(osTimer(RTC676x_Debug_Timer), osTimerPeriodic, NULL);
	osTimerStart(RTC676x_Debug_Timer, RTC676x_DEBUG_TIMER);
#endif

	pRtc676xData->checkCrcMode = CRC_CHECK_ONLY;

    if(RTC676x_SPI_ThreadId == NULL)
    {
        RTC676x_SPI_ThreadId = osThreadCreate(osThread(RTC676x_SPI_SlavePolling), pRtc676xData);
        if(RTC676x_SPI_ThreadId == NULL)
        {
            printd(DBG_CriticalLvl, "RTC676x: Create RTC676x_Thread fail !!!!\r\n");
            while(1);
        }
    }
//    INTC_IrqSetup(INTC_SSP_IRQ, INTC_EDGE_TRIG, RTC676x_ISR);

	RTC676x_Reset();

}
//------------------------------------------------------------------------------
#if 0
#if defined(VBM_PU)   /*LCD : RX*/
#define	RTC676x_SPI_CLK_PADIO		GLB->PADIO47
#define	RTC676x_SPI_CS_PADIO		GLB->PADIO48
#define	RTC676x_SPI_TX_PADIO		GLB->PADIO54
#define	RTC676x_SPI_RX_PADIO		GLB->PADIO55

#define	RTC676x_IRQ_PADIO			GLB->PADIO10

#define	RTC676x_RESET_PADIO			GLB->PADIO56

#define	RTC676x_IRQ_OUT_EN			GPIO->GPIO_OE10
#define	RTC676x_IRQ_PULL_HIGH		GPIO->GPIO_PULL_HIGH10
#define	RTC676x_IRQ_PULL_EN			GPIO->GPIO_PULL_EN10
#define	RTC676x_IRQ_TRG_MODE		GPIO->GPIO_TRG_MODE10
#define	RTC676x_IRQ_TRG_EDGE		GPIO->GPIO_TRG_EDGE10
#define	RTC676x_IRQ_TRG_LEVEL		GPIO->GPIO_TRG_LEVEL10
#define	RTC676x_IRQ_CLR_INTR		GPIO->CLR_GPIO_INTR10
#define	RTC676x_IRQ_INTR_MASK		GPIO->GPIO_INTR_MSK10
#define	RTC676x_IRQ_INTR_EN			GPIO->GPIO_INTR_EN10
#define	RTC676x_IRQ_INPUT			GPIO->GPIO_I10
#define	RTC676x_IRQ_INTR_FLAG		GPIO->GPIO_INTR_FLAG10
#define	RTC676x_IRQ_OUT				GPIO->GPIO_O10

#define	RTC676x_RESET_OUT_EN		GPIO->GPIO_OE0
#define	RTC676x_RESET_OUT			GPIO->GPIO_O0

#define	RTC676x_PADIO_TX_SPI_MODE	2
#define	RTC676x_PADIO_RX_SPI_MODE	2

#elif defined(VBM_BU) /*Camera : TX*/

#define	RTC676x_SPI_CLK_PADIO		GLB->PADIO33
#define	RTC676x_SPI_CS_PADIO		GLB->PADIO34
#define	RTC676x_SPI_TX_PADIO		GLB->PADIO35
#define	RTC676x_SPI_RX_PADIO		GLB->PADIO36

#define	RTC676x_IRQ_PADIO			GLB->PADIO43

#define	RTC676x_RESET_PADIO			GLB->PADIO44

#define	RTC676x_IRQ_OUT_EN			GPIO->GPIO_OE1
#define	RTC676x_IRQ_PULL_HIGH		GPIO->GPIO_PULL_HIGH1
#define	RTC676x_IRQ_PULL_EN			GPIO->GPIO_PULL_EN1
#define	RTC676x_IRQ_TRG_MODE		GPIO->GPIO_TRG_MODE1
#define	RTC676x_IRQ_TRG_EDGE		GPIO->GPIO_TRG_EDGE1
#define	RTC676x_IRQ_TRG_LEVEL		GPIO->GPIO_TRG_LEVEL1
#define	RTC676x_IRQ_CLR_INTR		GPIO->CLR_GPIO_INTR1
#define	RTC676x_IRQ_INTR_MASK		GPIO->GPIO_INTR_MSK1
#define	RTC676x_IRQ_INTR_EN			GPIO->GPIO_INTR_EN1
#define	RTC676x_IRQ_INPUT			GPIO->GPIO_I1
#define	RTC676x_IRQ_INTR_FLAG		GPIO->GPIO_INTR_FLAG1
#define	RTC676x_IRQ_OUT				GPIO->GPIO_O1

#define	RTC676x_RESET_OUT_EN		GPIO->GPIO_OE2
#define	RTC676x_RESET_OUT			GPIO->GPIO_O2

#define	RTC676x_PADIO_TX_SPI_MODE	1
#define	RTC676x_PADIO_RX_SPI_MODE	1

#endif

#define	RTC676x_SPI_CS_OUT_EN		GPIO->GPIO_OE6
#define	RTC676x_SPI_CS_OUT			GPIO->GPIO_O6

#endif

#if 0
void RTC676x_GPIO_Init(void)
{
#if defined(VBM_PU)   /*LCD : RX*/
	GLB->PADIO50 = 3;
	GLB->PADIO49 = 3;
#elif defined(VBM_BU) /*Camera : TX*/
	GLB->PADIO29 = 3;
	GLB->PADIO57 = 3;
	GLB->PADIO71 = 3;
#endif

	// IRQ_N
	RTC676x_IRQ_PADIO = 0;
	RTC676x_IRQ_OUT_EN = 0;

	// RF_RST
	RTC676x_RESET_PADIO = 0;
	RTC676x_RESET_OUT_EN = 1;
	RTC676x_RESET_OUT = 1;

	// RF_INT
	//GPIO->GPIO_OE1 = 1;
	//GPIO->GPIO_O1 = 1;
	RTC676x_IRQ_OUT_EN = 0;
	//GPIO->GPIO_INTR_EN1 = 1;
	RTC676x_IRQ_PULL_HIGH = 1;
	RTC676x_IRQ_PULL_EN = 1;
	//GPIO->GPIO_PULL_HIGH1 = 1;
	INTC_IrqSetup(INTC_GPIO_IRQ, RTC676x_RF_ISR);
	INTC_IrqClear(INTC_GPIO_IRQ);
	INTC_IrqEnable(INTC_GPIO_IRQ);
	RTC676x_IRQ_TRG_MODE = 0;
	RTC676x_IRQ_TRG_EDGE = 0;
	RTC676x_IRQ_TRG_LEVEL = 1;
	RTC676x_IRQ_CLR_INTR = 1;
	RTC676x_IRQ_INTR_MASK = 0;

	// SPI Pad mux setting
	RTC676x_SPI_CLK_PADIO = 1;
	RTC676x_SPI_CS_PADIO = 0;
	RTC676x_SPI_TX_PADIO = RTC676x_PADIO_TX_SPI_MODE;
	RTC676x_SPI_RX_PADIO = RTC676x_PADIO_RX_SPI_MODE;

	//CS pin; both TH and RX are GPIO6
	RTC676x_SPI_CS_OUT_EN = 1;
	RTC676x_SPI_CS_OUT = 1;
}
#endif
//------------------------------------------------------------------------------
int RTC676x_DMA_Buffer_Init(uint32_t poll_ptr, RTC676x_RF_Device tRFDevice)
{
	uint32_t poll_offset = 0;
#if defined(RTC676x_DMA_QUEUE_UNCACHED_MALLOC)

	int i;
	uint8_t *pbDmaBufTemp;

#if defined(RTC676x_DMA_QUEUE_OS_MSG_Q_MGMT)
	osMessageQDef(RTC676x_DMA_Big_Size_MSGQ, RTC676x_DMA_Queue_Size, RTC676x_DMA_BUFFER_MGMT_t);
	osMessageQDef(RTC676x_DMA_Small_Size_MSGQ, RTC676x_DMA_Queue_Size, RTC676x_DMA_BUFFER_MGMT_t);

	RTC676x_DMA_Big_Size_MSGQ = osMessageCreate(osMessageQ(RTC676x_DMA_Big_Size_MSGQ), NULL);
	RTC676x_DMA_Small_Size_MSGQ = osMessageCreate(osMessageQ(RTC676x_DMA_Small_Size_MSGQ), NULL);
#else
	RTC676x_DMA_Big_Size_Sema = osSemaphoreCreate(osSemaphore(RTC676x_DMA_Big_Size_Sema), RTC676x_DMA_Queue_Size);
	RTC676x_DMA_Small_Size_Sema = osSemaphoreCreate(osSemaphore(RTC676x_DMA_Small_Size_Sema), RTC676x_DMA_Queue_Size);
#endif

	RTC676x_DMA_Transmit_Mutex = osMutexCreate(osMutex(RTC676x_DMA_Transmit_Mutex));
	RTC676x_DMA_Receive_Mutex = osMutexCreate(osMutex(RTC676x_DMA_Receive_Mutex));

#if defined(RTC676x_DMA_QUEUE_OS_MSG_Q_MGMT)
#else
	RTC676x_DMA_Transmit_Queue_Consumer = RTC676x_DMA_Transmit_Queue_Producer = 0;
	RTC676x_DMA_Receive_Queue_Consumer = RTC676x_DMA_Receive_Queue_Producer = 0;
#endif

	RTC676x_DMA_Big_Pkt_Queue = (uint8_t **)(poll_ptr + poll_offset);
	poll_offset += sizeof(uint8_t *) * (RTC676x_DMA_Queue_Size + 2); /*extra 1 : under bound, extra 2 : upper bound*/
	RTC676x_DMA_Small_Pkt_Queue = (uint8_t **)(poll_ptr + poll_offset);
	poll_offset += sizeof(uint8_t *) * (RTC676x_DMA_Queue_Size + 2); /*extra 1 : under bound, extra 2 : upper bound*/

	//pbDmaBufTemp = osUncachedCalloc(RTC676x_DMA_Queue_MAX, RTC676x_SPI_PACKET_BIG_SIZE);
	pbDmaBufTemp = (uint8_t *)(poll_ptr + poll_offset);
	poll_offset += RTC676x_DMA_Queue_Size * RTC676x_SPI_PACKET_BIG_SIZE;

	for (i = 0; i < RTC676x_DMA_Queue_Size; i ++)
	{
		RTC676x_DMA_Big_Pkt_Queue[i] = pbDmaBufTemp + (i * RTC676x_SPI_PACKET_BIG_SIZE);
	//	osSemaphoreRelease(RTC676x_DMA_Big_Size_Sema);
#if defined(RTC676x_DMA_QUEUE_OS_MSG_Q_MGMT)
		{
			RTC676x_DMA_BUFFER_MGMT_t dma_buffer_mgmt = {.pubBuffer = RTC676x_DMA_Big_Pkt_Queue[i]};
			osMessagePut(RTC676x_DMA_Big_Size_MSGQ , &dma_buffer_mgmt, 0);
		}
#endif
	}
	RTC676x_DMA_Big_Pkt_Queue[RTC676x_DMA_Queue_Size] = RTC676x_DMA_Big_Pkt_Queue[0];
	RTC676x_DMA_Big_Pkt_Queue[RTC676x_DMA_Queue_Size + 1] = RTC676x_DMA_Big_Pkt_Queue[RTC676x_DMA_Queue_Size - 1];

	//pbDmaBufTemp = osUncachedCalloc(RTC676x_DMA_Queue_MAX, RTC676x_SPI_PACKET_SMALL_SIZE);
	pbDmaBufTemp = (uint8_t *)(poll_ptr + poll_offset);
	poll_offset += RTC676x_DMA_Queue_Size * RTC676x_SPI_PACKET_SMALL_SIZE;

	for (i = 0; i < RTC676x_DMA_Queue_Size; i ++)
	{
		RTC676x_DMA_Small_Pkt_Queue[i] = pbDmaBufTemp + (i * RTC676x_SPI_PACKET_SMALL_SIZE);
	//	osSemaphoreRelease(RTC676x_DMA_Small_Size_Sema);
#if defined(RTC676x_DMA_QUEUE_OS_MSG_Q_MGMT)
		{
			RTC676x_DMA_BUFFER_MGMT_t dma_buffer_mgmt = {.pubBuffer = RTC676x_DMA_Small_Pkt_Queue[i]};
			osMessagePut(RTC676x_DMA_Small_Size_MSGQ , &dma_buffer_mgmt, 0);
		}
#endif
	}
	RTC676x_DMA_Small_Pkt_Queue[RTC676x_DMA_Queue_Size] = RTC676x_DMA_Small_Pkt_Queue[0];
	RTC676x_DMA_Small_Pkt_Queue[RTC676x_DMA_Queue_Size + 1] = RTC676x_DMA_Small_Pkt_Queue[RTC676x_DMA_Queue_Size - 1];

	if (tRFDevice == RTC676x_RF_Master)
	{

		RTC676x_DMA_Transmit_Queue = (uint32_t *)RTC676x_DMA_Small_Pkt_Queue;
		RTC676x_DMA_Receive_Queue = (uint32_t *)RTC676x_DMA_Big_Pkt_Queue;

#if defined(RTC676x_DMA_QUEUE_OS_MSG_Q_MGMT)
		RTC676x_DMA_Transmit_MSGQ = RTC676x_DMA_Small_Size_MSGQ;
		RTC676x_DMA_Receive_MSGQ = RTC676x_DMA_Big_Size_MSGQ;
#else
		RTC676x_DMA_Transmit_Sema = RTC676x_DMA_Small_Size_Sema;
		RTC676x_DMA_Receive_Sema = RTC676x_DMA_Big_Size_Sema;
#endif

		RTC676x_DMA_Transmit_Buffer_Size = RTC676x_SPI_PACKET_SMALL_SIZE;
		RTC676x_DMA_Receive_Buffer_Size = RTC676x_SPI_PACKET_BIG_SIZE;
	}
	else
	{
		RTC676x_DMA_Transmit_Queue = (uint32_t *)RTC676x_DMA_Big_Pkt_Queue;
		RTC676x_DMA_Receive_Queue = (uint32_t *)RTC676x_DMA_Small_Pkt_Queue;

#if defined(RTC676x_DMA_QUEUE_OS_MSG_Q_MGMT)
		RTC676x_DMA_Transmit_MSGQ = RTC676x_DMA_Big_Size_MSGQ;
		RTC676x_DMA_Receive_MSGQ = RTC676x_DMA_Small_Size_MSGQ;
#else
		RTC676x_DMA_Transmit_Sema = RTC676x_DMA_Big_Size_Sema;
		RTC676x_DMA_Receive_Sema = RTC676x_DMA_Small_Size_Sema;
#endif

		RTC676x_DMA_Transmit_Buffer_Size = RTC676x_SPI_PACKET_BIG_SIZE;
		RTC676x_DMA_Receive_Buffer_Size = RTC676x_SPI_PACKET_SMALL_SIZE;
	}

	//for (i = 0; i < RTC676x_DMA_Queue_MAX; i ++)
	//	printf("[RF] TDMA ptr:%08X\n", RTC676x_DMA_Transmit_Queue[i]);

	//for (i = 0; i < RTC676x_DMA_Queue_MAX; i ++)
	//	printf("[RF] RDMA ptr:%08X\n", RTC676x_DMA_Receive_Queue[i]);
#endif

	//printf("[RF] allocate each %u buffers to transmit/receive\n", RTC676x_DMA_Queue_Size);
	printf("[RF]:%u Buf to T/R\n", RTC676x_DMA_Queue_Size);

	return poll_offset;
}
//------------------------------------------------------------------------------
uint32_t RTC676x_DMA_Transmit_Buffer_Get_Total_Number(void)
{
	return RTC676x_DMA_Queue_Size;
}
//------------------------------------------------------------------------------
uint32_t RTC676x_DMA_Transmit_Buffer_Get_Free_Number(void)
{
	uint32_t free_number = 0;
#if defined(RTC676x_DMA_QUEUE_UNCACHED_MALLOC)
#if defined(RTC676x_DMA_QUEUE_OS_MSG_Q_MGMT)
	free_number = osMessages(RTC676x_DMA_Transmit_MSGQ);
#else
	int i;

	osMutexWait(RTC676x_DMA_Transmit_Mutex, osWaitForever);
	for (i = 0; i < RTC676x_DMA_Queue_Size; i ++)
	{
		if (RTC676x_DMA_Transmit_Queue[i] != 0)
			free_number ++;
	}
	osMutexRelease(RTC676x_DMA_Transmit_Mutex);
#endif
#endif
	return free_number;
}
//------------------------------------------------------------------------------
uint8_t* RTC676x_DMA_Transmit_Buffer_Alloc(uint32_t ulTimeoutMSec)
{
	uint8_t *pbDmaBuf = NULL;
#if defined(RTC676x_DMA_QUEUE_UNCACHED_MALLOC)
	osMutexWait(RTC676x_DMA_Transmit_Mutex, osWaitForever);
#if defined(RTC676x_DMA_QUEUE_OS_MSG_Q_MGMT)
	{
		osStatus ret;
		RTC676x_DMA_BUFFER_MGMT_t dma_buffer;

#if 0 //don't wait forever here
		do
		{
			ret = osMessageGet(RTC676x_DMA_Transmit_MSGQ, &dma_buffer, osWaitForever);
		} while(ret != osEventMessage);
		pbDmaBuf = dma_buffer.pubBuffer;
#else
		ret = osMessageGet(RTC676x_DMA_Transmit_MSGQ, &dma_buffer, ((ulTimeoutMSec == 0) || (ulTimeoutMSec == osWaitForever) || (ulTimeoutMSec < 500)) ? 500 : ulTimeoutMSec);
		if (ret == osEventMessage)
			pbDmaBuf = dma_buffer.pubBuffer;
#endif
	}
#else
#if 0 //don't wait forever here
	osSemaphoreWait(RTC676x_DMA_Transmit_Sema, osWaitForever);

	pbDmaBuf = (uint8_t *)RTC676x_DMA_Transmit_Queue[RTC676x_DMA_Transmit_Queue_Consumer];
	RTC676x_DMA_Transmit_Queue[RTC676x_DMA_Transmit_Queue_Consumer] = 0;
	RTC676x_DMA_Transmit_Queue_Consumer = (RTC676x_DMA_Transmit_Queue_Consumer + 1) % RTC676x_DMA_Queue_Size;
#else
	if (osSemaphoreWait(RTC676x_DMA_Transmit_Sema, ((ulTimeoutMSec == 0) || (ulTimeoutMSec == osWaitForever)) ? 7 /* 1.67ms (600/s) * max 4 TX */ : ulTimeoutMSec) == osOK)
	{
		
		pbDmaBuf = (uint8_t *)RTC676x_DMA_Transmit_Queue[RTC676x_DMA_Transmit_Queue_Consumer];
		RTC676x_DMA_Transmit_Queue[RTC676x_DMA_Transmit_Queue_Consumer] = 0;
		RTC676x_DMA_Transmit_Queue_Consumer = (RTC676x_DMA_Transmit_Queue_Consumer + 1) % RTC676x_DMA_Queue_Size;
	}
#endif
#endif
	osMutexRelease(RTC676x_DMA_Transmit_Mutex);
#endif
	return pbDmaBuf;
}
//------------------------------------------------------------------------------
uint8_t* RTC676x_DMA_Receive_Buffer_Alloc(void)
{
	uint8_t *pbDmaBuf = NULL;
#if defined(RTC676x_DMA_QUEUE_UNCACHED_MALLOC)
	osMutexWait(RTC676x_DMA_Receive_Mutex, osWaitForever);
#if defined(RTC676x_DMA_QUEUE_OS_MSG_Q_MGMT)
	{
		osStatus ret;
		RTC676x_DMA_BUFFER_MGMT_t dma_buffer;

		do
		{
			ret = osMessageGet(RTC676x_DMA_Receive_MSGQ, &dma_buffer, osWaitForever);
		} while(ret != osEventMessage);
		pbDmaBuf = dma_buffer.pubBuffer;
	}
#else
	osSemaphoreWait(RTC676x_DMA_Receive_Sema, osWaitForever);

	pbDmaBuf = (uint8_t *)RTC676x_DMA_Receive_Queue[RTC676x_DMA_Receive_Queue_Consumer];
	RTC676x_DMA_Receive_Queue[RTC676x_DMA_Receive_Queue_Consumer] = 0;
	RTC676x_DMA_Receive_Queue_Consumer = (RTC676x_DMA_Receive_Queue_Consumer + 1) % RTC676x_DMA_Queue_Size;
#endif
	osMutexRelease(RTC676x_DMA_Receive_Mutex);
#endif
	return pbDmaBuf;
}
//------------------------------------------------------------------------------
void RTC676x_DMA_Transmit_Buffer_Free(uint8_t *pbDmaBuf)
{
#if defined(RTC676x_DMA_QUEUE_UNCACHED_MALLOC)
	if ((pbDmaBuf >= (uint8_t *)RTC676x_DMA_Transmit_Queue[RTC676x_DMA_Queue_Size]) && (pbDmaBuf <= (uint8_t *)RTC676x_DMA_Transmit_Queue[RTC676x_DMA_Queue_Size + 1]))
	{
#if defined(RTC676x_DMA_QUEUE_OS_MSG_Q_MGMT)
		RTC676x_DMA_BUFFER_MGMT_t dma_buffer = {.pubBuffer = pbDmaBuf};
		osMessagePut(RTC676x_DMA_Transmit_MSGQ, &dma_buffer, 0);
#else
		if (RTC676x_DMA_Transmit_Queue[RTC676x_DMA_Transmit_Queue_Producer] == 0)
		{
			RTC676x_DMA_Transmit_Queue[RTC676x_DMA_Transmit_Queue_Producer] = (uint32_t)pbDmaBuf;
			RTC676x_DMA_Transmit_Queue_Producer = (RTC676x_DMA_Transmit_Queue_Producer + 1) % RTC676x_DMA_Queue_Size;

			osSemaphoreRelease(RTC676x_DMA_Transmit_Sema);
		}
#endif
	}
#endif
}
//------------------------------------------------------------------------------
void RTC676x_DMA_Receive_Buffer_Free(uint8_t *pbDmaBuf)
{
#if defined(RTC676x_DMA_QUEUE_UNCACHED_MALLOC)
	if ((pbDmaBuf >= (uint8_t *)RTC676x_DMA_Receive_Queue[RTC676x_DMA_Queue_Size]) && (pbDmaBuf <= (uint8_t *)RTC676x_DMA_Receive_Queue[RTC676x_DMA_Queue_Size + 1]))
	{
#if defined(RTC676x_DMA_QUEUE_OS_MSG_Q_MGMT)
		RTC676x_DMA_BUFFER_MGMT_t dma_buffer = {.pubBuffer = pbDmaBuf};
		osMessagePut(RTC676x_DMA_Receive_MSGQ, &dma_buffer, 0);
#else

		if (RTC676x_DMA_Receive_Queue[RTC676x_DMA_Receive_Queue_Producer] == 0)
		{
			RTC676x_DMA_Receive_Queue[RTC676x_DMA_Receive_Queue_Producer] = (uint32_t)pbDmaBuf;
			RTC676x_DMA_Receive_Queue_Producer = (RTC676x_DMA_Receive_Queue_Producer + 1) % RTC676x_DMA_Queue_Size;

			osSemaphoreRelease(RTC676x_DMA_Receive_Sema);
		}
#endif
	}
#endif
}
//------------------------------------------------------------------------------
int RTC676x_Recv_Queue_Init(uint32_t poll_ptr)
{
	uint32_t poll_offset = 0;

	int i, pkt_type;
	uint8_t *pbRecvBufTemp;

	RTC676x_Recv_Queue_Sema[PKT_TYPE_MGMT] = RTC676x_Recv_Queue_MGMT_Sema = osSemaphoreCreate(osSemaphore(RTC676x_Recv_Queue_MGMT_Sema), RTC676x_Recv_Queue_MAX);
	RTC676x_Recv_Queue_Sema[PKT_TYPE_SMSG] = RTC676x_Recv_Queue_SMSG_Sema = osSemaphoreCreate(osSemaphore(RTC676x_Recv_Queue_SMSG_Sema), RTC676x_Recv_Queue_MAX);
	RTC676x_Recv_Queue_Sema[PKT_TYPE_VIDEO] = RTC676x_Recv_Queue_VIDEO_Sema = osSemaphoreCreate(osSemaphore(RTC676x_Recv_Queue_VIDEO_Sema), RTC676x_Recv_Queue_MAX);
	RTC676x_Recv_Queue_Sema[PKT_TYPE_AUDIO] = RTC676x_Recv_Queue_AUDIO_Sema = osSemaphoreCreate(osSemaphore(RTC676x_Recv_Queue_AUDIO_Sema), RTC676x_Recv_Queue_MAX);

	RTC676x_Recv_Queue_Mutex[PKT_TYPE_MGMT] = RTC676x_Recv_Queue_MGMT_Mutex = osMutexCreate(osMutex(RTC676x_Recv_Queue_MGMT_Mutex));
	RTC676x_Recv_Queue_Mutex[PKT_TYPE_SMSG] = RTC676x_Recv_Queue_SMSG_Mutex = osMutexCreate(osMutex(RTC676x_Recv_Queue_SMSG_Mutex));
	RTC676x_Recv_Queue_Mutex[PKT_TYPE_VIDEO] = RTC676x_Recv_Queue_VIDEO_Mutex = osMutexCreate(osMutex(RTC676x_Recv_Queue_VIDEO_Mutex));
	RTC676x_Recv_Queue_Mutex[PKT_TYPE_AUDIO] = RTC676x_Recv_Queue_AUDIO_Mutex = osMutexCreate(osMutex(RTC676x_Recv_Queue_AUDIO_Mutex));

	for (pkt_type = PKT_TYPE_MGMT; pkt_type < PKT_TYPE_MAX; pkt_type ++)
	{
		RTC676x_Recv_Queue_Consumer[pkt_type] = RTC676x_Recv_Queue_Producer[pkt_type] = RTC676x_Recv_Queue_Used[pkt_type] = 0;

		pbRecvBufTemp = (uint8_t *)(poll_ptr + poll_offset);
		poll_offset += RTC676x_Recv_Queue_MAX * RTC676x_RECV_PACKET_SIZE;

		for (i = 0; i < RTC676x_Recv_Queue_MAX; i ++)
		{
			RTC676x_Recv_Queue[pkt_type][i] = pbRecvBufTemp + (i * RTC676x_RECV_PACKET_SIZE);
			osSemaphoreWait(RTC676x_Recv_Queue_Sema[pkt_type], 0);
		}
	}

	return poll_offset;
}
//------------------------------------------------------------------------------
void RTC676x_Recv_Queue_Put(int32_t pkt_type, uint8_t *pbDmaBuf, int32_t length, int32_t *plOverrunState, bool bSkipIfOverwrite)
{
	osMutexWait(RTC676x_Recv_Queue_Mutex[pkt_type], osWaitForever);

	
	memcpy(RTC676x_Recv_Queue[pkt_type][RTC676x_Recv_Queue_Producer[pkt_type]], pbDmaBuf, length);
	RTC676x_Recv_Queue_Data_Len[pkt_type][RTC676x_Recv_Queue_Producer[pkt_type]] = length;

	RTC676x_Recv_Queue_Producer[pkt_type] = (RTC676x_Recv_Queue_Producer[pkt_type] + 1) % RTC676x_Recv_Queue_MAX;

	if (RTC676x_Recv_Queue_Used[pkt_type] < RTC676x_Recv_Queue_MAX)
	{
		RTC676x_Recv_Queue_Used[pkt_type] ++;
		osSemaphoreRelease(RTC676x_Recv_Queue_Sema[pkt_type]);
	}
	else
	{
		if (! bSkipIfOverwrite)
			RTC676x_Recv_Queue_Consumer[pkt_type] = (RTC676x_Recv_Queue_Consumer[pkt_type] + 1) % RTC676x_Recv_Queue_MAX;
		*plOverrunState = 1;
	}
	osMutexRelease(RTC676x_Recv_Queue_Mutex[pkt_type]);
}
//------------------------------------------------------------------------------
int RTC676x_Recv_Queue_Get(int pkt_type, uint8_t *pbBuf, uint32_t ulTimeoutMsec, uint32_t *plDataLen)
{
	int osRet;
	int ret = -1;
	osRet = osSemaphoreWait(RTC676x_Recv_Queue_Sema[pkt_type], ulTimeoutMsec);
	osMutexWait(RTC676x_Recv_Queue_Mutex[pkt_type], osWaitForever);
	if (osRet == osOK)
	{
		//Get last packet if overrun
		if (RTC676x_Rx_Overrun_State[pkt_type] == 1)
		{
			while(RTC676x_Recv_Queue_Used[pkt_type] > 1)
			{
				RTC676x_Recv_Queue_Consumer[pkt_type] = (RTC676x_Recv_Queue_Consumer[pkt_type] + 1) % RTC676x_Recv_Queue_MAX;
				RTC676x_Recv_Queue_Used[pkt_type] --;
				osSemaphoreWait(RTC676x_Recv_Queue_Sema[pkt_type], 0);
			}
		}

		if (*plDataLen < RTC676x_Recv_Queue_Data_Len[pkt_type][RTC676x_Recv_Queue_Consumer[pkt_type]])
		{
			//printf("[R] ty:%d RQ len:%u/%u\n", pkt_type, *plDataLen, RTC676x_Recv_Queue_Data_Len[pkt_type][RTC676x_Recv_Queue_Consumer[pkt_type]]);
			RTC676x_Recv_Queue_Data_Len[pkt_type][RTC676x_Recv_Queue_Consumer[pkt_type]] = *plDataLen;
			RTC676x_Recv_Queue_Consumer[pkt_type] = (RTC676x_Recv_Queue_Consumer[pkt_type] + 1) % RTC676x_Recv_Queue_MAX;
			RTC676x_Recv_Queue_Used[pkt_type] --;
			osMutexRelease(RTC676x_Recv_Queue_Mutex[pkt_type]);
			return ret;
		}
		
		*plDataLen = RTC676x_Recv_Queue_Data_Len[pkt_type][RTC676x_Recv_Queue_Consumer[pkt_type]];
		memcpy(pbBuf, RTC676x_Recv_Queue[pkt_type][RTC676x_Recv_Queue_Consumer[pkt_type]], RTC676x_Recv_Queue_Data_Len[pkt_type][RTC676x_Recv_Queue_Consumer[pkt_type]]);
		RTC676x_Recv_Queue_Consumer[pkt_type] = (RTC676x_Recv_Queue_Consumer[pkt_type] + 1) % RTC676x_Recv_Queue_MAX;
		RTC676x_Recv_Queue_Used[pkt_type] --;
		ret = 0;
	}
	osMutexRelease(RTC676x_Recv_Queue_Mutex[pkt_type]);

	return ret;
}
//------------------------------------------------------------------------------
void RTC676x_Transmit_Priority_Get(RTC676x_PKT_PRIORITY *messagePriority, RTC676x_PKT_PRIORITY *audioPriority, RTC676x_PKT_PRIORITY *videoPriority)
{
#if defined(RTC676x_USER_DEFINE_TRANSMIT_QUEUE_PRIORIT)
	*messagePriority = (RTC676x_PKT_PRIORITY)((RTC676x_Transmit_user_priority[TYPE_SMSG] == 0) ? PRIORITY_HIGH : RTC676x_Transmit_user_priority[TYPE_SMSG] - 1);
	*audioPriority = (RTC676x_PKT_PRIORITY)((RTC676x_Transmit_user_priority[TYPE_AUDIO] == 0) ? PRIORITY_HIGH : RTC676x_Transmit_user_priority[TYPE_AUDIO] - 1);
	*videoPriority = (RTC676x_PKT_PRIORITY)((RTC676x_Transmit_user_priority[TYPE_VIDEO] == 0) ? PRIORITY_HIGH : RTC676x_Transmit_user_priority[TYPE_VIDEO] - 1);
#else
	messagePriority = 0;
	*audioPriority = 0;
	*videoPriority = 0;
#endif
}
//------------------------------------------------------------------------------
int RTC676x_Transmit_Priority_Set(RTC676x_PKT_PRIORITY messagePriority, RTC676x_PKT_PRIORITY audioPriority, RTC676x_PKT_PRIORITY videoPriority)
{
	if (((int)messagePriority < (int)PRIORITY_HIGH) || ((int)messagePriority > (int)PRIORITY_LOW) ||
		((int)audioPriority < (int)PRIORITY_HIGH) || ((int)audioPriority > (int)PRIORITY_LOW) ||
		((int)videoPriority < (int)PRIORITY_HIGH) || ((int)videoPriority > (int)PRIORITY_LOW))
		return -1;

#if defined(RTC676x_USER_DEFINE_TRANSMIT_QUEUE_PRIORIT)
	RTC676x_Transmit_user_priority[TYPE_SMSG] = (int)messagePriority + 1;
	RTC676x_Transmit_user_priority[TYPE_AUDIO] = (int)audioPriority + 1;
	RTC676x_Transmit_user_priority[TYPE_VIDEO] = (int)videoPriority + 1;
#endif

	return 0;
}
#if defined(RTC676x_CHECK_CRC)
//------------------------------------------------------------------------------
RTC676x_CHECK_CRC_MODE RTC676x_CRC_Check_Mode_Get(void)
{
	return pgRtc676xData->checkCrcMode;
}
//------------------------------------------------------------------------------
int RTC676x_CRC_Check_Mode_Set(RTC676x_CHECK_CRC_MODE mode)
{
	if (((int)mode >= CRC_DONT_CHECK) && (mode <= CRC_CHECK_AND_DROP))
	{
		pgRtc676xData->checkCrcMode = mode;
		return 0;
	}

	return -1;
}
#endif
//------------------------------------------------------------------------------
void RTC676x_Debug_Print_Enable(bool enable)
{
	RTC676x_Debug_Print = enable;
}
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
void RTC676x_Reset(void)
{
	RTC676x_MEAS message;
	//osStatus ret;

	message.ulEvent = RTC676x_SPI_RF_RESET;

	osMessagePut(RTC676x_Msg_Q, &message, osWaitForever);
}
//------------------------------------------------------------------------------
int RTC676x_Packet_Size_Support_Get(RTC676x_PKT_SIZE_SUPPORT_CMD *pkt_size)
{
	int i;

	for (i = 0; i < sizeof(pkt_size_support); i ++)
	{
		if ((pkt_size->slBandwidthMode == pkt_size_support[i].slBandwidthMode)
			&& (pkt_size->ulWholeSlot == pkt_size_support[i].ulWholeSlot))
		{
			if (pkt_size->slMacIdentify == I_AM_SLAVE)
			{
				pkt_size->ulEvenSlot = pkt_size_support[i].ulEvenSlot;
				pkt_size->ulBpskLength = pkt_size_support[i].ulSlaveBpskLength;
				pkt_size->ulQpskLength = pkt_size_support[i].ulSlaveQpskLength;
				pkt_size->ul16QamLengthFullBandwidth = pkt_size_support[i].ulSlave16QamLengthFullMode;
				pkt_size->ul16QamLengthHalfOrQuarterBandwidth = pkt_size_support[i].ulSlave16QamLength4DivMode;
				return 0;
			}
			else if (pkt_size->slMacIdentify == I_AM_MASTER)
			{
				pkt_size->ulEvenSlot = pkt_size_support[i].ulEvenSlot;
				pkt_size->ulBpskLength = pkt_size_support[i].ulMasterBpskLength;
				pkt_size->ulQpskLength = pkt_size_support[i].ulMasterQpskLength;
				pkt_size->ul16QamLengthFullBandwidth = pkt_size_support[i].ulMaster16QamLengthFullMode;
				pkt_size->ul16QamLengthHalfOrQuarterBandwidth = pkt_size_support[i].ulMaster16QamLength4DivMode;
				return 0;
			}
		}
	}

	return -1;
}
//------------------------------------------------------------------------------
uint32_t RTC676x_Lowermac_Reg_Read(uint8_t ubRegOffset, uint8_t ubRegLength)
{
	RTC676x_MEAS message;
	osStatus ret;
	int32_t value = 0;
#if defined(RTC676x_WAIT_RETURN_BY_EVENT)
	osEvent evt;
#else
	RTC676x_Return_MEAS tReturnMes;
#endif
	uint32_t ulRegData = 0;

	memset(&message, 0x00, sizeof(RTC676x_MEAS));
	value = osSemaphoreWait(RTC676x_Reg_RW_Sema, osWaitForever);

	if (value >= 0)
	{
		message.ulEvent = RTC676x_SPI_REG_GET;
		message.ubEventData[0] = RTC676x_SPI_REG_LOWERMAC;
		message.ubEventData[1] = ubRegOffset;
		message.ubEventData[2] = ubRegLength;
#if defined(RTC676x_WAIT_RETURN_BY_EVENT)
		*((osThreadId*)&message.ubEventData[8]) = osThreadGetId();
#endif

//		while(ubRTC676x_SPI_WaitEvent)
//			TIMER_Delay_us(10);

		ret = osMessagePut(RTC676x_Msg_Q, &message, 0 /*osWaitForever*/);

		if (ret != osOK)
			goto err_sema;

#if defined(RTC676x_WAIT_RETURN_BY_EVENT)
		evt = osSignalWait(RTC676x_WAIT_GET_REG_RETURN_EVENT, osWaitForever);
		if (evt.status == osEventSignal)
			//memcpy(pbRegData, RTC676x_Reg_Return_Data, ubRegLength);
			memcpy((uint8_t *)&ulRegData, RTC676x_Reg_Return_Data, ubRegLength);
		else
			goto err_sema;
#else /* RTC676x_WAIT_RETURN_BY_EVENT */
		//value = osSemaphoreWait(RTC676x_Reg_Read_Done_Sema, osWaitForever);
		//if (value >= 0)
		//	memcpy(pbRegData, &message.ubEventData[3], ubRegLength);
		//else
		//	goto err_sema;
		ret = osMessageGet(RTC676x_Reg_Return_Msg_Q, tReturnMes, osWaitForever);
		if (ret == osEventMessage)
		{
			//memcpy(pbRegData, &tReturnMes.ret_return.ulRegValue, ubRegLength);
			memcpy((uint8_t *)&ulRegData, &tReturnMes.ret_return.ulRegValue, ubRegLength);
		}
		else
			goto err_sema;
#endif

		osSemaphoreRelease(RTC676x_Reg_RW_Sema);
	}
	else
		goto err;

	return ulRegData;

err_sema:
	osSemaphoreRelease(RTC676x_Reg_RW_Sema);
err:
	return ulRegData;
}
//------------------------------------------------------------------------------
int RTC676x_Lowermac_Reg_Write(uint8_t ubRegOffset, uint32_t ulRegData, uint8_t ubRegLength)
{
	RTC676x_MEAS message;
	osStatus ret;
	int32_t value;
#if defined(RTC676x_WAIT_RETURN_BY_EVENT)
	osEvent evt;
#endif

	memset(&message, 0x00, sizeof(RTC676x_MEAS));
	value = osSemaphoreWait(RTC676x_Reg_RW_Sema, osWaitForever);

	if (value >= 0)
	{
		message.ulEvent = RTC676x_SPI_REG_SET;
		message.ubEventData[0] = RTC676x_SPI_REG_LOWERMAC;
		message.ubEventData[1] = ubRegOffset;
		message.ubEventData[2] = ubRegLength;
		//memcpy(&message.ubEventData[3], pbRegData, ubRegLength);
		memcpy(&message.ubEventData[3], (uint8_t *)&ulRegData, ubRegLength);
#if defined(RTC676x_WAIT_RETURN_BY_EVENT)
		*((osThreadId*)&message.ubEventData[8]) = osThreadGetId();
#endif

//		while(ubRTC676x_SPI_WaitEvent)
//			TIMER_Delay_us(10);

		
		ret = osMessagePut(RTC676x_Msg_Q, &message, 0 /*osWaitForever*/);
		if (ret != osOK)
			goto err_sema;


#if defined(RTC676x_WAIT_RETURN_BY_EVENT)
		evt = osSignalWait(RTC676x_WAIT_SET_REG_DONE_EVENT, osWaitForever);
		if (evt.status == osEventSignal)
			//memcpy(pbRegData, RTC676x_Reg_Return_Data, ubRegLength);
			memcpy((uint8_t *)&ulRegData, RTC676x_Reg_Return_Data, ubRegLength);
		else
			goto err_sema;
#endif


		osSemaphoreRelease(RTC676x_Reg_RW_Sema);

	}
	else
		goto err;

	return 0;

err_sema:
	osSemaphoreRelease(RTC676x_Reg_RW_Sema);
err:
	return -1;
}
//------------------------------------------------------------------------------
uint32_t RTC676x_Baseband_Reg_Read(uint8_t ubRegOffset, uint8_t ubRegLength)
{
	RTC676x_MEAS message;
	osStatus ret;
	int32_t value = 0;
#if defined(RTC676x_WAIT_RETURN_BY_EVENT)
	osEvent evt;
#else
	RTC676x_Return_MEAS tReturnMes;
#endif
	uint32_t ulRegData = 0;

	memset(&message, 0x00, sizeof(RTC676x_MEAS));
	value = osSemaphoreWait(RTC676x_Reg_RW_Sema, osWaitForever);

	if (value >= 0)
	{
		message.ulEvent = RTC676x_SPI_REG_GET;
		message.ubEventData[0] = RTC676x_SPI_REG_BASEBAND;
		message.ubEventData[1] = ubRegOffset;
		message.ubEventData[2] = ubRegLength;
#if defined(RTC676x_WAIT_RETURN_BY_EVENT)
		*((osThreadId*)&message.ubEventData[8]) = osThreadGetId();
#endif

//		while(ubRTC676x_SPI_WaitEvent)
//			TIMER_Delay_us(10);

		ret = osMessagePut(RTC676x_Msg_Q, &message, 0 /*osWaitForever*/);

		if (ret != osOK)
			goto err_sema;

#if defined(RTC676x_WAIT_RETURN_BY_EVENT)
		evt = osSignalWait(RTC676x_WAIT_GET_REG_RETURN_EVENT, osWaitForever);
		if (evt.status == osEventSignal)
			memcpy(&ulRegData, RTC676x_Reg_Return_Data, ubRegLength);
		else
			goto err_sema;
#else /* RTC676x_WAIT_RETURN_BY_EVENT */
		//value = osSemaphoreWait(RTC676x_Reg_Read_Done_Sema, osWaitForever);
		//if (value >= 0)
		//	memcpy(pbRegData, &message.ubEventData[3], ubRegLength);
		//else
		//	goto err_sema;
		ret = osMessageGet(RTC676x_Reg_Return_Msg_Q, tReturnMes, osWaitForever);
		if (ret == osEventMessage)
		{
			memcpy(&ulRegData, &tReturnMes.ret_return.ulRegValue, ubRegLength);
		}
		else
			goto err_sema;
#endif

		osSemaphoreRelease(RTC676x_Reg_RW_Sema);
	}
	else
		goto err;

	return ulRegData;

err_sema:
	osSemaphoreRelease(RTC676x_Reg_RW_Sema);
err:	
	return ulRegData;
}
//------------------------------------------------------------------------------
int RTC676x_Baseband_Reg_Write(uint8_t ubRegOffset, uint32_t ulRegData, uint8_t ubRegLength)
{
	RTC676x_MEAS message;
	osStatus ret;
	int32_t value;
#if defined(RTC676x_WAIT_RETURN_BY_EVENT)
	osEvent evt;
#endif

	memset(&message, 0x00, sizeof(RTC676x_MEAS));
	value = osSemaphoreWait(RTC676x_Reg_RW_Sema, osWaitForever);

	if (value >= 0)
	{
		message.ulEvent = RTC676x_SPI_REG_SET;
		message.ubEventData[0] = RTC676x_SPI_REG_BASEBAND;
		message.ubEventData[1] = ubRegOffset;
		message.ubEventData[2] = ubRegLength;
		memcpy(&message.ubEventData[3], (void *)&ulRegData, ubRegLength);
#if defined(RTC676x_WAIT_RETURN_BY_EVENT)
		*((osThreadId*)&message.ubEventData[8]) = osThreadGetId();
#endif

//		while(ubRTC676x_SPI_WaitEvent)
//			TIMER_Delay_us(10);

		ret = osMessagePut(RTC676x_Msg_Q, &message, 0 /*osWaitForever*/);
		if (ret != osOK)
			goto err_sema;

#if defined(RTC676x_WAIT_RETURN_BY_EVENT)
		evt = osSignalWait(RTC676x_WAIT_SET_REG_DONE_EVENT, osWaitForever);
		if (evt.status == osEventSignal)
			//memcpy(pbRegData, RTC676x_Reg_Return_Data, ubRegLength);
			memcpy((uint8_t *)&ulRegData, RTC676x_Reg_Return_Data, ubRegLength);
		else
			goto err_sema;
#endif

		osSemaphoreRelease(RTC676x_Reg_RW_Sema);
	}
	else
		goto err;

	return 0;

err_sema:
	osSemaphoreRelease(RTC676x_Reg_RW_Sema);
err:	
	return -1;
}
//------------------------------------------------------------------------------
uint32_t RTC676x_RF_Reg_Read(uint8_t ubRegOffset, uint8_t ubRegLength)
{
	RTC676x_MEAS message;
	osStatus ret;
	int32_t value = 0;
#if defined(RTC676x_WAIT_RETURN_BY_EVENT)
	osEvent evt;
#else
	RTC676x_Return_MEAS tReturnMes;
#endif
	uint32_t ulRegData = 0;

	memset(&message, 0x00, sizeof(RTC676x_MEAS));
	value = osSemaphoreWait(RTC676x_Reg_RW_Sema, osWaitForever);

	if (value >= 0)
	{
		message.ulEvent = RTC676x_SPI_REG_GET;
		message.ubEventData[0] = RTC676x_SPI_REG_RF;
		message.ubEventData[1] = ubRegOffset;
		message.ubEventData[2] = ubRegLength;
#if defined(RTC676x_WAIT_RETURN_BY_EVENT)
		*((osThreadId*)&message.ubEventData[8]) = osThreadGetId();
#endif

//		while(ubRTC676x_SPI_WaitEvent)
//			TIMER_Delay_us(10);

		ret = osMessagePut(RTC676x_Msg_Q, &message, 0 /*osWaitForever*/);

		if (ret != osOK)
			goto err_sema;

#if defined(RTC676x_WAIT_RETURN_BY_EVENT)
		evt = osSignalWait(RTC676x_WAIT_GET_REG_RETURN_EVENT, osWaitForever);
		if (evt.status == osEventSignal)
			memcpy(&ulRegData, RTC676x_Reg_Return_Data, ubRegLength);
		else
			goto err_sema;
#else /* RTC676x_WAIT_RETURN_BY_EVENT */
		//value = osSemaphoreWait(RTC676x_Reg_Read_Done_Sema, osWaitForever);
		//if (value >= 0)
		//	memcpy(pbRegData, &message.ubEventData[3], ubRegLength);
		//else
		//	goto err_sema;
		ret = osMessageGet(RTC676x_Reg_Return_Msg_Q, tReturnMes, osWaitForever);
		if (ret == osEventMessage)
		{
			memcpy(&ulRegData, &tReturnMes.ret_return.ulRegValue, ubRegLength);
		}
		else
			goto err_sema;
#endif

		osSemaphoreRelease(RTC676x_Reg_RW_Sema);
	}
	else
		goto err;

	return ulRegData;

err_sema:
	osSemaphoreRelease(RTC676x_Reg_RW_Sema);
err:	
	return ulRegData;
}
//------------------------------------------------------------------------------
int RTC676x_RF_Reg_Write(uint8_t ubRegOffset, uint32_t ulRegData, uint8_t ubRegLength)
{
	RTC676x_MEAS message;
	osStatus ret;
	int32_t value;
#if defined(RTC676x_WAIT_RETURN_BY_EVENT)
	osEvent evt;
#endif

	memset(&message, 0x00, sizeof(RTC676x_MEAS));
	value = osSemaphoreWait(RTC676x_Reg_RW_Sema, osWaitForever);

	if (value >= 0)
	{
		message.ulEvent = RTC676x_SPI_REG_SET;
		message.ubEventData[0] = RTC676x_SPI_REG_RF;
		message.ubEventData[1] = ubRegOffset;
		message.ubEventData[2] = ubRegLength;
		memcpy(&message.ubEventData[3], (void *)&ulRegData, ubRegLength);
#if defined(RTC676x_WAIT_RETURN_BY_EVENT)
		*((osThreadId*)&message.ubEventData[8]) = osThreadGetId();
#endif

//		while(ubRTC676x_SPI_WaitEvent)
//			TIMER_Delay_us(10);

		ret = osMessagePut(RTC676x_Msg_Q, &message, 0 /*osWaitForever*/);
		if (ret != osOK)
			goto err_sema;

#if defined(RTC676x_WAIT_RETURN_BY_EVENT)
		evt = osSignalWait(RTC676x_WAIT_SET_REG_DONE_EVENT, osWaitForever);
		if (evt.status == osEventSignal)
			//memcpy(pbRegData, RTC676x_Reg_Return_Data, ubRegLength);
			memcpy((uint8_t *)&ulRegData, RTC676x_Reg_Return_Data, ubRegLength);
		else
			goto err_sema;
#endif

		osSemaphoreRelease(RTC676x_Reg_RW_Sema);
	}
	else
		goto err;

	return 0;

err_sema:
	osSemaphoreRelease(RTC676x_Reg_RW_Sema);
err:	
	return -1;
}
//------------------------------------------------------------------------------
uint32_t RTC676x_PMU_Reg_Read(uint8_t ubRegOffset, uint8_t ubRegLength)
{
	RTC676x_MEAS message;
	osStatus ret;
	int32_t value = 0;
#if defined(RTC676x_WAIT_RETURN_BY_EVENT)
	osEvent evt;
#else
	RTC676x_Return_MEAS tReturnMes;
#endif
	uint32_t ulRegData = 0;

	memset(&message, 0x00, sizeof(RTC676x_MEAS));
	value = osSemaphoreWait(RTC676x_Reg_RW_Sema, osWaitForever);

	if (value >= 0)
	{
		message.ulEvent = RTC676x_SPI_REG_GET;
		message.ubEventData[0] = RTC676x_SPI_REG_PMU;
		message.ubEventData[1] = ubRegOffset;
		message.ubEventData[2] = ubRegLength;
#if defined(RTC676x_WAIT_RETURN_BY_EVENT)
		*((osThreadId*)&message.ubEventData[8]) = osThreadGetId();
#endif

//		while(ubRTC676x_SPI_WaitEvent)
//			TIMER_Delay_us(10);

		ret = osMessagePut(RTC676x_Msg_Q, &message, 0 /*osWaitForever*/);

		if (ret != osOK)
			goto err_sema;

#if defined(RTC676x_WAIT_RETURN_BY_EVENT)
		evt = osSignalWait(RTC676x_WAIT_GET_REG_RETURN_EVENT, osWaitForever);
		if (evt.status == osEventSignal)
			memcpy(&ulRegData, RTC676x_Reg_Return_Data, ubRegLength);
		else
			goto err_sema;
#else /* RTC676x_WAIT_RETURN_BY_EVENT */
		//value = osSemaphoreWait(RTC676x_Reg_Read_Done_Sema, osWaitForever);
		//if (value >= 0)
		//	memcpy(pbRegData, &message.ubEventData[3], ubRegLength);
		//else
		//	goto err_sema;
		ret = osMessageGet(RTC676x_Reg_Return_Msg_Q, tReturnMes, osWaitForever);
		if (ret == osEventMessage)
		{
			memcpy(&ulRegData, &tReturnMes.ret_return.ulRegValue, ubRegLength);
		}
		else
			goto err_sema;
#endif

		osSemaphoreRelease(RTC676x_Reg_RW_Sema);
	}
	else
		goto err;

	return ulRegData;

err_sema:
	osSemaphoreRelease(RTC676x_Reg_RW_Sema);
err:	
	return ulRegData;
}
//------------------------------------------------------------------------------
int RTC676x_PMU_Reg_Write(uint8_t ubRegOffset, uint32_t ulRegData, uint8_t ubRegLength)
{
	RTC676x_MEAS message;
	osStatus ret;
	int32_t value;
#if defined(RTC676x_WAIT_RETURN_BY_EVENT)
	osEvent evt;
#endif

	memset(&message, 0x00, sizeof(RTC676x_MEAS));
	value = osSemaphoreWait(RTC676x_Reg_RW_Sema, osWaitForever);

	if (value >= 0)
	{
		message.ulEvent = RTC676x_SPI_REG_SET;
		message.ubEventData[0] = RTC676x_SPI_REG_PMU;
		message.ubEventData[1] = ubRegOffset;
		message.ubEventData[2] = ubRegLength;
		memcpy(&message.ubEventData[3], (void *)&ulRegData, ubRegLength);
#if defined(RTC676x_WAIT_RETURN_BY_EVENT)
		*((osThreadId*)&message.ubEventData[8]) = osThreadGetId();
#endif

//		while(ubRTC676x_SPI_WaitEvent)
//			TIMER_Delay_us(10);

		ret = osMessagePut(RTC676x_Msg_Q, &message, 0 /*osWaitForever*/);
		if (ret != osOK)
			goto err_sema;

#if defined(RTC676x_WAIT_RETURN_BY_EVENT)
		evt = osSignalWait(RTC676x_WAIT_SET_REG_DONE_EVENT, osWaitForever);
		if (evt.status == osEventSignal)
			//memcpy(pbRegData, RTC676x_Reg_Return_Data, ubRegLength);
			memcpy((uint8_t *)&ulRegData, RTC676x_Reg_Return_Data, ubRegLength);
		else
			goto err_sema;
#endif

		osSemaphoreRelease(RTC676x_Reg_RW_Sema);
	}
	else
		goto err;

	return 0;

err_sema:
	osSemaphoreRelease(RTC676x_Reg_RW_Sema);
err:	
	return -1;
}
//------------------------------------------------------------------------------
void RTC676x_Wait_Beacon_Timeout(void)
{
	//int32_t ret;

	while(osSemaphoreWait(RTC676x_Mac_Wait_Beacom_Timeout_Sema, 0) == osOK){};
	
	RTC676x_Mac_Wait_Beacon_Timeout_Flag = true;

	if (osSemaphoreWait(RTC676x_Mac_Wait_Beacom_Timeout_Sema, /*osWaitForever*/ 150) != osOK)
	{
//		printf("[Wait_Beacon] Timeout\n");
	}
	
	RTC676x_Tx_Stop = 0;

	RTC676x_Mac_Wait_Beacon_Timeout_Flag = false;
}
//------------------------------------------------------------------------------
uint64_t RTC676x_Get_Last_ACK_Timestamp(void)
{
	return ullRTC676x_ACK_Timestamp;
}
//------------------------------------------------------------------------------
void RTC676x_Stop_Transmit_and_wait_clear(void)
{
	osMutexWait(RTC676x_Tx_Global_Mutex, osWaitForever);

	RTC676x_Tx_Stop = 1;
	RTC676x_Tx_Stop_Waiting = 1;

	osMutexRelease(RTC676x_Tx_Global_Mutex);

	while(osSemaphoreWait(RTC676x_Tx_Clear_Wait_Sema, 10) != osOK);
}
//------------------------------------------------------------------------------
void RTC676x_Temp_Stop_Transmit(void)
{
	osMutexWait(RTC676x_Tx_Global_Mutex, osWaitForever);

	RTC676x_Tx_Temp_Stop_Check = 1;
	RTC676x_Tx_Temp_Stop = 1;

	osMutexRelease(RTC676x_Tx_Global_Mutex);

	while(osSemaphoreWait(RTC676x_Tx_Temp_Stop_Done_Sema, 10) != osOK);

	osDelay(2);
}
//------------------------------------------------------------------------------
void RTC676x_Start_Transmit(void)
{
	osMutexWait(RTC676x_Tx_Global_Mutex, osWaitForever);

	RTC676x_Tx_Temp_Stop = 0;

	osMutexRelease(RTC676x_Tx_Global_Mutex);
}
//------------------------------------------------------------------------------
#define SWAP(x, y) \
{\
	uint8_t temp = x;\
	x = y;\
	y = temp;\
}
#define SWAP_INT32(x) \
{\
	x = ((x & 0xFF000000) >> 24) | ((x & 0x00FF0000) >> 8) | ((x & 0x0000FF00) << 8) | ((x & 0x000000FF) << 24);\
}

void RTC676x_SPI_CS_High(void);
void RTC676x_SPI_CS_Low(void);

static inline int RTC676x_Transmit_Request_Macro(int pkt_type, RTC676x_Tx_Cmd *tx_cmd)
{
	RTC676x_MEAS message;
	osStatus ret;
	uint32_t ubPreOffset = 0;
	uint8_t* tx_buf;

	osMutexWait(RTC676x_Tx_Global_Mutex, osWaitForever);

	if (RTC676x_Tx_Stop == 1)
	{
		osMutexRelease(RTC676x_Tx_Global_Mutex);

		osSemaphoreWait(RTC676x_Timeout_Sema, (tx_cmd->ulTimeoutMsec == 0) ? 10 : tx_cmd->ulTimeoutMsec);

		tx_cmd->slIsTimeoutFlag = 1;
		return -1;
	}

	if (RTC676x_Tx_Temp_Stop == 1)
	{
		osMutexRelease(RTC676x_Tx_Global_Mutex);

		while(RTC676x_Tx_Temp_Stop)
			osSemaphoreWait(RTC676x_Timeout_Sema, 5);

		osMutexWait(RTC676x_Tx_Global_Mutex, osWaitForever);
	}

#if defined(RTC676x_DMA_QUEUE_UNCACHED_MALLOC)
	tx_buf = RTC676x_DMA_Transmit_Buffer_Alloc(tx_cmd->ulTimeoutMsec);
#else
	tx_buf = osPoolCAlloc(RTC676x_Pkt_Trans_Poll);
	message.tx_request.ptDmaPool = RTC676x_Pkt_Trans_Poll;
#endif

	if (tx_buf == NULL)
	{
		osMutexRelease(RTC676x_Tx_Global_Mutex);

		tx_cmd->slIsTimeoutFlag = 1;
		return -1;
	}

	ubPreOffset = (4 - (( 1 + RTC676x_Vector_Header_Size + tx_cmd->ulPackedLen) & 0x03)) & 0x03;
	tx_buf[ubPreOffset + 0] = RTC676x_REG_TX_FIFO;
	memcpy(&tx_buf[ubPreOffset + 1], tx_cmd->pbVectorMac, RTC676x_Vector_Mac_Size);
	if (tx_cmd->ulTxHeaderLen)
		memcpy(&tx_buf[ubPreOffset + 1 + RTC676x_Vector_Mac_Size], tx_cmd->pbHeader, tx_cmd->ulTxHeaderLen);
	memcpy(&tx_buf[ubPreOffset + 1 + RTC676x_Vector_Mac_Size + tx_cmd->ulTxHeaderLen], tx_cmd->pbBuffer, tx_cmd->ulTxDataLen);

	message.ulEvent = RTC676x_SPI_TX_PACKET;
	message.tx_request.ulPktType = pkt_type;
	message.tx_request.ulTransLen = 1 + RTC676x_Vector_Header_Size + tx_cmd->ulTxHeaderLen + tx_cmd->ulPackedLen;
	message.tx_request.pbTransBuf = tx_buf;
	message.tx_request.ulNeedAck = tx_cmd->ulNeedAck;
	message.tx_request.ubPreOffset = ubPreOffset ;
	message.tx_request.slBBR = tx_cmd->slBBR;

#if defined(RTC676x_WAIT_ACK_BY_EVENT)
	message.tx_request.txThreadId = osThreadGetId();
#endif


	// printf("%s putting message len:%u tick:%u, tx_buf:%X\n", __func__, message.tx_request.ulTransLen, osKernelSysTick(), tx_buf);
//	ret = osMessagePut(RTC676x_Msg_Q, &message, osWaitForever);
#if defined(RTC676x_TRANSMIT_REQUEST_MULTI_Q)
#if defined(RTC676x_USER_DEFINE_TRANSMIT_QUEUE_PRIORIT)
	ret = osMessagePut(RTC676x_Tx_Request_Q[RTC676x_Transmit_user_priority[message.tx_request.ulPktType]], &message.tx_request, 0);
#else
	ret = osMessagePut(RTC676x_Tx_Request_Q[message.tx_request.ulPktType], &message.tx_request, 0);
#endif
#else /* RTC676x_TRANSMIT_REQUEST_MULTI_Q */
	ret = osMessagePut(RTC676x_Tx_Request_Q, &message.tx_request, 0);
#endif

	osMutexRelease(RTC676x_Tx_Global_Mutex);

	if (ret == osOK)
	{
		if (tx_cmd->ulNeedAck)
		{
#if defined(RTC676x_WAIT_ACK_BY_EVENT)
			printf("%s waiting signal\n", __func__);
			osEvent value = osSignalWait(ulWaitEvent[pkt_type], tx_cmd->ulTimeoutMsec == 0 ? osWaitForever : tx_cmd->ulTimeoutMsec);

			if (value.status != osEventSignal)
			{
				tx_cmd->slIsTimeoutFlag = 1;
				return -1;
			}
#else
			int32_t waitTick = osKernelSysTick();
			//printf("%s waiting semaphore, timeout:%u\n", __func__, tx_cmd->ulTimeoutMsec);
			int32_t value = osSemaphoreWait(RTC676x_Wait_Ack_Sem[pkt_type], tx_cmd->ulTimeoutMsec == 0 ? osWaitForever : tx_cmd->ulTimeoutMsec);

			if (value != osOK)
			{
				tx_cmd->slIsTimeoutFlag = 1;
				return -1;
			}
#endif
		}
		else
		{
			// printf("%s waiting mutex\n", __func__);
			osMutexWait(RTC676x_Tx_Ack_List_Mutex[pkt_type], osWaitForever);

			tx_cmd->ulAck0_31 = (uint32_t)RTC676x_Tx_Ack_List[pkt_type] & 0xFFFFFFFF;
			tx_cmd->ulAckMask0_31 = (uint32_t)RTC676x_Tx_Ack_List_Mask[pkt_type] & 0xFFFFFFFF;

			if ((pkt_type == PKT_TYPE_VIDEO) || (pkt_type == PKT_TYPE_AUDIO))
			{
				tx_cmd->ulAck32_63 = (uint32_t)((RTC676x_Tx_Ack_List[PKT_TYPE_VIDEO] >> 32) & 0xFFFFFFFF);
				tx_cmd->ulAckMask32_63 = (uint32_t)((RTC676x_Tx_Ack_List_Mask[PKT_TYPE_VIDEO] >> 32) & 0xFFFFFFFF);
			}

			osMutexRelease(RTC676x_Tx_Ack_List_Mutex[pkt_type]);
		}
	}
	else
	{
		return -1;
	}

	return 0;
}
//------------------------------------------------------------------------------
int RTC676x_Tx_MGMT_Pkt(RTC676x_Tx_Cmd *tx_cmd)
{
	//return RTC676x_Transmit_Request(PKT_TYPE_MGMT, tx_cmd);
	return RTC676x_Transmit_Request_Macro(PKT_TYPE_MGMT, tx_cmd);
}
//------------------------------------------------------------------------------
int RTC676x_Tx_SMSG_Pkt(RTC676x_Tx_Cmd *tx_cmd)
{
	//return RTC676x_Transmit_Request(PKT_TYPE_MGMT, tx_cmd);
	return RTC676x_Transmit_Request_Macro(PKT_TYPE_SMSG, tx_cmd);
}
//------------------------------------------------------------------------------
int RTC676x_Tx_VIDEO_Pkt(RTC676x_Tx_Cmd *tx_cmd)
{
	//return RTC676x_Transmit_Request(PKT_TYPE_MGMT, tx_cmd);
	return RTC676x_Transmit_Request_Macro(PKT_TYPE_VIDEO, tx_cmd);
}
//------------------------------------------------------------------------------
int RTC676x_Tx_AUDIO_Pkt(RTC676x_Tx_Cmd *tx_cmd)
{
	//return RTC676x_Transmit_Request(PKT_TYPE_MGMT, tx_cmd);
	return RTC676x_Transmit_Request_Macro(PKT_TYPE_AUDIO, tx_cmd);
}
//------------------------------------------------------------------------------
static int RTC676x_Get_Ack_List(uint64_t* plAck, RTC676x_PKT_TYPE type)
{
	int count;

	osMutexWait(RTC676x_Tx_Ack_List_Mutex[type], osWaitForever);

	count = RTC676x_Tx_Ack_List_Count[type];
	*plAck = RTC676x_Tx_Ack_List[type];

	RTC676x_Tx_Ack_List_Count[type] = 0;
	RTC676x_Tx_Ack_List[type] = 0;

	osMutexRelease(RTC676x_Tx_Ack_List_Mutex[type]);

	return count;
}
//------------------------------------------------------------------------------
inline int RTC676x_Get_MGMT_Ack_List(uint64_t* plAck)
{
	return RTC676x_Get_Ack_List(plAck, PKT_TYPE_MGMT);
}
//------------------------------------------------------------------------------
inline int RTC676x_Get_SMSG_Ack_List(uint64_t* plAck)
{
	return RTC676x_Get_Ack_List(plAck, PKT_TYPE_SMSG);
}
//------------------------------------------------------------------------------
inline int RTC676x_Get_VIDEO_Ack_List(uint64_t* plAck)
{
	return RTC676x_Get_Ack_List(plAck, PKT_TYPE_VIDEO);
}
//------------------------------------------------------------------------------
inline int RTC676x_Get_AUDIO_Ack_List(uint64_t* plAck)
{
	return RTC676x_Get_Ack_List(plAck, PKT_TYPE_AUDIO);
}
//------------------------------------------------------------------------------
static inline int RTC676x_Receive_Data_Macro(int pkt_type, osSemaphoreId Recv_Sema, osMessageQId Recv_Q, RTC676x_Rx_Cmd *rx_cmd)
{
	int32_t value = 0;
//	osStatus ret;
//	RTC676x_Recv_Data tRecvData;
	uint32_t ulTimeoutMsec = osWaitForever;
	uint8_t ubRedoRecv = 0;

#if defined(RTC676x_CHECK_CRC)
	rx_cmd->slIsCrcError = 0;
#endif

	if (rx_cmd->ulTimeoutMsec)
		ulTimeoutMsec = rx_cmd->ulTimeoutMsec;

	do
	{
		if (Recv_Sema)
			value = osSemaphoreWait(Recv_Sema, ulTimeoutMsec);

		ubRedoRecv = 0;
		if (value == osOK)
		{
#if 0
			ret = osMessageGet(Recv_Q, &tRecvData, ulTimeoutMsec);

			if (ret == osEventMessage)
			{
				rx_cmd->ulRxDataLen = tRecvData.ulDataLen;
				memcpy(rx_cmd->pbBuffer, tRecvData.pbDataBuf, rx_cmd->ulRxDataLen);

				rx_cmd->slIsTimeoutFlag = RTC676x_Rx_Overrun_State[pkt_type];
				RTC676x_Rx_Overrun_State[pkt_type] = 0;
				/*osPoolFree(RTC676x_Pkt_Recv_Poll, tRecvData.pbDataBuf);*/
#if defined(RTC676x_DMA_QUEUE_UNCACHED_MALLOC)
				RTC676x_DMA_Receive_Buffer_Free(tRecvData.pbDataBuf);
#else
				osPoolFree(tRecvData.ptDmaPool, tRecvData.pbDataBuf);
#endif
			}
			else
			{
				rx_cmd->slIsTimeoutFlag = 1;
			}
#else
			if (RTC676x_Recv_Queue_Get(pkt_type, rx_cmd->pbBuffer, ulTimeoutMsec, &rx_cmd->ulRxDataLen) == -1)
				rx_cmd->slIsTimeoutFlag = 1;
			rx_cmd->slIsOverrun = RTC676x_Rx_Overrun_State[pkt_type];
			RTC676x_Rx_Overrun_State[pkt_type] = 0;
#endif

			if (Recv_Sema)
				osSemaphoreRelease(Recv_Sema);
		}
		else
		{
			rx_cmd->slIsTimeoutFlag = 1;
		}

		if (rx_cmd->slIsTimeoutFlag)
			return -1;

#if defined(RTC676x_CHECK_CRC)
		if ((pgRtc676xData->checkCrcMode > CRC_DONT_CHECK) && (pgRtc676xData->checkCrcMode != CRC_CHECK_ONLY))
		{
			uint16_t crc16_payload, crc16_pkt;

			crc16_pkt = (rx_cmd->pbBuffer[rx_cmd->ulRxDataLen - 2] << 8) | rx_cmd->pbBuffer[rx_cmd->ulRxDataLen - 1];			
			crc16_payload = crc16_ccitt(&rx_cmd->pbBuffer[RTC676x_Vector_Mac_Size], rx_cmd->ulRxDataLen - RTC676x_Vector_Mac_Size - 2);
			if (crc16_pkt != crc16_payload)
			{
				if (pgRtc676xData->checkCrcMode == CRC_CHECK_AND_REPORT)
					rx_cmd->slIsCrcError = 1;
				else if (pgRtc676xData->checkCrcMode == CRC_CHECK_AND_DROP)
				{
					//drop this packet
					ubRedoRecv = 1;
				}
			//	printf("[RF_t:%d] pkt crc err :%04X/%04X\n", pkt_type, crc16_payload, crc16_pkt);
			}
		}
		else if (pkt_type == PKT_TYPE_MGMT)
		{
			
			uint16_t crc16_payload, crc16_pkt;

			crc16_pkt = (rx_cmd->pbBuffer[rx_cmd->ulRxDataLen - 2] << 8) | rx_cmd->pbBuffer[rx_cmd->ulRxDataLen - 1];			
			crc16_payload = crc16_ccitt(&rx_cmd->pbBuffer[RTC676x_Vector_Mac_Size], rx_cmd->ulRxDataLen - RTC676x_Vector_Mac_Size - 2);
			if (crc16_pkt != crc16_payload)
				ubRedoRecv = 1;
		}
#endif
	}
	while(ubRedoRecv > 0);

	return 0;
}
//------------------------------------------------------------------------------
int RTC676x_Rx_MGMT_Pkt(RTC676x_Rx_Cmd *rx_cmd)
{
	return RTC676x_Receive_Data_Macro(PKT_TYPE_MGMT, RTC676x_Receive_MGMT_Sema, RTC676x_Recv_MGMT_Q, rx_cmd);
}
//------------------------------------------------------------------------------
int RTC676x_Rx_SMSG_Pkt(RTC676x_Rx_Cmd *rx_cmd)
{
	return RTC676x_Receive_Data_Macro(PKT_TYPE_SMSG, RTC676x_Receive_SMSG_Sema, RTC676x_Recv_SMSG_Q, rx_cmd);
}
//------------------------------------------------------------------------------
int RTC676x_Rx_VIDEO_Pkt(RTC676x_Rx_Cmd *rx_cmd)
{
	return RTC676x_Receive_Data_Macro(PKT_TYPE_VIDEO, RTC676x_Receive_VIDEO_Sema, RTC676x_Recv_VIDEO_Q, rx_cmd);
}
//------------------------------------------------------------------------------
int RTC676x_Rx_AUDIO_Pkt(RTC676x_Rx_Cmd *rx_cmd)
{
	return RTC676x_Receive_Data_Macro(PKT_TYPE_AUDIO, RTC676x_Receive_AUDIO_Sema, RTC676x_Recv_AUDIO_Q, rx_cmd);
}
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//test
#if defined(RTC676x_DEBUG_MODE)
//------------------------------------------------------------------------------
void RTC676x_Get_CS(void)
{
	RTC676x_MEAS message;

	message.ulEvent = RTC676x_SPI_DEBUG;
	message.pDBGEvent.ubDBGEvent = RTC676x_SPI_DEBUG_CS_GET;

	osMessagePut(RTC676x_Msg_Q, &message, osWaitForever);
}
//------------------------------------------------------------------------------
void RTC676x_Set_CS(uint8_t value)
{
	RTC676x_MEAS message;

	message.ulEvent = RTC676x_SPI_DEBUG;
	message.pDBGEvent.ubDBGEvent = RTC676x_SPI_DEBUG_CS_SET;
	message.pDBGEvent.ubDBGData = value;

	osMessagePut(RTC676x_Msg_Q, &message, osWaitForever);
}
//------------------------------------------------------------------------------
void RTC676x_Get_Reset(void)
{
	RTC676x_MEAS message;

	message.ulEvent = RTC676x_SPI_DEBUG;
	message.pDBGEvent.ubDBGEvent = RTC676x_SPI_DEBUG_RESET_GET;

	osMessagePut(RTC676x_Msg_Q, &message, osWaitForever);
}
//------------------------------------------------------------------------------
void RTC676x_Set_Reset(uint8_t value)
{
	RTC676x_MEAS message;

	message.ulEvent = RTC676x_SPI_DEBUG;
	message.pDBGEvent.ubDBGEvent = RTC676x_SPI_DEBUG_RESET_SET;
	message.pDBGEvent.ubDBGData = value;

	osMessagePut(RTC676x_Msg_Q, &message, osWaitForever);
}
//------------------------------------------------------------------------------
void RTC676x_Get_Interrupt(void)
{
	RTC676x_MEAS message;

	message.ulEvent = RTC676x_SPI_DEBUG;
	message.pDBGEvent.ubDBGEvent = RTC676x_SPI_DEBUG_INT_GET;

	osMessagePut(RTC676x_Msg_Q, &message, osWaitForever);
}
//------------------------------------------------------------------------------
void RTC676x_Set_Interrupt(uint8_t value)
{
	RTC676x_MEAS message;

	message.ulEvent = RTC676x_SPI_DEBUG;
	message.pDBGEvent.ubDBGEvent = RTC676x_SPI_DEBUG_INT_SET;
	message.pDBGEvent.ubDBGData = value;

	osMessagePut(RTC676x_Msg_Q, &message, osWaitForever);
}
//------------------------------------------------------------------------------
void RTC676x_Dma_Pkt_Test(void)
{
	RTC676x_MEAS message;

	message.ulEvent = RTC676x_SPI_DEBUG;
	message.pDBGEvent.ubDBGEvent = RTC676x_SPI_DEBUG_DMA_TRANS_PKT;

	osMessagePut(RTC676x_Msg_Q, &message, osWaitForever);
}
//------------------------------------------------------------------------------
void RTC676x_Add_NA_Request_To_Q(uint8_t type)
{
	RTC676x_MEAS message;

	if ((type >= PKT_TYPE_MAX))
		return;

	message.ulEvent = RTC676x_SPI_DEBUG;
	message.pDBGEvent.ubDBGEvent = RTC676x_SPI_DEBUG_ADD_REQ_TO_NA_WAIT;
	message.pDBGEvent.ubDBGData = type;

	osMessagePut(RTC676x_Msg_Q, &message, osWaitForever);
}
//------------------------------------------------------------------------------
void RTC676x_Put_NA_Event(uint8_t type, uint8_t isAck)
{
	RTC676x_MEAS message;

	if ((type >= PKT_TYPE_MAX))
		return;

	message.ulEvent = RTC676x_SPI_DEBUG;
	message.pDBGEvent.ubDBGEvent = RTC676x_SPI_DEBUG_PUT_NA_EVENT;
	message.pDBGEvent.ubDBGData = (type << 4) | (isAck ? 1 : 0);

	osMessagePut(RTC676x_Msg_Q, &message, osWaitForever);
}
//------------------------------------------------------------------------------
void RTC676x_DMA_Write_Long_Test(uint8_t data)
{
	RTC676x_MEAS message;

	message.ulEvent = RTC676x_SPI_DEBUG;
	message.pDBGEvent.ubDBGEvent = RTC676x_SPI_DEBUG_DMA_WRITE_TEST;
	message.pDBGEvent.ubDBGData = data;

	osMessagePut(RTC676x_Msg_Q, &message, osWaitForever);
}
//------------------------------------------------------------------------------
void RTC676x_DMA_Read_Long_Event(void)
{
	RTC676x_MEAS message;

	message.ulEvent = RTC676x_SPI_DEBUG;
	message.pDBGEvent.ubDBGEvent = RTC676x_SPI_DEBUG_DMA_READ_TEST;
	message.pDBGEvent.ubDBGData = 0;

	osMessagePut(RTC676x_Msg_Q, &message, osWaitForever);
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
#endif /* RTC676x_DEBUG_MODE */
//test


//static void RTC676x_SPI_DmaRW(uint8_t* pbTxData, uint8_t* pbRxData, uint32_t ulDataLen, uint8_t ubDataWidth, SPI_WaitMode_t tWaitMode, RTC676x_DMA_Request_t* tDmaRequest);
void RTC676x_SPI_DmaRW(uint8_t* pbTxData, uint8_t* pbRxData, uint32_t ulDataLen, uint8_t ubDataWidth, SPI_WaitMode_t tWaitMode, RTC676x_DMA_Request_t* tDmaRequest);

//API for internal using
//------------------------------------------------------------------------------
static void RTC676x_RF_SPI_Setting_Get(void)
{
#if 0
	uint8_t clk, cs, tx,rx;
#if defined(VBM_PU)   /*LCD : RX*/
	clk = GLB->PADIO47;
	cs = GLB->PADIO48;
	tx = GLB->PADIO49;
	rx = GLB->PADIO50;

	printf("==============\n");
	printf("              (SPI)  (RF1_SPI)  (RF2_SPI)\n");
	printf("==============\n");
	printf("IO47(CLK) %u  (1)\n", GLB->PADIO47);
	printf("IO48(CS)  %u  (1)\n", GLB->PADIO48);
	printf("IO49(TX)  %u  (1)\n", GLB->PADIO49);
	printf("IO59(RX)  %u  (1)\n", GLB->PADIO50);
	printf("==============\n");
	printf("IO10(CLK) %u  (2)\n", GLB->PADIO10);
	printf("IO9 (CS)  %u  (2)\n", GLB->PADIO9);
	printf("IO11(TX)  %u  (2)\n", GLB->PADIO11);
	printf("IO8 (RX)  %u  (2)\n", GLB->PADIO8);
	printf("==============\n");
	printf("IO18(CLK) %u  (2)    (1)\n", GLB->PADIO18);
	printf("IO19(CS)  %u  (2)    (1)\n", GLB->PADIO19);
	printf("IO20(TX)  %u  (2)    (1)\n", GLB->PADIO20);
	printf("IO21(RX)  %u  (2)    (1)\n", GLB->PADIO21);
	printf("==============\n");
	printf("IO24(CLK) %u                    (1)\n", GLB->PADIO24);
	printf("IO25(CS)  %u                    (1)\n", GLB->PADIO25);
	printf("IO22(TX)  %u                    (1)\n", GLB->PADIO22);
	printf("IO23(RX)  %u                    (1)\n", GLB->PADIO23);
	printf("==============\n");
	printf("IO33(CLK) %u  (1)\n", GLB->PADIO33);
	printf("IO34(CS)  %u  (1)\n", GLB->PADIO34);
	printf("IO35(TX)  %u  (1)\n", GLB->PADIO35);
	printf("IO36(RX)  %u  (1)\n", GLB->PADIO36);
	printf("==============\n");
	printf("IO37(CLK) %u  (4)\n", GLB->PADIO37);
	printf("IO38(CS)  %u  (4)\n", GLB->PADIO38);
	printf("IO39(TX)  %u  (4)\n", GLB->PADIO39);
	printf("IO40(RX)  %u  (4)\n", GLB->PADIO40);
	printf("==============\n");
	printf("IO39(CLK) %u         (2)\n", GLB->PADIO39);
	printf("IO42(CS)  %u         (2)\n", GLB->PADIO42);
	printf("IO41(TX)  %u         (2)\n", GLB->PADIO41);
	printf("IO40(RX)  %u         (2)\n", GLB->PADIO40);
	printf("==============\n");
	printf("IO44(CLK) %u                    (2)\n", GLB->PADIO44);
	printf("IO43(CS)  %u                    (2)\n", GLB->PADIO43);
	printf("IO45(TX)  %u                    (2)\n", GLB->PADIO45);
	printf("IO46(RX)  %u                    (2)\n", GLB->PADIO46);
	printf("==============\n");
	printf("IO52(CLK) %u  (2)\n", GLB->PADIO52);
	printf("IO53(CS)  %u  (2)\n", GLB->PADIO53);
	printf("IO54(TX)  %u  (2)\n", GLB->PADIO54);
	printf("IO55(RX)  %u  (2)\n", GLB->PADIO55);
	printf("==============\n");
	printf("IO58(CLK) %u  (4)\n", GLB->PADIO58);
	printf("IO59(CS)  %u  (4)\n", GLB->PADIO59);
	printf("IO60(TX)  %u  (4)\n", GLB->PADIO60);
	printf("IO61(RX)  %u  (4)\n", GLB->PADIO61);
	printf("==============\n");
	printf("IO64(CLK) %u  (5)    (2)\n", GLB->PADIO64);
	printf("IO65(CS)  %u  (5)    (2)\n", GLB->PADIO65);
	printf("IO66(TX)  %u  (5)    (2)\n", GLB->PADIO66);
	printf("IO67(RX)  %u  (5)    (2)\n", GLB->PADIO67);
	printf("==============\n");
	printf("IO70(CLK) %u                    (2)\n", GLB->PADIO70);
	printf("IO71(CS)  %u                    (2)\n", GLB->PADIO71);
	printf("IO68(TX)  %u                    (2)\n", GLB->PADIO68);
	printf("IO69(RX)  %u                    (2)\n", GLB->PADIO69);
	printf("==============\n");
#elif defined(VBM_BU) /*Camera : TX*/
	clk = GLB->PADIO33;
	cs = GLB->PADIO34;
	tx = GLB->PADIO35;
	rx = GLB->PADIO36;
#endif

	printf("RTC676x_SPI : pin - clk:%u cs:%u tx:%u rx:%u\n", clk, cs, tx, rx);
#endif
	return;
}
//------------------------------------------------------------------------------
#if 0
static void RTC676x_RF_Interrupt_Pin_High(void)
{
	RTC676x_IRQ_OUT = 1;
}
//------------------------------------------------------------------------------
static void RTC676x_RF_Interrupt_Pin_Low(void)
{
	RTC676x_IRQ_OUT = 0;
}
#endif
//------------------------------------------------------------------------------
#if 0
static uint32_t RTC676x_RF_Interrupt_Pin_Get(void)
{
	uint32_t value;

	value = RTC676x_IRQ_INPUT;

	if (value)
		return 1;
	else
		return 0;
}
#endif
//------------------------------------------------------------------------------
#if 0
static void RTC676x_RF_Reset_Pin_High(void)
{
	RTC676x_RESET_OUT = 1;
}
//------------------------------------------------------------------------------
static void RTC676x_RF_Reset_Pin_Low(void)
{
	RTC676x_RESET_OUT = 0;
}
#endif
//------------------------------------------------------------------------------
#if 0
static uint32_t RTC676x_RF_Reset_Pin_Get(void)
{
	uint32_t value;

	value = RTC676x_RESET_OUT;

	if (value)
		return 1;
	else
		return 0;
}
#endif
//------------------------------------------------------------------------------
void RTC676x_Proc_Info(void)
{
	printf("Interrupt:%llu\n", RTC676x_ISR_Cnt);
	printf("===========================\n");
	printf("  MGMT  T(A/N):%llu(%llu/%llu), R:%llu\n", RTC676x_Transmit_Cnt[PKT_TYPE_MGMT], RTC676x_ACK_Cnt[PKT_TYPE_MGMT], RTC676x_NACK_Cnt[PKT_TYPE_MGMT], RTC676x_Receive_Cnt[PKT_TYPE_MGMT]);
	printf("  SMSG  T(A/N):%llu(%llu/%llu), R:%llu\n", RTC676x_Transmit_Cnt[PKT_TYPE_SMSG], RTC676x_ACK_Cnt[PKT_TYPE_SMSG], RTC676x_NACK_Cnt[PKT_TYPE_SMSG], RTC676x_Receive_Cnt[PKT_TYPE_SMSG]);
	printf("  VIDEO T(A/N):%llu(%llu/%llu), R:%llu\n", RTC676x_Transmit_Cnt[PKT_TYPE_VIDEO], RTC676x_ACK_Cnt[PKT_TYPE_VIDEO], RTC676x_NACK_Cnt[PKT_TYPE_VIDEO], RTC676x_Receive_Cnt[PKT_TYPE_VIDEO]);
	printf("  AUDIO T(A/N):%llu(%llu/%llu), R:%llu\n", RTC676x_Transmit_Cnt[PKT_TYPE_AUDIO], RTC676x_ACK_Cnt[PKT_TYPE_AUDIO], RTC676x_NACK_Cnt[PKT_TYPE_AUDIO], RTC676x_Receive_Cnt[PKT_TYPE_AUDIO]);
	printf(" TSLOT:%llu\n", RTC676x_TSLOT_Cnt);
	printf(" None :%llu\n", RTC676x_None_Cnt);
//	printf(" INTR_EN:%u, INTR_MASK:%u\n", RTC676x_IRQ_INTR_GET(), RTC676x_IRQ_INTR_MASK_GET());
}
//------------------------------------------------------------------------------
static void RTC676x_RF_Reset(void)
{	
	//printf("Reset1\r\n");
	RTC676x_RF_Reset_Pin_Low();
	TIMER_Delay_us(10);
	//osDelay(1);
	RTC676x_RF_Reset_Pin_High();
	osMessageReset(RTC676x_Msg_Q);	//2020.08.21 Denny
	
	//osMessageReset(RTC676x_Msg_Q);	//justin 2019.11.11 Modify
	//printf("Reset2\r\n");
}
//------------------------------------------------------------------------------
static void RTC676x_Put_Ack_Nack_To_List(RTC676x_Transmit_Request_t* tx_request, uint8_t ubIsAck)
{
	if (tx_request->ulPktType < PKT_TYPE_MAX)
	{
		osMutexWait(RTC676x_Tx_Ack_List_Mutex[tx_request->ulPktType], osWaitForever);

		RTC676x_Tx_Ack_List_Mask[tx_request->ulPktType] = RTC676x_Tx_Ack_List_Mask[tx_request->ulPktType] << 1;
		RTC676x_Tx_Ack_List_Mask[tx_request->ulPktType] |= 1;

		RTC676x_Tx_Ack_List[tx_request->ulPktType] = RTC676x_Tx_Ack_List[tx_request->ulPktType] << 1;
		if (ubIsAck)
			RTC676x_Tx_Ack_List[tx_request->ulPktType] |= 1;

		RTC676x_Tx_Ack_List_Count[tx_request->ulPktType]++;

		osMutexRelease(RTC676x_Tx_Ack_List_Mutex[tx_request->ulPktType]);

#if defined(RTC676x_WAIT_ACK_BY_EVENT)
		if (tx_request->ulNeedAck)
			osSignalSet(tx_request->txThreadId, ulWaitEvent[tx_request->ulPktType]);
#else
		if (tx_request->ulNeedAck)
			osSemaphoreRelease(RTC676x_Wait_Ack_Sem[tx_request->ulPktType]);
#endif
	}
}
//------------------------------------------------------------------------------
#if 0
static inline void RTC676x_SPI_CS_High(void)
{
	RTC676x_SPI_CS_OUT = 1;
}
//------------------------------------------------------------------------------
static inline void RTC676x_SPI_CS_Low(void)
{
	RTC676x_SPI_CS_OUT = 0;
}
#endif
//------------------------------------------------------------------------------
static inline void RTC676x_SPI_Enable(void)
{
	SSP->SSP_EN = 1;
}
//------------------------------------------------------------------------------
static inline void RTC676x_SPI_Disable(void)
{
	SSP->SSP_EN = 0;
}
//------------------------------------------------------------------------------
void RTC676x_Reg_Lowermac_SPI_Read(uint8_t ubRegOffset, uint8_t ubRegLength, uint8_t* pbRegData)
{
	//uint8_t ubRegReadData[5] = {0}, ubRegWriteData[5] = {0};
	uint8_t *ubRegReadData = ubRegReadRData, *ubRegWriteData = ubRegReadWData;

	memset(ubRegReadRData, 0x00, 16);
	memset(ubRegReadWData, 0x00, 16);

	ubRegWriteData[0] = ubRegOffset |= RTC676x_SPI_REG_OFFSET_READ_BIT;

//	RTC676x_SPI_Enable();
	RTC676x_SPI_CS_Low();

#if defined(RTC676x_SPI_DMA_RW_ONLY)
	RTC676x_SPI_DMA_Predo();
	RTC676x_SPI_DmaRW(ubRegWriteData, ubRegReadData, ubRegLength + 1, 8 /*bit*/, SPI_WaitReady, NULL);
#else
	RTC676x_SPI_CpuRW(ubRegWriteData, &ubRegReadData[0], ubRegLength + 1, 8 /*bit*/);
#endif

	RTC676x_SPI_CS_High();
//	RTC676x_SPI_Disable();

//	printf("Get Low len(%d/%d) data:%x-%x-%x-%x-%x\n",  ubRegLength,  ubRegLength + 1, ubRegReadData[0], ubRegReadData[1], ubRegReadData[2], ubRegReadData[3], ubRegReadData[4]);

	if (pbRegData)
		memcpy(pbRegData, &ubRegReadData[1], ubRegLength);
}
//------------------------------------------------------------------------------
static void RTC676x_Reg_Lowermac_SPI_Write_Single_Byte(uint8_t ubRegOffset, uint8_t pbRegData)
{
	//uint8_t ubRegReadData[5] = {0}, ubRegWriteData[5] = {0};
	uint8_t *ubRegReadData = ubRegWriteRData, *ubRegWriteData = ubRegWriteWData;

	memset(ubRegWriteRData, 0x00, 16);
	memset(ubRegWriteWData, 0x00, 16);

	ubRegWriteData[0] = ubRegOffset;
	ubRegWriteData[1] = pbRegData;

//	RTC676x_SPI_Enable();
	RTC676x_SPI_CS_Low();

//	printf("Set Low len(%d/%d) data:%x-%x-%x-%x-%x\n",  ubRegLength,  ubRegLength + 1, ubRegWriteData[0], ubRegWriteData[1], ubRegWriteData[2], ubRegWriteData[3], ubRegWriteData[4]);

#if defined(RTC676x_SPI_DMA_RW_ONLY)
	RTC676x_SPI_DMA_Predo();
	RTC676x_SPI_DmaRW(ubRegWriteData, ubRegReadData, 2, 8 /*bit*/, SPI_WaitReady, NULL);
#else
	RTC676x_SPI_CpuRW(ubRegWriteData, ubRegReadData, 2, 8 /*bit*/);
#endif

	RTC676x_SPI_CS_High();
//	RTC676x_SPI_Disable();
}
//------------------------------------------------------------------------------
void RTC676x_Reg_Lowermac_SPI_Write(uint8_t ubRegOffset, uint8_t ubRegLength, uint8_t* pbRegData)
{
	//uint8_t ubRegReadData[5] = {0}, ubRegWriteData[5] = {0};
	uint8_t *ubRegReadData = ubRegWriteRData, *ubRegWriteData = ubRegWriteWData;

	memset(ubRegWriteRData, 0x00, 16);
	memset(ubRegWriteWData, 0x00, 16);

	ubRegWriteData[0] = ubRegOffset;

	if (pbRegData)
		memcpy(&ubRegWriteData[1], pbRegData, ubRegLength);

	if (ubRegOffset == RTC676x_REG_CONFIG2)
	{
		if ((ubRegWriteData[1] & RTC676x_REG_CONFIG2_BB_RESET) && (ubRegWriteData[1] & RTC676x_REG_CONFIG2_MAC_RESET))
		{
//			RTC676x_IRQ_INTR_EN = 1;
			RTC676x_IRQ_INTR(1);
		}
	}

//	RTC676x_SPI_Enable();
	RTC676x_SPI_CS_Low();

#if defined(RTC676x_SPI_DMA_RW_ONLY)
	RTC676x_SPI_DMA_Predo();
	RTC676x_SPI_DmaRW(ubRegWriteData, ubRegReadData, ubRegLength + 1, 8 /*bit*/, SPI_WaitReady, NULL);
#else
	RTC676x_SPI_CpuRW(ubRegWriteData, ubRegReadData, ubRegLength + 1, 8 /*bit*/);
#endif

	RTC676x_SPI_CS_High();
//	RTC676x_SPI_Disable();
}
//------------------------------------------------------------------------------
static void RTC676x_Reg_Baseband_SPI_Read(uint8_t ubRegOffset, uint8_t ubRegLength, uint8_t* pbRegData)
{
	uint8_t ubRegBBOffset = ubRegOffset;

	RTC676x_Reg_Lowermac_SPI_Write_Single_Byte(RTC676x_REG_R_BB_ADDR, ubRegBBOffset);
	RTC676x_Reg_Lowermac_SPI_Read(RTC676x_REG_BB_DATA, ubRegLength, pbRegData);
}
//------------------------------------------------------------------------------
static void RTC676x_Reg_Baseband_SPI_Write(uint8_t ubRegOffset, uint8_t ubRegLength, uint8_t* pbRegData)
{
	uint8_t ubRegBBOffset = ubRegOffset;

	RTC676x_Reg_Lowermac_SPI_Write_Single_Byte(RTC676x_REG_W_BB_ADDR, ubRegBBOffset);
	RTC676x_Reg_Lowermac_SPI_Write(RTC676x_REG_BB_DATA, ubRegLength, pbRegData);
}
//------------------------------------------------------------------------------
static void RTC676x_Reg_RF_SPI_Read(uint8_t ubRegOffset, uint8_t ubRegLength, uint8_t* pbRegData)
{
	uint8_t ubRegRFOffset = ubRegOffset;

	RTC676x_Reg_Lowermac_SPI_Write_Single_Byte(RTC676x_REG_R_RF_ADDR, ubRegRFOffset);

	TIMER_Delay_us(20);

	RTC676x_Reg_Lowermac_SPI_Read(RTC676x_REG_RF_DATA1, ubRegLength, pbRegData);
}
//------------------------------------------------------------------------------
static void RTC676x_Reg_RF_SPI_Write(uint8_t ubRegOffset, uint8_t ubRegLength, uint8_t* pbRegData)
{
	uint8_t ubRegRFOffset = ubRegOffset;

	RTC676x_Reg_Lowermac_SPI_Write_Single_Byte(RTC676x_REG_W_RF_ADDR, ubRegRFOffset);
	RTC676x_Reg_Lowermac_SPI_Write(RTC676x_REG_RF_DATA1, ubRegLength, pbRegData);

	TIMER_Delay_us(20);
}
//------------------------------------------------------------------------------
static void RTC676x_Reg_PMU_SPI_Read(uint8_t ubRegOffset, uint8_t ubRegLength, uint8_t* pbRegData)
{
	uint8_t ubRegPMUOffset = ubRegOffset;

	RTC676x_Reg_Lowermac_SPI_Write_Single_Byte(RTC676x_REG_PMU_ADDR, ubRegPMUOffset);

	TIMER_Delay_us(10);

	RTC676x_Reg_Lowermac_SPI_Read(RTC676x_REG_PMU_DATA, ubRegLength, pbRegData);
}
//------------------------------------------------------------------------------
static void RTC676x_Reg_PMU_SPI_Write(uint8_t ubRegOffset, uint8_t ubRegLength, uint8_t* pbRegData)
{
	uint8_t ubRegPMUOffset = ubRegOffset;

	RTC676x_Reg_Lowermac_SPI_Write_Single_Byte(RTC676x_REG_PMU_ADDR, ubRegPMUOffset);
	RTC676x_Reg_Lowermac_SPI_Write(RTC676x_REG_PMU_DATA, ubRegLength, pbRegData);

	TIMER_Delay_us(10);
}
//------------------------------------------------------------------------------
#if !defined(RTC676x_SPI_DMA_RW_ONLY)
#define	SPI_CPI_LOOP_LENGTH		1
static void RTC676x_SPI_CpuRW(uint8_t* pbTxData, uint8_t* pbRxData, uint8_t ubDataLen, uint8_t ubDataWidth)
{
    int i;
    uint8_t ubByteWidth;

//    if(SSP->SSP_OPM == SPI_MASTER)
//        if((ubDataLen == 0) || (ubDataLen > 16)) return;
//    if((ubDataWidth == 0) || (ubDataWidth > 32)) return;

    SSP->SSP_SDL = ubDataWidth - 1;
    ubByteWidth = (ubDataWidth + 7) >> 3;

    //! clear rx/tx fifo
    SSP->SSP_RXF_CLR = 1;
    SSP->SSP_TXF_CLR = 1;

	RTC676x_SPI_Enable();

	//SSP->SSP_RXF_TH = 1;
	//SSP->SSP_TXF_TH = 1;
//    if(SSP->SSP_OPM == SPI_MASTER)
	{
		uint8_t recv_cnt = 0;

		for(i=0;i<ubDataLen;i += SPI_CPI_LOOP_LENGTH)
		{
			int j, retry = 0;

			for(j = 0; j < ( ((ubDataLen - i) >= SPI_CPI_LOOP_LENGTH) ? SPI_CPI_LOOP_LENGTH : 1); j ++)
			{
				switch(ubByteWidth)
	            {
	                case 1:
	                    SSP->SSP_DATA = pbTxData[i + j];
	                    break;
	                case 2:
	                    SSP->SSP_DATA = ((uint16_t*)pbTxData)[i + j];
	                    break;
	                case 3:
	                case 4:
	                    SSP->SSP_DATA = ((uint32_t*)pbTxData)[i + j];
	                    break;
	                default:
	                    printd(DBG_ErrorLvl, "RTC676x: CPU RW...wrong byte width\n");
	                    break;
	            }
			}

			j = 0;
			retry = 0;

			while(j < ( ((ubDataLen - i) >= SPI_CPI_LOOP_LENGTH) ? SPI_CPI_LOOP_LENGTH : 1))
			{
				//        SSP->SSP_EN = 1;
		        while(SSP->SSP_BUSY);
				//        SSP->SSP_EN = 0;

				while(SSP->SSP_RXF_VE)
				{
					switch(ubByteWidth)
		            {
		                case 1:
		                    pbRxData[i + j] = (uint8_t)SSP->SSP_DATA;
		                    break;
		                case 2:
		                    ((uint16_t*)pbRxData)[i + j] = (uint16_t)SSP->SSP_DATA;
		                    break;
		                case 3:
		                case 4:
		                    ((uint32_t*)pbRxData)[i + j] = SSP->SSP_DATA;
		                    break;
		                default:
		                    printd(DBG_ErrorLvl, "RTC676x: CPU RW...wrong byte width\n");
		                    break;
		            }
					j ++;
					recv_cnt ++;
				}

				retry ++;

				if (retry > 50)
					break;
			}
		}

	//	if (recv_cnt != ubDataLen)
	//		printd(DBG_ErrorLvl, "RTC676x: Rx data length not match: %u/%u\n", recv_cnt, ubDataLen);
	}

	RTC676x_SPI_Disable();
}
#endif /* #if !defined(RTC676x_SPI_DMA_RW_ONLY) */
//------------------------------------------------------------------------------
static void RTC676x_SPI_TxDmaIsr(APBC_INT_t tEvent)
{
    if(tEvent == APBC_FINISH)
    {
        RTC676x_SPI_TxEndFlag = 1;
        if(ubRTC676x_SPI_WaitEvent == 1)
        {
        //    osSignalSet(RTC676x_SPI_ThreadId, RTC676x_SPI_TX_DMA_END);
        	RTC676x_MEAS message;
			//osStatus ret;

			message.ulEvent = RTC676x_SPI_TX_DMA_END;

			osMessagePutToFront(RTC676x_Msg_Q, &message, osWaitForever);
        }

		if (SSP->SSP_RXF_OR_FLAG && !RTC676x_SPI_RxEndFlag)
			RTC676x_SPI_RxEndFlag = 2;

    }
    if(tEvent == APBC_ERROR)
        printd(DBG_ErrorLvl, "RTC676x: Tx DMA Error!\n");
}
//------------------------------------------------------------------------------
static void RTC676x_SPI_RxDmaIsr(APBC_INT_t tEvent)
{
    if(tEvent == APBC_FINISH)
        RTC676x_SPI_RxEndFlag = 1;
    if(tEvent == APBC_ERROR)
        printd(DBG_ErrorLvl, "RTC676x: Rx DMA Error!\n");
}
//------------------------------------------------------------------------------
static void RTC676x_SPI_DMA_Predo(void)
{
	SSP->SSP_RXF_TH = 4;
	SSP->SSP_TXF_TH = 4;
}
//------------------------------------------------------------------------------
static void RTC676x_SPI_DmaRW(uint8_t* pbTxData, uint8_t* pbRxData, uint32_t ulDataLen, uint8_t ubDataWidth, SPI_WaitMode_t tWaitMode, RTC676x_DMA_Request_t* tDmaRequest)
{
    APBC_DMA_WIDTH_t tDmaWidth;
    if(ulDataLen == 0) return;
    if((ubDataWidth == 0) || (ubDataWidth > 32)) return;

    switch(ubDataWidth)
    {
        case 32:
            tDmaWidth = APBC_DMA_32BIT;
            break;
        case 16:
            tDmaWidth = APBC_DMA_16BIT;
            break;
        case 8:
            tDmaWidth = APBC_DMA_8BIT;
            break;
        default:
            printd(DBG_ErrorLvl, "RTC676x: DMA Data Width Error %d!\n", ubDataWidth);
            return;
    }

    SSP->SSP_SDL = ubDataWidth - 1;

    //! clear rx/tx fifo
    SSP->SSP_RXF_CLR = 1;
    SSP->SSP_TXF_CLR = 1;


	RTC676x_SPI_TxEndFlag = 0;
    RTC676x_SPI_RxEndFlag = 0;

	if((tWaitMode == SPI_DontWait) && (pfRTC676x_SPI_DmaEndHook != NULL))
    {
        ubRTC676x_SPI_WaitEvent = 1;
    }

	if (tDmaRequest && (tWaitMode == SPI_DontWait))
	{
	//	SSP->SSP_RXF_TH = 4;
	//	SSP->SSP_TXF_TH = 4;
#if 1
		if (tDmaWidth == APBC_DMA_8BIT)
		{
	    	APBC_ChA_Setup((uint32_t)pbTxData, APBC_SSP1_TX, ulDataLen >> 2, tDmaWidth, APBC_BURST, RTC676x_SPI_TxDmaIsr);
	    	APBC_ChB_Setup(APBC_SSP1_RX, (uint32_t)pbRxData, ulDataLen >> 2, tDmaWidth, APBC_BURST, RTC676x_SPI_RxDmaIsr);
		}
#else
		if (tDmaWidth == APBC_DMA_8BIT)
		{
	    	APBC_ChA_Setup((uint32_t)pbTxData, APBC_SSP1_TX, ulDataLen, tDmaWidth, APBC_NONBURST, RTC676x_SPI_TxDmaIsr);
	    	APBC_ChB_Setup(APBC_SSP1_RX, (uint32_t)pbRxData, ulDataLen, tDmaWidth, APBC_NONBURST, RTC676x_SPI_RxDmaIsr);
		}
#endif
		else if (tDmaWidth == APBC_DMA_32BIT)
		{
			APBC_ChA_Setup((uint32_t)pbTxData, APBC_SSP1_TX, ulDataLen >> 2, tDmaWidth, APBC_NONBURST, RTC676x_SPI_TxDmaIsr);
	    	APBC_ChB_Setup(APBC_SSP1_RX, (uint32_t)pbRxData, ulDataLen >> 2, tDmaWidth, APBC_NONBURST, RTC676x_SPI_RxDmaIsr);
		}
		else if (tDmaWidth == APBC_DMA_16BIT)
		{
			APBC_ChA_Setup((uint32_t)pbTxData, APBC_SSP1_TX, ulDataLen >> 1, tDmaWidth, APBC_NONBURST, RTC676x_SPI_TxDmaIsr);
	    	APBC_ChB_Setup(APBC_SSP1_RX, (uint32_t)pbRxData, ulDataLen >> 1, tDmaWidth, APBC_NONBURST, RTC676x_SPI_RxDmaIsr);
		}
	}
	else
	{
	//	SSP->SSP_RXF_TH = 1;
	//	SSP->SSP_TXF_TH = 1;
		APBC_ChA_Setup((uint32_t)pbTxData, APBC_SSP1_TX, ulDataLen, tDmaWidth, APBC_NONBURST, RTC676x_SPI_TxDmaIsr);
	    APBC_ChB_Setup(APBC_SSP1_RX, (uint32_t)pbRxData, ulDataLen, tDmaWidth, APBC_NONBURST, RTC676x_SPI_RxDmaIsr);
	}

    APBC_CHA_ENABLE;
    APBC_CHB_ENABLE;

    SSP->SSP_TXF_DMA_EN = 1;
    SSP->SSP_RXF_DMA_EN = 1;

    //RTC676x_SPI_TxEndFlag = 0;
    //RTC676x_SPI_RxEndFlag = 0;

    //SSP->SSP_EN = 1;
    RTC676x_SPI_Enable();

	// add DMA request to queue
	if (tDmaRequest && (tWaitMode == SPI_DontWait))
	{
		osStatus ret;

		ret = osMessagePut(RTC676x_DMA_Request_Q, tDmaRequest, 0);
		if (ret == osOK)
		{
		}
	}

    if((tWaitMode == SPI_DontWait) && (pfRTC676x_SPI_DmaEndHook != NULL))
    {
        ubRTC676x_SPI_WaitEvent = 1;
        return;
    }

	if ((tWaitMode == SPI_WaitReady) && tDmaRequest)
	{
		RTC676x_SPI_DmaEnd2();
		return;
	}

    RTC676x_SPI_DmaEnd();
}

void RTC676x_SPI_TxDmaIsr_Transmit_Event(void)
{
	//    osSignalSet(RTC676x_SPI_ThreadId, RTC676x_SPI_TX_DMA_END);
	RTC676x_MEAS message;
	//osStatus ret;

	message.ulEvent = RTC676x_SPI_TX_DMA_END;

	osMessagePutToFront(RTC676x_Msg_Q, &message, osWaitForever);
}
//------------------------------------------------------------------------------
static void RTC676x_SPI_Mac_Write_Fifo(RTC676x_Data_t* pRtc676xData, RTC676x_Transmit_Request_t *tx_request)
{
	//RTC676x_DMA_Request_t tDmaRequest;
	//RTC676x_DMA_Request_t *tDmaRequest = osUncachedMalloc(sizeof(RTC676x_DMA_Request_t));
	RTC676x_DMA_Request_t *tDmaRequest = &pRtc676xData->tDmaRequest;

	tDmaRequest->ulPktType = tx_request->ulPktType;
	tDmaRequest->transDirection = DMA_Trans_D_TX;
	tDmaRequest->ulPktLen = tx_request->ulTransLen;
	tDmaRequest->pbDmaBuf = tx_request->pbTransBuf;
	tDmaRequest->ptDmaPool = tx_request->ptDmaPool;
	tDmaRequest->ulPreDoOffset = (tx_request->ulTransLen & 0x03);
	tDmaRequest->ubPreOffset = tx_request->ubPreOffset;

//	RTC676x_SPI_Enable();
	RTC676x_SPI_CS_Low();

	if (tDmaRequest->ulPreDoOffset)
		RTC676x_SPI_DmaRW(tx_request->pbTransBuf + tDmaRequest->ubPreOffset, pRtc676xData->ubDmaDummy + tDmaRequest->ubPreOffset, tDmaRequest->ulPreDoOffset, 8 /*bit*/, SPI_WaitReady, tDmaRequest);
	RTC676x_SPI_DMA_Predo();
	RTC676x_SPI_DmaRW(tx_request->pbTransBuf + tDmaRequest->ubPreOffset + tDmaRequest->ulPreDoOffset, pRtc676xData->ubDmaDummy + tDmaRequest->ubPreOffset + tDmaRequest->ulPreDoOffset, tx_request->ulTransLen - tDmaRequest->ulPreDoOffset, 8 /*bit*/, SPI_DontWait, tDmaRequest);

	RTC676x_Transmit_Cnt[tDmaRequest->ulPktType] ++;

	//osFree(tDmaRequest);

	pRtc676xData->ubDmaRunning = 1;
}
//------------------------------------------------------------------------------
static int RTC676x_SPI_Mac_Read_Fifo(RTC676x_Data_t* pRtc676xData)
{
	uint32_t ulRemainLen;
	uint8_t ubHeaderType;
	uint8_t ubPreOffset = 0;
	//RTC676x_DMA_Request_t tDmaRequest;
	//uint8_t ubHeader[RTC676x_RECV_PRE_HEADER_LEN] = {RTC676x_REG_RX_FIFO | RTC676x_SPI_REG_OFFSET_READ_BIT, 0, 0, 0, 0}, ubHeaderRead[RTC676x_RECV_PRE_HEADER_LEN];
	uint8_t *ubHeader = ubRecvHeader, *ubHeaderRead = ubRecvHeaderRead;
	//RTC676x_DMA_Request_t *tDmaRequest = osUncachedMalloc(sizeof(RTC676x_DMA_Request_t));
	RTC676x_DMA_Request_t *tDmaRequest = &pRtc676xData->tDmaRequest;

	memset(ubHeader, 0x00, RTC676x_RECV_PRE_HEADER_LEN);
	ubHeader[0] = RTC676x_REG_RX_FIFO | RTC676x_SPI_REG_OFFSET_READ_BIT;

//	RTC676x_SPI_Enable();
	RTC676x_SPI_CS_Low();

#if defined(RTC676x_SPI_MAC_READ_PRE_DO)
	RTC676x_SPI_DmaRW(ubHeader, ubHeaderRead, RTC676x_SPI_MAC_READ_PRE_DO + 1, 8 /*bit*/, SPI_WaitReady, tDmaRequest);
#else
#if defined(RTC676x_SPI_DMA_RW_ONLY)
	RTC676x_SPI_DmaRW(ubHeader, ubHeaderRead, 4, 8 /*bit*/, SPI_WaitReady, tDmaRequest);
#else
	RTC676x_SPI_CpuRW(ubHeader, ubHeaderRead, 4, 8 /*bit*/);
#endif
#endif

#if defined(RTC676x_SPI_MAC_READ_PRE_DO)
	ulRemainLen = ((ubHeaderRead[2] & 0x0F) << 8) + ubHeaderRead[3] + 10 - RTC676x_SPI_MAC_READ_PRE_DO;
#else
	ulRemainLen = ((ubHeaderRead[2] & 0x0F) << 8) + ubHeaderRead[3] + 7;
#endif

	ubPreOffset = ulRemainLen & 0x03;
	if (ubPreOffset)
	{
#if defined(RTC676x_SPI_MAC_READ_PRE_DO)
		RTC676x_SPI_DmaRW(&ubHeader[RTC676x_SPI_MAC_READ_PRE_DO + 1], &ubHeaderRead[RTC676x_SPI_MAC_READ_PRE_DO + 1], ubPreOffset, 8 /*bit*/, SPI_WaitReady, tDmaRequest);
#else
#if defined(RTC676x_SPI_DMA_RW_ONLY)
		RTC676x_SPI_DmaRW(&ubHeader[4], &ubHeaderRead[4], ubPreOffset, 8 /*bit*/, SPI_WaitReady, tDmaRequest);
#else
		RTC676x_SPI_CpuRW(&ubHeader[4], &ubHeaderRead[4], ubPreOffset, 8 /*bit*/);
#endif
#endif
	}

	ubHeaderType = ubHeaderRead[1] & 0x1F;

	switch(ubHeaderType)
	{
		case MAC_PKT_BEACON:
		case MAC_PKT_CMD:
		case MAC_PKT_CMD_ACK:
			tDmaRequest->ulPktType = PKT_TYPE_MGMT;
			break;
		case MAC_PKT_SMSG:
		case MAC_PKT_SMS_ACK:
			tDmaRequest->ulPktType = PKT_TYPE_SMSG;
			break;
		case MAC_PKT_VIDEO:
		case MAC_PKT_VIDEO_ACK:
			tDmaRequest->ulPktType = PKT_TYPE_VIDEO;
			break;
		case MAC_PKT_AUDIO:
		case MAC_PKT_AUDIO_ACK:
			tDmaRequest->ulPktType = PKT_TYPE_AUDIO;
			break;
		default:
			printd(DBG_ErrorLvl, "RTC676x: Vector Header Type Error %d!\n", ubHeaderType);
			tDmaRequest->ulPktType = PKT_TYPE_MGMT;

			
			//osFree(tDmaRequest);

			RTC676x_SPI_CS_High();
			RTC676x_SPI_Disable();

			return -1; //return error, need clear RF TX FIFO
	}

#if defined(RTC676x_SPI_MAC_READ_PRE_DO)
	tDmaRequest->ulPktLen = ulRemainLen + RTC676x_SPI_MAC_READ_PRE_DO;
#else
	tDmaRequest->ulPktLen = ulRemainLen + 3;
#endif

	if (tDmaRequest->ulPktLen > RTC676x_RECV_PACKET_SIZE)
	{
		//osFree(tDmaRequest);

		RTC676x_SPI_CS_High();
		RTC676x_SPI_Disable();
		return -1;
	}

	RTC676x_SPI_DMA_Predo();

	tDmaRequest->transDirection = DMA_Trans_D_RX;
#if defined(RTC676x_DMA_QUEUE_UNCACHED_MALLOC)
	tDmaRequest->pbDmaBuf = RTC676x_DMA_Receive_Buffer_Alloc();
#else
	tDmaRequest->pbDmaBuf = osPoolCAlloc(RTC676x_Pkt_Recv_Poll);
	tDmaRequest->ptDmaPool = RTC676x_Pkt_Recv_Poll;
#endif
	tDmaRequest->ulPreDoOffset = ubPreOffset;


#if defined(RTC676x_SPI_MAC_READ_PRE_DO)
	memcpy(tDmaRequest->pbDmaBuf, &ubHeaderRead[1], RTC676x_SPI_MAC_READ_PRE_DO + ubPreOffset);

	RTC676x_SPI_DmaRW(pRtc676xData->ubDmaDummy, &tDmaRequest->pbDmaBuf[RTC676x_SPI_MAC_READ_PRE_DO + ubPreOffset], ulRemainLen - ubPreOffset, 8 /*bit*/, SPI_DontWait, tDmaRequest);
#else
	memcpy(tDmaRequest->pbDmaBuf, &ubHeaderRead[1], 3 + ubPreOffset);

	RTC676x_SPI_DmaRW(pRtc676xData->ubDmaDummy, &tDmaRequest->pbDmaBuf[3 + ubPreOffset], ulRemainLen - ubPreOffset, 8 /*bit*/, SPI_DontWait, tDmaRequest);
#endif

	RTC676x_Receive_Cnt[tDmaRequest->ulPktType] ++;
	RTC676x_RXBytes[tDmaRequest->ulPktType] += tDmaRequest->ulPktLen;

	//osFree(tDmaRequest);

	pRtc676xData->ubDmaRunning = 1;

	return 0;
}
//------------------------------------------------------------------------------
static void RTC676x_SPI_Clear_FIFO(RTC676x_Data_t* pRtc676xData, bool bClearTransmit, bool bClearRecv)
{
	ubDbgConf[0] = RTC676x_REG_DBGCONF | RTC676x_SPI_REG_OFFSET_READ_BIT;
	memset(&ubDbgConf[1], 0, RTC676x_REG_DBGCONF_LENGTH);
	memset(ubDbgConfRead, 0, RTC676x_REG_DBGCONF_LENGTH + 1);

	RTC676x_SPI_CS_Low();
#if defined(RTC676x_SPI_DMA_RW_ONLY)
	RTC676x_SPI_DmaRW(ubDbgConf, ubDbgConfRead, RTC676x_REG_DBGCONF_LENGTH + 1, 8 /*bit*/, SPI_WaitReady, NULL);
#else
	RTC676x_SPI_CpuRW(ubDbgConf, &ubDbgConfRead[0], RTC676x_REG_DBGCONF_LENGTH + 1, 8 /*bit*/);
#endif
	RTC676x_SPI_CS_High();

	ubDbgConf[0] = RTC676x_REG_DBGCONF;
	ubDbgConf[1] = ubDbgConfRead[1];

	if (bClearRecv == true)
		ubDbgConf[1] |= pRtc676xData->Reg_Rx_Fifo_Clear_Mask;
	if (bClearTransmit== true)
		ubDbgConf[1] |= pRtc676xData->Reg_Tx_Fifo_Clear_Mask;

	RTC676x_SPI_CS_Low();
#if defined(RTC676x_SPI_DMA_RW_ONLY)
	RTC676x_SPI_DmaRW(ubDbgConf, ubDbgConfRead, RTC676x_REG_DBGCONF_LENGTH + 1, 8 /*bit*/, SPI_WaitReady, NULL);
#else
	RTC676x_SPI_CpuRW(ubDbgConf, &ubDbgConfRead[0], RTC676x_REG_DBGCONF_LENGTH + 1, 8 /*bit*/);
#endif
	RTC676x_SPI_CS_High();

	if (bClearRecv == true)
		ubDbgConf[1] &= ~(pRtc676xData->Reg_Rx_Fifo_Clear_Mask);
	if (bClearTransmit == true)
		ubDbgConf[1] &= ~(pRtc676xData->Reg_Tx_Fifo_Clear_Mask);

	RTC676x_SPI_CS_Low();
#if defined(RTC676x_SPI_DMA_RW_ONLY)
	RTC676x_SPI_DmaRW(ubDbgConf, ubDbgConfRead, RTC676x_REG_DBGCONF_LENGTH + 1, 8 /*bit*/, SPI_WaitReady, NULL);
#else
	RTC676x_SPI_CpuRW(ubDbgConf, &ubDbgConfRead[0], RTC676x_REG_DBGCONF_LENGTH + 1, 8 /*bit*/);
#endif
	RTC676x_SPI_CS_High();

	if (bClearRecv == true)
		RTC676x_Clear_Recv_FIFO_Cnt ++;
	if (bClearTransmit == true)
		RTC676x_Clear_Transmit_FIFO_Cnt ++;
}
//------------------------------------------------------------------------------
static void RTC676x_SPI_Interrupt_Status(uint8_t* pbRfStatus, uint8_t* pbTimStatus, uint8_t* pbConfig3)
{
//	uint8_t	ubRfStatus[2] = {RTC676x_REG_STATUS | RTC676x_SPI_REG_OFFSET_READ_BIT, 0}, ubRfStatusRead[2];
//	uint8_t	ubTimStatus[2] =  {RTC676x_REG_TIM_STATUS | RTC676x_SPI_REG_OFFSET_READ_BIT, 0}, ubTimStatusRead[2];
//#if defined(RTC676x_6765)
//	uint8_t ubConfig3[3] = {RTC676x_REG_CONFIG3 | RTC676x_SPI_REG_OFFSET_READ_BIT, 0, 0}, ubConfig3Read[3];
//#else /*6763*/
//	uint8_t ubConfig3[2] = {RTC676x_REG_CONFIG3 | RTC676x_SPI_REG_OFFSET_READ_BIT, 0}, ubConfig3Read[2];
//#endif
	memset(ubRfStatusRead, 0, RTC676x_REG_STATUS_LENGTH + 1);
	memset(ubTimStatusRead, 0, RTC676x_REG_TIM_STATIS_LENGTH + 1);
	memset(ubConfig3Read, 0, RTC676x_REG_CONFIG3_LENGTH + 1);

//	RTC676x_SPI_Enable();
	RTC676x_SPI_CS_Low();

	if (pbRfStatus)
	{
#if defined(RTC676x_SPI_DMA_RW_ONLY)
		RTC676x_SPI_DmaRW(ubRfStatus, ubRfStatusRead, RTC676x_REG_STATUS_LENGTH + 1, 8 /*bit*/, SPI_WaitReady, NULL);
#else
		RTC676x_SPI_CpuRW(ubRfStatus, &ubRfStatusRead[0], RTC676x_REG_STATUS_LENGTH + 1, 8 /*bit*/);
#endif

#if !defined(RTC676x_6765)
		RTC676x_SPI_CS_High();
		RTC676x_SPI_CS_Low();

		memcpy(&ubRfStatusClean[1], &ubRfStatusRead[1], RTC676x_REG_STATUS_LENGTH);

#if defined(RTC676x_SPI_DMA_RW_ONLY)
		RTC676x_SPI_DmaRW(ubRfStatusClean, ubRegReadDummy, RTC676x_REG_STATUS_LENGTH + 1, 8 /*bit*/, SPI_WaitReady, NULL);
#else
		RTC676x_SPI_CpuRW(ubRfStatusClean, ubRegReadDummy, RTC676x_REG_STATUS_LENGTH + 1, 8 /*bit*/);
#endif
		memcpy(pbRfStatus, &ubRfStatusRead[1], RTC676x_REG_STATUS_LENGTH);
#endif
	}

	RTC676x_SPI_CS_High();
	RTC676x_SPI_CS_Low();

    if (pbTimStatus && ((pbRfStatus[0] & (RTC676x_REG_STATUS_TSLOT_END | RTC676x_REG_STATUS_RX_DR | RTC676x_REG_STATUS_TX_DS | RTC676x_REG_STATUS_TX_NS)) == 0))
	{
#if defined(RTC676x_SPI_DMA_RW_ONLY)
		RTC676x_SPI_DmaRW(ubTimStatus, ubTimStatusRead, RTC676x_REG_TIM_STATIS_LENGTH + 1, 8 /*bit*/, SPI_WaitReady, NULL);
#else
		RTC676x_SPI_CpuRW(ubTimStatus, &ubTimStatusRead[0], RTC676x_REG_TIM_STATIS_LENGTH + 1, 8 /*bit*/);
#endif

#if !defined(RTC676x_6765)
		RTC676x_SPI_CS_High();
		RTC676x_SPI_CS_Low();

		memcpy(&ubTimStatusClean[1], &ubTimStatusRead[1], RTC676x_REG_TIM_STATIS_LENGTH);
	//	ubTimStatusRead[1] = 0xFF;
#if defined(RTC676x_SPI_DMA_RW_ONLY)
		RTC676x_SPI_DmaRW(ubTimStatusClean, ubRegReadDummy, RTC676x_REG_TIM_STATIS_LENGTH + 1, 8 /*bit*/, SPI_WaitReady, NULL);
#else
		RTC676x_SPI_CpuRW(ubTimStatusClean, ubRegReadDummy, RTC676x_REG_TIM_STATIS_LENGTH + 1, 8 /*bit*/);
#endif
		memcpy(pbTimStatus, &ubTimStatusRead[1], RTC676x_REG_TIM_STATIS_LENGTH);
#endif
	}

	RTC676x_SPI_CS_High();
	RTC676x_SPI_CS_Low();

	if (pbConfig3)
	{
#if defined(RTC676x_SPI_DMA_RW_ONLY)
		RTC676x_SPI_DmaRW(ubConfig3, ubConfig3Read, RTC676x_REG_CONFIG3_LENGTH + 1, 8 /*bit*/, SPI_WaitReady, NULL);
#else
		RTC676x_SPI_CpuRW(ubConfig3, &ubConfig3Read[0], RTC676x_REG_CONFIG3_LENGTH + 1, 8 /*bit*/);
#endif
	}

#if defined(RTC676x_6765)
	RTC676x_SPI_CS_High();
	RTC676x_SPI_CS_Low();

	if (pbRfStatus)
	{
	//	memcpy(&ubRfStatusClean[1], &ubRfStatusRead[1], RTC676x_REG_STATUS_LENGTH);
		ubRfStatusClean[1] = 0xFF;
#if defined(RTC676x_SPI_DMA_RW_ONLY)
		RTC676x_SPI_DmaRW(ubRfStatusClean, ubRegReadDummy, RTC676x_REG_STATUS_LENGTH + 1, 8 /*bit*/, SPI_WaitReady, NULL);
#else
		RTC676x_SPI_CpuRW(ubRfStatusClean, ubRegReadDummy, RTC676x_REG_STATUS_LENGTH + 1, 8 /*bit*/);
#endif
		memcpy(pbRfStatus, &ubRfStatusRead[1], RTC676x_REG_STATUS_LENGTH);
	}

	RTC676x_SPI_CS_High();
	RTC676x_SPI_CS_Low();

	if (pbTimStatus)
	{
		memcpy(&ubTimStatusClean[1], &ubTimStatusRead[1], RTC676x_REG_TIM_STATIS_LENGTH);
#if defined(RTC676x_SPI_DMA_RW_ONLY)
		RTC676x_SPI_DmaRW(ubTimStatusClean, ubRegReadDummy, RTC676x_REG_TIM_STATIS_LENGTH + 1, 8 /*bit*/, SPI_WaitReady, NULL);
#else
		RTC676x_SPI_CpuRW(ubTimStatusClean, ubRegReadDummy, RTC676x_REG_TIM_STATIS_LENGTH + 1, 8 /*bit*/);
#endif
		memcpy(pbTimStatus, &ubTimStatusRead[1], RTC676x_REG_TIM_STATIS_LENGTH);
	}
#endif

	RTC676x_SPI_CS_High();
//	RTC676x_SPI_Disable();

	if (pbConfig3)
	{
		memcpy(pbConfig3, &ubConfig3Read[1], RTC676x_REG_CONFIG3_LENGTH);
	}

#if 0
	if (pbRfStatus && ((ubRfStatusRead[1] & RTC676x_REG_STATUS_RX_DR) != RTC676x_REG_STATUS_RX_DR) && (RTC676x_IRQ_INPUT == 0))
	{
		uint8_t rfID[5], rfID_read[5];
		rfID[0] = 0x0C | RTC676x_SPI_REG_OFFSET_READ_BIT;
		RTC676x_SPI_CS_Low();
#if defined(RTC676x_SPI_DMA_RW_ONLY)
		RTC676x_SPI_DmaRW(rfID, rfID_read, 5, 8 /*bit*/, SPI_WaitReady, NULL);
#else
		RTC676x_SPI_CpuRW(rfID, rfID_read, 5, 8 /*bit*/);
#endif
		RTC676x_SPI_CS_High();
		rfID[0] = 0x0D | RTC676x_SPI_REG_OFFSET_READ_BIT;
		RTC676x_SPI_CS_Low();
#if defined(RTC676x_SPI_DMA_RW_ONLY)
		RTC676x_SPI_DmaRW(rfID, rfID_read, 5, 8 /*bit*/, SPI_WaitReady, NULL);
#else
		RTC676x_SPI_CpuRW(rfID, rfID_read, 5, 8 /*bit*/);
#endif
		RTC676x_SPI_CS_High();

		RTC676x_SPI_CS_Low();
#if defined(RTC676x_SPI_DMA_RW_ONLY)
		RTC676x_SPI_DmaRW(ubRfStatus, ubRfStatusRead, RTC676x_REG_STATUS_LENGTH + 1, 8 /*bit*/, SPI_WaitReady, NULL);
#else
		RTC676x_SPI_CpuRW(ubRfStatus, &ubRfStatusRead[0], RTC676x_REG_STATUS_LENGTH + 1, 8 /*bit*/);
#endif
		RTC676x_SPI_CS_High();

		RTC676x_RESET_OUT = 0;
	}
#endif

//	RTC676x_SPI_Disable();
}
//------------------------------------------------------------------------------
static void RTC676x_SPI_Config3(uint8_t* pbConfig3)
{
	memset(ubConfig3Read, 0, RTC676x_REG_CONFIG3_LENGTH + 1);

//	RTC676x_SPI_Enable();
	RTC676x_SPI_CS_Low();

	if (pbConfig3)
	{
#if defined(RTC676x_SPI_DMA_RW_ONLY)
		RTC676x_SPI_DmaRW(ubConfig3, ubConfig3Read, RTC676x_REG_CONFIG3_LENGTH + 1, 8 /*bit*/, SPI_WaitReady, NULL);
#else
		RTC676x_SPI_CpuRW(ubConfig3, &ubConfig3Read[0], RTC676x_REG_CONFIG3_LENGTH + 1, 8 /*bit*/);
#endif
	}

	RTC676x_SPI_CS_High();
//	RTC676x_SPI_Disable();

	if (pbConfig3)
	{
		memcpy(pbConfig3, &ubConfig3Read[1], RTC676x_REG_CONFIG3_LENGTH);
	}
}
//------------------------------------------------------------------------------
void RTC676x_SPI_Thread(void const *argument)
{
	RTC676x_Data_t* pRtc676xData = (RTC676x_Data_t*)argument;
    //osEvent evt;
	osStatus ret;
	uint32_t ulRtc676xRegRWCnt = 0;

	uint32_t ulTSLOTCnt = 0;

	pRtc676xData->ubRFTxMode = 0;
//	pRtc676xData->ulCheckTimerInterval = RTC676x_CHECK_TIMER;

    while(1)
    {
		RTC676x_MEAS message;
#if defined(RTC676x_ISR_RESEND_EVENT_WHEN_DMA_DOING)
		RTC676x_MEAS messageBack;
		bool bRFIntInDma = false;
#endif

		RTC676x_Thread_State = 0;

		ret = osMessageGet(RTC676x_Msg_Q, (void*)&message, osWaitForever);

		RTC676x_Thread_Run_Cnt ++;

		if (ret == osEventMessage)
		{
			RTC676x_Thread_State = message.ulEvent;
#if defined(RTC676x_ISR_RESEND_EVENT_WHEN_DMA_DOING)
check_event:
#endif
			switch(message.ulEvent)
			{
				case RTC676x_SPI_TX_DMA_END:
					RTC676x_SPI_DmaEnd();
					if (pRtc676xData->ubDmaRunning)
					{
						pRtc676xData->ubDmaRunning = 0;

//						if (RTC676x_IRQ_INPUT == 0)
						if (RTC676x_IRQ_INPUT_GET() == 0)
						{
							RTC676x_MEAS message;

							message.ulEvent = RTC676x_SPI_RF_INTERRUPT;

							osMessagePutToFront(RTC676x_Msg_Q, &message, 0);
							RTC676x_REISR_EVENT_Cnt ++;
						}
					}
					break;
				case RTC676x_SPI_REG_GET:					
					if ((ubRTC676x_SPI_WaitEvent == 1) || (osMessages(RTC676x_Tx_Wait_Ack_Q) > 0))
					{
						ulRtc676xRegRWCnt ++;

						if (ulRtc676xRegRWCnt >= 3)
						{
							ulRtc676xRegRWCnt = 0;
							TIMER_Delay_us(1);
						}

						osMessagePut(RTC676x_Msg_Q, &message, 0);
						break;
					}
					else
						ulRtc676xRegRWCnt = 0;

//					printf("RTC676x_SPI: get type:%x reg:%x len:%u\n", message.ubEventData[0],
//						message.ubEventData[1], message.ubEventData[2]);

					switch(message.ubEventData[0])
					{
						case RTC676x_SPI_REG_LOWERMAC:
							RTC676x_Reg_Lowermac_SPI_Read(message.ubEventData[1], message.ubEventData[2], &message.ubEventData[3]);
							break;
						case RTC676x_SPI_REG_BASEBAND:
							RTC676x_Reg_Baseband_SPI_Read(message.ubEventData[1], message.ubEventData[2], &message.ubEventData[3]);
							break;
						case RTC676x_SPI_REG_RF:
							RTC676x_Reg_RF_SPI_Read(message.ubEventData[1], message.ubEventData[2], &message.ubEventData[3]);
							break;
						case RTC676x_SPI_REG_PMU:
							RTC676x_Reg_PMU_SPI_Read(message.ubEventData[1], message.ubEventData[2], &message.ubEventData[3]);
							break;
					}
//					printf("RTC676x_SPI reg_data:%x-%x-%x-%x\n", message.ubEventData[3], message.ubEventData[4], message.ubEventData[5], message.ubEventData[6]);
#if defined(RTC676x_WAIT_RETURN_BY_EVENT)
					{
						osThreadId getRegThreadId = *((osThreadId*)(&message.ubEventData[8]));

						memset(RTC676x_Reg_Return_Data, 0, 12);
						memcpy(RTC676x_Reg_Return_Data, &message.ubEventData[3], message.ubEventData[2]);
						osSignalSet(getRegThreadId, RTC676x_WAIT_GET_REG_RETURN_EVENT);
					}
#else
					//osSemaphoreRelease(RTC676x_Reg_Read_Done_Sema);
					{
						RTC676x_Return_MEAS tReturnMsg;

						tReturnMsg.ulEvent = RTC676x_SPI_RETURN_REG;
						tReturnMsg.ilReturnValue = 0;
						memcpy((uint8_t*)&tReturnMsg.reg_return.ulRegValue, &message.ubEventData[3], message.ubEventData[2]);

						osMessagePut(RTC676x_Reg_Return_Msg_Q, *tReturnMsg, 0);
					}
#endif
					break;
				case RTC676x_SPI_REG_SET:
					if ((ubRTC676x_SPI_WaitEvent == 1) || (osMessages(RTC676x_Tx_Wait_Ack_Q) > 0))
					{
						ulRtc676xRegRWCnt ++;

						if (ulRtc676xRegRWCnt >= 3)
						{
							ulRtc676xRegRWCnt = 0;
							TIMER_Delay_us(1);
						}

						osMessagePut(RTC676x_Msg_Q, &message, 0);
						break;
					}
					else
						ulRtc676xRegRWCnt = 0;

//					printf("RTC676x_SPI: set type:%x reg:%x len:%u data:%x-%x-%x-%x\n", message.ubEventData[0],
//						message.ubEventData[1], message.ubEventData[2],
//						message.ubEventData[3], message.ubEventData[4],
//						message.ubEventData[5], message.ubEventData[6]);

					switch(message.ubEventData[0])
					{
						case RTC676x_SPI_REG_LOWERMAC:
							RTC676x_Reg_Lowermac_SPI_Write(message.ubEventData[1], message.ubEventData[2], &message.ubEventData[3]);
#if !defined(RTC676x_6765)
							if (message.ubEventData[1] == RTC676x_REG_CONFIG4)
							{
								if (message.ubEventData[3] & RTC676x_REG_CONFIG4_BF_TX)
								{
									pRtc676xData->ubRFTxMode = 1;
//									pRtc676xData->ulCheckTimerInterval = RTC676x_CHECK_TIMER_SLAVE;
									
									//printf("RF: TX mode\n");
									printf("RF:T\n");
									
									pRtc676xData->Reg_Rx_Fifo_Check_Mask = RTC676x_REG_CONFIG3_SF_EMPTY;
									pRtc676xData->Reg_Rx_Fifo_Clear_Mask = RTC676x_REG_DBG_CONF_SW_SFCLR;
									pRtc676xData->Reg_Tx_Fifo_Check_Mask = RTC676x_REG_CONFIG3_BF_EMPTY;
									pRtc676xData->Reg_Tx_Fifo_Clear_Mask = RTC676x_REG_DBG_CONF_SW_BFCLR;

									//try: SDO is output by 676x at negative edge of SCK
									uint8_t ubConfig4 = message.ubEventData[3] | (0x1 << 6);
									RTC676x_Reg_Lowermac_SPI_Write(message.ubEventData[1], message.ubEventData[2], &ubConfig4);
								}
								else
								{
									pRtc676xData->ubRFTxMode = 0;
//									pRtc676xData->ulCheckTimerInterval = RTC676x_CHECK_TIMER;
									
									//printf("RF: RX mode\n");
									printf("RF:R\n");
									
									pRtc676xData->Reg_Rx_Fifo_Check_Mask = RTC676x_REG_CONFIG3_BF_EMPTY;
									pRtc676xData->Reg_Rx_Fifo_Clear_Mask = RTC676x_REG_DBG_CONF_SW_BFCLR;
									pRtc676xData->Reg_Tx_Fifo_Check_Mask = RTC676x_REG_CONFIG3_SF_EMPTY;
									pRtc676xData->Reg_Tx_Fifo_Clear_Mask = RTC676x_REG_DBG_CONF_SW_SFCLR;

									//try: SDO is output by 676x at negative edge of SCK
									uint8_t ubConfig4 = message.ubEventData[3] | (0x1 << 6);
									RTC676x_Reg_Lowermac_SPI_Write(message.ubEventData[1], message.ubEventData[2], &ubConfig4);
//									SSP->SSP_SCLKPH = 1;
								}

#if defined(RTC676x_CHECK_TIMER)
//								osTimerStop(RTC676x_Check_Timer);
//								osTimerStart(RTC676x_Check_Timer, pRtc676xData->ulCheckTimerInterval);
#endif
							}
#endif
							if (message.ubEventData[1] == RTC676x_REG_OP_MODE)
							{
								pRtc676xData->ubRFOpMode = message.ubEventData[3];
							}

							if (message.ubEventData[1] == RTC676x_REG_TIMER_EN)
							{
								if (message.ubEventData[3] & RTC676x_REG_TIMER_EN_INQR_TO)
								{
									pRtc676xData->ubInqrBeaconMode = 1;
								}
								else if (pRtc676xData->ubInqrBeaconMode == 1)
								{
									pRtc676xData->ubInqrBeaconMode = 0;
								}
							}

							if (message.ubEventData[1] == RTC676x_REG_CONFIG2)
							{
								pRtc676xData->Reg_Config2 = message.ubEventData[3];
								//printf("[RF] set Config2: %02X\n", message.ubEventData[3]);

								if (message.ubEventData[3] & RTC676x_REG_CONFIG2_SW_HOP_EN)
								{
									//enable to pairing
									RTC676x_Tx_Stop = 0;
								}
								
								printf("[RF]SC2:%02X\n", message.ubEventData[3]);
							}
							if (message.ubEventData[1] == RTC676x_REG_CONFIG0)
							{
								pRtc676xData->Reg_Config = message.ubEventData[3];
							}
							if (message.ubEventData[1] == RTC676x_REG_D_SLOT)
							{
								memcpy((void *)&pRtc676xData->Reg_D_SLOT, &message.ubEventData[3], sizeof(pRtc676xData->Reg_D_SLOT));
							}
							if (message.ubEventData[1] == RTC676x_REG_D_EVEN_SLOT)
							{
								memcpy((void *)&pRtc676xData->Reg_D_EVEN_SLOT, &message.ubEventData[3], sizeof(pRtc676xData->Reg_D_EVEN_SLOT));
							}
							break;
						case RTC676x_SPI_REG_BASEBAND:
							RTC676x_Reg_Baseband_SPI_Write(message.ubEventData[1], message.ubEventData[2], &message.ubEventData[3]);
							break;
						case RTC676x_SPI_REG_RF:
							RTC676x_Reg_RF_SPI_Write(message.ubEventData[1], message.ubEventData[2], &message.ubEventData[3]);
							break;
						case RTC676x_SPI_REG_PMU:
							RTC676x_Reg_PMU_SPI_Write(message.ubEventData[1], message.ubEventData[2], &message.ubEventData[3]);
							break;
					}
#if defined(RTC676x_WAIT_RETURN_BY_EVENT)
					{
						osThreadId getRegThreadId = *((osThreadId*)(&message.ubEventData[8]));
						osSignalSet(getRegThreadId, RTC676x_WAIT_SET_REG_DONE_EVENT);
					}
#endif
					break;
				case RTC676x_SPI_TX_PACKET:
#if defined(RTC676x_TRANSMIT_REQUEST_MULTI_Q)
#if defined(RTC676x_USER_DEFINE_TRANSMIT_QUEUE_PRIORIT)
					ret = osMessagePut(RTC676x_Tx_Request_Q[RTC676x_Transmit_user_priority[message.tx_request.ulPktType]], &message.tx_request, 0);
#else
					ret = osMessagePut(RTC676x_Tx_Request_Q[message.tx_request.ulPktType], &message.tx_request, 0);
#endif
#else /* RTC676x_TRANSMIT_REQUEST_MULTI_Q */
					ret = osMessagePut(RTC676x_Tx_Request_Q, &message.tx_request, 0);
#endif
					break;
				case RTC676x_SPI_RX_PACKET:
				#if 0
					switch(message.rx_cmd.ubPktType)
					{
						case PKT_TYPE_MGMT:
							break;
						case PKT_TYPE_SMSG:
							break;
						case PKT_TYPE_VIDEO:
							break;
						case PKT_TYPE_AUDIO:
							break;
					}
				#endif
					break;
				case RTC676x_SPI_RF_CHECK_INT:
#if defined(RTC676x_CHECK_INTERRUPT_STOP)
					{
						bool bClearTransmit = false, bClearRercv = false;
						int i, postNackCnt = 0;
						RTC676x_SPI_Config3((uint8_t *)&pRtc676xData->Reg_Config3);
						RTC676x_INT_ISSUE_CHECK_CLEAR = 0;

						if ((pRtc676xData->Reg_Config3 & pRtc676xData->Reg_Rx_Fifo_Check_Mask) == 0)
						{
							bClearRercv = true;
							postNackCnt ++;
							RTC676x_INT_ISSUE_CHECK_CLEAR |= 0x1;
						}

						if ((pRtc676xData->Reg_Config3 & pRtc676xData->Reg_Tx_Fifo_Check_Mask) == 0)
						{
							bClearTransmit = true;
							postNackCnt ++;
							RTC676x_INT_ISSUE_CHECK_CLEAR |= 0x2;
						}

						for (i = 0; i < postNackCnt; i ++)
						{
							if (osMessages(RTC676x_Tx_Wait_Ack_Q) > 0)
							{
								RTC676x_Transmit_Request_t tx_request;

								ret = osMessageGet(RTC676x_Tx_Wait_Ack_Q, &tx_request, 0);

								if (ret == osEventMessage)
								{
									RTC676x_Put_Ack_Nack_To_List(&tx_request, 0);
									RTC676x_NACK_Cnt[tx_request.ulPktType] ++ ;
									RTC676x_NACK_PAD_Cnt[tx_request.ulPktType] ++;
								}
								RTC676x_TX_NS_PAD_Cnt ++;
							}
						}

						if (RTC676x_Tx_Stop_Waiting == 1)
						{
//							if (pRtc676xData->ubRFTxMode == 1)
							{
								while(osMessages(RTC676x_Tx_Wait_Ack_Q))
								{
									RTC676x_Transmit_Request_t tx_request;

									ret = osMessageGet(RTC676x_Tx_Wait_Ack_Q, &tx_request, 0);
									if (ret == osEventMessage)
									{
										RTC676x_Put_Ack_Nack_To_List(&tx_request, 0);
										RTC676x_NACK_Cnt[tx_request.ulPktType] ++ ;
										RTC676x_NACK_PAD_Cnt[tx_request.ulPktType] ++;
									}
									RTC676x_TX_NS_PAD_Cnt ++;
								}
#if defined(RTC676x_TRANSMIT_REQUEST_MULTI_Q)
								for (i = 0; i < PKT_TYPE_MAX; i ++)
								{
									while(osMessages(RTC676x_Tx_Request_Q[i]) != 0)
									{
										RTC676x_Transmit_Request_t tx_request;

										ret = osMessageGet(RTC676x_Tx_Request_Q[i], &tx_request, 0);
										if (ret == osEventMessage)
										{											
											RTC676x_Put_Ack_Nack_To_List(&tx_request, 0);
											RTC676x_NACK_Cnt[tx_request.ulPktType] ++ ;
											RTC676x_NACK_PAD_Cnt[tx_request.ulPktType] ++;											
											RTC676x_DMA_Transmit_Buffer_Free(tx_request.pbTransBuf);
										}										
										RTC676x_TX_NS_PAD_Cnt ++;										
									}
								}
#else
								while(osMessages(RTC676x_Tx_Request_Q) != 0)
								{
									RTC676x_Transmit_Request_t tx_request;

									ret = osMessageGet(RTC676x_Tx_Request_Q, &tx_request, 0);
									if (ret == osEventMessage)
									{										
										RTC676x_Put_Ack_Nack_To_List(&tx_request, 0);
										RTC676x_NACK_Cnt[tx_request.ulPktType] ++ ;
										RTC676x_NACK_PAD_Cnt[tx_request.ulPktType] ++;											
										RTC676x_DMA_Transmit_Buffer_Free(tx_request.pbTransBuf);
									}									
									RTC676x_TX_NS_PAD_Cnt ++;									
								}
#endif
								osMessageReset(RTC676x_Tx_Wait_Ack_Q);
								{
									RTC676x_Tx_Stop_Waiting = 0;
									osSemaphoreRelease(RTC676x_Tx_Clear_Wait_Sema);
									printf("[RF] release clear blocking in workaround....\n");
									//[Justin] Modify
									//-------------------------------------
									//WDT_RST_Enable(WDT_CLK_EXTCLK, 0);
									//while(1);									
									//=====================================
								}
							}
/*
							else
							{
								RTC676x_Tx_Stop = 0;
								RTC676x_Tx_Stop_Waiting = 0;
							}
*/
						}

						RTC676x_SPI_Clear_FIFO(pRtc676xData, bClearTransmit, bClearRercv);

						if ((RTC676x_ISR_Keep_Going == false) && (RTC676x_IRQ_INPUT_GET() == 0))
						{
							//try to clear Interrupt flag
							RTC676x_SPI_Interrupt_Status(&pRtc676xData->Reg_Rf_Status, NULL, NULL);

							if (RTC676x_IRQ_INTR_MASK_GET())
								RTC676x_IRQ_INTR_MASK_SET(0);
						}
						
						RTC676x_INT_ISSUE_CHECK_Cnt ++;
						if ((bClearTransmit == true) || (bClearRercv == true))
							RTC676x_INT_ISSUE_CHECK_CLEAR_Cnt ++;
						RTC676x_Check_Int = false;

						if (RTC676x_ISR_Keep_Going)
							RTC676x_ISR_Keep_Going = false;
					}
#endif
					break;
				case RTC676x_SPI_RF_INTERRUPT:
					// printf("RTC676x_SPI_RF_INTERRUPT. ubRTC676x_SPI_WaitEvent = %d\n", ubRTC676x_SPI_WaitEvent);
					if (!ubRTC676x_SPI_WaitEvent)
					{
						int countNoneCnt = 1;
#if defined(RTC676x_6765)
						RTC676x_SPI_Interrupt_Status(&pRtc676xData->Reg_Rf_Status, &pRtc676xData->Reg_Tim_Status, (uint8_t *)&pRtc676xData->Reg_Config3);
#else
						if (pRtc676xData->ubRFTxMode)
							RTC676x_SPI_Interrupt_Status(&pRtc676xData->Reg_Rf_Status, /*&pRtc676xData->Reg_Tim_Status, (uint8_t *)&pRtc676xData->Reg_Config3*/ NULL, NULL);
						else
                            RTC676x_SPI_Interrupt_Status(&pRtc676xData->Reg_Rf_Status, &pRtc676xData->Reg_Tim_Status, /*(uint8_t *)&pRtc676xData->Reg_Config3*/ NULL);
#endif

#if defined(RTC676x_Interrupt_Status_Keep_Q_Max)
						pRtc676xData->Reg_Rf_Status_Log[pRtc676xData->Reg_Interrupt_Log_Index] = pRtc676xData->Reg_Rf_Status;
						pRtc676xData->Reg_Interrupt_Log_Index = (pRtc676xData->Reg_Interrupt_Log_Index + 1) % RTC676x_Interrupt_Status_Keep_Q_Max;
#endif

						if (pRtc676xData->Reg_Tim_Status & RTC676x_REG_TIM_STATUS_BEACON_EXP)
						{
							if (RTC676x_Mac_Wait_Beacon_Timeout_Flag)
								osSemaphoreRelease(RTC676x_Mac_Wait_Beacom_Timeout_Sema);
							countNoneCnt = 0;
						}

						if (pRtc676xData->Reg_Rf_Status & RTC676x_REG_STATUS_RX_DR)
						{
							if (RTC676x_SPI_Mac_Read_Fifo(pRtc676xData) == 0)
							{
								RTC676x_RXDR_Cnt ++;
								countNoneCnt = 0;
								RTC676x_Recv_FIFO_Not_Empty_Cnt = 0;
							}
							else
							{
								RTC676x_SPI_Clear_FIFO(pRtc676xData, false, true);
								RTC676x_RXDR_Err_Cnt ++;
							}
						}
						else if (pRtc676xData->Reg_Rf_Status & RTC676x_REG_STATUS_TSLOT_END)
						{
#if defined(RTC676x_TRANSMIT_REQUEST_MULTI_Q)
#if defined(RTC676x_USER_DEFINE_TRANSMIT_QUEUE_PRIORIT)
							int tPktTypePri[PKT_TYPE_MAX] = {0, (int)PRIORITY_HIGH + 1, (int)PRIORITY_MIDDLE + 1, (int)PRIORITY_LOW + 1};
#else
							RTC676x_PKT_TYPE tPktTypePri[PKT_TYPE_MAX] = {PKT_TYPE_MGMT, PKT_TYPE_AUDIO, PKT_TYPE_VIDEO, PKT_TYPE_SMSG};
#endif
							int32_t i;
#endif
							RTC676x_Transmit_Request_t tx_request;

                            if (RTC676x_TX_DATARATE_Cnt_Clear == 1)
                            {
                                RTC676x_TX_DATARATE_Cnt = 0;
                                RTC676x_TX_DATARATE_Cnt_Clear = 0;
                            }

							if (osMessages(RTC676x_Tx_Wait_Ack_Q) > 0)
								ulTSLOTCnt ++;
							else
								ulTSLOTCnt = 0;

#if 0 //don't need this, net mode need to set single slave or full bandwidth
#if !defined(RTC676x_6765)
							if ((pRtc676xData->ubNeedGetAck) && ((pRtc676xData->Reg_Rf_Status & (RTC676x_REG_STATUS_TX_DS | RTC676x_REG_STATUS_TX_NS)) == 0))
							{
								RTC676x_Transmit_Request_t tx_request;

								ret = osMessageGet(RTC676x_Tx_Wait_Ack_Q, &tx_request, 0);

								if (ret == osEventMessage)
								{
									RTC676x_Put_Ack_Nack_To_List(&tx_request, 0);
								}

								RTV676x_TX_NS_SOFT_Cnt ++;
								RTC676x_TX_NS_Cnt ++;
							}
#endif
#endif

							if (osMessages(RTC676x_Tx_Wait_Ack_Q) > 0)
							{
								pRtc676xData->ubNeedGetAck = 1;
							}

							if (RTC676x_Tx_Temp_Stop == 1)
							{
								if ((osMessages(RTC676x_Tx_Wait_Ack_Q) == 0) && (RTC676x_Tx_Temp_Stop_Check == 1))
								{
									RTC676x_Tx_Temp_Stop_Check = 0;
									osSemaphoreRelease(RTC676x_Tx_Temp_Stop_Done_Sema);
								}
							}
							else
							{
#if defined(RTC676x_TRANSMIT_REQUEST_MULTI_Q)
								for (i = 0; i < PKT_TYPE_MAX; i ++)
								{
									ret = osMessageGet(RTC676x_Tx_Request_Q[tPktTypePri[i]], &tx_request, 0);

									if (ret == osEventMessage)
									{
										//transmit by DMA
										RTC676x_SPI_Mac_Write_Fifo(pRtc676xData, &tx_request);

										ret = osMessagePut(RTC676x_Tx_Wait_Ack_Q, &tx_request, 0);

										RTC676x_TX_Done_Cnt ++;
										RTC676x_TX_DATARATE_Cnt += (tx_request.ulTransLen - 1 - RTC676x_Vector_Header_Size);
										RTC676x_TXBytes[tx_request.ulPktType] += (tx_request.ulTransLen - 1 - RTC676x_Vector_Header_Size);
#if defined(RTC676x_USER_DEFINE_TRANSMIT_QUEUE_PRIORIT)
										RTC676x_Priority_Q_Transmit_Cnt[i] ++;
#endif

										break;
									}
								}
#else /* RTC676x_TRANSMIT_REQUEST_MULTI_Q */
								ret = osMessageGet(RTC676x_Tx_Request_Q, &tx_request, 0);

								if (ret == osEventMessage)
								{
									//transmit by DMA
									RTC676x_SPI_Mac_Write_Fifo(pRtc676xData, &tx_request);

									ret = osMessagePut(RTC676x_Tx_Wait_Ack_Q, &tx_request, 0);

									RTC676x_TX_Done_Cnt ++;
									RTC676x_TX_DATARATE_Cnt += (tx_request.ulTransLen - 1 - RTC676x_Vector_Header_Size);
									RTC676x_TXBytes[tx_request.ulPktType] += (tx_request.ulTransLen - 1 - RTC676x_Vector_Header_Size);
								}
#endif
							}

							RTC676x_TSLOT_Cnt ++;
							countNoneCnt = 0;

							if (RTC676x_Tx_Stop_Waiting == 1)
							{
//								if (pRtc676xData->ubRFTxMode == 1)
								{
#if defined(RTC676x_TRANSMIT_REQUEST_MULTI_Q)
#if defined(RTC676x_USER_DEFINE_TRANSMIT_QUEUE_PRIORIT)
									if ((osMessages(RTC676x_Tx_Request_Q[0]) == 0) &&
										(osMessages(RTC676x_Tx_Request_Q[(int)PRIORITY_HIGH + 1]) == 0) &&
										(osMessages(RTC676x_Tx_Request_Q[(int)PRIORITY_MIDDLE + 1]) == 0) &&
										(osMessages(RTC676x_Tx_Request_Q[(int)PRIORITY_LOW + 1]) == 0) &&
										(osMessages(RTC676x_Tx_Wait_Ack_Q) == 0))
#else
									if ((osMessages(RTC676x_Tx_Request_Q[PKT_TYPE_MGMT]) == 0) &&
										(osMessages(RTC676x_Tx_Request_Q[PKT_TYPE_SMSG]) == 0) &&
										(osMessages(RTC676x_Tx_Request_Q[PKT_TYPE_VIDEO]) == 0) &&
										(osMessages(RTC676x_Tx_Request_Q[PKT_TYPE_AUDIO]) == 0) &&
										(osMessages(RTC676x_Tx_Wait_Ack_Q) == 0))
#endif
#else
									if ((osMessages(RTC676x_Tx_Request_Q) == 0) &&
										(osMessages(RTC676x_Tx_Wait_Ack_Q) == 0))
#endif
									{
										RTC676x_Tx_Stop_Waiting = 0;
										osSemaphoreRelease(RTC676x_Tx_Clear_Wait_Sema);
									}
								}
/*
								else
								{
									RTC676x_Tx_Stop = 0;
									RTC676x_Tx_Stop_Waiting = 0;
								}
*/
							}

						}

						if (pRtc676xData->Reg_Rf_Status & RTC676x_REG_STATUS_TX_DS)
						{
							RTC676x_Transmit_Request_t tx_request;

                            if (RTC676x_TX_VALIDIRY_DATARATE_Cnt_Clear == 1)
                                RTC676x_TX_VALIDIRY_DATARATE_Cnt_Clear = 0;

#if 0
							ret = osMessageGet(RTC676x_Tx_Wait_Ack_Q, &tx_request, 0);

							if (ret == osEventMessage)
							{
								RTC676x_Put_Ack_Nack_To_List(&tx_request, 1);
								RTC676x_ACK_Cnt[tx_request.ulPktType] ++ ;
							}

							RTC676x_TX_DS_Cnt ++;
#else							
							if (osMessages(RTC676x_Tx_Wait_Ack_Q) > 0)
							{
								ret = osMessageGet(RTC676x_Tx_Wait_Ack_Q, &tx_request, 0);

								if (ret == osEventMessage)
								{
									RTC676x_Put_Ack_Nack_To_List(&tx_request, 1);
									RTC676x_ACK_Cnt[tx_request.ulPktType] ++ ;

                                    RTC676x_TX_VALIDIRY_DATARATE_Cnt += (tx_request.ulTransLen - 1 - RTC676x_Vector_Header_Size);
									RTC676x_ACKedBytes[tx_request.ulPktType] += (tx_request.ulTransLen - 1 - RTC676x_Vector_Header_Size);

									if (RTC676x_Tx_Temp_Stop == 1)
									{
										if ((osMessages(RTC676x_Tx_Wait_Ack_Q) == 0) && (RTC676x_Tx_Temp_Stop_Check == 1))
										{
											RTC676x_Tx_Temp_Stop_Check = 0;
											osSemaphoreRelease(RTC676x_Tx_Temp_Stop_Done_Sema);
										}
									}
								}

								RTC676x_TX_DS_Cnt ++;
							}
							else
							{
								RTC676x_TX_Wait_Empty_Cnt ++;
							}
#endif
							countNoneCnt = 0;

							pRtc676xData->ubNeedGetAck = 0;

                            ullRTC676x_ACK_Timestamp = RTC676x_Sys_Micro_Second();
						}
						else if (pRtc676xData->Reg_Rf_Status & RTC676x_REG_STATUS_TX_NS)
						{
							RTC676x_Transmit_Request_t tx_request;

#if 0
							ret = osMessageGet(RTC676x_Tx_Wait_Ack_Q, &tx_request, 0);

							if (ret == osEventMessage)
							{
								RTC676x_Put_Ack_Nack_To_List(&tx_request, 0);
								RTC676x_NACK_Cnt[tx_request.ulPktType] ++ ;
							}

							RTC676x_TX_NS_Cnt ++;
#else
							if (osMessages(RTC676x_Tx_Wait_Ack_Q) > 0)
							{
								ret = osMessageGet(RTC676x_Tx_Wait_Ack_Q, &tx_request, 0);

								if (ret == osEventMessage)
								{
									RTC676x_Put_Ack_Nack_To_List(&tx_request, 0);
									RTC676x_NACK_Cnt[tx_request.ulPktType] ++ ;
								}

								RTC676x_TX_NS_Cnt ++;

								if (RTC676x_Tx_Temp_Stop == 1)
								{
									if ((osMessages(RTC676x_Tx_Wait_Ack_Q) == 0) && (RTC676x_Tx_Temp_Stop_Check == 1))
									{
										RTC676x_Tx_Temp_Stop_Check = 0;
										osSemaphoreRelease(RTC676x_Tx_Temp_Stop_Done_Sema);
									}
								}
							}
							else
							{
								RTC676x_TX_Wait_Empty_Cnt ++;
							}
#endif
							countNoneCnt = 0;

 							pRtc676xData->ubNeedGetAck = 0;
						}
						else if (pRtc676xData->Reg_Rf_Status & RTC676x_REG_STATUS_MAX_RT)
						{
							RTC676x_MAX_RT_Cnt ++;
							countNoneCnt = 0;
						}
						if (pRtc676xData->Reg_Rf_Status & (RTC676x_REG_STATUS_TX_DS | RTC676x_REG_STATUS_TX_NS))
						{
							if ((((pRtc676xData->ubRFOpMode == RTC676x_REG_OP_MODE_4_DIV) && (ulTSLOTCnt > 2)) 
									|| ((pRtc676xData->ubRFOpMode == RTC676x_REG_OP_MODE_NORMAL) && (ulTSLOTCnt > 1))) 
								&& (osMessages(RTC676x_Tx_Wait_Ack_Q) > 0))
							{
								RTC676x_Transmit_Request_t tx_request;

								ret = osMessageGet(RTC676x_Tx_Wait_Ack_Q, &tx_request, 0);

								if (ret == osEventMessage)
								{
									RTC676x_Put_Ack_Nack_To_List(&tx_request, 0);
									RTC676x_NACK_Cnt[tx_request.ulPktType] ++ ;
									RTC676x_NACK_PAD_Cnt[tx_request.ulPktType] ++;

									if (RTC676x_Tx_Temp_Stop == 1)
									{
										if ((osMessages(RTC676x_Tx_Wait_Ack_Q) == 0) && (RTC676x_Tx_Temp_Stop_Check == 1))
										{
											RTC676x_Tx_Temp_Stop_Check = 0;
											osSemaphoreRelease(RTC676x_Tx_Temp_Stop_Done_Sema);
										}
									}
								}
								RTC676x_TX_NS_PAD_Cnt ++;
							}
							else if ((ulTSLOTCnt == 2) && (osMessages(RTC676x_Tx_Wait_Ack_Q) > 0))
							{
								RTC676x_PUT_NACK_ERR ++;
							}

							ulTSLOTCnt = 0;
						}
						if ((pRtc676xData->Reg_Rf_Status & RTC676x_REG_STATUS_TSLOT_END) && (pRtc676xData->ubDmaRunning == 0))
						{
							if (((pRtc676xData->ubRFOpMode == RTC676x_REG_OP_MODE_4_DIV) && (ulTSLOTCnt > 2))
								&& (osMessages(RTC676x_Tx_Wait_Ack_Q) > 0)
								&& (pRtc676xData->ubRFTxMode == 1))
							{
								RTC676x_Transmit_Request_t tx_request;

								ret = osMessageGet(RTC676x_Tx_Wait_Ack_Q, &tx_request, 0);

								if (ret == osEventMessage)
								{
									RTC676x_Put_Ack_Nack_To_List(&tx_request, 0);
									RTC676x_NACK_Cnt[tx_request.ulPktType] ++ ;
									RTC676x_NACK_PAD_Cnt[tx_request.ulPktType] ++;

									if (RTC676x_Tx_Temp_Stop == 1)
									{
										if ((osMessages(RTC676x_Tx_Wait_Ack_Q) == 0) && (RTC676x_Tx_Temp_Stop_Check == 1))
										{
											RTC676x_Tx_Temp_Stop_Check = 0;
											osSemaphoreRelease(RTC676x_Tx_Temp_Stop_Done_Sema);
										}
									}
								}
								RTC676x_TX_NS_PAD_Cnt ++;

								ulTSLOTCnt = 0;
							}
						}
						if (countNoneCnt == 1)
						{
							//if ((RTC676x_None_Cnt % 6000) == 0)
							//	printf("none cnt:%llu status:%X time:%X\n", RTC676x_None_Cnt, pRtc676xData->Reg_Rf_Status, pRtc676xData->Reg_Tim_Status);
							RTC676x_None_Cnt ++;
						}

//#if defined(RTC676x_6765)
//						if ((pRtc676xData->ubDmaRunning == 0) && (RTC676x_IRQ_INPUT == 0))
						if ((pRtc676xData->ubDmaRunning == 0) && (RTC676x_IRQ_INPUT_GET() == 0))
						{
							RTC676x_MEAS message;

							message.ulEvent = RTC676x_SPI_RF_INTERRUPT;

							osMessagePutToFront(RTC676x_Msg_Q, &message, 0);
							RTC676x_REISR_EVENT_Cnt ++;
						}
//#endif

						if ((pRtc676xData->Reg_Rf_Status & RTC676x_REG_STATUS_TSLOT_END) 
							&& (pRtc676xData->ubDmaRunning != 0)
							&& (((pRtc676xData->Reg_Rf_Status & (RTC676x_REG_STATUS_TX_DS | RTC676x_REG_STATUS_TX_NS)) && (osMessages(RTC676x_Tx_Wait_Ack_Q) > 1)) 
								|| (!(pRtc676xData->Reg_Rf_Status & (RTC676x_REG_STATUS_TX_DS | RTC676x_REG_STATUS_TX_NS)) && (osMessages(RTC676x_Tx_Wait_Ack_Q) > 2))))
						{
							RTC676x_Transmit_Request_t tx_request;

							ret = osMessageGet(RTC676x_Tx_Wait_Ack_Q, &tx_request, 0);

							if (ret == osEventMessage)
							{
								RTC676x_Put_Ack_Nack_To_List(&tx_request, 0);
								RTC676x_NACK_Cnt[tx_request.ulPktType] ++ ;
								RTC676x_NACK_PAD_Cnt[tx_request.ulPktType] ++;

								if (RTC676x_Tx_Temp_Stop == 1)
								{
									if ((osMessages(RTC676x_Tx_Wait_Ack_Q) == 0) && (RTC676x_Tx_Temp_Stop_Check == 1))
									{
										RTC676x_Tx_Temp_Stop_Check = 0;
										osSemaphoreRelease(RTC676x_Tx_Temp_Stop_Done_Sema);
									}
								}
							}
							RTC676x_TX_NS_PAD_Cnt ++;
							RTC676x_TX_NS_Q3_PAD_Cnt ++;

							ulTSLOTCnt = 0;
						}

						if(pRtc676xData->ubDmaRunning == 0)
						{
							RTC676x_SPI_Interrupt_Status(NULL, NULL, &pRtc676xData->Reg_Config3);
#if defined(RTC676x_CLEAR_RECV_FIFO)
							if ((pRtc676xData->Reg_Config3 & pRtc676xData->Reg_Rx_Fifo_Check_Mask) == 0)
							{
								RTC676x_Recv_FIFO_Not_Empty_Cnt ++;

								if (RTC676x_Recv_FIFO_Not_Empty_Cnt > 6)
								{
									RTC676x_Recv_FIFO_Not_Empty_Cnt = 0;
									RTC676x_SPI_Clear_FIFO(pRtc676xData, false, true);
								}
							}
							else
								RTC676x_Recv_FIFO_Not_Empty_Cnt = 0;
#endif
						}
					}
#if defined(RTC676x_ISR_RESEND_EVENT_WHEN_DMA_DOING)
					else
					{
						memcpy(&messageBack, &message, sizeof(RTC676x_MEAS));
						bRFIntInDma = true;

						RTC676x_INT_BUT_NON_DONE_Cnt ++;

						ret = osMessageGet(RTC676x_Msg_Q, &message, 0);

						if (ret == osEventMessage)
						{
							goto check_event;
						}
					}
#endif
//					RTC676x_IRQ_INTR_MASK = 0;
					RTC676x_IRQ_INTR_MASK_SET(0);
					//RTC676x_IRQ_INTR_EN = 1;

//					if ((RTC676x_IRQ_INTR_FLAG == 0) && (RTC676x_IRQ_INPUT == 0))
					{
					//	RTC676x_RESET_OUT = 0;
					}
					RTC676x_ISR_Keep_Going = true;
					break;
				case RTC676x_SPI_RF_RESET:
					RTC676x_RF_Reset();
					break;
				//test
#if defined(RTC676x_DEBUG_MODE)
				case RTC676x_SPI_DEBUG:
					switch(message.pDBGEvent.ubDBGEvent)
					{
						case RTC676x_SPI_DEBUG_CS_GET:
//							printf("CS:%u\r\n", /*GPIO->GPIO_O6*/ RTC676x_SPI_CS_OUT);
							RTC676x_RF_SPI_Setting_Get();
							break;
						case RTC676x_SPI_DEBUG_CS_SET:
							if (message.pDBGEvent.ubDBGData)
								RTC676x_SPI_CS_High();
							else
								RTC676x_SPI_CS_Low();
							break;
						case RTC676x_SPI_DEBUG_RESET_GET:
//							printf("Reset:%u\r\n", RTC676x_RF_Reset_Pin_Get());
							break;
						case RTC676x_SPI_DEBUG_RESET_SET:
							if (message.pDBGEvent.ubDBGData)
								RTC676x_RF_Reset_Pin_High();
							else
								RTC676x_RF_Reset_Pin_Low();
							break;
						case RTC676x_SPI_DEBUG_INT_GET:
//							printf("INTR:%u\r\n", RTC676x_RF_Interrupt_Pin_Get());
							break;
						case RTC676x_SPI_DEBUG_INT_SET:
#if 0
							if (RTC676x_IRQ_INPUT)
							{
								printf("Int set:%u\n", message.pDBGEvent.ubDBGData);
								if (message.pDBGEvent.ubDBGData)
									RTC676x_RF_Interrupt_Pin_High();
								else
									RTC676x_RF_Interrupt_Pin_Low();
							}
#endif
							break;
						case RTC676x_SPI_DEBUG_DMA_TRANS_PKT:
							RTC676x_SPI_Interrupt_Status(&pRtc676xData->Reg_Rf_Status, &pRtc676xData->Reg_Tim_Status, (uint8_t *)&pRtc676xData->Reg_Config3);
							{
#if defined(RTC676x_TRANSMIT_REQUEST_MULTI_Q)
#if defined(RTC676x_USER_DEFINE_TRANSMIT_QUEUE_PRIORIT)
								int tPktTypePri[PKT_TYPE_MAX] = {0, (int)PRIORITY_HIGH + 1, (int)PRIORITY_MIDDLE + 1, (int)PRIORITY_LOW + 1};
#else
								RTC676x_PKT_TYPE tPktTypePri[PKT_TYPE_MAX] = {PKT_TYPE_MGMT, PKT_TYPE_AUDIO, PKT_TYPE_VIDEO, PKT_TYPE_SMSG};
#endif
								int32_t i;
#endif
								RTC676x_Transmit_Request_t tx_request;

#if defined(RTC676x_TRANSMIT_REQUEST_MULTI_Q)
								for (i = 0; i < PKT_TYPE_MAX; i ++)
								{
									ret = osMessageGet(RTC676x_Tx_Request_Q[tPktTypePri[i]], &tx_request, 0);

									if (ret == osEventMessage)
									{
									//	printf("[RTC6776x_SPI] tx_request type:%u len:%u buf:0x%X\n", tx_request.ulPktType, tx_request.ulTransLen, tx_request.pbTransBuf);
										//transmit by DMA
										RTC676x_SPI_Mac_Write_Fifo(pRtc676xData, &tx_request);

										ret = osMessagePut(RTC676x_Tx_Wait_Ack_Q, &tx_request, 0);

#if defined(RTC676x_USER_DEFINE_TRANSMIT_QUEUE_PRIORIT)
										RTC676x_Priority_Q_Transmit_Cnt[i] ++;
#endif
										break;
									}
								}
#else /* RTC676x_TRANSMIT_REQUEST_MULTI_Q */
								ret = osMessageGet(RTC676x_Tx_Request_Q, &tx_request, 0);

								if (ret == osEventMessage)
								{
									//transmit by DMA
									RTC676x_SPI_Mac_Write_Fifo(pRtc676xData, &tx_request);

									ret = osMessagePut(RTC676x_Tx_Wait_Ack_Q, &tx_request, 0);
								}
#endif
							}
							break;
						case RTC676x_SPI_DEBUG_ADD_REQ_TO_NA_WAIT:
							{
								RTC676x_Transmit_Request_t tx_request;

								tx_request.ulPktType = message.pDBGEvent.ubDBGData;
								tx_request.ulNeedAck = 0;

								printf("[676x_SPI] add type:%u req into Q\n", tx_request.ulPktType);

								ret = osMessagePut(RTC676x_Tx_Wait_Ack_Q, &tx_request, 0);
							}
							break;
						case RTC676x_SPI_DEBUG_PUT_NA_EVENT:
							{
								uint8_t type, isAck;

								type =  message.pDBGEvent.ubDBGData >> 4;
								isAck =  message.pDBGEvent.ubDBGData & 0x0F;

								printf("[676x_SPI] put %s for type:%u\n", isAck ? "ACK" : "NACK", type);

								if (isAck)
								{
									RTC676x_Transmit_Request_t tx_request;

									ret = osMessageGet(RTC676x_Tx_Wait_Ack_Q, &tx_request, 0);

									if (ret == osEventMessage)
									{
										RTC676x_Put_Ack_Nack_To_List(&tx_request, 1);
									}

									RTC676x_TX_DS_Cnt ++;
								}
								else
								{
									RTC676x_Transmit_Request_t tx_request;

									ret = osMessageGet(RTC676x_Tx_Wait_Ack_Q, &tx_request, 0);

									if (ret == osEventMessage)
									{
										RTC676x_Put_Ack_Nack_To_List(&tx_request, 0);
									}

									RTC676x_TX_NS_Cnt ++;
								}
							}
							break;
						case RTC676x_SPI_DEBUG_DMA_WRITE_TEST:
							{
								RTC676x_Transmit_Request_t	*tx_request = osUncachedMalloc(sizeof(RTC676x_Transmit_Request_t));
#if defined(RTC676x_DMA_QUEUE_UNCACHED_MALLOC)
								uint8_t* tx_buf = RTC676x_DMA_Transmit_Buffer_Alloc(0);
#else
								uint8_t* tx_buf = osPoolCAlloc(RTC676x_Pkt_Trans_Poll);
#endif
								uint8_t ubFreqTblLength = 80;
								uint8_t ubPreOffset = (4 - (( 1 + ubFreqTblLength) & 0x03)) & 0x03;
#if !defined(RTC676x_DMA_QUEUE_UNCACHED_MALLOC)
								tx_request->ptDmaPool = RTC676x_Pkt_Trans_Poll;
#endif

								if (tx_buf != NULL)
								{
									tx_buf[ubPreOffset + 0] = 0x1C; /* Freq TBL */
									memset(&tx_buf[ubPreOffset + 1], message.pDBGEvent.ubDBGData, ubFreqTblLength);

									tx_request->ulPktType = PKT_TYPE_VIDEO;
									tx_request->ulTransLen = 1 + ubFreqTblLength;
									tx_request->pbTransBuf = tx_buf;
									tx_request->ulNeedAck = 0;
									tx_request->ubPreOffset = ubPreOffset ;
#if defined(RTC676x_WAIT_ACK_BY_EVENT)
									tx_request->txThreadId = 0;
#endif

									RTC676x_SPI_Mac_Write_Fifo(pRtc676xData, tx_request);
								}

								osFree(tx_request);
							}
							break;
						case RTC676x_SPI_DEBUG_DMA_READ_TEST:
							{
								uint32_t ulRemainLen;
								//RTC676x_DMA_Request_t tDmaRequest;
								//uint8_t ubHeader[5] = {RTC676x_REG_RX_FIFO | RTC676x_SPI_REG_OFFSET_READ_BIT, 0, 0, 0, 0}, ubHeaderRead[5];
								uint8_t *ubHeader = ubRecvHeader, *ubHeaderRead = ubRecvHeaderRead;
								RTC676x_DMA_Request_t *tDmaRequest = osUncachedMalloc(sizeof(RTC676x_DMA_Request_t));

								memset(ubHeader, 0x00, 5);
								ubHeader[0] = 0x1C | RTC676x_SPI_REG_OFFSET_READ_BIT;

								RTC676x_SPI_Enable();
								RTC676x_SPI_CS_Low();

#if defined(RTC676x_SPI_DMA_RW_ONLY)
								RTC676x_SPI_DmaRW(ubHeader, ubHeaderRead, 5, 8 /*bit*/, SPI_WaitReady, NULL);
#else
								RTC676x_SPI_CpuRW(ubHeader, ubHeaderRead, 5, 8 /*bit*/);
#endif

								ulRemainLen = 80 - 4;

								tDmaRequest->ulPktType = PKT_TYPE_VIDEO;

								tDmaRequest->ulPktLen = 80;
								tDmaRequest->transDirection = DMA_Trans_D_RX;
#if defined(RTC676x_DMA_QUEUE_UNCACHED_MALLOC)
								tDmaRequest->pbDmaBuf = RTC676x_DMA_Receive_Buffer_Alloc();
#else
								tDmaRequest->pbDmaBuf = osPoolCAlloc(RTC676x_Pkt_Recv_Poll);
								tDmaRequest->ptDmaPool = RTC676x_Pkt_Recv_Poll;
#endif

								memcpy(tDmaRequest->pbDmaBuf, &ubHeaderRead[1], 4);

								RTC676x_SPI_DmaRW(pRtc676xData->ubDmaDummy, &tDmaRequest->pbDmaBuf[4], ulRemainLen, 8 /*bit*/, SPI_DontWait, tDmaRequest);

								osFree(tDmaRequest);
							}
							break;
					}
					break;
#endif /*RTC676x_DEBUG_MODE*/
				//test
				default:
					break;
			}


#if defined(RTC676x_ISR_RESEND_EVENT_WHEN_DMA_DOING)
			if (bRFIntInDma)
				osMessagePut(RTC676x_Msg_Q, &messageBack, 0);
#endif

		}
		else
		{
			RTC676x_Thread_State = 0xFFFFFFFF;
			printd(DBG_ErrorLvl, "RTC676x: SPI_Thread got error event, not a signal event\n");
		}
    }
}
//------------------------------------------------------------------------------
void RTC676x_ISR(void)
{
    INTC_IrqClear(INTC_SSP_IRQ);
    if(SSP->SSP_RXF_TH_FLAG)
        osSignalSet(RTC676x_SPI_ThreadId, RTC676x_SPI_SLAVE_GOT_DATA);
}
//------------------------------------------------------------------------------
//uint32_t ulRfIsrCntTime = 0;
//uint32_t ulPreRfIsrCntTime = 0;
void RTC676x_RF_ISR(void)
{
	if (RTC676x_IRQ_INPUT_GET())
		return;

	if (RTC676x_IRQ_INTR_FLAG_GET())
	{
		RTC676x_MEAS message;

		RTC676x_IRQ_INTR_MASK_SET(1);
		//RTC676x_IRQ_INTR_EN = 0;
		RTC676x_IRQ_CLR_INTR_SET(1);

		message.ulEvent = RTC676x_SPI_RF_INTERRUPT;

		osMessagePutToFront(RTC676x_Msg_Q, &message, 0);
	}
#if defined(VBM_PU)
#elif defined(VBM_BU)
#endif

	//INTC_IrqClear(INTC_GPIO_IRQ);
	RTC676x_INTC_IrqClear();

	RTC676x_ISR_Cnt ++;
}

#if 0
void RTC676x_RF_ISR_INTERRUPT_EVENT(void)
{
	RTC676x_MEAS message;

	message.ulEvent = RTC676x_SPI_RF_INTERRUPT;

	osMessagePutToFront(RTC676x_Msg_Q, &message, 0);
}
#endif
//------------------------------------------------------------------------------
static void RTC676x_RF_CHECK_INT_EVENT(void)
{
	RTC676x_MEAS message;

	message.ulEvent = RTC676x_SPI_RF_CHECK_INT;

	osMessagePutToFront(RTC676x_Msg_Q, &message, 0);
}
//------------------------------------------------------------------------------
#if 0
static inline void RTC676x_SPI_Try_Put_Recv_Q(osMessageQId RTC676x_Recv_Q, osSemaphoreId RTC676x_Receive_Sema, RTC676x_Recv_Data *ptRecvData, int32_t *plOverrunState, int32_t ilPktType, bool bSkipIfOverwrite)
{
#if 0
	osStatus ret = osMessagePut(RTC676x_Recv_Q, ptRecvData, 0);

	if (ret != osOK)
	{
		//free the oldest used dma buffer
		RTC676x_Recv_Data tRecvData;

		osSemaphoreWait(RTC676x_Receive_Sema, osWaitForever);

		if (osMessageGet(RTC676x_Recv_Q, &tRecvData, 0) == osEventMessage)
		{
			//osPoolFree(RTC676x_Pkt_Recv_Poll, tRecvData.pbDataBuf);
#if defined(RTC676x_DMA_QUEUE_UNCACHED_MALLOC)
			RTC676x_DMA_Receive_Buffer_Free(tRecvData.pbDataBuf);
#else
			osPoolFree(tRecvData.ptDmaPool, tRecvData.pbDataBuf);
#endif

			osMessagePut(RTC676x_Recv_Q, ptRecvData, 0);
		}

		*plOverrunState = 1;

		osSemaphoreRelease(RTC676x_Receive_Sema);
	}
	else
		osSemaphoreRelease(RTC676x_Receive_Sema);
#else
	RTC676x_Recv_Queue_Put(ilPktType, ptRecvData->pbDataBuf, ptRecvData->ulDataLen, plOverrunState, bSkipIfOverwrite);
#if defined(RTC676x_DMA_QUEUE_UNCACHED_MALLOC)
	RTC676x_DMA_Receive_Buffer_Free(ptRecvData->pbDataBuf);
#else
	osPoolFree(ptRecvData->ptDmaPool, ptRecvData->pbDataBuf);
#endif
	osSemaphoreRelease(RTC676x_Receive_Sema);
#endif

	// Send signal for mgmt
	if (RTC676x_Recv_Q == RTC676x_Recv_MGMT_Q && mgmtPacketArriveSignalThreadId)
		osSignalSet(mgmtPacketArriveSignalThreadId, mgmtPacketArriveSignal);
}
#else
static inline void RTC676x_SPI_Try_Put_Recv_Q(osMessageQId RTC676x_Recv_Q, osSemaphoreId RTC676x_Receive_Sema, RTC676x_Recv_Data *ptRecvData, int32_t *plOverrunState, int32_t ilPktType, bool bSkipIfOverwrite)
{
	if (ptRecvData->ulDataLen > RTC676x_RECV_PACKET_SIZE)
	{
		return;
	}
	
	RTC676x_Recv_Queue_Put(ilPktType, ptRecvData->pbDataBuf, ptRecvData->ulDataLen, plOverrunState, bSkipIfOverwrite);
	osSemaphoreRelease(RTC676x_Receive_Sema);

	// Send signal for mgmt
	if (RTC676x_Recv_Q == RTC676x_Recv_MGMT_Q && mgmtPacketArriveSignalThreadId)
		osSignalSet(mgmtPacketArriveSignalThreadId, mgmtPacketArriveSignal);
}
#endif
//------------------------------------------------------------------------------
static void RTC676x_SPI_DmaEnd_CallBack(void)
{
	RTC676x_DMA_Request_t tDmaRequest;
	osStatus ret;

	ret = osMessageGet(RTC676x_DMA_Request_Q, &tDmaRequest, osWaitForever);

	if (ret == osEventMessage)
	{
		if (tDmaRequest.transDirection == DMA_Trans_D_TX)
		{
			//osPoolFree(RTC676x_Pkt_Trans_Poll, tDmaRequest.pbDmaBuf);
#if defined(RTC676x_DMA_QUEUE_UNCACHED_MALLOC)
			RTC676x_DMA_Transmit_Buffer_Free(tDmaRequest.pbDmaBuf);
#else
			osPoolFree(tDmaRequest.ptDmaPool, tDmaRequest.pbDmaBuf);
#endif
//			printf("[RF] write done tick:%u\n", osKernelSysTick());
		}
		else if (tDmaRequest.transDirection == DMA_Trans_D_RX)
		{
			RTC676x_Recv_Data tRecvData;
			uint8_t ubHeaderType;
			uint32_t timeStamp = osKernelSysTick();


			tRecvData.ulDataLen = tDmaRequest.ulPktLen;
			tRecvData.pbDataBuf = tDmaRequest.pbDmaBuf;
			tRecvData.ptDmaPool = tDmaRequest.ptDmaPool;

			ubHeaderType = tRecvData.pbDataBuf[0] & 0x1F;

			switch(tDmaRequest.ulPktType)
			{
				case PKT_TYPE_MGMT:
					//osMessagePut(RTC676x_Recv_MGMT_Q, &tRecvData, osWaitForever);
					if ((ubHeaderType == MAC_PKT_BEACON) && (ubHeaderType == RTC676x_Recv_Last_Pkt_Type[PKT_TYPE_MGMT]) && ((timeStamp - RTC676x_Recv_Last_Beacon_Timestamp) < 5) && (RTC676x_Recv_Last_Beacon_Timestamp != 0))
					{
						RTC676x_Drop_Consecutive_Beacons_Cnt ++;
						break;
					}
					RTC676x_SPI_Try_Put_Recv_Q(RTC676x_Recv_MGMT_Q, RTC676x_Receive_MGMT_Sema, &tRecvData, &RTC676x_Rx_Overrun_State[PKT_TYPE_MGMT], PKT_TYPE_MGMT, true);

					RTC676x_Recv_Last_Beacon_Timestamp = 0;
					if (ubHeaderType == MAC_PKT_BEACON)
					{
						RTC676x_Recv_Last_Beacon_Timestamp = timeStamp;
						RTC676x_Tx_Stop = 0;
					}

					RTC676x_Recv_Last_Pkt_Type[PKT_TYPE_MGMT] = ubHeaderType;
					break;
				case PKT_TYPE_SMSG:
					//osMessagePut(RTC676x_Recv_SMSG_Q, &tRecvData, osWaitForever);
					RTC676x_SPI_Try_Put_Recv_Q(RTC676x_Recv_SMSG_Q, RTC676x_Receive_SMSG_Sema, &tRecvData, &RTC676x_Rx_Overrun_State[PKT_TYPE_SMSG], PKT_TYPE_SMSG, true);
					RTC676x_Recv_Last_Pkt_Type[PKT_TYPE_SMSG] = ubHeaderType;
					break;
				case PKT_TYPE_VIDEO:
					//osMessagePut(RTC676x_Recv_VIDEO_Q, &tRecvData, osWaitForever);
					RTC676x_SPI_Try_Put_Recv_Q(RTC676x_Recv_VIDEO_Q, RTC676x_Receive_VIDEO_Sema, &tRecvData, &RTC676x_Rx_Overrun_State[PKT_TYPE_VIDEO], PKT_TYPE_VIDEO, true);
					RTC676x_Recv_Last_Pkt_Type[PKT_TYPE_VIDEO] = ubHeaderType;
					break;
				case PKT_TYPE_AUDIO:
					//osMessagePut(RTC676x_Recv_AUDIO_Q, &tRecvData, osWaitForever);
					RTC676x_SPI_Try_Put_Recv_Q(RTC676x_Recv_AUDIO_Q, RTC676x_Receive_AUDIO_Sema, &tRecvData, &RTC676x_Rx_Overrun_State[PKT_TYPE_AUDIO], PKT_TYPE_AUDIO, true);
					RTC676x_Recv_Last_Pkt_Type[PKT_TYPE_AUDIO] = ubHeaderType;
					break;
				default:
					//osPoolFree(RTC676x_Pkt_Recv_Poll, tRecvData.pbDataBuf);
#if defined(RTC676x_DMA_QUEUE_UNCACHED_MALLOC)
					//RTC676x_DMA_Receive_Buffer_Free(tDmaRequest.pbDmaBuf);
#else
					//osPoolFree(tRecvData.ptDmaPool, tRecvData.pbDataBuf);
#endif
					printd(DBG_ErrorLvl, "RTC676x: Recv Data with a unknown type\n");
					break;
			}

#if defined(RTC676x_CHECK_CRC)
			if (pgRtc676xData->checkCrcMode > CRC_DONT_CHECK)
			{
				uint16_t crc16_payload, crc16_pkt;


				crc16_pkt = (tRecvData.pbDataBuf[tRecvData.ulDataLen - 2] << 8) | tRecvData.pbDataBuf[tRecvData.ulDataLen - 1];			
				crc16_payload = crc16_ccitt(&tRecvData.pbDataBuf[RTC676x_Vector_Mac_Size], tRecvData.ulDataLen - RTC676x_Vector_Mac_Size - 2);
				if (crc16_pkt != crc16_payload)
				{
					RTC676x_RX_CRC_ERRs ++;
				}
				else 
					RTC676x_RX_CRC_ERRs = 0;

				if ((RTC676x_RX_CRC_ERRs > 0) && ((RTC676x_RX_CRC_ERRs % 5) == 0))
				{
					RTC676x_RX_CRC_ERRs_Fix ++;
				//	pRtc676xData->Reg_Config2 = pRtc676xData->Reg_Config2 & 0xCF;
				//	RTC676x_Reg_Lowermac_SPI_Write(RTC676x_REG_CONFIG2, 1, &pRtc676xData->Reg_Config2);
				//	pRtc676xData->Reg_Config2 = pRtc676xData->Reg_Config2 | 0x30;
				//	RTC676x_Reg_Lowermac_SPI_Write(RTC676x_REG_CONFIG2, 1, &pRtc676xData->Reg_Config2);
					if (linkStateResetSignalThreadId)
						osSignalSet(linkStateResetSignalThreadId, linkStateResetSignal);
				}
			}
#endif

			RTC676x_DMA_Receive_Buffer_Free(tDmaRequest.pbDmaBuf);
		}
	}
	else
		printd(DBG_ErrorLvl, "RTC676x: DMA end got error request, not a Event Message\n");

}
#if 0
void RTC676x_SPI_DmaEnd_CallBack_Trans_D_TX(RTC676x_DMA_Request_t *pDmaRequest)
{
	//osPoolFree(RTC676x_Pkt_Trans_Poll, tDmaRequest.pbDmaBuf);
#if defined(RTC676x_DMA_QUEUE_UNCACHED_MALLOC)
			RTC676x_DMA_Transmit_Buffer_Free(pDmaRequest->pbDmaBuf);
#else
			osPoolFree(pDmaRequest->ptDmaPool, pDmaRequest->pbDmaBuf);
#endif
//			printf("[RF] write done tick:%u\n", osKernelSysTick());
}

void RTC676x_SPI_DmaEnd_CallBack_Trans_D_RX(RTC676x_DMA_Request_t *pDmaRequest)
{
	RTC676x_Recv_Data tRecvData;
	uint8_t ubHeaderType;
	uint32_t timeStamp = osKernelSysTick();

	tRecvData.ulDataLen = pDmaRequest->ulPktLen;
	tRecvData.pbDataBuf = pDmaRequest->pbDmaBuf;
	tRecvData.ptDmaPool = pDmaRequest->ptDmaPool;

	ubHeaderType = tRecvData.pbDataBuf[0] & 0x1F;

	switch(pDmaRequest->ulPktType)
	{
		case PKT_TYPE_MGMT:
			//osMessagePut(RTC676x_Recv_MGMT_Q, &tRecvData, osWaitForever);
			if ((ubHeaderType == MAC_PKT_BEACON) && (ubHeaderType == RTC676x_Recv_Last_Pkt_Type[PKT_TYPE_MGMT]) && ((timeStamp - RTC676x_Recv_Last_Beacon_Timestamp) < 20))
				break;

			RTC676x_SPI_Try_Put_Recv_Q(RTC676x_Recv_MGMT_Q, RTC676x_Receive_MGMT_Sema, &tRecvData, &RTC676x_Rx_Overrun_State[PKT_TYPE_MGMT], PKT_TYPE_MGMT, true);

			if (ubHeaderType == MAC_PKT_BEACON)
			{
				RTC676x_Recv_Last_Beacon_Timestamp = timeStamp;
				RTC676x_Tx_Stop = 0;
			}

			RTC676x_Recv_Last_Pkt_Type[PKT_TYPE_MGMT] = ubHeaderType;
			break;
		case PKT_TYPE_SMSG:
			//osMessagePut(RTC676x_Recv_SMSG_Q, &tRecvData, osWaitForever);
			RTC676x_SPI_Try_Put_Recv_Q(RTC676x_Recv_SMSG_Q, RTC676x_Receive_SMSG_Sema, &tRecvData, &RTC676x_Rx_Overrun_State[PKT_TYPE_SMSG], PKT_TYPE_SMSG, true);
			RTC676x_Recv_Last_Pkt_Type[PKT_TYPE_SMSG] = ubHeaderType;
			break;
		case PKT_TYPE_VIDEO:
			//osMessagePut(RTC676x_Recv_VIDEO_Q, &tRecvData, osWaitForever);
			RTC676x_SPI_Try_Put_Recv_Q(RTC676x_Recv_VIDEO_Q, RTC676x_Receive_VIDEO_Sema, &tRecvData, &RTC676x_Rx_Overrun_State[PKT_TYPE_VIDEO], PKT_TYPE_VIDEO, true);
			RTC676x_Recv_Last_Pkt_Type[PKT_TYPE_VIDEO] = ubHeaderType;
			break;
		case PKT_TYPE_AUDIO:
			//osMessagePut(RTC676x_Recv_AUDIO_Q, &tRecvData, osWaitForever);
			RTC676x_SPI_Try_Put_Recv_Q(RTC676x_Recv_AUDIO_Q, RTC676x_Receive_AUDIO_Sema, &tRecvData, &RTC676x_Rx_Overrun_State[PKT_TYPE_AUDIO], PKT_TYPE_AUDIO, true);
			RTC676x_Recv_Last_Pkt_Type[PKT_TYPE_AUDIO] = ubHeaderType;
			break;
		default:
			//osPoolFree(RTC676x_Pkt_Recv_Poll, tRecvData.pbDataBuf);
#if defined(RTC676x_DMA_QUEUE_UNCACHED_MALLOC)
			//RTC676x_DMA_Receive_Buffer_Free(pDmaRequest->pbDmaBuf);
#else
			//osPoolFree(tRecvData.ptDmaPool, tRecvData.pbDataBuf);
#endif
			printd(DBG_ErrorLvl, "RTC676x: Recv Data with a unknown type\n");
			break;
	}
}
#endif
//------------------------------------------------------------------------------
static void RTC676x_SPI_DmaEnd(void)
{
    while(SSP->SSP_BUSY) TIMER_Delay_us(20);;
    while(!RTC676x_SPI_TxEndFlag) TIMER_Delay_us(20);;
//    while(!RTC676x_SPI_RxEndFlag) TIMER_Delay_us(20);;

	if(ubRTC676x_SPI_WaitEvent == 1)
		RTC676x_SPI_CS_High();

    //SSP->SSP_EN = 0;
	RTC676x_SPI_Disable();

	SSP->SSP_RXF_TH = 1;
	SSP->SSP_TXF_TH = 1;

    SSP->SSP_TXF_DMA_EN = 0;
    SSP->SSP_RXF_DMA_EN = 0;

    APBC_CHA_DISABLE;
    APBC_CHB_DISABLE;

    if(ubRTC676x_SPI_WaitEvent == 1)
    {
        ubRTC676x_SPI_WaitEvent = 0;
        pfRTC676x_SPI_DmaEndHook();
    }
}
//------------------------------------------------------------------------------
static void RTC676x_SPI_DmaEnd2(void)
{
    while(SSP->SSP_BUSY);
    while(!RTC676x_SPI_TxEndFlag);
//    while(!RTC676x_SPI_RxEndFlag);

	RTC676x_SPI_Disable();

    SSP->SSP_TXF_DMA_EN = 0;
    SSP->SSP_RXF_DMA_EN = 0;

    APBC_CHA_DISABLE;
    APBC_CHB_DISABLE;

    if(ubRTC676x_SPI_WaitEvent == 1)
    {
        ubRTC676x_SPI_WaitEvent = 0;
        pfRTC676x_SPI_DmaEndHook();
    }
}
//------------------------------------------------------------------------------
uint8_t ubTimerCnt = 0;
uint32_t ulOSTick_Pre = 0;
uint32_t ulCheckTimerCnt = 0;
uint64_t ullPreRTC676x_ISR_Cnt = 0;
void RTC676x_Check_Timer_Handler(void const *arg)
{
#if defined(RTC676x_CHECK_INTERRUPT_STOP)
	//if (pgRtc676xData->ubRFTxMode == 1)
	{
		if ((ullPreRTC676x_ISR_Cnt != 0) && (RTC676x_ISR_Cnt != 0) && (RTC676x_ISR_Cnt == ullPreRTC676x_ISR_Cnt))
		{
			if (RTC676x_Check_Int == false)
			{
				RTC676x_Check_Int = true;
				RTC676x_RF_CHECK_INT_EVENT();
			}
		}
		else if(ullPreRTC676x_ISR_Cnt != RTC676x_ISR_Cnt)
		{
			ullPreRTC676x_ISR_Cnt = RTC676x_ISR_Cnt;
		}
	}
#endif

	ulCheckTimerCnt ++;
	if (ulCheckTimerCnt >= (1000 / pgRtc676xData->ulCheckTimerInterval))
	{
		uint32_t ulOSTick_Temp = osKernelSysTick();
		RTC676x_ISR_Per_S = (uint16_t)(RTC676x_ISR_Cnt - RTC676x_ISR_Pre_Cnt);
		RTC676x_TSLOT_Per_S = (uint16_t)(RTC676x_TSLOT_Cnt - RTC676x_TSLOT_Pre_Cnt);
		RTC676x_RXDR_Per_S = (uint16_t)(RTC676x_RXDR_Cnt - RTC676x_RXDR_Pre_Cnt);
		RTC676x_TX_DS_Per_S = (uint16_t)(RTC676x_TX_DS_Cnt - RTC676x_TX_DS_Pre_Cnt);
		RTC676x_TX_NS_Per_S = (uint16_t)(RTC676x_TX_NS_Cnt - RTC676x_TX_NS_Pre_Cnt);
		RTC676x_TSLOT_Transmit_Per_S = (uint16_t)(RTC676x_TX_Done_Cnt - RTC676x_TX_Done_Pre_Cnt);
		RTC676x_TX_DATARATE_Per_S= RTC676x_TX_DATARATE_Cnt;
	    RTC676x_VALIDIRY_TX_DATARATE_Per_S = RTC676x_TX_VALIDIRY_DATARATE_Cnt;

		RTC676x_ISR_Pre_Cnt = RTC676x_ISR_Cnt;
		RTC676x_TSLOT_Pre_Cnt = RTC676x_TSLOT_Cnt;
		RTC676x_RXDR_Pre_Cnt = RTC676x_RXDR_Cnt;
		RTC676x_TX_DS_Pre_Cnt = RTC676x_TX_DS_Cnt;
		RTC676x_TX_NS_Pre_Cnt = RTC676x_TX_NS_Cnt;
		RTC676x_TX_Done_Pre_Cnt = RTC676x_TX_Done_Cnt;

	    RTC676x_TX_DATARATE_Cnt_Clear = 1;
	    RTC676x_TX_VALIDIRY_DATARATE_Cnt_Clear = 1;

		ubTimerCnt ++;
		if (ubTimerCnt >= 100)
		{
			ubTimerCnt = 0;
			printf("TSLOT/s: %u Ticks:%u\nRXDR/s : %u, Int Check:%llu/%02X\n", RTC676x_TSLOT_Per_S, ulOSTick_Temp - ulOSTick_Pre, RTC676x_RXDR_Per_S, RTC676x_INT_ISSUE_CHECK_Cnt, RTC676x_INT_ISSUE_CHECK_CLEAR);
			printf("RF_Workaround: %llu/%llu, RX err:%llu\n", RTC676x_INT_ISSUE_CHECK_Cnt, RTC676x_INT_ISSUE_CHECK_CLEAR_Cnt, RTC676x_RXDR_Err_Cnt);
#if defined(RTC676x_USER_DEFINE_TRANSMIT_QUEUE_PRIORIT)
			printf("Pri_Q[%llu/%llu/%llu/%llu]\n", RTC676x_Priority_Q_Transmit_Cnt[0], RTC676x_Priority_Q_Transmit_Cnt[PRIORITY_HIGH + 1], RTC676x_Priority_Q_Transmit_Cnt[PRIORITY_MIDDLE + 1], RTC676x_Priority_Q_Transmit_Cnt[PRIORITY_LOW + 1]);
#endif
			printf("Drop_B: %llu\n", RTC676x_Drop_Consecutive_Beacons_Cnt);
		}

#if defined(RTC676x_DEBUG_CHECK_RUNNING)
		if (RTC676x_Debug_Print == true)
		{
			printf("<RF><I-%llu/T-%llu><%08X>\n", RTC676x_ISR_Cnt, RTC676x_Thread_Run_Cnt, RTC676x_Thread_State);
		}
#endif

		ulOSTick_Pre = ulOSTick_Temp;
		ulCheckTimerCnt = 0;
	}
}
//------------------------------------------------------------------------------
#if defined(RTC676x_DEBUG_TIMER)
uint64_t RTC676x_ISR_Cnt_Temp = 0;
void RTC676x_Debug_Timer_Handler(void const * arg)
{
	if ((RTC676x_IRQ_INPUT == 0) && (RTC676x_ISR_Cnt > 0) && (RTC676x_ISR_Cnt == RTC676x_ISR_Cnt_Temp))
	{
		RTC676x_RESET_OUT = 0;
	}

	RTC676x_ISR_Cnt_Temp = RTC676x_ISR_Cnt;
}
#endif
//------------------------------------------------------------------------------
void RTC676x_Register_MGMT_Pkt_Arrive_Signal(osThreadId thread_id, int32_t signal)
{
	mgmtPacketArriveSignalThreadId = thread_id;
	mgmtPacketArriveSignal = signal;
}

void RTC676x_Register_Link_State_Reset_Signal(osThreadId thread_id, int32_t signal)
{
	linkStateResetSignalThreadId = thread_id;
	linkStateResetSignal = signal;
}

uint64_t RTC676x_GetTXPackets(int type)
{
	return RTC676x_Transmit_Cnt[type];
}

uint64_t RTC676x_GetRXPackets(int type)
{
	return RTC676x_Receive_Cnt[type];
}

uint64_t RTC676x_GetACKPackets(int type)
{
	return RTC676x_ACK_Cnt[type];
}

uint64_t RTC676x_GetNACKPackets(int type)
{
	return RTC676x_NACK_Cnt[type];
}

uint64_t RTC676x_GetTXBytes(int type)
{
	return RTC676x_TXBytes[type];
}

uint64_t RTC676x_GetRXBytes(int type)
{
	return RTC676x_RXBytes[type];
}

uint64_t RTC676x_GetACKBytes(int type)
{
	return RTC676x_ACKedBytes[type];
}

uint64_t udlRTC676x_GetIsrCnt(void)
{
	return RTC676x_ISR_Cnt;
}


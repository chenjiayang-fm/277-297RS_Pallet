#ifndef _RWRF_H
#define _RWRF_H

#include <stddef.h>
#include <stdint.h>

/**
 * @name    Sending flags
 *
 * Flags to pass to rf_sendto_ack()
 */
/**@{*/
/** Request ack status */
#define MSG_REQACK      0x1
/**@}*/

#define PAIRING_EVENT_READY     1

/**
 * A callback function for pairing data event.
 *
 * @param arg   A user-supplied argument.
 */
typedef void(* pairing_data_cb) (void *arg);

/**
 * A callback function for pairing state event.
 *
 * @param what  event type
 * @param arg   A user-supplied argument.
 */
typedef void(* pairing_event_cb) (int what, void *arg);

/** RF statistics */
struct rfstats {
    uint64_t tx_packets;
    uint64_t rx_packets;
    uint64_t ack_packets;
    uint64_t nack_packets;
    uint64_t tx_bytes;
    uint64_t rx_bytes;
    uint64_t ack_bytes;
};

/** Preferred baseband rate */
enum PreferredBBR {
    /** BPSK */
    PREF_BBR_BPSK   = 10,

    /** QPSK */
    PREF_BBR_QPSK   = 9,

    /** 16QAM */
    PREF_BBR_16QAM  = 5,
};

enum BandwidthConfig {
    BW_NONE,
    BW_QUARTER_BANDWIDTH    = 1,
    BW_HALF_BANDWIDTH       = 2,
    BW_FULL_BANDWIDTH       = 4,
};

enum PriorityType {
    /** Video packet priority */
    PRI_TYPE_VID,

    /** Audio packet priority */
    PRI_TYPE_AUD,

    /** Short message packet priority */
    PRI_TYPE_SMS,
};

enum PriorityLevel {
    PRI_LVL_HIGH,
    PRI_LVL_MEDIUM,
    PRI_LVL_LOW,
};

/** RF option */
enum RFOption {
    /** Buffer size of sending */
    RF_SO_SNDBUF = 0x1001,

    /** Buffer size of receiving */
    RF_SO_RCVBUF,

    /** Listen before talk */
    RF_SO_LBT,

    /** Preferred baseband rate */
    RF_SO_PREFBBR,

    /** Tx power att. From 0 ~ 4. 4 is maximum */
    RF_SO_TXPWRATT,

    /** Tx power mgc. From 0 ~ 7. 0 is maximum */
    RF_SO_TXPWRMGC,

    /** Beacon power att. From 0 ~ 4. 4 is maximum */
    RF_SO_BCPWRATT,

    /** Beacon power txm ogc. From 0 ~ 7. 0 is maximum */
    RF_SO_BCPWRMGC,

    /** Ack power att. From 0 ~ 4. 4 is maximum */
    RF_SO_ACKPWRATT,

    /** Ack power txm ogc. From 0 ~ 7. 0 is maximum */
    RF_SO_ACKPWRMGC,

    /** PLCP_OFFSET. Default 432 units. 432 * 1/3 us = 144 us */
    RF_PLCP_OFFSET,

    /** Sending timeout. Default 100ms */
    RF_SO_SNDTIMEO,

    /** Software CRC. Default off */
    RF_SO_SW_CRC,

    /** Frequency table */
    RF_OPT_FREQTABLE = 0x2001,

    /** Inquiry channel map */
    RF_OPT_INQ_CH_MAP,

    /** Max sendable packet length */
    RF_OPT_PACKET_LENGTH_LIMIT,

    /** Software reset. Set any value to do reset */
    RF_OPT_SW_RST,

    /** Default 1000 ms */
    RF_OPT_BROADCAST_LINK_INTERVAL,

    /** Default 600 ms */
    RF_OPT_CHECK_LINK_INTERVAL,

    /** Default 1000 ms */
    RF_OPT_LOST_LINK_TIMEOUT,

    /** Default 400 ms */
    RF_OPT_TOUCH_TIMEOUT,

    RF_OPT_P_LISTEN,

    /** Default 160 */
    RF_OPT_MAX_NUM_BC,

    /** Default 3 */
    RF_OPT_IF_FLT_LOOP,

    /** Default 30 ms */
    RF_OPT_IF_FLT_LOOP_DELAY,

    /** Default 3 */
    RF_OPT_DC_FLT_LOOP,

    /** Default 30 ms */
    RF_OPT_DC_FLT_LOOP_DELAY,

    /** VCO calibration max retry. Default 10 */
    RF_OPT_VCO_RETRY,

    /** Default 100 ms */
    RF_OPT_VCO_LOOP_DELAY,

    /** Inquiry beacon time. Default 67 ms */
    RF_OPT_INQ_BC_TIME,

    /** Default 1200 */
    RF_OPT_EVEN_SECTION,

    /** Statistics */
    RF_OPT_STATISTICS,
    RF_OPT_STATISTICS_VIDEO,
    RF_OPT_STATISTICS_AUDIO,
    RF_OPT_STATISTICS_SMSG,

    /** Total buffer count of sending */
    RF_DRV_OPT_TX_TOTAL_BUF_CNT = 0x3001,

    /** Free buffer count of sending */
    RF_DRV_OPT_TX_FREE_BUF_CNT,

    /** Driver level debug print */
    RF_DRV_OPT_DEBUG_PRINT,

    /** VCO calibration initial values */
    RF_OPT_VCO_GROUP_0,
    RF_OPT_VCO_GROUP_1,
    RF_OPT_VCO_GROUP_2,
    RF_OPT_VCO_GROUP_3,

    /** Enable user defined VCO calibration initial value.
     *  Enable this flag also force VCO calibration pass even the result is not good enough */
    RF_OPT_VCO_USER_SETTING,
};

/**
  @brief       Initialize master side

  @param[in]   id   master id

  @return      Return 0 on success or -1 on failure.

  Example:
  @code
    int master_id = 1234;
    rf_master_init(master_id);
  @endcode
 */
int rf_master_init(int id);

/**
 * @brief       Initialize slave side
 *
 * @param[in]   id   slave id
 *
 * @return      Return 0 on success or -1 on failure.
 *
 * Example:
 * @code
 *   int slave_id = 5678;
 *   rf_slave_init(slave_id);
 * @endcode
 */
int rf_slave_init(int id);

/**
 * @brief       Initialize master production mode
 *
 * @param[in]   id   master id
 *
 * @return      Return 0 on success or -1 on failure.
 *
 * Example:
 * @code
 *   int master_id = 5678;
 *   rf_mp_init(slave_id);
 * @endcode
 */
int rf_mp_init(int id);

/**
 * @brief       End library
 *
 */
void rf_cleanup(void);

/**
 * @brief       Send packet to remote peer
 *
 * @param[in]   buf     a pointer to the data to be sent
 * @param[in]   len     the length of the data.
 *                      The maximum length depend on platform and sending mode
 * @param[in]   flags   option flags.
 *                      Use MSG_REQACK to get ack result
 * @param[in]   dest_id destination id
 * @param[out]  ack     receives ack result if not NULL
 *
 * @return      Return the number of bytes sent on success or a negative error code on failure
 *
 * Example:
 * @code
 *   unsigned char buf[] = "Hello World!";
 *   int dest_id = 1234;
 *   uint64_t ack = 0;
 *   int result = rf_send_video_packet(buf, 12, MSG_REQACK, dest_id, &ack);
 * @endcode
 */
int rf_send_video_packet(const void *buf, size_t len, int flags, int dest_id, uint64_t *ack);
int rf_send_audio_packet(const void *buf, size_t len, int flags, int dest_id, uint64_t *ack);
int rf_send_smsg_packet(const void *buf, size_t len, int flags, int dest_id, uint64_t *ack);

/**
 * @brief       Receive packet from remote peer
 *
 * @param[out]  buf     pointer to a buffer that will store the data
 * @param[in]   len     the size of the data buffer
 * @param[in]   flags   option flags
 * @param[out]  src_id  pointer to a buffer that will place the source id if not NULL
 *
 * @return      Return the number of bytes received on success or a negative error code on failure
 *
 * Example:
 * @code
 *   unsigned char buf[1024];
 *   int src_id;
 *   int result = rf_recv_video_packet(buf, sizeof(buf), 0, &src_id);
 * @endcode
 */
int rf_recv_video_packet(void *buf, size_t len, int flags, int *src_id);
int rf_recv_audio_packet(void *buf, size_t len, int flags, int *src_id);
int rf_recv_smsg_packet(void *buf, size_t len, int flags, int *src_id);

/**
 * @brief       Get ack results of sending
 *
 * @param       ack     pointer to a buffer that will store the ack results
 * @return      Return the number of acks on success or a negative error code on failure
 */
int rf_get_video_ack(uint64_t *ack);
int rf_get_audio_ack(uint64_t *ack);
int rf_get_smsg_ack(uint64_t *ack);

/**
 * @brief       Set RF option
 *
 * @param[in]   optname One of RFOption value
 * @param[in]   optval  pointer to option value
 * @param[in]   optlen  option value length
 *
 * @return      Return 0 on success or a negative on failure.
 *
 * Example:
 * @code
 *   unsigned char frequency_table[80];
 *   memset(frequency_table, 125, sizeof(frequency_table));
 *   rf_setopt(RF_OPT_FREQTABLE, frequency_table, 80);
 * @endcode
 *
 * @see         RFOption
 */
int rf_setopt(int optname, const void *optval, size_t optlen);

/**
 * @brief       Get RF option
 *
 * @param       optname One of RFOption value
 * @param       optval  pointer to a buffer that will store the value
 * @param       optlen  the size of the buffer
 *
 * @return      Return 0 on success or a negative on failure.
 *
 * Example:
 * @code
 *   unsigned char frequency_table[80];
 *   size_t frequency_table_len = sizeof(frequency_table);
 *   rf_getopt(RF_OPT_FREQTABLE, frequency_table, &frequency_table_len);
 * @endcode
 *
 * @see         RFOption
 */
int rf_getopt(int optname, void *optval, size_t *optlen);

/**
 * @brief       Start pairing mode
 *
 * Enter pairing mode.
 *
 * You have to assign read callback and event callback function.
 * Will invoke event callback and pass PAIRING_EVENT_READY when state is ready.
 *
 * Will invoke read callback function when there is data can be received.
 * Once the read callback be invoked. You should receive all data until return -1.
 *
 * @param       read_cb         callback function to be invoked when there is data can be received
 * @param       event_cb        callback function to be invoked when state is ready
 * @param       arg             an argument to be passed to the callback function
 * @param       customized_code the code to identify to prevent pairing between different products.
 *                              Pass 0 to use default code 0xFF.
 *
 * @return      Return 0 when enter pairing mode successfully or -1 on failure.
 *
 * Example:
 * @code
 *   void read_cb(void *arg)
 *   {
 *       while (1) {
 *           unsigned char buf[64] = {0};
 *           if (rf_recv_pairing_data(buf, sizeof(buf)) < 0)
 *               break;
 *
 *           // Do a quick/short processing or set a flag/signal to other thread to do processing jobs
 *           // Return this callback asap or internal jobs will not work
 *       }
 *   }
 *
 *   void event_cb(int event, void *arg)
 *   {
 *       if (event == PAIRING_EVENT_READY) {
 *           // Ready to send pairing data
 *           // Set a flag or signal to other thread to do sending jobs
 *           // Return this callback asap or internal jobs will not work
 *       }
 *   }
 *
 *   if (rf_start_pairing(read_cb, event_cb, NULL, 0) < 0) {
 *       // fail
 *   }
 * @endcode
 */
int rf_start_pairing(pairing_data_cb read_cb, pairing_event_cb event_cb, void *cb_arg, uint8_t customized_code);

/**
 * @brief       Send pairing data
 *
 * Call this function to send data after start pairing return success.
 *
 * @param[in]   buf     a pointer to the data to be written
 * @param[in]   len     the length of the data
 *                      The maximum length depend on platform and sending mode
 *
 * @return      Return the number of bytes sent on success or a negative error code on failure
 *
 * Example:
 * @code
 *   unsigned char buf[32];
 *   if (rf_send_pairing_data(buf, sizeof(buf)) < 0) {
 *       // fail
 *   }
 * @endcode
 */
int rf_send_pairing_data(const void *buf, size_t len);

/**
 * @brief       Receive pairing data
 *
 * Call this function to receive data only when the readable callback has been invoked.
 * Once the readable callback beed invoke. You should receive all data until return -1.
 *
 * @param[out]  buf     pointer to a buffer that will store the data
 * @param[in]   len     the size of the data buffer
 *
 * @return      Return the number of bytes received on success or a negative error code on failure
 *
 * @see         rf_start_pairing()
 */
int rf_recv_pairing_data(void *buf, size_t len);

/**
 * @brief       Stop pairing mode
 *
 * Call this function to exit pairing mode.
 *
 * @return      Return 0 on success or -1 on failure.
 */
int rf_stop_pairing(void);

/**
 * @brief       Get local id
 *
 * @return      -1 on failure. Return local id on success.
 */
int rf_get_local_id(void);

/**
 * @brief       Get remote id
 *
 * @param[in]   index   The index.
 *                      Master side get id of slaves at index 0 ~ 3.
 *                      Slave side get master id at index 0.
 *
 * @return      -1 on failure. Return remote id on success.
 */
int rf_get_remote_id(unsigned int index);

/**
 * @brief       Set local id
 *
 * @param[in]   id      The id
 *
 * @return      Return 0 on success or -1 on failure.
 */
int rf_set_local_id(int id);

/**
 * @brief       Set remote id
 *
 * @param[in]   idv     The id vector
 * @param[in]   idvcnt  The id count
 *
 * @return      Return 0 on success or -1 on failure.
 *
 * Example:
 * @code
 *   int slave_id[4] = { 0xC0, 0xC1, 0xC2, 0xC3 };
 *   rf_set_remote_id(slave_id, 4);
 * @endcode
 */
int rf_set_remote_id(int *idv, int idvcnt);

/**
 * @brief       Get the link status of the corresponding id
 *
 * @param[in]   id      The remote id
 *
 * @return      0 Lost link
 *              1 Link established
 */
int rf_get_link_status(int id);

/**
 * @brief       Get the allocated bandwidth configuration
 *
 * @return      Return BW_NONE, BW_FULL_BANDWIDTH, BW_HALF_BANDWIDTH or BW_QUARTER_BANDWIDTH
 *
 * @see         BandwidthConfig
 */
int rf_get_bandwidth_config(void);

/**
 * @brief       Allocate bandwidth of all slaves manually. Set all BW_NONE to reset to auto mode
 *              This function is only for master
 *              The total amount of bandwidth can not exceed BW_FULL_BANDWIDTH
 *
 * @param[in]   bwv      The bandwidth configuration vector. One of BandwidthConfig value.
 * @param[in]   bwvcnt   The bandwidth configuration vector size.
 *
 * @return      Return 0 on success or -1 on failure.
 *
 * Example:
 * @code
 *   int bwv[4];
 *   bwv[0] = BW_QUARTER_BANDWIDTH;
 *   bwv[1] = BW_QUARTER_BANDWIDTH;
 *   bwv[2] = BW_HALF_BANDWIDTH;
 *   bwv[3] = BW_NONE;
 *   int result = rf_set_slave_bandwidth(bwv, 4);
 * @endcode
 *
 * @see         BandwidthConfig
 */
int rf_set_slave_bandwidth(int *bwv, int bwvcnt);

/**
 * @brief       Get the sending priority of specific type packet
 *
 * @param[in]   type    The packet type. One of PriorityType value
 * @return      Return PRI_LVL_HIGH, PRI_LVL_MEDIUM, PRI_LVL_LOW or -1 on failure.
 *
 * Example:
 * @code
 *   int priority = rf_get_packet_priority(PRI_TYPE_VID);
 * @endcode
 *
 * @see         PriorityType, PriorityLevel
 */
int rf_get_packet_priority(int type);

/**
 * @brief       Set the sending priority of specific type packet
 *
 * @param[in]   type    The packet type. One of PriorityType value
 * @param[in]   level   The priority level. One of PriorityLevel value
 * @return      Return 0 on success or -1 on failure.
 *
 * Example:
 * @code
 *   rf_set_packet_priority(PRI_TYPE_AUD, PRI_LVL_HIGH);
 * @endcode
 *
 * @see         PriorityType, PriorityLevel
 */
int rf_set_packet_priority(int type, int level);

//==============================RF MP Test========================================
/**
 * @brief       Set the input string from for RF MP test
 *
 * @param[in]   len    The length of string.
 * @param[in]   pStr   The pointer of string
 * @return      Return 0 on success or -1 on failure.
 *
 * Example:
 * @code
 *   rf_mp_put_uart_string(strlen(cmd), cmd);
 * @endcode
 *
 * @see
 */
int rf_mp_put_uart_string(int len, char *pStr);

/**
 * @brief       Set the key event for RF MP test
 */
void rf_mp_put_pairkey_event(void);

/**
 * @brief       Initialize RF MP test
 *
 * @return      Return 0 on success or -1 on failure.
 */
int rf_mp_test_main(void);
//==============================RF MP Test end====================================

/**
 * @brief       Get library version
 *
 * @return      a string containing the version number of library
 */
const char *rf_version(void);

/**
 * @brief       Get the library build timestamp
 *
 * @return      Returns the library build timestamp string
 */
const char *rf_build_time(void);

/**
 * @brief       Get the debug trace information (counter, line and function name)
 *
 *              Sample:
 *              #30, 37, foo
 *              #31, 24, bar
 *              #32, 49, set_link
 *
 * @param       buf     The pointer to a buffer that trace info will write to
 * @param       len     The buffer length
 * @return      Return the number of characters printed
 */
int rf_print_trace(char *buf, size_t len);
#endif /* _RWRF_H */

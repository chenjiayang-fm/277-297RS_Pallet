/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		ADO_API.h
	\brief		Audio header file
	\author		Chinwei Hsu/Bruce Hsu
	\version	3.56
	\date		2021/10/28
	\copyright	Copyright(C) 2017 SONiX Technology Co.,Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------
#ifndef _AUDIO_API_H_
#define _AUDIO_API_H_

#include "_510PF.h"

#define ADO_AUDIO32_MAX_NUM 	7	//don't modify, otherwise you must rebuild lib
#define AUDIO32_ENCODE_SIZE		640
#define AUDIO32_DECODE_SIZE		40
#define ADO_SRC_NUM				4
#define ADO_REC_SRC_NUM			(ADO_SRC_NUM+1)	//+1 is for local recording

#define ADO_STREAM_TAG			"SNX"

#pragma pack(push)
#pragma pack(1)
// Audio function switch
typedef enum 
{
	ADO_OFF = 0,    //!< Audio register switch : off
	ADO_ON          //!< Audio register switch : on
}ADO_FUN_SWITCH;
//------------------------------------------------------------------------------
typedef enum 
{
	NONE = 0,	//no software encode
	ADO32,
	AAC,
	ALAW
}ADO_ENCODE_TYPE;
//------------------------------------------------------------------------------
// AEC/NR return flag
typedef enum
{
	AEC_NR_ERROR = 0,  //!< AEC/NR return error
	AEC_NR_OK          //!< AEC/NR return ok
}AEC_NR_RETURN_FLAG;
//------------------------------------------------------------------------------
typedef enum 
{
	ADO_SUCCESS,
	ADO_FAIL
}ADO_RETURN_FLAG;
//------------------------------------------------------------------------------
typedef enum 
{
    DAC_RDY,
	PLAY_BUF_EMP	
}ADO_DAC_EVENT;
//------------------------------------------------------------------------------
typedef enum 
{
	NOISE_DISABLE,
	NOISE_AEC,
	NOISE_NR,
	NOISE_ALL
}ADO_NOISE_PROCESS_TYPE;
//------------------------------------------------------------------------------
// System speed mode
typedef enum
{
	HIGH_SPEED = 0, //!< ST53510 system speed : high(DDR mode)
	LOW_POWER       //!< ST53510 system speed : low(SRAM mode)
}ADO_SYS_SPEED_MODE_t;
//------------------------------------------------------------------------------
// Audio ADC device
typedef enum
{
	SIG_DEL_ADC = 0,    //!< Sigma-delta ADC
	I2S_ADC,            //!< I2S ADC
}ADO_ADC_DEV_t;
//------------------------------------------------------------------------------
// Audio DAC device
typedef enum
{
	R2R_DAC = 0,    //!< R2R DAC
	I2S_DAC         //!< I2S DAC
}ADO_DAC_DEV_t;
//------------------------------------------------------------------------------
// Audio unsigned/signed signal
typedef enum
{
	UNSIGNED = 0,   //!< Unsigned signal
	SIGNED          //!< Signed signal
}ADO_SIGNAL_MODE;
//------------------------------------------------------------------------------
// Audio channel : Stereo/Mono
typedef enum
{
	STEREO = 0, //!< Stereo channel
	MONO        //!< Mono channel
}ADO_CHANNEL_MODE;
//------------------------------------------------------------------------------
// Audio sample size : 16/8-bit
typedef enum
{
	SAMPLESIZE_16_BIT = 0,  //!< 16-bit sample size
	SAMPLESIZE_8_BIT        //!< 8-bit sample size
}ADO_SAMPLESIZE;
//------------------------------------------------------------------------------
// Audio sample rate
typedef enum
{
	SAMPLERATE_8kHZ = 0,    //!< 8kHz sample rate
	SAMPLERATE_16kHZ,       //!< 16kHz sample rate
	SAMPLERATE_24kHZ,       //!< 24kHz sample rate
	SAMPLERATE_32kHZ,       //!< 32kHz sample rate
	SAMPLERATE_48kHZ,       //!< 48kHz sample rate
	SAMPLERATE_11_025kHZ,   //!< 11.025kHz sample rate
	SAMPLERATE_22_05kHZ,	//!< 22.05kHz sample rate
	SAMPLERATE_44_1kHZ      //!< 44.1kHz sample rate
}ADO_SAMPLERATE;
//------------------------------------------------------------------------------
// Audio compress function
typedef enum
{
	COMPRESS_NONE = 0,  //!< No compress audio
	COMPRESS_ALAW,      //!< A-law compression
}ADO_COMPRESS_MODE;
//------------------------------------------------------------------------------
// Audio adpcm step size
typedef enum
{
	STEP_16 = 0,    //!< ADPCM minimum step size : 16
	STEP_1          //!< ADPCM minimum step size : 1
}ADO_ADPCM_STEP_SIZE;
//------------------------------------------------------------------------------
// Audio buffer size
typedef enum
{
	BUF_SIZE_512B = 0,  //!< 512Bytes buffer size
	BUF_SIZE_1KB,       //!< 1KB buffer size
	BUF_SIZE_2KB,       //!< 2KB buffer size
	BUF_SIZE_4KB,       //!< 4KB buffer size
	BUF_SIZE_8KB,       //!< 8KB buffer size
	BUF_SIZE_16KB,      //!< 16KB buffer size
	BUF_SIZE_32KB,      //!< 32KB buffer size
	BUF_SIZE_64KB,      //!< 64KB buffer size
	//the following buf size don't be allowed to use for adc/dac
	BUF_SIZE_0KB,		//!< 0KB buffer size
	BUF_SIZE_96KB,      //!< 96KB buffer size
	BUF_SIZE_128KB,		//!< 128KB buffer size
	BUF_SIZE_160KB,		//!< 160KB buffer size
	BUF_SIZE_192KB,		//!< 192KB buffer size
	BUF_SIZE_224KB,		//!< 224KB buffer size
	BUF_SIZE_256KB,		//!< 256KB buffer size
	BUF_SIZE_288KB,		//!< 288KB buffer size
	BUF_SIZE_320KB,		//!< 320KB buffer size
	BUF_SIZE_384KB,		//!< 384KB buffer size
}ADO_BUFFERSIZE;
//------------------------------------------------------------------------------
// Audio buffer threshold
typedef enum
{
	BUF_TH_1KB = 2,     //!< 1KB buffer threshold
	BUF_TH_2KB,         //!< 2KB buffer threshold
	BUF_TH_4KB,         //!< 4KB buffer threshold
	BUF_TH_8KB,         //!< 8KB buffer threshold
	BUF_TH_16KB,        //!< 16KB buffer threshold
	BUF_TH_32KB         //!< 32KB buffer threshold
}ADO_BUFFERTH;
//------------------------------------------------------------------------------// Audio ADC Gain
typedef enum
{
	ADC_GAIN_0DB = 0,   //!< ADC gain : 0dB
	ADC_GAIN_0p5DB,     //!< ADC gain : 0.5dB
	ADC_GAIN_1DB,       //!< ADC gain : 1dB
	ADC_GAIN_1p5DB,     //!< ADC gain : 1.5dB
	ADC_GAIN_2DB,       //!< ADC gain : 2dB
	ADC_GAIN_2p5DB,     //!< ADC gain : 2.5dB
	ADC_GAIN_3DB,       //!< ADC gain : 3dB
	ADC_GAIN_3p5DB,     //!< ADC gain : 3.5dB
	ADC_GAIN_4DB,       //!< ADC gain : 4dB
	ADC_GAIN_4p5DB,     //!< ADC gain : 4.5dB
	ADC_GAIN_5DB,       //!< ADC gain : 5dB
	ADC_GAIN_5p5DB,     //!< ADC gain : 5.5dB
	ADC_GAIN_6DB,       //!< ADC gain : 6dB
	ADC_GAIN_6p5DB,     //!< ADC gain : 6.5dB
	ADC_GAIN_7DB,       //!< ADC gain : 7dB
	ADC_GAIN_7p5DB,     //!< ADC gain : 7.5dB
	ADC_GAIN_8DB,       //!< ADC gain : 8dB
	ADC_GAIN_8p5DB,     //!< ADC gain : 8.5dB
	ADC_GAIN_9DB,       //!< ADC gain : 9dB
	ADC_GAIN_9p5DB,     //!< ADC gain : 9.5dB
	ADC_GAIN_10DB       //!< ADC gain : 10dB
}ADO_ADCGAIN;
//------------------------------------------------------------------------------
// Audio ADC mute speed
typedef enum
{
	ADC_MS_3DB_4SAMPLE = 0, //!< ADC mute speed : amplitude -3dB every 4 sample
	ADC_MS_3DB_8SAMPLE,     //!< ADC mute speed : amplitude -3dB every 8 sample
	ADC_MS_3DB_16SAMPLE,    //!< ADC mute speed : amplitude -3dB every 16 sample
	ADC_MS_3DB_32SAMPLE     //!< ADC mute speed : amplitude -3dB every 32 sample
}ADO_ADCMUTESPEED;
//------------------------------------------------------------------------------
// Audio DAC rmp rate
typedef enum
{
	DAC_MR_0p5DB_1SAMPLE = 0,   //!< DAC mute speed : 0.5 dB per 1 sample
	DAC_MR_0p5DB_2SAMPLE,       //!< DAC mute speed : 0.5 dB per 2 sample
	DAC_MR_0p5DB_4SAMPLE,       //!< DAC mute speed : 0.5 dB per 4 sample
	DAC_MR_0p5DB_8SAMPLE        //!< DAC mute speed : 0.5 dB per 8 sample
}AUDIO_DACMUTERMP;
//------------------------------------------------------------------------------
// Audio R2R volume
typedef enum 
{
	R2R_VOL_n45DB = 0,  //!< R2R volume : -45dB
	R2R_VOL_n42DB,      //!< R2R volume : -42dB
	R2R_VOL_n39p1DB,    //!< R2R volume : -39.1dB
	R2R_VOL_n36DB,      //!< R2R volume : -36dB
	R2R_VOL_n32p4DB,    //!< R2R volume : -32.4dB
	R2R_VOL_n29p8DB,    //!< R2R volume : -29.8dB
	R2R_VOL_n26p2DB,    //!< R2R volume : -26.2dB
	R2R_VOL_n23p5DB,    //!< R2R volume : -23.5dB
	R2R_VOL_n21p4DB,    //!< R2R volume : -21.4dB
	R2R_VOL_n18p2DB,    //!< R2R volume : -18.2dB
	R2R_VOL_n14p6DB,    //!< R2R volume : -14.6dB
	R2R_VOL_n11p9DB,    //!< R2R volume : -11.9dB
	R2R_VOL_n8p2DB,     //!< R2R volume : -8.2dB
	R2R_VOL_n5p6DB,     //!< R2R volume : -5.6dB
	R2R_VOL_n3DB,       //!< R2R volume : -3dB
	R2R_VOL_n0DB        //!< R2R volume : -0dB
}ADO_R2R_VOL;
//------------------------------------------------------------------------------
// Audio DAC Gain
typedef enum 
{
	DAC_GAIN_n0DB = 0,  //!< DAC gain : -0dB
	DAC_GAIN_n2p5DB,    //!< DAC gain : -2.5dB
	DAC_GAIN_n6DB,      //!< DAC gain : -6dB
	DAC_GAIN_n8p5DB,    //!< DAC gain : -8.5dB
	DAC_GAIN_n12DB,     //!< DAC gain : -12dB
	DAC_GAIN_n14DB,     //!< DAC gain : -14dB
	DAC_GAIN_n18DB,     //!< DAC gain : -18dB
	DAC_GAIN_n20DB,     //!< DAC gain : -20dB
	DAC_GAIN_n24DB,     //!< DAC gain : -24dB
	DAC_GAIN_n26p6DB,   //!< DAC gain : -26.6dB
	DAC_GAIN_n30p1DB,   //!< DAC gain : -30.1dB
	DAC_GAIN_n32p6DB,   //!< DAC gain : -32.6dB
	DAC_GAIN_n36p1DB,   //!< DAC gain : -36.1dB
	DAC_GAIN_n38p6DB,   //!< DAC gain : -38.6dB
	DAC_GAIN_n42p1DB,   //!< DAC gain : -42.1dB
	DAC_GAIN_n44p6DB    //!< DAC gain : -44.6dB
}ADO_DACGAIN;
//------------------------------------------------------------------------------
// Audio upsampling rate
typedef enum 
{
	UPSAMPLING_2x_out = 0,  //!< Upsample 2 times output
	UPSAMPLING_3x_out,      //!< Upsample 3 times output
	UPSAMPLING_4x_out,      //!< Upsample 4 times output
	UPSAMPLING_6x_out       //!< Upsample 6 times output
}ADO_UPSAMPLING;
//------------------------------------------------------------------------------
// Audio gpio pin
typedef enum 
{
	ADO_I2S_ADC_DATA = 0x01,    //!< Audio gpio pin : I2S ADC DATA
	ADO_I2S_ADC_BCLK = 0x02,    //!< Audio gpio pin : I2S ADC BCLK
	ADO_I2S_DAC_DATA = 0x04,    //!< Audio gpio pin : I2S DAC DATA
	ADO_I2S_ADC_LRCLK = 0x08,   //!< Audio gpio pin : I2S ADC LRCLK
	ADO_I2S_ADC_MCLK = 0x10     //!< Audio gpio pin : I2S ADC MCLK
}ADO_GPIO_PIN;
//------------------------------------------------------------------------------
// Audio gpio type
typedef enum 
{
	ADO_GPIO_INPUT = 0, //!< Audio gpio type : input
	ADO_GPIO_OUTPUT     //!< Audio gpio type : output
}ADO_GPIO_TYPE;
//------------------------------------------------------------------------------
// Audio gpio value
typedef enum 
{
	ADO_GPIO_LOW = 0,   //!< Audio gpio value : low
	ADO_GPIO_HIGH       //!< Audio gpio value : high
}ADO_GPIO_VALUE;
//------------------------------------------------------------------------------
typedef enum SNX_AUD32_FORMAT {
	SNX_AUD32_FMT16_8K_8KBPS = 8,
	SNX_AUD32_FMT16_8K_16KBPS  = 13,
	SNX_AUD32_FMT16_16K_16KBPS = 29,
}ADO_SNX_AUD32_FORMAT;
//------------------------------------------------------------------------------
// ALAW return flag
typedef enum
{
	ALAW_ERROR = 0,  //!< ALAW return error
	ALAW_OK          //!< ALAW return ok
}ALAW_RETURN_FLAG_t;
//------------------------------------------------------------------------------
// AEC/NR sample rate mode
typedef enum
{
	AEC_NR_8kHZ = 8000,         //!< AEC/NR sample rate mode : 8kHz
	AEC_NR_16kHZ = 16000,       //!< AEC/NR sample rate mode : 16kHz
	AEC_NR_11_025kHZ = 11025    //!< AEC/NR sample rate mode : 11.025kHz
}AEC_NR_SAMPLERATE_MODE_t;
//------------------------------------------------------------------------------
// Audio IP ready status
typedef enum
{
    ADO_IP_NONREADY = 0,
    ADO_IP_READY
}ADO_IP_READY_t;
//------------------------------------------------------------------------------
// Sigma-delta ADC mode
typedef enum
{
    ADO_SIG_DIFFERENTIAL = 0,
    ADO_SIG_SINGLE_END = 1
}ADO_SIG_DEL_ADC_MODE;
//------------------------------------------------------------------------------
// Sigma-delta ADC BOOST gain
typedef enum
{
    ADO_SIG_BOOST_0DB = 0,  //!< Sigma-delta ADC BOOST gain: +0dB
    ADO_SIG_BOOST_20DB,     //!< Sigma-delta ADC BOOST gain: +20dB
    ADO_SIG_BOOST_30DB,     //!< Sigma-delta ADC BOOST gain: +30dB
    ADO_SIG_BOOST_37DB      //!< Sigma-delta ADC BOOST gain: +37dB
}ADO_SIG_DEL_ADC_BOOST_GAIN;
//------------------------------------------------------------------------------
// Sigma-delta ADC PGA gain
typedef enum
{
    ADO_SIG_PGA_MUTE = 0,   //!< Sigma-delta ADC PGA gain: mute
    ADO_SIG_PGA_n12DB,      //!< Sigma-delta ADC PGA gain: -12dB
    ADO_SIG_PGA_n10p5DB,    //!< Sigma-delta ADC PGA gain: -10.5dB
    ADO_SIG_PGA_n9DB,       //!< Sigma-delta ADC PGA gain: -9dB
    ADO_SIG_PGA_n7p5DB,     //!< Sigma-delta ADC PGA gain: -7.5dB
    ADO_SIG_PGA_n6DB,       //!< Sigma-delta ADC PGA gain: -6dB
    ADO_SIG_PGA_n4p5DB,     //!< Sigma-delta ADC PGA gain: -4.5dB
    ADO_SIG_PGA_n3DB,       //!< Sigma-delta ADC PGA gain: -3dB
    ADO_SIG_PGA_n1p5DB,     //!< Sigma-delta ADC PGA gain: -1.5dB
    ADO_SIG_PGA_0DB,        //!< Sigma-delta ADC PGA gain: +0dB
    ADO_SIG_PGA_1p5DB,      //!< Sigma-delta ADC PGA gain: +1.5dB
    ADO_SIG_PGA_3DB,        //!< Sigma-delta ADC PGA gain: +3dB
    ADO_SIG_PGA_4p5DB,      //!< Sigma-delta ADC PGA gain: +4.5dB
    ADO_SIG_PGA_6DB,        //!< Sigma-delta ADC PGA gain: +6dB
    ADO_SIG_PGA_7p5DB,      //!< Sigma-delta ADC PGA gain: +7.5dB
    ADO_SIG_PGA_9DB,        //!< Sigma-delta ADC PGA gain: +9dB
    ADO_SIG_PGA_10p5DB,     //!< Sigma-delta ADC PGA gain: +10.5dB
    ADO_SIG_PGA_12DB,       //!< Sigma-delta ADC PGA gain: +12dB
    ADO_SIG_PGA_13p5DB,     //!< Sigma-delta ADC PGA gain: +13.5dB
    ADO_SIG_PGA_15DB,       //!< Sigma-delta ADC PGA gain: +15dB
    ADO_SIG_PGA_16p5DB,     //!< Sigma-delta ADC PGA gain: +16.5dB
    ADO_SIG_PGA_18DB,       //!< Sigma-delta ADC PGA gain: +18dB
    ADO_SIG_PGA_19p5DB,     //!< Sigma-delta ADC PGA gain: +19.5dB
    ADO_SIG_PGA_21DB,       //!< Sigma-delta ADC PGA gain: +21dB
    ADO_SIG_PGA_22p5DB,     //!< Sigma-delta ADC PGA gain: +22.5dB
    ADO_SIG_PGA_24DB,       //!< Sigma-delta ADC PGA gain: +24dB
    ADO_SIG_PGA_25p5DB,     //!< Sigma-delta ADC PGA gain: +25.5dB
    ADO_SIG_PGA_27DB,       //!< Sigma-delta ADC PGA gain: +27dB
    ADO_SIG_PGA_28p5DB,     //!< Sigma-delta ADC PGA gain: +28.5dB
    ADO_SIG_PGA_30DB,       //!< Sigma-delta ADC PGA gain: +30dB
    ADO_SIG_PGA_31p5DB,     //!< Sigma-delta ADC PGA gain: +31.5dB
    ADO_SIG_PGA_33DB        //!< Sigma-delta ADC PGA gain: +33dB
}ADO_SIG_DEL_ADC_PGA_GAIN;
//------------------------------------------------------------------------------
// Audio play mode
typedef enum
{
	NORMAL_PLY = 0,	//!< Dac normal play type
	WAV_PLY,		//!< Dac wav play type
	MOVIE_PLY		//!< Sd card movie play
}ADO_PLY_TYPE;
//------------------------------------------------------------------------------
typedef enum 
{
	ADO_WAV_IDLE = 0,
	ADO_WAV_PRESET_PLAYING,
	ADO_WAV_PLAYING,
	ADO_WAV_STOP
}ADO_WAV_STATE;
//------------------------------------------------------------------------------
typedef enum
{
	DeHowlingLV0 = 0,
	DeHowlingLV1 = 1,										//!< cant play at this state
	DeHowlingLV2 = 2,
	DeHowlingLV3 = 3,
	DeHowlingLV4 = 4,
	DeHowlingLV5 = 5,
	DeHowlingLV6 = 6
	//!< allow play data
}ADO_DeHowling_LV;
//------------------------------------------------------------------------------
// Audio I2S clock mode
typedef enum
{
	I2S_MODE_MASTER = 0, //!< I2S master mode
	I2S_MODE_SLAVE       //!< I2S slave mode
}AUDIO_I2S_MODE;
//------------------------------------------------------------------------------
// Audio dac play buffer access status
typedef enum
{
	DEC_BUF_ERR = 0,	//!< it has some incorrect parameter
	DEC_BUF_FULL,		//!< can't write data to dec buf
	DEC_BUF_EMPTY,		//!< can write data to dec buf
}ADO_DEC_BUF_SPACE_CHK;
//------------------------------------------------------------------------------
typedef struct ADO_Aud32_Enc_Info
{
	uint32_t ulOutputSize;
	uint32_t ulRemainStartAddr;
	uint32_t ulRemainSize;
}ADO_AUD32_ENC_INFO;
//------------------------------------------------------------------------------
typedef struct ADO_Aud32_Dec_Info
{
	uint32_t ulResultSize;
}ADO_AUD32_DEC_INFO;
//------------------------------------------------------------------------------
// Audio format
typedef struct AUDIO_FORMAT
{
	ADO_SIGNAL_MODE sign_flag;        //!< Unsigned/Signed signal
	ADO_CHANNEL_MODE channel;        //!< Stereo/Mono channel
	ADO_SAMPLESIZE sample_size;    //!< 16/8-bit sample size
	ADO_SAMPLERATE sample_rate;    //!< Sample rate
}ADO_FORMAT_t;
//------------------------------------------------------------------------------
typedef struct AUDIO_BUFFER_SIZE_SETTING
{
	ADO_BUFFERSIZE Adc;			//!< adc hw buffer size
	ADO_BUFFERSIZE Dac;			//!< dac hw buffer size
	ADO_BUFFERSIZE PreHwPly;	//!< prepare to play data from sw buffer to hw buffer
	ADO_BUFFERSIZE TempProc;	//!< temp process buffer size(store data from adc hw buffer)
	ADO_BUFFERSIZE DeHowling;	//!< de-howling process buffer size
	ADO_BUFFERSIZE WavPlyTemp;	//!< wav play temp buffer size(store data from SF temporally)
	ADO_BUFFERSIZE WavPly;		//!< wav play buffer size(store data from WavPlyTemp and will send to decode thread)
	ADO_BUFFERSIZE Ado32SwEn;	//!< sofrware audio32 encode buffer size
	ADO_BUFFERSIZE Ado32SwDe;	//!< sofrware audio32 decode buffer size
	ADO_BUFFERSIZE AacSwEn;		//!< sofrware aac encode buffer size
	ADO_BUFFERSIZE AacSwDe;		//!< sofrware aac decode buffer size
	ADO_BUFFERSIZE AlawSwEn;	//!< sofrware alaw encode buffer size
	ADO_BUFFERSIZE AlawSwDe;	//!< sofrware alaw decode buffer size
	ADO_BUFFERSIZE RecPkt;		//!< recording packet buffer size(store data from external thread and will send to other thread for further processing)
	ADO_BUFFERSIZE RecEn;		//!< recording encoded buffer size(store data encoded by encoder and will send to other thread for recording)
	ADO_BUFFERSIZE RecMix;		//!< recording mixer buffer(used only for Rx with lens, this buffer will store encoded mixed sound(from Tx and Rx) and send to other thread for recording)
	ADO_BUFFERSIZE Mix;			//!< mixer buffer(used only for mixing Tx sound and this buffer will store each Tx sound for further mixing processing)
}ADO_BUF_SZ_SETTING_t;
//------------------------------------------------------------------------------
typedef struct AUDIO_BUFFER_THRESHOLD_SETTING
{
	ADO_BUFFERTH Adc;	//!< adc hw buffer threshold
	ADO_BUFFERTH Dac;	//!< dac hw buffer threshold
}ADO_BUF_TH_SETTING_t;
//------------------------------------------------------------------------------
typedef uint32_t(*ADO_Get1msHandler)(void);
// Audio parameter for kernal setting
typedef struct KNL_ADO_PARAMETER
{	
	uint8_t ubQ_InitFlg;                //!< Audio queue initial, 0:No init; 1:Init
	
	uint8_t ubSupSrcNum;                //!< Audio support source number, the range of value=1~ADO_SRC_NUM
	
	ADO_SYS_SPEED_MODE_t SysSpeed;		//!< System speed mode
	
	ADO_ADC_DEV_t AdcDev;				//!< ADC device
	ADO_DAC_DEV_t DacDev;				//!< DAC device	
	
    ADO_SIG_DEL_ADC_MODE SigDelAdcMode; //!< Sigma-delta ADC mode(if ADC device is Sigma-delta ADC)
    
	AUDIO_I2S_MODE I2S_Mode;			//!< I2S master/slave mode
	
	ADO_FORMAT_t AdcFmt;               	//!< ADC format
	ADO_FORMAT_t DacFmt;               	//!< DAC format
	
	ADO_COMPRESS_MODE HwCompressMode;	//!< hardware compress mode
	
	ADO_BUF_SZ_SETTING_t BufSz;			//!< buffer size setting
	ADO_BUF_TH_SETTING_t BufTh;			//!< buffer threshold setting
	
	uint32_t ulSelfTestTime;			// unit: 1 second
	
	ADO_Get1msHandler Get1ms;
	
	uint32_t ulBufStartAddr;        	//!< Audio buffer start address(4-byte alingment!!)
}ADO_KNL_PARA_t;
//------------------------------------------------------------------------------
typedef struct ADO_Queue_Info
{
	ADO_PLY_TYPE PlyType;
	ADO_ENCODE_TYPE EncType;
	uint32_t SrcAddr;
	uint32_t SrcSize;
	uint8_t ubSrcNum;
	uint32_t ulDecAddr;
	uint8_t ubPlaySrcNum;
	uint32_t ulQueueNum;
}ADO_Queue_INFO;
//------------------------------------------------------------------------------
// Audio additional packet header info
typedef struct AUDIO_ENC_PACK_INFO
{
	char chADO_StreamTag[4];
	uint32_t ulStartupFlag;
	uint32_t ulTimestamp;
	uint32_t ulEncodedSize;
	//other information
	ADO_ENCODE_TYPE EncType;
	ADO_SAMPLERATE SampleRate;
	ADO_CHANNEL_MODE Channel;
	uint8_t ubReserved;
}ADO_ENC_PACK_INFO_t;
//------------------------------------------------------------------------------
typedef ALAW_RETURN_FLAG_t(*ADO_CbFun_AlawEnc)(uint32_t, uint32_t, uint32_t, uint32_t*);
typedef ALAW_RETURN_FLAG_t(*ADO_CbFun_AlawDec)(uint32_t, uint32_t, uint32_t, uint32_t*);

typedef void (*ADO_CbFunc_Ado32EncInit)(uint8_t, ADO_SNX_AUD32_FORMAT);
typedef void (*ADO_CbFunc_Ado32DecInit)(uint8_t, ADO_SNX_AUD32_FORMAT);
typedef ADO_AUD32_ENC_INFO (*ADO_CbFunc_Ado32Enc)(uint8_t, uint32_t, uint32_t, uint32_t);
typedef ADO_AUD32_DEC_INFO (*ADO_CbFunc_Ado32Dec)(uint8_t, uint32_t, uint32_t, uint32_t);

typedef ADO_RETURN_FLAG (*ADO_CbFunc_AacEncInit)(uint8_t, uint8_t, uint32_t, uint32_t);
typedef ADO_RETURN_FLAG (*ADO_CbFunc_AacDecInit)(uint8_t, uint8_t, uint32_t, uint32_t);
typedef void (*ADO_CbFunc_AacEncUnInit)(uint8_t);
typedef void (*ADO_CbFunc_AacDecUnInit)(uint8_t);
typedef uint32_t (*ulADO_CbFunc_AacGetEncUnitSize)(uint8_t);
typedef ADO_RETURN_FLAG (*ADO_CbFunc_AacEncEncode)(uint8_t, uint32_t, uint32_t, uint32_t, uint32_t*);
typedef ADO_RETURN_FLAG (*ADO_CbFunc_AacEncEncodeFlush)(uint8_t ubUseIdx, uint32_t DestAddr, uint32_t*);
typedef ADO_RETURN_FLAG (*ADO_CbFunc_AacDecDecode)(uint8_t, uint32_t, uint32_t, uint32_t, uint32_t*);
//------------------------------------------------------------------------------
#pragma pack(pop)
//------------------------------------------------------------------------------

//==============================================================================
// Audio External API
//==============================================================================
//------------------------------------------------------------------------------
/*
\brief Set ado enc/dec cb function(must call it before ADO_Setup)
*/
void ADO_SetAlawEncDecCb(ADO_CbFun_AlawEnc EncCb, ADO_CbFun_AlawDec DecCb);
void ADO_SetAdo32EncDecCb(ADO_CbFunc_Ado32EncInit EncInitCb, ADO_CbFunc_Ado32DecInit DecInitCb, ADO_CbFunc_Ado32Enc EncCb, ADO_CbFunc_Ado32Dec DecCb);
void ADO_SetAacEncDecCb(ADO_CbFunc_AacEncInit EncInitCb, ADO_CbFunc_AacDecInit DecInitCb, ADO_CbFunc_AacEncUnInit EncUninitCb, ADO_CbFunc_AacDecUnInit DecUninitCb, ulADO_CbFunc_AacGetEncUnitSize GetEncUnitSzCb, ADO_CbFunc_AacEncEncode EncCb, ADO_CbFunc_AacEncEncodeFlush EncFlushCb, ADO_CbFunc_AacDecDecode DecCb);
//------------------------------------------------------------------------------
/*!
\brief ADO version
\return ((ADO_MAJORVER << 8) + ADO_MINORVER)
*/
uint16_t uwADO_GetVersion(void);
//------------------------------------------------------------------------------
/*!
\brief ADO IP status
\return ADO_IP_READY_t
*/
ADO_IP_READY_t ADO_GetIpReadyStatus(void);
//------------------------------------------------------------------------------
/*!
\brief Audio Setup
\param ADO_KNL_PARA_t
\param pADO_EncodeQIdHandle
\param pADO_DecodeQIdHandle
\par Note:
	1. You can set up the audio parameter and it will register ADC/DAC ISR and disable ADC/DAC related interrupt.
\return(no)
*/
void ADO_Setup(ADO_KNL_PARA_t *AdoPara, osMessageQId* pADO_EncodeQIdHandle, osMessageQId* pADO_DecodeQIdHandle);
//------------------------------------------------------------------------------
/*!
\brief get audio total buffer size
\param ADO_KNL_PARA_t
\return total buffer size(unit:bytes)
*/
uint32_t ulADO_GetTotalBuffSize(ADO_KNL_PARA_t *AdoPara);
//------------------------------------------------------------------------------
/*!
\brief get audio AAC enable switch
\return AAC enable switch
*/
ADO_FUN_SWITCH tADO_GetAACEnable(void);
//------------------------------------------------------------------------------
/*!
\brief get audio Audio32 enable switch
\return Audio32 enable switch
*/
ADO_FUN_SWITCH tADO_GetAdo32Enable(void);
//------------------------------------------------------------------------------
/*!
\brief get audio software alaw enable switch
\return software alaw enable switch
*/
ADO_FUN_SWITCH tADO_GetSwAlawEnable(void);
//------------------------------------------------------------------------------
/*!
\brief Audio ADC start
\par Note:
	1. It will enable ADC related interrupt.
\return(no)
*/
void ADO_AdcStart(void);
//------------------------------------------------------------------------------
/*!
\brief Audio ADC stop
\par Note:
	1. It will disable ADC related interrupt.
\return(no)
*/
void ADO_AdcStop(void);
//------------------------------------------------------------------------------
/*!
\brief Audio DAC start
\par Note:
	1. It will enable DAC related interrupt.
\return(no)
*/
void ADO_DacStart(void);
//------------------------------------------------------------------------------
/*!
\brief Audio DAC stop
\par Note:
	1. It will disable DAC related interrupt.
\return(no)
*/
void ADO_DacStop(void);
//------------------------------------------------------------------------------
/*!
\brief AEC NR Setting
\param Type             Process Type Switch
\param SampleRate       Data SampleRate
\return(no)
\par [Example]
\code 
    ADO_Noise_Process_Type(NOISE_DISABLE ,AEC_NR_16kHZ);
\endcode
*/
void ADO_Noise_Process_Type(ADO_NOISE_PROCESS_TYPE Type ,AEC_NR_SAMPLERATE_MODE_t SampleRate);
//------------------------------------------------------------------------------
/*!
\brief Sigma-delta ADC gain setting
\param Boost        BOOST gain
\param Pga          PGA gain
\return(no)
*/
void ADO_SetSigmaDeltaAdcGain(ADO_SIG_DEL_ADC_BOOST_GAIN Boost, ADO_SIG_DEL_ADC_PGA_GAIN Pga);
//------------------------------------------------------------------------------

/*!
\brief ADC adjust function(DC compensation)
\param ubTH1			ADC DC compensation STEP Threshold 1.
\param ubTH2 			ADC DC compensation STEP Threshold 2.
\param ubTH3		 	ADC DC compensation STEP Threshold 3.
\param Switch		 	ADO_ON/ADO_OFF
\par Note:
	1. Threshold 3 > Threshold 2 > Threshold 1.
\return(no)
*/
void ADO_SetAdcDcComp(uint8_t ubTH1, uint8_t ubTH2, uint8_t ubTH3, ADO_FUN_SWITCH Switch);
//------------------------------------------------------------------------------
/*!
\brief ADC adjust function(Gain)
\param Gain                 Digital gain for ADC (Range: 0-20).
\param ubGainRmpRate        Amplitude +0.5dB every (GAINRMP_RATE*512) sample.
\param Switch               ADO_ON/ADO_OFF
\par Note:
	1. Gain:\n
					00000:0dB,   00111:3.5dB, 01110: 7dB,\n
					00001:0.5dB, 01000:4dB,   01111: 7.5dB,\n
					00010:1dB,   01001:4.5dB, 10000: 8dB,\n
					00011:1.5dB, 01010:5dB,   10001: 8.5dB,\n
					00100:2dB,   01011:5.5dB, 10010: 9dB,\n
					00101:2.5dB, 01100:6dB,   10011: 9.5dB,\n
					00110:3dB,   01101:6.5dB, 10100: 10dB.
\return(no)
*/
void ADO_SetAdcGain(ADO_ADCGAIN Gain, uint8_t ubGainRmpRate, ADO_FUN_SWITCH Switch);
//------------------------------------------------------------------------------
/*!
\brief ADC adjust function(Mute)
\param Mode         Mute speed.
\param Switch       ADO_ON/ADO_OFF
\par Note:
	1. Mute speed:\n
                    00:Amplitude -3dB every 4 sample\n
                    01:Amplitude -3dB every 8 sample\n
                    10:Amplitude -3dB every 16 sample\n
                    11:Amplitude -3dB every 32 sample.
\return(no)
*/
void ADO_SetAdcMute(ADO_ADCMUTESPEED Mode, ADO_FUN_SWITCH Switch);
//------------------------------------------------------------------------------
/*!
\brief ADC adjust function(Report)
\param uwLowLvPeak      ADC low level peak detection for 16-bit pcm.
\param uwHighLvPeak     ADC high level peak detection for 16-bit pcm.
\param Switch           ADO_ON/ADO_OFF
\par Note:
	1. Must enable ADC gain if you want to use ADC report.
\return(no)
*/
void ADO_SetAdcRpt(uint16_t uwLowLvPeak, uint16_t uwHighLvPeak, ADO_FUN_SWITCH Switch);
//------------------------------------------------------------------------------
/*!
\brief Get ADC report(Sum_L)
\return Adc_Sum_L(summation of 16-bit PCM for ADC when the value>Arch-band ADC_TH_L)
*/
uint32_t ulADO_GetAdcSumLow(void);
//------------------------------------------------------------------------------
/*!
\brief Get ADC report(Sum_H)
\return Adc_Sum_L(summation of 16-bit PCM for ADC when the value>Arch-band ADC_TH_H)
*/
uint32_t ulADO_GetAdcSumHigh(void);
//------------------------------------------------------------------------------
/*!
\brief DAC adjust function(Mute)
\param Mode         Attenuation ramp rate for DAC.
\param Switch       ADO_ON/ADO_OFF
\par Note:
	1. Ramp rate:\n
            00:0.5 dB per 1 sample.  10:0.5 dB per 4 samples.\n
            01:0.5 dB per 2 samples. 11:0.5 dB per 8 samples.
\return(no)
*/
void ADO_SetDacMute(AUDIO_DACMUTERMP Mode, ADO_FUN_SWITCH Switch);
//------------------------------------------------------------------------------
/*!
\brief Get DAC mute status
\return ADO_ON/ADO_OFF
*/
ADO_FUN_SWITCH ADO_GetDacMuteStatus(void);
//------------------------------------------------------------------------------
/*!
\brief DAC adjust function(Automute)
\param Switch       ADO_ON/ADO_OFF
\return(no)
*/
void ADO_SetDacAutoMute(ADO_FUN_SWITCH Switch);
//------------------------------------------------------------------------------
/*!
\brief DAC adjust function(R2R volume)
\param Volume       Volume control.
\par Note:
	1. Volume control:\n
                    15: 0dB,    7:-23.5dB\n
                    14:-3dB,    6:-26.2dB\n
                    13:-5.6dB,  5:-29.8dB\n
                    12:-8.2dB,  4:-32.4dB\n
                    11:-11.9dB, 3:-36dB\n
                    10:-14.6dB, 2:-39.1dB\n
                    9:-18.2dB, 1:-42dB\n
                    8:-21.4dB, 0:-45dB
\return(no)
*/
void ADO_SetDacR2RVol(ADO_R2R_VOL Volume);
//------------------------------------------------------------------------------
/*!
\brief Get DAC adjust function(R2R volume)
\return ADO_R2R_VOL
*/
ADO_R2R_VOL ADO_GetDacR2RVol(void);
//------------------------------------------------------------------------------
/*!
\brief DAC adjust function(Gain)
\param Gain         Digital gain for DAC (Range: 0-15).
\param Switch       ADO_ON/ADO_OFF
\par Note:
	1. Gain:\n
					0:0db,     8:-24db\n
					1:-2.5db,  9:-26.6db\n
					2:-6db,   10:-30.1db\n
					3:-8.5db, 11:-32.6db\n
					4:-12db,  12:-36.1db\n
					5:-14db,  13:-38.6db\n
					6:-18db,  14:42.1db\n
					7:-20db,  15:-44.6db
\return(no)
*/
void ADO_SetDacGain(ADO_DACGAIN Gain, ADO_FUN_SWITCH Switch);
//------------------------------------------------------------------------------
/*!
\brief DAC adjust function(Upsample)
\param Mode         Upsample mode.
\param Switch       ADO_ON/ADO_OFF
\par Note:
	1. Upsample mode:\n
                    00:2x output samples\n
                    01:3x output samples\n
                    10:4x output samples\n
                    11:6x output samples
\return(no)
*/
void ADO_SetDacUpsample(ADO_UPSAMPLING Mode, ADO_FUN_SWITCH Switch);
//------------------------------------------------------------------------------
/*!
\brief DAC adjust function(M2SO)
\param Switch       ADO_ON/ADO_OFF
\par Note:
	1. The left channel audio data will be copied to right channel in I2S DAC MONO mode\n
		 when this bit is enabled. (Mono to Stereo out)
\return(no)
*/
void ADO_SetDacM2so(ADO_FUN_SWITCH Switch);
//------------------------------------------------------------------------------
/*!
\brief DAC adjust function(Report)
\param Switch       ADO_ON/ADO_OFF
\return(no)
*/
void ADO_SetDacRpt(ADO_FUN_SWITCH Switch);
//------------------------------------------------------------------------------
/*!
\brief Get DAC left channel report
\return 16-bit report
*/
uint16_t uwADO_GetDacSumRptLeft(void);
//------------------------------------------------------------------------------
/*!
\brief Get DAC right channel report
\return 16-bit report
*/
uint16_t uwADO_GetDacSumRptRight(void);
//------------------------------------------------------------------------------
/*!
\brief Audio gpio initial
\param Mask     ADO_I2S_ADC_DATA\n
                ADO_I2S_ADC_BCLK\n
                ADO_I2S_DAC_DATA\n
                ADO_I2S_ADC_LRCLK\n
                ADO_I2S_ADC_MCLK
\param Type     ADO_GPIO_INPUT/ADO_GPIO_OUTPUT
\return(no)
*/
void ADO_GpioInit(ADO_GPIO_PIN Mask, ADO_GPIO_TYPE Type);
//------------------------------------------------------------------------------
/*!
\brief Audio gpio write
\param Mask     ADO_I2S_ADC_DATA\n
                ADO_I2S_ADC_BCLK\n
                ADO_I2S_DAC_DATA\n
                ADO_I2S_ADC_LRCLK\n
                ADO_I2S_ADC_MCLK
\param Value    ADO_GPIO_LOW/ADO_GPIO_HIGH
\return(no)
*/
void ADO_GpioWrt(ADO_GPIO_PIN Mask, ADO_GPIO_VALUE Value);
//------------------------------------------------------------------------------
/*!
\brief Audio gpio read
\param Mask     ADO_I2S_ADC_DATA\n
                ADO_I2S_ADC_BCLK\n
                ADO_I2S_DAC_DATA\n
                ADO_I2S_ADC_LRCLK\n
                ADO_I2S_ADC_MCLK
\return ADO_GPIO_VALUE      ADO_GPIO_LOW/ADO_GPIO_HIGH
*/
ADO_GPIO_VALUE ubADO_GpioRd(ADO_GPIO_PIN Mask);
//------------------------------------------------------------------------------
/*!
\brief Get audio sample rate
\return SampleRateValue
*/
uint16_t uwADO_GetSampleRate(void);
//------------------------------------------------------------------------------
/*!
\brief chk dec buf is full or empty to write data
\param PlyType		play type
\param EncType		encode type
\param ulSrcSize	source size
\return ADO_DEC_BUF_SPACE_CHK
*/
ADO_DEC_BUF_SPACE_CHK ADO_DecBufWrtInChk(ADO_PLY_TYPE PlyType, ADO_ENCODE_TYPE EncType, uint32_t ulSrcSize);
//------------------------------------------------------------------------------
/*!
\brief Write audio data to decode buffer for decoding and dac play.
\param Input		information of decoding for dac play
\return 0:fail, 1:success
*/
uint8_t ubADO_DecBufWrtIn(ADO_Queue_INFO *INFO);
//------------------------------------------------------------------------------
/*!
\brief Audio32 Enable Setting
\param Switch       ADO_ON/ADO_OFF
\return(no)
\par [Example]
\code 
			ADO_Set_Audio32_Enable(ADO_ON);
\endcode
*/
void ADO_Set_Audio32_Enable(ADO_FUN_SWITCH Switch);
//------------------------------------------------------------------------------
/*!
\brief Audio32 Encoder Initial
\param ubUseIdx		range:0~(ADO_AUDIO32_MAX_NUM-1); support (ADO_AUDIO32_MAX_NUM) encoder initial
\param Format   	Audio32 Format Enum 
\return(no)
\par [Example]
\code 
        ADO_Ado32EncInit(0,SNX_AUD32_FMT8_8KBPS);
\endcode
*/
void ADO_Ado32EncInit(uint8_t ubUseIdx, ADO_SNX_AUD32_FORMAT Format);
//------------------------------------------------------------------------------
/*!
\brief Audio32 Decoder Initial
\param ubUseIdx		range:0~(ADO_AUDIO32_MAX_NUM-1); support (ADO_AUDIO32_MAX_NUM) decoder initial
\param Format       Audio32 Format Enum 
\return(no)
\par [Example]
\code 
        ADO_Ado32DecInit(0,SNX_AUD32_FMT8_8KBPS);
\endcode
*/
void ADO_Ado32DecInit(uint8_t ubUseIdx, ADO_SNX_AUD32_FORMAT Format);
//------------------------------------------------------------------------------
/*!
\brief Get Audio32 encoder format
\param ubUseIdx		range:0~(ADO_AUDIO32_MAX_NUM-1); support to get (ADO_AUDIO32_MAX_NUM) encoder format
\return	ADO_SNX_AUD32_FORMAT
\par [Example]
\code 
		ADO_SNX_AUD32_FORMAT fmt
        fmt = ADO_GetAdo32EncFmt(0);
\endcode
*/
ADO_SNX_AUD32_FORMAT ADO_GetAdo32EncFmt(uint8_t ubUseIdx);
//------------------------------------------------------------------------------
/*!
\brief Get Audio32 decoder format
\param ubUseIdx		range:0~(ADO_AUDIO32_MAX_NUM-1); support to get (ADO_AUDIO32_MAX_NUM) encoder format
\return	ADO_SNX_AUD32_FORMAT
\par [Example]
\code 
		ADO_SNX_AUD32_FORMAT fmt
        fmt = ADO_GetAdo32DecFmt(0);
\endcode
*/
ADO_SNX_AUD32_FORMAT ADO_GetAdo32DecFmt(uint8_t ubUseIdx);
//------------------------------------------------------------------------------
/*!
\brief Audio32 Encode Function
\param ubUseIdx			range:0~(ADO_AUDIO32_MAX_NUM-1); support (ADO_AUDIO32_MAX_NUM) encoder
\param ulSrc            Address of Source
\param ulDes            Address of Destition
\param ulTotalSize      Total Size of Source Data
\return ADO_AUD32_ENC_INFO
\par [Example]
\code
		ADO_AUD32_ENC_INFO Aud32EncInfo;
        Aud32EncInfo = ADO_Ado32_Encode(0,SrcAddr,DesAddr,100);
\endcode
*/
ADO_AUD32_ENC_INFO ADO_Ado32_Encode(uint8_t ubUseIdx, uint32_t ulSrcAddr, uint32_t ulDestAddr, uint32_t ulSize);
//------------------------------------------------------------------------------
/*!
\brief Audio32 Decode Function
\param ubUseIdx			range:0~(ADO_AUDIO32_MAX_NUM-1); support (ADO_AUDIO32_MAX_NUM) decoder
\param ulSrc            Address of Source
\param ulDes            Address of Destition
\param ulTotalSize      Total Size of Source Data
\return ADO_AUD32_DEC_INFO
\par [Example]
\code
		ADO_AUD32_DEC_INFO Aud32DecInfo;
        Aud32DecInfo = ADO_Ado32_Decode(0,SrcAddr,DesAddr,100);
\endcode
*/
ADO_AUD32_DEC_INFO ADO_Ado32_Decode(uint8_t ubUseIdx, uint32_t ulSrcAddr, uint32_t ulDestAddr, uint32_t ulSize);
//------------------------------------------------------------------------
/*!
\brief Software alaw Enable Setting
\param Switch       ADO_ON/ADO_OFF
\return(no)
\par [Example]
\code 
			ADO_Set_SwAlaw_Enable(ADO_ON);
\endcode
*/
void ADO_Set_SwAlaw_Enable(ADO_FUN_SWITCH Switch);
//------------------------------------------------------------------------------
/*!
\brief Alaw encode Function
\param ulSrc            Address of Source
\param ulDes            Address of Destition
\param ulInputSize      input size
\param ulOutputSize     pointer of output size
\return ALAW_RETURN_FLAG_t
\par [Example]
\code
		ALAW_RETURN_FLAG_t AlawEncFlag;
        AlawEncFlag = ADO_Alaw_Encode(SrcAddr,DesAddr,100);
\endcode
*/
ALAW_RETURN_FLAG_t ADO_Alaw_Encode(uint32_t ulSrcAddr, uint32_t ulDstAddr, uint32_t ulInputSize, uint32_t *ulOutputSize);
//------------------------------------------------------------------------------
/*!
\brief Alaw decode Function
\param ulSrc            Address of Source
\param ulDes            Address of Destition
\param ulInputSize      input size
\param ulOutputSize     pointer of output size
\return ALAW_RETURN_FLAG_t
\par [Example]
\code
		ALAW_RETURN_FLAG_t AlawDecFlag;
        AlawDecFlag = ADO_Alaw_Decode(SrcAddr,DesAddr,100);
\endcode
*/
ALAW_RETURN_FLAG_t ADO_Alaw_Decode(uint32_t ulSrcAddr, uint32_t ulDstAddr, uint32_t ulInputSize, uint32_t *ulOutputSize);
//------------------------------------------------------------------------------
/*!
\brief AAC Enable Setting
\param Switch 					ADO_ON/ADO_OFF
\return(no)
\par [Example]
\code 
			ADO_Set_ACC_Enable(ADO_ON);
\endcode
*/
void ADO_Set_AAC_Enable(ADO_FUN_SWITCH Switch);
//------------------------------------------------------------------------------
/*!
\brief aac encode initial(ADTS format)
\param ubUseIdx			support 5 groups(0~4) AAC encode function
\param ubEncType		0->raw format; 1->ADTS format
\param ulSampleRate		sample rate, ex:16000
\param ulChannels		1->mono; 2->stereo
\return	ADO_RETURN_FLAG
*/
ADO_RETURN_FLAG ADO_AAC_EncInit(uint8_t ubUseIdx, uint8_t ubEncType, uint32_t ulSampleRate, uint32_t ulChannels);
//------------------------------------------------------------------------------
/*!
\brief Get aac encode unit size
\param ubUseIdx			support 0~4 AAC encode function
\return encode unit size
*/
uint32_t ulADO_AAC_GetEncUnitSize(uint8_t ubUseIdx);
//------------------------------------------------------------------------------
/*!
\brief aac encode function
\param ubUseIdx			support 5 groups(0~4) AAC encode function
\param SrcAddr			source address
\param SrcSize			source size
\param DestAddr			destination address
\param DestSize			destination size
\return	ADO_RETURN_FLAG
*/
ADO_RETURN_FLAG ADO_AAC_EncEncode(uint8_t ubUseIdx, uint32_t SrcAddr, uint32_t SrcSize, uint32_t DestAddr, uint32_t *DestSize);
//------------------------------------------------------------------------------
/*!
\brief aac encode flush(encode the remaind data in the aac encode lib)
\param ubUseIdx			support 5 groups(0~4) AAC encode function
\param DestAddr			destination address
\param DestSize			destination size
\return 1->success; 0->fail
*/
ADO_RETURN_FLAG ADO_AAC_EncEncodeFlush(uint8_t ubUseIdx, uint32_t DestAddr, uint32_t *DestSize);
//------------------------------------------------------------------------------
/*!
\brief aac encode un-initial
\param ubUseIdx			support 5 groups(0~4) AAC encode function
\return 1->success; 0->fail
*/
void ADO_AAC_EncUnInit(uint8_t ubUseIdx);
//------------------------------------------------------------------------------
/*!
\brief aac decode initial(ADTS format)
\param ubUseIdx			support 5 groups(0~4) AAC decode function
\param ubEncType		0->raw format; 1->ADTS format
\param ulSampleRate		sample rate, ex:16000
\param ulChannels		1->mono; 2->stereo
\return 1->success; 0->fail
*/
ADO_RETURN_FLAG ADO_AAC_DecInit(uint8_t ubUseIdx, uint8_t ubDecType, uint32_t ulSampleRate, uint32_t ulChannels);
//------------------------------------------------------------------------------
/*!
\brief aac decode function
\param ubUseIdx			support 5 groups(0~4) AAC decode function
\param SrcAddr			source address
\param SrcSize			source size
\param DestAddr			destination address
\param DestSize			destination size
\return 1->success; 0->fail
*/
ADO_RETURN_FLAG ADO_AAC_DecDecode(uint8_t ubUseIdx, uint32_t SrcAddr, uint32_t SrcSize, uint32_t DestAddr, uint32_t *DestSize);
//------------------------------------------------------------------------------
/*!
\brief aac decode un-initial
\param ubUseIdx			support 5 groups(0~4) AAC decode function
\return 1->success; 0->fail
*/
void ADO_AAC_DecUnInit(uint8_t ubUseIdx);
//------------------------------------------------------------------------------
/*!
\brief 	Get Audio Version	
\return	Version
*/
uint16_t uwADO_GetVersion(void);
//------------------------------------------------------------------------------
/*!
\brief DeHowling Enable
\param Switch   On/OFF
\return(no)
\par [Example]
\code 
        ADO_Set_DeHowling_Enable(ADO_ON);
\endcode
*/
void ADO_Set_DeHowling_Enable(ADO_FUN_SWITCH Switch);
//------------------------------------------------------------------------------
/*!
\brief Play Wav Audio
\param ubIndex Song Index
\return(no)
\par [Example]
\code 
        ADO_WavPlay(0);
\endcode
*/
void ADO_WavPlay(uint8_t ubIndex);
//------------------------------------------------------------------------------
/*!
\brief Repeat Wav Audio
\param ubIndex Song Index
\return(no)
\par [Example]
\code 
        ADO_WavRepeat(0);
\endcode
*/
void ADO_WavRepeat(uint8_t ubIndex);
//------------------------------------------------------------------------------
/*!
\brief Stop Wav Audio
\return(no)
\par [Example]
\code 
        ADO_WavStop();
\endcode
*/
void ADO_WavStop(void);
//------------------------------------------------------------------------------
/*!
\brief Resume Wav Audio. RePlay at last ADO_WavStop time 
\return(no)
\par [Example]
\code 
        ADO_WavResume();
\endcode
*/
void ADO_WavResume(void);
//------------------------------------------------------------------------------
/*!
\brief Get Wav Play state 
\return 0: wav idle
		1: wav playing
\code 
        tADO_GetWavState();
\endcode
*/
ADO_WAV_STATE tADO_GetWavState(void);
//------------------------------------------------------------------------------
/*!
\brief decide the max time latency received from source. NOTE: Must call it after ADO_Setup!
\param ubSrcNum				source number, value=0,1,...,(ADO_SRC_NUM-1)
\param ulTargetLatency		latency timing, unit:ms
\return(no)
\par [Example]
\code
		ADO_Setup();
        ADO_SetRcvTimeLatency(0, 250);	// control time latency of receiving ado source=0 by 250ms
\endcode
*/
void ADO_SetRcvTimeLatency(uint8_t ubSrcNum, uint32_t ulTargetLatency);
//------------------------------------------------------------------------------
/*!
\brief reset time latency received from source
\param ubSrcNum		source number, value=0,1,...,(ADO_SRC_NUM-1)
\return(no)
\par [Example]
\code
        ADO_RstRcvTimeLatency(0);
\endcode
*/
void ADO_RstRcvTimeLatency(uint8_t ubSrcNum);
//------------------------------------------------------------------------------
/*!
\brief queue some data in dac for smooth play(lauch only for normal play mode). NOTE: Must call it after ADO_Setup!
\param ulQueue_ms		queue the time of ado data, unit:ms, max value depends on PreHwPly_buf_size
\return(no)
\par [Example]
\code
		ADO_Setup();
		ADO_SetQueueDataForSmoothPly(200);
\endcode
*/
void ADO_SetQueueDataForSmoothPly(uint32_t ulQueueTime);
//------------------------------------------------------------------------------
/*!
\brief test self sigma-delta adc and r2r dac loopback function; test lengh: define with ulSelfTestAdoTime
\return(no)
\par [Example]
\code 
		//control flow
        ADO_SelfTest_Init();
		ADO_SelfTest_Record();
		ADO_SelfTest_Play();
		ADO_SelfTest_Close();
\endcode
*/
void ADO_SelfTest_Init(void);
void ADO_SelfTest_Record(void);
void ADO_SelfTest_Play(void);
void ADO_SelfTest_Close(void);
//------------------------------------------------------------------------------
/*!
\brief ADC software gain compensation
\param ulGainValue		gain value: 0/1/2/3/4...
\par [Example]
\code 
        ADO_AdcSwGainCompensation(4);
\endcode
*/
void ADO_AdcSwGainCompensation(float fGainValue);
//------------------------------------------------------------------------------
/*!
\brief DAC software gain compensation
\param ulGainValue		gain value: 0/1/2/3/4...
\par [Example]
\code 
        ADO_AdcSwGainCompensation(4);
\endcode
*/
void ADO_DacSwGainCompensation(float fGainValue);
//------------------------------------------------------------------------------
/*!
\brief audio remote record path switch(from transmitter)
\param Copy		ADO_ON:open the path;	ADO_OFF:close the path
\par [Example]
\code 
        ADO_RemoteRecordPathSwitch(ADO_OFF);	//stop send alaw or aac stream
        ADO_RemoteRecordPathSwitch(ADO_ON);		//start send alaw or aac stream
\endcode
*/
void ADO_RemoteRecordPathSwitch(ADO_FUN_SWITCH Status);
//------------------------------------------------------------------------------
/*!
\brief get audio remote record path status(from transmitter)
\return RecBuf_Th
*/
ADO_FUN_SWITCH ADO_GetRemoteRecordPathStatus(void);
//------------------------------------------------------------------------------
typedef void(*pvADO_RemoteSenQCbFunc)(uint8_t, uint32_t, uint32_t, uint32_t);
/*!
\brief audio remote record path handle(process aud32 decode and alaw encode)(for Rx recording)
\param ubStreamType		0:aud32; 1:aac
\param ubSrcNum			source number
\param ulSrcAddr		source address
\param ulSize			source size
\return 0:fail, 1:success
*/
uint8_t ubADO_RemoteRecordingPathHandle(uint8_t ubStreamType, uint8_t ubSrcNum, uint32_t ulSrcAddr, uint32_t ulSize, pvADO_RemoteSenQCbFunc pRemoteSenQ_Cb);
//------------------------------------------------------------------------------
typedef void(*pvADO_LocalSendQCbFunc)(uint32_t, uint32_t, uint32_t);
/*!
\brief audio local record path switch(for local recording)
\param Copy				ADO_ON:open the path;	ADO_OFF:close the path
\param EncType			encode type: only support alaw/aac
\param pLocalSenQ_Cb	Tx record callback function=>timestamp, address, size
\par [Example]
\code
        ADO_LocalRecordPathSwitch(ADO_OFF, NONE, NULL);			//stop send alaw/aac stream
        ADO_LocalRecordPathSwitch(ADO_ON, AAC, &pLocalSenQ_Cb);	//start send alaw/aac stream
\endcode
*/
void ADO_LocalRecordPathSwitch(ADO_FUN_SWITCH Status, ADO_ENCODE_TYPE EncType, pvADO_LocalSendQCbFunc pLocalSenQ_Cb);
//------------------------------------------------------------------------------
/*!
\brief get audio local record path status(for local recording)
\return RecBuf_Th
*/
ADO_FUN_SWITCH ADO_GetLocalRecordPathStatus(void);
//------------------------------------------------------------------------------
/*!
\brief Set DeHowling LV
\param ADO_DeHowling_LV		DeHowling Level
\par [Example]
\code 
        ADO_SetDeHowlingLV(DeHowlingLV0);
\endcode
*/
void ADO_SetDeHowlingLV(ADO_DeHowling_LV LV);
//------------------------------------------------------------------------------
typedef void(*pvADO_OutputCbFunc)(uint32_t, uint32_t, ADO_SAMPLERATE, ADO_CHANNEL_MODE);
void ADO_SetOutputCbFunc(pvADO_OutputCbFunc pOutput_Cb);
//------------------------------------------------------------------------------
#if APP_ADO_AEC_NR_TYPE == AEC_NR_HW
/*!
\brief initial i2c-1 for control 7300
\param (none)
\par [Example]
\code 
        ADO_HwAecNr_I2C1_Init();
\endcode
*/
void ADO_HwAecNr_I2C1_Init(void);
//------------------------------------------------------------------------------
/*!
\brief send aec/nr setting command to 7300
\param ubCommand
	0x80->aec on  + nr off
	0x08->aec off + nr on
	0x88->aec on  + nr on
	0x00->aec off + nr off
\par [Example]
\code 
        ADO_HwAecNr_Command(0x80);
\endcode
*/
void ADO_HwAecNr_Command(uint8_t ubCommand);
#endif
//------------------------------------------------------------------------------
#ifdef VBM_PU
/*!
\brief Set Audio Mix Mode Open (use in Rx only)         
\return(no)
\par [Example]
\code
        ADO_MixModeON(); 
\endcode
*/
void ADO_MixModeON(void);
#endif
//------------------------------------------------------------------------------
/*!
\brief Set Audio Mix Mode Close           
\return(no)
\par [Example]
\code
        ADO_MixModeOFF(); 
\endcode
*/
//------------------------------------------------------------------------------
void ADO_MixModeOFF(void);
/*!
\brief Get Mix Mode Status           
\return(no)
\par [Example]
\code
		ADO_FUN_SWITCH MixModeEN;
        MixModeEN = ADO_MixModeON(); 
\endcode
*/
ADO_FUN_SWITCH tADO_GetMixMode(void);
//------------------------------------------------------------------------------
/*!
\brief Set Mix Mode each Ratio in 2 ,3 or 4 channel case            
\return(no)
\par uint8_t Ratio_2CH  ratio of 2 channel		pcm amplitude  = original pcm *Ratio_2CH/10
\par uint8_t Ratio_3CH  ratio of 3 channel		pcm amplitude  = original pcm *Ratio_3CH/10
\par uint8_t Ratio_4CH  ratio of 4 channel		pcm amplitude  = original pcm *Ratio_4CH/10
\code		
\endcode
*/
void ADO_SetMixModeRatio(uint8_t Ratio_2CH, uint8_t Ratio_3CH, uint8_t Ratio_4CH);
//------------------------------------------------------------------------------
/*!
\brief Get 1ms timer counter
\return 1ms timer counter
*/
uint32_t ulADO_Get1ms(void);
//------------------------------------------------------------------------------
/*!
\brief enable recording mixer function with local and remote
\par ubLocalRatio 	local mix ratio, range:0~10, represents 0~100%, if value setting is fault, it will set to default ration=5
\par ubRemoteRatio 	remote mix ratio, range:0~10, represents 0~100%, if value setting is fault, it will set to default ration=5
\par Status 		ADO_ON/ADO_OFF
\code
		ADO_LocalRecMixRemoteSwitch(7, 7, ADO_ON);
\return(no)
*/
void ADO_LocalRecMixRemoteSwitch(uint8_t ubLocalRatio, uint8_t ubRemoteRatio, ADO_FUN_SWITCH Status);
//------------------------------------------------------------------------------
extern const uint32_t ulADO_BufTh[];
//------------------------------------------------------------------------------
#endif

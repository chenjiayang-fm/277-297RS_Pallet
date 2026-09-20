#ifndef _IR_H_
#define _IR_H_

#define REMO_MUTE		0x16
#define REMO_POWER		0x12
#define REMO_OK		    0x13
#define REMO_SEL		0x17
#define REMO_LEFT		0x14
#define REMO_RIGHT		0x10
#define REMO_MENU		0x0E
#define REMO_PAIRING    0x19
#define REMO_SYS		0x1B
#define REMO_MIRROR		0x0F
#define REMO_VERSION	0x1C
/*以下没有用到*/
#define REMO_LANG       0x1E
#define REMO_RESET      0x0D
#define REMO_AV			0x0B


void IR_init(void);
extern uint8_t SendIrCodeFlag;



#endif

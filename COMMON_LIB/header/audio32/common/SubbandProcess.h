//#define SUBBAND_NUM 4
#define SUBBAND_NUM 8
#define FILTER_TAPS 24//48
#define INPUT_SCALE 1
#define DELAY_BUF_SIZE 50//FILTER_TAPS+2
#define FILTER_SCALE 15
#define FILTER_CARRY 0
#define CIRCULAR_BUFFER



void QMFInit(void);

void QMF2SubbandAna(long int *inputX,short lenX,long int *outY,short s,short ch,short order);
void QMF2SubbandSyn(long int *subbandX,short lenX,long int *outY,short s,short ch,short order);
void QMF4SubbandAna(short *insamples,short lenX,long int *outputY,short ch,short order);
void QMF4SubbandSyn(long int *subbandX,short lenX,short *outY,short ch,short order);

/*!


	\file			RTC676x_SPI_src.c
	\brief		SPI (Serial Peripheral Interface) function for Richwave RTC676x serial RF chip
	\author		Bruce Cheng
	\version	0.9
	\date		2021/12/01
	\copyright	Copyright (C) 2021 Richwave Co., Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "APBC.h"
#include "INTC.h"
#include "BSP.h"
#include "GPIO.h"
//------------------------------------------------------------------------------

typedef enum
{
    SPI_SLAVE = 1,
    SPI_MASTER = 3
} SPI_MODE_t;

typedef enum
{
    SPI_WaitReady,
    SPI_DontWait
} SPI_WaitMode_t;

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

//Don't modify these enum and struct
//------------------------------------------------------------------------------
typedef enum
{
	RTC676x_RF_Slave = 0,
	RTC676x_RF_Master
}RTC676x_RF_Device;

//------------------------------------------------------------------------------

void RTC676x_RF_ISR(void);
void RTC676x_SPI_Init_Step2(RTC676x_RF_Device tRFDevice);


SPI_SlaveHook pfRTC676x_SPI_SlaveHook;

#if defined(BUC_CU)   /*LCD : RX*/
RTC676x_RF_Device tRFDevice = RTC676x_RF_Master;
#elif defined(BUC_CAM) /*Camera : TX*/

RTC676x_RF_Device tRFDevice = RTC676x_RF_Slave;
#endif

//RX Start
#if defined(BUC_CU)   /*LCD : RX*/
	//BUC_CU Board(2)_Start
	#if (defined(BSP_SN93711_FHD_REC_RX_V4)|| defined(BSP_D_SN93701_SSD2828_RX_V5) || defined(BSP_D_SN93701_TC358778_RX_V6))
		#define	RTC676x_SPI_CLK_PADIO		GLB->PADIO18
		#define	RTC676x_SPI_CS_PADIO		GLB->PADIO19
		#define	RTC676x_SPI_TX_PADIO		GLB->PADIO20
		#define	RTC676x_SPI_RX_PADIO		GLB->PADIO21
		#define	RTC676x_IRQ_PADIO			GLB->PADIO15
		#define	RTC676x_RESET_PADIO			GLB->PADIO6
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
		#define RTC676x_INTR_SEL			GPIO_INTR_SEL_1
		#define	RTC676x_RESET_OUT_EN		GPIO->GPIO_OE6
		#define	RTC676x_RESET_OUT			GPIO->GPIO_O6
		#define	RTC676x_SPI_CS_OUT_EN		GPIO->GPIO_OE5
		#define	RTC676x_SPI_CS_OUT			GPIO->GPIO_O5
		#define	RTC676x_PADIO_CLK_SPI_MODE	2
		#define	RTC676x_PADIO_CS_SPI_MODE	0
		#define	RTC676x_PADIO_TX_SPI_MODE	2
		#define	RTC676x_PADIO_RX_SPI_MODE	2
		#define RTC676x_SPI_CLK_PIN			18
		#define RTC676x_SPI_CS_PIN			19
		#define RTC676x_SPI_TX_PIN			20
		#define RTC676x_SPI_RX_PIN			21
	#endif	
	//BUC_CU Board(2)_End

	//BUC_CU Board(1/4)_Start
	#if (	defined(BSP_D_SNCC71_GM8285C_RX_V2)							||\
			(defined(BSP_D_SNCC71_TP2915_AHD_RX_V3)&&defined(RTC676x))	||\
			(defined(BSP_D_SNCC71_NVP6021_AHD_RX_V4)&&defined(RTC676x))		)
	
		#define	RTC676x_SPI_CLK_PADIO		GLB->PADIO18
		#define	RTC676x_SPI_CS_PADIO		GLB->PADIO19
		#define	RTC676x_SPI_TX_PADIO		GLB->PADIO20
		#define	RTC676x_SPI_RX_PADIO		GLB->PADIO21

		#define	RTC676x_IRQ_PADIO			GLB->PADIO6

		#define	RTC676x_RESET_PADIO			GLB->PADIO17

		#define	RTC676x_IRQ_OUT_EN			GPIO->GPIO_OE6
		#define	RTC676x_IRQ_PULL_HIGH		GPIO->GPIO_PULL_HIGH6
		#define	RTC676x_IRQ_PULL_EN			GPIO->GPIO_PULL_EN6
		#define	RTC676x_IRQ_TRG_MODE		GPIO->GPIO_TRG_MODE6
		#define	RTC676x_IRQ_TRG_EDGE		GPIO->GPIO_TRG_EDGE6
		#define	RTC676x_IRQ_TRG_LEVEL		GPIO->GPIO_TRG_LEVEL6
		#define	RTC676x_IRQ_CLR_INTR		GPIO->CLR_GPIO_INTR6
		#define	RTC676x_IRQ_INTR_MASK		GPIO->GPIO_INTR_MSK6
		#define	RTC676x_IRQ_INTR_EN			GPIO->GPIO_INTR_EN6
		#define	RTC676x_IRQ_INPUT			GPIO->GPIO_I6
		#define	RTC676x_IRQ_INTR_FLAG		GPIO->GPIO_INTR_FLAG6
		#define	RTC676x_IRQ_OUT				GPIO->GPIO_O6
		#define RTC676x_INTR_SEL			GPIO_INTR_SEL_6

		#define	RTC676x_RESET_OUT_EN		GPIO->GPIO_OE3
		#define	RTC676x_RESET_OUT			GPIO->GPIO_O3
		#define RTC676x_SPI_CS_OUT_EN(x)	{	PWM->PWM3_RATE 		= 127;	\
												PWM->PWM3_PERIOD	= 1;	\
												PWM->PWM_EN3		= x;}

		#define RTC676x_SPI_CS_OUT			(PWM->PWM3_HIGH_CNT)


		#define	RTC676x_PADIO_CLK_SPI_MODE	2
		#define	RTC676x_PADIO_CS_SPI_MODE	7
		#define	RTC676x_PADIO_TX_SPI_MODE	2
		#define	RTC676x_PADIO_RX_SPI_MODE	2

		#define RTC676x_SPI_CLK_PIN			18
		#define RTC676x_SPI_CS_PIN			19
		#define RTC676x_SPI_TX_PIN			20
		#define RTC676x_SPI_RX_PIN			21
	#endif
	//BUC_CU Board(1/4)_End
		#if (defined(RTC676x) && defined(BSP_D_SN93714_TX_V1))		
		#define	RTC676x_SPI_CLK_PADIO		GLB->PADIO18
		#define	RTC676x_SPI_CS_PADIO		GLB->PADIO19
		#define	RTC676x_SPI_TX_PADIO		GLB->PADIO20
		#define	RTC676x_SPI_RX_PADIO		GLB->PADIO21
		#define	RTC676x_IRQ_PADIO			GLB->PADIO55			
		#define	RTC676x_RESET_PADIO			GLB->PADIO9

		#define	RTC676x_IRQ_OUT_EN			GPIO->GPIO_OE13
		#define	RTC676x_IRQ_PULL_HIGH		GPIO->GPIO_PULL_HIGH13
		#define	RTC676x_IRQ_PULL_EN			GPIO->GPIO_PULL_EN13
		#define	RTC676x_IRQ_TRG_MODE		GPIO->GPIO_TRG_MODE13
		#define	RTC676x_IRQ_TRG_EDGE		GPIO->GPIO_TRG_EDGE13
		#define	RTC676x_IRQ_TRG_LEVEL		GPIO->GPIO_TRG_LEVEL13
		#define	RTC676x_IRQ_CLR_INTR		GPIO->CLR_GPIO_INTR13
		#define	RTC676x_IRQ_INTR_MASK		GPIO->GPIO_INTR_MSK13
		#define	RTC676x_IRQ_INTR_EN			GPIO->GPIO_INTR_EN13
		#define	RTC676x_IRQ_INPUT			GPIO->GPIO_I13
		#define	RTC676x_IRQ_INTR_FLAG		GPIO->GPIO_INTR_FLAG13
		#define	RTC676x_IRQ_OUT				GPIO->GPIO_O13
		#define RTC676x_INTR_SEL			GPIO_INTR_SEL_13
		
		#define	RTC676x_RESET_OUT_EN		GPIO->GPIO_OE9
		#define	RTC676x_RESET_OUT			GPIO->GPIO_O9		
		#define	RTC676x_SPI_CS_OUT_EN		GPIO->GPIO_OE5	//Others
		#define	RTC676x_SPI_CS_OUT			GPIO->GPIO_O5

		#define	RTC676x_PADIO_CLK_SPI_MODE	2
		#define	RTC676x_PADIO_CS_SPI_MODE	0
		#define	RTC676x_PADIO_TX_SPI_MODE	2
		#define	RTC676x_PADIO_RX_SPI_MODE	2

		#define RTC676x_SPI_CLK_PIN			18
		#define RTC676x_SPI_CS_PIN			19
		#define RTC676x_SPI_TX_PIN			20
		#define RTC676x_SPI_RX_PIN			21		
	#endif
//RX End

//TX Start
#elif defined(BUC_CAM) /*Camera : TX*/
	//Board Start
	#ifdef BSP_SN93710_FHD_REC_TX_V4		
		#define	RTC676x_SPI_CLK_PADIO		GLB->PADIO18
		#define	RTC676x_SPI_CS_PADIO		GLB->PADIO19
		#define	RTC676x_SPI_TX_PADIO		GLB->PADIO20
		#define	RTC676x_SPI_RX_PADIO		GLB->PADIO21

		#define	RTC676x_IRQ_PADIO			GLB->PADIO27

		#define	RTC676x_RESET_PADIO			GLB->PADIO28

		#define	RTC676x_IRQ_OUT_EN			GPIO->GPIO_OE13
		#define	RTC676x_IRQ_PULL_HIGH		GPIO->GPIO_PULL_HIGH13
		#define	RTC676x_IRQ_PULL_EN			GPIO->GPIO_PULL_EN13
		#define	RTC676x_IRQ_TRG_MODE		GPIO->GPIO_TRG_MODE13
		#define	RTC676x_IRQ_TRG_EDGE		GPIO->GPIO_TRG_EDGE13
		#define	RTC676x_IRQ_TRG_LEVEL		GPIO->GPIO_TRG_LEVEL13
		#define	RTC676x_IRQ_CLR_INTR		GPIO->CLR_GPIO_INTR13
		#define	RTC676x_IRQ_INTR_MASK		GPIO->GPIO_INTR_MSK13
		#define	RTC676x_IRQ_INTR_EN			GPIO->GPIO_INTR_EN13
		#define	RTC676x_IRQ_INPUT			GPIO->GPIO_I13
		#define	RTC676x_IRQ_INTR_FLAG		GPIO->GPIO_INTR_FLAG13
		#define	RTC676x_IRQ_OUT				GPIO->GPIO_O13
		#define RTC676x_INTR_SEL			GPIO_INTR_SEL_13

		#define	RTC676x_RESET_OUT_EN		GPIO->GPIO_OE0
		#define	RTC676x_RESET_OUT			GPIO->GPIO_O0
		#define	RTC676x_SPI_CS_OUT_EN		GPIO->GPIO_OE5	//Others
		#define	RTC676x_SPI_CS_OUT			GPIO->GPIO_O5

		#define	RTC676x_PADIO_CLK_SPI_MODE	2
		#define	RTC676x_PADIO_CS_SPI_MODE	0
		#define	RTC676x_PADIO_TX_SPI_MODE	2
		#define	RTC676x_PADIO_RX_SPI_MODE	2

		#define RTC676x_SPI_CLK_PIN			18
		#define RTC676x_SPI_CS_PIN			19
		#define RTC676x_SPI_TX_PIN			20
		#define RTC676x_SPI_RX_PIN			21		
	#endif
	//Board End

	//Board Start
	#if (defined(RTC676x) && (defined(BSP_D_SNCC70_TX_V2) || defined(BSP_SNCC70_AHD_TP9950_TX_V1)|| defined(BSP_SNCC70_AHD_RN6752M_TX_V1)))		
		#define	RTC676x_SPI_CLK_PADIO		GLB->PADIO18
		#define	RTC676x_SPI_CS_PADIO		GLB->PADIO19
		#define	RTC676x_SPI_TX_PADIO		GLB->PADIO20
		#define	RTC676x_SPI_RX_PADIO		GLB->PADIO21
		#define	RTC676x_IRQ_PADIO			GLB->PADIO27			
		#define	RTC676x_RESET_PADIO			GLB->PADIO9

		#define	RTC676x_IRQ_OUT_EN			GPIO->GPIO_OE13
		#define	RTC676x_IRQ_PULL_HIGH		GPIO->GPIO_PULL_HIGH13
		#define	RTC676x_IRQ_PULL_EN			GPIO->GPIO_PULL_EN13
		#define	RTC676x_IRQ_TRG_MODE		GPIO->GPIO_TRG_MODE13
		#define	RTC676x_IRQ_TRG_EDGE		GPIO->GPIO_TRG_EDGE13
		#define	RTC676x_IRQ_TRG_LEVEL		GPIO->GPIO_TRG_LEVEL13
		#define	RTC676x_IRQ_CLR_INTR		GPIO->CLR_GPIO_INTR13
		#define	RTC676x_IRQ_INTR_MASK		GPIO->GPIO_INTR_MSK13
		#define	RTC676x_IRQ_INTR_EN			GPIO->GPIO_INTR_EN13
		#define	RTC676x_IRQ_INPUT			GPIO->GPIO_I13
		#define	RTC676x_IRQ_INTR_FLAG		GPIO->GPIO_INTR_FLAG13
		#define	RTC676x_IRQ_OUT				GPIO->GPIO_O13
		#define RTC676x_INTR_SEL			GPIO_INTR_SEL_13
		
		#define	RTC676x_RESET_OUT_EN		GPIO->GPIO_OE9
		#define	RTC676x_RESET_OUT			GPIO->GPIO_O9		
		#define	RTC676x_SPI_CS_OUT_EN		GPIO->GPIO_OE5	//Others
		#define	RTC676x_SPI_CS_OUT			GPIO->GPIO_O5

		#define	RTC676x_PADIO_CLK_SPI_MODE	2
		#define	RTC676x_PADIO_CS_SPI_MODE	0
		#define	RTC676x_PADIO_TX_SPI_MODE	2
		#define	RTC676x_PADIO_RX_SPI_MODE	2

		#define RTC676x_SPI_CLK_PIN			18
		#define RTC676x_SPI_CS_PIN			19
		#define RTC676x_SPI_TX_PIN			20
		#define RTC676x_SPI_RX_PIN			21		
	#endif
	//Board End

	//Board Start
    #if ((defined(BSP_SNCC72_AHD_RN6752M_TX_V1)||defined(BSP_SNCC72_AHD_TP9950_TX_V2)) && defined(RTC676x))
		#define	RTC676x_SPI_CLK_PADIO		GLB->PADIO6
		#define	RTC676x_SPI_CS_PADIO		GLB->PADIO7		//GPIO 7
		#define	RTC676x_SPI_TX_PADIO		GLB->PADIO4
		#define	RTC676x_SPI_RX_PADIO		GLB->PADIO5

		#define	RTC676x_IRQ_PADIO			GLB->PADIO41	//GPIO 13
		#define	RTC676x_RESET_PADIO			GLB->PADIO42	//GPIO 0

		#define	RTC676x_IRQ_OUT_EN			GPIO->GPIO_OE13
		#define	RTC676x_IRQ_PULL_HIGH		GPIO->GPIO_PULL_HIGH13
		#define	RTC676x_IRQ_PULL_EN			GPIO->GPIO_PULL_EN13
		#define	RTC676x_IRQ_TRG_MODE		GPIO->GPIO_TRG_MODE13
		#define	RTC676x_IRQ_TRG_EDGE		GPIO->GPIO_TRG_EDGE13
		#define	RTC676x_IRQ_TRG_LEVEL		GPIO->GPIO_TRG_LEVEL13
		#define	RTC676x_IRQ_CLR_INTR		GPIO->CLR_GPIO_INTR13
		#define	RTC676x_IRQ_INTR_MASK		GPIO->GPIO_INTR_MSK13
		#define	RTC676x_IRQ_INTR_EN			GPIO->GPIO_INTR_EN13
		#define	RTC676x_IRQ_INPUT			GPIO->GPIO_I13
		#define	RTC676x_IRQ_INTR_FLAG		GPIO->GPIO_INTR_FLAG13
		#define	RTC676x_IRQ_OUT				GPIO->GPIO_O13
		#define RTC676x_INTR_SEL			GPIO_INTR_SEL_13

		#define	RTC676x_RESET_OUT_EN		GPIO->GPIO_OE0
		#define	RTC676x_RESET_OUT			GPIO->GPIO_O0
		
		#define	RTC676x_SPI_CS_OUT_EN		GPIO->GPIO_OE7
		#define	RTC676x_SPI_CS_OUT			GPIO->GPIO_O7

		#define	RTC676x_PADIO_CLK_SPI_MODE	6
		#define	RTC676x_PADIO_CS_SPI_MODE	0
		#define	RTC676x_PADIO_TX_SPI_MODE	6
		#define	RTC676x_PADIO_RX_SPI_MODE	6

		#define RTC676x_SPI_CLK_PIN			6
		#define RTC676x_SPI_CS_PIN			7
		#define RTC676x_SPI_TX_PIN			4
		#define RTC676x_SPI_RX_PIN			5
	#endif
	#if (defined(RTC676x) && defined(BSP_D_SNCC72_TX_V1))
		#define	RTC676x_SPI_CLK_PADIO		GLB->PADIO6
		#define	RTC676x_SPI_CS_PADIO		GLB->PADIO7		//GPIO 7
		#define	RTC676x_SPI_TX_PADIO		GLB->PADIO4
		#define	RTC676x_SPI_RX_PADIO		GLB->PADIO5

		#define	RTC676x_IRQ_PADIO			GLB->PADIO41	//GPIO 13
		#define	RTC676x_RESET_PADIO			GLB->PADIO42	//GPIO 0

		#define	RTC676x_IRQ_OUT_EN			GPIO->GPIO_OE13
		#define	RTC676x_IRQ_PULL_HIGH		GPIO->GPIO_PULL_HIGH13
		#define	RTC676x_IRQ_PULL_EN			GPIO->GPIO_PULL_EN13
		#define	RTC676x_IRQ_TRG_MODE		GPIO->GPIO_TRG_MODE13
		#define	RTC676x_IRQ_TRG_EDGE		GPIO->GPIO_TRG_EDGE13
		#define	RTC676x_IRQ_TRG_LEVEL		GPIO->GPIO_TRG_LEVEL13
		#define	RTC676x_IRQ_CLR_INTR		GPIO->CLR_GPIO_INTR13
		#define	RTC676x_IRQ_INTR_MASK		GPIO->GPIO_INTR_MSK13
		#define	RTC676x_IRQ_INTR_EN			GPIO->GPIO_INTR_EN13
		#define	RTC676x_IRQ_INPUT			GPIO->GPIO_I13
		#define	RTC676x_IRQ_INTR_FLAG		GPIO->GPIO_INTR_FLAG13
		#define	RTC676x_IRQ_OUT				GPIO->GPIO_O13
		#define RTC676x_INTR_SEL			GPIO_INTR_SEL_13

		#define	RTC676x_RESET_OUT_EN		GPIO->GPIO_OE0
		#define	RTC676x_RESET_OUT			GPIO->GPIO_O0
		
		#define	RTC676x_SPI_CS_OUT_EN		GPIO->GPIO_OE7
		#define	RTC676x_SPI_CS_OUT			GPIO->GPIO_O7

		#define	RTC676x_PADIO_CLK_SPI_MODE	6
		#define	RTC676x_PADIO_CS_SPI_MODE	0
		#define	RTC676x_PADIO_TX_SPI_MODE	6
		#define	RTC676x_PADIO_RX_SPI_MODE	6

		#define RTC676x_SPI_CLK_PIN			6
		#define RTC676x_SPI_CS_PIN			7
		#define RTC676x_SPI_TX_PIN			4
		#define RTC676x_SPI_RX_PIN			5
	#endif    
	//Board End
	//Board Start
	#if (defined(RTC676x) && defined(BSP_D_SN93714_TX_V1))		
		#define	RTC676x_SPI_CLK_PADIO		GLB->PADIO18
		#define	RTC676x_SPI_CS_PADIO		GLB->PADIO19
		#define	RTC676x_SPI_TX_PADIO		GLB->PADIO20
		#define	RTC676x_SPI_RX_PADIO		GLB->PADIO21
		#define	RTC676x_IRQ_PADIO			GLB->PADIO55			
		#define	RTC676x_RESET_PADIO			GLB->PADIO9

		#define	RTC676x_IRQ_OUT_EN			GPIO->GPIO_OE13
		#define	RTC676x_IRQ_PULL_HIGH		GPIO->GPIO_PULL_HIGH13
		#define	RTC676x_IRQ_PULL_EN			GPIO->GPIO_PULL_EN13
		#define	RTC676x_IRQ_TRG_MODE		GPIO->GPIO_TRG_MODE13
		#define	RTC676x_IRQ_TRG_EDGE		GPIO->GPIO_TRG_EDGE13
		#define	RTC676x_IRQ_TRG_LEVEL		GPIO->GPIO_TRG_LEVEL13
		#define	RTC676x_IRQ_CLR_INTR		GPIO->CLR_GPIO_INTR13
		#define	RTC676x_IRQ_INTR_MASK		GPIO->GPIO_INTR_MSK13
		#define	RTC676x_IRQ_INTR_EN			GPIO->GPIO_INTR_EN13
		#define	RTC676x_IRQ_INPUT			GPIO->GPIO_I13
		#define	RTC676x_IRQ_INTR_FLAG		GPIO->GPIO_INTR_FLAG13
		#define	RTC676x_IRQ_OUT				GPIO->GPIO_O13
		#define RTC676x_INTR_SEL			GPIO_INTR_SEL_13
		
		#define	RTC676x_RESET_OUT_EN		GPIO->GPIO_OE9
		#define	RTC676x_RESET_OUT			GPIO->GPIO_O9		
		#define	RTC676x_SPI_CS_OUT_EN		GPIO->GPIO_OE5	//Others
		#define	RTC676x_SPI_CS_OUT			GPIO->GPIO_O5

		#define	RTC676x_PADIO_CLK_SPI_MODE	2
		#define	RTC676x_PADIO_CS_SPI_MODE	0
		#define	RTC676x_PADIO_TX_SPI_MODE	2
		#define	RTC676x_PADIO_RX_SPI_MODE	2

		#define RTC676x_SPI_CLK_PIN			18
		#define RTC676x_SPI_CS_PIN			19
		#define RTC676x_SPI_TX_PIN			20
		#define RTC676x_SPI_RX_PIN			21		
	#endif
	#if (defined(RTC676x) && defined(BSP_D_SN93716_TX_V1))		
		#define	RTC676x_SPI_CLK_PADIO		GLB->PADIO18
		#define	RTC676x_SPI_CS_PADIO		GLB->PADIO19
		#define	RTC676x_SPI_TX_PADIO		GLB->PADIO20
		#define	RTC676x_SPI_RX_PADIO		GLB->PADIO21
		#define	RTC676x_IRQ_PADIO			GLB->PADIO36			
		#define	RTC676x_RESET_PADIO			GLB->PADIO35

		#define	RTC676x_IRQ_OUT_EN			GPIO->GPIO_OE8
		#define	RTC676x_IRQ_PULL_HIGH		GPIO->GPIO_PULL_HIGH8
		#define	RTC676x_IRQ_PULL_EN			GPIO->GPIO_PULL_EN8
		#define	RTC676x_IRQ_TRG_MODE		GPIO->GPIO_TRG_MODE8
		#define	RTC676x_IRQ_TRG_EDGE		GPIO->GPIO_TRG_EDGE8
		#define	RTC676x_IRQ_TRG_LEVEL		GPIO->GPIO_TRG_LEVEL8
		#define	RTC676x_IRQ_CLR_INTR		GPIO->CLR_GPIO_INTR8
		#define	RTC676x_IRQ_INTR_MASK		GPIO->GPIO_INTR_MSK8
		#define	RTC676x_IRQ_INTR_EN			GPIO->GPIO_INTR_EN8
		#define	RTC676x_IRQ_INPUT			GPIO->GPIO_I8
		#define	RTC676x_IRQ_INTR_FLAG		GPIO->GPIO_INTR_FLAG8
		#define	RTC676x_IRQ_OUT				GPIO->GPIO_O8
		#define RTC676x_INTR_SEL			GPIO_INTR_SEL_8
		
		#define	RTC676x_RESET_OUT_EN		PWM->PWM_EN3
		#define	RTC676x_RESET_OUT			PWM->PWM3_HIGH_CNT		
		#define	RTC676x_SPI_CS_OUT_EN		GPIO->GPIO_OE5	//Others
		#define	RTC676x_SPI_CS_OUT			GPIO->GPIO_O5

		#define	RTC676x_PADIO_CLK_SPI_MODE	2
		#define	RTC676x_PADIO_CS_SPI_MODE	0
		#define	RTC676x_PADIO_TX_SPI_MODE	2
		#define	RTC676x_PADIO_RX_SPI_MODE	2

		#define RTC676x_SPI_CLK_PIN			18
		#define RTC676x_SPI_CS_PIN			19
		#define RTC676x_SPI_TX_PIN			20
		#define RTC676x_SPI_RX_PIN			21		
	#endif
	//Board End
#endif
//TX End

void RTC676x_GPIO_Init(void)
{
#if defined(BUC_CU)   /*LCD : RX*/
#if (defined(BSP_SN93711_FHD_REC_RX_V4)||defined(BSP_D_SN93701_SSD2828_RX_V5) || defined(BSP_D_SN93701_TC358778_RX_V6))
	GLB->PADIO50 = 3;
	GLB->PADIO49 = 3;
	#endif	
#elif defined(BUC_CAM) /*Camera : TX*/
	#if defined(USERDEF_SPI_PIN)
	GLB->PADIO29 = 3;
	GLB->PADIO57 = 3;
	GLB->PADIO71 = 3;
	#endif
#endif

	// IRQ_N
	RTC676x_IRQ_PADIO = 0;
	RTC676x_IRQ_OUT_EN = 0;

	// RF_RST
	RTC676x_RESET_PADIO = 0;
	RTC676x_RESET_OUT_EN = 1;
	RTC676x_RESET_OUT = 1;

	GPIO_Intr_Setup(RTC676x_INTR_SEL, GPIO_INTR_EDGE_TRG, GPIO_INTR_FALLING_EDGE, RTC676x_RF_ISR);

	// SPI Pad mux setting
	RTC676x_SPI_CLK_PADIO = RTC676x_PADIO_CLK_SPI_MODE;
	RTC676x_SPI_CS_PADIO = RTC676x_PADIO_CS_SPI_MODE;
	RTC676x_SPI_TX_PADIO = RTC676x_PADIO_TX_SPI_MODE;
	RTC676x_SPI_RX_PADIO = RTC676x_PADIO_RX_SPI_MODE;

	//CS pin; both TH and RX are GPIO6
	RTC676x_SPI_CS_OUT_EN(1);
	RTC676x_SPI_CS_OUT = 1;

	printf("[RTC676x] SPI pin CLK/CS/TX/RX %d/%d/%d/%d\n",
		RTC676x_SPI_CLK_PIN,
		RTC676x_SPI_CS_PIN,
		RTC676x_SPI_TX_PIN,
		RTC676x_SPI_RX_PIN);
}
//------------------------------------------------------------------------------
void RTC676x_SPI_Init(SPI_Setup_t* setup)
{

	RTC676x_GPIO_Init();

    SSP->SSP_SCLKPH     = setup->ubSPI_CPHA;
    SSP->SSP_SCLKPO     = setup->ubSPI_CPOL;
    SSP->SSP_OPM        = setup->tSPI_Mode;
    SSP->SSP_SCLKDIV    = setup->uwClkDiv;
    SSP->SSP_FFMT       = 1;            // Motorola SPI

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

	RTC676x_SPI_Init_Step2(tRFDevice);

	printf("RTC676x: SPI init done!\r\n");
}
//------------------------------------------------------------------------------
void RTC676x_SPI_CS_High(void)
{
	RTC676x_SPI_CS_OUT = 1;
	RTC676x_SPI_CS_OUT = 0;
}
//------------------------------------------------------------------------------
void RTC676x_SPI_CS_Low(void)
{
	RTC676x_SPI_CS_OUT = 1;
	RTC676x_SPI_CS_OUT = 0;
}
//------------------------------------------------------------------------------
void RTC676x_RF_Reset_Pin_High(void)
{
	RTC676x_RESET_OUT = 1;
}
//------------------------------------------------------------------------------
void RTC676x_RF_Reset_Pin_Low(void)
{
	RTC676x_RESET_OUT = 0;
}
//------------------------------------------------------------------------------
void RTC676x_IRQ_INTR(uint32_t vaue)
{
	RTC676x_IRQ_INTR_EN = vaue;
}
//------------------------------------------------------------------------------
uint32_t RTC676x_IRQ_INTR_GET(void)
{
	return RTC676x_IRQ_INTR_EN;
}
//------------------------------------------------------------------------------
uint32_t RTC676x_IRQ_INPUT_GET(void)
{
	return RTC676x_IRQ_INPUT;
}
//------------------------------------------------------------------------------
void RTC676x_IRQ_INTR_MASK_SET(uint32_t value)
{
	RTC676x_IRQ_INTR_MASK = value;
}
//------------------------------------------------------------------------------
uint32_t RTC676x_IRQ_INTR_MASK_GET(void)
{
	return RTC676x_IRQ_INTR_MASK;
}
//------------------------------------------------------------------------------
uint32_t RTC676x_IRQ_INTR_FLAG_GET(void)
{
	return RTC676x_IRQ_INTR_FLAG;
}
//------------------------------------------------------------------------------
void RTC676x_IRQ_CLR_INTR_SET(uint32_t value)
{
	RTC676x_IRQ_CLR_INTR = value;
}
//------------------------------------------------------------------------------

void RTC676x_INTC_IrqClear(void)
{
  #if defined(BUC_CU)
  //    INTC_IrqClear(INTC_GPIO_IRQ);
  #else
  //    INTC_IrqClear(INTC_GPIO_IRQ);
  #endif
}


/**************************************************************************//**
 * @file     main.c
 * @version  V3.00
 * $Revision: 2 $
 * $Date: 14/11/20 2:21p $
 * @brief    NUC029 Series I2C Driver Sample Code (Master)
 *
 * @note
 * Copyright (C) 2014 Nuvoton Technology Corp. All rights reserved.
 *
 ******************************************************************************/
#include <stdio.h>

#include "NUC029xAN.h"

#include "sys_const.h"
#include "pub_ram.h"
#include "oled_ssd1306.h"

#define PLLCON_SETTING      SYSCLK_PLLCON_50MHz_XTAL
#define PLL_CLOCK           50000000



#define		OPTION_241119		TRUE    // <241119>  When Lifting Mode 2KHz ->  1KHz
#define		OPTION_250729		TRUE	// <250729>  When Connect USB  P24  PWM Port (0.2V) ->   OUPPUT Port (0 V)
//#define		OPTION_250902		TRUE	// <25090229>  Lifting ->  Rebooting
/*---------------------------------------------------------------------------------------------------------*/
/* Global variables                                                                                        */
/*---------------------------------------------------------------------------------------------------------*/
void ADC_InitContinuousScan(void);

extern	void	SOD_TEST(void);
extern	void 	SF_LED_RIR(unsigned char status);
extern	void 	SF_LED_RGB(unsigned char status);
void	SF_POWER_ON(void);
extern	void	Task_SOD_Display(void);

void  DEMO_LED_Color(unsigned char	seq ,unsigned char	color);
void	SF_LED_Control_OFF(void);

void SF_Muti_LED(unsigned char position,  unsigned char color);
void    SF_PWMOUT1_ON(void);
void    SF_PWMOUT1_OFF(void);
void    SF_PWMOUT2_ON(void);
void    SF_PWMOUT2_OFF(void);

void    SF_OUT1_OUT2(unsigned char Out1,unsigned char Out2 );

void	SF_MOTOR_WISTEC_Mode0(void);
void	SF_MOTOR_WISTEC_Mode1(void);
void	SF_MOTOR_WISTEC_Mode2(void);

volatile uint8_t g_u8PWMCount = 1;
volatile uint16_t g_u16Frequency;
volatile uint32_t g_u32Pulse = 0;
volatile uint32_t FM_LOOP_1MSEC = 0;
volatile uint32_t wCountDelay = 0;
volatile uint32_t rCount_IIC = 3000;

volatile uint32_t FM_LOOP_10MSEC = 0;
volatile uint32_t wWaitCount = 10;
volatile uint32_t	wCount_1sec =1000;

volatile uint32_t 	g_au32TMRINTCount[4] = {0};
volatile uint32_t 	g_TouchCnt = 0;
uint32_t	rInterupt_MotorStatus;
uint32_t	rPowerOnCnt;

/***********************************************/
uint32_t 		F_MOTOR_ON;
volatile uint32_t 		F_TouchCheck;
volatile uint32_t 		F_TouchCheckWindow;		// TRUE only during the 50ms/1000ms touch-check window (task_MotorCtrl -> TMR2_IRQHandler)
volatile uint32_t 		r_SenceCnt;
volatile uint32_t 		g_MotorCnt_1m; 		//

#define TOUCH_ADC_LOG_SIZE		20
volatile uint32_t		g_TouchADCLog[TOUCH_ADC_LOG_SIZE];		// 디버그용: 최근 TOUCH_ADC_LOG_SIZE개 터치 ADC 원시값 순환버퍼 (Watch창에서 확인)
volatile uint32_t		g_TouchADCLogIdx;						// 다음에 기록할 위치(0 ~ TOUCH_ADC_LOG_SIZE-1)

/***********************************************/


static volatile uint8_t g_u8SlvTRxAbortFlag = 0;
void I2C_SlaveTRx(uint32_t u32Status);

extern void 	oledTest(void);
extern void 	OLED_1in3_WriteData(uint8_t cmd,uint8_t r_data);
extern void NX11_SPI_Init(void);


/*---------------------------------------------------------------------------------------------------------*/

void 	PWM_PwmIRQHandler(void);
void	task_pwm_ctrl(void);
void	task_adc_ctrl(void);
void	task_ADC2_ADCIN(void);
void	SF_MOTOR_POWERB_Mode(void);
void	SF_MOTOR_Lifting_Mode(void);
void	SF_MOTOR_Intensive_Mode(void);
void	task_ADC0_Touch(void);		// Touch 
void	taskAI_ADC0_Touch(void);		// Touch 


extern	void	KEY_KeyScan(void);
extern	void	KEY_CheckAndExecute(void);
extern	void 	Task_InitTS06N(void);
extern	void	Task_PAN_Control(void);
extern	void	Task_Feltier_Control(void);
extern	void	Task_LEDBase_1msec(void);
extern	void 	SH1106_sendData(unsigned char i );
extern	void	Task_Door_Control(void);
extern	void	Task_LED_Control(void);
extern	void	Task_OSDFND_Control(void);
extern	void	Task_Feltier_Control (void);
extern	void I2C0_Init(void);
extern	void	SF_PWM_ZeroLevel(void);
extern	void 	SF_PowerHoldCtrl(unsigned char status);
extern	unsigned char  IsVccPinEnable(void);
extern	void	task_BatteryHandlerCtrl(void);

extern	void 	SF_H_FreqCtrl(unsigned char status);

//return '1' if initial DS18B20 successfully
extern	unsigned char init_DS18B20(void);
extern	void	SF_PWM_FREQ_ZeroLevel(void);

///return Temperature in float format
extern	float read_Temperature(void);
float temperature_test;
extern void    SF_SoundOut(unsigned int  Number);

void	task_MotorCtrl(void);
extern		void			SF_LED_Control_1msec(void);

/*---------------------------------------------------------------------------------------------------------*/
/*  I2C0 IRQ Handler                                                                                       */
/*---------------------------------------------------------------------------------------------------------*/
/* Assume PWM output frequency is 523Hz and duty ratio is 60%, user can calculate PWM settings by follows.
   PWM clock source frequency = __HXT = 12000000 in the sample code.
   (CNR+1) = PWM clock source frequency/prescaler/clock source divider/PWM output frequency
           = 12000000/2/1/523 = 11472 < 65536  (Note: If calculated value is larger than 65536, user should increase prescale value.)
   CNR = 11471 =>g_au16ScaleCnr[0] = 11471
   duty ratio = 60% = (CMR+1)/(CNR+1) ==> CMR = (CNR+1)*60/100-1 = 11472*60/100-1
   CMR = 6882 =>g_au16ScaleCmr[0] = 6882
*/

/**
 * @brief       PWMA IRQ Handler
 *
 * @param       None
 *
 * @return      None
 *
 * @details     ISR to handle PWMA interrupt event
 */
void PWMA_IRQHandler(void)
{
//    static uint32_t u32PwmIntFlag;

//    /* Handle PWMA Timer function */
//    u32PwmIntFlag = PWMA->PIIR;

//    /* PWMA channel 0 PWM timer interrupt */
//    if(u32PwmIntFlag & PWM_PIIR_PWMIF0_Msk)
//    {
//        PWMA->PIIR = PWM_PIIR_PWMIF0_Msk;
// //       PWM_PwmIRQHandler();
//    }
}
/**
 * @brief       PWMA IRQ Handler
 *
 * @param       None
 *
 * @return      None
 *
 * @details     ISR to handle PWMA interrupt event
 */
void PWMB_IRQHandler(void)
{
//    static uint32_t u32PwmIntFlag;

//    /* Handle PWMA Timer function */
//    u32PwmIntFlag = PWMB->PIIR;

//    /* PWMB channel 0 PWM timer interrupt */
//    if(u32PwmIntFlag & PWM_PIIR_PWMIF0_Msk)
//    {
//        PWMB->PIIR = PWM_PIIR_PWMIF0_Msk;
//  //      PWM_PwmIRQHandler();
//    }
}

/*---------------------------------------------------------------------------------------------------------*/
/* PWM Timer function                                                                                      */
/*---------------------------------------------------------------------------------------------------------*/
void PWM_PwmIRQHandler(void)
{
    if(g_u32Pulse == 1 * g_u16Frequency / 10)
    {
        /*--------------------------------------------------------------------------------------*/
        /* Stop PWMA channel 0 Timer (Recommended procedure method 2)                           */
        /* Set PWM Timer counter as 0, When interrupt request happen, disable PWM Timer         */
        /*--------------------------------------------------------------------------------------*/
        PWMA->CNR0 = 0;
    }

    if(g_u32Pulse == 1 * g_u16Frequency / 10 + 1)
        g_u8PWMCount = 0;
    g_u32Pulse++;
}

/**
 * @brief       Timer0 IRQ
 *
 * @param       None
 *
 * @return      None
 *
 * @details     The Timer0 default IRQ, declared in startup_NUC029xAN.s.
 */
static unsigned int		g_LEDTMRINTCount;





void SF_Muti_LED(unsigned char position,  unsigned char color)
{
	 switch(position)
	 {
		 case	POSIOTION_DL1:
			if ( color & 0x02)				P_DL1_G = FALSE;
			else							P_DL1_G = TRUE;	

			if ( color & 0x04)				P_DL1_R = FALSE;
			else							P_DL1_R = TRUE;	

		 break;
		 
		 case	POSIOTION_DL2:
			if ( color & 0x02)				P_DL2_G = FALSE;
			else							P_DL2_G = TRUE;	

			if ( color & 0x04)				P_DL2_R = FALSE;
			else							P_DL2_R = TRUE;	

		break;
		 
		 case	POSIOTION_DL3:
			if ( color & 0x02)				P_DL3_G = FALSE;
			else							P_DL3_G = TRUE;	

			if ( color & 0x04)				P_DL3_R = FALSE;
			else							P_DL3_R = TRUE;	
		break;

		 case	POSIOTION_DL4:
			if ( color & 0x02)				P_DL4_G = FALSE;
			else							P_DL4_G = TRUE;	

			if ( color & 0x04)				P_DL4_R = FALSE;
			else							P_DL4_R = TRUE;	
		break;

		 case	POSIOTION_DL5:
			if ( color & 0x02)				P_DL5_G = FALSE;
			else							P_DL5_G = TRUE;	

			if ( color & 0x04)				P_DL5_R = FALSE;
			else							P_DL5_R = TRUE;	
		break;
		 
		 case	POSIOTION_DL6:
			if ( color & 0x02)				P_DL6_G = FALSE;
			else							P_DL6_G = TRUE;	

			if ( color & 0x04)				P_DL6_R = FALSE;
			else							P_DL6_R = TRUE;	
		break;
		
		 case	POSIOTION_DL7:
			if ( color & 0x02)				P_DL7_G = FALSE;
			else							P_DL7_G = TRUE;	

			if ( color & 0x04)				P_DL7_R = FALSE;
			else							P_DL7_R = TRUE;	
		break;
		
		case	POSIOTION_DL8:
			if ( color & 0x02)				P_DL8_G = FALSE;
			else							P_DL8_G = TRUE;	

			if ( color & 0x04)				P_DL8_R = FALSE;
			else							P_DL8_R = TRUE;	
		break;
		//==============================================================
		case	POSIOTION_DL9_DL10:
			if ( color & 0x04)				P_DL9_DL10_R = FALSE;
			else							P_DL9_DL10_R = TRUE;	
		break;

		case	POSIOTION_DL11_DL12:
			if ( color & 0x04)				P_DL11_DL12_R = FALSE;
			else							P_DL11_DL12_R = TRUE;	
		break;

		default:
			break;	
	 }

}	



void  DEMO_LED_Color(unsigned char	seq ,unsigned char	color)
{
	switch(seq)
	{
		case 0:		// 1
		case 9:		// 1
			SF_Muti_LED(POSIOTION_DL1,color);				// 1
			SF_Muti_LED(POSIOTION_DL8,COLOR_BLACK);			// 2
			SF_Muti_LED(POSIOTION_DL2,COLOR_BLACK);			// 3
			SF_Muti_LED(POSIOTION_DL7,COLOR_BLACK);			// 4
			SF_Muti_LED(POSIOTION_DL5,COLOR_BLACK);			// 5
			SF_Muti_LED(POSIOTION_DL3,COLOR_BLACK);			// 6
			SF_Muti_LED(POSIOTION_DL6,COLOR_BLACK);			// 7
			SF_Muti_LED(POSIOTION_DL4,COLOR_BLACK);			// 8
			break;
		
		case 1:		// 1 2
			SF_Muti_LED(POSIOTION_DL1,color);				// 1
			SF_Muti_LED(POSIOTION_DL8,color);				// 2
			SF_Muti_LED(POSIOTION_DL2,COLOR_BLACK);			// 3
			SF_Muti_LED(POSIOTION_DL7,COLOR_BLACK);			// 4
			SF_Muti_LED(POSIOTION_DL5,COLOR_BLACK);			// 5
			SF_Muti_LED(POSIOTION_DL3,COLOR_BLACK);			// 6
			SF_Muti_LED(POSIOTION_DL6,COLOR_BLACK);			// 7
			SF_Muti_LED(POSIOTION_DL4,COLOR_BLACK);			// 8
			break;
			break;

		case 2:		// 2 3
		case 11:	// 2 3 ****	
			SF_Muti_LED(POSIOTION_DL1,COLOR_BLACK);			// 1
			SF_Muti_LED(POSIOTION_DL8,color);				// 2
			SF_Muti_LED(POSIOTION_DL2,color);				// 3
			SF_Muti_LED(POSIOTION_DL7,COLOR_BLACK);			// 4
			SF_Muti_LED(POSIOTION_DL5,COLOR_BLACK);			// 5
			SF_Muti_LED(POSIOTION_DL3,COLOR_BLACK);			// 6
			SF_Muti_LED(POSIOTION_DL6,COLOR_BLACK);			// 7
			SF_Muti_LED(POSIOTION_DL4,COLOR_BLACK);			// 8
			break;
		
		case 3:		// 3 6
			SF_Muti_LED(POSIOTION_DL1,COLOR_BLACK);			// 1
			SF_Muti_LED(POSIOTION_DL8,COLOR_BLACK);			// 2
			SF_Muti_LED(POSIOTION_DL2,color);				// 3
			SF_Muti_LED(POSIOTION_DL7,COLOR_BLACK);			// 4
			SF_Muti_LED(POSIOTION_DL5,COLOR_BLACK);			// 5
			SF_Muti_LED(POSIOTION_DL3,color);				// 6
			SF_Muti_LED(POSIOTION_DL6,COLOR_BLACK);			// 7
			SF_Muti_LED(POSIOTION_DL4,COLOR_BLACK);			// 8
			break;
		
		case 4:		// 6 5
		case 14:	// 5 6 ****	
	
			SF_Muti_LED(POSIOTION_DL1,COLOR_BLACK);			// 1
			SF_Muti_LED(POSIOTION_DL8,COLOR_BLACK);			// 2
			SF_Muti_LED(POSIOTION_DL2,COLOR_BLACK);			// 3
			SF_Muti_LED(POSIOTION_DL7,COLOR_BLACK);			// 4
			SF_Muti_LED(POSIOTION_DL5,color);				// 5
			SF_Muti_LED(POSIOTION_DL3,color);				// 6
			SF_Muti_LED(POSIOTION_DL6,COLOR_BLACK);			// 7
			SF_Muti_LED(POSIOTION_DL4,COLOR_BLACK);			// 8
			break;

		case 5:	// 5 4	
		case 13:	// 4 5 ****	
			SF_Muti_LED(POSIOTION_DL1,COLOR_BLACK);			// 1
			SF_Muti_LED(POSIOTION_DL8,COLOR_BLACK);			// 2
			SF_Muti_LED(POSIOTION_DL2,COLOR_BLACK);			// 3
			SF_Muti_LED(POSIOTION_DL7,color);				// 4
			SF_Muti_LED(POSIOTION_DL5,color);				// 5
			SF_Muti_LED(POSIOTION_DL3,COLOR_BLACK);			// 6
			SF_Muti_LED(POSIOTION_DL6,COLOR_BLACK);			// 7
			SF_Muti_LED(POSIOTION_DL4,COLOR_BLACK);			// 8
			break;
		
		case 6:		// 4 7
			SF_Muti_LED(POSIOTION_DL1,COLOR_BLACK);			// 1
			SF_Muti_LED(POSIOTION_DL8,COLOR_BLACK);			// 2
			SF_Muti_LED(POSIOTION_DL2,COLOR_BLACK);			// 3
			SF_Muti_LED(POSIOTION_DL7,color);				// 4
			SF_Muti_LED(POSIOTION_DL5,COLOR_BLACK);			// 5
			SF_Muti_LED(POSIOTION_DL3,COLOR_BLACK);			// 6
			SF_Muti_LED(POSIOTION_DL6,color);				// 7
			SF_Muti_LED(POSIOTION_DL4,COLOR_BLACK);			// 8
			break;

		case 7:	// 7 8	
		case 16:	// 7 8 	
			SF_Muti_LED(POSIOTION_DL1,COLOR_BLACK);			// 1
			SF_Muti_LED(POSIOTION_DL8,COLOR_BLACK);			// 2
			SF_Muti_LED(POSIOTION_DL2,COLOR_BLACK);			// 3
			SF_Muti_LED(POSIOTION_DL7,COLOR_BLACK);			// 4
			SF_Muti_LED(POSIOTION_DL5,COLOR_BLACK);			// 5
			SF_Muti_LED(POSIOTION_DL3,COLOR_BLACK);			// 6
			SF_Muti_LED(POSIOTION_DL6,color);				// 7
			SF_Muti_LED(POSIOTION_DL4,color);			    // 8
			break;

		case 8:		// 8
			SF_Muti_LED(POSIOTION_DL1,COLOR_BLACK);			// 1
			SF_Muti_LED(POSIOTION_DL8,COLOR_BLACK);			// 2
			SF_Muti_LED(POSIOTION_DL2,COLOR_BLACK);			// 3
			SF_Muti_LED(POSIOTION_DL7,COLOR_BLACK);			// 4
			SF_Muti_LED(POSIOTION_DL5,COLOR_BLACK);			// 5
			SF_Muti_LED(POSIOTION_DL3,COLOR_BLACK);			// 6
			SF_Muti_LED(POSIOTION_DL6,COLOR_BLACK);				// 7
			SF_Muti_LED(POSIOTION_DL4,color);			    // 8
			break;

//		case 9:		// 1
//			SF_Muti_LED(POSIOTION_DL1,COLOR_BLACK);			// 1
//			SF_Muti_LED(POSIOTION_DL8,COLOR_BLACK);			// 2
//			SF_Muti_LED(POSIOTION_DL2,COLOR_BLACK);			// 3
//			SF_Muti_LED(POSIOTION_DL7,COLOR_BLACK);			// 4
//			SF_Muti_LED(POSIOTION_DL5,COLOR_BLACK);			// 5
//			SF_Muti_LED(POSIOTION_DL3,COLOR_BLACK);			// 6
//			SF_Muti_LED(POSIOTION_DL6,color);				// 7
//			SF_Muti_LED(POSIOTION_DL4,color);			    // 8
//			break;

		case 10:		// 1 3
			SF_Muti_LED(POSIOTION_DL1,color);			// 1
			SF_Muti_LED(POSIOTION_DL8,COLOR_BLACK);			// 2
			SF_Muti_LED(POSIOTION_DL2,color);			// 3
			SF_Muti_LED(POSIOTION_DL7,COLOR_BLACK);			// 4
			SF_Muti_LED(POSIOTION_DL5,COLOR_BLACK);			// 5
			SF_Muti_LED(POSIOTION_DL3,COLOR_BLACK);			// 6
			SF_Muti_LED(POSIOTION_DL6,COLOR_BLACK);				// 7
			SF_Muti_LED(POSIOTION_DL4,COLOR_BLACK);			    // 8
			break;

//		case 11:	// 2 3 ****	
//			SF_Muti_LED(POSIOTION_DL1,COLOR_BLACK);			// 1
//			SF_Muti_LED(POSIOTION_DL8,COLOR_BLACK);			// 2
//			SF_Muti_LED(POSIOTION_DL2,COLOR_BLACK);			// 3
//			SF_Muti_LED(POSIOTION_DL7,COLOR_BLACK);			// 4
//			SF_Muti_LED(POSIOTION_DL5,COLOR_BLACK);			// 5
//			SF_Muti_LED(POSIOTION_DL3,COLOR_BLACK);			// 6
//			SF_Muti_LED(POSIOTION_DL6,color);				// 7
//			SF_Muti_LED(POSIOTION_DL4,color);			    // 8
//			break;

		
		case 12:	// 2 4 ****	
			SF_Muti_LED(POSIOTION_DL1,COLOR_BLACK);			// 1
			SF_Muti_LED(POSIOTION_DL8,color);			// 2
			SF_Muti_LED(POSIOTION_DL2,COLOR_BLACK);			// 3
			SF_Muti_LED(POSIOTION_DL7,color);			// 4
			SF_Muti_LED(POSIOTION_DL5,COLOR_BLACK);			// 5
			SF_Muti_LED(POSIOTION_DL3,COLOR_BLACK);			// 6
			SF_Muti_LED(POSIOTION_DL6,COLOR_BLACK);				// 7
			SF_Muti_LED(POSIOTION_DL4,COLOR_BLACK);			    // 8
			break;

//		case 13:	// 4 5 ****	
//			SF_Muti_LED(POSIOTION_DL1,COLOR_BLACK);			// 1
//			SF_Muti_LED(POSIOTION_DL8,COLOR_BLACK);			// 2
//			SF_Muti_LED(POSIOTION_DL2,COLOR_BLACK);			// 3
//			SF_Muti_LED(POSIOTION_DL7,COLOR_BLACK);			// 4
//			SF_Muti_LED(POSIOTION_DL5,COLOR_BLACK);			// 5
//			SF_Muti_LED(POSIOTION_DL3,COLOR_BLACK);			// 6
//			SF_Muti_LED(POSIOTION_DL6,color);				// 7
//			SF_Muti_LED(POSIOTION_DL4,color);			    // 8
//			break;
		
//		case 14:	// 5 6 ****	
//			SF_Muti_LED(POSIOTION_DL1,COLOR_BLACK);			// 1
//			SF_Muti_LED(POSIOTION_DL8,COLOR_BLACK);			// 2
//			SF_Muti_LED(POSIOTION_DL2,COLOR_BLACK);			// 3
//			SF_Muti_LED(POSIOTION_DL7,COLOR_BLACK);			// 4
//			SF_Muti_LED(POSIOTION_DL5,COLOR_BLACK);			// 5
//			SF_Muti_LED(POSIOTION_DL3,COLOR_BLACK);			// 6
//			SF_Muti_LED(POSIOTION_DL6,COLOR_BLACK);				// 7
//			SF_Muti_LED(POSIOTION_DL4,COLOR_BLACK);			    // 8
//			break;

		case 15:	// 6 8 	
			SF_Muti_LED(POSIOTION_DL1,COLOR_BLACK);			// 1
			SF_Muti_LED(POSIOTION_DL8,COLOR_BLACK);			// 2
			SF_Muti_LED(POSIOTION_DL2,COLOR_BLACK);			// 3
			SF_Muti_LED(POSIOTION_DL7,COLOR_BLACK);			// 4
			SF_Muti_LED(POSIOTION_DL5,COLOR_BLACK);			// 5
			SF_Muti_LED(POSIOTION_DL3,color);			// 6
			SF_Muti_LED(POSIOTION_DL6,COLOR_BLACK);			// 7
			SF_Muti_LED(POSIOTION_DL4,color);			   // 8
			break;
//		case 16:	// 7 8 	
//			SF_Muti_LED(POSIOTION_DL1,COLOR_BLACK);			// 1
//			SF_Muti_LED(POSIOTION_DL8,COLOR_BLACK);			// 2
//			SF_Muti_LED(POSIOTION_DL2,COLOR_BLACK);			// 3
//			SF_Muti_LED(POSIOTION_DL7,COLOR_BLACK);			// 4
//			SF_Muti_LED(POSIOTION_DL5,COLOR_BLACK);			// 5
//			SF_Muti_LED(POSIOTION_DL3,COLOR_BLACK);			// 6
//			SF_Muti_LED(POSIOTION_DL6,color);				// 7
//			SF_Muti_LED(POSIOTION_DL4,color);			    // 8
//			break;

		case 17:	// 7  	
			SF_Muti_LED(POSIOTION_DL1,COLOR_BLACK);			// 1
			SF_Muti_LED(POSIOTION_DL8,COLOR_BLACK);			// 2
			SF_Muti_LED(POSIOTION_DL2,COLOR_BLACK);			// 3
			SF_Muti_LED(POSIOTION_DL7,COLOR_BLACK);			// 4
			SF_Muti_LED(POSIOTION_DL5,COLOR_BLACK);			// 5
			SF_Muti_LED(POSIOTION_DL3,COLOR_BLACK);			// 6
			SF_Muti_LED(POSIOTION_DL6,color);				// 7
			SF_Muti_LED(POSIOTION_DL4,COLOR_BLACK);			    // 8
			break;
		
		default:
			break;
	}
}


void TMR0_IRQHandler(void)
{
    if(TIMER_GetCaptureIntFlag(TIMER0) == 1)
    {
        /* Clear Timer0 time-out interrupt flag */
        TIMER_ClearCaptureIntFlag(TIMER0);
        g_au32TMRINTCount[0]++;
   }
}


#define		V2_MODE1_ENTRY			1
#define		V2_MODE1_PROCESS		2
#define		V2_MODE2_ENTRY			3
#define		V2_MODE2_PROCESS		4
#define		V2_MODE3_ENTRY			5
#define		V2_MODE3_PROCESS		6

#define		V2_LONG_PRESS_OFF		7
#define		V2_LONG_PRESS_ON		8

#define		V2_LONG_PRESS_OFF		7
#define		V2_LONG_PRESS_ON		8


/**
 * @brief       Timer1 IRQ
 *
 * @param       None
 *
 * @return      None
 *
 * @details     The Timer1 default IRQ, declared in startup_NUC029xAN.s.
 */
void TMR1_IRQHandler(void)
{
	static unsigned char	CNT;

	if(TIMER_GetIntFlag(TIMER1) == 1)     // 1msec
    {
        /* Clear Timer1 capture interrupt flag */
        TIMER_ClearIntFlag(TIMER1);
//       g_au32TMRINTCount[1]++;
// 		P_NC_P02 ^= TRUE;				// 38PIN 
		
		
		if ( rF_demoLED )
		{
			if ( ++rF_demoLEDCnt > 120)	
			{
				rF_demoLEDCnt = 0;
				
				if ( ++rF_demoLEDSeq < 18)
				{
					DEMO_LED_Color(rF_demoLEDSeq - 1,COLOR_RED);
				}
				else
				{
					rF_demoLED = FALSE;
					SF_LED_Control_OFF();
				}
			}			
		}
		
		
		if ( ++CNT >= 10 )   CNT = 0;  

//		if ( CNT < rF_demoLEDduty ) 	P_NC_P07 = FALSE;
//		else							P_NC_P07 = TRUE;
		
		if ( rInterupt_MotorStatus == FALSE )
		{
			MOTOR_OFF;
			return;
		}		
		if (F_MOTOR_ON == FALSE )  MOTOR_OFF;
		else
		{		
			if ( ++g_MotorCnt_1m >= 2500  ) 	g_MotorCnt_1m =  0;
			
			if ( rF_MainMode ==  C_WISTEC_MODE0 )		SF_MOTOR_WISTEC_Mode0();
			else if ( rF_MainMode ==  C_WISTEC_MODE1 )  SF_MOTOR_WISTEC_Mode1();
			else if ( rF_MainMode ==  C_WISTEC_MODE2 )  SF_MOTOR_WISTEC_Mode2();
		}

	}
}

/**
 * @brief       Timer2 IRQ
 *
 * @param       None
 *
 * @return      None
 *
 * @details     The Timer2 default IRQ, declared in startup_NUC029xAN.s.
 */
	#define     RECTANGUALR_TOTAL_PEREOD		400 * 10					//  125 * 4000 = 25msec	
	#define     RECTANGUALR_PEREOD				400							//  125 * 400  = 25msec	
	#define     RECTANGUALR_DUTY				RECTANGUALR_PEREOD * 2 		//  125 * 800  =  10KHz	
	static uint32_t		Cnt10usec = 1;
//	static unsigned int		rIntensiveCnt = 1;
//	static unsigned char	rVoltageCnt = 1;
//	static unsigned int		rPowerBCnt = 1;
//	static uint32_t		rLiftingCnt = 1;
//	static uint32_t		rFillingCnt = 1;

//#define		VOLTAGE_FREQUENCY		400000		// 250Kz	PWMOUT  PWM4 P24        23Pin	

#define		VOLTAGE_FREQUENCY		200000		// 200Kz	PWMOUT  PWM4 P24        23Pin	
#define		TOUCH_FREQUENCY			100000		// 100Kz	PWM2,PWM3    P22,P43    21Pin,12Pin	


void    SF_PWMOUT_P24(uint32_t u32Frequency,uint32_t u32DutyCycle)
{
	PWM_EnableOutput(PWMB, 1 << 0x0);									// PWM4	P24  
	PWM_ConfigOutputChannel(PWMB, PWM_CH0, u32Frequency, u32DutyCycle);		// PWM4	P24  
	PWM_EnablePeriodInt(PWMB, PWM_CH0, PWM_PERIOD_INT_UNDERFLOW);		// PWM4	P24  
	PWM_Start(PWMB, 1 << 0x0);											// PWM4	P24  
}

void    SF_PWMOUT_P22(uint32_t u32Frequency,uint32_t u32DutyCycle)
{
	PWM_EnableOutput(PWMA, 1 << 0x2);									// PWM2	P22  
	PWM_ConfigOutputChannel(PWMA, PWM_CH2, BUZZER_FREQ, u32DutyCycle);	// PWM2	P22  
	PWM_EnablePeriodInt(PWMA, PWM_CH2, PWM_PERIOD_INT_UNDERFLOW);		// PWM2	P22  
	PWM_Start(PWMA, 1 << 0x2);											// PWM2	P22  
}




void    SF_PWMOUT1_ON()
{
	SYS->P2_MFP = SYS_MFP_P22_PWM2 ;
	PWM_EnableOutput(PWMA, 1 << 0x2);												// PWM4	P24  
	PWM_ConfigOutputChannel(PWMA, PWM_CH2, 20000,75);
	PWM_EnablePeriodInt(PWMA, PWM_CH2, PWM_PERIOD_INT_UNDERFLOW);					// PWM4	P24  
	PWM_Start(PWMA, 1 << 0x2);														// PWM4	P24  
}

void    SF_PWMOUT1_OFF()
{
	PWM_DisableOutput(PWMA, 1 << 0x02);
	SYS->P2_MFP &= ~SYS_MFP_P22_Msk;
	P22 = FALSE;
}


void    SF_PWMOUT2_ON()
{
	SYS->P2_MFP = SYS_MFP_P23_PWM3 ;
	PWM_EnableOutput(PWMA, 1 << 0x3);									// PWM2	P22  
	PWM_ConfigOutputChannel(PWMA, PWM_CH3, 20000, 75);	// PWM2	P22  
	PWM_EnablePeriodInt(PWMA, PWM_CH3, PWM_PERIOD_INT_UNDERFLOW);		// PWM2	P22  
	PWM_Start(PWMA, 1 << 0x3);											// PWM2	P22  
}

void    SF_PWMOUT2_OFF()
{
	PWM_DisableOutput(PWMA, 1 << 0x03);
	SYS->P2_MFP &= ~SYS_MFP_P23_Msk;
	P23 = FALSE;
}

void    SF_OUT1_OUT2(unsigned char Out1,unsigned char Out2 )
{
	if ( Out1 )  P_O_OUT1_ON;
	else		 P_O_OUT1_OFF;

	if ( Out2 )  P_O_OUT2_ON;
	else		 P_O_OUT2_OFF;
	
}



// https://deepbluembedded.com/sine-lookup-table-generator-calculator/
static const  uint32_t P22_DUTY_10KHz[3][10] = 
{
	{ 	// Level Mode = 0										
		11, 14, 16, 16, 14, 11, 7, 5, 5, 7,
	},
	{ 	// Level Mode = 1										
		16, 22, 25, 25, 22, 16, 9, 6, 6, 9,
	},
	{ 	// Level Mode = 2										
		26, 38, 45, 45, 38, 26, 13, 6, 6, 13,
	},
};	



uint32_t		rDutyMax_test;
uint32_t		rCanRead_Yes;
	#define		MODE3_250MSEC		400  //  
	#define		MODE3_500MSEC		100 * 25 
	#define		MODE3_750MSEC		200 * 25  
	#define		MODE2_1000MSEC		10   //  

	#define		LEVEL1_DUTY		5   //  
	#define		LEVEL2_DUTY		5   // 10 
	#define		LEVEL3_DUTY		5   // 15 


#define		MODE0_1Msec		10		
#define		MODE0_START		2840		
		#define		MODE1_2Sec		4000
		#define		MODE1_Front1Sec		MODE1_2Sec/2

		#define		MODE0_20Hz		50				// 25msec + 25msec
		#define		MODE0_30Hz		33
		#define		MODE0_80Hz		25

		#define		MODE1_LEVEL1		4				// 25msec + 25msec
		#define		MODE1_LEVEL2		16				// 25msec + 25msec
		#define		MODE1_LEVEL3		32				// 25msec + 25msec

		#define		MODE2_LEVEL1		4				// 25msec + 25msec
		#define		MODE2_LEVEL2		20				// 25msec + 25msec
		#define		MODE2_LEVEL3		32				// 25msec + 25msec


void TMR2_IRQHandler(void)		// duty : 10usec(H)  + 10 usec(L)	50khZ
{
//	static unsigned char	F_Timer2Toggle = FALSE;
	static unsigned   char F_OutToggle;
	static uint32_t		rPhaseCnt;		// MODE1's sub-cycle phase, kept in sync with Cnt10usec without using '%' (no HW divider on this MCU)
//	static unsigned   int  quotient;
	unsigned 	char	remaider;

    if(TIMER_GetIntFlag(TIMER2) == 1)			// 100Khz 10usec ->  10Kz 500usec   
    {
        /* Clear Timer2 time-out interrupt flag */
        TIMER_ClearIntFlag(TIMER2);
//	    g_au32TMRINTCount[2]++;
//			P_NC_P06 ^= TRUE;	

		
		if ( F_FirstPowerOn )			return;	
		
		if ( IsVccPinEnable() || rF_MainMode == C_WISTEC_MODE_OFF )											//  VCC_IN
		{
			SF_PWMOUT1_OFF();
			SF_PWMOUT2_OFF();
			return;
		}


		if ( rF_MainMode == C_WISTEC_MODE0)
		{
			if ( F_TouchCheckWindow )		// task_MotorCtrl: 50ms/1000ms touch-check window - checked first here so Cnt10usec/F_OutToggle/PWM freeze and OUT1/OUT2 stay glitch-free at ON/ON
			{
				P_O_OUT1_ON;
				P_O_OUT2_ON;
				return;
			}

			if ( ++Cnt10usec >= 50 || FOD_UpdateLevel)		// 500usec * 25 = 50msec / 50msec
			{
				FOD_UpdateLevel = FALSE;
				if (rF_LevelMode == C_LEVEL_0 )   		Cnt10usec = 0;
				else if (rF_LevelMode == C_LEVEL_1 )   	Cnt10usec = 17;
				else   									Cnt10usec = 25;
				
				SF_PWMOUT1_OFF();
				SF_PWMOUT2_OFF();
				F_OutToggle ^= TRUE;
			}

			if ( F_OutToggle )
			{
				P_O_OUT1_ON;
				P_O_OUT2_OFF;
			}
			else
			{
				P_O_OUT1_OFF;
				P_O_OUT2_ON;
			}
		}
		else if ( rF_MainMode == C_WISTEC_MODE1) 
		{
			if ( ++Cnt10usec >= MODE1_2Sec || FOD_UpdateLevel)		// 100usec * 10 * 1000 * 8
			{
				FOD_UpdateLevel = FALSE;

				SF_PWMOUT1_ON();
				SF_PWMOUT2_OFF();
				SF_OUT1_OUT2(FALSE,TRUE);
				Cnt10usec = 0;			// 8sec
				rPhaseCnt = 0;
			}
			else if ( Cnt10usec < MODE1_Front1Sec )		//   40Msec
			{
				if ( ++rPhaseCnt >= 80 )	rPhaseCnt = 0;		// same value as (Cnt10usec % 80), no divide
				remaider = rPhaseCnt;
				if ( remaider == 0 )
				{
					SF_PWMOUT1_ON();
					SF_PWMOUT2_OFF();
					SF_OUT1_OUT2(FALSE,TRUE);
				}
				else 
				{
					if (rF_LevelMode == C_LEVEL_0 )
					{  
						if( remaider == MODE1_LEVEL1 )    		
						{
							SF_PWMOUT1_OFF();
							SF_OUT1_OUT2(FALSE,FALSE);
						}
						else if( remaider == 40 )   {
							SF_PWMOUT2_ON();
							SF_OUT1_OUT2(TRUE,FALSE);

						}
						else if( remaider == 40 + MODE1_LEVEL1 ) 
						{  
							SF_PWMOUT2_OFF();
							SF_OUT1_OUT2(FALSE,FALSE);
						}
					}
					else if (rF_LevelMode == C_LEVEL_1 )
					{  

						if( remaider == MODE1_LEVEL2 )    		
						{
							SF_PWMOUT1_OFF();
							SF_OUT1_OUT2(FALSE,FALSE);
						}
						else if( remaider == 40 )   {
							SF_PWMOUT2_ON();
							SF_OUT1_OUT2(TRUE,FALSE);

						}
						else if( remaider == 40 + MODE1_LEVEL2 ) 
						{  
							SF_PWMOUT2_OFF();
							SF_OUT1_OUT2(FALSE,FALSE);
						}
					}

					else if (rF_LevelMode == C_LEVEL_2 )
					{  
						if( remaider == MODE1_LEVEL3 )    		
						{
							SF_PWMOUT1_OFF();
							SF_OUT1_OUT2(FALSE,FALSE);
						}
						else if( remaider == 40 )   {
							SF_PWMOUT2_ON();
							SF_OUT1_OUT2(TRUE,FALSE);

						}
						else if( remaider == 40 + MODE1_LEVEL3 ) 
						{  
							SF_PWMOUT2_OFF();
							SF_OUT1_OUT2(FALSE,FALSE);
						}
					}
				}					
			}
			else
			{
				if ( Cnt10usec == MODE1_Front1Sec )		rPhaseCnt = 0;		// fresh 250-cycle starts exactly here (MODE1_Front1Sec is a multiple of 80)
				else if ( ++rPhaseCnt >= 250 )				rPhaseCnt = 0;
				remaider = rPhaseCnt;		// same value as (Cnt10usec % 250), no divide
				
				if ( remaider == 0 )
				{
					SF_PWMOUT1_ON();
					SF_PWMOUT2_OFF();
					SF_OUT1_OUT2(FALSE,TRUE);
				}

				if (rF_LevelMode == C_LEVEL_0 )
				{  
					if( remaider == MODE1_LEVEL1 * 2 )    		
					{
						SF_PWMOUT1_OFF();
						SF_OUT1_OUT2(FALSE,FALSE);
					}
					else if( remaider == 125 )   {
						SF_PWMOUT2_ON();
						SF_OUT1_OUT2(TRUE,FALSE);

					}
					else if( remaider == 125+ MODE1_LEVEL1  * 2) 
					{  
						SF_PWMOUT2_OFF();
						SF_OUT1_OUT2(FALSE,FALSE);
					}

				}

				else if (rF_LevelMode == C_LEVEL_1 )
				{  
					if( remaider == MODE1_LEVEL2 *2 )    		
					{
						SF_PWMOUT1_OFF();
						SF_OUT1_OUT2(FALSE,FALSE);
					}
					else if( remaider == 125 )   {
						SF_PWMOUT2_ON();
						SF_OUT1_OUT2(TRUE,FALSE);

					}
					else if( remaider == 125+ MODE1_LEVEL2 * 2 ) 
					{  
						SF_PWMOUT2_OFF();
						SF_OUT1_OUT2(FALSE,FALSE);
					}
				}

				else if (rF_LevelMode == C_LEVEL_2 )
				{  

					if( remaider == MODE1_LEVEL3 * 2)    		
					{
						SF_PWMOUT1_OFF();
						SF_OUT1_OUT2(FALSE,FALSE);
					}
					else if( remaider == 125 )   {
						SF_PWMOUT2_ON();
						SF_OUT1_OUT2(TRUE,FALSE);

					}
					else if( remaider == 125 + MODE1_LEVEL3 * 2) 
					{  
						SF_PWMOUT2_OFF();
						SF_OUT1_OUT2(FALSE,FALSE);
					}
				}
			}
		}
		else	// (rF_MainMode == C_WISTEC_MODE2)	
		{
			if ( ++Cnt10usec == 250  || FOD_UpdateLevel)   // 100usec
			{
				FOD_UpdateLevel = FALSE;
				Cnt10usec = 0;
				P_O_OUT1_OFF;
				P_O_OUT2_OFF;
			}	

			remaider = Cnt10usec;		// Cnt10usec is reset to 0 the instant it would reach 250 above, so it's already always in [0,249] - '% 250' was a no-op divide
			if ( remaider == 0 )
			{

				SF_OUT1_OUT2(FALSE,TRUE);
				SF_PWMOUT1_ON();
				SF_PWMOUT2_OFF();
			}

			if (rF_LevelMode == C_LEVEL_0 )
			{  
				if( remaider == MODE2_LEVEL1 )    		
				{
					SF_OUT1_OUT2(FALSE,FALSE);
					SF_PWMOUT1_OFF();
				}
				else if( remaider == 125 )   {
					SF_OUT1_OUT2(TRUE,FALSE);
					SF_PWMOUT2_ON();
				}
				else if( remaider == 125+MODE2_LEVEL1 ) 
				{  
					SF_OUT1_OUT2(FALSE,FALSE);
					SF_PWMOUT2_OFF();
				}
			}

			else if (rF_LevelMode == C_LEVEL_1 )
			{  
				if( remaider == MODE2_LEVEL2 )    		
				{
					SF_OUT1_OUT2(FALSE,FALSE);
					SF_PWMOUT1_OFF();
				}
				else if( remaider == 125 )   {
					SF_OUT1_OUT2(TRUE,FALSE);
					SF_PWMOUT2_ON();
				}
				else if( remaider == 125+ MODE2_LEVEL2 ) 
				{  
					SF_OUT1_OUT2(FALSE,FALSE);
					SF_PWMOUT2_OFF();
				}

			}

			else if (rF_LevelMode == C_LEVEL_2 )
			{  
				if( remaider == MODE2_LEVEL3 )    		
				{
					SF_PWMOUT1_OFF();
					SF_OUT1_OUT2(FALSE,FALSE);
				}
				else if( remaider == 125 )   {
					SF_PWMOUT2_ON();
					SF_OUT1_OUT2(TRUE,FALSE);
				}
				else if( remaider == 125 + MODE2_LEVEL3 )
				{
					SF_PWMOUT2_OFF();
					SF_OUT1_OUT2(FALSE,FALSE);
				}

			}
		}

		if ( F_TouchCheckWindow )		// task_MotorCtrl: 50ms/1000ms touch-check window - mode logic runs as usual, only OUT1/OUT2 forced ON (H/H) afterward
		{
			P_O_OUT1_ON;
			P_O_OUT2_ON;
		}
    }
}


/**
 * @brief       Timer3 IRQ
 *
 * @param       None
 *
 * @return      None
 *
 * @details     The Timer3 default IRQ, declared in startup_NUC029xAN.s.
 */
void TMR3_IRQHandler(void)
{
 	
    if(TIMER_GetIntFlag(TIMER3) == 1)
    {
        /* Clear Timer3 time-out interrupt flag */
        TIMER_ClearIntFlag(TIMER3);
		FM_LOOP_1MSEC = TRUE;
		if ( wCountDelay )
			--wCountDelay;

		if ( rCount_IIC)
			--rCount_IIC;
			

		if ( !--wWaitCount)
		{
			FM_LOOP_10MSEC = TRUE;
			wWaitCount = 10;
		}
		
		if ( !--wCount_1sec )
		{
			FM_LOOP_1SEC= TRUE;
			wCount_1sec = 1003;
			rFeedBackFreq[0] = g_au32TMRINTCount[0];
			g_au32TMRINTCount[1]= 0;;
			g_au32TMRINTCount[0]= 0;;
		}	
    }
}


void Delay_1ms(uint32_t time)
{
	wCountDelay = time;

    while (wCountDelay)
    {
    }
}



void SYS_Init(void)
{
     /*---------------------------------------------------------------------------------------------------------*/
    /* Init System Clock                                                                                       */
    /*---------------------------------------------------------------------------------------------------------*/

    /* Enable Internal RC clock */
//    CLK_EnableXtalRC(CLK_PWRCON_OSC22M_EN_Msk);
    /* Enable clock source */
    CLK_EnableXtalRC(CLK_PWRCON_OSC22M_EN_Msk);		// CLK->PWRCON |= u32ClkMask;
    /* Waiting for clock source ready */
    CLK_WaitClockReady(CLK_CLKSTATUS_OSC22M_STB_Msk);

    /* Disable PLL first to avoid unstable when setting PLL */
    CLK_DisablePLL();		// CLK->PLLCON |= CLK_PLLCON_PD_Msk;
    /* Set PLL frequency */
    CLK->PLLCON = (CLK->PLLCON & ~(0x000FFFFFul)) | 0x00004217ul;
    /* Waiting for PLL ready */
    CLK_WaitClockReady(CLK_CLKSTATUS_PLL_STB_Msk);


	
    /* If the defines do not exist in your project, please refer to the related clk.h in the clk_h folder appended to the tool package. */
    /* Set HCLK clock */
	// void CLK_SetHCLK(uint32_t u32ClkSrc, uint32_t u32ClkDiv)
	//{
	//	CLK->CLKSEL0 = (CLK->CLKSEL0 & ~CLK_CLKSEL0_HCLK_S_Msk) | u32ClkSrc;
	//	CLK->CLKDIV = (CLK->CLKDIV & ~CLK_CLKDIV_HCLK_N_Msk) | u32ClkDiv;
	//	SystemCoreClockUpdate();
	//}

//	CLK_SetHCLK(CLK_CLKSEL0_HCLK_S_PLL, CLK_CLKDIV_HCLK(1));
	CLK_SetHCLK(CLK_CLKSEL0_HCLK_S_HIRC, CLK_CLKDIV_HCLK(1));

    /* Enable IP clock */
//void CLK_EnableModuleClock(uint32_t u32ModuleIdx)
//{
//   *(volatile uint32_t *)((uint32_t)&CLK->APBCLK + (MODULE_APBCLK(u32ModuleIdx) * 4))  |= 1 << MODULE_IP_EN_Pos(u32ModuleIdx);
//}

	/* Enable peripheral clock */
    CLK->APBCLK = 	CLK_APBCLK_ADC_EN_Msk |
//					CLK_APBCLK_PWM67_EN_Msk | 
					CLK_APBCLK_PWM45_EN_Msk |
					CLK_APBCLK_PWM23_EN_Msk |
					CLK_APBCLK_PWM01_EN_Msk |
		//			CLK_APBCLK_UART0_EN_Msk |

					CLK_APBCLK_TMR0_EN_Msk |
					CLK_APBCLK_TMR1_EN_Msk |
					CLK_APBCLK_TMR2_EN_Msk |
					CLK_APBCLK_TMR3_EN_Msk ;

	/* Peripheral clock source */
    CLK->CLKSEL1 = 	CLK_CLKSEL1_ADC_S_HIRC |
					CLK_CLKSEL1_TMR0_S_HIRC |
					CLK_CLKSEL1_TMR1_S_HIRC |
					CLK_CLKSEL1_TMR2_S_HIRC |
					CLK_CLKSEL1_TMR3_S_HIRC |
		//			CLK_CLKSEL1_UART_S_HIRC |
					CLK_CLKSEL1_PWM23_S_HIRC | 
					CLK_CLKSEL1_PWM01_S_HIRC ;

	/* Peripheral clock source */
    CLK->CLKSEL2 = 	CLK_CLKSEL2_PWM67_S_HIRC |
					CLK_CLKSEL2_PWM45_S_HIRC;


    /* Update System Core Clock */
    /* User can use SystemCoreClockUpdate() to calculate SystemCoreClock. */
    SystemCoreClockUpdate();

	//  Reset 
    SYS_ResetModule(TMR0_RST);
    SYS_ResetModule(TMR1_RST);
    SYS_ResetModule(TMR2_RST);
    SYS_ResetModule(TMR3_RST);

	SYS_ResetModule(ADC_RST);
    SYS_ResetModule(PWM03_RST);
    SYS_ResetModule(PWM47_RST);
//    SYS_ResetModule(I2C0_RST);

	/* Update System Core Clock */
    /* User can use SystemCoreClockUpdate() to calculate PllClock, SystemCoreClock and CycylesPerUs automatically. */
    //SystemCoreClockUpdate();
    PllClock        = PLL_CLOCK;            // PLL
    SystemCoreClock = PLL_CLOCK / 1;        // HCLK
    CyclesPerUs     = PLL_CLOCK / 1000000;  // For SYS_SysTickDelay()


//	P00	P_O_S_DATA		P10	P_I_TOUCHADC		P20	P_O_OUT1		P30	P_DL2_R				P40	P_DL7_G
//	P01	P_O_S_CLK		P11	P_NC_P11			P21	P_O_OUT2		P31	P_DL2_G          	P41	P_LED_WHITE_1
//	P02	P_I_POWER		P12	P_I_ADCIN			P22	P_O_PWM_OUT		P32	P_DL3_R          	P42	P_NC_P42
//	P03	P_I_VCCIN		P13	P_O_MOTORCTRL		P23	P_DL6_G			P33	P_DL3_G				P43	P_DL5_R
//	P04	P_LED_WHITE_2	P14	P_O_POWERHOLD		P24	P_DL7_R			P34	P_DL4_R				P44	P_O_S_RESET
//	P05	P_I_VBAT		P15	P_I_SW2				P25	P_DL8_R			P35	P_DL4_G				P45	P_O_S_ENABLE
//	P06	P_LED_WHITE_3	P16	P_DL1_R				P26	P_DL8_G			P36	P_DL5_G				P46	ICE_CLK
//	P07	P_DL11_DL12_R	P17	P_DL1_G				P27	P_DL9_DL10_R	P37	P_DL6_R				P47	ICP_DATA

    /* Configure P1.2 and P4.1 to default Quasi-bidirectional mode */

	P0->PMD = (P0->PMD & (~GPIO_PMD_PMD0_Msk)) | (GPIO_PMD_OUTPUT << GPIO_PMD_PMD0_Pos);		// P00  OK P_O_S_DATA  	NC
	P0->PMD = (P0->PMD & (~GPIO_PMD_PMD1_Msk)) | (GPIO_PMD_OUTPUT  << GPIO_PMD_PMD1_Pos);		// P01  OK P_O_S_CLK    P_O_RF  	
	P0->PMD = (P0->PMD & (~GPIO_PMD_PMD2_Msk)) | (GPIO_PMD_OPEN_DRAIN << GPIO_PMD_PMD2_Pos);	// P02  OK P_I_POWER    P_O_HIFU  	
	P0->PMD = (P0->PMD & (~GPIO_PMD_PMD3_Msk)) | (GPIO_PMD_INPUT << GPIO_PMD_PMD3_Pos);			// P03  OK P_I_VCCIN  	INPUT  OC
	P0->PMD = (P0->PMD & (~GPIO_PMD_PMD4_Msk)) | (GPIO_PMD_OUTPUT << GPIO_PMD_PMD4_Pos);		// P04  OK P_LED_WHITE_2  	NC
	P0->PMD = (P0->PMD & (~GPIO_PMD_PMD5_Msk)) | (GPIO_PMD_OPEN_DRAIN << GPIO_PMD_PMD5_Pos);	// P05  OK P_I_VBAT          OC 
	P0->PMD = (P0->PMD & (~GPIO_PMD_PMD6_Msk)) | (GPIO_PMD_OUTPUT << GPIO_PMD_PMD6_Pos);		// P06  OK  P_LED_WHITE_3    NC 
	P0->PMD = (P0->PMD & (~GPIO_PMD_PMD7_Msk)) | (GPIO_PMD_OUTPUT << GPIO_PMD_PMD7_Pos);		// P07  OK  P_DL11_DL12_R    NC  

    P1->PMD = (P1->PMD & (~GPIO_PMD_PMD0_Msk)) | (GPIO_PMD_INPUT << GPIO_PMD_PMD0_Pos);			// P10  OK P_I_TOUCHADC   
    P1->PMD = (P1->PMD & (~GPIO_PMD_PMD1_Msk)) | (GPIO_PMD_INPUT << GPIO_PMD_PMD1_Pos);			// P11  OK P_NC_P11       
    P1->PMD = (P1->PMD & (~GPIO_PMD_PMD2_Msk)) | (GPIO_PMD_OPEN_DRAIN << GPIO_PMD_PMD2_Pos);			// P12  OK P_I_ADCIN      
    P1->PMD = (P1->PMD & (~GPIO_PMD_PMD3_Msk)) | (GPIO_PMD_OUTPUT << GPIO_PMD_PMD3_Pos);		// P13  OK P_O_MOTORCTRL  
	P1->PMD = (P1->PMD & (~GPIO_PMD_PMD4_Msk)) | (GPIO_PMD_OUTPUT << GPIO_PMD_PMD4_Pos);	// P14  OK P_O_POWERHOLD  
    P1->PMD = (P1->PMD & (~GPIO_PMD_PMD5_Msk)) | (GPIO_PMD_QUASI << GPIO_PMD_PMD5_Pos);			// P15  OK  P_I_SW2   
    P1->PMD = (P1->PMD & (~GPIO_PMD_PMD6_Msk)) | (GPIO_PMD_OUTPUT << GPIO_PMD_PMD6_Pos);		// P16  OK P_DL1_R  
    P1->PMD = (P1->PMD & (~GPIO_PMD_PMD7_Msk)) | (GPIO_PMD_OUTPUT << GPIO_PMD_PMD7_Pos);		// P17  OK P_DL1_G  

	//  P20 P21 P24 P25   Open Drain Port 
    P2->PMD = (P2->PMD & (~GPIO_PMD_PMD0_Msk)) | (GPIO_PMD_OUTPUT << GPIO_PMD_PMD0_Pos);		// P20  OK P_O_OUT1 
	P2->PMD = (P2->PMD & (~GPIO_PMD_PMD1_Msk)) | (GPIO_PMD_OUTPUT << GPIO_PMD_PMD1_Pos);		// P21  OK P_O_OUT2 
	P2->PMD = (P2->PMD & (~GPIO_PMD_PMD2_Msk)) | (GPIO_PMD_OUTPUT << GPIO_PMD_PMD2_Pos);		// P22  OK P_O_PWM_OUT 
    P2->PMD = (P2->PMD & (~GPIO_PMD_PMD3_Msk)) | (GPIO_PMD_OUTPUT << GPIO_PMD_PMD3_Pos);		// P23  OK P_DL6_G    P_O_LED_RGB_R
    P2->PMD = (P2->PMD & (~GPIO_PMD_PMD4_Msk)) | (GPIO_PMD_OUTPUT << GPIO_PMD_PMD4_Pos);		// P24  OK P_DL7_R    P_O_LED_RGB_G    
    P2->PMD = (P2->PMD & (~GPIO_PMD_PMD5_Msk)) | (GPIO_PMD_OUTPUT << GPIO_PMD_PMD5_Pos);		// P25  OK P_DL8_R    P_O_VDD_HIGH_ON
    P2->PMD = (P2->PMD & (~GPIO_PMD_PMD6_Msk)) | (GPIO_PMD_OUTPUT << GPIO_PMD_PMD6_Pos);	   	// P26  OK P_DL8_G    P_O_LED_MULTI_G
    P2->PMD = (P2->PMD & (~GPIO_PMD_PMD7_Msk)) | (GPIO_PMD_OUTPUT << GPIO_PMD_PMD7_Pos);		// P27  OK P_DL9_DL10_R    


    P3->PMD = (P3->PMD & (~GPIO_PMD_PMD0_Msk)) | (GPIO_PMD_OUTPUT << GPIO_PMD_PMD0_Pos);		// P30  OK P_DL2_R
    P3->PMD = (P3->PMD & (~GPIO_PMD_PMD1_Msk)) | (GPIO_PMD_OUTPUT << GPIO_PMD_PMD1_Pos);		// P31  OK P_DL2_G
    P3->PMD = (P3->PMD & (~GPIO_PMD_PMD2_Msk)) | (GPIO_PMD_OUTPUT << GPIO_PMD_PMD2_Pos);		// P32  OK P_DL3_R
    P3->PMD = (P3->PMD & (~GPIO_PMD_PMD3_Msk)) | (GPIO_PMD_OUTPUT << GPIO_PMD_PMD3_Pos);		// P33  OK P_DL3_G 
	P3->PMD = (P3->PMD & (~GPIO_PMD_PMD4_Msk)) | (GPIO_PMD_OUTPUT << GPIO_PMD_PMD4_Pos);		// P34  OK P_DL4_R 
	P3->PMD = (P3->PMD & (~GPIO_PMD_PMD5_Msk)) | (GPIO_PMD_OUTPUT << GPIO_PMD_PMD5_Pos);		// P35  OK P_DL4_G 
	P3->PMD = (P3->PMD & (~GPIO_PMD_PMD6_Msk)) | (GPIO_PMD_OUTPUT << GPIO_PMD_PMD6_Pos);		// P36  OK P_DL5_G 
    P3->PMD = (P3->PMD & (~GPIO_PMD_PMD7_Msk)) | (GPIO_PMD_OUTPUT << GPIO_PMD_PMD7_Pos);		// P37  OK P_DL6_R 
	
	P4->PMD = (P4->PMD & (~GPIO_PMD_PMD0_Msk)) | (GPIO_PMD_OUTPUT << GPIO_PMD_PMD0_Pos);	// P40  OK P_DL7_G	
	P4->PMD = (P4->PMD & (~GPIO_PMD_PMD1_Msk)) | (GPIO_PMD_OUTPUT << GPIO_PMD_PMD1_Pos);	// P41  OK P_LED_WHITE_1  
	P4->PMD = (P4->PMD & (~GPIO_PMD_PMD2_Msk)) | (GPIO_PMD_OUTPUT << GPIO_PMD_PMD2_Pos);	// P42  OK P_NC_P42  P_I_SW2
	P4->PMD = (P4->PMD & (~GPIO_PMD_PMD3_Msk)) | (GPIO_PMD_OUTPUT << GPIO_PMD_PMD3_Pos);	// P43  OK P_DL5_R
	P4->PMD = (P4->PMD & (~GPIO_PMD_PMD4_Msk)) | (GPIO_PMD_OUTPUT << GPIO_PMD_PMD4_Pos);	// P44  OK P_O_S_RESET  
	P4->PMD = (P4->PMD & (~GPIO_PMD_PMD5_Msk)) | (GPIO_PMD_OUTPUT << GPIO_PMD_PMD5_Pos);	// P45  OK P_O_S_ENABLE 
	
    /*---------------------------------------------------------------------------------------------------------*/
    /* Init I/O Multi-function                                                                                 */
    /*---------------------------------------------------------------------------------------------------------*/
	//If the defines do not exist in your project, please refer to the related sys.h in the Header folder appended to the tool package.
    //If the defines do not exist in your project, please refer to the related sys.h in the Header folder appended to the tool package.


    /* ADC clock source is 22.1184MHz, set divider to 7, ADC clock is 22.1184/7 MHz */
    CLK->CLKDIV  = (CLK->CLKDIV & ~CLK_CLKDIV_ADC_N_Msk) | (((7) - 1) << CLK_CLKDIV_ADC_N_Pos);


	SYS->P0_MFP = 0x00000000;
//    SYS->P1_MFP = SYS_MFP_P10_AIN0 | SYS_MFP_P11_AIN1 | SYS_MFP_P12_AIN2 ;

   /* Disable the P1.0 - P1.3 digital input path to avoid the leakage current. */

    //If the defines do not exist in your project, please refer to the related sys.h in the Header folder appended to the tool package.
    SYS->P0_MFP = 0x00000000;
    SYS->P1_MFP = SYS_MFP_P12_AIN2 | SYS_MFP_P10_AIN0;

//    P1->OFFD = 0xF << GPIO_OFFD_OFFD_Pos;	// 1111    P10 P11 P12 P13 ADC Input
    P1->OFFD = 0x7 << GPIO_OFFD_OFFD_Pos;	// 0111    P10 P11 P12  ADC Input

	SYS->P2_MFP = SYS_MFP_P22_PWM2 ;
    SYS->P3_MFP = 0x00000000;
	SYS->P4_MFP =  SYS_MFP_P47_ICE_DAT | SYS_MFP_P46_ICE_CLK;

}




void UART0_Init()
{
    /*---------------------------------------------------------------------------------------------------------*/
    /* Init UART                                                                                               */
    /*---------------------------------------------------------------------------------------------------------*/
    /* Reset IP */
    SYS_ResetModule(UART0_RST);
    /* Configure UART0 and set UART0 Baudrate */
    UART_Open(UART0, 115200);
}


void I2C0_Close(void)
{
    /* Disable I2C0 interrupt and clear corresponding NVIC bit */
    I2C_DisableInt(I2C0);
    NVIC_DisableIRQ(I2C0_IRQn);

    /* Disable I2C0 and close I2C0 clock */
    I2C_Close(I2C0);
    CLK_DisableModuleClock(I2C0_MODULE);

}

/*---------------------------------------------------------------------------------------------------------*/
/*  Main Function                                                                                          */
/*---------------------------------------------------------------------------------------------------------*/
int32_t main(void)
{
    uint32_t i;
	unsigned char   rVCCinCheck;

   /* Unlock protected registers */
	//=====================================================================
	P22 = FALSE;
	P23 = FALSE;

    SYS_UnlockReg();
	MOTOR_OFF;
	
	P22 = FALSE;
	P_O_OUT1_OFF;
	P_O_OUT2_OFF;

	SF_PowerHoldCtrl(FALSE);
	//=====================================================================
	F_MOTOR_ON = FALSE;	   // ??? 
	//===============================
	
    /* Init System, IP clock and multi-function I/O */
    SYS_Init();

//	PWM_DisableOutput(PWMB, 1 << 0x02);
//	SYS->P2_MFP &= ~SYS_MFP_P22_Msk;
	P22 = FALSE;
	
	
	
    /* Init UART0 for printf */
//    UART0_Init();

    /* Lock protected registers */
    SYS_LockReg();

	rF_LevelMode = C_LEVEL_0;
	rF_MainMode =   C_WISTEC_MODE0;
 
 /*
        This sample code sets I2C bus clock to 100kHz. Then, Master accesses Slave with Byte Write
        and Byte Read operations, and check if the read data is equal to the programmed data.
    */
    /* Init I2C0 */
	//  I2C0_Init();
	//	PWM_DisableOutput(PWMB, 1 << 0x02);
	//	P22 = FALSE;
	
    /* Open Timer0 frequency to 0.5 Hz in periodic mode, and enable interrupt */
    /* Initial Timer1 default setting */
    TIMER_Open(TIMER0, TIMER_CONTINUOUS_MODE, 1);
     /* Configure Timer1 setting for external counter input and capture function */
    TIMER_SET_PRESCALE_VALUE(TIMER0, 0);
    TIMER_SET_CMP_VALUE(TIMER0, 0xFFFFFF);
    TIMER_EnableEventCounter(TIMER0, TIMER_COUNTER_RISING_EDGE);
    TIMER_EnableCapture(TIMER0, TIMER_CAPTURE_FREE_COUNTING_MODE, TIMER_CAPTURE_FALLING_EDGE);
    TIMER_EnableCaptureInt(TIMER0);


//    TIMER_Open(TIMER1, TIMER_CONTINUOUS_MODE, 1);
     /* Configure Timer1 setting for external counter input and capture function */
//    TIMER_SET_PRESCALE_VALUE(TIMER1, 0);
//    TIMER_SET_CMP_VALUE(TIMER1, 0xFFFFFF);
//    TIMER_EnableEventCounter(TIMER1, TIMER_COUNTER_FALLING_EDGE);
//    TIMER_EnableCapture(TIMER1, TIMER_CAPTURE_FREE_COUNTING_MODE, TIMER_CAPTURE_FALLING_EDGE);
//    TIMER_EnableCaptureInt(TIMER1);


//	TIMER0->TCMPR = __HXT;
//	TIMER0->TCSR = TIMER_TCSR_IE_Msk | TIMER_PERIODIC_MODE;
//    TIMER_SET_PRESCALE_VALUE(TIMER0, 0);


//	TIMER1->TCMPR = ((SystemCoreClock / 4) / 2);
//	TIMER1->TCSR = TIMER_TCSR_IE_Msk | TIMER_PERIODIC_MODE;
//	TIMER_SET_PRESCALE_VALUE(TIMER1, 3);

	TIMER_Open( TIMER1,TIMER_TCSR_IE_Msk | TIMER_PERIODIC_MODE,1000);				// Every 


    /* Open Timer2 frequency to 2 Hz in periodic mode, and enable interrupt */
	TIMER_Open( TIMER2,TIMER_TCSR_IE_Msk | TIMER_PERIODIC_MODE,2000 );				// Every 100msec  
//	TIMER_Open( TIMER2,TIMER_TCSR_IE_Msk | TIMER_PERIODIC_MODE,1100000 );			// Every 100msec  

//	TIMER2->TCMPR = ((__HIRC / 1) / (1840));
//    TIMER2->TCSR = TIMER_TCSR_IE_Msk | TIMER_PERIODIC_MODE;
//    TIMER_SET_PRESCALE_VALUE(TIMER2, 20);

    /* Open Timer3 frequency to 4 Hz in periodic mode, and enable interrupt */
//    TIMER3->TCMPR = ((__HIRC / 1) / (1840));
//    TIMER3->TCSR = TIMER_TCSR_IE_Msk | TIMER_PERIODIC_MODE;
//    TIMER_SET_PRESCALE_VALUE(TIMER3, 0);
	TIMER_Open( TIMER3,TIMER_TCSR_IE_Msk | TIMER_PERIODIC_MODE,1000);				// Every 100msec  

    /* Enable Timer0 ~ Timer3 NVIC */
    NVIC_EnableIRQ(TMR0_IRQn);
    NVIC_EnableIRQ(TMR1_IRQn);
    NVIC_EnableIRQ(TMR2_IRQn);
    NVIC_EnableIRQ(TMR3_IRQn);

    /* Clear Timer0 ~ Timer3 interrupt counts to 0 */
    g_au32TMRINTCount[0] = g_au32TMRINTCount[1] = g_au32TMRINTCount[2] = g_au32TMRINTCount[3] = 0;
//    u32InitCount = g_au32TMRINTCount[0];


	F_FirstPowerOn = TRUE;

	F_UpDATE_PWM = FALSE;
	r_SenceCnt = 19;
	MOTOR_OFF;								// Motor Off

	rPowerOffByKey = FALSE;
	rInterupt_MotorStatus = FALSE;
	rPowerOnCnt = 500;
    /* Start Timer0 ~ Timer3 counting */
    TIMER_Start(TIMER0);
    TIMER_Start(TIMER1);
//    TIMER_Start(TIMER2);
    TIMER_Start(TIMER3);

   /* Set the ADC operation mode as continuous scan, input mode as single-end and enable the ADC converter */
 //   ADC->ADCR = (ADC_ADCR_ADMD_CONTINUOUS | ADC_ADCR_DIFFEN_SINGLE_END | ADC_ADCR_ADEN_CONVERTER_ENABLE);
    /* Enable analog input channel 0, 1, 2 and 3 */
 //   ADC->ADCHER |= ((ADC->ADCHER & ~ADC_ADCHER_CHEN_Msk) | (0x7));
	
	ADC_InitContinuousScan();
 
	rVCCinCheck = 0;
	for ( i = 0 ;  i <  8 ;  i++)  
	{
		if ( IsVccPinEnable() )											//  VCC_IN
		{
			rVCCinCheck = 8;
			break;	
		}	
		else
		{
			if ( P_I_POWER )
			{
				Delay_1ms(5);  //1
				++rVCCinCheck;
			}
			else
			{
				rVCCinCheck = 0;
				break;	
			}
		}
	}
	if ( rVCCinCheck >  4 )	
		SF_PowerHoldCtrl(TRUE);
//	else
//		SF_PowerHoldCtrl(FALSE);

//	rBatStatus = 1;

	if ( IsVccPinEnable() )			F_USBConnected = TRUE;

	
	SF_POWER_ON();	
	F_FirstPowerOn = FALSE;
	F_UpDATE_PWM = TRUE;
	FOD_UpdateLevel = TRUE;
//	rBatStatus = 0;		

    TIMER_Start(TIMER2);

	F_UpDATE_PWM = FALSE;
//	g_u8TimeoutFlag = 0;
	F_TouchCheck = FALSE;
	r_SenceCnt = 19;
	F_MOTOR_ON = FALSE;	   // ??? 
	MOTOR_OFF;								// Motor Off


	rBatStatusCnt = 25;

	if ( 1) //IsVccPinEnable() )
	{	
		task_ADC2_ADCIN();
		i32ADCData[0] =  i32ConversionData[2];
		Delay_1ms(20);  //1
		task_ADC2_ADCIN();
		i32ADCData[1] =  i32ConversionData[2];
		Delay_1ms(20);  //1
		task_ADC2_ADCIN();
		i32ADCData[2] =  i32ConversionData[2];
		Delay_1ms(20);  //1
		task_ADC2_ADCIN();
		i32ADCData[3] =  i32ConversionData[2];
		Delay_1ms(20);  //1
		task_ADC2_ADCIN();
		i32ADCData[4] =  i32ConversionData[2];
		Delay_1ms(20);  //1
		task_ADC2_ADCIN();
		i32ADCData[5] =  i32ConversionData[2];
		Delay_1ms(20);  //1
		task_ADC2_ADCIN();
		i32ADCData[6] =  i32ConversionData[2];
		Delay_1ms(20);  //1
		task_ADC2_ADCIN();
		i32ADCData[7] =  i32ConversionData[2];
		
		Delay_1ms(20);  //1
		task_ADC2_ADCIN();
		i32ADCData[8] =  i32ConversionData[2];
		Delay_1ms(20);  //1
		task_ADC2_ADCIN();
		i32ADCData[9] =  i32ConversionData[2];


		for (i = 0 ; i < 10;  ++i) 
		{
			i32ADCResult += i32ADCData[i];
		}		
		i32ADCResult =  i32ADCResult / 10; 
	}
	
	if ( IsVccPinEnable() )
	{
		if ( i32ADCResult > ADC3P9 ) 			rBatStatus = 3;			
		else if ( i32ADCResult > ADC3P7 ) 		rBatStatus = 2;		
		else if ( i32ADCResult > ADC3P5 ) 		rBatStatus = 1;		
		else	rBatStatus = 1;
	}
	else
	{
	
	


		rBatStatus = 2;
	}	

	
	i32ADCPowerOnCnt = 4;
	F_LowBattery = FALSE;
	rF_OK_OLED = TRUE;
	r_8sec = 0;

	
	while(1)
    {

		if ( FM_LOOP_1MSEC )
		{
			FM_LOOP_1MSEC = FALSE;

			task_MotorCtrl();
		//	if ( F_ENABLE_WISTEC_MODE2 )
		//		SF_LED_Control_1msec();

			//========================================================
//			CLK_SysTickDelay(500);
			//========================================================
		}
		if ( FM_LOOP_10MSEC )
		{

			FM_LOOP_10MSEC = FALSE;
			wWaitCount = 10;
//			P_NC_P42 ^= TRUE;

			
			if ( F_FirstPowerOnSound  && !F_USBConnected )
			{
				if ( ( --F_FirstPowerOnSoundDelay == 0) /* && !IsVccPinEnable() */) 
				{
					F_FirstPowerOnSound = FALSE;
				//	SF_SoundOut(3+1);		
				}
			//	else 		
			//		F_FirstPowerOnSound = FALSE;
 				
			}
			else			
			{
				F_FirstPowerOnSound = FALSE;
			}
//			if ( IsVccPinEnable() )  	
			{
//				task_BatteryHandlerCtrl();
			}	
//			else
			{
				KEY_KeyScan();
				KEY_CheckAndExecute();
				Task_LED_Control();
			}
		}
		if ( FM_LOOP_1SEC )
		{
			FM_LOOP_1SEC = FALSE;
			wCount_1sec = 1000;
			Task_SOD_Display();

		}
    }
}
	#define		_MOTOR_SPEED_100		100
	#define		_MOTOR_SPEED_80		80
	#define		_MOTOR_SPEED_60		60
	#define		_MOTOR_SPEED_70		70



void	SF_MOTOR_POWERB_Mode(void)
{
		if ( g_MotorCnt_1m < 300 )    			MOTOR_ON;	
		else if ( g_MotorCnt_1m < 500 )   		MOTOR_OFF;
		else if ( g_MotorCnt_1m < 800 )			MOTOR_ON;
		else if ( g_MotorCnt_1m < 1000 )		MOTOR_OFF;


		else if ( g_MotorCnt_1m < 1300 )		MOTOR_ON;
		else if ( g_MotorCnt_1m < 1500 )		MOTOR_OFF;
		else if ( g_MotorCnt_1m < 1800 )		MOTOR_ON;
		else 									MOTOR_OFF;
}

void	SF_MOTOR_Lifting_Mode(void)
{
			if ( g_MotorCnt_1m < 300 )
			{
	//			if ( g_MotorCnt_1m % 100  <  60 )
					MOTOR_ON;
	//			else
	//				MOTOR_OFF;
			}				
			else 
			{
	//			if ( g_MotorCnt_1m % 100  <  40 )
	//				MOTOR_ON;
	//			else
					MOTOR_OFF;
			}				

}
void	SF_MOTOR_Intensive_Mode(void)
{
	if ( g_MotorCnt_1m < 480 )			MOTOR_ON;
	else if ( g_MotorCnt_1m < 600 )	    MOTOR_OFF;

	else if ( g_MotorCnt_1m < 1020 )	MOTOR_ON;
	else if ( g_MotorCnt_1m < 1200 )	MOTOR_OFF;
	else if ( g_MotorCnt_1m < 1560 )	MOTOR_ON;
	else 	MOTOR_OFF;
	
}	
//  300 200  300  200  300  200  300  200  300  200  => 2500 
// Mode
#define    MOTOR_MODE2_START		400
#define    MOTOR_MODE2_ON			400
#define    MOTOR_MODE2_OFF			100

void	SF_MOTOR_WISTEC_Mode2(void)  //mode2
{
		if ( g_MotorCnt_1m < MOTOR_MODE2_START )    			MOTOR_ON;	
		else if ( g_MotorCnt_1m < MOTOR_MODE2_START + MOTOR_MODE2_OFF )   		MOTOR_OFF;
		else if ( g_MotorCnt_1m < MOTOR_MODE2_START + MOTOR_MODE2_OFF + MOTOR_MODE2_ON )    		MOTOR_ON;	
		else if ( g_MotorCnt_1m < MOTOR_MODE2_START + MOTOR_MODE2_OFF + MOTOR_MODE2_ON + MOTOR_MODE2_OFF)    		MOTOR_OFF;	
		else if ( g_MotorCnt_1m < MOTOR_MODE2_START + MOTOR_MODE2_OFF + MOTOR_MODE2_ON + MOTOR_MODE2_OFF + MOTOR_MODE2_ON)    		MOTOR_ON;	
		else if ( g_MotorCnt_1m < MOTOR_MODE2_START + MOTOR_MODE2_OFF + MOTOR_MODE2_ON + MOTOR_MODE2_OFF + MOTOR_MODE2_ON + MOTOR_MODE2_OFF)    		MOTOR_OFF;	
		else if ( g_MotorCnt_1m < MOTOR_MODE2_START + MOTOR_MODE2_OFF + MOTOR_MODE2_ON + MOTOR_MODE2_OFF + MOTOR_MODE2_ON + MOTOR_MODE2_OFF + MOTOR_MODE2_ON)    		MOTOR_ON;	
		else if ( g_MotorCnt_1m < MOTOR_MODE2_START + MOTOR_MODE2_OFF + MOTOR_MODE2_ON + MOTOR_MODE2_OFF + MOTOR_MODE2_ON + MOTOR_MODE2_OFF + MOTOR_MODE2_ON +  MOTOR_MODE2_OFF)    		MOTOR_OFF;	
		else if ( g_MotorCnt_1m < MOTOR_MODE2_START + MOTOR_MODE2_OFF + MOTOR_MODE2_ON + MOTOR_MODE2_OFF + MOTOR_MODE2_ON + MOTOR_MODE2_OFF + MOTOR_MODE2_ON +  MOTOR_MODE2_OFF + MOTOR_MODE2_ON )    		MOTOR_ON;	
		else if ( g_MotorCnt_1m < MOTOR_MODE2_START + MOTOR_MODE2_OFF + MOTOR_MODE2_ON + MOTOR_MODE2_OFF + MOTOR_MODE2_ON + MOTOR_MODE2_OFF + MOTOR_MODE2_ON +  MOTOR_MODE2_OFF + MOTOR_MODE2_ON +  MOTOR_MODE2_OFF)    		MOTOR_OFF;	

		else 	MOTOR_OFF;
	
}
//  300 200  300  200  300  200  600  400   => 2500 
// Mode
#define    MOTOR_MODE1_START		300
#define    MOTOR_MODE1_ON			300
#define    MOTOR_MODE1_OFF			200

void	SF_MOTOR_WISTEC_Mode1(void)
{		// 0  - 1800 
		if ( g_MotorCnt_1m < MOTOR_MODE1_START )    			MOTOR_ON;	
		else if ( g_MotorCnt_1m < MOTOR_MODE1_START + MOTOR_MODE1_OFF )   		MOTOR_OFF;
		else if ( g_MotorCnt_1m < MOTOR_MODE1_START + MOTOR_MODE1_OFF + MOTOR_MODE1_ON )    		MOTOR_ON;	
		else if ( g_MotorCnt_1m < MOTOR_MODE1_START + MOTOR_MODE1_OFF + MOTOR_MODE1_ON + MOTOR_MODE1_OFF)    		MOTOR_OFF;	
		else if ( g_MotorCnt_1m < MOTOR_MODE1_START + MOTOR_MODE1_OFF + MOTOR_MODE1_ON + MOTOR_MODE1_OFF + MOTOR_MODE1_ON)    		MOTOR_ON;	
		else if ( g_MotorCnt_1m < MOTOR_MODE1_START + MOTOR_MODE1_OFF + MOTOR_MODE1_ON + MOTOR_MODE1_OFF + MOTOR_MODE1_ON + MOTOR_MODE1_OFF)    		MOTOR_OFF;	
		else if ( g_MotorCnt_1m < MOTOR_MODE1_START + MOTOR_MODE1_OFF + MOTOR_MODE1_ON + MOTOR_MODE1_OFF + MOTOR_MODE1_ON + MOTOR_MODE1_OFF + MOTOR_MODE1_ON*2)    		MOTOR_ON;	
		else if ( g_MotorCnt_1m < MOTOR_MODE1_START + MOTOR_MODE1_OFF + MOTOR_MODE1_ON + MOTOR_MODE1_OFF + MOTOR_MODE1_ON + MOTOR_MODE1_OFF + MOTOR_MODE1_ON*2 + + MOTOR_MODE1_OFF*2)    		MOTOR_OFF;	
		else 	MOTOR_OFF;

}	
void	SF_MOTOR_WISTEC_Mode0(void)
{
	if ( g_MotorCnt_1m < 1000 )				MOTOR_ON;
	else if ( g_MotorCnt_1m < 1250 )	    MOTOR_OFF;

	if ( g_MotorCnt_1m < 2250 )				MOTOR_ON;
	else if ( g_MotorCnt_1m < 2500 )	    MOTOR_OFF;

	else 								MOTOR_OFF;

}	


		
#define		TOUCH_CHECK_PERIOD_MS		1000
#define		TOUCH_CHECK_WINDOW_MS		50

// 2026-09-14: "연속 N샘플" 디바운스 → "윈도우(50샘플) 내 누적 카운트" 방식으로 변경.
//   실측 결과 마른손 접촉이 중간에 1~2샘플씩 끊기며 짧은 조각(3~5샘플)으로 나뉘는 경우가 많아,
//   연속성을 요구하는 방식으로는 놓치는 케이스가 많았음(main.c 히스토리 참고). 누적 카운트 방식은
//   윈도우 내에 Connect 샘플이 흩어져 있어도(연속이 아니어도) 총 개수만 채우면 인식되어 더 관대함.
#define		TOUCH_ON_MIN_COUNT			6		// 윈도우(최대 50샘플) 중 Connect 누적 개수가 이 값 이상이면 ON 확정
#define		TOUCH_OFF_MAX_COUNT			2		// 윈도우 중 Connect 누적 개수가 이 값 이하면 OFF 확정
													// (OFF_MAX_COUNT < 개수 < ON_MIN_COUNT 구간은 애매하므로 이전 F_MOTOR_ON 상태 유지 - 히스테리시스)

void	task_MotorCtrl(void)
{

	static uint32_t		rTouchADC;
	static uint32_t		rTouchConnectCnt;		// 현재 윈도우 내 Connect(터치) 샘플 누적 개수
	static uint32_t		rTouchCheckCnt;

	if ( IsVccPinEnable() || F_LowBattery || F_USBConnected || F_FirstPowerOn)
	{
		rInterupt_MotorStatus = FALSE;
		F_TouchCheckWindow = FALSE;
	}
	else
	{
		if ( --rPowerOnCnt == 0 )
		{
			rPowerOnCnt = 1;
			rInterupt_MotorStatus = TRUE;
		}

		if ( ++rTouchCheckCnt >= TOUCH_CHECK_PERIOD_MS )	rTouchCheckCnt = 0;

		F_TouchCheckWindow = ( rTouchCheckCnt < TOUCH_CHECK_WINDOW_MS ) ? TRUE : FALSE;

		if ( rTouchCheckCnt < TOUCH_CHECK_WINDOW_MS )
		{
			if ( rTouchCheckCnt == 0 )		rTouchConnectCnt = 0;		// 윈도우 시작 시 누적 카운트 리셋

			taskAI_ADC0_Touch();
			rTouchADC = i32ConversionData[0];

			/* 디버그용: 최근 TOUCH_ADC_LOG_SIZE(20)개 원시 ADC값을 순환버퍼에 기록 - Watch창에서 g_TouchADCLog 확인 */
			g_TouchADCLog[g_TouchADCLogIdx] = rTouchADC;
			if ( ++g_TouchADCLogIdx >= TOUCH_ADC_LOG_SIZE )	g_TouchADCLogIdx = 0;

			// Touch threshold: 4095 : 5V  =  x : 2.3V  ->  x = 1884
			// 2026-09-11 재조정: R13=8.2M(제너 장착 보드) 실측 기준 - 미터치 2458~2466(~3.0V), 터치 387~1379(~0.5~1.7V)
			//   → 그 사이 빈 구간(1379~2458) 중앙 근처로 임계값을 내려서 양쪽 마진 확보(각각 약 0.6~0.7V)
			// rTouchADC > threshold  : pad not touched  (TOUCH_ADC pulled up near 5V)
			// rTouchADC <= threshold : pad touched      (TOUCH_ADC pulled down toward GND)
			if ( rTouchADC <= (4095 * 2.3) / 5 )		// Connect - 연속 여부 무관하게 윈도우 내 누적만 카운트
			{
				rTouchConnectCnt++;
			}
		}
		else
		{
			// 윈도우(50샘플) 종료 시점에 누적 Connect 개수로 한 번만 판정
			if ( rTouchConnectCnt >= TOUCH_ON_MIN_COUNT )			F_MOTOR_ON = TRUE;
			else if ( rTouchConnectCnt <= TOUCH_OFF_MAX_COUNT )	F_MOTOR_ON = FALSE;
			// else: TOUCH_OFF_MAX_COUNT < rTouchConnectCnt < TOUCH_ON_MIN_COUNT 인 애매한 구간 - 이전 F_MOTOR_ON 상태 유지
		}
	}
}


/*---------------------------------------------------------------------------------------------------------*/
/* ???? ?? � ADC ??(22.1184MHz/7 � 3.16MHz) ??, 1?? ??? ?? ? �s ?? ??.               */
/* 3?? ?? 1??? + ??? ??? ??? ??. ??? ??? ??? ?? ?????.                     */
/*---------------------------------------------------------------------------------------------------------*/
/* ADC ???? ?? ??? � ?? ? ??? ??? ?? ?? ?? */
#define ADC_TIMEOUT_LOOP_COUNT   50000u
 
/* ?? ???? ??? � ? ?? ???? ADC ???? ?? ???? ?? ???? */
#define ADC_TIMEOUT_RECOVER_CNT  10u
 
/* Low Battery ??? ???? ??? ?? */
#define ADC_AVG_WINDOW           10
 
/* Low Battery ???? ??? ?? ??? ?? ?? (10ms * 200 = 2?) */
#define LOWBAT_CONFIRM_CNT       200
 
/* Low Battery ?? ? ??? ?? ?? ? ?? Power-Off ??(20? = 10ms*1000) */
#define LOWBAT_BEEP_PERIOD       200
#define LOWBAT_POWEROFF_CNT      1000
/* ??? ?? ??? � ?? ?? ??? (????? ?? ???? ??) */
static uint32_t gADCTimeoutCnt[3] = {0, 0, 0};
/*---------------------------------------------------------------------------------------------------------*/
/* ADC                                                                                              */
/*---------------------------------------------------------------------------------------------------------*/
void ADC_InitContinuousScan(void)
{
  ADC->ADCR = (ADC_ADCR_ADMD_CONTINUOUS | ADC_ADCR_DIFFEN_SINGLE_END | ADC_ADCR_ADEN_CONVERTER_ENABLE);
    ADC->ADCHER = (ADC->ADCHER & ~ADC_ADCHER_CHEN_Msk) | 0x7;   /* ?? 0,1,2 ??? */
    ADC->ADSR = ADC_ADSR_ADF_Msk;
    ADC->ADCR |= ADC_ADCR_ADST_Msk;                              /* ?? ??, ?? ?? ?? */
 
    gADCTimeoutCnt[0] = gADCTimeoutCnt[1] = gADCTimeoutCnt[2] = 0;
}


/*---------------------------------------------------------------------------------------------------------*/
/* ?? ?? ?? ?? � ???? ??                                                                       */
/* ???: TRUE = ?? ??, FALSE = ????(? ?? ? ?, ?? ? ??)                                   */
/*---------------------------------------------------------------------------------------------------------*/
static uint8_t ADC_ReadChannel(uint8_t ch, int32_t *pResult)
{
    uint32_t timeout = ADC_TIMEOUT_LOOP_COUNT;
 
    while (!(ADC->ADDR[ch] & ADC_ADDR_VALID_Msk))
    {
        if (--timeout == 0)
        {
            gADCTimeoutCnt[ch]++;
 
            /* ?? ????? ???? ??? ADC ?? ???? (?? ??) */
            if (gADCTimeoutCnt[ch] >= ADC_TIMEOUT_RECOVER_CNT)
            {
                ADC_InitContinuousScan();
            }
            return FALSE;
        }
    }
 
    *pResult = (ADC->ADDR[ch] & ADC_ADDR_RSLT_Msk) >> ADC_ADDR_RSLT_Pos;
    gADCTimeoutCnt[ch] = 0;
    return TRUE;
}

/*---------------------------------------------------------------------------------------------------------*/
/* 채널0 = Touch (P1.0)                                                                                     */
/* R13(8.2M) 등 소스 임피던스가 매우 높아서, 연속스캔 라운드로빈(채널0/1/2 순환) 상태로는 ADC의 S/H       */
/* 커패시터가 짧은 acquisition time 안에 실제 전압까지 충전되지 못해 값이 낮게 읽히는 문제가 있음.        */
/* (RC 시정수 ≈ 8.21MΩ×10pF ≈ 82µs 인데, 라운드로빈 1회 변환시간은 수 µs 수준이라 정착이 안 됨)            */
/* → 채널0만 분리하여 싱글모드로 전환 + 사전 대기(R13 경로로 핀을 미리 충전) + 동일채널 연속 재변환으로   */
/*   S/H가 실제 전압쪽으로 수렴하도록 한 뒤 마지막 값만 채택하고, 채널1/2용 연속스캔 모드로 복귀시킨다.    */
/*---------------------------------------------------------------------------------------------------------*/
#define TOUCH_ADC_PRECHARGE_US      200u    /* ADST 전 대기(µs) - R13 8.2M 경로로 핀 커패시턴스 사전 충전 */
                                             /* 2026-09-11: 200→2000µs 진단 결과 전압이 0.1V만 상승 → 정착시간 문제 아님(원인은 실제 누설 경로), 200µs로 원복 */
#define TOUCH_ADC_SETTLE_READS      4u      /* 동일 채널(0) 연속 재변환 횟수 - 마지막 값만 채택 */

void taskAI_ADC0_Touch(void)
{
    int32_t  result = i32ConversionData[0];   /* 타임아웃 시 이전 값 유지 */
    uint8_t  i;
    uint32_t timeout;

    /* 채널0만 단발(Single) 모드로 전환 - 채널1/2가 끼어들어 S/H를 다른 전압으로 끌고가지 않게 함 */
    ADC->ADCR   = (ADC_ADCR_ADMD_SINGLE | ADC_ADCR_DIFFEN_SINGLE_END | ADC_ADCR_ADEN_CONVERTER_ENABLE);
    ADC->ADCHER = (ADC->ADCHER & ~ADC_ADCHER_CHEN_Msk) | 0x1;

    CLK_SysTickDelay(TOUCH_ADC_PRECHARGE_US);  /* R13 경로로 핀이 실제 전압 근처까지 사전 충전되도록 대기 */

    for (i = 0; i < TOUCH_ADC_SETTLE_READS; i++)
    {
        ADC->ADSR = ADC_ADSR_ADF_Msk;
        ADC->ADCR |= ADC_ADCR_ADST_Msk;

        timeout = ADC_TIMEOUT_LOOP_COUNT;
        while (!(ADC->ADSR & ADC_ADSR_ADF_Msk))
        {
            if (--timeout == 0)
            {
                gADCTimeoutCnt[0]++;
                if (gADCTimeoutCnt[0] >= ADC_TIMEOUT_RECOVER_CNT)  ADC_InitContinuousScan();
                return;                         /* 이번 샘플은 포기 - i32ConversionData[0]는 이전 값 유지 */
            }
        }
        result = (ADC->ADDR[0] & ADC_ADDR_RSLT_Msk) >> ADC_ADDR_RSLT_Pos;
        /* 같은 채널을 바로 재변환 - S/H가 다른 채널로 끌려가지 않으므로 매 회 실제 전압쪽으로 수렴 */
    }

    i32ConversionData[0] = result;              /* 가장 수렴된 마지막 값만 채택 */
    gADCTimeoutCnt[0] = 0;

    ADC_InitContinuousScan();                    /* 채널1(TempSensor)/채널2(ADC_IN)용으로 연속스캔 복귀 */
}

/*---------------------------------------------------------------------------------------------------------*/
/* ??1 � TempSensor                                                                                        */
/*---------------------------------------------------------------------------------------------------------*/
void taskAI_ADC1_TempSensor(void)
{
    int32_t result;
    if (ADC_ReadChannel(1, &result))
    {
        i32ConversionData[1] = result;
    }
}

/*---------------------------------------------------------------------------------------------------------*/
/* ??2 � ADC_IN (??? ??)                                                                              */
/*---------------------------------------------------------------------------------------------------------*/
void taskAI_ADC2_ADCIN(void)
{
    int32_t result;
    if (ADC_ReadChannel(2, &result))
    {
        i32ConversionData[2] = result;
    }
}

void	task_ADC0_Touch(void)		// Touch 
{

            /* clear the A/D interrupt flag for safe */
            ADC->ADSR = ADC_ADSR_ADF_Msk;

            /* start A/D conversion */
            ADC->ADCR |= ADC_ADCR_ADST_Msk;

            /* Wait conversion done */
            while(!((ADC->ADSR & ADC_ADSR_ADF_Msk) >> ADC_ADSR_ADF_Pos));

            i32ConversionData[0] = (ADC->ADDR[(0)] & ADC_ADDR_RSLT_Msk) >> ADC_ADDR_RSLT_Pos;
			return;	



	/* Set the ADC operation mode as continuous scan, input mode as single-end and enable the ADC converter */
//    ADC->ADCR = (ADC_ADCR_ADMD_CONTINUOUS | ADC_ADCR_DIFFEN_SINGLE_END | ADC_ADCR_ADEN_CONVERTER_ENABLE);
    ADC->ADCR = (ADC_ADCR_ADMD_SINGLE | ADC_ADCR_DIFFEN_SINGLE_END | ADC_ADCR_ADEN_CONVERTER_ENABLE);
    /* Enable analog input channel 0, 1, 2 and 3 */
    ADC->ADCHER |= ((ADC->ADCHER & ~ADC_ADCHER_CHEN_Msk) | (0x7));
    /* clear the A/D interrupt flag for safe */
    ADC->ADSR = ADC_ADSR_ADF_Msk;

    /* start A/D conversion */
    ADC->ADCR |= ADC_ADCR_ADST_Msk;
    /* Wait conversion done */
    while(!((ADC->ADSR & ADC_ADSR_ADF_Msk) >> ADC_ADSR_ADF_Pos));

    /* Clear the ADC interrupt flag */
    ADC->ADSR = ADC_ADSR_ADF_Msk;
    i32ConversionData[0] = (ADC->ADDR[(0)] & ADC_ADDR_RSLT_Msk) >> ADC_ADDR_RSLT_Pos;

    /* Wait conversion done */
    while(!((ADC->ADSR & ADC_ADSR_ADF_Msk) >> ADC_ADSR_ADF_Pos));

    /* Stop A/D conversion */
    ADC->ADCR &= ~ADC_ADCR_ADST_Msk;
    i32ConversionData[0] = (ADC->ADDR[(0)] & ADC_ADDR_RSLT_Msk) >> ADC_ADDR_RSLT_Pos;

    /* Clear the ADC interrupt flag */
    ADC->ADSR = ADC_ADSR_ADF_Msk;
			 

}
void	task_ADC1_TempSensor(void)		// TempSensor 
{
//            /* Set the ADC operation mode as single-cycle, input mode as single-end and enable the ADC converter  */
//            ADC->ADCR = (ADC_ADCR_ADMD_SINGLE_CYCLE | ADC_ADCR_DIFFEN_SINGLE_END | ADC_ADCR_ADEN_CONVERTER_ENABLE);
//            /* Enable analog input channel 0, 1, 2 and 3 */
//            ADC->ADCHER |= ((ADC->ADCHER & ~ADC_ADCHER_CHEN_Msk) | (0x2));

//            /* clear the A/D interrupt flag for safe */
//            ADC->ADSR = ADC_ADSR_ADF_Msk;

//            /* start A/D conversion */
//            ADC->ADCR |= ADC_ADCR_ADST_Msk;

//            /* Wait conversion done */
//            while(!((ADC->ADSR & ADC_ADSR_ADF_Msk) >> ADC_ADSR_ADF_Pos));

//            i32ConversionData[1] = (ADC->ADDR[(1)] & ADC_ADDR_RSLT_Msk) >> ADC_ADDR_RSLT_Pos;

//			return;	


    /* clear the A/D interrupt flag for safe */
    ADC->ADSR = ADC_ADSR_ADF_Msk;

    /* start A/D conversion */
    ADC->ADCR |= ADC_ADCR_ADST_Msk;
    /* Wait conversion done */
    while(!((ADC->ADSR & ADC_ADSR_ADF_Msk) >> ADC_ADSR_ADF_Pos));

    /* Clear the ADC interrupt flag */
    ADC->ADSR = ADC_ADSR_ADF_Msk;
    i32ConversionData[1] = (ADC->ADDR[(1)] & ADC_ADDR_RSLT_Msk) >> ADC_ADDR_RSLT_Pos;

    /* Wait conversion done */
    while(!((ADC->ADSR & ADC_ADSR_ADF_Msk) >> ADC_ADSR_ADF_Pos));

    /* Stop A/D conversion */
    ADC->ADCR &= ~ADC_ADCR_ADST_Msk;
    i32ConversionData[1] = (ADC->ADDR[(1)] & ADC_ADDR_RSLT_Msk) >> ADC_ADDR_RSLT_Pos;

    /* Clear the ADC interrupt flag */
    ADC->ADSR = ADC_ADSR_ADF_Msk;

}


void	task_ADC2_ADCIN(void)
{
	taskAI_ADC2_ADCIN();
	return;
	
	
   /* clear the A/D interrupt flag for safe */
    ADC->ADSR = ADC_ADSR_ADF_Msk;

    /* start A/D conversion */
    ADC->ADCR |= ADC_ADCR_ADST_Msk;
    /* Wait conversion done */
    while(!((ADC->ADSR & ADC_ADSR_ADF_Msk) >> ADC_ADSR_ADF_Pos));

    /* Clear the ADC interrupt flag */
    ADC->ADSR = ADC_ADSR_ADF_Msk;
    i32ConversionData[2] = (ADC->ADDR[(2)] & ADC_ADDR_RSLT_Msk) >> ADC_ADDR_RSLT_Pos;

    /* Wait conversion done */
    while(!((ADC->ADSR & ADC_ADSR_ADF_Msk) >> ADC_ADSR_ADF_Pos));

    /* Stop A/D conversion */
    ADC->ADCR &= ~ADC_ADCR_ADST_Msk;
    i32ConversionData[2] = (ADC->ADDR[(2)] & ADC_ADDR_RSLT_Msk) >> ADC_ADDR_RSLT_Pos;

    /* Clear the ADC interrupt flag */
    ADC->ADSR = ADC_ADSR_ADF_Msk;
			 

}

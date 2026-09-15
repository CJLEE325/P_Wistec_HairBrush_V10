/**************************************************************************//**
 * @file     fuction.c
 * @version  V1.00
 * $Revision: 3 $
 * $Date: 14/01/28 11:44a $
 * @brief    NUC029 Series PWM Generator and Capture Timer Driver Sample Code
 *
 * @note
 * Copyright (C) 2014 Nuvoton Technology Corp. All rights reserved.
 *
 ******************************************************************************/
#include <stdio.h>
#include "NUC029xAN.h"

#include "sys_const.h"

#include "pub_ram.h"
#include "keyexec.h"



void	SF_ResetTimer(void);

void    SF_SoundOut(unsigned int  Number);
void	task_LowBatteryCtrl(void);
void NX11_SPI_Init(void);


unsigned char  IsVccPinEnable(void);

void	SF_LED_LEVEL(void);




void 	SF_LED_PortCtrl( unsigned char status);
void 	SF_InSide_IRBCtrl(unsigned char status);
void 	SF_OutSide_IRCtrl(unsigned char status);




void 	SF_StatusLEDCtrl(unsigned char status);
void 	SF_PowerHoldCtrl(unsigned char status);
void 	SF_P_20V_Ctrl(unsigned char status);
void 	SF_H_FreqCtrl(unsigned char status);

void	SF_POWER_OFF(void);


void	Task_PAN_Control(void);
void	Task_Feltier_Control(void);
void	Task_Door_Control(void);
void	Task_LED_Control(void);
void	Task_Feltier_Control (void);

void	Task_LEDBase_1msec(void);
void	SF_PWM_FREQ_ZeroLevel(void);


void	SF_LED_OnOff(unsigned char	Kind,  unsigned char	Status);
void	SF_USBFAN_OnOff(unsigned char	Status);
void	SF_BaseLED_CTRL(unsigned char Status);
void	SF_Read_ADC_Charge(void);
void	SF_Read_ADC_CDS(void);
void	SF_Read_ADC_Temp(void);

void	SF_Sound_Mode0(void);
void	SF_Sound_Mode1(void);
void	SF_Sound_Mode2(void);
void	SF_Sound_Mode3(void);


void	SF_PWM_ZeroLevel(void);
void	SF_PWM_MiddleFreq(unsigned char Status);
void	SF_PWM_HighFreq(unsigned char Status);
void	SF_PWM_FREQ_HighLevel_P24_P25(void);

void 	SF_InSide_IRBCtrl(unsigned char status);
void 	SF_OutSide_IRCtrl(unsigned char status);





extern	volatile uint32_t g_au32TMRINTCount[4] ;
extern	void Delay_1ms(uint32_t time);



extern	unsigned char	rK_Data ;
extern	unsigned char	rK_Last;
extern	uint32_t 		F_MOTOR_ON;
extern	uint32_t	rInterupt_MotorStatus;

 extern	void	task_ADC1_TempSensor(void);		// TempSensor 
 extern	void	task_ADC2_ADCIN(void);
 extern	void	taskAI_ADC2_ADCIN(void);
 extern	void	taskAI_ADC1_TempSensor(void);		// TempSensor 
 extern	 void  DEMO_LED_Color(unsigned char	seq ,unsigned char	color);
 extern	 void SF_Muti_LED(unsigned char position,  unsigned char color);


void	SF_LED_Control(void);
void	SF_LED_Control_OFF(void);
void	Task_SOD_Display(void);

// extern	 void    SF_PWMOUT1_ON();
// extern	 void    SF_PWMOUT1_OFF();
// extern	 void    SF_PWMOUT2_ON();
// extern	 void    SF_PWMOUT2_OFF();



 /**************************************************************************//**
 *
 *
 ******************************************************************************/
void	SF_POWER_ON(void)
{
//	SF_PowerHoldCtrl(FALSE);
	unsigned char  r_cnt;

	NX11_SPI_Init();
	Delay_1ms(10);


	F_USBConnected = FALSE;


	rF_demoLEDCnt = 150;
	rF_demoLEDSeq = 0;
	rF_demoLED = TRUE;

	
	r_cnt = 0;
	Delay_1ms(10);
	if ( IsVccPinEnable() )   ++r_cnt;
	Delay_1ms(10);
	if ( IsVccPinEnable() )   ++r_cnt;
	Delay_1ms(10);
	if ( IsVccPinEnable() )   ++r_cnt;
	Delay_1ms(10);
	if ( IsVccPinEnable() )   ++r_cnt;
	Delay_1ms(10);
	if ( IsVccPinEnable() )   ++r_cnt;
	Delay_1ms(10);
	if ( IsVccPinEnable() )   ++r_cnt;
	Delay_1ms(10);
	if ( IsVccPinEnable() )   ++r_cnt;
	Delay_1ms(10);
	if ( IsVccPinEnable() )   ++r_cnt;


	
	if ( r_cnt > 6 )    F_USBConnected = TRUE;
	else              	F_USBConnected = FALSE;
	
	if ( !F_USBConnected )
	{
		SF_SoundOut(SS_Mode1_WakeUp);
	//	Delay_1ms(200);
	}


//	for ( r_cnt = 0 ;  r_cnt < 9 ; r_cnt++ )  
//	{
//		DEMO_LED_Color(r_cnt,COLOR_RED);
//		Delay_1ms(100);
//	}
//	SF_LED_Control_OFF();

	
	if ( !IsVccPinEnable() )
	{
		F_FirstPowerOnSound = TRUE;
		F_FirstPowerOnSoundDelay = 100;
	}
	
	bKey_Repeat = TRUE;
	rK_Last = 1;      	/*                                  */
	rK_Data = 1;
	//
	SF_ResetTimer();
	r_OperatingSec = 0;

	
}


void	SF_POWER_OFF(void)
{
	
	F_USBConnected = FALSE;
	rInterupt_MotorStatus = FALSE;
	rF_MainMode  = C_WISTEC_MODE_OFF;
		rF_demoLED = FALSE;

//	rF_LevelMode = C_LEVEL_4;
//	PWM_DisableOutput(PWMB, 1 << 0x02);
//	SYS->P2_MFP &= ~SYS_MFP_P22_Msk;
//	P22 = FALSE;
	
	SF_LED_Control_OFF();
	
	P_LED_WHITE_1 = TRUE;
	P_LED_WHITE_2 = TRUE;
	P_LED_WHITE_3 = TRUE;
//	PWM_DisableOutput(PWMB, 1 << 0x00);
//	P24 = FALSE;

	if ( rPowerOffByKey)
	{		
		SF_SoundOut(SS_PowerOff);
		Delay_1ms(200);
		Delay_1ms(200);
		Delay_1ms(200);
		Delay_1ms(200);
		Delay_1ms(200);
		Delay_1ms(200);
		Delay_1ms(200);
		Delay_1ms(200);
/*
		Delay_1ms(200);
		Delay_1ms(200);
		Delay_1ms(200);
		Delay_1ms(200);
		Delay_1ms(200);
		Delay_1ms(200);
		Delay_1ms(200);
		Delay_1ms(200);
*/
	}

	SF_PowerHoldCtrl(FALSE);

	Delay_1ms(200);
	Delay_1ms(200);
	Delay_1ms(200);
	Delay_1ms(200);
	Delay_1ms(200);
	Delay_1ms(200);
	Delay_1ms(200);
	Delay_1ms(200);

	rPowerOffByKey = FALSE;

	
	
	
//	while(1)
//	{
//
//		if (  IsVccPinEnable() )											//  VCC_IN
//		{
//			break;
//		}
//	};
//	SF_POWER_ON();


}

void	SF_ResetTimer(void)
{
	r_OperatingSec = 01;
	r_OperatingMin = 5;
}

void	task_BatteryHandlerCtrl(void)
{


}





unsigned char  IsVccPinEnable()
{
	if ( P_I_VCCIN )    return   TRUE; 
	else			 	return   FALSE;	
	
}




void	SF_Read_ADC_Charge()
{
}



//++++++++++++++++++++++++++++++++++++++++++++
/**************************************************************************//**
*  
*  Output	:  r_OutPanDuty
******************************************************************************/

void	Task_SOD_Display(void)
{
	if ( F_USBConnected )  return;

	if ( ++	r_8sec >= 4 )
	{
		r_8sec = 0;

		 if ( !F_LowBattery )
		 {
			if ( rF_MainMode == C_WISTEC_MODE0 ) 	SF_SoundOut(SS_Mode1_Beep);
			else if ( rF_MainMode == C_WISTEC_MODE1 ) 	SF_SoundOut(SS_Mode2_Beep);
			else if ( rF_MainMode == C_WISTEC_MODE2 ) 	SF_SoundOut(SS_Mode3_Beep);
		 }
	}
	
	if ( -- r_OperatingSec >  59 ) 
	{
		r_OperatingSec = 59;

		// 
		if ( !F_LowBattery )
		{
	//		if ( r_OperatingMin == 7 )			SF_SoundOut(8+1);    // 7 min  
	//		else if ( r_OperatingMin == 4 )		SF_SoundOut(11+1);
	//		else if ( r_OperatingMin == 3 )		SF_SoundOut(12+1);
	//		else if ( r_OperatingMin == 2 )		SF_SoundOut(13+1);
	//		else if ( r_OperatingMin == 1 )		SF_SoundOut(14+1);
		}
		
		if ( --r_OperatingMin > 5 )
		{
			// Power Off
			SF_ResetTimer();			
		
			rInterupt_MotorStatus = FALSE;
			SF_SoundOut(SS_PowerOff);
			Delay_1ms(2000);
			SF_POWER_OFF();
		}
		else
		{

		}
	}
	else
	{
		
		
	}
	

}


//void	SF_HEXtoDEC(void)
//{
//	rDecValue[0] =  i32ADCResult / 1000;
//	rDecValue[1] =  (i32ADCResult - ( rDecValue[0] * 1000 )) / 100;
//	rDecValue[2] =  (i32ADCResult - ( rDecValue[0] * 1000 + rDecValue[1] * 100 )) / 10;
//	rDecValue[3] =  (i32ADCResult - ( rDecValue[0] * 1000 + rDecValue[1] * 100 + rDecValue[2] * 10)) ;
//}	


uint32_t  	gLowBat_mainCnt;
#define		CHANGING_CHATTERING  	25

//====================================================================
#define   Option_Test1   1
void	Task_LED_Control(void)
{	// Every 10msec
	static	unsigned char	Cnt_PeriodCharging= 10;
	static 	uint32_t 		rLowCnt = 200;
	static	unsigned char	Cnt_LEDFlikinging= 100;
	static	unsigned char   rUSB_Chattering = 20;
		unsigned char	i;

	// Must Check Chattering for P30 P13 PORT 

	if ( IsVccPinEnable() )											//  VCC_IN
	{
		//================================================================
		if ( !F_USBConnected  )
		{
			F_LowBattery = FALSE;

//			F_UpdateOSD= TRUE;
			SF_ResetTimer();			
			F_USBConnected = TRUE;
			rF_demoLED = FALSE;

			SF_LED_Control_OFF();
		}


		if ( --Cnt_PeriodCharging  ) 	return;
		Cnt_PeriodCharging = 10;	
	
		// Every 100msec
		task_ADC2_ADCIN();

		i32ADCData[0] =  i32ADCData[1];
		i32ADCData[1] =  i32ADCData[2];
		i32ADCData[2] =  i32ADCData[3];
		i32ADCData[3] =  i32ADCData[4];
		i32ADCData[4] =  i32ADCData[5];
		i32ADCData[5] =  i32ADCData[6];
		i32ADCData[6] =  i32ADCData[7];
		i32ADCData[7] =  i32ADCData[8];
		i32ADCData[8] =  i32ADCData[9];
		i32ADCData[9] =  i32ConversionData[2];

		i32ADCResult = 0;
		for (i = 0 ; i < 10;  ++i) 
		{
			i32ADCResult += i32ADCData[i];
		}		
		i32ADCResult =  i32ADCResult / 10; 		
		
		if ( P_I_VBAT ) 
		{
			if ( --rBatStatusCnt == 0 )  	
			{	
				rBatStatusCnt = CHANGING_CHATTERING;
				rBatStatus = 4;		
			}
		}
		else
		{
			if ( rBatStatus != 4  &&  ++rBatStatusCnt == 30)
			{
				rBatStatusCnt = CHANGING_CHATTERING;

				if ( i32ADCResult > ADC4P0 ) 		// <20240904  ADC4P0 -> ADC3P9  // <<260608>    ADC4P1  ->  ADC4P0			
				{
				//	if ( --rBatStatusCnt )
				//	{
				//		if ( rBatStatusCnt > CHANGING_CHATTERING )	rBatStatusCnt = CHANGING_CHATTERING;
				//	}
				//	else
				//	{
				//		rBatStatusCnt = CHANGING_CHATTERING;
						if ( rBatStatus > 3 )      rBatStatus = 4;	
						else                       rBatStatus = 3;		
				//	}
				}
				else if ( i32ADCResult > ADC3P7 ) 	//<20240904   ADC3P8 ->ADC3P7    3.7V - 4.1V		
				{
				//	if ( --rBatStatusCnt )
				//	{
				//		if ( rBatStatusCnt > CHANGING_CHATTERING )	rBatStatusCnt = CHANGING_CHATTERING;
				//	}
				//	else
				//	{
				//		rBatStatusCnt = CHANGING_CHATTERING;
						if ( rBatStatus > 2 )      rBatStatus = 3;	
						else                       rBatStatus = 2;		
				//	}
				}
				else  		
				{
				//	if ( --rBatStatusCnt )
				//	{
				//		if ( rBatStatusCnt > CHANGING_CHATTERING )	rBatStatusCnt = CHANGING_CHATTERING;
				//	}
				//	else
				//	{
				//		rBatStatusCnt = CHANGING_CHATTERING;
						if ( rBatStatus > 1 )      rBatStatus = 2; 
						else  						rBatStatus = 1;		
				//	}
				}
			}
		}
		if ( ++rUSB_Chattering >= 20 ) 		rUSB_Chattering = 0;

		
		if ( rBatStatus == 1 )
		{
			if( rUSB_Chattering == 0 )
			{
				P_LED_WHITE_1 = FALSE;
				P_LED_WHITE_2 = TRUE;
				P_LED_WHITE_3 = TRUE;
			}
			if( rUSB_Chattering == 10 )
			{
				P_LED_WHITE_1 = TRUE;
				P_LED_WHITE_2 = TRUE;
				P_LED_WHITE_3 = TRUE;
			}
		}

		else if ( rBatStatus == 2 )
		{
			if( rUSB_Chattering == 0 )
			{
				P_LED_WHITE_1 = FALSE;
				P_LED_WHITE_2 = FALSE;
				P_LED_WHITE_3 = TRUE;
				
			}
			if( rUSB_Chattering == 10 )
			{
				P_LED_WHITE_1 = FALSE;
				P_LED_WHITE_2 = TRUE;
				P_LED_WHITE_3 = TRUE;
			}
		}
		
		else if ( rBatStatus == 3 )
		{
			if( rUSB_Chattering == 0 )
			{
				P_LED_WHITE_1 = FALSE;
				P_LED_WHITE_2 = FALSE;
				P_LED_WHITE_3 = FALSE;
				
			}
			if( rUSB_Chattering == 10 )
			{
				P_LED_WHITE_1 = FALSE;
				P_LED_WHITE_2 = FALSE;
				P_LED_WHITE_3 = TRUE;
				
			}
		}
		
		else if ( rBatStatus == 4 )
		{
			P_LED_WHITE_1 = FALSE;
			P_LED_WHITE_2 = FALSE;
			P_LED_WHITE_3 = FALSE;
		}
	}	
	else
	{		
		if ( F_USBConnected  )
		{
			F_USBConnected = FALSE;
			SF_ResetTimer();
			SF_POWER_OFF();
			return;
		}

		if ( !rF_demoLED )
		{
			if ( ++Cnt_LEDFlikinging > 100 )    Cnt_LEDFlikinging = 0;		// <20240904   100-> 200 -> 50

			if (  F_MOTOR_ON ) 					// <20240904
			{
				if ( Cnt_LEDFlikinging < 50 ) 					// <20240904
					SF_LED_Control_OFF();
				else
					SF_LED_Control();
			}
			else
				SF_LED_Control();
		}	
		SF_LED_LEVEL();

		if ( F_LowBattery )
		{
			-- gLowBat_mainCnt;
			if ( gLowBat_mainCnt == 0 )
			{
				F_USBConnected = FALSE;
				SF_ResetTimer();
				rPowerOffByKey = TRUE;
				SF_POWER_OFF();
			}
			if ( gLowBat_mainCnt % 200 == 0 )	
				SF_SoundOut(SS_LowBat);
			return;
		}

	//	if ( --rBatStatusCnt )		return;	
	//		rBatStatusCnt = 25;

		task_ADC2_ADCIN();
	//  Every 10msec
	// 5v : 4095 = 4.1V : 3100
		i32ADCData[0] =  i32ADCData[1];
		i32ADCData[1] =  i32ADCData[2];
		i32ADCData[2] =  i32ADCData[3];
		i32ADCData[3] =  i32ADCData[4];
		i32ADCData[4] =  i32ADCData[5];
		i32ADCData[5] =  i32ADCData[6];
		i32ADCData[6] =  i32ADCData[7];
		i32ADCData[7] =  i32ADCData[8];
		i32ADCData[8] =  i32ADCData[9];
		i32ADCData[9] =  i32ConversionData[2];
	
		i32ADCResult = 0;
	
		for (i = 0 ; i < 10;  ++i) 
		{
			i32ADCResult += i32ADCData[i];
		}		
		i32ADCResult =  i32ADCResult / 10; 
	

		if ( i32ADCResult > ADC3P1 ) 			
		{
			rLowCnt = 200;
			F_LowBattery = FALSE;

		}
		else
		{
			if ( --rLowCnt == 0  )  
			{
				F_LowBattery = TRUE;
				gLowBat_mainCnt  = 1000;			// 20Sec Count Setting
			}
		}
	}
}


void	SF_LED_Control_OFF(void)
{
	SF_Muti_LED(POSIOTION_DL1,COLOR_BLACK);
	SF_Muti_LED(POSIOTION_DL2,COLOR_BLACK);
	SF_Muti_LED(POSIOTION_DL3,COLOR_BLACK);
	SF_Muti_LED(POSIOTION_DL4,COLOR_BLACK);
	SF_Muti_LED(POSIOTION_DL5,COLOR_BLACK);
	SF_Muti_LED(POSIOTION_DL6,COLOR_BLACK);
	SF_Muti_LED(POSIOTION_DL7,COLOR_BLACK);
	SF_Muti_LED(POSIOTION_DL8,COLOR_BLACK);
	SF_Muti_LED(POSIOTION_DL9_DL10,COLOR_BLACK);
	SF_Muti_LED(POSIOTION_DL11_DL12,COLOR_BLACK);
}



void	SF_LED_Control_1msec(void)
{
	static	unsigned char		LED_duty = 6;
	if ( ++LED_duty > 5  )		LED_duty = 0;
	
	if  ( rF_MainMode == C_WISTEC_MODE2)
	{
		if ( LED_duty < 3)
		{
			SF_Muti_LED(POSIOTION_DL1,COLOR_BLACK);
			SF_Muti_LED(POSIOTION_DL2,COLOR_BLACK);
			SF_Muti_LED(POSIOTION_DL3,COLOR_BLACK);
			SF_Muti_LED(POSIOTION_DL4,COLOR_BLACK);
			SF_Muti_LED(POSIOTION_DL5,COLOR_BLACK);
			SF_Muti_LED(POSIOTION_DL6,COLOR_BLACK);
			SF_Muti_LED(POSIOTION_DL7,COLOR_BLACK);
			SF_Muti_LED(POSIOTION_DL8,COLOR_BLACK);
		}
		else
		{
			SF_Muti_LED(POSIOTION_DL1,COLOR_RED+COLOR_GREEN);
			SF_Muti_LED(POSIOTION_DL3,COLOR_RED+COLOR_GREEN);
			SF_Muti_LED(POSIOTION_DL5,COLOR_RED+COLOR_GREEN);
			SF_Muti_LED(POSIOTION_DL7,COLOR_RED+COLOR_GREEN);
			SF_Muti_LED(POSIOTION_DL2,COLOR_RED+COLOR_GREEN);
			SF_Muti_LED(POSIOTION_DL4,COLOR_RED+COLOR_GREEN);
			SF_Muti_LED(POSIOTION_DL6,COLOR_RED+COLOR_GREEN);
			SF_Muti_LED(POSIOTION_DL8,COLOR_RED+COLOR_GREEN);


		}				
	}		
}
void	SF_LED_Control(void)
{
	switch ( rF_MainMode)
	{
		case  C_WISTEC_MODE0:	
			F_ENABLE_WISTEC_MODE2 = FALSE;

			SF_Muti_LED(POSIOTION_DL1,COLOR_RED);
			SF_Muti_LED(POSIOTION_DL2,COLOR_RED);
			SF_Muti_LED(POSIOTION_DL3,COLOR_RED);
			SF_Muti_LED(POSIOTION_DL4,COLOR_RED);
			SF_Muti_LED(POSIOTION_DL5,COLOR_RED);
			SF_Muti_LED(POSIOTION_DL6,COLOR_RED);
			SF_Muti_LED(POSIOTION_DL7,COLOR_RED);
			SF_Muti_LED(POSIOTION_DL8,COLOR_RED);
			SF_Muti_LED(POSIOTION_DL9_DL10,COLOR_RED);
			SF_Muti_LED(POSIOTION_DL11_DL12,COLOR_RED);
		break;
		
		case  C_WISTEC_MODE1:	
			F_ENABLE_WISTEC_MODE2 = FALSE;

			SF_Muti_LED(POSIOTION_DL1,COLOR_GREEN);
			SF_Muti_LED(POSIOTION_DL2,COLOR_GREEN);
			SF_Muti_LED(POSIOTION_DL3,COLOR_GREEN);
			SF_Muti_LED(POSIOTION_DL4,COLOR_GREEN);
			SF_Muti_LED(POSIOTION_DL5,COLOR_GREEN);
			SF_Muti_LED(POSIOTION_DL6,COLOR_GREEN);
			SF_Muti_LED(POSIOTION_DL7,COLOR_GREEN);
			SF_Muti_LED(POSIOTION_DL8,COLOR_GREEN);
			SF_Muti_LED(POSIOTION_DL9_DL10,COLOR_BLACK);
			SF_Muti_LED(POSIOTION_DL11_DL12,COLOR_BLACK);
		break;
		
		case  C_WISTEC_MODE2:	


			if ( F_MOTOR_ON) //!LED_duty )
			{
				F_ENABLE_WISTEC_MODE2 = FALSE;
				SF_Muti_LED(POSIOTION_DL1,COLOR_RED+COLOR_GREEN);
				SF_Muti_LED(POSIOTION_DL3,COLOR_RED+COLOR_GREEN);
				SF_Muti_LED(POSIOTION_DL5,COLOR_RED+COLOR_GREEN);
				SF_Muti_LED(POSIOTION_DL7,COLOR_RED+COLOR_GREEN);
				SF_Muti_LED(POSIOTION_DL2,COLOR_RED+COLOR_GREEN);
				SF_Muti_LED(POSIOTION_DL4,COLOR_RED+COLOR_GREEN);
				SF_Muti_LED(POSIOTION_DL6,COLOR_RED+COLOR_GREEN);
				SF_Muti_LED(POSIOTION_DL8,COLOR_RED+COLOR_GREEN);
			}
			else
			{
				F_ENABLE_WISTEC_MODE2 = TRUE;
				SF_Muti_LED(POSIOTION_DL1,COLOR_RED+COLOR_GREEN);
				SF_Muti_LED(POSIOTION_DL3,COLOR_RED+COLOR_GREEN);
				SF_Muti_LED(POSIOTION_DL5,COLOR_RED+COLOR_GREEN);
				SF_Muti_LED(POSIOTION_DL7,COLOR_RED+COLOR_GREEN);
				SF_Muti_LED(POSIOTION_DL2,COLOR_RED+COLOR_GREEN);
				SF_Muti_LED(POSIOTION_DL4,COLOR_RED+COLOR_GREEN);
				SF_Muti_LED(POSIOTION_DL6,COLOR_RED+COLOR_GREEN);
				SF_Muti_LED(POSIOTION_DL8,COLOR_RED+COLOR_GREEN);
				
			}				
			SF_Muti_LED(POSIOTION_DL9_DL10,COLOR_BLACK);
			SF_Muti_LED(POSIOTION_DL11_DL12,COLOR_BLACK);

		break;

		default:

			SF_Muti_LED(POSIOTION_DL1,COLOR_BLACK);
			SF_Muti_LED(POSIOTION_DL2,COLOR_BLACK);
			SF_Muti_LED(POSIOTION_DL3,COLOR_BLACK);
			SF_Muti_LED(POSIOTION_DL4,COLOR_BLACK);
			SF_Muti_LED(POSIOTION_DL5,COLOR_BLACK);
			SF_Muti_LED(POSIOTION_DL6,COLOR_BLACK);
			SF_Muti_LED(POSIOTION_DL7,COLOR_BLACK);
			SF_Muti_LED(POSIOTION_DL8,COLOR_BLACK);
			SF_Muti_LED(POSIOTION_DL9_DL10,COLOR_BLACK);
			SF_Muti_LED(POSIOTION_DL11_DL12,COLOR_BLACK);

		break;
	}




}

void	SF_LED_LEVEL(void)
{
	switch ( rF_LevelMode)
	{
		case  C_LEVEL_2:	
			P_LED_WHITE_1 = FALSE;
			P_LED_WHITE_2 = FALSE;
			P_LED_WHITE_3 = FALSE;
			break;
		case  C_LEVEL_1:	
			P_LED_WHITE_1 = FALSE;
			P_LED_WHITE_2 = FALSE;
			P_LED_WHITE_3 = TRUE;
			break;
	
		case  C_LEVEL_0:	
			P_LED_WHITE_1 = FALSE;
			P_LED_WHITE_2 = TRUE;
			P_LED_WHITE_3 = TRUE;
			break;
	
		default:
			P_LED_WHITE_1 = TRUE;
			P_LED_WHITE_2 = TRUE;
			P_LED_WHITE_3 = TRUE;
			break;
	}
}



void	SF_PWM_Control()
{

}

void	SF_Heater_Control(uint32_t duty)
{
	PWM_EnableOutput(PWMA, 1 << 0x3);									// PWM3	P23  
	PWM_ConfigOutputChannel(PWMA, PWM_CH3, BUZZER_FREQ, 50);			// PWM3	P23  
	PWM_EnablePeriodInt(PWMA, PWM_CH3, PWM_PERIOD_INT_UNDERFLOW);		// PWM3	P23  
	PWM_Start(PWMA, 1 << 0x3);											// PWM3	P23  

	PWM_EnableOutput(PWMA, 1 << 0x2);									// PWM2	P22  
	PWM_ConfigOutputChannel(PWMA, PWM_CH2, BUZZER_FREQ, 50);			// PWM2	P22  
	PWM_EnablePeriodInt(PWMA, PWM_CH2, PWM_PERIOD_INT_UNDERFLOW);		// PWM2	P22  
	PWM_Start(PWMA, 1 << 0x2);								

}



void	SF_PWM_FREQ_ZeroLevel(void)
{
    PWM_DisableOutput(PWMB, 1 << 0x00);
	P24 = FALSE;
}
void	SF_PWM_HighLevel(void)
{

}






void 	SF_LED_PortCtrl( unsigned char status)
{

}



void 	SF_OutSide_IRCtrl(unsigned char status)
{
}


void 	SF_PowerHoldCtrl(unsigned char status)
{
	if ( status )	P_O_POWERHOLD = TRUE;
//	if ( 1 )	P_O_POWERHOLD = TRUE;
	else			P_O_POWERHOLD = FALSE;
}




void 	SF_StatusLEDCtrl(unsigned char status)
{
}

void    Test_delay(unsigned  int i)
{
	unsigned int    delay   ;
	
	for ( delay = 0 ;  delay == 60000; delay++)
	{
	}
	for ( delay = 0 ;  delay == 60000; delay++)
	{
	}
	for ( delay = 0 ;  delay == 60000; delay++)
	{
	}
	for ( delay = 0 ;  delay == 60000; delay++)
	{
	}
	for ( delay = 0 ;  delay == 60000; delay++)
	{
	}
	for ( delay = 0 ;  delay == 60000; delay++)
	{
	}
	for ( delay = 0 ;  delay == 60000; delay++)
	{
	}

	for ( delay = 0 ;  delay == 60000; delay++)
	{
	}

	for ( delay = 0 ;  delay == 60000; delay++)
	{
	}

	for ( delay = 0 ;  delay == 60000; delay++)
	{
	}

	
}

/* =====================================================================
* 
*===================================================================== */

#define SPI_EN_LOW()        P_O_S_ENABLE = FALSE
#define SPI_EN_HIGH()       P_O_S_ENABLE = TRUE
#define SPI_CLK_LOW()       P_O_S_CLK = FALSE
#define SPI_CLK_HIGH()      P_O_S_CLK = TRUE
#define SPI_DATA_LOW()      P_O_S_DATA = FALSE 
#define SPI_DATA_HIGH()     P_O_S_DATA = TRUE  

/*=====================================================================

**===================================================================== */
#define NX11_CMD_STOP               ((uint16_t)0U)
#define NX11_SOUND_CODE_MIN         ((uint16_t)1U)
#define NX11_SOUND_CODE_MAX         ((uint16_t)1023U)
#define NX11_VOLUME_CODE_BASE       ((uint16_t)1024U)
#define NX11_VOLUME_LEVEL_MAX       ((uint8_t)15U)
#define NX11_CMD_MAX                ((uint16_t)1039U)

#define NX11_EN_SETUP_DELAY_US      ((uint32_t)900U)
#define NX11_CLK_HALF_PERIOD_US     ((uint32_t)50U)
#define NX11_END_DELAY_US           ((uint32_t)10U)
#define NX11_POST_CMD_DELAY_US      ((uint32_t)100U)


	// P_O_S_DATA
	// P_O_S_CLK
	// P_O_S_RESET
	// P_O_S_ENABLE


/**

* @brief NX11 ?? ? ?? ?? ??

*/

void NX11_SPI_Init(void)
{

	// NX11 RESET
	P_O_S_RESET = TRUE;	
	CLK_SysTickDelay(500);
	P_O_S_RESET = FALSE;	
	CLK_SysTickDelay(500);
	P_O_S_RESET = TRUE;	
	CLK_SysTickDelay(500);

    SPI_EN_HIGH();
    SPI_CLK_HIGH();
    SPI_DATA_LOW();
}


void    SF_SoundOut(unsigned int  code)
{
	static unsigned char   i;
	static unsigned int mask;
	static unsigned int code_data;

	SPI_EN_LOW();
	CLK_SysTickDelay(900);
	/* 2. ??? ?? (16-bit, MSB First) */
    /* ???: ??? ?? ?? ??? ?? ?? */

    mask = 0x8000;
	code_data = code;
    for ( i = 0; i < 16; i++)
    {
        /* [CPHA=1 ?? ??]
         * 1. Leading Edge (Falling) : ??? ?? (Setup)
         * 2. Trailing Edge (Rising) : ??? ??? (Capture)
         */
        /* Step A: Clock Falling (Setup Edge) */
        SPI_CLK_LOW();
        /* Step B: Data Setup */

        if (code_data & mask) 
		{
            SPI_DATA_HIGH();
        } else 
		{
            SPI_DATA_LOW();
        }

        /* Step C: Setup Time Delay */
		CLK_SysTickDelay(50);
        /* Step D: Clock Rising (Sample Edge) */
        SPI_CLK_HIGH();
        /* Step E: Hold Time Delay */
		CLK_SysTickDelay(50);
        mask >>= 1;
    }
	CLK_SysTickDelay(10);
    SPI_EN_HIGH();  /* Chip Select ?? */
    SPI_DATA_LOW(); /* Data ?? ?? */
	CLK_SysTickDelay(100);
}

/**************************************************************************//**
*  
*  Output	:  
*****************************************************************************/
void SF_UpDown(unsigned char max,unsigned char min,uint32_t *data_ptr)
{
	if( FK_DataUp )
	{
		if( *data_ptr < max )		// <000728-0>
			++*data_ptr;
		else
			*data_ptr = max;		// <000728-0>
	}
	else
	{
		if( *data_ptr > min )		// <000728-0>
			--*data_ptr;
		else
			*data_ptr = min;		// <000728-0>
	}
}



/**************************************************************************//**
 * @file     osd_fnd.c
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


void	SF_LOCAL_TP1(void); 			/*  0x00                     */
void	SF_LOCAL_TP2(void); 			/*  0x01                     */
void	SF_LOCAL_TP3(void); 			/*  0x02                     */
//=========================================================================
extern	void	SF_Sound_Mode0(void);
extern	void	SF_Sound_Mode1(void);
extern	void	SF_Sound_Mode2(void);
extern	void	SF_Sound_Mode3(void);
extern	void	SF_PWM_ZeroLevel(void);
extern	void	SF_PWM_HighLevel(void);
extern	void SF_UpDown(unsigned char max,unsigned char min,uint32_t *data_ptr);
extern	void    SF_SoundOut(unsigned int  Number);
extern	void 	SF_PowerHoldCtrl(unsigned char status);
extern	unsigned char  IsVccPinEnable(void);
extern	void	SF_ResetTimer(void);


extern	void	SF_PWM_MiddleFreq(unsigned char Status);
extern	void	SF_PWM_HighFreq(unsigned char Status);
extern	void	SF_POWER_OFF(void);
extern	void 	SF_H_FreqCtrl(unsigned char status);
extern	void 	SF_StatusLEDCtrl(unsigned char status);
extern	void 	SF_P_20V_Ctrl(unsigned char status);


extern	void 	SF_H_PortCtrl(unsigned  mode , unsigned char status);
extern	void 	SF_LED_PortCtrl( unsigned char status);
extern	void 	Delay_1ms(uint32_t time);


extern	void 	SF_InSide_IRBCtrl(unsigned char status);
extern	void 	SF_OutSide_IRCtrl(unsigned char status);


//=========================================================================




/**************************************************************************//**
*  
*  Output	:  
*****************************************************************************/
void	SF_LOCAL_TP1(void) 				/* POWER KEY                     */
{
	static   unsigned char rChattering_Power = 20;	//  2sec set  
	if ( IsVccPinEnable()) 					return;

	if ( !bKey_Repeat  )		
	{	//  1st Key
		if ( IsVccPinEnable() || F_LowBattery) 		return;

	//	FOD_UpdateMode = TRUE;
		if (  rF_MainMode == C_WISTEC_MODE0 )
		{
			rF_MainMode = C_WISTEC_MODE1;
			SF_SoundOut(SS_Mode2_LiftStage);		
		}
		else if (  rF_MainMode == C_WISTEC_MODE1 )
		{
			rF_MainMode = C_WISTEC_MODE2; 
			SF_SoundOut(SS_Mode3_Infuse);		
		}
		else if (  rF_MainMode == C_WISTEC_MODE2 )
		{
			rF_MainMode = C_WISTEC_MODE0; 
			SF_SoundOut(SS_Mode1_WakeUp);		
		}

		
		F_FirstPowerOnSound = FALSE;
		r_8sec = 0;
		rF_LevelMode = C_LEVEL_0;		// <20240904
		Delay_1ms(50);
		F_UpDATE_PWM = TRUE;
		SF_ResetTimer();
	//	F_UpdateOSD= TRUE;
		FOD_UpdateMode = TRUE;
		FOD_UpdateLevel = TRUE;
			
			
			
		rChattering_Power = 10;
	
	}
	else
	{
	// 2 sec	
	//	if ( rChattering_Power  )	
	//	P_RIR_RED ^= TRUE;

		if ( rChattering_Power  )	
		{
			--rChattering_Power;
			if ( rChattering_Power == 0 ) 
			{
				rPowerOffByKey = TRUE;
				SF_POWER_OFF();
				F_FirstPowerOn = FALSE;
			}					
		}
	}
}
/**************************************************************************//**
*  
*  Output	:  
*****************************************************************************/

extern void NX11_SPI_Init(void);

void	SF_LOCAL_TP3(void) 					/*  LEVEL Down                  */
{
	if ( bKey_Repeat )			return;
	if ( IsVccPinEnable() || F_LowBattery) 		return;

	FOD_UpdateMode = TRUE;

//	NX11_SPI_Init();

	switch ( rF_MainMode)
	{
		case  C_WISTEC_MODE0:	
			rF_MainMode = C_WISTEC_MODE1;
	//		SF_SoundOut(1024);		
			SF_SoundOut(SS_Mode2_LiftStage);		
		break;
		
		case  C_WISTEC_MODE1:	
			rF_MainMode = C_WISTEC_MODE2; 
	//		F_UpDATE_PWM = TRUE;
			SF_SoundOut(SS_Mode3_Infuse);		
		break;
		
		case  C_WISTEC_MODE2:	
			rF_MainMode = C_WISTEC_MODE0; 
			SF_SoundOut(SS_Mode1_WakeUp);		
		break;

		default:
		break;
		
	}	
	F_FirstPowerOnSound = FALSE;
	r_8sec = 0;

	rF_LevelMode = C_LEVEL_0;		// <20240904
	Delay_1ms(50);
	F_UpDATE_PWM = TRUE;
	SF_ResetTimer();
//	F_UpdateOSD= TRUE;
	FOD_UpdateMode = TRUE;
	FOD_UpdateLevel = TRUE;

}
/**************************************************************************//**
*  
*  Output	:  
*****************************************************************************/


void	SF_LOCAL_TP2(void) 					/*  Level UP                   */
{
	if ( bKey_Repeat )						return;
	if ( IsVccPinEnable() || F_LowBattery) 		return;

//	NX11_SPI_Init();

	switch ( rF_LevelMode)
	{
		case  C_LEVEL_0:	
			rF_LevelMode = C_LEVEL_1; 
			SF_SoundOut(SS_VolumeMid);		
		break;
		
		case  C_LEVEL_1:	
			rF_LevelMode = C_LEVEL_2; 
			SF_SoundOut(SS_VolumeHigh);		
		break;
		
		case  C_LEVEL_2:	
			rF_LevelMode = C_LEVEL_0; 
			SF_SoundOut(SS_VolumeLow);		

		break;
		
		default:
			rF_LevelMode = C_LEVEL_0; 
			SF_SoundOut(SS_VolumeLow);		
		break;
	};
	F_FirstPowerOnSound = FALSE;
		FOD_UpdateMode = TRUE;
	FOD_UpdateLevel = TRUE;


}

/**************************************************************************//**
 * @file     adc.h
 * @version  V3.0
 * $Revision: 5 $
 * $Date: 15/05/06 3:20p $
 * @brief    NUC029 series ADC driver header file
 *
 * @note
 * Copyright (C) 2014 Nuvoton Technology Corp. All rights reserved.
 *****************************************************************************/
#ifndef __sys_const_H__
#define __sys_const_H__

// Port redefine 
//=================================================


//	P00	P_O_S_DATA		P10	P_I_TOUCHADC		P20	P_O_OUT1		P30	P_DL2_R				P40	P_DL7_G
//	P01	P_O_S_CLK		P11	P_NC_P11			P21	P_O_OUT2		P31	P_DL2_G          	P41	P_LED_WHITE_1
//	P02	P_I_POWER		P12	P_I_ADCIN			P22	P_O_PWM_OUT		P32	P_DL3_R          	P42	P_NC_P42
//	P03	P_I_VCCIN		P13	P_O_MOTORCTRL		P23	P_DL6_G			P33	P_DL3_G				P43	P_DL5_R
//	P04	P_LED_WHITE_2	P14	P_O_POWERHOLD		P24	P_DL7_R			P34	P_DL4_R				P44	P_O_S_RESET
//	P05	P_I_VBAT		P15	P_I_SW2				P25	P_DL8_R			P35	P_DL4_G				P45	P_O_S_ENABLE
//	P06	P_LED_WHITE_3	P16	P_DL1_R				P26	P_DL8_G			P36	P_DL5_G				P46	ICE_CLK
//	P07	P_DL11_DL12_R	P17	P_DL1_G				P27	P_DL9_DL10_R	P37	P_DL6_R				P47	ICP_DATA


#define	  P_O_S_DATA				P00		//OK
#define	  P_O_S_CLK					P01		//OK
#define	  P_I_POWER					P02		//OK
#define	  P_I_VCCIN					P03		//OK
#define	  P_LED_WHITE_2				P04		//OK
#define	  P_I_VBAT					P05		//OK
#define	  P_LED_WHITE_1				P06		//OK
#define	  P_DL11_DL12_R				P07		//OK


#define	  P_I_TOUCHADC				P10		//OK
#define	  P_NC_P11					P11		//OK ADC1
#define	  P_I_ADCIN					P12			//OK ADC2
#define	  P_O_MOTORCTRL				P13		//OK
#define	  P_O_POWERHOLD				P14			//OK 
#define	  P_I_SW2				P15			//OK
#define	  P_DL1_R				P16			//OK
#define	  P_DL1_G				P17			//OK

#define	  P_O_OUT1				P20		//OK
#define	  P_O_OUT2				P21		//OK
#define	  P_O_PWM_OUT			P22		//OK	
#define	  P_DL6_G				P42		//OK	
#define	  P_DL7_R				P24		//OK
#define	  P_DL8_R				P25		//OK
#define	  P_DL8_G				P26		//OK
#define	  P_DL9_DL10_R			P27		//OK

#define     MOTOR_OFF				P_O_MOTORCTRL 	= FALSE    		//
#define     MOTOR_ON				P_O_MOTORCTRL 	= TRUE    		//

#define	  P_O_OUT1_ON			P_O_OUT1 = TRUE		//OK
#define	  P_O_OUT1_OFF			P_O_OUT1 = FALSE		//OK

#define	  P_O_OUT2_ON			P_O_OUT2 = TRUE		//OK
#define	  P_O_OUT2_OFF			P_O_OUT2 = FALSE		//OK

#define	  P_O_OUT2				P21		//OK

#define	  P_DL2_R				P30			//OK
#define	  P_DL2_G				P31			//OK
#define	  P_DL3_R				P32			//OK
#define	  P_DL3_G				P33			//OK
#define	  P_DL4_R				P34			//OK SDA
#define	  P_DL4_G				P35			//OK SCL
#define	  P_DL5_G				P36			//OK
#define	  P_DL6_R				P37			//OK

#define	  P_DL7_G				P40			//OK	
#define	  P_LED_WHITE_3			P41			//OK
#define	  P_NC_P42				P42			//OK
#define	  P_DL5_R				P43			//OK 
#define	  P_O_S_RESET			P44			//OK 
#define	  P_O_S_ENABLE			P45			//OK
#define	  ICE_CLK				P46			//OK	
#define	  ICP_DATA				P47			//OK



//  ADC
#define	  C_ADC0_TOUCH			0
#define	  C_ADC1_TEMP			1
#define	  C_ADC2_ADC			2

#define		ADC4P1		(4095*4.1) / 5		// 
#define		ADC4P0		(4095*4.0) / 5		// 
#define		ADC3P9		(4095*3.9) / 5		// 
#define		ADC3P8		(4095*3.8) / 5		// 
#define		ADC3P7		(4095*3.7) / 5		// 
#define		ADC3P6		(4095*3.6) / 5		// 
#define		ADC3P55		(4095*3.55) / 5		// 
#define		ADC3P5		(4095*3.5) / 5		// 
#define		ADC3P4		(4095*3.4) / 5		// 
#define		ADC3P3		(4095*3.3) / 5		// 
#define		ADC3P2		(4095*3.2) / 5		// 

//#define		ADC3P1		(4095*3.1) / 5		// 
#define 	ADC3P1                  ((int32_t)((4095.0 * 3.1) / 5.0))   /* ˜ 2539 */
#define		ADC3P0		(4095*3.0) / 5		// 

#define		LOWBATADC	(4095*2.8) / 5	// 

// 4095 :  5   =  x  :   3.1


//=================================================


#define		C_POWERMODE_OFF			0
#define		C_POWERMODE_STANBY		1
#define		C_POWERMODE_ON			2

#define		C_WISTEC_MODE0		0
#define		C_WISTEC_MODE1		1
#define		C_WISTEC_MODE2		2
#define		C_WISTEC_MODE_OFF	3

#define		C_LEVEL_0			0
#define		C_LEVEL_1			1
#define		C_LEVEL_2			2
#define		C_LEVEL_3			3
#define		C_LEVEL_4			4
#define		C_LEVEL_5			5
#define		C_LEVEL_6			6
#define		C_LEVEL_7			7
#define		C_LEVEL_OFF			0xFF

//  rF_SubFreqMode = FREQ_MODE_H
#define		FREQ_MODE_L			0
#define		FREQ_MODE_H			1

// rF_SubFreqLevel = FREQLEVEL_MODE0
#define		FREQLEVEL_MODE0		0
#define		FREQLEVEL_MODE1		1
#define		FREQLEVEL_MODE2		2
#define		FREQLEVEL_MODE3		3
#define		FREQLEVEL_MODE4		4


#define		HIGHFREQ_MODE0		0
#define		HIGHFREQ_MODE1		1
#define		HIGHFREQ_MODE2		2
#define		HIGHFREQ_MODE3		3

#define		MIDDLEFREQ_MODE0		0
#define		MIDDLEFREQ_MODE1		1
#define		MIDDLEFREQ_MODE2		2
#define		MIDDLEFREQ_MODE3		3






#define		BUZZER_FREQ			4000
#define		VCCBUZZER_FREQ		4000
#define		WISTEC_FREQUENCY		400000		


#define	  C_Sterilization		0
#define	  C_ColdStorage			1
#define	  C_Mood				2
#define	  C_USBALL				3
#define	  C_USBOnOff			4
#define	  C_Lock				5
#define	  C_UV					6
#define	  C_BASE				7
#define	  C_TOPBOT				8
#define	  C_Mood_Type			9


#define	  C_BaseLEDCurrent		0
#define	  C_BaseLEDTarget		1
#define	  C_BaseLEDAuto			2



#define	  C_Mood_LOW			20
#define	  C_Mood_MIDDLE			50
#define	  C_Mood_HIGH			100
#define	  C_Mood_AUTO			0


#define	  C_Time_NoKeyStaus    100 * 10;

/* IIC define costant   */
#define RD              0x80            /* IIC Read flag!                 */
#define WR              0x00            /* IIC Write flag!                */
#define SUB             0x40            /* IIC with Subaddress!           */
#define NOSUB           0x00            /* IIC without subaddress!        */
#define IIC_ADDR            0x20            /* IIC with other address!        */
#define NOADDR          0x00            /* IIC without other address!     */
#define MORE16          0x10            /* IIC without other address!     */

#define Read			0
#define WRITE			1

#define				COLOR_BLACK		0
#define				COLOR_BLUE		1	
#define				COLOR_GREEN		2		
#define				COLOR_CYAN		3		
#define				COLOR_RED		4		
#define				COLOR_MAGETA	5	
#define				COLOR_YELLOW	6	
#define				COLOR_WHITE		7

#define		POSIOTION_DL1	0
#define		POSIOTION_DL2	1
#define		POSIOTION_DL3	2
#define		POSIOTION_DL4	3
#define		POSIOTION_DL5	4
#define		POSIOTION_DL6	5
#define		POSIOTION_DL7	6
#define		POSIOTION_DL8	7

#define		POSIOTION_DL9_DL10	8
#define		POSIOTION_DL11_DL12	9



//   code No	?? 
#define   SS_Mode1_WakeUp  		1     // V3-??1 ?? (Wake mode).wav
#define   SS_Mode1_Beep			2     // V3-??1 ?? Beep.wav

#define   SS_Mode2_LiftStage   	3
#define   SS_Mode2_Beep   		4

#define   SS_Mode3_Infuse   	5
#define   SS_Mode3_Beep   		6

#define   SS_LongPress_On   	7
#define   SS_PowerOff   		7

//#define   SS_LongPress_Off   	8


#define   SS_VolumeHigh   	9
#define   SS_VolumeLow   	10
#define   SS_VolumeMid   	11

#define   SS_LowBat   		12

//#define   SS_LEVEL0	   		14
//#define   SS_LEVEL1   		15
//#define   SS_LEVEL2   		16




#endif

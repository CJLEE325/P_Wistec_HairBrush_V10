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
#ifndef __pub_ram_H__
#define __pub_ram_H__


extern unsigned char      ErrorCnt;
extern unsigned char      SendCnt;

extern unsigned char   	  rII_Info;             /* IIC Information data!            */
//sbit                FII_Read;       7     /* IIC READ or WRITE Selection.     */
//sbit				  FII_SubAddr;    6     /* Sub-Address exist or not.        */
//sbit				  FII_OtherAddr;  5     /* 2Byte additional Addr. exist or not*/
//sbit				  FII_IData;      4     /* Idata pointer or XData pointer!  */
//sbit				  Number of bytes 3-0   /* Idata pointer or XData pointer!  */
/********************************************************************************/
extern unsigned char   		rII_Slave;          /* Slave Address                    */
extern unsigned char        rII_Sub;            /* Sub Address                      */
extern unsigned int        rII_AddrHiLo;       /* For VPC, DDP IIC protocol        */
extern unsigned char        rII_AddrHi;         /* For MSP3410D Address High        */
extern unsigned char        rII_AddrLo;         /* For MSP3410D Address Low         */
extern unsigned char        rII_Data[2];		/* For MSP3410D Address Low         */
extern unsigned char    	rII_NumberOfData;   /* Number of Data for read/Write    */
extern unsigned char*         rII_Pointer;        /* Pointer!                         */

extern unsigned char  r_8sec;

extern 	unsigned char	rF_demoLEDSeq,rF_demoLED,F_ENABLE_WISTEC_MODE2;
extern 	unsigned int  rF_demoLEDCnt;


extern	uint8_t rMainMode,rLevelStatus,rHotStaus,rCnt500msec,rCntAntiAging,rCntEqu ,rBatStatus,rBatStatusOld,rBatStatusCnt;

extern	unsigned char			F_FirstPowerOnSound;
extern	unsigned char			F_FirstPowerOnSoundDelay;
extern	uint8_t		rF_PowerLevel4;


extern	uint8_t 	FOD_UpdateBat;
extern	uint8_t 	FOD_UpdateMode; 
extern	uint8_t 	FOD_UpdateLevel;
extern	uint8_t 	FOD_UpdateClock;






extern	uint8_t    F_UpDATE_PWM;
extern	 uint32_t 		r_OperatingMin;
extern	 uint32_t 		r_OperatingSec;


extern	volatile uint32_t rCount_IIC;
extern	uint32_t		rF_MainMode,F_FirstPowerOn;
extern	uint8_t		rF_HeadMode,rF_HeadModeOld,F_UpdateOSD,F_LowBattery;
extern	uint32_t		rF_LevelMode;
extern	uint32_t		rF_SubFreqMode,rF_SubFreqLevel;

extern	uint32_t	rF_OperatingTime,rF_Stanby2Time;		// Max 11Min
extern	uint8_t		F_USBConnected;
extern	uint32_t	rF_FreqStatus;
extern	unsigned char	rStausAutoMode;

extern  uint8_t SSD1306_Buffer[1280];
extern  	unsigned char	rDecValue[4];



extern	uint8_t		rF_LED[10];
extern	uint8_t		rF_OK_OLED;

extern	uint8_t		rF_TimePan;					// 10msec

extern	uint16_t	rF_Time_NoKeyStaus;			// 10msec * Chattering
extern	uint16_t	rOD_TimeSetupDisplay;				// 

extern	uint32_t	rF_TimeColdStorage;		// 6 Hour / 20sec
extern	uint32_t	rF_TimeSterilization;		// 6 Hour / 20sec
extern	uint32_t	r_InnerPanDuty;
extern	uint32_t	r_OutPanDuty;

extern	uint32_t	rF_FeltierPowerOn;		// 6 Hour / 20sec
extern	uint32_t	rF_TimeFeltier;		// 6 Hour / 20sec
extern	uint32_t	r_FeltierDuty;

extern	uint32_t	rFeedBackFreq[2];


extern	unsigned char	bKey_Repeat,FK_DataUp,FM_LOOP_1SEC,rPowerOffByKey;
extern	uint32_t	rFND_SetTemp,rFND_CurrentTemp;

extern	int32_t  	i32ConversionData[4];
extern	int32_t  	i32ADCData[10];
extern	int32_t  	i32ADCResult;
extern	int32_t  	i32ADCPowerOnCnt;






#endif

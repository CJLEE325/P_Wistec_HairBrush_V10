/**************************************************************************//**
 * @file     pub_ram.c
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
  unsigned char      ErrorCnt;
  unsigned char      SendCnt;

  unsigned char   	  rII_Info;             /* IIC Information data!            */
//sbit                FII_Read;       7     /* IIC READ or WRITE Selection.     */
//sbit				  FII_SubAddr;    6     /* Sub-Address exist or not.        */
//sbit				  FII_OtherAddr;  5     /* 2Byte additional Addr. exist or not*/
//sbit				  FII_IData;      4     /* Idata pointer or XData pointer!  */
//sbit				  Number of bytes 3-0   /* Idata pointer or XData pointer!  */

 unsigned char   		rII_Slave;          /* Slave Address                    */
 unsigned char        rII_Sub;            /* Sub Address                      */
 unsigned int        rII_AddrHiLo;       /* For VPC, DDP IIC protocol        */
 unsigned char        rII_AddrHi;         /* For MSP3410D Address High        */
 unsigned char        rII_AddrLo;         /* For MSP3410D Address Low         */
 unsigned char        rII_Data[2];		/* For MSP3410D Address Low         */
 unsigned char    	rII_NumberOfData;   /* Number of Data for read/Write    */
 unsigned char*         rII_Pointer;        /* Pointer!                         */

unsigned char  r_8sec;

	unsigned char	rF_demoLEDSeq,rF_demoLED,F_ENABLE_WISTEC_MODE2;
	unsigned int  rF_demoLEDCnt;



uint32_t			rF_MainMode,F_FirstPowerOn;
unsigned char			F_FirstPowerOnSound;
unsigned char			F_FirstPowerOnSoundDelay;

uint32_t			rF_LevelMode;
uint32_t 		r_OperatingMin;
uint32_t 		r_OperatingSec;
uint8_t    F_UpDATE_PWM;


uint8_t rMainMode,rLevelStatus,rHotStaus,rCnt500msec,rCntAntiAging,rCntEqu,rBatStatus,rBatStatusOld,rBatStatusCnt;

	uint8_t 	FOD_UpdateBat;
	uint8_t 	FOD_UpdateMode; 
	uint8_t 	FOD_UpdateLevel;
	uint8_t 	FOD_UpdateClock;


uint8_t		rF_HeadMode,rF_HeadModeOld,F_UpdateOSD,F_LowBattery;
uint8_t		rF_PowerLevel4;
uint32_t		rF_SubFreqMode,rF_SubFreqLevel;
unsigned char	rStausAutoMode;


uint8_t		F_USBConnected;
uint32_t	rF_OperatingTime;		// Max 11Min
uint32_t	rF_Stanby2Time;		// Max 11Min
uint32_t	rF_FreqStatus;
uint8_t		rF_LED[10];
uint8_t		rF_OK_OLED;

uint8_t		rF_TimePan;					// 10msec

uint16_t		rF_Time_NoKeyStaus;				// 10msec * Chattering
uint16_t		rOD_TimeSetupDisplay;				// 
uint8_t SSD1306_Buffer[1280];

unsigned char	rDecValue[4];

uint32_t	rF_TimeColdStorage;		// 6 Hour / 20sec
uint32_t	rF_TimeSterilization;		// 6 Hour / 20sec
uint32_t	r_InnerPanDuty;
uint32_t	r_OutPanDuty;

uint32_t	rF_FeltierPowerOn;		// 6 Hour / 20sec
uint32_t	rF_TimeFeltier;		// 6 Hour / 20sec
uint32_t	r_FeltierDuty;

uint32_t	rFeedBackFreq[2];


unsigned char	bKey_Repeat,FK_DataUp,FM_LOOP_1SEC,rPowerOffByKey;

uint32_t	rFND_SetTemp,rFND_CurrentTemp;


//  ADC DATA
int32_t  	i32ConversionData[4];
int32_t  	i32ADCData[10];
int32_t  	i32ADCResult;
int32_t  	i32ADCPowerOnCnt;




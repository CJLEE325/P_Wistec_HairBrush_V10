/**************************************************************************//**
 * @file     Key.c
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



#define TS06_Addr 0xd2 >>1 // I2C Address, Reset Value: 0xD2(Read, Write=Read + 1)


//volatile uint8_t g_u8DeviceAddr;
volatile uint8_t g_au8TxData[3];
volatile uint8_t g_u8RxData;
volatile uint8_t g_u8DataLen;
volatile uint8_t g_u8EndFlag = 0;
unsigned char	bLocal_In;
unsigned char	rK_Data, rK_Back;
unsigned char	rK_Time;
unsigned char	bLocal_Ok;
unsigned char	bKey_DataUp;
unsigned char	rK_ChatCnt;
unsigned char	rK_Last;

void Task_InitTS06N(void);
//void I2C0_TS06_WRITE(unsigned char	Reg, unsigned char	data);
unsigned char Key_GetLocalKey(void);
void KEY_CheckAndExecute(void);
void I2C0_Init(void);


extern	void	SF_LOCAL_TP1(void); 			/*  0x00                     */
extern	void	SF_LOCAL_TP2(void); 			/*  0x01                     */
extern	void	SF_LOCAL_TP3(void); 			/*  0x02                     */


void delay_(unsigned int del_t)
{
	while(del_t--); // 24Mhz? = 41.66 nsec //1000=>41.67usec
}


void I2C0_Init(void)
{
  /* Reset I2C0 */
    SYS->IPRSTC2 |=  SYS_IPRSTC2_I2C0_RST_Msk;
    SYS->IPRSTC2 &= ~SYS_IPRSTC2_I2C0_RST_Msk;

    /* Enable I2C0 Controller */
    I2C0->I2CON |= I2C_I2CON_ENS1_Msk;

    /* I2C0 clock divider, I2C Bus Clock = PCLK / (4*125) = 100kHz */
//    I2C0->I2CLK = 222 - 1;
//    I2C0->I2CLK = 71 - 1;		// 315KHz
    I2C0->I2CLK = 10 - 1;		// 315KHz

    /* Get I2C0 Bus Clock */
//    printf("I2C clock %d Hz\n", (SystemCoreClock / (((I2C0->I2CLK) + 1) << 2)));

    /* Set I2C0 4 Slave Addresses */
    /* Slave Address : 0x15 */
    I2C0->I2CADDR0 = (I2C0->I2CADDR0 & ~I2C_I2CADDR_I2CADDR_Msk) | (0x15 << I2C_I2CADDR_I2CADDR_Pos);
    /* Slave Address : 0x35 */
    I2C0->I2CADDR1 = (I2C0->I2CADDR1 & ~I2C_I2CADDR_I2CADDR_Msk) | (0x35 << I2C_I2CADDR_I2CADDR_Pos);
    /* Slave Address : 0x55 */
    I2C0->I2CADDR2 = (I2C0->I2CADDR2 & ~I2C_I2CADDR_I2CADDR_Msk) | (0x55 << I2C_I2CADDR_I2CADDR_Pos);
    /* Slave Address : 0x75 */
    I2C0->I2CADDR3 = (I2C0->I2CADDR3 & ~I2C_I2CADDR_I2CADDR_Msk) | (0x75 << I2C_I2CADDR_I2CADDR_Pos);

    /* Enable I2C0 interrupt and set corresponding NVIC bit */
    I2C0->I2CON |= I2C_I2CON_EI_Msk;
    NVIC_EnableIRQ(I2C0_IRQn);
}



/*---------------------------------------------------------------------------------------------------------*/
/*  I2C Rx Callback Function                                                                               */
/*---------------------------------------------------------------------------------------------------------*/
//void I2C_MasterRx(uint32_t u32Status)
//{
//    if(u32Status == 0x08)                       /* START has been transmitted and prepare SLA+W */
//    {
//        I2C_SET_DATA(I2C0, (g_u8DeviceAddr << 1));    /* Write SLA+W to Register I2CDAT */
//        I2C_SET_CONTROL_REG(I2C0, I2C_I2CON_SI);
//    }
//    else if(u32Status == 0x18)                  /* SLA+W has been transmitted and ACK has been received */
//    {
//        I2C_SET_DATA(I2C0, g_au8TxData[g_u8DataLen++]);
//        I2C_SET_CONTROL_REG(I2C0, I2C_I2CON_SI);
//    }
//    else if(u32Status == 0x20)                  /* SLA+W has been transmitted and NACK has been received */
//    {
//        I2C_SET_CONTROL_REG(I2C0, I2C_I2CON_STA_STO_SI);
//    }
//    else if(u32Status == 0x28)                  /* DATA has been transmitted and ACK has been received */
//    {
//        if(g_u8DataLen != 2)
//        {
//            I2C_SET_DATA(I2C0, g_au8TxData[g_u8DataLen++]);
//            I2C_SET_CONTROL_REG(I2C0, I2C_I2CON_SI);
//        }
//        else
//        {
//            I2C_SET_CONTROL_REG(I2C0, I2C_I2CON_STA_SI);
//        }
//    }
//    else if(u32Status == 0x10)                  /* Repeat START has been transmitted and prepare SLA+R */
//    {
//        I2C_SET_DATA(I2C0, ((g_u8DeviceAddr << 1) | 0x01));   /* Write SLA+R to Register I2CDAT */
//        I2C_SET_CONTROL_REG(I2C0, I2C_I2CON_SI);
//    }
//    else if(u32Status == 0x40)                  /* SLA+R has been transmitted and ACK has been received */
//    {
//        I2C_SET_CONTROL_REG(I2C0, I2C_I2CON_SI);
//    }
//    else if(u32Status == 0x58)                  /* DATA has been received and NACK has been returned */
//    {
//        g_u8RxData = (unsigned char) I2C_GET_DATA(I2C0);
//        I2C_SET_CONTROL_REG(I2C0, I2C_I2CON_STO_SI);
//        g_u8EndFlag = 1;
//    }
//    else
//    {
//        /* TO DO */
//        printf("Status 0x%x is NOT processed\n", u32Status);
//    }
//}

///*---------------------------------------------------------------------------------------------------------*/
///*  I2C Tx Callback Function                                                                               */
///*---------------------------------------------------------------------------------------------------------*/
//void I2C_MasterTx(uint32_t u32Status)
//{
//    if(u32Status == 0x08)                       /* START has been transmitted */
//    {
//        I2C_SET_DATA(I2C0, g_u8DeviceAddr << 1);    /* Write SLA+W to Register I2CDAT */
//        I2C_SET_CONTROL_REG(I2C0, I2C_I2CON_SI);
//    }
//    else if(u32Status == 0x18)                  /* SLA+W has been transmitted and ACK has been received */
//    {
//        I2C_SET_DATA(I2C0, g_au8TxData[g_u8DataLen++]);
//        I2C_SET_CONTROL_REG(I2C0, I2C_I2CON_SI);
//    }
//    else if(u32Status == 0x20)                  /* SLA+W has been transmitted and NACK has been received */
//    {
//        I2C_SET_CONTROL_REG(I2C0, I2C_I2CON_STA_STO_SI);
//    }
//    else if(u32Status == 0x28)                  /* DATA has been transmitted and ACK has been received */
//    {
//        if(g_u8DataLen != 2)
//        {
//            I2C_SET_DATA(I2C0, g_au8TxData[g_u8DataLen++]);
//            I2C_SET_CONTROL_REG(I2C0, I2C_I2CON_SI);
//        }
//        else
//        {
//            I2C_SET_CONTROL_REG(I2C0, I2C_I2CON_STO_SI);
//            g_u8EndFlag = 1;
//        }
//    }
//    else
//    {
//        /* TO DO */

//    }
//}

void Task_InitTS06N()
{

}



unsigned char Key_GetLocalKey(void)
{
	static unsigned char GetKey;
	static unsigned char 	result;
	result = 0;
	P_I_SW2 = TRUE;

	if ( P_I_POWER )
		result |=  0x01;
	if ( !P_I_SW2 )
		result |=  0x02;
	
	if( result == 0x01 )				// 2.89v  ->  2.98V
		GetKey = 1;
	else if( result == 0x02   )			// 2.89v  ->  2.98V
		GetKey = 2;
	else if( result == 0x04   )			// 2.89v  ->  2.98V
		GetKey = 3;
	else
		GetKey = 0xFF;

	bLocal_In = FALSE;			// local key not pressed!

	if(GetKey != 0xFF)
	{
		bLocal_In = TRUE;				// local key pressed!
		rK_Data = GetKey;				// KEY CONVERION
		return TRUE;
	}
 	
	return FALSE;
}

/********************************************************************************/
/*  Module  : KEY_KeyScan                                                       */
/*  Function: decoding local(Front Panel) key input                             */
/*  Input   : ---                                                               */
/*  Output  : ---                                                               */
/********************************************************************************/
void KEY_KeyScan()
{
	if ( Key_GetLocalKey() )
	{
		if( rK_Data == rK_Back )        	/*                                  */
		{                               	/*                                  */
			rK_ChatCnt -- ;             	/*                                  */
			if( !rK_ChatCnt )           	/*                                  */
			{                           	/*                                  */
				bLocal_Ok = TRUE;           /*                                  */
				bKey_Repeat = FALSE;      	/*                                  */
				if( rK_Data == rK_Last )	/*                                  */
					bKey_Repeat = TRUE;   	/*                                  */
				rK_Last = rK_Data;      	/*                                  */
				rK_ChatCnt = 10;			//<000614-0> 2;         /* 3*50ms = 150ms (Remocon=108ms)   */
			}                           	/*                                  */
		}                               /*                                  */
		else                            /*                                  */
		{                               /*                                  */
			rK_Back = rK_Data;          	/*                                  */
			rK_ChatCnt = 1;             	/*                                  */
		}                               /*                                  */
	}
	else
	{
		bLocal_Ok = FALSE;
		rK_Data = 0xFF;
		rK_Last = 0xFF;
		rK_Back = 0xFF;
		rK_ChatCnt = 1;
	}
}

#define ALL             0x00                /* Allowed in ALL mode              */
#define REPEAT          0x40                /* Not Allowed if Repeat Code In    */
#define STDBY           0x80                /* Not allowed in Power Off         */
/********************************************************************************/

const char  TKey_ModeFlag[] =        /*                                  */
{                                           /*                                  */
        ALL, 	/* 0x00  Key_NoOperation                   */
        ALL, 	/* 0x00  Key_NoOperation                   */
        ALL, 	/* 0x00  Key_NoOperation                   */
        ALL, 	/* 0x00  Key_NoOperation                   */
        ALL, 	/* 0x00  Key_NoOperation                   */
        ALL, 	/* 0x00  Key_NoOperation                   */
        ALL, 	/* 0x00  Key_NoOperation                   */
};                                         

/********************************************************************************/
/*  Key_CheckCondition   : Make a Current TV Status!                                 */
/*------------------------------------------------------------------------------*/
/*  Output          : rKD_Allowed = TV Status!                                  */
/********************************************************************************/
unsigned char Key_CheckCondition()               /*                                  */
{                                           /*                                  */
    unsigned char rKD_Allowed =0;           /*                                  */
                                            /*                                  */
     if ( bKey_Repeat )                        /*                                  */
        rKD_Allowed |= REPEAT;                /*                                  */
    return rKD_Allowed;                     /*                                  */
}                                           /*                                  */


/********************************************************************************/
/*  Key_CheckAndExecute  : Key_CheckAndExecute                                                */
/*------------------------------------------------------------------------------*/
/*  Input       : bLocal_Ok = 1  - Local Key Pressed!                               */
/*              : bIR_OK = 1 - Remote Key Pressed!                              */
/*              : rK_Data    - Local Key code                                   */
/*              : rKR_Data   - Remote Key Code Data                             */
/*              : rK_Time    - Key Execution Time                               */
/*------------------------------------------------------------------------------*/
/*  Output      : Key Service Routine!                                          */
/********************************************************************************/

void KEY_CheckAndExecute(void)
{
    if( rK_Time )                            	/*                                  */
        --rK_Time;														/* Key Service Run?                 */
    if ( bLocal_Ok )                          /* Local Key Check!                 */
    {                                       	/*                                  */
        bLocal_Ok  = FALSE;                  	/* Local Key Pressed!               */
     }                                       /*                                  */
    else                                    /*                                  */
    {                                       /*                                  */
JKD_Return:
		bLocal_Ok      = FALSE;        
		bLocal_In = FALSE;             
        return;                        
    }                         
		
	if ( TKey_ModeFlag[rK_Data] & Key_CheckCondition())	/*                                  */
        goto	JKD_Return;                 

	if ( !bKey_Repeat )                       
		rK_Time = 12;      

	if( rK_Data > 5 )
		goto	JKD_Return;                 
    	
  bKey_DataUp = FALSE;                   

	switch(rK_Data)
		{
			case	1:
				SF_LOCAL_TP1();
				break;
			case	2:
				SF_LOCAL_TP2();
				break;
			case	3:
				SF_LOCAL_TP3();
				break;
			default:
				break;
		}
	
	
}    





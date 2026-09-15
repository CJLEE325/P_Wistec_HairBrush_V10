/**************************************************************************//**
 * @file     sh1106.c
 * @version  V1.00
 * $Revision: 3 $
 * $Date: 14/01/28 11:44a $
 * @brief    NUC029 Series PWM Generator and Capture Timer Driver Sample Code
 *
 * @note
 * Copyright (C) 2014 Nuvoton Technology Corp. All rights reserved.
 *
 ******************************************************************************/
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include "NUC029xAN.h"


#include "sys_const.h"
#include "pub_ram.h"


#include "oled_ssd1306.h"
#include "gui_paint.h"
#include "oled_font.h"


extern	void Delay_1ms(uint32_t time);

// Data buffer voor het scherm
static uint8_t SSD1306_Buffer[SSD1306_WIDTH * SSD1306_HEIGHT / 8];
// Een scherm-object om lokaal in te werken
static SSD1306_t SSD1306;


typedef void (*I2C_FUNC)(uint32_t u32Status);
I2C_FUNC s_I2C0HandlerFn = NULL;

int32_t Read_Write_SLAVE(uint8_t slvaddr);

void SH1106_init(void); 
void SH1106_setContrast(char contrast); 
void SH1106_resetDisplay(void); 
void SH1106_displayOn(void); 
void SH1106_displayOff(void); 
void SH1106_setPixel(int x, int y); 
void SH1106_display(void);
void SH1106_clear(void);
void OLED_1in3_WriteData(uint8_t cmd,uint8_t r_data);

/*---------------------------------------------------------------------------------------------------------*/
/* Global variables (I2C)                                                                                        */
/*---------------------------------------------------------------------------------------------------------*/
//volatile uint8_t g_u8DeviceAddr;
volatile uint8_t g_au8MstTxData[3];
volatile uint8_t g_u8MstRxData;
volatile  uint32_t  g_u8MstDataLen;
volatile uint8_t g_u8MstEndFlag = 0;
volatile uint8_t g_u8DeviceAddr;

volatile uint8_t g_u8DeviceAddr;
volatile uint8_t g_u8SlvDataLen;

typedef struct IIC_DEF
{
	uint8_t 		IIC_SLAddr;    /*!< Font width in pixels */
	uint8_t 		IIC_SUBADDR;   /*!< Font height in pixels */
	uint32_t		IIC_Length;
	uint8_t 		*data; /*!< Pointer to data font data array */

} iic_def;

	iic_def	info;

/*---------------------------------------------------------------------------------------------------------*/
/*  I2C0 IRQ Handler                                                                                       */
/*---------------------------------------------------------------------------------------------------------*/
void I2C0_IRQHandler(void)
{
	uint32_t u32Status;
    u32Status = I2C0->I2CSTATUS;

    if(I2C0->I2CTOC & I2C_I2CTOC_TIF_Msk)
    {
        /* Clear I2C0 Timeout Flag */
        I2C0->I2CTOC |= I2C_I2CTOC_TIF_Msk;
   //     g_u8TimeoutFlag = 1;
    }
    else
    {
        if(s_I2C0HandlerFn != NULL)
            s_I2C0HandlerFn(u32Status);
    } 
}


void I2C0_TS06_WRITE(unsigned char	Reg, unsigned char	data)
{

	g_u8DeviceAddr = 0x3C;

    I2C_SET_CONTROL_REG(I2C0, I2C_I2CON_STA_SI);		//  Start
//    while(I2C_GET_STATUS(I2C0) != 0x08);

    I2C_SET_CONTROL_REG(I2C0, I2C_I2CON_SI);			//  Start
	I2C_SET_DATA(I2C0, g_u8DeviceAddr);    			/* Write SLA+W to Register I2CDAT */
//    while(I2C_GET_STATUS(I2C0) != 0x18);

    I2C_SET_CONTROL_REG(I2C0, I2C_I2CON_SI);			//  Start
    I2C_SET_DATA(I2C0, Reg);
//    while(I2C_GET_STATUS(I2C0) != 0x28);

    I2C_SET_CONTROL_REG(I2C0, I2C_I2CON_SI);			//  Start
    I2C_SET_DATA(I2C0, data);
//    while(I2C_GET_STATUS(I2C0) != 0x28);

    I2C_SET_CONTROL_REG(I2C0, I2C_I2CON_STO_SI);			//  Stop
	
}

/*---------------------------------------------------------------------------------------------------------*/
/*  I2C Tx Callback Function                                                                               */
/*---------------------------------------------------------------------------------------------------------*/
void I2C_MasterTxCmd(uint32_t u32Status)
{
    if(u32Status == 0x08)                       /* START has been transmitted */
    {
        I2C0->I2CDAT = g_u8DeviceAddr << 1;     /* Write SLA+W to Register I2CDAT */
        I2C_SET_CONTROL_REG(I2C0, I2C_I2CON_SI);
    }
    else if(u32Status == 0x18)                  /* SLA+W has been transmitted and ACK has been received */
    {
        I2C0->I2CDAT = g_au8MstTxData[g_u8MstDataLen++];
        I2C_SET_CONTROL_REG(I2C0, I2C_I2CON_SI);
    }
    else if(u32Status == 0x20)                  /* SLA+W has been transmitted and NACK has been received */
    {
        I2C_SET_CONTROL_REG(I2C0, I2C_I2CON_STA_STO_SI);
    }
    else if(u32Status == 0x28)                  /* DATA has been transmitted and ACK has been received */
    {
        if(g_u8MstDataLen != 2)
        {
            I2C0->I2CDAT = g_au8MstTxData[g_u8MstDataLen++];
            I2C_SET_CONTROL_REG(I2C0, I2C_I2CON_SI);
        }
        else
        {
            I2C_SET_CONTROL_REG(I2C0, I2C_I2CON_STO_SI);
            g_u8MstEndFlag = 1;
        }
    }
//    else if(u32Status == 0x30)                  /* DATA has been transmitted and ACK has been received */
//	{
//            I2C_SET_CONTROL_REG(I2C0, I2C_I2CON_STO_SI);
//            I2C_SET_CONTROL_REG(I2C0, I2C_I2CON_SI);
//             g_u8MstEndFlag = 1;
//		
//	}
    else
    {
		/* TO DO */
    //    printf("Status 0x%x is NOT processed\n", u32Status);
    }
}

/*---------------------------------------------------------------------------------------------------------*/
/*  I2C Tx Callback Function                                                                               */
/*---------------------------------------------------------------------------------------------------------*/
void I2C_MasterTxData(uint32_t u32Status)
{
	static  unsigned int	rDataSeq;		
//	info.data =  &SSD1306_Buffer[0];
    if(u32Status == 0x08)                       /* START has been transmitted */
    {
        I2C0->I2CDAT = info.IIC_SLAddr << 1;     /* Write SLA+W to Register I2CDAT */
        I2C_SET_CONTROL_REG(I2C0, I2C_I2CON_SI);
    }
    else if(u32Status == 0x18)                  /* SLA+W has been transmitted and ACK has been received */
    {
        I2C0->I2CDAT = info.IIC_SUBADDR;
        I2C_SET_CONTROL_REG(I2C0, I2C_I2CON_SI);
		rDataSeq = 0;
    }
    else if(u32Status == 0x20)                  /* SLA+W has been transmitted and NACK has been received */
    {
        I2C_SET_CONTROL_REG(I2C0, I2C_I2CON_STA_STO_SI);
    }
    else if(u32Status == 0x28)                  /* DATA has been transmitted and ACK has been received */
    {
		if ( -- info.IIC_Length ) 	
		{
 //           I2C0->I2CDAT = SSD1306_Buffer[rDataSeq++];
            I2C0->I2CDAT = (info.data[rDataSeq++]);
            I2C_SET_CONTROL_REG(I2C0, I2C_I2CON_SI);
			
		}
        else
        {
            I2C_SET_CONTROL_REG(I2C0, I2C_I2CON_STO_SI);
            g_u8MstEndFlag = 1;
        }
    }
//    else if(u32Status == 0x30)                  /* DATA has been transmitted and ACK has been received */
//	{
//            I2C_SET_CONTROL_REG(I2C0, I2C_I2CON_STO_SI);
//            I2C_SET_CONTROL_REG(I2C0, I2C_I2CON_SI);
//             g_u8MstEndFlag = 1;
//		
//	}
    else
    {
		/* TO DO */
    //    printf("Status 0x%x is NOT processed\n", u32Status);
    }
}


static void OLED_1in3_WriteCmd(uint8_t cmd,uint8_t r_data)
{
   g_u8DeviceAddr = 0x3C;
   g_au8MstTxData[0] = cmd;
   g_au8MstTxData[1] = r_data;

	g_u8MstDataLen = 0;
   g_u8MstEndFlag = 0;

   /* I2C function to write data to slave */
   s_I2C0HandlerFn = (I2C_FUNC)I2C_MasterTxCmd;

   /* I2C as master sends START signal */
   I2C_SET_CONTROL_REG(I2C0, I2C_I2CON_STA);
   /* Wait I2C Tx Finish */
   while(g_u8MstEndFlag == 0);
   g_u8MstEndFlag = 0;


}
/**
  ******************************************************************************
  * @file    Font8.c
  * @author  MCD Application Team
  * @version V1.0.0
  * @date    18-February-2014
  * @brief   This file provides text Font8 for STM32xx-EVAL's LCD driver. 
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT(c) 2014 STMicroelectronics</center></h2>
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/

// 
//  Font data for Courier New 12pt
// 


const unsigned char gImage_1in3[1030] = { /*0X00,0X01,0X40,0X00,0X80,0X00,*/
0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,
0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,
0X00,0X00,0X00,0X00,0X78,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0XFE,0X00,0X00,0X00,
0X00,0X00,0X00,0X01,0XC7,0X00,0X00,0X00,0X00,0X00,0X00,0X03,0X83,0X80,0X70,0X00,
0X00,0X00,0X1F,0X07,0X01,0X83,0XFE,0X00,0X00,0X00,0X3F,0XC6,0X01,0XCF,0X8F,0X00,
0X00,0X00,0X30,0XE6,0X00,0XFC,0X03,0X00,0X00,0X00,0X70,0X7E,0X00,0XF0,0X03,0X00,
0X00,0X00,0X60,0X3C,0X00,0X60,0X03,0X00,0X00,0X00,0X60,0X1C,0X00,0X00,0X03,0X00,
0X00,0X00,0X60,0X0C,0X00,0X00,0X07,0X00,0X00,0X00,0X70,0X0C,0X00,0X00,0X06,0X00,
0X00,0X00,0X30,0X00,0X00,0X00,0X0E,0X00,0X00,0X00,0X30,0X00,0X42,0X00,0X0C,0X00,
0X00,0X00,0X38,0X00,0X42,0X00,0X1C,0X00,0X00,0X00,0X18,0X00,0X66,0X00,0X38,0X00,
0X00,0X00,0X1C,0X00,0X6E,0X00,0X70,0X00,0X00,0X00,0X0F,0X00,0X3C,0X01,0XE0,0X00,
0X00,0X00,0X07,0XF8,0X10,0X7F,0XC0,0X00,0X00,0X00,0X07,0XE0,0X00,0X1F,0X00,0X00,
0X00,0X00,0X0F,0X00,0X00,0X07,0X80,0X00,0X00,0X00,0X1C,0X00,0X20,0X01,0XC0,0X00,
0X00,0X00,0X38,0X00,0X60,0X00,0XE0,0X00,0X00,0X00,0X30,0X00,0XC0,0X00,0XF0,0X00,
0X00,0X00,0X70,0X01,0X80,0X01,0XF0,0X00,0X00,0X00,0X60,0X03,0X00,0X03,0XB8,0X00,
0X00,0X00,0X60,0X0E,0X00,0X07,0X18,0X00,0X00,0X00,0XE0,0X1C,0X00,0X0E,0X1C,0X00,
0X00,0X00,0XE0,0X38,0X00,0X1C,0X0C,0X00,0X00,0X00,0XC0,0X70,0X00,0X38,0X0E,0X00,
0X00,0X00,0XC1,0XC0,0X00,0X70,0X0E,0X00,0X00,0X00,0XC3,0X80,0X00,0XE0,0X06,0X00,
0X00,0X00,0XC7,0X00,0X03,0XC0,0X06,0X00,0X00,0X00,0XDC,0X00,0X07,0X00,0X07,0X00,
0X00,0X00,0XF8,0X00,0X0E,0X00,0X07,0X00,0X00,0X01,0XF0,0X00,0X3C,0X00,0X0F,0X00,
0X00,0X01,0XC0,0X00,0X70,0X00,0X1B,0X00,0X00,0X01,0XC0,0X01,0XE0,0X00,0X33,0X00,
0X00,0X01,0XC0,0X03,0XC0,0X00,0X73,0X80,0X00,0X01,0XC0,0X0F,0X00,0X01,0XC3,0X80,
0X00,0X01,0XC0,0X3C,0X00,0X03,0X83,0X80,0X00,0X01,0XC0,0XF8,0X00,0X07,0X01,0X80,
0X00,0X01,0XC3,0XE0,0X00,0X1E,0X01,0X80,0X00,0X01,0XCF,0X80,0X00,0X3C,0X01,0X80,
0X00,0X01,0XFE,0X00,0X00,0XF0,0X01,0X80,0X00,0X01,0XF8,0X00,0X01,0XE0,0X01,0XC0,
0X00,0X01,0XE0,0X00,0X03,0XC0,0X01,0XC0,0X00,0X01,0XC0,0X00,0X0F,0X00,0X01,0XC0,
0X00,0X00,0XC0,0X00,0X3C,0X00,0X01,0XC0,0X00,0X00,0XC0,0X00,0X78,0X00,0X03,0XC0,
0X00,0X00,0XC0,0X03,0XE0,0X00,0X07,0XC0,0X00,0X00,0XC0,0X0F,0X80,0X00,0X0C,0XC0,
0X00,0X00,0XC0,0X3E,0X00,0X00,0X38,0XC0,0X00,0X00,0XC0,0XF8,0X00,0X00,0X70,0XC0,
0X00,0X00,0XC3,0XE0,0X00,0X01,0XE0,0XC0,0X00,0X00,0XFF,0X00,0X00,0X07,0X80,0XC0,
0X00,0X00,0XFC,0X00,0X00,0X0F,0X00,0XC0,0X00,0X00,0XE0,0X00,0X00,0X3C,0X00,0XC0,
0X00,0X00,0XC0,0X00,0X00,0XF0,0X00,0XC0,0X00,0X00,0XE0,0X00,0X03,0XC0,0X00,0XC0,
0X00,0X00,0XE0,0X00,0X0F,0X00,0X00,0XE0,0X00,0X00,0XE0,0X00,0X7C,0X00,0X00,0XE0,
0X00,0X00,0XE0,0X03,0XF0,0X00,0X01,0XE0,0X00,0X00,0X60,0X1F,0X80,0X00,0X03,0XE0,
0X00,0X00,0X63,0XFC,0X00,0X00,0X0E,0XE0,0X00,0X00,0X7F,0XE0,0X00,0X00,0X1C,0XE0,
0X00,0X00,0X7E,0X00,0X00,0X00,0X78,0XE0,0X00,0X00,0X60,0X00,0X00,0X01,0XE0,0XE0,
0X00,0X00,0X60,0X00,0X00,0X07,0XC0,0XE0,0X00,0X00,0X70,0X00,0X00,0X1F,0X00,0XE0,
0X00,0X00,0X70,0X00,0X00,0X7C,0X00,0XE0,0X00,0X00,0X70,0X00,0X03,0XF0,0X00,0XE0,
0X00,0X00,0X30,0X00,0X1F,0X80,0X00,0XE0,0X00,0X00,0X30,0X01,0XFE,0X00,0X00,0XE0,
0X00,0X00,0X30,0X3F,0XF0,0X00,0X03,0XE0,0X00,0X00,0X3F,0XFE,0X00,0X00,0X06,0XE0,
0X00,0X00,0X3F,0XC0,0X00,0X00,0X1C,0XE0,0X00,0X00,0X38,0X00,0X00,0X00,0X38,0XE0,
0X00,0X00,0X38,0X00,0X00,0X01,0XF0,0XE0,0X00,0X00,0X38,0X00,0X00,0X03,0XC0,0XE0,
0X00,0X00,0X38,0X00,0X00,0X1F,0X00,0XE0,0X00,0X00,0X38,0X00,0X00,0XFC,0X00,0XE0,
0X00,0X00,0X38,0X00,0X03,0XF0,0X00,0XE0,0X00,0X00,0X18,0X00,0X3F,0X80,0X01,0XE0,
0X00,0X00,0X18,0X07,0XFC,0X00,0X03,0XE0,0X00,0X00,0X1F,0XFF,0XE0,0X00,0X0E,0XE0,
0X00,0X00,0X1F,0XFC,0X00,0X00,0X1C,0XC0,0X00,0X00,0X1C,0X00,0X00,0X00,0X70,0XC0,
0X00,0X00,0X1C,0X00,0X00,0X01,0XE0,0XC0,0X00,0X00,0X1C,0X00,0X00,0X07,0X80,0XC0,
0X00,0X00,0X0C,0X00,0X00,0X1F,0X00,0XC0,0X00,0X00,0X0C,0X00,0X00,0XFC,0X01,0XC0,
0X00,0X00,0X0E,0X00,0X07,0XE0,0X01,0XC0,0X00,0X00,0X0E,0X00,0X3F,0X80,0X01,0XC0,
0X00,0X00,0X06,0X03,0XFC,0X00,0X07,0X80,0X00,0X00,0X07,0X7F,0XE0,0X00,0X0F,0X80,
0X00,0X00,0X07,0XFE,0X00,0X00,0X1B,0X80,0X00,0X00,0X03,0X80,0X00,0X00,0X73,0X00,
0X00,0X00,0X03,0X80,0X00,0X01,0XE7,0X00,0X00,0X00,0X01,0X80,0X00,0X03,0X8E,0X00,
0X00,0X00,0X01,0XC0,0X00,0X1E,0X3C,0X00,0X00,0X00,0X00,0XE0,0X00,0X78,0X78,0X00,
0X00,0X00,0X00,0X78,0X01,0XF3,0XF8,0X00,0X00,0X00,0X00,0X3F,0X07,0X9F,0X9E,0X00,
0X00,0X00,0X00,0X3F,0XC3,0X80,0X07,0X00,0X00,0X00,0X00,0X70,0X03,0XE0,0X03,0X80,
0X00,0X00,0X00,0XE0,0X03,0X70,0X01,0XC0,0X00,0X00,0X00,0XE0,0X03,0X18,0X00,0XE0,
0X00,0X00,0X00,0XC0,0X02,0X00,0X00,0X60,0X00,0X00,0X00,0XC0,0X02,0X00,0X00,0X30,
0X00,0X00,0X00,0XC0,0X00,0X00,0X00,0X30,0X00,0X00,0X01,0XC0,0X00,0X00,0X00,0X38,
0X00,0X00,0X01,0XC0,0X00,0X08,0X00,0X18,0X00,0X00,0X01,0XC0,0X00,0X1C,0X00,0X18,
0X00,0X00,0X01,0XC0,0X40,0X1F,0X00,0X18,0X00,0X00,0X01,0XC0,0XC0,0X1B,0X80,0X18,
0X00,0X00,0X00,0XC1,0XE0,0X19,0XE0,0X18,0X00,0X00,0X00,0XC1,0XF0,0X30,0X78,0X38,
0X00,0X00,0X00,0XE3,0XF8,0X70,0X3F,0XF0,0X00,0X00,0X00,0XE7,0X3F,0XE0,0X07,0XF0,
0X00,0X00,0X00,0X7F,0X0F,0XC0,0X00,0X80,0X00,0X00,0X00,0X3C,0X00,0X00,0X00,0X00,
0X00,0X00,0X00,0X18,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X18,0X00,0X00,0X00,0X00,
};





const uint8_t Font8_Table[] = 
{
	// @0 ' ' (5 pixels wide)
	0x00, //      
	0x00, //      
	0x00, //      
	0x00, //      
	0x00, //      
	0x00, //      
	0x00, //      
	0x00, //      

	// @8 '!' (5 pixels wide)
	0x20, //   #  
	0x20, //   #  
	0x20, //   #  
	0x20, //   #  
	0x00, //      
	0x20, //   #  
	0x00, //      
	0x00, //      

	// @16 '"' (5 pixels wide)
	0x50, //  # # 
	0x50, //  # # 
	0x00, //      
	0x00, //      
	0x00, //      
	0x00, //      
	0x00, //      
	0x00, //      

	// @24 '#' (5 pixels wide)
	0x28, //   # #
	0x50, //  # # 
	0xF8, // #####
	0x50, //  # # 
	0xF8, // #####
	0x50, //  # # 
	0xA0, // # #  
	0x00, //      

	// @32 '$' (5 pixels wide)
	0x20, //   #  
	0x30, //   ## 
	0x60, //  ##  
	0x30, //   ## 
	0x10, //    # 
	0x60, //  ##  
	0x20, //   #  
	0x00, //      

	// @40 '%' (5 pixels wide)
	0x20, //   #  
	0x20, //   #  
	0x18, //    ##
	0x60, //  ##  
	0x10, //    # 
	0x10, //    # 
	0x00, //      
	0x00, //      

	// @48 '&' (5 pixels wide)
	0x00, //      
	0x38, //   ###
	0x20, //   #  
	0x60, //  ##  
	0x50, //  # # 
	0x78, //  ####
	0x00, //      
	0x00, //      

	// @56 ''' (5 pixels wide)
	0x20, //   #  
	0x20, //   #  
	0x20, //   #  
	0x00, //      
	0x00, //      
	0x00, //      
	0x00, //      
	0x00, //      

	// @64 '(' (5 pixels wide)
	0x10, //    # 
	0x20, //   #  
	0x20, //   #  
	0x20, //   #  
	0x20, //   #  
	0x20, //   #  
	0x10, //    # 
	0x00, //      

	// @72 ')' (5 pixels wide)
	0x40, //  #   
	0x20, //   #  
	0x20, //   #  
	0x20, //   #  
	0x20, //   #  
	0x20, //   #  
	0x40, //  #   
	0x00, //      

	// @80 '*' (5 pixels wide)
	0x20, //   #  
	0x70, //  ### 
	0x20, //   #  
	0x50, //  # # 
	0x00, //      
	0x00, //      
	0x00, //      
	0x00, //      

	// @88 '+' (5 pixels wide)
	0x00, //      
	0x20, //   #  
	0x20, //   #  
	0xF8, // #####
	0x20, //   #  
	0x20, //   #  
	0x00, //      
	0x00, //      

	// @96 ',' (5 pixels wide)
	0x00, //      
	0x00, //      
	0x00, //      
	0x00, //      
	0x10, //    # 
	0x20, //   #  
	0x20, //   #  
	0x00, //      

	// @104 '-' (5 pixels wide)
	0x00, //      
	0x00, //      
	0x00, //      
	0x70, //  ### 
	0x00, //      
	0x00, //      
	0x00, //      
	0x00, //      

	// @112 '.' (5 pixels wide)
	0x00, //      
	0x00, //      
	0x00, //      
	0x00, //      
	0x00, //      
	0x20, //   #  
	0x00, //      
	0x00, //      

	// @120 '/' (5 pixels wide)
	0x10, //    # 
	0x20, //   #  
	0x20, //   #  
	0x20, //   #  
	0x40, //  #   
	0x40, //  #   
	0x80, // #    
	0x00, //      

	// @128 '0' (5 pixels wide)
	0x20, //   #  
	0x50, //  # # 
	0x50, //  # # 
	0x50, //  # # 
	0x50, //  # # 
	0x20, //   #  
	0x00, //      
	0x00, //      

	// @136 '1' (5 pixels wide)
	0x60, //  ##  
	0x20, //   #  
	0x20, //   #  
	0x20, //   #  
	0x20, //   #  
	0xF8, // #####
	0x00, //      
	0x00, //      

	// @144 '2' (5 pixels wide)
	0x20, //   #  
	0x50, //  # # 
	0x20, //   #  
	0x20, //   #  
	0x40, //  #   
	0x70, //  ### 
	0x00, //      
	0x00, //      

	// @152 '3' (5 pixels wide)
	0x20, //   #  
	0x50, //  # # 
	0x10, //    # 
	0x20, //   #  
	0x10, //    # 
	0x60, //  ##  
	0x00, //      
	0x00, //      

	// @160 '4' (5 pixels wide)
	0x10, //    # 
	0x30, //   ## 
	0x50, //  # # 
	0x78, //  ####
	0x10, //    # 
	0x38, //   ###
	0x00, //      
	0x00, //      

	// @168 '5' (5 pixels wide)
	0x70, //  ### 
	0x40, //  #   
	0x60, //  ##  
	0x10, //    # 
	0x50, //  # # 
	0x20, //   #  
	0x00, //      
	0x00, //      

	// @176 '6' (5 pixels wide)
	0x30, //   ## 
	0x40, //  #   
	0x60, //  ##  
	0x50, //  # # 
	0x50, //  # # 
	0x60, //  ##  
	0x00, //      
	0x00, //      

	// @184 '7' (5 pixels wide)
	0x70, //  ### 
	0x50, //  # # 
	0x10, //    # 
	0x20, //   #  
	0x20, //   #  
	0x20, //   #  
	0x00, //      
	0x00, //      

	// @192 '8' (5 pixels wide)
	0x20, //   #  
	0x50, //  # # 
	0x20, //   #  
	0x50, //  # # 
	0x50, //  # # 
	0x20, //   #  
	0x00, //      
	0x00, //      

	// @200 '9' (5 pixels wide)
	0x30, //   ## 
	0x50, //  # # 
	0x50, //  # # 
	0x30, //   ## 
	0x10, //    # 
	0x60, //  ##  
	0x00, //      
	0x00, //      

	// @208 ':' (5 pixels wide)
	0x00, //      
	0x00, //      
	0x20, //   #  
	0x00, //      
	0x00, //      
	0x20, //   #  
	0x00, //      
	0x00, //      

	// @216 ';' (5 pixels wide)
	0x00, //      
	0x00, //      
	0x10, //    # 
	0x00, //      
	0x10, //    # 
	0x20, //   #  
	0x00, //      
	0x00, //      

	// @224 '<' (5 pixels wide)
	0x00, //      
	0x10, //    # 
	0x20, //   #  
	0xC0, // ##   
	0x20, //   #  
	0x10, //    # 
	0x00, //      
	0x00, //      

	// @232 '=' (5 pixels wide)
	0x00, //      
	0x70, //  ### 
	0x00, //      
	0x70, //  ### 
	0x00, //      
	0x00, //      
	0x00, //      
	0x00, //      

	// @240 '>' (5 pixels wide)
	0x00, //      
	0x40, //  #   
	0x20, //   #  
	0x18, //    ##
	0x20, //   #  
	0x40, //  #   
	0x00, //      
	0x00, //      

	// @248 '?' (5 pixels wide)
	0x20, //   #  
	0x50, //  # # 
	0x10, //    # 
	0x20, //   #  
	0x00, //      
	0x20, //   #  
	0x00, //      
	0x00, //      

	// @256 '@' (5 pixels wide)
	0x30, //   ## 
	0x48, //  #  #
	0x48, //  #  #
	0x58, //  # ##
	0x48, //  #  #
	0x40, //  #   
	0x38, //   ###
	0x00, //      

	// @264 'A' (5 pixels wide)
	0x60, //  ##  
	0x20, //   #  
	0x50, //  # # 
	0x70, //  ### 
	0x88, // #   #
	0xD8, // ## ##
	0x00, //      
	0x00, //      

	// @272 'B' (5 pixels wide)
	0xF0, // #### 
	0x48, //  #  #
	0x70, //  ### 
	0x48, //  #  #
	0x48, //  #  #
	0xF0, // #### 
	0x00, //      
	0x00, //      

	// @280 'C' (5 pixels wide)
	0x70, //  ### 
	0x50, //  # # 
	0x40, //  #   
	0x40, //  #   
	0x40, //  #   
	0x30, //   ## 
	0x00, //      
	0x00, //      

	// @288 'D' (5 pixels wide)
	0xF0, // #### 
	0x48, //  #  #
	0x48, //  #  #
	0x48, //  #  #
	0x48, //  #  #
	0xF0, // #### 
	0x00, //      
	0x00, //      

	// @296 'E' (5 pixels wide)
	0xF8, // #####
	0x48, //  #  #
	0x60, //  ##  
	0x40, //  #   
	0x48, //  #  #
	0xF8, // #####
	0x00, //      
	0x00, //      

	// @304 'F' (5 pixels wide)
	0xF8, // #####
	0x48, //  #  #
	0x60, //  ##  
	0x40, //  #   
	0x40, //  #   
	0xE0, // ###  
	0x00, //      
	0x00, //      

	// @312 'G' (5 pixels wide)
	0x70, //  ### 
	0x40, //  #   
	0x40, //  #   
	0x58, //  # ##
	0x50, //  # # 
	0x30, //   ## 
	0x00, //      
	0x00, //      

	// @320 'H' (5 pixels wide)
	0xE8, // ### #
	0x48, //  #  #
	0x78, //  ####
	0x48, //  #  #
	0x48, //  #  #
	0xE8, // ### #
	0x00, //      
	0x00, //      

	// @328 'I' (5 pixels wide)
	0x70, //  ### 
	0x20, //   #  
	0x20, //   #  
	0x20, //   #  
	0x20, //   #  
	0x70, //  ### 
	0x00, //      
	0x00, //      

	// @336 'J' (5 pixels wide)
	0x38, //   ###
	0x10, //    # 
	0x10, //    # 
	0x50, //  # # 
	0x50, //  # # 
	0x20, //   #  
	0x00, //      
	0x00, //      

	// @344 'K' (5 pixels wide)
	0xD8, // ## ##
	0x50, //  # # 
	0x60, //  ##  
	0x70, //  ### 
	0x50, //  # # 
	0xD8, // ## ##
	0x00, //      
	0x00, //      

	// @352 'L' (5 pixels wide)
	0xE0, // ###  
	0x40, //  #   
	0x40, //  #   
	0x40, //  #   
	0x48, //  #  #
	0xF8, // #####
	0x00, //      
	0x00, //      

	// @360 'M' (5 pixels wide)
	0xD8, // ## ##
	0xD8, // ## ##
	0xD8, // ## ##
	0xA8, // # # #
	0x88, // #   #
	0xD8, // ## ##
	0x00, //      
	0x00, //      

	// @368 'N' (5 pixels wide)
	0xD8, // ## ##
	0x68, //  ## #
	0x68, //  ## #
	0x58, //  # ##
	0x58, //  # ##
	0xE8, // ### #
	0x00, //      
	0x00, //      

	// @376 'O' (5 pixels wide)
	0x30, //   ## 
	0x48, //  #  #
	0x48, //  #  #
	0x48, //  #  #
	0x48, //  #  #
	0x30, //   ## 
	0x00, //      
	0x00, //      

	// @384 'P' (5 pixels wide)
	0xF0, // #### 
	0x48, //  #  #
	0x48, //  #  #
	0x70, //  ### 
	0x40, //  #   
	0xE0, // ###  
	0x00, //      
	0x00, //      

	// @392 'Q' (5 pixels wide)
	0x30, //   ## 
	0x48, //  #  #
	0x48, //  #  #
	0x48, //  #  #
	0x48, //  #  #
	0x30, //   ## 
	0x18, //    ##
	0x00, //      

	// @400 'R' (5 pixels wide)
	0xF0, // #### 
	0x48, //  #  #
	0x48, //  #  #
	0x70, //  ### 
	0x48, //  #  #
	0xE8, // ### #
	0x00, //      
	0x00, //      

	// @408 'S' (5 pixels wide)
	0x70, //  ### 
	0x50, //  # # 
	0x20, //   #  
	0x10, //    # 
	0x50, //  # # 
	0x70, //  ### 
	0x00, //      
	0x00, //      

	// @416 'T' (5 pixels wide)
	0xF8, // #####
	0xA8, // # # #
	0x20, //   #  
	0x20, //   #  
	0x20, //   #  
	0x70, //  ### 
	0x00, //      
	0x00, //      

	// @424 'U' (5 pixels wide)
	0xD8, // ## ##
	0x48, //  #  #
	0x48, //  #  #
	0x48, //  #  #
	0x48, //  #  #
	0x30, //   ## 
	0x00, //      
	0x00, //      

	// @432 'V' (5 pixels wide)
	0xD8, // ## ##
	0x88, // #   #
	0x48, //  #  #
	0x50, //  # # 
	0x50, //  # # 
	0x30, //   ## 
	0x00, //      
	0x00, //      

	// @440 'W' (5 pixels wide)
	0xD8, // ## ##
	0x88, // #   #
	0xA8, // # # #
	0xA8, // # # #
	0xA8, // # # #
	0x50, //  # # 
	0x00, //      
	0x00, //      

	// @448 'X' (5 pixels wide)
	0xD8, // ## ##
	0x50, //  # # 
	0x20, //   #  
	0x20, //   #  
	0x50, //  # # 
	0xD8, // ## ##
	0x00, //      
	0x00, //      

	// @456 'Y' (5 pixels wide)
	0xD8, // ## ##
	0x88, // #   #
	0x50, //  # # 
	0x20, //   #  
	0x20, //   #  
	0x70, //  ### 
	0x00, //      
	0x00, //      

	// @464 'Z' (5 pixels wide)
	0x78, //  ####
	0x48, //  #  #
	0x10, //    # 
	0x20, //   #  
	0x48, //  #  #
	0x78, //  ####
	0x00, //      
	0x00, //      

	// @472 '[' (5 pixels wide)
	0x30, //   ## 
	0x20, //   #  
	0x20, //   #  
	0x20, //   #  
	0x20, //   #  
	0x20, //   #  
	0x30, //   ## 
	0x00, //      

	// @480 '\' (5 pixels wide)
	0x80, // #    
	0x40, //  #   
	0x40, //  #   
	0x20, //   #  
	0x20, //   #  
	0x20, //   #  
	0x10, //    # 
	0x00, //      

	// @488 ']' (5 pixels wide)
	0x60, //  ##  
	0x20, //   #  
	0x20, //   #  
	0x20, //   #  
	0x20, //   #  
	0x20, //   #  
	0x60, //  ##  
	0x00, //      

	// @496 '^' (5 pixels wide)
	0x20, //   #  
	0x20, //   #  
	0x50, //  # # 
	0x00, //      
	0x00, //      
	0x00, //      
	0x00, //      
	0x00, //      

	// @504 '_' (5 pixels wide)
	0x00, //      
	0x00, //      
	0x00, //      
	0x00, //      
	0x00, //      
	0x00, //      
	0x00, //      
	0xF8, // #####

	// @512 '`' (5 pixels wide)
	0x20, //   #  
	0x10, //    # 
	0x00, //      
	0x00, //      
	0x00, //      
	0x00, //      
	0x00, //      
	0x00, //      

	// @520 'a' (5 pixels wide)
	0x00, //      
	0x00, //      
	0x30, //   ## 
	0x10, //    # 
	0x70, //  ### 
	0x78, //  ####
	0x00, //      
	0x00, //      

	// @528 'b' (5 pixels wide)
	0xC0, // ##   
	0x40, //  #   
	0x70, //  ### 
	0x48, //  #  #
	0x48, //  #  #
	0xF0, // #### 
	0x00, //      
	0x00, //      

	// @536 'c' (5 pixels wide)
	0x00, //      
	0x00, //      
	0x70, //  ### 
	0x40, //  #   
	0x40, //  #   
	0x70, //  ### 
	0x00, //      
	0x00, //      

	// @544 'd' (5 pixels wide)
	0x18, //    ##
	0x08, //     #
	0x38, //   ###
	0x48, //  #  #
	0x48, //  #  #
	0x38, //   ###
	0x00, //      
	0x00, //      

	// @552 'e' (5 pixels wide)
	0x00, //      
	0x00, //      
	0x70, //  ### 
	0x70, //  ### 
	0x40, //  #   
	0x30, //   ## 
	0x00, //      
	0x00, //      

	// @560 'f' (5 pixels wide)
	0x10, //    # 
	0x20, //   #  
	0x70, //  ### 
	0x20, //   #  
	0x20, //   #  
	0x70, //  ### 
	0x00, //      
	0x00, //      

	// @568 'g' (5 pixels wide)
	0x00, //      
	0x00, //      
	0x38, //   ###
	0x48, //  #  #
	0x48, //  #  #
	0x38, //   ###
	0x08, //     #
	0x30, //   ## 

	// @576 'h' (5 pixels wide)
	0xC0, // ##   
	0x40, //  #   
	0x70, //  ### 
	0x48, //  #  #
	0x48, //  #  #
	0xE8, // ### #
	0x00, //      
	0x00, //      

	// @584 'i' (5 pixels wide)
	0x20, //   #  
	0x00, //      
	0x60, //  ##  
	0x20, //   #  
	0x20, //   #  
	0x70, //  ### 
	0x00, //      
	0x00, //      

	// @592 'j' (5 pixels wide)
	0x20, //   #  
	0x00, //      
	0x70, //  ### 
	0x10, //    # 
	0x10, //    # 
	0x10, //    # 
	0x10, //    # 
	0x70, //  ### 

	// @600 'k' (5 pixels wide)
	0xC0, // ##   
	0x40, //  #   
	0x58, //  # ##
	0x70, //  ### 
	0x50, //  # # 
	0xD8, // ## ##
	0x00, //      
	0x00, //      

	// @608 'l' (5 pixels wide)
	0x60, //  ##  
	0x20, //   #  
	0x20, //   #  
	0x20, //   #  
	0x20, //   #  
	0x70, //  ### 
	0x00, //      
	0x00, //      

	// @616 'm' (5 pixels wide)
	0x00, //      
	0x00, //      
	0xD0, // ## # 
	0xA8, // # # #
	0xA8, // # # #
	0xA8, // # # #
	0x00, //      
	0x00, //      

	// @624 'n' (5 pixels wide)
	0x00, //      
	0x00, //      
	0xF0, // #### 
	0x48, //  #  #
	0x48, //  #  #
	0xC8, // ##  #
	0x00, //      
	0x00, //      

	// @632 'o' (5 pixels wide)
	0x00, //      
	0x00, //      
	0x30, //   ## 
	0x48, //  #  #
	0x48, //  #  #
	0x30, //   ## 
	0x00, //      
	0x00, //      

	// @640 'p' (5 pixels wide)
	0x00, //      
	0x00, //      
	0xF0, // #### 
	0x48, //  #  #
	0x48, //  #  #
	0x70, //  ### 
	0x40, //  #   
	0xE0, // ###  

	// @648 'q' (5 pixels wide)
	0x00, //      
	0x00, //      
	0x38, //   ###
	0x48, //  #  #
	0x48, //  #  #
	0x38, //   ###
	0x08, //     #
	0x18, //    ##

	// @656 'r' (5 pixels wide)
	0x00, //      
	0x00, //      
	0x78, //  ####
	0x20, //   #  
	0x20, //   #  
	0x70, //  ### 
	0x00, //      
	0x00, //      

	// @664 's' (5 pixels wide)
	0x00, //      
	0x00, //      
	0x30, //   ## 
	0x20, //   #  
	0x10, //    # 
	0x60, //  ##  
	0x00, //      
	0x00, //      

	// @672 't' (5 pixels wide)
	0x00, //      
	0x40, //  #   
	0xF0, // #### 
	0x40, //  #   
	0x48, //  #  #
	0x30, //   ## 
	0x00, //      
	0x00, //      

	// @680 'u' (5 pixels wide)
	0x00, //      
	0x00, //      
	0xD8, // ## ##
	0x48, //  #  #
	0x48, //  #  #
	0x38, //   ###
	0x00, //      
	0x00, //      

	// @688 'v' (5 pixels wide)
	0x00, //      
	0x00, //      
	0xC8, // ##  #
	0x48, //  #  #
	0x30, //   ## 
	0x30, //   ## 
	0x00, //      
	0x00, //      

	// @696 'w' (5 pixels wide)
	0x00, //      
	0x00, //      
	0xD8, // ## ##
	0xA8, // # # #
	0xA8, // # # #
	0x50, //  # # 
	0x00, //      
	0x00, //      

	// @704 'x' (5 pixels wide)
	0x00, //      
	0x00, //      
	0x48, //  #  #
	0x30, //   ## 
	0x30, //   ## 
	0x48, //  #  #
	0x00, //      
	0x00, //      

	// @712 'y' (5 pixels wide)
	0x00, //      
	0x00, //      
	0xD8, // ## ##
	0x50, //  # # 
	0x50, //  # # 
	0x20, //   #  
	0x20, //   #  
	0x60, //  ##  

	// @720 'z' (5 pixels wide)
	0x00, //      
	0x00, //      
	0x78, //  ####
	0x50, //  # # 
	0x28, //   # #
	0x78, //  ####
	0x00, //      
	0x00, //      

	// @728 '{' (5 pixels wide)
	0x10, //    # 
	0x20, //   #  
	0x20, //   #  
	0x60, //  ##  
	0x20, //   #  
	0x20, //   #  
	0x10, //    # 
	0x00, //      

	// @736 '|' (5 pixels wide)
	0x20, //   #  
	0x20, //   #  
	0x20, //   #  
	0x20, //   #  
	0x20, //   #  
	0x20, //   #  
	0x20, //   #  
	0x00, //      

	// @744 '}' (5 pixels wide)
	0x40, //  #   
	0x20, //   #  
	0x20, //   #  
	0x30, //   ## 
	0x20, //   #  
	0x20, //   #  
	0x40, //  #   
	0x00, //      

	// @752 '~' (5 pixels wide)
	0x00, //      
	0x00, //      
	0x00, //      
	0x28, //   # #
	0x50, //  # # 
	0x00, //      
	0x00, //      
	0x00, //      
};

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

void OLED_1in3_WriteData(uint8_t cmd,uint8_t r_data)
{
 
	SSD1306_Buffer[0] = 0x00;
	SSD1306_Buffer[1] = 0x11;
	SSD1306_Buffer[2] = 0x22;
	SSD1306_Buffer[3] = 0x33;
	info.IIC_SLAddr 	= 0x3C;
	info.IIC_SUBADDR 	= 0x40;
//	info.IIC_Length 	= 0x04+0x01;
	info.IIC_Length 	= (sizeof(Font8_Table) / sizeof(char)) +0x01;
	info.data =  (unsigned char *) &Font8_Table[0];
	
	g_u8MstEndFlag = 0;

   /* I2C function to write data to slave */
   s_I2C0HandlerFn = (I2C_FUNC)I2C_MasterTxData;

   /* I2C as master sends START signal */
   I2C_SET_CONTROL_REG(I2C0, I2C_I2CON_STA);
   /* Wait I2C Tx Finish */
   while(g_u8MstEndFlag == 0);
   g_u8MstEndFlag = 0;

}



static void ssd1306_WriteCommand(unsigned char cmd)
{
	OLED_1in3_WriteCmd(0x00,cmd);//--turn off oled panel
}

void systemOledWriteFunc( unsigned char  *pString, unsigned char length )
{
//	while(--length)  // ??? ?? ??? ?? ??.
//	{
//		SSD1306_Buffer[i++] = *pString++;	
//	}

	info.IIC_SLAddr 	= 0x3C;
	info.IIC_SUBADDR 	= 0x40;
	info.IIC_Length 	= length+0x01;
	info.data =  pString;


	/* I2C function to write data to slave */
	s_I2C0HandlerFn = (I2C_FUNC)I2C_MasterTxData;

	/* I2C as master sends START signal */
	I2C_SET_CONTROL_REG(I2C0, I2C_I2CON_STA);
	/* Wait I2C Tx Finish */
	while(g_u8MstEndFlag == 0);
	g_u8MstEndFlag = 0;



//	OLED_1in3_WriteData(0x40,0x50);//--turn off oled panel
}


uint8_t ssd1306_Init() 
{

//	I2C0_TS06_WRITE(0x00, 0xAE);

	ssd1306_WriteCommand(0xAE); //display off
	ssd1306_WriteCommand(0x20); //Set Memory Addressing Mode   
	ssd1306_WriteCommand(0x10); //00,Horizontal Addressing Mode;01,Vertical Addressing Mode;10,Page Addressing Mode (RESET);11,Invalid
	ssd1306_WriteCommand(0xB0); //Set Page Start Address for Page Addressing Mode,0-7
	ssd1306_WriteCommand(0xC8); //Set COM Output Scan Direction
	ssd1306_WriteCommand(0x00); //---set low column address
	ssd1306_WriteCommand(0x10); //---set high column address
	ssd1306_WriteCommand(0x40); //--set start line address
	ssd1306_WriteCommand(0x81); //--set contrast control register
	ssd1306_WriteCommand(0xFF);
	ssd1306_WriteCommand(0xA1); //--set segment re-map 0 to 127
	ssd1306_WriteCommand(0xA6); //--set normal display
	ssd1306_WriteCommand(0xA8); //--set multiplex ratio(1 to 64)
	ssd1306_WriteCommand(0x3F); //
	ssd1306_WriteCommand(0xA4); //0xa4,Output follows RAM content;0xa5,Output ignores RAM content
	ssd1306_WriteCommand(0xD3); //-set display offset
	ssd1306_WriteCommand(0x00); //-not offset
	ssd1306_WriteCommand(0xD5); //--set display clock divide ratio/oscillator frequency
	ssd1306_WriteCommand(0xF0); //--set divide ratio
	ssd1306_WriteCommand(0xD9); //--set pre-charge period
	ssd1306_WriteCommand(0x22); //
	ssd1306_WriteCommand(0xDA); //--set com pins hardware configuration
	ssd1306_WriteCommand(0x12);
	ssd1306_WriteCommand(0xDB); //--set vcomh
	ssd1306_WriteCommand(0x20); //0x20,0.77xVcc
	ssd1306_WriteCommand(0x8D); //--set DC-DC enable
	ssd1306_WriteCommand(0x14); //
	ssd1306_WriteCommand(0xAF); //--turn on SSD1306 panel
	
	ssd1306_Fill(Black);
	
	/* Update screen */
//	ssd1306_UpdateScreen();
	
	/* Set default values */
	SSD1306.CurrentX = 0;
	SSD1306.CurrentY = 0;
	
	/* Initialized OK */
	SSD1306.Initialized = 1;
	return 1;
	
}

//
//	We zetten de hele buffer op een bepaalde kleur
// 	color 	=> de kleur waarin alles moet
//
void ssd1306_Fill(SSD1306_COLOR color) 
{
	/* Set memory */
	uint32_t i;

	for(i = 0; i < sizeof(SSD1306_Buffer); i++)
	{
		SSD1306_Buffer[i] = (color == Black) ? 0x00 : 0xFF;
	}


}

//
//	Alle weizigingen in de buffer naar het scherm sturen
//
void ssd1306_UpdateScreen(void) 
{
	uint8_t i;

	for (i = 0; i < 8; i++) {
		ssd1306_WriteCommand(0xB0 + i);
		ssd1306_WriteCommand(0x00);
		ssd1306_WriteCommand(0x10);

		// We schrijven alles map per map weg
		//HAL_I2C_Mem_Write(SSD1306_I2C_PORT,SSD1306_I2C_ADDR,0x40,1,&SSD1306_Buffer[SSD1306_WIDTH * i],SSD1306_WIDTH,100);
		systemOledWriteFunc( &SSD1306_Buffer[SSD1306_WIDTH * i], SSD1306_WIDTH);
	}
}

//
//	1 pixel op het scherm tekenen
//	X => X coordinaat
//	Y => Y coordinaat
//	color => kleur die pixel moet krijgen
//
void ssd1306_DrawPixel(uint8_t x, uint8_t y, SSD1306_COLOR color)
{
	if (x >= SSD1306_WIDTH || y >= SSD1306_HEIGHT) 
	{
		// We gaan niet buiten het scherm schrijven
		return;
	}
	
	// Kijken of de pixel geinverteerd moet worden
	if (SSD1306.Inverted) 
	{
		color = (SSD1306_COLOR)!color;
	}
	
	// We zetten de juiste kleur voor de pixel
	if (color == White)
	{
		SSD1306_Buffer[x + (y / 8) * SSD1306_WIDTH] |= 1 << (y % 8);
	} 
	else 
	{
		SSD1306_Buffer[x + (y / 8) * SSD1306_WIDTH] &= ~(1 << (y % 8));
	}
}


//
//	We willen 1 char naar het scherm sturen
//	ch 		=> char om weg te schrijven
//	Font 	=> Font waarmee we gaan schrijven
//	color 	=> Black or White
//
char ssd1306_WriteChar(char ch, FontDef Font, SSD1306_COLOR color)
{
	uint32_t i, b, j;
	
	// Kijken of er nog plaats is op deze lijn
	if (SSD1306_WIDTH <= (SSD1306.CurrentX + Font.FontWidth) ||
		SSD1306_HEIGHT <= (SSD1306.CurrentY + Font.FontHeight))
	{
		// Er is geen plaats meer
		return 0;
	}
	
	// We gaan door het font
	for (i = 0; i < Font.FontHeight; i++)
	{
		b = Font.data[(ch - 32) * Font.FontHeight + i];
		for (j = 0; j < Font.FontWidth; j++)
		{
			if ((b << j) & 0x8000) 
			{
				ssd1306_DrawPixel(SSD1306.CurrentX + j, (SSD1306.CurrentY + i), (SSD1306_COLOR) color);
			} 
			else 
			{
				ssd1306_DrawPixel(SSD1306.CurrentX + j, (SSD1306.CurrentY + i), (SSD1306_COLOR)!color);
			}
		}
	}
	
	// De huidige positie is nu verplaatst
	SSD1306.CurrentX += Font.FontWidth;
	
	// We geven het geschreven char terug voor validatie
	return ch;
}

//
//	Functie voor het wegschrijven van een hele string
// 	str => string om op het scherm te schrijven
//	Font => Het font dat gebruikt moet worden
//	color => Black or White
//
char ssd1306_WriteString(char* str, FontDef Font, SSD1306_COLOR color)
{
	// We schrijven alle char tot een nulbyte
	while (*str) 
	{
		if (ssd1306_WriteChar(*str, Font, color) != *str)
		{
			// Het karakter is niet juist weggeschreven
			return *str;
		}
		
		// Volgende char
		str++;
	}
	
	// Alles gelukt, we sturen dus 0 terug
	return *str;
}

//
//	Zet de cursor op een coordinaat
//
void ssd1306_SetCursor(uint8_t x, uint8_t y) 
{
	/* Set write pointers */
	SSD1306.CurrentX = x;
	SSD1306.CurrentY = y;
}

void ssd1306_DrawLine(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, SSD1306_COLOR c)
{
	int16_t dx, dy, sx, sy, err, e2, i, tmp; 
	
	/* Check for overflow */
	if (x0 >= SSD1306_WIDTH) {
		x0 = SSD1306_WIDTH - 1;
	}
	if (x1 >= SSD1306_WIDTH) {
		x1 = SSD1306_WIDTH - 1;
	}
	if (y0 >= SSD1306_HEIGHT) {
		y0 = SSD1306_HEIGHT - 1;
	}
	if (y1 >= SSD1306_HEIGHT) {
		y1 = SSD1306_HEIGHT - 1;
	}
	
	dx = (x0 < x1) ? (x1 - x0) : (x0 - x1); 
	dy = (y0 < y1) ? (y1 - y0) : (y0 - y1); 
	sx = (x0 < x1) ? 1 : -1; 
	sy = (y0 < y1) ? 1 : -1; 
	err = ((dx > dy) ? dx : -dy) / 2; 

	if (dx == 0) {
		if (y1 < y0) {
			tmp = y1;
			y1 = y0;
			y0 = tmp;
		}
		
		if (x1 < x0) {
			tmp = x1;
			x1 = x0;
			x0 = tmp;
		}
		
		/* Vertical line */
		for (i = y0; i <= y1; i++) {
			ssd1306_DrawPixel(x0, i, c);
		}
		
		/* Return from function */
		return;
	}
	
	if (dy == 0) {
		if (y1 < y0) {
			tmp = y1;
			y1 = y0;
			y0 = tmp;
		}
		
		if (x1 < x0) {
			tmp = x1;
			x1 = x0;
			x0 = tmp;
		}
		
		/* Horizontal line */
		for (i = x0; i <= x1; i++) {
			ssd1306_DrawPixel(i, y0, c);
		}
		
		/* Return from function */
		return;
	}
	
	while (1) {
		ssd1306_DrawPixel(x0, y0, c); 
		if (x0 == x1 && y0 == y1) {
			break;
		}
		e2 = err; 
		if (e2 > -dx) {
			err -= dy;
			x0 += sx;
		} 
		if (e2 < dy) {
			err += dx;
			y0 += sy;
		} 
	}
}

void ssd1306_DrawRectangle(uint16_t x, uint16_t y, uint16_t w, uint16_t h, SSD1306_COLOR c)
{
	/* Check input parameters */
	if (
		x >= SSD1306_WIDTH ||
		y >= SSD1306_HEIGHT
	) {
		/* Return error */
		return;
	}
	
	/* Check width and height */
	if ((x + w) >= SSD1306_WIDTH) {
		w = SSD1306_WIDTH - x;
	}
	if ((y + h) >= SSD1306_HEIGHT) {
		h = SSD1306_HEIGHT - y;
	}
	
	/* Draw 4 lines */
	ssd1306_DrawLine(x, y, x + w, y, c);         /* Top line */
	ssd1306_DrawLine(x, y + h, x + w, y + h, c); /* Bottom line */
	ssd1306_DrawLine(x, y, x, y + h, c);         /* Left line */
	ssd1306_DrawLine(x + w, y, x + w, y + h, c); /* Right line */
}

void ssd1306_DrawFilledRectangle(uint16_t x, uint16_t y, uint16_t w, uint16_t h, SSD1306_COLOR c)
{
	uint8_t i;
	
	/* Check input parameters */
	if (
		x >= SSD1306_WIDTH ||
		y >= SSD1306_HEIGHT
	) {
		/* Return error */
		return;
	}
	
	/* Check width and height */
	if ((x + w) >= SSD1306_WIDTH) {
		w = SSD1306_WIDTH - x;
	}
	if ((y + h) >= SSD1306_HEIGHT) {
		h = SSD1306_HEIGHT - y;
	}
	
	/* Draw lines */
	for (i = 0; i <= h; i++) {
		/* Draw lines */
		ssd1306_DrawLine(x, y + i, x + w, y + i, c);
	}
}

void ssd1306_DrawTriangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t x3, uint16_t y3, SSD1306_COLOR color)
{
	/* Draw lines */
	ssd1306_DrawLine(x1, y1, x2, y2, color);
	ssd1306_DrawLine(x2, y2, x3, y3, color);
	ssd1306_DrawLine(x3, y3, x1, y1, color);
}


void ssd1306_DrawFilledTriangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t x3, uint16_t y3, SSD1306_COLOR color)
{
	int16_t deltax = 0, deltay = 0, x = 0, y = 0, xinc1 = 0, xinc2 = 0, 
	yinc1 = 0, yinc2 = 0, den = 0, num = 0, numadd = 0, numpixels = 0, 
	curpixel = 0;
	
	deltax = ABS(x2 - x1);
	deltay = ABS(y2 - y1);
	x = x1;
	y = y1;

	if (x2 >= x1) {
		xinc1 = 1;
		xinc2 = 1;
	} else {
		xinc1 = -1;
		xinc2 = -1;
	}

	if (y2 >= y1) {
		yinc1 = 1;
		yinc2 = 1;
	} else {
		yinc1 = -1;
		yinc2 = -1;
	}

	if (deltax >= deltay){
		xinc1 = 0;
		yinc2 = 0;
		den = deltax;
		num = deltax / 2;
		numadd = deltay;
		numpixels = deltax;
	} else {
		xinc2 = 0;
		yinc1 = 0;
		den = deltay;
		num = deltay / 2;
		numadd = deltax;
		numpixels = deltay;
	}

	for (curpixel = 0; curpixel <= numpixels; curpixel++) {
		ssd1306_DrawLine(x, y, x3, y3, color);

		num += numadd;
		if (num >= den) {
			num -= den;
			x += xinc1;
			y += yinc1;
		}
		x += xinc2;
		y += yinc2;
	}
}

void ssd1306_DrawCircle(int16_t x0, int16_t y0, int16_t r, SSD1306_COLOR c)
{
	int16_t f = 1 - r;
	int16_t ddF_x = 1;
	int16_t ddF_y = -2 * r;
	int16_t x = 0;
	int16_t y = r;

    ssd1306_DrawPixel(x0, y0 + r, c);
    ssd1306_DrawPixel(x0, y0 - r, c);
    ssd1306_DrawPixel(x0 + r, y0, c);
    ssd1306_DrawPixel(x0 - r, y0, c);

    while (x < y) {
        if (f >= 0) {
            y--;
            ddF_y += 2;
            f += ddF_y;
        }
        x++;
        ddF_x += 2;
        f += ddF_x;

        ssd1306_DrawPixel(x0 + x, y0 + y, c);
        ssd1306_DrawPixel(x0 - x, y0 + y, c);
        ssd1306_DrawPixel(x0 + x, y0 - y, c);
        ssd1306_DrawPixel(x0 - x, y0 - y, c);

        ssd1306_DrawPixel(x0 + y, y0 + x, c);
        ssd1306_DrawPixel(x0 - y, y0 + x, c);
        ssd1306_DrawPixel(x0 + y, y0 - x, c);
        ssd1306_DrawPixel(x0 - y, y0 - x, c);
    }
}

void ssd1306_DrawFilledCircle(int16_t x0, int16_t y0, int16_t r, SSD1306_COLOR c)
{
	int16_t f = 1 - r;
	int16_t ddF_x = 1;
	int16_t ddF_y = -2 * r;
	int16_t x = 0;
	int16_t y = r;

    ssd1306_DrawPixel(x0, y0 + r, c);
    ssd1306_DrawPixel(x0, y0 - r, c);
    ssd1306_DrawPixel(x0 + r, y0, c);
    ssd1306_DrawPixel(x0 - r, y0, c);
    ssd1306_DrawLine(x0 - r, y0, x0 + r, y0, c);

    while (x < y) {
        if (f >= 0) {
            y--;
            ddF_y += 2;
            f += ddF_y;
        }
        x++;
        ddF_x += 2;
        f += ddF_x;

        ssd1306_DrawLine(x0 - x, y0 + y, x0 + x, y0 + y, c);
        ssd1306_DrawLine(x0 + x, y0 - y, x0 - x, y0 - y, c);

        ssd1306_DrawLine(x0 + y, y0 + x, x0 - y, y0 + x, c);
        ssd1306_DrawLine(x0 + y, y0 - x, x0 - y, y0 - x, c);
    }
}

void ssd1306_ON(void)
{
	ssd1306_WriteCommand(0x8D);  
	ssd1306_WriteCommand(0x14);  
	ssd1306_WriteCommand(0xAF);  
}

void ssd1306_OFF(void)
{
	ssd1306_WriteCommand(0x8D);  
	ssd1306_WriteCommand(0x10);
	ssd1306_WriteCommand(0xAE);  
}

void oledShowString(uint8_t px, uint8_t py, char* strMsg, uint8_t Font)
{
    ssd1306_SetCursor(px, py);
	ssd1306_WriteString(strMsg,getFontTblData(Font), White); 
	ssd1306_UpdateScreen();	
}

void oledTest(void)
{
	ssd1306_SetCursor(0, 0);
	ssd1306_WriteString("WIZIO",getFontTblData(FONT_TABLE_11X18), White);
	ssd1306_SetCursor(0, 18);
	ssd1306_WriteString("DVIEW",getFontTblData(FONT_TABLE_11X18), White);
	ssd1306_SetCursor(0, 36);
	ssd1306_WriteString("DVIEW",getFontTblData(FONT_TABLE_11X18), White);
	ssd1306_SetCursor(0, 54);
	ssd1306_WriteString("LCJ TEST",getFontTblData(FONT_TABLE_11X18), White);
	ssd1306_UpdateScreen();
}



/*******************************************************************************
function:
		Common register initialization
*******************************************************************************/
static void OLED_1in3_InitReg(void)
{
    ssd1306_WriteCommand(0xAE);//--turn off oled panel

    ssd1306_WriteCommand(0x02);//---set low column address
    ssd1306_WriteCommand(0x10);//---set high column address

    ssd1306_WriteCommand(0x40);//--set start line address  Set Mapping RAM Display Start Line (0x00~0x3F)
    ssd1306_WriteCommand(0x81);//--set contrast control register
    ssd1306_WriteCommand(0xA0);//--Set SEG/Column Mapping a0/a1
    ssd1306_WriteCommand(0xC0);//Set COM/Row Scan Direction
    ssd1306_WriteCommand(0xA6);//--set normal display a6/a7
    ssd1306_WriteCommand(0xA8);//--set multiplex ratio(1 to 64)
    ssd1306_WriteCommand(0x3F);//--1/64 duty
    ssd1306_WriteCommand(0xD3);//-set display offset    Shift Mapping RAM Counter (0x00~0x3F)
    ssd1306_WriteCommand(0x00);//-not offset
    ssd1306_WriteCommand(0xd5);//--set display clock divide ratio/oscillator frequency
    ssd1306_WriteCommand(0x80);//--set divide ratio, Set Clock as 100 Frames/Sec
    ssd1306_WriteCommand(0xD9);//--set pre-charge period
    ssd1306_WriteCommand(0xF1);//Set Pre-Charge as 15 Clocks & Discharge as 1 Clock
    ssd1306_WriteCommand(0xDA);//--set com pins hardware configuration
    ssd1306_WriteCommand(0x12);
    ssd1306_WriteCommand(0xDB);//--set vcomh
    ssd1306_WriteCommand(0x40);//Set VCOM Deselect Level
    ssd1306_WriteCommand(0x20);//-Set Page Addressing Mode (0x00/0x01/0x02)
    ssd1306_WriteCommand(0x02);//
    ssd1306_WriteCommand(0xA4);// Disable Entire Display On (0xa4/0xa5)
    ssd1306_WriteCommand(0xA6);// Disable Inverse Display On (0xa6/a7)

}

/********************************************************************************
function:
            initialization
********************************************************************************/
void OLED_1IN3_Init()
{
    //Hardware reset
 //   OLED_1in3_Reset();

    //Set the initialization register
    OLED_1in3_InitReg();
    Delay_1ms(200);

    //Turn on the OLED display
    ssd1306_WriteCommand(0xaf);
}

/********************************************************************************
function:
			Clear screen
********************************************************************************/
void OLED_1IN3_Clear()
{
    // UWORD Width, Height;
    UWORD i, j;
    // Width = (OLED_1IN3_WIDTH % 8 == 0)? (OLED_1IN3_WIDTH / 8 ): (OLED_1IN3_WIDTH / 8 + 1);
    // Height = OLED_1IN3_HEIGHT; 
    for (i=0; i<8; i++) {
        /* set page address */
        ssd1306_WriteCommand(0xB0 + i);
        /* set low column address */
        ssd1306_WriteCommand(0x02);
        /* set high column address */
        ssd1306_WriteCommand(0x10);
        for(j=0; j<128; j++) {
            /* write data */
            OLED_1in3_WriteData(0x00,0x00);  
        }
    }
}

/********************************************************************************
function:	Update memory to OLED
********************************************************************************/
void OLED_1IN3_Display(const UBYTE *Image)
{
    UWORD page, column, temp;

    for (page=0; page<8; page++) {
        /* set page address */
        ssd1306_WriteCommand(0xB0 + page);
        /* set low column address */
        ssd1306_WriteCommand(0x02);
        /* set high column address */
        ssd1306_WriteCommand(0x10);

        /* write data */
        for(column=0; column<128; column++) {
            temp = Image[(7-page) + column*8];
            OLED_1in3_WriteData(0x00,temp);
        }       
    }
}




void oledScreenClear(void)
{
	ssd1306_Fill(Black);
	ssd1306_UpdateScreen();	
}

int OLED_1in3_test(void)
{
	unsigned char *BlackImage;
	UWORD Imagesize;
	
	OLED_1IN3_Init();
	Delay_1ms(500);	
	// 0.Create a new image cache
	Imagesize = ((OLED_1IN3_WIDTH%8==0)? (OLED_1IN3_WIDTH/8): (OLED_1IN3_WIDTH/8+1)) * OLED_1IN3_HEIGHT;
	if((BlackImage = (UBYTE *)malloc(Imagesize)) == NULL) {
			printf("Failed to apply for black memory...\r\n");
			return -1;
	}
	printf("Paint_NewImage\r\n");
	Paint_NewImage(BlackImage, OLED_1IN3_WIDTH, OLED_1IN3_HEIGHT, 90, BLACK);	

	printf("Drawing\r\n");
	//1.Select Image
	Paint_SelectImage(BlackImage);
	Delay_1ms(500);
//	Paint_Clear(BLACK);
	while(1) {
		
		// 2.Drawing on the image		
		printf("Drawing:page 1\r\n");
		Paint_DrawPoint(20, 10, WHITE, DOT_PIXEL_1X1, DOT_STYLE_DFT);
		Paint_DrawPoint(30, 10, WHITE, DOT_PIXEL_2X2, DOT_STYLE_DFT);
		Paint_DrawPoint(40, 10, WHITE, DOT_PIXEL_3X3, DOT_STYLE_DFT);
		Paint_DrawLine(10, 10, 10, 20, WHITE, DOT_PIXEL_1X1, LINE_STYLE_SOLID);
		Paint_DrawLine(20, 20, 20, 30, WHITE, DOT_PIXEL_1X1, LINE_STYLE_SOLID);
		Paint_DrawLine(30, 30, 30, 40, WHITE, DOT_PIXEL_1X1, LINE_STYLE_DOTTED);
		Paint_DrawLine(40, 40, 40, 50, WHITE, DOT_PIXEL_1X1, LINE_STYLE_DOTTED);
		Paint_DrawCircle(60, 30, 15, WHITE, DOT_PIXEL_1X1, DRAW_FILL_EMPTY);
		Paint_DrawCircle(100, 40, 20, WHITE, DOT_PIXEL_1X1, DRAW_FILL_FULL);			
		Paint_DrawRectangle(50, 30, 60, 40, WHITE, DOT_PIXEL_1X1, DRAW_FILL_EMPTY);
		Paint_DrawRectangle(90, 30, 110, 50, BLACK, DOT_PIXEL_1X1, DRAW_FILL_FULL);		
		// 3.Show image on page1
		OLED_1IN3_Display(BlackImage);
		Delay_1ms(2000);			
		Paint_Clear(BLACK);
		
		// Drawing on the image
		printf("Drawing:page 2\r\n");			
//		Paint_DrawString_EN(10, 0, "waveshare", &Font16, WHITE, WHITE);
//		Paint_DrawString_EN(10, 17, "hello world", &Font8, WHITE, WHITE);
//		Paint_DrawNum(10, 30, 123.456789, &Font8, 4, WHITE, WHITE);
//		Paint_DrawNum(10, 43, 987654, &Font12, 5, WHITE, WHITE);
		// Show image on page2
		OLED_1IN3_Display(BlackImage);
		Delay_1ms(2000);	
		Paint_Clear(BLACK);		
		
		// Drawing on the image
//		printf("Drawing:page 3\r\n");
//		Paint_DrawString_CN(10, 0,"ãºÃAbc", &ÄFont12CN, WHITE, WHITE);
//		Paint_DrawString_CN(0, 20, "Î¢Ñ©µç×Ó", &Font24CN, WHITE, WHITE);
		// Show image on page3
		OLED_1IN3_Display(BlackImage);
		Delay_1ms(2000);		
		Paint_Clear(BLACK);	

		// Drawing on the image
		printf("Drawing:page 4\r\n");
		OLED_1IN3_Display(gImage_1in3);
		Delay_1ms(2000);		
		Paint_Clear(BLACK);	

	}
}



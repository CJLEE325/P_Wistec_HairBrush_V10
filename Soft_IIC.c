#include "Soft_IIC.h"
#include "pub_ram.h"
#include "sys_const.h"
#include "stdio.h"

#define CII_TryCnt                      5  /* Try 10 times                     */
#define __IICCheckACK__


void iic_init(void)
{
	__IIC_SCL_SET();
	__IIC_SDA_SET();
	CLK_SysTickDelay(1000);
}
/******************************************************************************/
/* Function: SDAHigh = IIC Data Line Set To High.                             */
/******************************************************************************/
void SDAHigh()
{
	IIC_SOFT_SDA_PIN = TRUE;
}

/******************************************************************************/
/* Function: SDALow  = IIC Data Line Set To Low..                             */
/******************************************************************************/
void SDALow()
{
	IIC_SOFT_SDA_PIN = FALSE;
}
/******************************************************************************/
/* Function: SCLLow  = IIC Clock Line Set To Low.                             */
/******************************************************************************/
void SCLLow()
{
	IIC_SOFT_SCL_PIN = FALSE;

}
/******************************************************************************/
/* Function: SCLHigh  = IIC Clock Line Set To Low.                             */
/******************************************************************************/
void SCLHigh()
{
	IIC_SOFT_SCL_PIN = TRUE;
}
/******************************************************************************/
/* Function: GetSDA1  = ACK check                                               */
/******************************************************************************/
unsigned char GetSDA1()
{
//	__IIC_SDA_IN(); 
	__IIC_SDA_READ();


}
/******************************************************************************/
/* Function: GetSCL  = ACK check                                               */
/******************************************************************************/
unsigned char GetSCL1()
{

//	P3 -> PMD = (P3->PMD & (~GPIO_PMD_PMD4_Msk)) | (GPIO_PMD_INPUT << GPIO_PMD_PMD4_Pos);
	return  IIC_SOFT_SCL_PIN; 
}

/******************************************************************************/
/* Function: SCLHighChk = IIC Clock Line Set To High.                            */
/******************************************************************************/
unsigned char SCLHighChk()                 /*                                    */
{                             /*                                    */
	 SCLHigh();                /*                                    */
//	 for(i=0; i<2; i++)         /*                                    */
//	 {                        /*                                    */
	  if ( !GetSCL1())      /*                                    */
		return FALSE;       /*                                    */
//	 };                       /*                                    */
	else
	  return TRUE;                            /*                                    */
}                                       /*                                    */
/******************************************************************************/
/* Function: SII_Start = Generates a start condition                          */
/******************************************************************************/
unsigned char SII_Start()               /*                                    */
{                             /*                                    */
     char      i;
	{
	 	SDAHigh();           /*                                    */
	    if ( !SCLHighChk() )        /*                                    */
	         return FALSE;       /*                                    */
	    for (i = 0; i < 2; i++)  /*                                    */
	    {                        /*                                    */
	    	SDALow();            /*                                    */
	    };
	    SCLLow();            /*                                    */
     	return TRUE;             /*                                    */
	}

}                             /*                                    */
/******************************************************************************/
/* Function: SII_Stop = Generates a stop condition                            */
/******************************************************************************/
void SII_Stop()               /*                                    */
{                             /*                                    */
     char      i;
	{
	 	SCLLow();            /*                                    */
	    SDALow();            /*                                    */
	    for (i = 0; i < 2; i++)  /*                                    */
	    {                        /*                                    */
	    	SCLHigh();           /*                                    */
	    };
     	SDAHigh();           /*                                    */
	}
}                             /*                                    */

/******************************************************************************/
/* Function: SII_Tx = Transmits one data (in master transmitter mode)         */
/* Input   : TxData = Data to transmit                                        */
/* Output  : 1 No Acknowledge from Chip or CLK pin is not to HIGH!            */
/*         : 0 Acknowledge Ok!                                                              */
/******************************************************************************/
unsigned char SII_Tx(char TxData)            /*                                    */
{                                  /*                                    */
    char      i;
    unsigned char      CY;
	
     for (i = 0; i < 8; i++)       /*                                    */
     {                             /*                                    */
          if (TxData & 0x80)       /*                                    */
               SDAHigh();          /*                                    */
          else                     /*                                    */
               SDALow();           /*                                    */
          TxData = TxData << 1;    /*                                    */
                                   /*                                    */
          if ( !SCLHighChk() )        /*                                    */
              return FALSE;       /* Clk is LOW!                        */
          SCLLow();                /*                                    */
     }                             /*                                    */
                                   /*                                    */
//     for(i=0; i<2;i++)              /*                                    */
//    {                             /*                                    */
     	SDAHigh();                 /*                                    */
//     };                            /*                                    */
     if ( !SCLHighChk() )             /*                                    */
          return FALSE;            /* Clk is LOW!                        */

 	 CY = !GetSDA1();              /* Get Acknowledge!                   */

	 SCLLow();                 		/*                                    */
     return CY;                    /* Return Acknowledge!                */
}                                  /*                                    */
/******************************************************************************/
/* Function: SII_Rx  = Reads one byte  data(in master receiver mode)          */
/* Input   : DataCnt = 0 : The Last Data for reading. So there's no need to   */
/*                         send acknowledge to Chip.                          */
/*                     !0: It need to send acknowledge to Chip!               */
/* Output  : RxData  = Received Data                                          */
/******************************************************************************/
unsigned char SII_Rx(unsigned char DataCnt)
{                                       /*                                    */
     unsigned char RxData,cnt;          /*                                    */
     char i;                            /*                                    */

     SDAHigh();                     /*release data line                   */
                                        /*                                    */
     RxData = 0;                        /*                                    */
     for ( cnt = 0; cnt < 8; cnt++)     /*                                    */
     {                                  /*                                    */
          RxData = RxData << 1;         /*                                    */
          SCLHighChk();

		  if ( GetSDA1() )                 /*                                    */
               RxData |= 0x01;          /*                                    */

		  for(i=0;i<3;i++)              /*                                    */
          {                             /*                                    */
              SCLLow();                 /*                                    */
          };                            /*                                    */
     }                                  /*                                    */
     if (DataCnt == 1)                   /*                                    */
        SDAHigh();                /*                                    */
     else
      	SDALow();                      /*                                    */

	 SCLHighChk();                         /*                                    */
     SCLLow();                      /*                                    */
     return  RxData;                    /*                                    */
}                                       /*                                    */
/********************************************************************************/
/* SII_TxRx();                                                                  */
/********************************************************************************/
/* Task   : Send or Read data by IIC port.                                      */
/********************************************************************************/
/* Input  : Slave + Sub + Pointer                                               */
/*        1.rII_Info   Write or Read/Sub or No Sub/....                         */
/*        2.if FII_OtherAddr bit is set to HIGH, followed 2 bytes are needed.   */
/*        - rII_AddrHi Addition Address data high.                              */
/*        - rII_AddrLo Addition Address data low.                               */
/*------------------------------------------------------------------------------*/
/* Output : CY = 1 Success!                                                     */
/*          CY = 0 Error Occured!                                               */
/********************************************************************************/
unsigned char SII_TxRx()
{
     ErrorCnt = CII_TryCnt;
	iic_init();
     do
     {
		if(rII_Info & MORE16)
        {
			SendCnt = rII_NumberOfData;
		}
		else
		{
          	SendCnt = rII_Info & 0x0F;
     		if(SendCnt == 0)
	          	SendCnt = 16;
		}
        if ( SII_Start() )
        {
               if ( SII_Tx(rII_Slave) )
               {
                    if (rII_Info & SUB)
                    {
	                     if ( !SII_Tx(rII_Sub) )
	                        goto    JII_ChkErrCnt;
                    }
                    if(rII_Info & RD)
                    {
                         do
                         {
                              *rII_Pointer++ = SII_Rx(SendCnt);
                         }while(--SendCnt);
                    }
                    else
                    {
                         do
                         {
                              if ( !SII_Tx(*rII_Pointer++) )
                                   goto    JII_ChkErrCnt;
                         }while(--SendCnt);
                    }
                    SII_Stop();
                    return TRUE;
               }
          }
JII_ChkErrCnt:
          SII_Stop();
     }while(ErrorCnt--);
     return FALSE;
}




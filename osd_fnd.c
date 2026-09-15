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


	#define     POSITION_LEFT								0			// 
	#define     POSITION_RIGHT								1			// 
	#define     POSITION_HIGH								1			// 
	#define     POSITION_LOW								0			// 


	#define     P_SEG1								P00			// 
	#define     P_SEG2								P04			// 

	#define     P_FND_A								P03			// 
	#define     P_FND_B								P41			// 
	#define     P_FND_C								P05		// 

	#define     P_FND_D								P07			// 
	#define     P_FND_E								P06			// 
	#define     P_FND_F								P02			// 
	#define     P_FND_G								P01			// 

void 	SF_Disp_FMD_(unsigned int cnt);
void 	SF_Disp_FMD_CLEAR(void);



void	Task_OSDFND_Control(void)
{

}


void 	SF_L_FMD_0(unsigned char position)
{
	if ( position == POSITION_HIGH )
	{
		P_SEG1 = TRUE;
		P_SEG2 = FALSE;
	}
	else
	{
		P_SEG1 = FALSE;
		P_SEG2 = TRUE;
	}
	P_FND_A  = TRUE;
	P_FND_B  = TRUE;
	P_FND_C  = TRUE;
	P_FND_D  = TRUE;
	P_FND_E  = TRUE;
	P_FND_F  = TRUE;
//	P_FND_G  = TRUE;
}

void 	SF_L_FMD_1(unsigned char position)
{
	if ( position == POSITION_HIGH )
	{
		P_SEG1 = TRUE;
		P_SEG2 = FALSE;
	}
	else
	{
		P_SEG1 = FALSE;
		P_SEG2 = TRUE;
	}
//	P_FND_A  = TRUE;
	P_FND_B  = TRUE;
	P_FND_C  = TRUE;
//	P_FND_D  = TRUE;
//	P_FND_E  = TRUE;
//	P_FND_F  = TRUE;
//	P_FND_G  = TRUE;

}

void 	SF_L_FMD_2(unsigned char position)
{
	if ( position == POSITION_HIGH )
	{
		P_SEG1 = TRUE;
		P_SEG2 = FALSE;
	}
	else
	{
		P_SEG1 = FALSE;
		P_SEG2 = TRUE;
	}
	P_FND_A  = TRUE;
	P_FND_B  = TRUE;
//	P_FND_C  = TRUE;
	P_FND_D  = TRUE;
	P_FND_E  = TRUE;
//	P_FND_F  = TRUE;
	P_FND_G  = TRUE;

}

void 	SF_L_FMD_3(unsigned char position)
{
	if ( position == POSITION_HIGH )
	{
		P_SEG1 = TRUE;
		P_SEG2 = FALSE;
	}
	else
	{
		P_SEG1 = FALSE;
		P_SEG2 = TRUE;
	}
	P_FND_A  = TRUE;
	P_FND_B  = TRUE;
	P_FND_C  = TRUE;
	P_FND_D  = TRUE;
//	P_FND_E  = TRUE;
//	P_FND_F  = TRUE;
	P_FND_G  = TRUE;

}

void 	SF_L_FMD_4(unsigned char position)
{
	if ( position == POSITION_HIGH )
	{
		P_SEG1 = TRUE;
		P_SEG2 = FALSE;
	}
	else
	{
		P_SEG1 = FALSE;
		P_SEG2 = TRUE;
	}
//	P_FND_A  = TRUE;
	P_FND_B  = TRUE;
	P_FND_C  = TRUE;
//	P_FND_D  = TRUE;
//	P_FND_E  = TRUE;
	P_FND_F  = TRUE;
	P_FND_G  = TRUE;

}
void 	SF_L_FMD_5(unsigned char position)
{
	if ( position == POSITION_HIGH )
	{
		P_SEG1 = TRUE;
		P_SEG2 = FALSE;
	}
	else
	{
		P_SEG1 = FALSE;
		P_SEG2 = TRUE;
	}
	P_FND_A  = TRUE;
//	P_FND_B  = TRUE;
	P_FND_C  = TRUE;
	P_FND_D  = TRUE;
//	P_FND_E  = TRUE;
	P_FND_F  = TRUE;
	P_FND_G  = TRUE;

}
void 	SF_L_FMD_6(unsigned char position)
{
	if ( position == POSITION_HIGH )
	{
		P_SEG1 = TRUE;
		P_SEG2 = FALSE;
	}
	else
	{
		P_SEG1 = FALSE;
		P_SEG2 = TRUE;
	}

	P_FND_A  = TRUE;
//	P_FND_B  = TRUE;
	P_FND_C  = TRUE;
	P_FND_D  = TRUE;
	P_FND_E  = TRUE;
	P_FND_F  = TRUE;
	P_FND_G  = TRUE;

}

void 	SF_L_FMD_7(unsigned char position)
{
	if ( position == POSITION_HIGH )
	{
		P_SEG1 = TRUE;
		P_SEG2 = FALSE;
	}
	else
	{
		P_SEG1 = FALSE;
		P_SEG2 = TRUE;
	}

	P_FND_A  = TRUE;
	P_FND_B  = TRUE;
	P_FND_C  = TRUE;
//	P_FND_D  = TRUE;
//	P_FND_E  = TRUE;
	P_FND_F  = TRUE;
//	P_FND_G  = TRUE;

}
void 	SF_L_FMD_8(unsigned char position)
{
	if ( position == POSITION_HIGH )
	{
		P_SEG1 = TRUE;
		P_SEG2 = FALSE;
	}
	else
	{
		P_SEG1 = FALSE;
		P_SEG2 = TRUE;
	}
	P_FND_A  = TRUE;
	P_FND_B  = TRUE;
	P_FND_C  = TRUE;
	P_FND_D  = TRUE;
	P_FND_E  = TRUE;
	P_FND_F  = TRUE;
	P_FND_G  = TRUE;

}
void 	SF_L_FMD_9(unsigned char position)
{
	if ( position == POSITION_HIGH )
	{
		P_SEG1 = TRUE;
		P_SEG2 = FALSE;
	}
	else
	{
		P_SEG1 = FALSE;
		P_SEG2 = TRUE;
	}
	P_FND_A  = TRUE;
	P_FND_B  = TRUE;
	P_FND_C  = TRUE;
	P_FND_D  = TRUE;
//	P_FND_E  = TRUE;
	P_FND_F  = TRUE;
	P_FND_G  = TRUE;

}


void 	SF_Disp_FMD_(unsigned int cnt)
{

	static  unsigned char		Toggle;
	static  unsigned char  i,j;
	i  = (unsigned char)cnt/ 10; 
	j  = (unsigned char)cnt % 10; 


	SF_Disp_FMD_CLEAR();
	Toggle ^= TRUE;
	
	if ( Toggle )
	{
		switch ( i )
		{
			case 0:
				SF_L_FMD_0(POSITION_HIGH);	
				break;
			case 1:
				SF_L_FMD_1(POSITION_HIGH);	
				break;
			case 2:
				SF_L_FMD_2(POSITION_HIGH);	
				break;
			case 3:
				SF_L_FMD_3(POSITION_HIGH);	
				break;

			case 4:
				SF_L_FMD_4(POSITION_HIGH);	
				break;
			case 5:
				SF_L_FMD_5(POSITION_HIGH);	
				break;
			case 6:
				SF_L_FMD_6(POSITION_HIGH);	
				break;
			case 7:
				SF_L_FMD_7(POSITION_HIGH);	
				break;
			case 8:
				SF_L_FMD_8(POSITION_HIGH);	
				break;

			default:
				SF_L_FMD_9(POSITION_HIGH);	
			break;
		};	
	}
	else
	{		
		switch ( j )
		{
			case 0:
				SF_L_FMD_0(POSITION_LOW);	
				break;
			case 1:
				SF_L_FMD_1(POSITION_LOW);	
				break;
			case 2:
				SF_L_FMD_2(POSITION_LOW);	
				break;
			case 3:
				SF_L_FMD_3(POSITION_LOW);	
				break;
			case 4:
				SF_L_FMD_4(POSITION_LOW);	
				break;

			case 5:
				SF_L_FMD_5(POSITION_LOW);	
				break;
			case 6:
				SF_L_FMD_6(POSITION_LOW);	
				break;
			case 7:
				SF_L_FMD_7(POSITION_LOW);	
				break;

			case 8:
				SF_L_FMD_8(POSITION_LOW);	
				break;
			case 9:
				SF_L_FMD_9(POSITION_LOW);	
				break;

			default:
				break;
		};	
	}
}


void 	SF_Disp_FMD_BAR(void)
{
	static  unsigned char		Toggle;

	Toggle ^= TRUE;
	
	if ( Toggle )
	{
		P_SEG1 = TRUE;
		P_SEG2 = FALSE;

	}
	else
	{
		P_SEG1 = FALSE;
		P_SEG2 = TRUE;
	}
	
	P_FND_A  = FALSE;
	P_FND_B  = FALSE;
	P_FND_C  = FALSE;
	P_FND_D  = FALSE;
	P_FND_E  = FALSE;
	P_FND_F  = FALSE;
	P_FND_G  = TRUE;


}


void 	SF_Disp_FMD_CLEAR(void)
{
		P_FND_A  = FALSE;
		P_FND_B  = FALSE;
		P_FND_C  = FALSE;
		P_FND_D  = FALSE;
		P_FND_E  = FALSE;
		P_FND_F  = FALSE;
		P_FND_G  = FALSE;

}


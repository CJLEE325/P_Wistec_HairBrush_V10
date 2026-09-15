#ifndef _SOFT_IIC_H_
#define _SOFT_IIC_H_

#include "NUC029xAN.h"

#define IIC_SOFT_SCL_PIN         P35
#define IIC_SOFT_SDA_PIN         P34

#define IIC_SOFT_SCL_GPIO        GPIO3
#define IIC_SOFT_SDA_GPIO        GPIO3


#define __IIC_SCL_SET()     IIC_SOFT_SCL_PIN = TRUE
#define __IIC_SCL_CLR()     IIC_SOFT_SCL_PIN = FALSE

#define __IIC_SDA_SET()		IIC_SOFT_SDA_PIN = TRUE
#define __IIC_SDA_CLR()     IIC_SOFT_SDA_PIN = FALSE

#define __IIC_SDA_IN()     	P3 -> PMD = (P3->PMD & (~GPIO_PMD_PMD4_Msk)) | (GPIO_PMD_INPUT << GPIO_PMD_PMD4_Pos)   	// P34  P_O_SDA
#define __IIC_SDA_OUT() 	P3 ->PMD = (P3->PMD & (~GPIO_PMD_PMD4_Msk)) | (GPIO_PMD_OUTPUT << GPIO_PMD_PMD4_Pos)   	// P34  P_O_SDA

#define __IIC_SDA_READ()     P34 


#define   SetSDA      __IIC_SDA_SET()
#define   ResSDA      __IIC_SDA_CLR()
//#define   GetSDA      __IIC_SDA_IN; __IIC_SDA_READ()
#define   SetSCL      __IIC_SCL_SET()
#define   ResSCL      __IIC_SCL_CLR()




extern void iic_init(void);
extern void iic_start(void);
extern void iic_stop(void);
extern int iic_wait_for_ack(void);
extern void iic_write_byte(unsigned char chData);

#endif


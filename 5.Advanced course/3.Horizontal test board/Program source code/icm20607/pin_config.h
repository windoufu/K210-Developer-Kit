/**
* @par  Copyright (C): 2016-2022, Shenzhen Yahboom Tech
* @file         pin_config.c
* @author       Gengyue
* @version      V1.0
* @date         2020.05.27
* @brief        Definition of hardware pin and software GPIO
* @details      
* @par History  Description below
*                 
* version:		Since K210 uses fpioa field programmable IO array, 
which allows users to map 255 internal functions to 48 free IOs on the chip periphery,
we set the hardware IO and software GPIO functions separately 
to make it easier to understand.
*/
#ifndef _PIN_CONFIG_H_
#define _PIN_CONFIG_H_
/*****************************HEAR-FILE************************************/
#include "fpioa.h"
#define MAIX_DOCK 0

/*****************************HARDWARE-PIN*********************************/
//Hardware IO port, corresponding Schematic 
#define PIN_ICM_SCL             (9)
#define PIN_ICM_SDA             (10)
#define PIN_ICM_INT             (11)

#define PIN_LCD_CS              (36)
#define PIN_LCD_RST             (37)
#define PIN_LCD_RS              (38)
#define PIN_LCD_WR              (39)

/*****************************SOICMWARE-GPIO********************************/
//Software GPIO port，corresponding program
#define LCD_RST_GPIONUM         (0)
#define LCD_RS_GPIONUM          (1)

#define ICM_INT_GPIONUM         (2)


/*****************************FUNC-GPIO************************************/
//Function of GPIO port, bound to hardware IO port
#define FUNC_ICM_INT             (FUNC_GPIOHS0 + ICM_INT_GPIONUM)
#define FUNC_ICM_SCL             (FUNC_I2C0_SCLK)
#define FUNC_ICM_SDA             (FUNC_I2C0_SDA)

#define FUNC_LCD_CS              (FUNC_SPI0_SS3)
#define FUNC_LCD_RST             (FUNC_GPIOHS0 + LCD_RST_GPIONUM)
#define FUNC_LCD_RS              (FUNC_GPIOHS0 + LCD_RS_GPIONUM)
#define FUNC_LCD_WR              (FUNC_SPI0_SCLK)

#endif /* _PIN_CONFIG_H_ */

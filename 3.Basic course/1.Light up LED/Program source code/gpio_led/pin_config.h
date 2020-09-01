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
* version:	Since K210 uses fpioa field programmable IO array, 
which allows users to map 255 internal functions to 48 free IOs on the chip periphery,
we set the hardware IO and software GPIO functions separately 
to make it easier to understand.
*/
#ifndef _PIN_CONFIG_H_
#define _PIN_CONFIG_H_
/*****************************HEAR-FILE************************************/
#include "fpioa.h"

/*****************************HARDWARE-PIN*********************************/
//Hardware IO port, corresponding Schematic 
#define PIN_LED_0             (0)
#define PIN_LED_1             (17)

/*****************************SOFTWARE-GPIO********************************/
//Software GPIO port，corresponding program
#define LED0_GPIONUM          (0)
#define LED1_GPIONUM          (1)

/*****************************FUNC-GPIO************************************/
//Function of GPIO port, bound to hardware IO port
#define FUNC_LED0             (FUNC_GPIO0 + LED0_GPIONUM)
#define FUNC_LED1             (FUNC_GPIO0 + LED1_GPIONUM)

#endif /* _PIN_CONFIG_H_ */

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
#define PIN_RGB_R              (6)
#define PIN_RGB_G              (7)
#define PIN_RGB_B              (8)

#define PIN_KEYPAD_LEFT        (1)
#define PIN_KEYPAD_MIDDLE      (2)
#define PIN_KEYPAD_RIGHT       (3)

/*****************************SOFTWARE-GPIO********************************/
//Software GPIO port，corresponding program
#define RGB_R_GPIONUM          (0)
#define RGB_G_GPIONUM          (1)
#define RGB_B_GPIONUM          (2)

#define KEYPAD_LEFT_GPIONUM    (3)
#define KEYPAD_MIDDLE_GPIONUM  (4)
#define KEYPAD_RIGHT_GPIONUM   (5)

/*****************************FUNC-GPIO************************************/
//Function of GPIO port, bound to hardware IO port
#define FUNC_RGB_R             (FUNC_GPIOHS0 + RGB_R_GPIONUM)
#define FUNC_RGB_G             (FUNC_GPIOHS0 + RGB_G_GPIONUM)
#define FUNC_RGB_B             (FUNC_GPIOHS0 + RGB_B_GPIONUM)

#define FUNC_KEYPAD_LEFT       (FUNC_GPIOHS0 + KEYPAD_LEFT_GPIONUM)
#define FUNC_KEYPAD_MIDDLE     (FUNC_GPIOHS0 + KEYPAD_MIDDLE_GPIONUM)
#define FUNC_KEYPAD_RIGHT      (FUNC_GPIOHS0 + KEYPAD_RIGHT_GPIONUM)

#endif /* _PIN_CONFIG_H_ */

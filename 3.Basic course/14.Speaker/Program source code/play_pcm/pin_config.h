/**
* @par  Copyright (C): 2016-2022, Shenzhen Yahboom Tech
* @file         pin_config.c
* @author       Gengyue
* @version      V1.0
* @date         2020.05.27
* @brief        Hardware pins and software GPIO Macro definitions
* @details      
* @par History  following description
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
#define PIN_SPK_WS             (30)
#define PIN_SPK_DATA           (31)
#define PIN_SPK_BCK            (32)

/*****************************SOFTWARE-GPIO********************************/
//Software GPIO port，corresponding program


/*****************************FUNC-GPIO************************************/
//Function of GPIO port, bound to hardware IO po
#define FUNC_SPK_WS            FUNC_I2S2_WS
#define FUNC_SPK_DATA          FUNC_I2S2_OUT_D0
#define FUNC_SPK_BCK           FUNC_I2S2_SCLK

#endif /* _PIN_CONFIG_H_ */

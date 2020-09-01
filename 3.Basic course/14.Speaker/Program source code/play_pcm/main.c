/**
* @par  Copyright (C): 2016-2022, Shenzhen Yahboom Tech
* @file         main.c
* @author       Gengyue
* @version      V1.0
* @date         2020.05.27
* @brief        Speaker play sound
* @details      
* @par History  Description below
*                 
* version:	V1.0: The speaker plays the sound of PCM data once. To play it again, please press the reset key.
*/
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "i2s.h"
#include "sysctl.h"
#include "fpioa.h"
#include "uarths.h"
#include "pcm.h"
#include "gpiohs.h"
#include "pin_config.h"

/**
* Function       hardware_init
* @author        Gengyue
* @date          2020.05.27
* @brief         Hardware initialization, bind GPIO port
* @param[in]     void
* @param[out]    void
* @retval        void
* @par History   NO
*/
void hardware_init(void)
{
    /*
    ** SPK_WS---IO30
    ** SPK_DATA-IO31
    ** SPK_BCK--IO32
    */
    fpioa_set_function(PIN_SPK_WS, FUNC_SPK_WS);
    fpioa_set_function(PIN_SPK_DATA, FUNC_SPK_DATA);
    fpioa_set_function(PIN_SPK_BCK, FUNC_SPK_BCK);
}

uint8_t state = 1;

/**
* Function       main
* @author        Gengyue
* @date          2020.05.27
* @brief         Main function, the entry of the program
* @param[in]     void
* @param[out]    void
* @retval        0
* @par History   no
*/
int main(void)
{
    /* Hardware pin initialization */
    hardware_init();

    /* set the system clock frequency */
    sysctl_pll_set_freq(SYSCTL_PLL0, 320000000UL);
    sysctl_pll_set_freq(SYSCTL_PLL1, 160000000UL);
    sysctl_pll_set_freq(SYSCTL_PLL2, 45158400UL);
    uarths_init();

    /* Initialize I2S, the third parameter is to set the channel mask, channel 0: 0x03, channel 1: 0x0C, channel 2: 0x30, channel 3: 0xC0*/
    i2s_init(I2S_DEVICE_2, I2S_TRANSMITTER, 0x03);

    /* Set the channel parameters of I2S sending data*/
    i2s_tx_channel_config(
        I2S_DEVICE_2, /* I2S device number*/
        I2S_CHANNEL_0, /* I2S channel */
        RESOLUTION_16_BIT, /* Number of data received */
        SCLK_CYCLES_32, /* Number of single data clocks */
        TRIGGER_LEVEL_4, /* FIFO depth when DMA is triggered*/
        RIGHT_JUSTIFYING_MODE); /* work mode/


    /* Play the music once. If you need to play it again, please press the reset button */
    i2s_play(
        I2S_DEVICE_2, /* I2S device number*/
        DMAC_CHANNEL0, /* DMA channel number */ 
        (uint8_t *)test_pcm, /* Playback of PCM data */
        sizeof(test_pcm), /* Length of PCM data */
        1024, /* ingle delivery quantity */
        16, /* Single sampling bit width */
        2); /* Track number */

    while (1);
    return 0;
}

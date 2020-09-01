/**
* @par  Copyright (C): 2016-2022, Shenzhen Yahboom Tech
* @file         main.c
* @author       Gengyue
* @version      V1.0
* @date         2020.05.27
* @brief        Microphone recording, speaker playing
* @details      
* @par History  Description below
*                 
* version:	V1.0: Microphone recording, speaker playing
*/
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "i2s.h"
#include "sysctl.h"
#include "fpioa.h"
#include "uarths.h"
#include "gpiohs.h"
#include "pin_config.h"

/* Microphone gain value, you can increase the volume of the recording according to the actual situation */
#define MIC_GAIN      1
#define FRAME_LEN     512
int16_t rx_buf[FRAME_LEN * 2 * 2];
uint32_t g_index = 0;
uint32_t g_tx_len = 0;

uint32_t g_rx_dma_buf[FRAME_LEN * 2 * 2];
uint8_t i2s_rec_flag = 0;

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
    /* mic */
    fpioa_set_function(PIN_MIC0_WS,   FUNC_MIC0_WS);
    fpioa_set_function(PIN_MIC0_DATA, FUNC_MIC0_DATA);
    fpioa_set_function(PIN_MIC0_SCK,  FUNC_MIC0_SCK);

    /* speak dac */
    fpioa_set_function(PIN_SPK_WS,   FUNC_SPK_WS);
    fpioa_set_function(PIN_SPK_DATA, FUNC_SPK_DATA);
    fpioa_set_function(PIN_SPK_BCK,  FUNC_SPK_BCK);
}

/**
* Function       i2s_receive_dma_cb
* @author        Gengyue
* @date          2020.05.27
* @brief         I2S0 receiving microphone data interrupt callback function
* @param[in]     void
* @param[out]    void
* @retval        0
* @par History   no
*/
int i2s_receive_dma_cb(void *ctx)
{
    uint32_t i;

    if(g_index)
    {
        /* receive DMA data */
        i2s_receive_data_dma(I2S_DEVICE_0, &g_rx_dma_buf[g_index], FRAME_LEN * 2, DMAC_CHANNEL1);
        g_index = 0;
        for(i = 0; i < FRAME_LEN; i++)
        {
            /* save data */
            rx_buf[2 * i] = (int16_t)((g_rx_dma_buf[2 * i + 1] * MIC_GAIN) & 0xffff);
            rx_buf[2 * i + 1] = (int16_t)((g_rx_dma_buf[2 * i + 1] * MIC_GAIN) & 0xffff);
        }
        i2s_rec_flag = 1;
    }
    else
    {
        i2s_receive_data_dma(I2S_DEVICE_0, &g_rx_dma_buf[0], FRAME_LEN * 2, DMAC_CHANNEL1);
        g_index = FRAME_LEN * 2;
        for(i = FRAME_LEN; i < FRAME_LEN * 2; i++)
        {
            rx_buf[2 * i] = (int16_t)((g_rx_dma_buf[2 * i + 1] * MIC_GAIN) & 0xffff);
            rx_buf[2 * i + 1] = (int16_t)((g_rx_dma_buf[2 * i + 1] * MIC_GAIN) & 0xffff);
        }
        i2s_rec_flag = 2;
    }
    return 0;
}

/**
* Function       init_mic
* @author        Gengyue
* @date          2020.05.27
* @brief         Initialize microphone configurati
* @param[in]     void
* @param[out]    void
* @retval        0
* @par History   no
*/
void init_mic(void)
{
    /* I2S device 0 is initialized to receive mode */
    i2s_init(I2S_DEVICE_0, I2S_RECEIVER, 0x0C);

    /* set channel */
    i2s_rx_channel_config(
        I2S_DEVICE_0, /* I2S device 0 */
        I2S_CHANNEL_1, /* channel1 */
        RESOLUTION_16_BIT, /* receive data 16bit */
        SCLK_CYCLES_32, /* single data clock is 32 */
        TRIGGER_LEVEL_4, /* FIFO depth is 4 */
        STANDARD_MODE); /* standard mode */
    
    /* set sample rate */
    i2s_set_sample_rate(I2S_DEVICE_0, 16000);

    /* Set the DMA interrupt callback*/
    dmac_set_irq(DMAC_CHANNEL1, i2s_receive_dma_cb, NULL, 4);

    /* I2S receives data through DMA and saves it to RX_BUF*/
    i2s_receive_data_dma(I2S_DEVICE_0, &rx_buf[g_index], FRAME_LEN * 2, DMAC_CHANNEL1);
}

/**
* Function       init_speaker
* @author        Gengyue
* @date          2020.05.27
* @brief         Initializes the DAC (speaker) configuration
* @param[in]     void
* @param[out]    void
* @retval        0
* @par History   no
*/
void init_speaker(void)
{
    /* Initialize I2S, and the third parameter is to set the channel mask, channel 0:0x03, channel 1:0x0c, channel 2:0x30, and channel 3:0xC0*/
    i2s_init(I2S_DEVICE_2, I2S_TRANSMITTER, 0x03);
    
    /* Set the channel parameters for I2S to send data*/
    i2s_tx_channel_config(
        I2S_DEVICE_2, /* I2S device number*/
        I2S_CHANNEL_0, /* I2S channle */
        RESOLUTION_16_BIT, /* Number of data received */
        SCLK_CYCLES_32, /* Number of individual data clocks*/
        TRIGGER_LEVEL_4, /* FIFO depth when DMA is triggered */
        RIGHT_JUSTIFYING_MODE); /* Working mode */
    /* Set sampling rate */
    i2s_set_sample_rate(I2S_DEVICE_2, 16000);
}

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

    /* Initialize interrupt, enable global interrupt, and initialize DMAC*/
    plic_init();
    sysctl_enable_irq();
    dmac_init();

    /* Initialize the speaker and microphone */
    init_speaker();
    init_mic();

    while (1)
    {
        if(i2s_rec_flag == 1)
        {
            i2s_play(
                I2S_DEVICE_2,  /* I2S device number */
                DMAC_CHANNEL0, /* DMA channel number*/ 
                (uint8_t *)(&rx_buf[0]), /* Play PCM data */
                FRAME_LEN * 4, /* length of PCM data */
                1024, /* Single delivery quantity */
                16,   /* Single sampling bit width */
                2);   /* Track number*/
            
            i2s_rec_flag = 0;
        }
        else if(i2s_rec_flag == 2)
        {
            i2s_play(
                I2S_DEVICE_2, /* I2S device number */
                DMAC_CHANNEL0, /* DMA channel number */ 
                (uint8_t *)(&rx_buf[FRAME_LEN * 2]), /* Play PCM data */
                FRAME_LEN * 4, /* length of PCM data */
                1024, /* Single delivery quantity */
                16, /* Single sampling bit width */
                2); /* Track number */
            
            i2s_rec_flag = 0;
        }
    }

    return 0;
}

/**
* @par  Copyright (C): 2016-2022, Shenzhen Yahboom Tech
* @file         main.c
* @author       Gengyue
* @version      V1.0
* @date         2020.05.27
* @brief        Memory access controller DMAC
* @details      
* @par History  description below
*                 
* version:	V1.0: The serial port transmits data through the DMA channel and controls the color of the RGB light.
* The serial port receives the following data corresponding functions:
*                 FFAA11 ---- red light on
*                 FFAA22 ---- red light off
*                 FFAA33 ---- green light on
*                 FFAA44 ---- green light off
*                 FFAA55 ---- blue light on
*                 FFAA66 ---- blue light off
*/
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "uart.h"
#include "gpiohs.h"
#include "sysctl.h"
#include "pin_config.h"


#define CMD_RGB_R_ON              0x11
#define CMD_RGB_R_OFF             0x22
#define CMD_RGB_G_ON              0x33
#define CMD_RGB_G_OFF             0x44
#define CMD_RGB_B_ON              0x55
#define CMD_RGB_B_OFF             0x66

/**
* Function       hardware_init
* @author        Gengyue
* @date          2020.05.27
* @brief         Hardware initialization, bind GPIO port
* @param[in]     void
* @param[out]    void
* @retval        void
* @par History   no
*/
void hardware_init(void)
{
    /* fpioa mapping */
    fpioa_set_function(PIN_RGB_R, FUNC_RGB_R);
    fpioa_set_function(PIN_RGB_G, FUNC_RGB_G);
    fpioa_set_function(PIN_RGB_B, FUNC_RGB_B);

    fpioa_set_function(PIN_UART_USB_RX, FUNC_UART_USB_RX);
    fpioa_set_function(PIN_UART_USB_TX, FUNC_UART_USB_TX);
}

/**
* Function       rgb_all_off
* @author        Gengyue
* @date          2020.05.27
* @brief         RGB off
* @param[in]     void
* @param[out]    void
* @retval        void
* @par History   no
*/
void rgb_all_off(void)
{
    gpiohs_set_pin(RGB_R_GPIONUM, GPIO_PV_HIGH);
    gpiohs_set_pin(RGB_G_GPIONUM, GPIO_PV_HIGH);
    gpiohs_set_pin(RGB_B_GPIONUM, GPIO_PV_HIGH);
}

/**
* Function       init_rgb
* @author        Gengyue
* @date          2020.05.27
* @brief         Initialize RGB
* @param[in]     void
* @param[out]    void
* @retval        void
* @par History   no
*/
void init_rgb(void)
{
    /* Set the GPIO mode of the RGB light to output */
    gpiohs_set_drive_mode(RGB_R_GPIONUM, GPIO_DM_OUTPUT);
    gpiohs_set_drive_mode(RGB_G_GPIONUM, GPIO_DM_OUTPUT);
    gpiohs_set_drive_mode(RGB_B_GPIONUM, GPIO_DM_OUTPUT);

    /* Close RGB */
    rgb_all_off();
}

/**
* Function       parse_cmd
* @author        Gengyue
* @date          2020.05.27
* @brief         Parse the received data
* @param[in]     cmd: received command 
* @param[out]    void
* @retval        0
* @par History   no
*/
int parse_cmd(uint8_t *cmd)
{
    switch(*cmd)
    {
    case CMD_RGB_R_ON:
        /* red light on*/
        gpiohs_set_pin(RGB_R_GPIONUM, GPIO_PV_LOW);
        break;
    case CMD_RGB_R_OFF:
        /* red light off*/
        gpiohs_set_pin(RGB_R_GPIONUM, GPIO_PV_HIGH);
        break;
    case CMD_RGB_G_ON:
        /* green light on*/
        gpiohs_set_pin(RGB_G_GPIONUM, GPIO_PV_LOW);
        break;
    case CMD_RGB_G_OFF:
        /* green light off*/
        gpiohs_set_pin(RGB_G_GPIONUM, GPIO_PV_HIGH);
        break;
    case CMD_RGB_B_ON:
        /* blue light on*/
        gpiohs_set_pin(RGB_B_GPIONUM, GPIO_PV_LOW);
        break;
    case CMD_RGB_B_OFF:
        /* blue light off*/
        gpiohs_set_pin(RGB_B_GPIONUM, GPIO_PV_HIGH);
        break;
    }
    return 0;
}

/**
* Function       main
* @author        Gengyue
* @date          2020.05.27
* @brief         Main function, the entry point of the program
* @param[in]     void
* @param[out]    void
* @retval        0
* @par History   no
*/
int main(void)
{
    /*Hardware pin initialization*/
    hardware_init();

    /* Initialize RGB lights */
    init_rgb();

    /* Initialize the serial port and set the baud rate to 115200 */
    uart_init(UART_USB_NUM);
    uart_configure(UART_USB_NUM, 115200, UART_BITWIDTH_8BIT, UART_STOP_1, UART_PARITY_NONE);

    /* Send "hello yahboom!"" when booting */
    char *hel = {"hello yahboom!\n"};
    uart_send_data_dma(UART_USB_NUM, DMAC_CHANNEL0, (uint8_t *)hel, strlen(hel));

    uint8_t recv = 0;
    int rec_flag = 0;

    while (1)
    {
        /* Receive serial port data through DMA channel 1 and save it in recv*/
        uart_receive_data_dma(UART_USB_NUM, DMAC_CHANNEL1, &recv, 1);
        /* Judgment agreement, it must be the data at the beginning of FFAA */
        switch(rec_flag)
        {
        case 0:
            if(recv == 0xFF)
                rec_flag = 1;
            break;
        case 1:
            if(recv == 0xAA)
                rec_flag = 2;
            else if(recv != 0xFF)
                rec_flag = 0;
            break;
        case 2:
            /* Parse the real data */
            parse_cmd(&recv);
            /* Send serial data through DMA channel 0 */
            uart_send_data_dma(UART_USB_NUM, DMAC_CHANNEL0, &recv, 1);
            rec_flag = 0;
            break;
        }
    }
    return 0;
}

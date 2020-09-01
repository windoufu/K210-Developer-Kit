/**
* @par  Copyright (C): 2016-2022, Shenzhen Yahboom Tech
* @file         main.c
* @author       Gengyue
* @version      V1.0
* @date         2020.05.27
* @brief        Serial port experiment
* @details      
* @par History  Description below
*                 
* version:	V1.0: 开机串口发送数据，WiFi模块接收数据传给K210处理.
*                 协议内容:$led0_1#：LED0点亮，$led0_0#：LED0熄灭，
*                         $led1_1#：LED1点亮，$led1_0#：LED1熄灭。
*/
#include <string.h>
#include "pin_config.h"
#include "led.h"

#define MAX_DATA        10

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
    /* USB Serial port  */
    fpioa_set_function(PIN_UART_USB_RX, FUNC_UART_USB_RX);
    fpioa_set_function(PIN_UART_USB_TX, FUNC_UART_USB_TX);

    /* WIFI module Serial port */
    fpioa_set_function(PIN_UART_WIFI_RX, FUNC_UART_WIFI_RX);
    fpioa_set_function(PIN_UART_WIFI_TX, FUNC_UART_WIFI_TX);

    /* LED light */
    led_init(LED_ALL);
}

void parse_data(char *data)
{
    // uart_send_data_dma(UART_USB_NUM, DMAC_CHANNEL0, data, sizeof(data));
    /* Analyze and compare the sent data */
    if (0 == memcmp(data, "led0_0", 6))
    {
        led0_state(LED_OFF);
    }
    else if (0 == memcmp(data, "led0_1", 6))
    {
        led0_state(LED_ON);
    }
    else if (0 == memcmp(data, "led1_0", 6))
    {
        led1_state(LED_OFF);
    }
    else if (0 == memcmp(data, "led1_1", 6))
    {
        led1_state(LED_ON);
    }
}

/**
* Function       main
* @author        Gengyue
* @date          2020.05.27
* @brief          Main function, the entry point of the program
* @param[in]     void
* @param[out]    void
* @retval        0
* @par History   no
*/
int main(void)
{
    hardware_init();
    // Initialize USB serial port, Set the baud rate to 115200
    uart_init(UART_USB_NUM);
    uart_configure(UART_USB_NUM, 115200, UART_BITWIDTH_8BIT, UART_STOP_1, UART_PARITY_NONE);

    /* Initialize the serial port of the WiFi module */
    uart_init(UART_WIFI_NUM);
    uart_configure(UART_WIFI_NUM, 115200, UART_BITWIDTH_8BIT, UART_STOP_1, UART_PARITY_NONE);

    /* send hello yahboom! when booting */
    char *hello = {"hello yahboom!\n"};
    uart_send_data(UART_USB_NUM, hello, strlen(hello));
    
    /* Receive and send buffered data */
    char recv = 0, send = 0;
    /*Receive WiFi module data flag */
    int rec_flag = 0;
    /* Save the received data */
    char recv_data[MAX_DATA] = {0}; 
    /* recv_data */
    uint16_t index = 0;

    while (1)
    {
        /* Receive information from WIFI module*/
        if(uart_receive_data(UART_WIFI_NUM, &recv, 1))
        {
            /* Send the received data to the USB serial port for display*/
            uart_send_data(UART_USB_NUM, &recv, 1);
            
            /* Determine whether it meets the data requirements */
            switch(rec_flag)
            {
            case 0:
                /* Start with ‘$’ sign as data */
                if(recv == '$')
                {
                    rec_flag = 1;
                    index = 0;
                    for (int i = 0; i < MAX_DATA; i++)
                        recv_data[i] = 0;
                }
                break;
            case 1:
                if (recv == '#')
                {
                    /* End with ‘#’ sign as data */
                    rec_flag = 0;
                    parse_data(recv_data);
                }
                else if (index >= MAX_DATA)
                {
                    /* If the data exceeds the maximum and the terminator ‘#’ is not received, it will be cleared*/
                    rec_flag = 0;
                    index = 0;
                }
                else
                {
                    /* Save the data to recv_data*/
                    recv_data[index++] = recv;
                }
                break;
            default:
                break;
            }
        }

        /* Receive the information from the serial port and send it to the WiFi module */
        if(uart_receive_data(UART_USB_NUM, &send, 1))
        {
            uart_send_data(UART_WIFI_NUM, &send, 1);
        }
    }
    return 0;
}

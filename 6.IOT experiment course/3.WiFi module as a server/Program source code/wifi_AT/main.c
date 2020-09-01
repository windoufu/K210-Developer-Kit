/**
* @par  Copyright (C): 2016-2022, Shenzhen Yahboom Tech
* @file         main.c
* @author       Gengyue
* @version      V1.0
* @date         2020.05.27
* @brief        Transfer WiFi module data
* @details      
* @par History  Description below
*                 
* version:	V1.0: Turn on the serial port to send data, receive data from the WiFi module, and display it on the computer through the USB serial port;
* Receive data from the USB serial port and send it to the WiFi module via the WiFi serial port.
*/
#include "pin_config.h"
#include "string.h"

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
    /* USB serial port  */
    fpioa_set_function(PIN_UART_USB_RX, FUNC_UART_USB_RX);
    fpioa_set_function(PIN_UART_USB_TX, FUNC_UART_USB_TX);

    /* WIFI module serial port  */
    fpioa_set_function(PIN_UART_WIFI_RX, FUNC_UART_WIFI_RX);
    fpioa_set_function(PIN_UART_WIFI_TX, FUNC_UART_WIFI_TX);
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
    /* Hardware pin initialization */
    hardware_init();

    // Initialize the USB serial port, set the baud rate to 115200
    uart_init(UART_USB_NUM);
    uart_configure(UART_USB_NUM, 115200, UART_BITWIDTH_8BIT, UART_STOP_1, UART_PARITY_NONE);

    /* Initialize the serial port of the WiFi module */
    uart_init(UART_WIFI_NUM);
    uart_configure(UART_WIFI_NUM, 115200, UART_BITWIDTH_8BIT, UART_STOP_1, UART_PARITY_NONE);

    /* Send hello yahboom! when booting*/
    char *hello = {"hello yahboom!\n"};
    uart_send_data(UART_USB_NUM, hello, strlen(hello));

    char recv = 0, send = 0;

    while (1)
    {
        /* Receive information from WIFI module */
        if(uart_receive_data(UART_WIFI_NUM, &recv, 1))
        {
            /* Send WiFi data to USB serial port display*/
            uart_send_data(UART_USB_NUM, &recv, 1);
        }

        /*Receive the information from the serial port and send it to the WiFi module */
        if(uart_receive_data(UART_USB_NUM, &send, 1))
        {
            uart_send_data(UART_WIFI_NUM, &send, 1);
        }
    }
    return 0;
}

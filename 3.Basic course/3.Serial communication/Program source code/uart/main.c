/**
* @par  Copyright (C): 2016-2022, Shenzhen Yahboom Tech
* @file         main.c
* @author       Gengyue
* @version      V1.0
* @date         2020.05.27
* @brief        Serial communication
* @details      
* @par History  Description below
*                 
* version:	V1.0: Turn on the serial port to send data, wait to receive data and send the received data.
*/
#include <string.h>
#include "pin_config.h"


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
    // fpioa mapping
    fpioa_set_function(PIN_UART_USB_RX, FUNC_UART_USB_RX);
    fpioa_set_function(PIN_UART_USB_TX, FUNC_UART_USB_TX);
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
    hardware_init();
    //Initialize serial port 3, set the baud rate to 115200
    uart_init(UART_USB_NUM);
    uart_configure(UART_USB_NUM, 115200, UART_BITWIDTH_8BIT, UART_STOP_1, UART_PARITY_NONE);

    /* Turn on send hello yahboom! */
    char *hello = {"hello yahboom!\n"};
    uart_send_data(UART_USB_NUM, hello, strlen(hello));

    char recv = 0;

    while (1)
    {
        /* wait to receive data and send the received data by Serial port*/
        while(uart_receive_data(UART_USB_NUM, &recv, 1))
        {
            uart_send_data(UART_USB_NUM, &recv, 1);
        }
    }
    return 0;
}

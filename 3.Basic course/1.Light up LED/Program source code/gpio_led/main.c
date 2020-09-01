/**
* @par  Copyright (C): 2016-2022, Shenzhen Yahboom Tech
* @file         main.c
* @author       Gengyue
* @version      V1.0
* @date         2020.05.27
* @brief        FPIOA mapping and GPIO drive LED lights
* @details      
* @par History  Description below
*                 
* version:	V1.0: LED0 and LED1 light up alternately, the time interval is 1 second.
*/
#include <stdio.h>
#include <unistd.h>
#include "gpio.h"
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
    fpioa_set_function(PIN_LED_0, FUNC_LED0);
    fpioa_set_function(PIN_LED_1, FUNC_LED1);
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
    hardware_init();// Hardware pin initialization

    gpio_init();    // Enable GPIO clock
    
    // Set the GPIO mode of LED0 and LED1 as output
    gpio_set_drive_mode(LED0_GPIONUM, GPIO_DM_OUTPUT);
    gpio_set_drive_mode(LED1_GPIONUM, GPIO_DM_OUTPUT);
    
    // close LED0 and LED1
    gpio_pin_value_t value = GPIO_PV_HIGH;
    gpio_set_pin(LED0_GPIONUM, value);
    gpio_set_pin(LED1_GPIONUM, value);

    while (1)
    {
        sleep(1);
        gpio_set_pin(LED0_GPIONUM, value);
        gpio_set_pin(LED1_GPIONUM, value = !value);
    }
    return 0;
}

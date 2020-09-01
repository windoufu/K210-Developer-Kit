/**
* @par  Copyright (C): 2016-2022, Shenzhen Yahboom Tech
* @file         main.c
* @author       Gengyue
* @version      V1.0
* @date         2020.05.27
* @brief        GPIOHS drive LED lights
* @details      
* @par History  Description below
*                 
* version:	V1.0: RGB change three color alternately, the time interval is 1 second.
*/
#include "sleep.h"
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
* @par History   no
*/
void hardware_init(void)
{
    // fpioa mapping
    fpioa_set_function(PIN_RGB_R, FUNC_RGB_R);
    fpioa_set_function(PIN_RGB_G, FUNC_RGB_G);
    fpioa_set_function(PIN_RGB_B, FUNC_RGB_B);

}

/**
* Function       hardware_init
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
* @brief         RGB initialization
* @param[in]     void
* @param[out]    void
* @retval        void
* @par History   NO
*/
void init_rgb(void)
{
    // Set the GPIO mode of RGB as output
    gpiohs_set_drive_mode(RGB_R_GPIONUM, GPIO_DM_OUTPUT);
    gpiohs_set_drive_mode(RGB_G_GPIONUM, GPIO_DM_OUTPUT);
    gpiohs_set_drive_mode(RGB_B_GPIONUM, GPIO_DM_OUTPUT);

    // Slose RGB
    rgb_all_off();
}

/**
* Function       main
* @author        Gengyue
* @date          2020.05.27
* @brief         Main function, the entry point of the program
* @param[in]     void
* @param[out]    void
* @retval        0
* @par History   NO
*/
int main(void)
{
    //RGB light status, 0=red light is on, 1=green light is on, 2=blue light is on
    int state = 0;

    //Hardware pin initialization
    hardware_init();
    //RGB initialization
    init_rgb();

    while (1)
    {
        rgb_all_off();   //Close RGB
        //Light up lights of different colors according to the value of state
        gpiohs_set_pin(state, GPIO_PV_LOW);
        msleep(500);
        state++;
        state = state % 3;
    }
    return 0;
}

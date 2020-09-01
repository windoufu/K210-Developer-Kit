/**
* @par  Copyright (C): 2016-2022, Shenzhen Yahboom Tech
* @file         main.c
* @author       Gengyue
* @version      V1.0
* @date         2020.05.27
* @brief        keypad control RGB
* @details      
* @par History  description below
*                 
* version:	V1.0: The dial switch keypad controls three colors of RGB light, the keypad scrolls to the left and the red light is on.
* The keypad is pressed and the green light is on, and the keypad is scrolled to the right and the blue light is on.
*
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
* @par History   NO
*/
void hardware_init(void)
{
    /* fpioa map */
    fpioa_set_function(PIN_RGB_R, FUNC_RGB_R);
    fpioa_set_function(PIN_RGB_G, FUNC_RGB_G);
    fpioa_set_function(PIN_RGB_B, FUNC_RGB_B);

    fpioa_set_function(PIN_KEYPAD_LEFT,   FUNC_KEYPAD_LEFT);
    fpioa_set_function(PIN_KEYPAD_MIDDLE, FUNC_KEYPAD_MIDDLE);
    fpioa_set_function(PIN_KEYPAD_RIGHT,  FUNC_KEYPAD_RIGHT);

}

/**
* Function       rgb_all_off
* @author        Gengyue
* @date          2020.05.27
* @brief         RGB is off
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
* @brief         Initialize RGB lights
* @param[in]     void
* @param[out]    void
* @retval        void
* @par History   no
*/
void init_rgb(void)
{
    /*Set the GPIO mode of the RGB light to output*/
    gpiohs_set_drive_mode(RGB_R_GPIONUM, GPIO_DM_OUTPUT);
    gpiohs_set_drive_mode(RGB_G_GPIONUM, GPIO_DM_OUTPUT);
    gpiohs_set_drive_mode(RGB_B_GPIONUM, GPIO_DM_OUTPUT);

    /* Close RGB light*/
    rgb_all_off();
}

/**
* Function       init_keypad
* @author        Gengyue
* @date          2020.05.27
* @brief         Initialize KEYPAD
* @param[in]     void
* @param[out]    void
* @retval        void
* @par History   no
*/
void init_keypad(void)
{
    /* Set the GPIO mode of the keypad to pull-up input */
    gpiohs_set_drive_mode(KEYPAD_LEFT_GPIONUM,   GPIO_DM_INPUT_PULL_UP);
    gpiohs_set_drive_mode(KEYPAD_MIDDLE_GPIONUM, GPIO_DM_INPUT_PULL_UP);
    gpiohs_set_drive_mode(KEYPAD_RIGHT_GPIONUM,  GPIO_DM_INPUT_PULL_UP);
}

/**
* Function       scan_keypad
* @author        Gengyue
* @date          2020.05.27
* @brief         Scan KEYPAD and handle press and release events
* @param[in]     void
* @param[out]    void
* @retval        void
* @par History   no
*/
void scan_keypad(void)
{
    /*Read the status of the three channels of the keypad*/
    gpio_pin_value_t state_keypad_left =   gpiohs_get_pin(KEYPAD_LEFT_GPIONUM);
    gpio_pin_value_t state_keypad_middle = gpiohs_get_pin(KEYPAD_MIDDLE_GPIONUM);
    gpio_pin_value_t state_keypad_right =  gpiohs_get_pin(KEYPAD_RIGHT_GPIONUM);

    /*Check if the keypad is scrolling to the left*/
    if (!state_keypad_left)
    {
        /*Delay debounce 10ms*/
        msleep(10);
        /*Read the status of the IO port to the left of the keypad again*/
        state_keypad_left = gpiohs_get_pin(KEYPAD_LEFT_GPIONUM);
        if (!state_keypad_left)
        {
            /*Scroll to the left to light up the red light */
            gpiohs_set_pin(RGB_R_GPIONUM, GPIO_PV_LOW);
        }
        else
        {
            /*Release, the red light is off*/
            gpiohs_set_pin(RGB_R_GPIONUM, GPIO_PV_HIGH);
        }
    } 
    /* Check whether the keypad is pressed */
    else if (!state_keypad_middle)
    {
        msleep(10);
        state_keypad_middle = gpiohs_get_pin(KEYPAD_MIDDLE_GPIONUM);
        if (!state_keypad_middle)
        {
            gpiohs_set_pin(RGB_G_GPIONUM, GPIO_PV_LOW);
        }
        else
        {
            gpiohs_set_pin(RGB_G_GPIONUM, GPIO_PV_HIGH);
        }
    } 
    /* Check if the keypad is scrolling to the right*/
    else if (!state_keypad_right)
    {
        msleep(10);
        state_keypad_right = gpiohs_get_pin(KEYPAD_RIGHT_GPIONUM);
        if (!state_keypad_right)
        {
            gpiohs_set_pin(RGB_B_GPIONUM, GPIO_PV_LOW);
        }
        else
        {
            gpiohs_set_pin(RGB_B_GPIONUM, GPIO_PV_HIGH);
        }
    }
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
    
    /* Initialize RGB lights*/
    init_rgb();

    /*  Initialize keypad */
    init_keypad();

    while (1)
    {
        /* Scan the keypad and control the RGB lights*/
        scan_keypad();
    }

    return 0;
}

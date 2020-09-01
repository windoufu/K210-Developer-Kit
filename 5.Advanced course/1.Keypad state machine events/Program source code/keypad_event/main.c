/**
* @par  Copyright (C): 2016-2022, Shenzhen Yahboom Tech
* @file         main.c
* @author       Gengyue
* @version      V1.0
* @date         2020.05.27
* @brief        Key state machine
* @details      
* @par History  Description below
*                 
* version:	V1.0: The dial switch keypad detects key events by means of a state machine, and then according to the event,
* Control the RGB light, short press to turn on the red light, long press the red light to go out, the blue light flashes, release the blue light to go out.
*/
#include <stdio.h>
#include "fpioa.h"
#include "gpiohs.h"
#include "sysctl.h"
#include "keypad.h"
#include "rgb.h"
#include "sleep.h"
#include "pin_config.h"


/**
* Function       hardware_init
* @author        Gengyue
* @date          2020.05.27
* @brief         Initialize the hardware 
* @param[in]     void
* @param[out]    void
* @retval        void
* @par History   no
*/
void hardware_init(void)
{
    fpioa_set_function(PIN_KEYPAD_LEFT,   FUNC_KEYPAD_LEFT);
    fpioa_set_function(PIN_KEYPAD_MIDDLE, FUNC_KEYPAD_MIDDLE);
    fpioa_set_function(PIN_KEYPAD_RIGHT,  FUNC_KEYPAD_RIGHT);
}

/**
* Function       key_press
* @author        Gengyue
* @date          2020.05.27
* @brief         Press the button to press event callback
* @param[in]     void
* @param[out]    void
* @retval        void
* @par History   no
*/
void key_press(void * arg)
{
    rgb_red_state(LIGHT_ON);
}

/**
* Function       key_release
* @author        Gengyue
* @date          2020.05.27
* @brief         Release the button to press event callback
* @param[in]     void
* @param[out]    void
* @retval        void
* @par History   no
*/
void key_release(void * arg)
{
    rgb_blue_state(LIGHT_OFF);
}

/**
* Function       key_long_press
* @author        Gengyue
* @date          2020.05.27
* @brief         Long press the event callback
* @param[in]     void
* @param[out]    void
* @retval        void
* @par History   no
*/
void key_long_press(void * arg)
{
    rgb_red_state(LIGHT_OFF);
}

/**
* Function       key_repeat
* @author        Gengyue
* @date          2020.05.27
* @brief         The event callback is triggered repeatedly
* @param[in]     void
* @param[out]    void
* @retval        void
* @par History   no
*/
void key_repeat(void * arg)
{
    static int state = 1;
    rgb_blue_state(state = !state);
}

/**
* Function       main
* @author        Gengyue
* @date          2020.05.27
* @brief         Main function, the entry of the program
* @param[in]     void
* @param[out]    void
* @retval        void
* @par History   no
*/
int main(void)
{
    /*Initialize the hardware  */
    hardware_init();

    /*Initialize system interrupts and enable global interrupts */
    plic_init();
    sysctl_enable_irq();

    /* Initialize the RGB */
    rgb_init(EN_RGB_ALL);

    /* Initialize the keypad */
    keypad_init();
    
    /* Set keypad call back */
    keypad[EN_KEY_ID_LEFT].short_key_down = key_press;
    keypad[EN_KEY_ID_LEFT].short_key_up = key_release;
    keypad[EN_KEY_ID_LEFT].long_key_down = key_long_press;
    keypad[EN_KEY_ID_LEFT].repeat_key_down = key_repeat;

    keypad[EN_KEY_ID_MIDDLE].short_key_down = key_press;
    keypad[EN_KEY_ID_MIDDLE].short_key_up = key_release;
    keypad[EN_KEY_ID_MIDDLE].long_key_down = key_long_press;
    keypad[EN_KEY_ID_MIDDLE].repeat_key_down = key_repeat;

    keypad[EN_KEY_ID_RIGHT].short_key_down = key_press;
    keypad[EN_KEY_ID_RIGHT].short_key_up = key_release;
    keypad[EN_KEY_ID_RIGHT].long_key_down = key_long_press;
    keypad[EN_KEY_ID_RIGHT].repeat_key_down = key_repeat;

    /* Keypad status value */
    keypad_status_t key_value = EN_KEY_NONE;
    printf("Please control keypad to get status!\n");

    while (1)
    {
        /* Read the status value of keypad, which defaults to 0 if there are no events*/
        key_value = get_keypad_state();
        if (key_value != 0)
        {
            switch (key_value)
            {
            case EN_KEY_LEFT_DOWN:
                printf("KEY_LEFT_DOWN:%d \n", (uint16_t)key_value);
                break;
            case EN_KEY_LEFT_UP:
                printf("KEY_LEFT_UP:%d \n", (uint16_t)key_value);
                break;
            case EN_KEY_LEFT_LONG:
                printf("KEY_LEFT_LONG:%d \n", (uint16_t)key_value);
                break;
            case EN_KEY_LEFT_REPEAT:
                printf("KEY_LEFT_REPEAT:%d \n", (uint16_t)key_value);
                break;
            
            case EN_KEY_MIDDLE_DOWN:
                printf("KEY_MIDDLE_DOWN:%d \n", (uint16_t)key_value);
                break;
            case EN_KEY_MIDDLE_UP:
                printf("KEY_MIDDLE_UP:%d \n", (uint16_t)key_value);
                break;
            case EN_KEY_MIDDLE_LONG:
                printf("KEY_MIDDLE_LONG:%d \n", (uint16_t)key_value);
                break;
            case EN_KEY_MIDDLE_REPEAT:
                printf("KEY_MIDDLE_REPEAT:%d \n", (uint16_t)key_value);
                break;
            
            case EN_KEY_RIGHT_DOWN:
                printf("KEY_RIGHT_DOWN:%d \n", (uint16_t)key_value);
                break;
            case EN_KEY_RIGHT_UP:
                printf("KEY_RIGHT_UP:%d \n", (uint16_t)key_value);
                break;
            case EN_KEY_RIGHT_LONG:
                printf("KEY_RIGHT_LONG:%d \n", (uint16_t)key_value);
                break;
            case EN_KEY_RIGHT_REPEAT:
                printf("KEY_RIGHT_REPEAT:%d \n", (uint16_t)key_value);
                break;
            
            default:
                break;
            } 
        }
        msleep(1);
    }
}

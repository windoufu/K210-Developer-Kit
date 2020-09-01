/**
* @par  Copyright (C): 2016-2022, Shenzhen Yahboom Tech
* @file         main.c
* @author       Gengyue
* @version      V1.0
* @date         2020.05.27
* @brief        button control RGB
* @details      
* @par History  Description below
*                 
* version:	V1.0: Each time you press the button, the RGB light turns red, and when you release it, it turns off.
*/
#include "pin_config.h"
#include "sleep.h"
#include "gpiohs.h"
#include "sysctl.h"
#include "plic.h"

uint32_t g_count;

/**
* Function       hardware_init
* @author        Gengyue
* @date          2020.05.27
* @brief         Hardware initialization, GPIO port
* @param[in]     void
* @param[out]    void
* @retval        void
* @par History   NO
*/
void hardware_init(void)
{
    // fpioa map
    fpioa_set_function(PIN_RGB_R, FUNC_RGB_R);
    fpioa_set_function(PIN_RGB_G, FUNC_RGB_G);
    fpioa_set_function(PIN_RGB_B, FUNC_RGB_B);

    fpioa_set_function(PIN_KEY, FUNC_KEY);
}

/**
* Function       hardware_init
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
    /* Set the GPIO mode of the RGB light to output*/
    gpiohs_set_drive_mode(RGB_R_GPIONUM, GPIO_DM_OUTPUT);
    gpiohs_set_drive_mode(RGB_G_GPIONUM, GPIO_DM_OUTPUT);
    gpiohs_set_drive_mode(RGB_B_GPIONUM, GPIO_DM_OUTPUT);

    /* Close RGB light */
    rgb_all_off();
}

/**
* Function       key_irq_cb
* @author        Gengyue
* @date          2020.05.27
* @brief         Key interrupt callback function
* @param[in]     ctx Callback parameter
* @param[out]    void
* @retval        0
* @par History   no
*/
int key_irq_cb(void* ctx)
{
    gpio_pin_value_t key_state = gpiohs_get_pin(KEY_GPIONUM);
    /*only for testing to interrupt the callback to print data*/
    printf("IRQ The PIN is %d\n", key_state);

    uint32_t *tmp = (uint32_t *)(ctx);
    printf("count is %d\n", (*tmp)++);

    if (!key_state)
        gpiohs_set_pin(RGB_R_GPIONUM, GPIO_PV_LOW);
    else
        gpiohs_set_pin(RGB_R_GPIONUM, GPIO_PV_HIGH);
    return 0;
}

/**
* Function       init_key
* @author        Gengyue
* @date          2020.05.27
* @brief         Initialize the key
* @param[in]     void
* @param[out]    void
* @retval        void
* @par History   no
*/
void init_key(void)
{
    /*Set the GPIO mode of the button to pull-up input*/
    gpiohs_set_drive_mode(KEY_GPIONUM, GPIO_DM_INPUT_PULL_UP);
    /*Set the button's GPIO level trigger mode to rising edge and falling edge*/
    gpiohs_set_pin_edge(KEY_GPIONUM, GPIO_PE_BOTH);
    /*Set the interrupt callback of the button GPIO port*/
    gpiohs_irq_register(KEY_GPIONUM, 1, key_irq_cb, &g_count);
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
    //Hardware pin initialization
    hardware_init();

    /*External interrupt initialization*/
    plic_init();
    /*Enable global interrupt*/
    sysctl_enable_irq();

    //Initialize RGB lights
    init_rgb();

    //Initialize the key
    init_key();
    
    while (1);
    return 0;
}

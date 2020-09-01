/**
* @par  Copyright (C): 2016-2022, Shenzhen Yahboom Tech
* @file         main.c
* @author       Gengyue
* @version      V1.0
* @date         2020.05.27
* @brief        PWM output breathing light
* @details      
* @par History  Description below
*                 
* version:	V1.0: PWM output breathing light
*/
#include <stdio.h>
#include <unistd.h>
#include "gpio.h"
#include "timer.h"
#include "pwm.h"
#include "sysctl.h"
#include "plic.h"
#include "pin_config.h"


/**
* Function       hardware_init
* @author        Gengyue
* @date          2020.05.27
* @brief         Hardware initialization, GPIO port
* @param[in]     void
* @param[out]    void
* @retval        void
* @par History   no
*/
void hardware_init(void)
{
    fpioa_set_function(PIN_RGB_R, FUNC_TIMER1_TOGGLE1);
}

/**
* Function       timer_timeout_cb
* @author        Gengyue
* @date          2020.05.27
* @brief         Timer interrupt callback
* @param[in]     ctx
* @param[out]    void
* @retval        0
* @par History   no
*/
int timer_timeout_cb(void *ctx) {
    static double duty_cycle = 0.01;
    /* 0为Gradually increase，1为Gradually decrease */
    static int flag = 0;      

    /*Pass in different values of cycle to adjust the occupancy ratio of PWM, that is, adjust the brightness of the lamp */
    pwm_set_frequency(PWM_DEVICE_1, PWM_CHANNEL_0, 200000, duty_cycle);

    /* Modify the value of cycle to increase and decrease gradually in the interval (0,1) */
    flag ? (duty_cycle -= 0.01): (duty_cycle += 0.01);
    if(duty_cycle > 1.0)
    {
        duty_cycle = 1.0;
        flag = 1;
    }
    else if (duty_cycle < 0.0)
    {
        duty_cycle = 0.0;
        flag = 0;
    }
    return 0;
}

/**
* Function       init_timer
* @author        Gengyue
* @date          2020.05.27
* @brief         Init timer
* @param[in]     ctx
* @param[out]    void
* @retval        0
* @par History   no
*/
void init_timer(void) {
    /*Init timer */
    timer_init(TIMER_DEVICE_0);
    /* Set the timer timeout time, unit:ns */
    timer_set_interval(TIMER_DEVICE_0, TIMER_CHANNEL_0, 10 * 1e6);
    /* Set timer interrupt callback */
    timer_irq_register(TIMER_DEVICE_0, TIMER_CHANNEL_0, 0, 1, timer_timeout_cb, NULL);
    /*enable timer */
    timer_set_enable(TIMER_DEVICE_0, TIMER_CHANNEL_0, 1);
}

/**
* Function       init_pwm
* @author        Gengyue
* @date          2020.05.27
* @brief         Init PWM
* @param[in]     void
* @param[out]    void
* @retval        void
* @par History   no
*/
void init_pwm(void)
{
    /* Init PWM */
    pwm_init(PWM_DEVICE_1);
    /* Set the PWM frequency to 200KHZ and a square wave with a duty cycle of 0.5*/
    pwm_set_frequency(PWM_DEVICE_1, PWM_CHANNEL_0, 200000, 0.5);
    /* enable PWM output */
    pwm_set_enable(PWM_DEVICE_1, PWM_CHANNEL_0, 1);
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

    /* System interrupt initialization and enable */
    plic_init();
    sysctl_enable_irq();
    
    /*Initialize the timer*/
    init_timer();

    /*Initialize PWM */
    init_pwm();

    while(1);

    return 0;
}

/**
* @par  Copyright (C): 2016-2022, Shenzhen Yahboom Tech
* @file         main.c
* @author       Gengyue
* @version      V1.0
* @date         2020.05.27
* @brief        watchdog
* @details      
* @par History  following description
*                 
* version:	V1.0: If the watchdog does not feed within the set time, the system will restart after timeout.
* If you modify WDT_TIMEOUT_REBOOT to 0, after the watchdog times out, only the serial port will send a message indicating that it has timed out, and will not restart.
*/
#include <stdio.h>
#include <unistd.h>
#include "wdt.h"
#include "sysctl.h"

#define WDT_TIMEOUT_REBOOT    1

/**
* Function       wdt0_irq_cb
* @author        Gengyue
* @date          2020.05.27
* @brief         Watchdog interrupt callback
* @param[in]     void
* @param[out]    void
* @retval        0
* @par History   no
*/
int wdt0_irq_cb(void *ctx)
{
    #if WDT_TIMEOUT_REBOOT
    printf("%s:The system will reboot soon!\n", __func__);
    while(1);
    #else
    printf("%s:The system is busy but not reboot!\n", __func__);
    wdt_clear_interrupt(WDT_DEVICE_0);
    #endif
    return 0;
}

/**
* Function       main
* @author        Gengyue
* @date          2020.05.27
* @brief         Main function, the entry of the program
* @param[in]     void
* @param[out]    void
* @retval        0
* @par History   no
*/
int main(void)
{
    /* Print system startup information*/
    printf("system start!\n");
    /* Record the number of feeds */
    int times = 0;

    /* System interrupt initialization */
    plic_init();
    sysctl_enable_irq();

    /* Start the watchdog and call the interrupt function wdt0_irq_cb after setting the timeout time to 2 seconds */
    int timeout = wdt_init(WDT_DEVICE_0, 2000, wdt0_irq_cb, NULL);

    /* Print the actual timeout of the watchdog*/
    printf("wdt timeout is %d ms!\n", timeout);
    
    while(1)
    {
        sleep(1);
        if(times++ < 5)
        {
            /* Number of feeds printed */
            printf("wdt_feed %d times!\n", times);

            /* Reset the watchdog's timer and restart timing*/
            wdt_feed(WDT_DEVICE_0);
        }
    }
}


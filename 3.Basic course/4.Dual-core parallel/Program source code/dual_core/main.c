/**
* @par  Copyright (C): 2016-2022, Shenzhen Yahboom Tech
* @file         main.c
* @author       Gengyue
* @version      V1.0
* @date         2020.05.27
* @brief        dual_core
* @details      
* @par History  Description below
*                 
* version:	V1.0: Turn on core 1, and core 0 and core 1 enter the while (1) loop at the same time, and print different data
*/
#include <stdio.h>
#include "bsp.h"
#include "sleep.h"
#include "sysctl.h"

/**
* Function       core1_main
* @author        Gengyue
* @date          2020.05.27
* @brief         core 1 main function
* @param[in]     void
* @param[out]    void
* @retval        0
* @par History   no
*/
int core1_main(void *ctx)
{
    int state = 1;
    uint64_t core = current_coreid();
    printf("Core %ld say: Hello world\n", core);

    while(1)
    {
        msleep(500);
        if (state = !state)
        {
            printf("Core %ld is running too!\n", core);
        }
        else
        {
            printf("Core %ld is running faster!\n", core);
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
    /* Read the currently running core number */
    uint64_t core = current_coreid();
    printf("Core %ld say: Hello yahboom\n", core);
    /* Register core 1, and start up core 1*/
    register_core1(core1_main, NULL);

    while(1)
    {
        sleep(1);
        printf("Core %ld is running\n", core);
    }
    return 0;
}

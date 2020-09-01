/**
* @par  Copyright (C): 2016-2022, Shenzhen Yahboom Tech
* @file         main.c
* @author       Gengyue
* @version      V1.0
* @date         2020.05.27
* @brief        flash 
* @details      
* @par History  Description below
*                 
* version:	V1.0: Write data to flash first, then read it out, and compare whether the written and read data are consistent.
* If they are inconsistent, an error message will be printed.
*/
*/
#include <stdio.h>
#include "fpioa.h"
#include "sysctl.h"
#include "w25qxx.h"
#include "uarths.h"
#include "spi.h"

#define BUF_LENGTH (40 * 1024 + 5)
#define DATA_ADDRESS 0xB00000

uint8_t write_buf[BUF_LENGTH];
uint8_t read_buf[BUF_LENGTH];

/**
* Function       flash_init
* @author        Gengyue
* @date          2020.05.27
* @brief         flash initialization 
* @param[in]     void
* @param[out]    void
* @retval        void
* @par History   no
*/
int flash_init(void)
{
    uint8_t manuf_id, device_id;
    uint8_t spi_index = 3, spi_ss = 0;
    printf("flash init \n");

    w25qxx_init(spi_index, spi_ss, 60000000);
    /* Read flash ID */
    w25qxx_read_id(&manuf_id, &device_id);
    printf("manuf_id:0x%02x, device_id:0x%02x\n", manuf_id, device_id);
    if ((manuf_id != 0xEF && manuf_id != 0xC8) || (device_id != 0x17 && device_id != 0x16))
    {
        /* flash failed to initialize */
        printf("w25qxx_read_id error\n");
        printf("manuf_id:0x%02x, device_id:0x%02x\n", manuf_id, device_id);
        return 0;
    }
    else
    {
        return 1;
    } 
}

/**
* Function       flash_write_data
* @author        Gengyue
* @date          2020.05.27
* @brief         flash write data
* @param[in]     data_buf
* @param[in]     length
* @param[out]    void
* @retval        void
* @par History   no
*/
void flash_write_data(uint8_t *data_buf, uint32_t length)
{
    uint64_t start = sysctl_get_time_us();
    /* flash write data  */
    w25qxx_write_data(DATA_ADDRESS, data_buf, length);
    uint64_t stop = sysctl_get_time_us();
    /* Print and write data time (us) */
    printf("write data finish:%ld us\n", (stop - start));
}

/**
* Function       flash_read_data
* @author        Gengyue
* @date          2020.05.27
* @brief         flash read data
* @param[in]     data_buf
* @param[in]     length
* @param[out]    void
* @retval        void
* @par History   no
*/
void flash_read_data(uint8_t *data_buf, uint32_t length)
{
    uint64_t start = sysctl_get_time_us();
    /* flash read data */
    w25qxx_read_data(DATA_ADDRESS, data_buf, length);
    uint64_t stop = sysctl_get_time_us();
    /* Print read data time(us) */
    printf("read data finish:%ld us\n", (stop - start));
}


/**
* Function       main
* @author        Gengyue
* @date          2020.05.27
* @brief         Main function, the entry of the program
* @param[in]     void
* @param[out]    void
* @retval        0
* @par History   NO
*/
int main(void)
{
    /* Set new PLL0 frequency*/
    sysctl_pll_set_freq(SYSCTL_PLL0, 800000000);
    uarths_init();
    
    /* Initialize flash */
    uint8_t res = 0;
    res = flash_init();
    if (res == 0) return 0;

    /* Assign a value to the data written to the cache */
    for (int i = 0; i < BUF_LENGTH; i++)
        write_buf[i] = (uint8_t)(i);
    
    /* Clear read cached data */
    for(int i = 0; i < BUF_LENGTH; i++)
        read_buf[i] = 0;

    printf("flash start write data\n");

    /* flash write data */
    flash_write_data(write_buf, BUF_LENGTH);

    /*flash read data*/
    flash_read_data(read_buf, BUF_LENGTH);

    /* Compare data and print error messages if there are differences*/
    for (int i = 0; i < BUF_LENGTH; i++)
    {
        if (read_buf[i] != write_buf[i])
        {
            printf("flash read error\n");
            return 0;
        }      
    }
    printf("spi3 flash master test ok\n");
    while (1)
        ;
    return 0;
}

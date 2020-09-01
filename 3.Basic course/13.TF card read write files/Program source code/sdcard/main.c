/**
* @par  Copyright (C): 2016-2022, Shenzhen Yahboom Tech
* @file         main.c
* @author       Gengyue
* @version      V1.0
* @date         2020.05.27
* @brief        TF card read write files
* @details      
* @par History  Description below
*                 
* version:	V1.0:Write data into TF, and then read it out.
* After burning the firmware, you need to close power swicth, then open the power switch of K210 development board, otherwise the memory card initialization will fail.
*/
#include <stdio.h>
#include "sysctl.h"
#include "dmac.h"
#include "fpioa.h"
#include "sdcard.h"
#include "ff.h"
#include "i2s.h"
#include "plic.h"
#include "uarths.h"
#include "bsp.h"
#include "pin_config.h"


static int check_sdcard(void);
static int check_fat32(void);
FRESULT sd_write_file(TCHAR *path);
FRESULT sd_read_file(TCHAR *path);


/**
* Function       hardware_init
* @author        Gengyue
* @date          2020.05.27
* @brief         Hardware initialization, binding GPIO port
* @param[in]     void
* @param[out]    void
* @retval        void
* @par History   NO
*/
void hardware_init(void)
{
    /*
    ** io26--miso--d1
    ** io27--clk---sclk
    ** io28--mosi--d0
    ** io29--cs----cs
    */
    fpioa_set_function(PIN_TF_MISO, FUNC_TF_SPI_MISO);
    fpioa_set_function(PIN_TF_CLK,  FUNC_TF_SPI_CLK);
    fpioa_set_function(PIN_TF_MOSI, FUNC_TF_SPI_MOSI);
    fpioa_set_function(PIN_TF_CS,   FUNC_TF_SPI_CS);
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
    // Hardware pin initialization 
    hardware_init();

    /*set the system clock frequency */
    sysctl_pll_set_freq(SYSCTL_PLL0, 800000000UL);
    sysctl_pll_set_freq(SYSCTL_PLL1, 300000000UL);
    sysctl_pll_set_freq(SYSCTL_PLL2, 45158400UL);
    uarths_init();

    if (check_sdcard())
    {
        printf("SD card err\n");
        return -1;
    }

    if (check_fat32())
    {
        printf("FAT32 err\n");
        return -1;
    }

    sleep(1);
    if (sd_write_file(_T("0:test.txt")))
    {
        printf("SD write err\n");
        return -1;
    }

    if (sd_read_file(_T("0:test.txt")))
    {
        printf("SD read err\n");
        return -1;
    }
    
    while (1);
    return 0;
}

/**
* Function       check_sdcard
* @author        Gengyue
* @date          2020.05.27
* @brief         Check if TF is normal
* @param[in]     void
* @param[out]    void
* @retval        0
* @par History   no
*/
static int check_sdcard(void)
{
    uint8_t status;

    printf("/******************check_sdcard*****************/\n");
    status = sd_init();
    printf("sd init :%d\n", status);
    if (status != 0)
    {
        return status;
    }

    printf("CardCapacity:%.1fG \n", (double)cardinfo.CardCapacity / 1024 / 1024 / 1024);
    printf("CardBlockSize:%d\n", cardinfo.CardBlockSize);
    return 0;
}

/**
* Function       check_fat32
* @author        Gengyue
* @date          2020.05.27
* @brief         Test whether the format of TF is FAT32
* @param[in]     void
* @param[out]    void
* @retval        0
* @par History   no
*/
static int check_fat32(void)
{
    static FATFS sdcard_fs;
    FRESULT status;
    DIR dj;
    FILINFO fno;

    printf("/********************check_fat32*******************/\n");
    status = f_mount(&sdcard_fs, _T("0:"), 1);
    printf("mount sdcard:%d\n", status);
    if (status != FR_OK)
        return status;

    printf("printf filename\n");
    status = f_findfirst(&dj, &fno, _T("0:"), _T("*"));
    while (status == FR_OK && fno.fname[0])
    {
        if (fno.fattrib & AM_DIR)
            printf("dir:%s\n", fno.fname);
        else
            printf("file:%s\n", fno.fname);
        status = f_findnext(&dj, &fno);
    }
    f_closedir(&dj);
    return 0;
}

/**
* Function       sd_write_file
* @author        Gengyue
* @date          2020.05.27
* @brief         Write the file to the TF card
* @param[in]     void
* @param[out]    void
* @retval        0
* @par History   no
*/
FRESULT sd_write_file(TCHAR *path)
{
    FIL file;
    FRESULT ret = FR_OK;
    printf("/*******************sd_write_file*******************/\n");
    uint32_t v_ret_len = 0;

    /* Open the file, if the file does not exist, create a new one */
    if ((ret = f_open(&file, path, FA_CREATE_ALWAYS | FA_WRITE)) != FR_OK)
    {
        printf("open file %s err[%d]\n", path, ret);
        return ret;
    }
    else
    {
        printf("Open %s ok\n", path);
    }

    /* Data to be written*/
    uint8_t data[] = {'H','i',',','D','a','t','a',' ','W','r','i','t','e',' ','O','k','!'};

    /* Write Data*/
    ret = f_write(&file, data, sizeof(data), &v_ret_len);
    if (ret != FR_OK)
    {
        printf("Write %s err[%d]\n", path, ret);
    }
    else
    {
        printf("Write %d bytes to %s ok\n", v_ret_len, path);
    }
    /* Close file*/
    f_close(&file);
    return ret;
}

/**
* Function       sd_read_file
* @author        Gengyue
* @date          2020.05.27
* @brief         Read files from TF card
* @param[in]     void
* @param[out]    void
* @retval        0
* @par History   no
*/
FRESULT sd_read_file(TCHAR *path)
{
    FIL file;
    FRESULT ret = FR_OK;
    printf("/*******************sd_read_file*******************/\n");
    uint32_t v_ret_len = 0;

    /*Check file status */
    FILINFO v_fileinfo;
    if ((ret = f_stat(path, &v_fileinfo)) == FR_OK)
    {
        printf("%s length is %lld\n", path, v_fileinfo.fsize);
    }
    else
    {
        printf("%s fstat err [%d]\n", path, ret);
        return ret;
    }

    /* Open the file as read-only*/
    if ((ret = f_open(&file, path, FA_READ)) == FR_OK)
    {
        char v_buf[64] = {0};
        ret = f_read(&file, (void *)v_buf, 64, &v_ret_len);
        if (ret != FR_OK)
        {
            printf("Read %s err[%d]\n", path, ret);
        }
        else
        {
            printf("Read :> %s \n", v_buf);
            printf("total %d bytes lenth\n", v_ret_len);
        }
        /* Close file */
        f_close(&file);
    }
    return ret;
}

/**
* @par  Copyright (C): 2016-2022, Shenzhen Yahboom Tech
* @file         main.c
* @author       Gengyue
* @version      V1.0
* @date         2020.05.27
* @brief        lcd display picture
* @details      
* @par History  Description below
*                 
* version:	V1.0: LCD display pictures and string.
*/
#include "sleep.h"
#include "gpiohs.h"
#include "lcd.h"
#include "st7789.h"
#include "sysctl.h"
#include "pin_config.h"


/**
* Function       io_set_power
* @author        Gengyue
* @date          2020.05.27
* @brief         Set the power domain of bank6 to 1.8V
* @param[in]     void
* @param[out]    void
* @retval        void
* @par History   NO
*/
void io_set_power(void)
{
    sysctl_set_power_mode(SYSCTL_POWER_BANK6, SYSCTL_POWER_V18);
}

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
    /**
    *PIN_LCD_CS	    36
    *PIN_LCD_RST	37
    *PIN_LCD_RS	    38
    *PIN_LCD_WR 	39
    **/
    fpioa_set_function(PIN_LCD_CS,  FUNC_LCD_CS);
    fpioa_set_function(PIN_LCD_RST, FUNC_LCD_RST);
    fpioa_set_function(PIN_LCD_RS,  FUNC_LCD_RS);
    fpioa_set_function(PIN_LCD_WR,  FUNC_LCD_WR);
    
    /* Enable SPI0 and DVP data*/
    sysctl_set_spi0_dvp_data(1);

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
    /*Hardware pin initialization */
    hardware_init();
    
    /*Set IO port voltage */
    io_set_power();
    
    /*Initialize LCD */
    lcd_init();

    /*Display picture*/
    lcd_draw_picture_half(0, 0, 320, 240, gImage_logo);
    sleep(1);

    /*Display string*/
    lcd_draw_string(16, 40, "Hello Yahboom!", RED);
    lcd_draw_string(16, 60, "Nice to meet you!", BLUE);
    
    while (1);
    return 0;
}

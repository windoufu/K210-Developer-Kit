/**
* @par  Copyright (C): 2016-2022, Shenzhen Yahboom Tech
* @file         main.c
* @author       Gengyue
* @version      V1.0
* @date         2020.05.27
* @brief        Horizontal test board
* @details      
* @par History  Description below
*                 
* version:	V1.0: Display a robot icon, calculate the angle of the development board based on the icm20607 sensor,
* Swing the development board to make the robot icon move. When it is flat on the desktop, the robot icon is in the middle.
*/
#include "sleep.h"
#include "gpiohs.h"
#include "sysctl.h"
#include "icm20607.h"
#include "pin_config.h"
#include "angle.h"
#include "plic.h"
#include "lcd.h"
#include "lvgl_display.h"


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
    /* I2C ICM20607 */
    fpioa_set_function(PIN_ICM_SCL, FUNC_ICM_SCL);
    fpioa_set_function(PIN_ICM_SDA, FUNC_ICM_SDA);

    /* SPI lcd */
    fpioa_set_function(PIN_LCD_CS,  FUNC_LCD_CS);
    fpioa_set_function(PIN_LCD_RST, FUNC_LCD_RST);
    fpioa_set_function(PIN_LCD_RS,  FUNC_LCD_RS);
    fpioa_set_function(PIN_LCD_WR,  FUNC_LCD_WR);
    sysctl_set_spi0_dvp_data(1);
}

/**
* Function       io_set_power
* @author        Gengyue
* @date          2020.05.27
* @brief         Set the IO port level voltage of the LCD to 1.8V. 
* @param[in]     void
* @param[out]    void
* @retval        void
* @par History   no
*/
static void io_set_power(void)
{
    /*Set the IO port level voltage of the LCD to 1.8V.  */
    sysctl_set_power_mode(SYSCTL_POWER_BANK6, SYSCTL_POWER_V18);
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
    /* Hardware pin initialization*/
    hardware_init();
    io_set_power();
    
    /* System interrupt enable */
    plic_init();
    sysctl_enable_irq();

    /* The LCD initializes and displays a picture for one second */
    lcd_init();
    lcd_draw_picture_half(0, 0, 320, 240, gImage_logo);
    sleep(1);

    /* Initialize ICM20607*/
    icm20607_init();
    msleep(100);

    /* Initialize lvgl */
    lvgl_disp_init();

    /*Create robot icon */
    lvgl_creat_image();

    int16_t lvgl_x, lvgl_y;
    while (1)
    {
        /* Reading angle */
        get_icm_attitude();

        /* Conversion data*/
        lvgl_x = lvgl_get_x(g_attitude.roll);
        lvgl_y = lvgl_get_y(g_attitude.pitch);

        /* Modify the position of the robot icon*/
        lvgl_move_image(lvgl_x, lvgl_y);
        msleep(1);
    }
    
    return 0;
}

/**
* @par  Copyright (C): 2016-2022, Shenzhen Yahboom Tech
* @file         main.c
* @author       Gengyue
* @version      V1.0
* @date         2020.05.27
* @brief        lvgl graphical experiment
* @details      
* @par History  Description below
*                 
* version:	V1.0: There are a total of three operation interfaces, click the characters in the top bar to switch, or slide left and right to switch.
* The first interface "Write": displays characters and virtual keyboard, click the position of the input box,
* A virtual keyboard will pop up, and you can touch it to input the corresponding characters.
* The second interface "List": Swipe up and down to switch to display the corresponding icons and characters,
* With one touch, the characters on the list will be entered on the first interface.
* The third interface "Chart": displays a spectrum image with a sliding bar at the bottom,
* Drag the slider to change the height of the bar graph on the spectrum in real time.
*/
#include <stdint.h>
#include <unistd.h>
#include "sleep.h"
#include "sysctl.h"
#include "fpioa.h"
#include "pin_config.h"
#include "lcd.h"
#include "st7789.h"
#include "lvgl/lvgl.h"
#include "lv_examples/lv_apps/demo/demo.h"
#include "ft6236u.h"
#include "gui_lvgl.h"

/**
* Function       io_set_power
* @author        Gengyue
* @date          2020.05.27
* @brief         Set the display screen IO port voltage to 1.8V
* @param[in]     void
* @param[out]    void
* @retval        0
* @par History   no
*/
static void io_set_power(void)
{
    /* Set the display voltage to 1.8V */
    sysctl_set_power_mode(SYSCTL_POWER_BANK6, SYSCTL_POWER_V18);
}

/**
* Function       hardware_init
* @author        Gengyue
* @date          2020.05.27
* @brief         hardware_init
* @param[in]     void
* @param[out]    void
* @retval        0
* @par History   no
*/
static void hardware_init(void)
{
    /* SPI lcd */
    fpioa_set_function(PIN_LCD_CS,  FUNC_LCD_CS);
    fpioa_set_function(PIN_LCD_RST, FUNC_LCD_RST);
    fpioa_set_function(PIN_LCD_RS,  FUNC_LCD_RS);
    fpioa_set_function(PIN_LCD_WR,  FUNC_LCD_WR);
    sysctl_set_spi0_dvp_data(1);

    /* I2C FT6236 */
    fpioa_set_function(PIN_FT_SCL, FUNC_FT_SCL);
    fpioa_set_function(PIN_FT_SDA, FUNC_FT_SDA);
    fpioa_set_function(PIN_FT_INT, FUNC_FT_INT);
    // fpioa_set_function(PIN_FT_RST, FUNC_FT_RST);
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
    printf("system start init\n");
    hardware_init();
    io_set_power();

    /* Initialize the touch screen and display the picture */
    lcd_init();
    ft6236_init();
    lcd_draw_picture_half(0, 0, 320, 240, gImage_logo);
    sleep(1);

    /* lvgl initialization */
    lvgl_disp_input_init();

    /* Run routine */
    demo_create();

    printf("system start ok\n");
    printf("Please touch the screen\n");
    while (1)
        ;
    return 0;
}

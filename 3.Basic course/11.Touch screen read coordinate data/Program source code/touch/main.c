/**
* @par  Copyright (C): 2016-2022, Shenzhen Yahboom Tech
* @file         main.c
* @author       Gengyue
* @version      V1.0
* @date         2020.05.27
* @brief        Touch screen read coordinate data
* @details      
* @par History  Description below
*                 
* version:	V1.0: Through the serial port, print out the currently touched coordinates (X, Y values).
*/
#include "sleep.h"
#include "gpiohs.h"
#include "lcd.h"
#include "sysctl.h"
#include "ft6236u.h"
#include "st7789.h"
#include "pin_config.h"

extern const unsigned char gImage_logo[153608];

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
    *lcd_cs	    36
    *lcd_rst	37
    *lcd_rs	    38
    *lcd_wr 	39
    **/
    fpioa_set_function(PIN_LCD_CS, FUNC_LCD_CS);
    fpioa_set_function(PIN_LCD_RST, FUNC_LCD_RST);
    fpioa_set_function(PIN_LCD_RS, FUNC_LCD_RS);
    fpioa_set_function(PIN_LCD_WR, FUNC_LCD_WR);
    
    /* Enable SPI0 and DVP */
    sysctl_set_spi0_dvp_data(1);

    /* I2C FT6236 */
    // fpioa_set_function(PIN_FT_RST, FUNC_FT_RST);
    fpioa_set_function(PIN_FT_INT, FUNC_FT_INT);
    fpioa_set_function(PIN_FT_SCL, FUNC_FT_SCL);
    fpioa_set_function(PIN_FT_SDA, FUNC_FT_SDA);
}

/**
* Function       lcd_clear_coord
* @author        Gengyue
* @date          2020.05.27
* @brief         Clear coordinate data
* @param[in]     void
* @param[out]    void
* @retval        void
* @par History   no
*/
void lcd_clear_coord(void)
{
    uint32_t color = 0xFFFFFFFF;
    uint8_t x1 = 120;
    uint8_t y1 = 200;
    uint8_t width = 100;
    uint8_t height = 16;
    
    lcd_set_area(x1, y1, x1 + width - 1, y1 + height - 1);
    tft_fill_data(&color, width * height / 2);
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
    /* Hardware pin initialization */
    hardware_init();

    /* Set IO port voltage*/
    io_set_power();

    /* System interrupt initialization, and enable global interrupt */
    plic_init();
    sysctl_enable_irq();
    
    /* Initialize LCD*/
    lcd_init();

    /* Display image */
    uint16_t * img = &gImage_logo; 
    lcd_draw_picture_half(0, 0, 320, 240, img);
    sleep(1);
    lcd_draw_string(16, 40, "Hello Yahboom", RED);
    lcd_draw_string(16, 60, "Nice to meet you!", BLUE);
    
    /* Initialize Touchpad */
    ft6236_init();
    printf("Hi!Please touch the screen to get coordinates!\n");
    lcd_draw_string(16, 180, "Please touch the screen to get coord!", RED);

    /* Displayed coordinates */
    char coord[30];
    uint8_t is_refresh = 0;

    while (1)
    {
        /* Refresh the data bit, clear the last displayed data*/
        if (is_refresh)
        {
            lcd_clear_coord();
            is_refresh = 0;
        }
        
        /* If you touch the touch screen */
        if (ft6236.touch_state & TP_COORD_UD)
        {
            ft6236.touch_state &= ~TP_COORD_UD;
            /* Scan touch screen */
            ft6236_scan();
            /* Serial print X Y coordinates */
            printf("X=%d, Y=%d \n ", ft6236.touch_x, ft6236.touch_y);
            sprintf(coord, "(%d, %d)", ft6236.touch_x, ft6236.touch_y);

            lcd_draw_string(120, 200, coord, BLUE);
            is_refresh = 1;
        }
        /* Delay 80 milliseconds to ensure normal refresh of screen data */
        msleep(80);
    }

    return 0;
}

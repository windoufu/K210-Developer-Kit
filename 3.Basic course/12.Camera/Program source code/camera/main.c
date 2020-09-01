/**
* @par  Copyright (C): 2016-2022, Shenzhen Yahboom Tech
* @file         main.c
* @author       Gengyue
* @version      V1.0
* @date         2020.05.27
* @brief        The camera shows the current picture
* @details      
* @par History  Description below
*                 
* version:	V1.0: The camera captures the current picture and displays it on the LCD.
*/
#include "unistd.h"
#include "gpiohs.h"
#include "lcd.h"
#include "st7789.h"
#include "sysctl.h"
#include "uarths.h"
#include "ov2640.h"
#include "dvp_cam.h"
#include "pin_config.h"

extern const unsigned char gImage_logo[153608];

/**
* Function       io_set_power
* @author        Gengyue
* @date          2020.05.27
* @brief         Set the voltage in the power domain of the camera and display
* @param[in]     void
* @param[out]    void
* @retval        void
* @par History   NO
*/
void io_set_power(void)
{
    sysctl_set_power_mode(SYSCTL_POWER_BANK6, SYSCTL_POWER_V18);
    sysctl_set_power_mode(SYSCTL_POWER_BANK7, SYSCTL_POWER_V18);
}

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
    /* lcd */
    fpioa_set_function(PIN_LCD_CS,  FUNC_LCD_CS);
    fpioa_set_function(PIN_LCD_RST, FUNC_LCD_RST);
    fpioa_set_function(PIN_LCD_RS,  FUNC_LCD_RS);
    fpioa_set_function(PIN_LCD_WR,  FUNC_LCD_WR);

    /* DVP camera */
    fpioa_set_function(PIN_DVP_RST,   FUNC_CMOS_RST);
    fpioa_set_function(PIN_DVP_PWDN,  FUNC_CMOS_PWDN);
    fpioa_set_function(PIN_DVP_XCLK,  FUNC_CMOS_XCLK);
    fpioa_set_function(PIN_DVP_VSYNC, FUNC_CMOS_VSYNC);
    fpioa_set_function(PIN_DVP_HSYNC, FUNC_CMOS_HREF);
    fpioa_set_function(PIN_DVP_PCLK,  FUNC_CMOS_PCLK);
    fpioa_set_function(PIN_DVP_SCL,   FUNC_SCCB_SCLK);
    fpioa_set_function(PIN_DVP_SDA,   FUNC_SCCB_SDA);
    
    /* Enable SPI0 and DVP */
    sysctl_set_spi0_dvp_data(1);
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
    /* Hardware pin initialization */
    hardware_init();
    
    /* Set the IO port voltage */
    io_set_power();

    /* Set the system clock and the DVP clock */
    sysctl_pll_set_freq(SYSCTL_PLL0, 800000000UL);
    sysctl_pll_set_freq(SYSCTL_PLL1, 300000000UL);
    sysctl_pll_set_freq(SYSCTL_PLL2, 45158400UL);
    uarths_init();

    /* System interrupt initialization, enabling global interrupt */
    plic_init();
    sysctl_enable_irq();
    
    /* Initialize LCD */
    lcd_init();
    
    /* LCD display picture */
    uint16_t *img = &gImage_logo;
    lcd_draw_picture_half(0, 0, 320, 240, img);
    lcd_draw_string(16, 40, "Hello Yahboom!", RED);
    lcd_draw_string(16, 60, "Nice to meet you!", BLUE);
    sleep(1);

    /* Initialize camera*/
    ov2640_init();

    while (1)
    {
        /* Wait for the end of the camera acquisition, and then clear the end sign */
        while (g_dvp_finish_flag == 0)
            ;
        g_dvp_finish_flag = 0;

        /* Display picture*/
        lcd_draw_picture(0, 0, 320, 240, display_buf_addr);
    }

    return 0;
}

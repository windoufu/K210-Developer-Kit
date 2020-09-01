#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include "dvp.h"
#include "fpioa.h"
#include "lcd.h"
#include "ov2640.h"
#include "plic.h"
#include "sysctl.h"
#include "uarths.h"
#include "st7789.h"
#include "dvp_cam.h"
#include "utils.h"
#include "kpu.h"
#include "l_conv.h"
#include "sleep.h"
#include "encoding.h"
#include "gpiohs.h"
#include "pin_config.h"
#include "dvp_cam.h"


int key_flag = 0;
gpio_pin_value_t key_state = 1;
volatile uint8_t g_ai_done_flag;
uint8_t g_ai_buf_out[320 * 240 * 3] __attribute__((aligned(128)));

/* KPU done*/
static int kpu_done(void *ctx)
{
	g_ai_done_flag = 1;
	return 0;
}

//  Convolution pooling batch normalization activates output bias
float conv_data[9*3*3] ={
//origin
//R
0,0,0,0,1,0,0,0,0,
0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,
//G
0,0,0,0,0,0,0,0,0,
0,0,0,0,1,0,0,0,0,
0,0,0,0,0,0,0,0,0,
//B
0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,
0,0,0,0,1,0,0,0,0,
};

int demo_index=0;
const float conv_data_demo[4][9*3*3] ={
{	//origin
//R
0,0,0,0,1,0,0,0,0,
0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,
//G
0,0,0,0,0,0,0,0,0,
0,0,0,0,1,0,0,0,0,
0,0,0,0,0,0,0,0,0,
//B
0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,
0,0,0,0,1,0,0,0,0,},
{	//edge
//R
-1,-1,-1,-1,8,-1,-1,-1,-1,
0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,
//G
0,0,0,0,0,0,0,0,0,
-1,-1,-1,-1,8,-1,-1,-1,-1,
0,0,0,0,0,0,0,0,0,
//B
0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,
-1,-1,-1,-1,8,-1,-1,-1,-1,},
{	//sharp
//R
-1,-1,-1,-1,9,-1,-1,-1,-1,
0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,
//G
0,0,0,0,0,0,0,0,0,
-1,-1,-1,-1,9,-1,-1,-1,-1,
0,0,0,0,0,0,0,0,0,
//B
0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,
-1,-1,-1,-1,9,-1,-1,-1,-1,},
{	//relievo
//R
2,0,0,0,-1,0,0,0,-1,
0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,
//G
0,0,0,0,0,0,0,0,0,
2,0,0,0,-1,0,0,0,-1,
0,0,0,0,0,0,0,0,0,
//B
0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,
2,0,0,0,-1,0,0,0,-1,},
};



/**
* Function       hardware_init
* @author        Gengyue
* @date          2020.05.27
* @brief         Hardware initialization, binding GPIO port
* @param[in]     void
* @param[out]    void
* @retval        void
* @par History   no
*/
void hardware_init(void)
{
    /* button */
	fpioa_set_function(PIN_KEYPAD_MIDDLE, FUNC_KEYPAD_MIDDLE);

    /* LCD */
    fpioa_set_function(PIN_LCD_CS,  FUNC_LCD_CS);
    fpioa_set_function(PIN_LCD_RST, FUNC_LCD_RST);
    fpioa_set_function(PIN_LCD_RS,  FUNC_LCD_RS);
    fpioa_set_function(PIN_LCD_WR,  FUNC_LCD_WR);

    // DVP camera
    fpioa_set_function(PIN_DVP_RST,   FUNC_CMOS_RST);
    fpioa_set_function(PIN_DVP_PWDN,  FUNC_CMOS_PWDN);
    fpioa_set_function(PIN_DVP_XCLK,  FUNC_CMOS_XCLK);
    fpioa_set_function(PIN_DVP_VSYNC, FUNC_CMOS_VSYNC);
    fpioa_set_function(PIN_DVP_HSYNC, FUNC_CMOS_HREF);
    fpioa_set_function(PIN_DVP_PCLK,  FUNC_CMOS_PCLK);
    fpioa_set_function(PIN_DVP_SCL,   FUNC_SCCB_SCLK);
    fpioa_set_function(PIN_DVP_SDA,   FUNC_SCCB_SDA);
    
    // enable SPI0 and DVP
    sysctl_set_spi0_dvp_data(1);
}

/**
* Function       io_set_power
* @author        Gengyue
* @date          2020.05.27
* @brief         Set bank6/ Bank7 power field 1.8V
* @param[in]     void
* @param[out]    void
* @retval        void
* @par History   no
*/
static void io_set_power(void)
{
	/* Set dvp and spi pin to 1.8V */
	sysctl_set_power_mode(SYSCTL_POWER_BANK6, SYSCTL_POWER_V18);
	sysctl_set_power_mode(SYSCTL_POWER_BANK7, SYSCTL_POWER_V18);
}

/* Convert image data format, because the camera output to AI is in RGB888 format, while the display needs RGB565 format */
void rgb888_to_565(uint8_t *src_r, uint8_t *src_g, uint8_t *src_b, uint16_t *dst, uint32_t len)
{
	uint32_t i;
	for (i = 0; i < len; i += 2)
	{
		dst[i] = (((uint16_t)(src_r[i + 1] >> 3)) << 11) + 
			(((uint16_t)src_g[i + 1] >> 2) << 5) + 
			(((uint16_t)src_b[i + 1]) >> 3);
		dst[i + 1] = (((uint16_t)(src_r[i] >> 3)) << 11) + 
			(((uint16_t)src_g[i] >> 2) << 5) + 
			(((uint16_t)src_b[i]) >> 3);
	}
}

/* Adding data (characters) to the original image*/
void lcd_ram_cpyimg(char *lcd, int lcdw, char *img, int imgw, int imgh, int x, int y)
{
	int i;
	for (i = 0; i < imgh; i++)
	{
		memcpy(lcd + lcdw * 2 * (y + i) + x * 2, img + imgw * 2 * i, imgw * 2);
	}
	return;
}

/* Upper left display mode */
void draw_text(void)
{
	char string_buf[8 * 16 * 2 * 16]; //16 characters
	char title[20];

	switch (demo_index)
	{
	case 0:
		sprintf(title, " origin ");
		lcd_ram_draw_string(title, (uint32_t *)string_buf, BLUE, BLACK);
		lcd_ram_cpyimg((char *)g_display_buf, 320, string_buf, strlen(title) * 8, 16, 0, 0);
		break;
	case 1:
		sprintf(title, "  edge  ");
		lcd_ram_draw_string(title, (uint32_t *)string_buf, BLUE, BLACK);
		lcd_ram_cpyimg((char *)g_display_buf, 320, string_buf, strlen(title) * 8, 16, 0, 0);
		break;
	case 2:
		sprintf(title, " sharp  ");
		lcd_ram_draw_string(title, (uint32_t *)string_buf, BLUE, BLACK);
		lcd_ram_cpyimg((char *)g_display_buf, 320, string_buf, strlen(title) * 8, 16, 0, 0);
		break;
	case 3:
		sprintf(title, "relievos");
		lcd_ram_draw_string(title, (uint32_t *)string_buf, BLUE, BLACK);
		lcd_ram_cpyimg((char *)g_display_buf, 320, string_buf, strlen(title) * 8, 16, 0, 0);
		break;
	
	default:
		break;
	}

	// sprintf(title, "% 2d % 2d % 2d", (int)conv_data[3], (int)conv_data[4], (int)conv_data[5]);
	// lcd_ram_draw_string(title, (uint32_t *)string_buf, BLUE, BLACK);
	// lcd_ram_cpyimg((char *)g_display_buf, 320, string_buf, strlen(title) * 8, 16, 0, 16);

}

/* Key interrupt callback*/
int key_irq_cb(void *ctx)
{
	key_flag = 1;
	key_state = gpiohs_get_pin(KEYPAD_MIDDLE_GPIONUM);
	return 0;
}

/* Initializing key */
void init_key(void)
{
    // Set the GPIO mode for the keys to be pull-up input
    gpiohs_set_drive_mode(KEYPAD_MIDDLE_GPIONUM, GPIO_DM_INPUT_PULL_UP);
    // Set the GPIO level trigger mode of the key to be rise edge and fall edge
    gpiohs_set_pin_edge(KEYPAD_MIDDLE_GPIONUM, GPIO_PE_BOTH);
    // Set the interrupt callback for the key GPIO port
    gpiohs_irq_register(KEYPAD_MIDDLE_GPIONUM, 1, key_irq_cb, NULL);
}

/**
* Function       main
* @author        Gengyue
* @date          2020.05.27
* @brief         Main function, the entry of the program
* @param[in]     void
* @param[out]    void
* @retval        void
* @par History   no
*/
int main(void)
{
	hardware_init();
	io_set_power();

    /* Set the system clock and the DVP clock*/
    sysctl_pll_set_freq(SYSCTL_PLL0, 800000000UL);
    sysctl_pll_set_freq(SYSCTL_PLL1, 300000000UL);
    sysctl_pll_set_freq(SYSCTL_PLL2, 45158400UL);
    uarths_init();

	/* System interrupt initialization*/
    plic_init();
	/* Enable system global interrupt */
    sysctl_enable_irq();
    
	/* Initialize the display screen and display a one-second image */
	printf("LCD init\r\n");
	lcd_init();
	lcd_draw_picture_half(0, 0, 320, 240, gImage_logo);
	sleep(1);

	/* Initialize ov2640 camera */
	printf("ov2640 init\r\n");
	ov2640_init();

	/* Initialize button*/
	init_key();

	/* Initialize kpu */
	kpu_task_t task;
	conv_init(&task, CONV_3_3, conv_data);

	printf("KPU TASK INIT, FREE MEM: %ld\r\n", get_free_heap_size());
	printf("Please press the keypad to switch mode\r\n");
	
	while (1)
	{
		while (g_dvp_finish_flag == 0)
			;
		/* Starting */
		conv_run(&task, g_ai_buf_in, g_ai_buf_out, kpu_done);
		
		while (!g_ai_done_flag)
			;
		g_ai_done_flag = 0;
		g_dvp_finish_flag = 0;
		/* Convert to RGB565 format supported by LCD */
		rgb888_to_565(g_ai_buf_out, g_ai_buf_out + 320 * 240, g_ai_buf_out + 320 * 240 * 2, 
			(uint16_t *)g_display_buf, 320 * 240);
		
		/* The upper left corner indicates which mode to write letters in */
		draw_text();
		/* Display image */
		lcd_draw_picture(0, 0, 320, 240, g_display_buf);
		
		if (key_flag) 
		{
			if (key_state == 0) //press
			{
				msleep(20); 
				key_flag = 0;
				demo_index = (demo_index + 1) % 4;
				memcpy((void *)conv_data, (void *)(conv_data_demo[demo_index]), 
					3 * 3 * 3 * 3 * sizeof(float));
				conv_init(&task, CONV_3_3, conv_data);
			}
			else 
			{
				msleep(20); 
				key_flag = 0;
			}
		}
	}
	return 0;
}

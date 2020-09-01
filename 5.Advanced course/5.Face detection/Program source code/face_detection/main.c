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
* version:	V1.0:Convert 320*240 images to data format and display on LCD.
*/

#include "gpiohs.h"
#include "lcd.h"
#include "st7789.h"
#include "image.h"
#include "sysctl.h"
#include "ov2640.h"
#include "dvp_cam.h"
#include "dvp.h"
#include "pin_config.h"
#include "uarths.h"
#include "region_layer.h"
#include "kpu.h"
#include "stdio.h"
#include "bsp.h"
#include "w25qxx.h"
#include "incbin.h"
#define INCBIN_STYLE INCBIN_STYLE_SNAKE
#define INCBIN_PREFIX



volatile uint32_t g_ai_done_flag;
//volatile uint8_t g_dvp_finish_flag;
//static image_t kpu_image, display_image;

kpu_model_context_t face_detect_task;
static region_layer_t face_detect_rl;
static obj_info_t face_detect_info;
#define ANCHOR_NUM 5

float g_anchor[ANCHOR_NUM * 2] = {0.57273, 0.677385, 1.87446, 2.06253, 3.33843, 5.47434, 7.88282, 3.52778, 9.77052, 9.16828};



//kpu_task_t task;

#define  LOAD_KMODEL_FROM_FLASH  1

#if LOAD_KMODEL_FROM_FLASH
#define KMODEL_SIZE (380 * 1024)
uint8_t* model_data;
#else
INCBIN(model, "detect.kmodel");
#endif


static int ai_done(void *ctx)
{
    g_ai_done_flag = 1;
    return 0;
}


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
* @par History   no
*/
void hardware_init(void)
{
    // fpioa mapping
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

    // DVP camera
    fpioa_set_function(PIN_DVP_RST, FUNC_CMOS_RST);
    fpioa_set_function(PIN_DVP_PWDN, FUNC_CMOS_PWDN);
    fpioa_set_function(PIN_DVP_XCLK, FUNC_CMOS_XCLK);
    fpioa_set_function(PIN_DVP_VSYNC, FUNC_CMOS_VSYNC);
    fpioa_set_function(PIN_DVP_HSYNC, FUNC_CMOS_HREF);
    fpioa_set_function(PIN_DVP_PCLK, FUNC_CMOS_PCLK);
    fpioa_set_function(PIN_DVP_SCL, FUNC_SCCB_SCLK);
    fpioa_set_function(PIN_DVP_SDA, FUNC_SCCB_SDA);
    
    // enable SPI0 and DVP
    sysctl_set_spi0_dvp_data(1);

}

//Draw a face recognition box
static void draw_edge(uint32_t *gram, obj_info_t *obj_info, uint32_t index, uint16_t color)
{
    uint32_t data = ((uint32_t)color << 16) | (uint32_t)color;
    uint32_t *addr1, *addr2, *addr3, *addr4, x1, y1, x2, y2;

    x1 = obj_info->obj[index].x1;
    y1 = obj_info->obj[index].y1;
    x2 = obj_info->obj[index].x2;
    y2 = obj_info->obj[index].y2;

    if (x1 <= 0)
        x1 = 1;
    if (x2 >= 319)
        x2 = 318;
    if (y1 <= 0)
        y1 = 1;
    if (y2 >= 239)
        y2 = 238;

    addr1 = gram + (320 * y1 + x1) / 2;
    addr2 = gram + (320 * y1 + x2 - 8) / 2;
    addr3 = gram + (320 * (y2 - 1) + x1) / 2;
    addr4 = gram + (320 * (y2 - 1) + x2 - 8) / 2;
    for (uint32_t i = 0; i < 4; i++)
    {
        *addr1 = data;
        *(addr1 + 160) = data;
        *addr2 = data;
        *(addr2 + 160) = data;
        *addr3 = data;
        *(addr3 + 160) = data;
        *addr4 = data;
        *(addr4 + 160) = data;
        addr1++;
        addr2++;
        addr3++;
        addr4++;
    }
    addr1 = gram + (320 * y1 + x1) / 2;
    addr2 = gram + (320 * y1 + x2 - 2) / 2;
    addr3 = gram + (320 * (y2 - 8) + x1) / 2;
    addr4 = gram + (320 * (y2 - 8) + x2 - 2) / 2;
    for (uint32_t i = 0; i < 8; i++)
    {
        *addr1 = data;
        *addr2 = data;
        *addr3 = data;
        *addr4 = data;
        addr1 += 160;
        addr2 += 160;
        addr3 += 160;
        addr4 += 160;
    }
}

/*System clock initialization */
void sysclock_init(void)
{
    /* Set the system clock and the DVP clock */
    sysctl_pll_set_freq(SYSCTL_PLL0, 800000000UL);
    sysctl_pll_set_freq(SYSCTL_PLL1, 400000000UL);
    //sysctl_pll_set_freq(SYSCTL_PLL2, 45158400UL);
    sysctl_clock_enable(SYSCTL_CLOCK_AI);
}

/**
* Function       main
* @author        liusen
* @date          2020.06.29
* @brief         Main function, the entry of the program
* @param[in]     void
* @param[out]    void
* @retval        0
* @par History   no
*/
int main(void)
{

    sysclock_init();   /* System clock initialization*/
    uarths_init();     /* Serial port initialization*/
    hardware_init();   /* Hardware pin initialization*/
    io_set_power();    /* Set the IO port voltage*/
    plic_init();       /* System interrupt initialization */

    
    printf("flash init\n");
    w25qxx_init(3, 0);           /* flash init */
    w25qxx_enable_quad_mode();   /* flash Quadruple mode on*/

/*Kmodel loading mode: 1: separate burning mode 2: directly merged with the code compiled*/
#if LOAD_KMODEL_FROM_FLASH
    model_data = (uint8_t*)malloc(KMODEL_SIZE + 255);
    uint8_t *model_data_align = (uint8_t*)(((uintptr_t)model_data+255)&(~255));
    w25qxx_read_data(0xA00000, model_data_align, KMODEL_SIZE, W25QXX_QUAD_FAST);
#else
    uint8_t *model_data_align = model_data;
#endif

    
    //LCD initialization
    lcd_init();
    lcd_draw_picture_half(0, 0, 320, 240, (uint32_t *)logo);
    lcd_draw_string(100, 40, "Hello Yahboom!", RED);
    lcd_draw_string(100, 60, "Demo: Face Detect!", BLUE);
    sleep(1);

    ov2640_init();

    /* init face detect model  KPU task handle kmodel data*/
    //Kmodel needs to be used in conjunction with NNCASE to load
    if (kpu_load_kmodel(&face_detect_task, model_data_align) != 0)  
    {
        printf("\nmodel init error\n");
        while (1);
    }
    //Face layer configuration parameters
    face_detect_rl.anchor_number = ANCHOR_NUM;
    face_detect_rl.anchor = g_anchor;
    face_detect_rl.threshold = 0.7;
    face_detect_rl.nms_value = 0.3;
    region_layer_init(&face_detect_rl, 20, 15, 30, 320, 240);

    
    printf("REGION LAYER INIT, FREE MEM: %ld\r\n", (long)get_free_heap_size());
    
    sysctl_enable_irq();
    /* system start */
    printf("System start \n");

    while (1)
    {
        g_dvp_finish_flag = 0;
        while (g_dvp_finish_flag == 0);   
        
        /* run face detect */
        g_ai_done_flag = 0;
        //Running the KModel KPU task handles the arguments to the callback function after the source data DMA channel completes
        kpu_run_kmodel(&face_detect_task, g_ai_red_buf_addr, DMAC_CHANNEL5, ai_done, NULL);
        while(!g_ai_done_flag);     //Wait for KPU processing to complete
        
        float *output;
        size_t output_size;
        //Gets the result of KPU final processing the index value of the KPU task handle result size in bytes
        kpu_get_output(&face_detect_task, 0, (uint8_t **)&output, &output_size);

        
        /*The algorithm detects face */
        face_detect_rl.input = output;
        region_layer_run(&face_detect_rl, &face_detect_info);

        /*Circle the face according to the return value */
        for (uint32_t face_cnt = 0; face_cnt < face_detect_info.obj_number; face_cnt++)
        {
            draw_edge((uint32_t *)display_buf_addr, &face_detect_info, face_cnt, RED);
        }
        /* display result */
        lcd_draw_picture(0, 0, 320, 240, (uint32_t *)display_buf_addr);
    }

    return 0;
}

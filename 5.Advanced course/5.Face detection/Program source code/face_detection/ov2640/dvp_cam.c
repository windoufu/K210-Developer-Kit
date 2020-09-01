#include <stdio.h>
#include "dvp_cam.h"
#include "dvp.h"
#include "plic.h"
#include "uarths.h"
#include "sysctl.h"
#include "iomem.h"
#include "kpu.h"

uint8_t *g_ai_buf_in = NULL;
uint32_t g_ai_red_buf_addr, g_ai_green_buf_addr, g_ai_blue_buf_addr;

uint32_t *display_buf = NULL;
uint32_t display_buf_addr = 0;
volatile uint8_t g_dvp_finish_flag;

static int on_irq_dvp(void *ctx)
{
    /*Read the DVP interrupt status, refresh the data of the display address if it is finished, and clear the interrupt flag, otherwise read the camera data*/
    if (dvp_get_interrupt(DVP_STS_FRAME_FINISH))
    {
        dvp_set_display_addr((uint32_t)display_buf_addr);
        dvp_clear_interrupt(DVP_STS_FRAME_FINISH);
        g_dvp_finish_flag = 1;
    }
    else
    {
        if (g_dvp_finish_flag == 0)
            dvp_start_convert();
        dvp_clear_interrupt(DVP_STS_FRAME_START);
    }
    return 0;
}

void dvp_cam_init(void)
{
    /* DVP Initialization */
    dvp_init(8);
    dvp_set_xclk_rate(24000000);
    dvp_enable_burst();
    dvp_set_output_enable(0, 1);
    dvp_set_output_enable(1, 1);
    dvp_set_image_format(DVP_CFG_RGB_FORMAT);
    dvp_set_image_size(CAM_WIDTH_PIXEL, CAM_HIGHT_PIXEL);

    /* Set the DVP display address parameters and interrupt */
    display_buf = (uint32_t*)iomem_malloc(CAM_WIDTH_PIXEL * CAM_HIGHT_PIXEL * 2);
    display_buf_addr = display_buf;
    dvp_set_display_addr((uint32_t)display_buf_addr);

    g_ai_buf_in = (uint8_t*)iomem_malloc(CAM_WIDTH_PIXEL * CAM_HIGHT_PIXEL * 3);
    g_ai_red_buf_addr =  (uint32_t)&g_ai_buf_in[0];
    g_ai_green_buf_addr = (uint32_t)&g_ai_buf_in[CAM_WIDTH_PIXEL * CAM_HIGHT_PIXEL];
    g_ai_blue_buf_addr = (uint32_t)&g_ai_buf_in[CAM_WIDTH_PIXEL * CAM_HIGHT_PIXEL * 2];
    dvp_set_ai_addr((uint32_t)g_ai_red_buf_addr, (uint32_t)g_ai_green_buf_addr, (uint32_t)g_ai_blue_buf_addr);

    dvp_config_interrupt(DVP_CFG_START_INT_ENABLE | DVP_CFG_FINISH_INT_ENABLE, 0);
    dvp_disable_auto();
}

void dvp_cam_set_irq(void)
{
     /* DVP interrupt configuration: Interrupt priority, interrupt callback, enable DVP interrupt */
    printf("DVP interrupt config\r\n");
    plic_set_priority(IRQN_DVP_INTERRUPT, 1);
    plic_irq_register(IRQN_DVP_INTERRUPT, on_irq_dvp, NULL);
    plic_irq_enable(IRQN_DVP_INTERRUPT);

    /*Global interrupt enabled system */
    sysctl_enable_irq();

      /* Clear the DVP interrupt bits */
    g_dvp_finish_flag = 0;
    dvp_clear_interrupt(DVP_STS_FRAME_START | DVP_STS_FRAME_FINISH);
    dvp_config_interrupt(DVP_CFG_START_INT_ENABLE | DVP_CFG_FINISH_INT_ENABLE, 1);
}

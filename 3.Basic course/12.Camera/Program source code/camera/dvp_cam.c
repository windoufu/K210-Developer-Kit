#include <stdio.h>
#include "dvp_cam.h"
#include "dvp.h"
#include "plic.h"
#include "sysctl.h"
#include "iomem.h"

uint32_t *display_buf = NULL;
uint32_t display_buf_addr = 0;
volatile uint8_t g_dvp_finish_flag;

/* dvp Interrupt callback function */
static int on_dvp_irq_cb(void *ctx)
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

/* dvp Initialization */
void dvp_cam_init(void)
{
    /* DVP is initialized, and the register length of SCCB is set to 8 bits*/
    dvp_init(8);
    /* Set the input clock to 24000000*/
    dvp_set_xclk_rate(24000000);
    /* Enable burst mode */
    dvp_enable_burst();
    /* Turn off the AI output mode, enable display mode*/
    dvp_set_output_enable(DVP_OUTPUT_AI, 0);
    dvp_set_output_enable(DVP_OUTPUT_DISPLAY, 1);
    /* Set the output format to RGB*/
    dvp_set_image_format(DVP_CFG_RGB_FORMAT);
    /* Set the output pixel size to 320*240 */
    dvp_set_image_size(CAM_WIDTH_PIXEL, CAM_HIGHT_PIXEL);

    /* Set the DVP display address parameters and interrupt */
    display_buf = (uint32_t*)iomem_malloc(CAM_WIDTH_PIXEL * CAM_HIGHT_PIXEL * 2);
    display_buf_addr = display_buf;
    dvp_set_display_addr((uint32_t)display_buf_addr);
    dvp_config_interrupt(DVP_CFG_START_INT_ENABLE | DVP_CFG_FINISH_INT_ENABLE, 0);
    dvp_disable_auto();
}

void dvp_cam_set_irq(void)
{
    /* DVP interrupt configuration: Interrupt priority, interrupt callback, enable DVP interrupt */
    printf("DVP interrupt config\r\n");
    plic_set_priority(IRQN_DVP_INTERRUPT, 1);
    plic_irq_register(IRQN_DVP_INTERRUPT, on_dvp_irq_cb, NULL);
    plic_irq_enable(IRQN_DVP_INTERRUPT);

    /* Clear the DVP interrupt bits */
    g_dvp_finish_flag = 0;
    dvp_clear_interrupt(DVP_STS_FRAME_START | DVP_STS_FRAME_FINISH);
    dvp_config_interrupt(DVP_CFG_START_INT_ENABLE | DVP_CFG_FINISH_INT_ENABLE, 1);
}

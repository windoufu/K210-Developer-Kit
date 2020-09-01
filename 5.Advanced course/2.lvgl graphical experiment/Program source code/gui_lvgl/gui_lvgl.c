#include "gui_lvgl.h"
#include "timer.h"
#include "ft6236u.h"
#include "lcd.h"


/**
* Function       timer_irq_cb
* @author        Gengyue
* @date          2020.05.27
* @brief         Timer interrupt callback
* @param[in]     ctx：callback parameter
* @param[out]    void
* @retval        0
* @par History   no
*/
static int timer_irq_cb(void * ctx)
{
    lv_task_handler();
    lv_tick_inc(1);
    return 0;
}

/**
* Function       mTimer_init
* @author        Gengyue
* @date          2020.05.27
* @brief         Initialize and start the timer
* @param[in]     void
* @param[out]    void
* @retval        0
* @par History   no
*/
static void mTimer_init(void)
{
    timer_init(TIMER_DEVICE_0);
    timer_set_interval(TIMER_DEVICE_0, TIMER_CHANNEL_0, 1e6);
    timer_irq_register(TIMER_DEVICE_0, TIMER_CHANNEL_0, 0, 1, timer_irq_cb, NULL);

    timer_set_enable(TIMER_DEVICE_0, TIMER_CHANNEL_0, 1);
}

/**
* Function       my_disp_flush
* @author        Gengyue
* @date          2020.05.27
* @brief         Refresh data, LCD display screen
* @param[in]     disp：Display structure
* @param[in]     area：Display area
* @param[in]     color_p：Display data
* @param[out]    void
* @retval        0
* @par History   no
*/
static void my_disp_flush(lv_disp_drv_t * disp, const lv_area_t * area, lv_color_t * color_p)
{
    uint16_t x1 = area->x1;
    uint16_t x2 = area->x2;
    uint16_t y1 = area->y1;
    uint16_t y2 = area->y2;

    lcd_draw_picture_half((uint16_t)x1, (uint16_t)y1, 
        (uint16_t)(x2 - x1 + 1), (uint16_t)(y2 - y1 + 1), 
        (uint16_t *)color_p);
    lv_disp_flush_ready(disp); /* Indicate you are ready with the flushing*/
}

/**
* Function       my_touchpad_read
* @author        Gengyue
* @date          2020.05.27
* @brief         Input detection, touch panel to read data
* @param[in]     indev_drv：Input structure
* @param[out]    data：Data read
* @retval        0
* @par History   no
*/
static bool my_touchpad_read(lv_indev_drv_t * indev_drv, lv_indev_data_t * data)
{
    static int a_state = 0;
    if (ft6236.touch_state & TP_COORD_UD)
    {
        ft6236.touch_state &= ~TP_COORD_UD;
        ft6236_scan();
        data->point.x = ft6236.touch_x;
        data->point.y = ft6236.touch_y;
        data->state = LV_INDEV_STATE_PR;
        a_state = 1;
        return false;
    }
    else if (ft6236.touch_state & 0xC0)
    {
        if (a_state == 1)
        {
            a_state = 0;
            data->point.x = ft6236.touch_x;
            data->point.y = ft6236.touch_y;
            data->state = LV_INDEV_STATE_REL;
            return false;
        }
    }
    return false;
}

/**
* Function       lvgl_disp_input_init
* @author        Gengyue
* @date          2020.05.27
* @brief         Initialize the input and display of lvgl
* @param[in]     void
* @param[out]    void
* @retval        0
* @par History   no
*/
void lvgl_disp_input_init(void)
{
    lv_init();

    static lv_disp_buf_t disp_buf;
    static lv_color_t buf[LV_HOR_RES_MAX * 10];
    lv_disp_buf_init(&disp_buf, buf, NULL, LV_HOR_RES_MAX * 10);

    lv_disp_drv_t disp_drv;               /*Descriptor of a display driver*/
    lv_disp_drv_init(&disp_drv);          /*Basic initialization*/
    disp_drv.flush_cb = my_disp_flush;    /*Set your driver function*/
    disp_drv.buffer = &disp_buf;          /*Assign the buffer to the display*/
    lv_disp_drv_register(&disp_drv);      /*Finally register the driver*/

    // input
    lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);             /*Descriptor of a input device driver*/
    indev_drv.type = LV_INDEV_TYPE_POINTER;    /*Touch pad is a pointer-like device*/
    indev_drv.read_cb = my_touchpad_read;      /*Set your driver function*/
    lv_indev_drv_register(&indev_drv);         /*Finally register the driver*/

    /* Initialize and start the timer*/
    mTimer_init();
}

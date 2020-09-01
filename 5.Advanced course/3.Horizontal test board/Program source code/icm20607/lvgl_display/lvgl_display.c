#include "lvgl_display.h"
#include "timer.h"
#include "lvgl.h"
#include "bsp.h"
#include "lcd.h"

/* Timer interrupt callback */
static int timer_irq_cb(void * ctx)
{
    lv_task_handler();
    lv_tick_inc(1);
    return 0;
}

/* Timer initialization and start timer*/
static void mTimer_init(void)
{
    timer_init(TIMER_DEVICE_0);
    timer_set_interval(TIMER_DEVICE_0, TIMER_CHANNEL_0, 1e6);
    timer_irq_register(TIMER_DEVICE_0, TIMER_CHANNEL_0, 0, 1, timer_irq_cb, NULL);

    timer_set_enable(TIMER_DEVICE_0, TIMER_CHANNEL_0, 1);
}

/* Display refresh callback */
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

/* lvgl display initialization, start timer */
void lvgl_disp_init(void)
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

    mTimer_init();
}

/* Declare robot icon data and image structure*/
LV_IMG_DECLARE(img_robot)
lv_obj_t * image_robot;

/* Create robot icon*/
void lvgl_creat_image(void)
{
    image_robot = lv_img_create(lv_scr_act(), NULL);
    lv_img_set_src(image_robot, &img_robot);
    lv_obj_align(image_robot, NULL, LV_ALIGN_IN_TOP_LEFT, -100, 0);
}

/*Modify the position of the robot icon */
void lvgl_move_image(int16_t x, int16_t y)
{
    lv_obj_set_pos(image_robot, (lv_coord_t)x, (lv_coord_t)y);
}

/*Convert the roll angle to the X coordinate of the image */
int16_t lvgl_get_x(float a)
{
    int16_t temp = (int16_t)a;
    temp = temp * (-1);
    temp = temp > 0 ? (temp - 180) : (temp + 180);
    temp = convert_value(temp, -90, 90, 0, 320-IMG_ROBOT_WIDTH);

    return temp;
}

/*Convert the pitch angle to the Y coordinate of the image*/
int16_t lvgl_get_y(float a)
{
    int16_t temp = (int16_t)a;
    // temp = temp * (-1);
    temp = convert_value(temp, -90, 90, 0, 240-IMG_ROBOT_HIGH);

    return temp;
}

/*Mapping relationship conversion */
int16_t convert_value(int16_t x, int16_t in_min, int16_t in_max, int16_t out_min, int16_t out_max)
{
    int16_t res = (x-in_min)*(out_max-out_min)/(in_max-in_min)+out_min;
    if (res <= out_min) res = out_min;
    else if (res >= out_max) res = out_max;
    return res;
}

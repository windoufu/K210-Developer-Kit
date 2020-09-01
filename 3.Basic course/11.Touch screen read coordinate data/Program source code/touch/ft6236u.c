#include <stdio.h>

#include "ft6236u.h"
#include "i2c_ctl.h"
#include "pin_config.h"

#include "gpiohs.h"
#include "sleep.h"
#include "plic.h"
#include "sysctl.h"

/* ft6236 Structure variable*/
ft6236_touch_point_t ft6236;

/* I2C write data */
static void ft_i2c_write(uint8_t reg, uint8_t data)
{
    i2c_hd_write(FT6236_I2C_ADDR, reg, data);
}

/* I2C read data  */
static void ft_i2c_read(uint8_t reg, uint8_t *data_buf, uint16_t length)
{
    i2c_hd_read(FT6236_I2C_ADDR, reg, data_buf, length);
}

/* ft6236 initialization*/
void ft6236_init(void)
{
    ft6236.touch_state = 0;
    ft6236.touch_x = 0;
    ft6236.touch_y = 0;

    /* Hardware initialization */
    ft6236_hardware_init();

    /* Software initializatio */
    i2c_hardware_init(FT6236_I2C_ADDR);
    ft_i2c_write(FT_DEVIDE_MODE, 0x00);

    ft_i2c_write(FT_ID_G_THGROUP, 0x12);    // 0x22

    ft_i2c_write(FT_ID_G_PERIODACTIVE, 0x06);
}

/* Reset FT_RST pin GPIO level*/
void ft6236_reset_pin(rst_level_t level)
{
    gpiohs_set_pin(FT_RST_GPIONUM, level);
}

/* Interrupt the callback function, modify the state of touch_state to touch */
void ft6236_isr_cb(void)
{
    ft6236.touch_state |= TP_COORD_UD;
}

/* FT6236 Hardware pin initialization */
void ft6236_hardware_init(void)
{
    /* Set to 1 when using a reset pin different from the screen*/
    #if (0) 
    {
        gpiohs_set_drive_mode(FT_RST_GOIONUM, GPIO_DM_OUTPUT);
        ft6236_reset_pin(LEVEL_LOW);
        msleep(50);
        ft6236_reset_pin(LEVEL_HIGH);
        msleep(120);
    }
    #endif

    gpiohs_set_drive_mode(FT_INT_GPIONUM, GPIO_DM_INPUT);
    gpiohs_set_pin_edge(FT_INT_GPIONUM, GPIO_PE_RISING);
    gpiohs_irq_register(FT_INT_GPIONUM, FT6236_IRQ_LEVEL, ft6236_isr_cb, NULL);
    msleep(5);
    
}

/* Scan FT6236, and read the XY value of the coordinate */
void ft6236_scan(void)
{
    uint8_t sta = 0;
    uint8_t data[4] = {0};
    ft_i2c_read(FT_REG_NUM_FINGER, &sta, 1);
    // printf("read-point:%x \n", sta);
    if (sta & 0x0f)
    {
        ft6236.touch_state = ~(0xFF << (sta & 0x0F));
        if (ft6236.touch_state & (1 << 0))
        {
            ft_i2c_read(FT_TP1_REG, data, 4);

            uint16_t temp_y = ((uint16_t)(data[0] & 0x0f) << 8) + data[1];
            ft6236.touch_y = (uint16_t)(240 - temp_y);
            // uint16_t temp_x = ((uint16_t)(data[2] & 0x0f) << 8) + data[3];
            ft6236.touch_x = ((uint16_t)(data[2] & 0x0f) << 8) + data[3];
            
            if ((data[0] & 0xC0) != 0x80)
            {
                ft6236.touch_x = ft6236.touch_y = 0;
                return;
            }
        }
        /* Touch the press mark*/
        ft6236.touch_state |= TP_PRES_DOWN;
    }
    else
    {
        /* Previously marked*/
        if (ft6236.touch_state & TP_PRES_DOWN)
        {
            /* Touch the release mark*/
            ft6236.touch_state &= ~0x80;
        }
        else
        {
            /* Clear coordinate value and clear touch valid mark*/
            ft6236.touch_x = 0;
            ft6236.touch_y = 0;
            ft6236.touch_state &= 0xe0;
        }
    }
}

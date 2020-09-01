#include "gpiohs.h"
#include "fpioa.h"
#include "rgb.h"
#include "pin_config.h"

/* Initialize the RGB lamp */
void rgb_init(rgb_color_t color)
{
    switch (color)
    {
    case EN_RGB_RED:
        fpioa_set_function(PIN_RGB_R, FUNC_RGB_R);
        gpiohs_set_drive_mode(RGB_R_GPIONUM, GPIO_DM_OUTPUT);
        gpiohs_set_pin(RGB_R_GPIONUM, GPIO_PV_HIGH);
        break;
    case EN_RGB_GREEN:
        fpioa_set_function(PIN_RGB_G, FUNC_RGB_G);
        gpiohs_set_drive_mode(RGB_G_GPIONUM, GPIO_DM_OUTPUT);
        gpiohs_set_pin(RGB_G_GPIONUM, GPIO_PV_HIGH);
        break;
    case EN_RGB_BLUE:
        fpioa_set_function(PIN_RGB_B, FUNC_RGB_B);
        gpiohs_set_drive_mode(RGB_B_GPIONUM, GPIO_DM_OUTPUT);
        gpiohs_set_pin(RGB_B_GPIONUM, GPIO_PV_HIGH);
        break;
    case EN_RGB_ALL:
        fpioa_set_function(PIN_RGB_R, FUNC_RGB_R);
        gpiohs_set_drive_mode(RGB_R_GPIONUM, GPIO_DM_OUTPUT);
        fpioa_set_function(PIN_RGB_G, FUNC_RGB_G);
        gpiohs_set_drive_mode(RGB_G_GPIONUM, GPIO_DM_OUTPUT);
        fpioa_set_function(PIN_RGB_B, FUNC_RGB_B);
        gpiohs_set_drive_mode(RGB_B_GPIONUM, GPIO_DM_OUTPUT);

        gpiohs_set_pin(RGB_R_GPIONUM, GPIO_PV_HIGH);
        gpiohs_set_pin(RGB_G_GPIONUM, GPIO_PV_HIGH);
        gpiohs_set_pin(RGB_B_GPIONUM, GPIO_PV_HIGH);
        break;
    
    default:
        break;
    }
}

/* Set RGB- red light status*/
void rgb_red_state(rgb_state_t state)
{
    gpiohs_set_pin(RGB_R_GPIONUM, state);
}

/* Set RGB- green light status */
void rgb_green_state(rgb_state_t state)
{
    gpiohs_set_pin(RGB_G_GPIONUM, state);
}

/* Set RGB- blue light status */
void rgb_blue_state(rgb_state_t state)
{
    gpiohs_set_pin(RGB_B_GPIONUM, state);
}

/* Set all RGB light on*/
void rgb_all_on(void)
{
    gpiohs_set_pin(RGB_R_GPIONUM, GPIO_PV_LOW);
    gpiohs_set_pin(RGB_G_GPIONUM, GPIO_PV_LOW);
    gpiohs_set_pin(RGB_B_GPIONUM, GPIO_PV_LOW);
}

/* et all RGB off*/
void rgb_all_off(void)
{
    gpiohs_set_pin(RGB_R_GPIONUM, GPIO_PV_HIGH);
    gpiohs_set_pin(RGB_G_GPIONUM, GPIO_PV_HIGH);
    gpiohs_set_pin(RGB_B_GPIONUM, GPIO_PV_HIGH);
}

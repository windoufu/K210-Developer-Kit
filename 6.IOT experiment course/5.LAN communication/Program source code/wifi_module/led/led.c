#include "gpiohs.h"
#include "fpioa.h"
#include "led.h"
#include "pin_config.h"


/*Initialize LED lights: num=0, only initialize LED0; num=1, only initialize LED1;
num=2, initialize LED0 and LED1 at the same time */
void led_init(uint8_t num)
{
    switch (num)
    {
    case 0:
        fpioa_set_function(PIN_LED_0, FUNC_LED0);
        gpiohs_set_drive_mode(LED0_GPIONUM, GPIO_DM_OUTPUT);
        gpiohs_set_pin(LED0_GPIONUM, GPIO_PV_HIGH);
        break;
    case 1:
        fpioa_set_function(PIN_LED_1, FUNC_LED1);
        gpiohs_set_drive_mode(LED1_GPIONUM, GPIO_DM_OUTPUT);
        gpiohs_set_pin(LED1_GPIONUM, GPIO_PV_HIGH);
        break;
    case 2:
        fpioa_set_function(PIN_LED_0, FUNC_LED0);
        gpiohs_set_drive_mode(LED0_GPIONUM, GPIO_DM_OUTPUT);
        
        fpioa_set_function(PIN_LED_1, FUNC_LED1);
        gpiohs_set_drive_mode(LED1_GPIONUM, GPIO_DM_OUTPUT);

        gpiohs_set_pin(LED0_GPIONUM, GPIO_PV_HIGH);
        gpiohs_set_pin(LED1_GPIONUM, GPIO_PV_HIGH);
        break;
    
    default:
        break;
    }
}

/* Set LED0 state */
void led0_state(uint8_t state)
{
    gpiohs_set_pin(LED0_GPIONUM, state);
}

/* Set LED1 status */
void led1_state(uint8_t state)
{
    gpiohs_set_pin(LED1_GPIONUM, state);
}




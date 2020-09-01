#include "keypad.h"
#include "gpiohs.h"
#include "timer.h"
#include "pin_config.h"

keypad_fifo_t keypad_fifo;         // FIFO is buffered by buttons
keypad_t keypad[EN_KEY_ID_MAX];

// Read the high and low values of the corresponding pins of the key, press to return to low, and release to return to high
static gpio_pin_value_t  get_key_value(key_id_t key_id)
{
    gpio_pin_value_t val = GPIO_PV_HIGH;
    switch (key_id)
    {
    case EN_KEY_ID_LEFT:
        val = gpiohs_get_pin(KEYPAD_LEFT_GPIONUM);
        break;
    case EN_KEY_ID_RIGHT:
        val = gpiohs_get_pin(KEYPAD_RIGHT_GPIONUM);
        break;
    case EN_KEY_ID_MIDDLE:
        val = gpiohs_get_pin(KEYPAD_MIDDLE_GPIONUM);
        break;
    default:
        break;
    }
    return val;
}

static uint8_t get_keys_state_hw(key_id_t key_id)
{
    gpio_pin_value_t key_value = get_key_value(key_id);
    
    if (key_value == GPIO_PV_LOW)
    {
        return PRESS;                // button is pressed
    }
    else
    {
        return RELEASE;              // button is released
    }
}


// Store the actual state of a key in FIFO
static void key_in_fifo(keypad_status_t keypad_status)
{
    keypad_fifo.fifo_buffer[keypad_fifo.write] = keypad_status;
    if (++keypad_fifo.write >= KEY_FIFO_SIZE)
    {
        keypad_fifo.write = 0;
    }
}

/*Read a keystroke event from FIFO */
keypad_status_t key_out_fifo(void)
{
    keypad_status_t key_event;
    if (keypad_fifo.read == keypad_fifo.write)
    {
        return EN_KEY_NONE;
    }
    else
    {
        key_event = keypad_fifo.fifo_buffer[keypad_fifo.read];
        if (++keypad_fifo.read >= KEY_FIFO_SIZE)
        {
            keypad_fifo.read = 0;
        }
        return key_event;
    }
}

/* State machine mode detects keys and analyzes key events */
static void detect_key_state(key_id_t key_id)
{
    keypad_t *p_key;
    p_key = &keypad[key_id];               //The pointer points to the key event structure
    uint8_t current_key_state;             // Current key status
    current_key_state = p_key->get_key_status(key_id);
    switch (p_key->key_state)
    {
    case EN_KEY_NULL:
        // If button is pressed
        if (current_key_state == PRESS)
        {
            p_key->key_state = EN_KEY_DOWN;
        }
        break;
    
    case EN_KEY_DOWN:
        // If the state is still there
        if (current_key_state == p_key->prev_key_state)
        {
            p_key->key_state = EN_KEY_DOWN_RECHECK;
            if(p_key->report_flag & KEY_REPORT_DOWN)   //If a button is defined to press the escalation function
            {
                //Store the button-down event
                key_in_fifo((keypad_status_t)(KEY_STATUS * key_id + 1));
            }
            if (p_key->short_key_down)	 //If the callback function is registered, it executes
            {
                p_key->short_key_down(p_key->skd_arg);
            }
        }
        else
        {
            p_key->key_state = EN_KEY_NULL;
        }
        break;
    
    // Long press, continuous send and button release judgment
    case EN_KEY_DOWN_RECHECK:
        //The button is still being pressed
        if(current_key_state == p_key->prev_key_state)
        {
            if(p_key->long_time > 0)
            {
                if (p_key->long_count < p_key->long_time)
                {
                    if ((p_key->long_count += KEY_TICKS) >= p_key->long_time)
                    {
                        if(p_key->report_flag & KEY_REPORT_LONG)
                        {
                            // Save long press events
                            key_in_fifo((keypad_status_t)(KEY_STATUS * key_id + 3));
                        }
                        if(p_key->long_key_down)   //call back
                        {
                            p_key->long_key_down(p_key->lkd_arg);
                        }
                    }
                }
                else  
                {
                    if(p_key->repeat_speed > 0)
                    {
                        if ((p_key->repeat_count  += KEY_TICKS) >= p_key->repeat_speed)
                        {
                            p_key->repeat_count = 0;
                            //If the definition of continuous delivery escalation
                            if(p_key->report_flag & KEY_REPORT_REPEAT)
                            {
                                key_in_fifo((keypad_status_t)(KEY_STATUS * key_id + 4));
                            }
                            if(p_key->repeat_key_down)
                            {
                                p_key->repeat_key_down(p_key->rkd_arg);
                            }
                        }
                    }
                }    
            }
        }
        else  // The key loosen
        {
            p_key->key_state = EN_KEY_UP;
        }
        break;
    
    case EN_KEY_UP:
        if (current_key_state == p_key->prev_key_state)
        {
            p_key->key_state = EN_KEY_UP_RECHECK;
            p_key->long_count = 0;  //Long press count to zero
            p_key->repeat_count = 0;  //Repeat send count to zero
            if(p_key->report_flag & KEY_REPORT_UP)
            {
                // The key loosen
                key_in_fifo((keypad_status_t)(KEY_STATUS * key_id + 2));
            }
            if (p_key->short_key_up)
            {
                p_key->short_key_up(p_key->sku_arg);
            }
        }
        else
        {
            p_key->key_state = EN_KEY_DOWN_RECHECK;
        }
        break;

    case EN_KEY_UP_RECHECK:
        if (current_key_state == p_key->prev_key_state)
        {
            p_key->key_state = EN_KEY_NULL;
        }
        else 
        {
            p_key->key_state = EN_KEY_UP;
        }
        break;
    default:
        break;
    }
    p_key->prev_key_state = current_key_state;
}

/* Detect keys and analyze their corresponding events */
static void detect_key(key_id_t key_id)
{
    keypad_t *p_key;
    p_key = &keypad[key_id];               //The pointer points to the key event structure
    if (p_key->get_key_status(key_id) == PRESS)  // If the button is pressed
    {
        if (p_key->count < KEY_FILTER_TIME)
        {
            p_key->count = KEY_FILTER_TIME;
        }
        else if (p_key->count < 2 * KEY_FILTER_TIME)
        {
            p_key->count += KEY_TICKS;  // Filtering and buffeting
        }
        else
        {
            if (p_key->state == RELEASE)
            {
                p_key->state = PRESS;
                if (p_key->report_flag & KEY_REPORT_DOWN)
                {
                    // Send the message that the button is pressed, and store the event that the button is pressed
                    key_in_fifo((keypad_status_t)(KEY_STATUS * key_id + 1));
                }
                // The callback function
                if (p_key->short_key_down)
                {
                    p_key->short_key_down(p_key->skd_arg);
                }
            }
            // Long press for testing
            if (p_key->long_time > 0)
            {
                if (p_key->long_count < p_key->long_time)
                {
                    //Send a long press event
                    if ((p_key->long_count += KEY_TICKS) >= p_key->long_time)
                    {
                        if (p_key->report_flag & KEY_REPORT_LONG)
                        {
                            // Long press the key value into FIFO
                            key_in_fifo((keypad_status_t)(KEY_STATUS * key_id + 3));
                        }
                        // Callback function
                        if (p_key->long_key_down)
                        {
                            p_key->long_key_down(p_key->lkd_arg);
                        }
                    }
                }
                else
                {
                    //If a continuous event is defined
                    if (p_key->repeat_speed > 0)
                    {
                        if ((p_key->repeat_count  += KEY_TICKS) >= p_key->repeat_speed)
                        {
                            p_key->repeat_count = 0;
                            //If the definition of continuous delivery escalation
                            if(p_key->report_flag & KEY_REPORT_REPEAT)  
                            {
                                /*After a long press of the key, one key is sent every repeat_speed */
                                key_in_fifo((keypad_status_t)(KEY_STATUS * key_id + 4));
                            }
                            if(p_key->repeat_key_down)
                            {
                                //Execute the serial callback function
                                p_key->repeat_key_down(p_key->rkd_arg);
                            }
                        }
                    }
                }
            }
        }
    }
    else    // The key loosen
    {
        if (p_key->count > KEY_FILTER_TIME)
        {
            p_key->count = KEY_FILTER_TIME;
        }
        else if (p_key->count !=0)
        {
        
            p_key->count -= KEY_TICKS;
        }
        else
        {
            // Ending of the filter
            if (p_key->state == PRESS)
            {
                p_key->state = RELEASE;

                if (p_key->report_flag & KEY_REPORT_UP)
                {
                    // 
                    key_in_fifo((keypad_status_t)(KEY_STATUS * key_id + 2));
                }
                if (p_key->short_key_up)
                {
                    p_key->short_key_up(p_key->sku_arg);
                }
            }
        }
        p_key->long_count = 0;   
        p_key->repeat_count = 0;    
    }
}

/* The timer callback function scans the Keypad */
static int timer_irq_cb(void * ctx)
{
    scan_keypad();
}

/* Initializes and starts channel 0 of timer 0, interrupts every millisecond*/
static void mTimer_init(void)
{
    timer_init(TIMER_DEVICE_0);
    timer_set_interval(TIMER_DEVICE_0, TIMER_CHANNEL_0, 1e6);
    timer_irq_register(TIMER_DEVICE_0, TIMER_CHANNEL_0, 0, 1, timer_irq_cb, NULL);

    timer_set_enable(TIMER_DEVICE_0, TIMER_CHANNEL_0, 1);
}

/* scan keypad */
void scan_keypad()
{
    for (uint8_t i = 0; i < EN_KEY_ID_MAX; i++)
    {
        #if USE_STATE_MACHINE
            detect_key_state((key_id_t)i);
        #else
            detect_key((key_id_t)i);
        #endif
    }
}

/* Gets the state of the Keypad, which defaults to 0 if there are no events*/
keypad_status_t get_keypad_state(void)
{
    return key_out_fifo();
}

/* Initialize the keypad*/
void keypad_init(void)
{
    
    gpiohs_set_drive_mode(KEYPAD_LEFT_GPIONUM, GPIO_DM_INPUT_PULL_UP);
    gpiohs_set_drive_mode(KEYPAD_MIDDLE_GPIONUM, GPIO_DM_INPUT_PULL_UP);
    gpiohs_set_drive_mode(KEYPAD_RIGHT_GPIONUM, GPIO_DM_INPUT_PULL_UP);

    

    keypad_fifo.read = 0;
    keypad_fifo.write = 0;

    for (int i = 0; i < EN_KEY_ID_MAX; i++)
    {
        keypad[i].long_time = KEY_LONG_TIME;			/* Long press time 0 means no long press event is detected */
        keypad[i].count = KEY_FILTER_TIME ;		        /* The counter is set to filter time */
        keypad[i].state = RELEASE;						/* By default, 0 is not pressed*/
        keypad[i].repeat_speed = KEY_REPEAT_TIME;		/* Button continuous delivery speed , 0 means that it is not supported */
        keypad[i].repeat_count = 0;						/* Continuous counter */

        keypad[i].short_key_down = NULL;                /* Press the button to press the callback function*/
        keypad[i].skd_arg = NULL;                       /* Press the button to call back function parameters*/
        keypad[i].short_key_up = NULL;                  /* Key lift callback function*/
        keypad[i].sku_arg = NULL;                       /* Button lift callback function parameters*/
        keypad[i].long_key_down = NULL;                 /* Key long press callback function*/
        keypad[i].lkd_arg = NULL;                       /* Key long press callback function parameters*/
		keypad[i].repeat_key_down = NULL;
		keypad[i].rkd_arg = NULL;
        keypad[i].get_key_status = get_keys_state_hw;

        keypad[i].report_flag = KEY_REPORT_DOWN | KEY_REPORT_UP | KEY_REPORT_LONG | KEY_REPORT_REPEAT ;
    }

    mTimer_init();
}

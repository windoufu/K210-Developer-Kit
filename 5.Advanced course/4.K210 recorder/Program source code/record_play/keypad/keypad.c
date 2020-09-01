#include "keypad.h"
#include "gpiohs.h"
#include "timer.h"
#include "pin_config.h"

/* FIFO buffer and three-way scroll button structure variable */
keypad_fifo_t keypad_fifo;
keypad_t keypad[EN_KEY_ID_MAX];

static int timer_irq_cb(void * ctx)
{
    key_scan();
}

static void mTimer_init(void)
{
    timer_init(TIMER_DEVICE_0);
    timer_set_interval(TIMER_DEVICE_0, TIMER_CHANNEL_0, 1e6);
    timer_irq_register(TIMER_DEVICE_0, TIMER_CHANNEL_0, 0, 1, timer_irq_cb, NULL);

    timer_set_enable(TIMER_DEVICE_0, TIMER_CHANNEL_0, 1);
}

// Read the value of pin level corresponding to the key, press to return low, release to return high.
static gpio_pin_value_t get_key_value(key_id_t key_id)
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

// Read the key status, PRESS return PRESS, RELEASE return RELEASE
static uint8_t get_keys_state_hw(key_id_t key_id)
{
    gpio_pin_value_t key_value = get_key_value(key_id);
    
    if (key_value == GPIO_PV_LOW)
    {
        return PRESS;               
    }
    else
    {
        return RELEASE;              
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

// Read a keystroke event from FIFO
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

// State machine mode detects keys and analyzes key events
static void detect_key_state(key_id_t key_id)
{
    keypad_t *p_key;
    p_key = &keypad[key_id];              
    uint8_t current_key_state;            
    current_key_state = p_key->get_key_status(key_id);
    switch (p_key->key_state)
    {
    case EN_KEY_NULL:
        
        if (current_key_state == PRESS)
        {
            p_key->key_state = EN_KEY_DOWN;
        }
        break;
    
    case EN_KEY_DOWN:
        
        if (current_key_state == p_key->prev_key_state)
        {
            p_key->key_state = EN_KEY_DOWN_RECHECK;
            if(p_key->report_flag & KEY_REPORT_DOWN)   
            {
                
                key_in_fifo((keypad_status_t)(KEY_STATUS * key_id + 1));
            }
            if (p_key->short_key_down)	 
            {
                p_key->short_key_down(p_key->skd_arg);
            }
        }
        else
        {
            p_key->key_state = EN_KEY_NULL;
        }
        break;
    
   
    case EN_KEY_DOWN_RECHECK:
    
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
                            
                            key_in_fifo((keypad_status_t)(KEY_STATUS * key_id + 3));
                        }
                        if(p_key->long_key_down)   
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
        else  
        {
            p_key->key_state = EN_KEY_UP;
        }
        break;
    
    case EN_KEY_UP:
        if (current_key_state == p_key->prev_key_state)
        {
            p_key->key_state = EN_KEY_UP_RECHECK;
            p_key->long_count = 0;  
            p_key->repeat_count = 0;  
            if(p_key->report_flag & KEY_REPORT_UP)
            {
                
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


static void detect_key(key_id_t key_id)
{
    keypad_t *p_key;
    p_key = &keypad[key_id];                     
    if (p_key->get_key_status(key_id) == PRESS) 
    {
        if (p_key->count < KEY_FILTER_TIME)
        {
            p_key->count = KEY_FILTER_TIME;
        }
        else if (p_key->count < 2 * KEY_FILTER_TIME)
        {
            p_key->count += KEY_TICKS;
        }
        else
        {
            if (p_key->state == RELEASE)
            {
                p_key->state = PRESS;
                if (p_key->report_flag & KEY_REPORT_DOWN)
                {
                    
                    key_in_fifo((keypad_status_t)(KEY_STATUS * key_id + 1));
                }
                
                if (p_key->short_key_down)
                {
                    p_key->short_key_down(p_key->skd_arg);
                }
            }
            
            if (p_key->long_time > 0)
            {
                if (p_key->long_count < p_key->long_time)
                {
                    
                    if ((p_key->long_count += KEY_TICKS) >= p_key->long_time)
                    {
                        if (p_key->report_flag & KEY_REPORT_LONG)
                        {
                            
                            key_in_fifo((keypad_status_t)(KEY_STATUS * key_id + 3));
                        }
                        
                        if (p_key->long_key_down)
                        {
                            p_key->long_key_down(p_key->lkd_arg);
                        }
                    }
                }
                else
                {
                    
                    if (p_key->repeat_speed > 0)
                    {
                        if ((p_key->repeat_count  += KEY_TICKS) >= p_key->repeat_speed)
                        {
                            p_key->repeat_count = 0;
                            
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
    }
    else    
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
            
            if (p_key->state == PRESS)
            {
                p_key->state = RELEASE;

                if (p_key->report_flag & KEY_REPORT_UP)
                {
                    
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



void key_scan()
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

void keypad_init(void)
{
    gpiohs_set_drive_mode(KEYPAD_LEFT_GPIONUM, GPIO_DM_INPUT_PULL_UP);
    gpiohs_set_drive_mode(KEYPAD_MIDDLE_GPIONUM, GPIO_DM_INPUT_PULL_UP);
    gpiohs_set_drive_mode(KEYPAD_RIGHT_GPIONUM, GPIO_DM_INPUT_PULL_UP);

    
    keypad_fifo.read = 0;
    keypad_fifo.write = 0;

    for (int i = 0; i < EN_KEY_ID_MAX; i++)
    {
        keypad[i].long_time = KEY_LONG_TIME;			
        keypad[i].count = KEY_FILTER_TIME ;		        
        keypad[i].state = RELEASE;						
        keypad[i].repeat_speed = KEY_REPEAT_TIME;		
        keypad[i].repeat_count = 0;						

        keypad[i].short_key_down = NULL;               
        keypad[i].skd_arg = NULL;                       
        keypad[i].short_key_up = NULL;                 
        keypad[i].sku_arg = NULL;                       
        keypad[i].long_key_down = NULL;                
        keypad[i].lkd_arg = NULL;                       
		keypad[i].repeat_key_down = NULL;
		keypad[i].rkd_arg = NULL;
        keypad[i].get_key_status = get_keys_state_hw;
        
        keypad[i].report_flag = KEY_REPORT_DOWN | KEY_REPORT_UP | KEY_REPORT_LONG | KEY_REPORT_REPEAT ;
    }
    mTimer_init();
}

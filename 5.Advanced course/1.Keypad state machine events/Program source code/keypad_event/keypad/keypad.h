#ifndef _KEYPAD_H_
#define _KEYPAD_H_

#include "stdio.h"

#define KEY_FIFO_SIZE	    50             // Key FIFO buffer size
#define RELEASE             0              // Release button
#define PRESS               1              // Press button
#define KEY_STATUS          4              // Button events: release, short press, long press, repeat press

/* Modify key trigger time */
#define KEY_TICKS           1              // Key scan period (MS),scan_keypad() function in which fixed scan period this value is equal to what value in principle should be in the common difier of 10 because key shake is 10ms
#define KEY_FILTER_TIME     10             // Key debounce time
#define KEY_LONG_TIME       1000           // Long press trigger time (ms)
#define KEY_REPEAT_TIME     200            // Burst interval (ms), the number of bursts (/s)=1000/KEY_REPEAT_TIME

#define USE_STATE_MACHINE   1              // State machine detection method

/* Report event flag */
#define KEY_REPORT_DOWN     (1<<0)         // Report key press event
#define KEY_REPORT_UP       (1<<1)         // Report key release event
#define KEY_REPORT_LONG     (1<<2)         // Report key long press event
#define KEY_REPORT_REPEAT   (1<<3)         // Report key repeat press event

/* Define key events*/
typedef enum _keypad_status_t
{
    EN_KEY_NONE = 0,   // No key event
    
    //key left press event
    EN_KEY_LEFT_DOWN,
    EN_KEY_LEFT_UP,
    EN_KEY_LEFT_LONG,
    EN_KEY_LEFT_REPEAT,

    //key right press event
    EN_KEY_RIGHT_DOWN,
    EN_KEY_RIGHT_UP,
    EN_KEY_RIGHT_LONG,
    EN_KEY_RIGHT_REPEAT,

    // Intermediate key event
    EN_KEY_MIDDLE_DOWN,
    EN_KEY_MIDDLE_UP,
    EN_KEY_MIDDLE_LONG,
    EN_KEY_MIDDLE_REPEAT,

}keypad_status_t;

/* Key state machine*/
typedef enum _key_state_t
{
    EN_KEY_NULL = 0,    // No button is pressed
    EN_KEY_DOWN,
    EN_KEY_DOWN_RECHECK,
    EN_KEY_UP,
    EN_KEY_UP_RECHECK,
    EN_LONG,
    EN_REPEAT
}key_state_t;

/* BUTTON ID */
typedef enum _key_id_t
{
    EN_KEY_ID_LEFT = 0,
    EN_KEY_ID_RIGHT ,
    EN_KEY_ID_MIDDLE ,
    EN_KEY_ID_MAX 
}key_id_t;

// Key structure
typedef struct _keypad_t
{
    /* function Pointers*/
    uint8_t (*get_key_status)(key_id_t key_id); //The function to determine whether the button is pressed, 1 means pressed
    void (*short_key_down)(void * skd_arg);     //short press call back function
    void * skd_arg;                             //The arguments passed in by the callback function when short press button
    void(*short_key_up)(void * sku_arg);        //The callback function when the button is short pressed and released
    void * sku_arg;                             //The arguments passed in by the callback function when short press button
    void (*long_key_down)(void *lkd_arg);       //Long press event callback function
    void *lkd_arg;                              //Long press event callback function
    void (*repeat_key_down)(void *rkd_arg);     //Press continuouly callback event 
    void *rkd_arg;                              //Press continuouly event callback function
    
    uint8_t  count;			    /* Filter and counter */
    uint16_t long_count;		/* Long press counter */
    uint16_t long_time;		    /* Button press duration, 0 means not to detect long press */
    uint8_t  state;			    /* Currently state of the button (press down or release) */
    uint8_t  repeat_speed;	    /* Continuous press cycle*/
    uint8_t  repeat_count;	    /* Continuous key counter */

    uint8_t report_flag;        /* Reported event mark*/
    
    key_state_t key_state ;      /* Key state machine*/
    uint8_t prev_key_state;      /* The last status of the  key */
}keypad_t;
extern keypad_t keypad[EN_KEY_ID_MAX];

/* FIFO The structure of the body*/
typedef struct _keypad_fifo_t
{
    keypad_status_t fifo_buffer[KEY_FIFO_SIZE];    /* Key value buffer */
    uint8_t read;					               /* Buffer read pointer 1 */
    uint8_t write;
} keypad_fifo_t;


void keypad_init(void);
void scan_keypad(void);
keypad_status_t get_keypad_state(void);

#endif  /* _KEYPAD_H_ */

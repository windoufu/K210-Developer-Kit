#ifndef _KEYPAD_H_
#define _KEYPAD_H_

#include "stdio.h"
#include "stdint.h"

#define KEY_FIFO_SIZE	  50             
#define KEY_TICKS         5             
#define RELEASE           0              
#define PRESS             1              
#define KEY_STATUS        4              

#define KEY_FILTER_TIME   10             
#define KEY_LONG_TIME     1500           
#define KEY_REPEAT_TIME   200            

#define USE_STATE_MACHINE 1

#define KEY_REPORT_DOWN     (1<<0)       
#define KEY_REPORT_UP       (1<<1)      
#define KEY_REPORT_LONG     (1<<2)       
#define KEY_REPORT_REPEAT   (1<<3)       

typedef enum _keypad_status_t
{
    
    EN_KEY_NONE = 0,
    
    
    EN_KEY_LEFT_DOWN,
    EN_KEY_LEFT_UP,
    EN_KEY_LEFT_LONG,
    EN_KEY_LEFT_REPEAT,

    
    EN_KEY_RIGHT_DOWN,
    EN_KEY_RIGHT_UP,
    EN_KEY_RIGHT_LONG,
    EN_KEY_RIGHT_REPEAT,

    
    EN_KEY_MIDDLE_DOWN,
    EN_KEY_MIDDLE_UP,
    EN_KEY_MIDDLE_LONG,
    EN_KEY_MIDDLE_REPEAT,

} keypad_status_t;


typedef enum _key_state_t
{
    
    EN_KEY_NULL = 0,
    EN_KEY_DOWN,
    EN_KEY_DOWN_RECHECK,
    EN_KEY_UP,
    EN_KEY_UP_RECHECK,
    EN_LONG,
    EN_REPEAT
} key_state_t;

typedef enum _key_id_t
{
    EN_KEY_ID_LEFT = 0,
    EN_KEY_ID_RIGHT ,
    EN_KEY_ID_MIDDLE ,
    EN_KEY_ID_MAX 
} key_id_t;


typedef struct _keypad_t
{
    
    uint8_t (*get_key_status)(key_id_t key_id); 
    void (*short_key_down)(void * skd_arg);     
    void * skd_arg;                             
    void(*short_key_up)(void * sku_arg);        
    void * sku_arg;                             
    void (*long_key_down)(void *lkd_arg);       
    void *lkd_arg;                              
    void (*repeat_key_down)(void *rkd_arg);     
    void *rkd_arg;                              
    
    uint8_t  count;			   
    uint16_t long_count;		
    uint16_t long_time;		    
    uint8_t  state;			    
    uint8_t  repeat_speed;	    
    uint8_t  repeat_count;	    

    uint8_t report_flag;        
    
    key_state_t key_state ;      
    uint8_t prev_key_state;     
} keypad_t;
extern keypad_t keypad[EN_KEY_ID_MAX];


typedef struct _keypad_fifo_t
{
    keypad_status_t fifo_buffer[KEY_FIFO_SIZE];    
    uint8_t read;					              
    uint8_t write;
} keypad_fifo_t;


void keypad_init(void);
void key_scan(void);
keypad_status_t key_out_fifo(); 

#endif  /* _KEYPAD_H_ */

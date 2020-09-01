#ifndef _FT6236U_H_
#define _FT6236U_H_

#include <stdint.h>

#define FT6236_I2C_ADDR         0x38

#define FT6236_IRQ_LEVEL        1

#define TP_PRES_DOWN            0x80  //The touch screen is pressed	
#define TP_COORD_UD             0x40  //Touch the coordinate update flag

/* FT6236 部分寄存器定义 */
#define FT_DEVIDE_MODE 			0x00   		//FT6236 mode control register
#define FT_REG_NUM_FINGER       0x02		//Touch status register

#define FT_TP1_REG 				0X03	  	//The first touch point data address
#define FT_TP2_REG 				0X09		//The second touch point data address

#define FT_ID_G_MODE 			0xA4   		//FT6236 Interrupt mode control register
#define FT_ID_G_THGROUP			0x80   		//Touch the valid values to set the register
#define FT_ID_G_PERIODACTIVE	0x88   		//Activate the state cycle to set registers


typedef enum _rst_level{
    LEVEL_LOW = 0,
    LEVEL_HIGH = 1
} rst_level_t;

/* ft6236 Touch structure */
typedef struct
{
    //Touch status b7: Press 1/ release 0;bB6 :0 No button press /1 button press;
    // bit5-bit1:Reservation; The bit0 touch point presses the valid flag, which is valid as 1
    uint8_t touch_state;
    uint16_t touch_x;
    uint16_t touch_y;
} ft6236_touch_point_t;

extern ft6236_touch_point_t ft6236;


void ft6236_init(void);
void ft6236_scan(void);
void ft6236_reset_pin(rst_level_t level);
void ft6236_hardware_init(void);


#endif /* _FT6236U_H_ */

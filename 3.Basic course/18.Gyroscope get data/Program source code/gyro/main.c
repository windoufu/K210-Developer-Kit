/**
* @par  Copyright (C): 2016-2022, Shenzhen Yahboom Tech
* @file         main.c
* @author       Gengyue
* @version      V1.0
* @date         2020.05.27
* @brief        Gyroscope get data
* @details      
* @par History  Description below
*                 
* version:	V1.0: Print out the gyro's XYZ axis data through a serial port.
*/
#include "sleep.h"
#include "gpiohs.h"
#include "sysctl.h"
#include "i2c_ctl.h"
#include "icm20607.h"
#include "pin_config.h"

#define GYRO_DATA     1
#define ACC_DATA      0

/**
* Function       hardware_init
* @author        Gengyue
* @date          2020.05.27
* @brief         Hardware initialization, bind GPIO port
* @param[in]     void
* @param[out]    void
* @retval        void
* @par History   no
*/
void hardware_init(void)
{
    /* I2C ICM20607 */
    fpioa_set_function(PIN_ICM_SCL, FUNC_ICM_SCL);
    fpioa_set_function(PIN_ICM_SDA, FUNC_ICM_SDA);
}

/**
* Function       main
* @author        Gengyue
* @date          2020.05.27
* @brief         Main function, the entry point of the program
* @param[in]     void
* @param[out]    void
* @retval        0
* @par History   no
*/
int main(void)
{
    // Hardware pin initialization
    hardware_init();
    /* Scan and print the device address of the current I2C bus  */
    printf("I2C SCAN START! \n");
    i2c_read_addr();
    printf("I2C SCAN END! \n");
    msleep(500);

    /* Initialize ICM20607 */
    icm20607_init();
    printf("icm20607 init ok!\n");

    #if ACC_DATA
    int16_t val_ax = 0;
    int16_t val_ay = 0;
    int16_t val_az = 0;
    #else
    int16_t val_gx = 0;
    int16_t val_gy = 0;
    int16_t val_gz = 0;
    #endif

    while (1)
    {
        #if GYRO_DATA
        val_gx = getRawGyroscopeX();
        val_gy = getRawGyroscopeY();
        val_gz = getRawGyroscopeZ();
        printf("gx=%d, gy=%d, gz=%d\n", val_gx, val_gy, val_gz);
        val_gx = val_gy = val_gz = 0;
        #elif ACC_DATA
        val_ax = getRawAccelerationX();
        val_ay = getRawAccelerationY();
        val_az = getRawAccelerationZ();
        printf("ax=%d, ay=%d, az=%d\n", val_ax, val_ay, val_az);
        val_ax = val_ay = val_az = 0;
        #else
        printf("Please set the GYRO_DATA or ACC_DATA to 1\n");
        #endif
        msleep(5);
    }

    return 0;
}

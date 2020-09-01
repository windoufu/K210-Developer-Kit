#include "icm20607.h"
#include "i2c_ctl.h"
#include "sleep.h"

uint16_t gyro_scale = 0;
uint16_t acc_scale = 0;


/* I2C write data */
static void icm_i2c_write(uint8_t reg, uint8_t data)
{
    i2c_hd_write(ICM_ADDRESS, reg, data);
}

/* I2C read data */
static void icm_i2c_read(uint8_t reg, uint8_t *data_buf, uint16_t length)
{
    i2c_hd_read(ICM_ADDRESS, reg, data_buf, length);
}

/* Read the original X-axis data of the gyroscope*/
int16_t getRawGyroscopeX(void) {
    uint8_t val[2] = {0};
    icm_i2c_read(GYRO_XOUT_H, val, 2);
    return ((int16_t)val[0] << 8) + val[1];
}

/* Read the original Y-axis data of the gyroscope */
int16_t getRawGyroscopeY(void) {
    uint8_t val[2] = {0};
    icm_i2c_read(GYRO_YOUT_H, val, 2);
    return ((int16_t)val[0] << 8) + val[1];
}

/* Read the original Z-axis data of the gyroscope */
int16_t getRawGyroscopeZ(void) {
    uint8_t val[2] = {0};
    icm_i2c_read(GYRO_ZOUT_H, val, 2);
    return ((int16_t)val[0] << 8) + val[1];
}

/* Read the original accelerometer X-axis data */ 
int16_t getRawAccelerationX(void) {
    uint8_t val[2] = {0};
    icm_i2c_read(ACCEL_XOUT_H, val, 2);
    return ((int16_t)val[0] << 8) + val[1];
}

/* Read the original accelerometer Y-axis data */
int16_t getRawAccelerationY(void) {
    uint8_t val[2] = {0};
    icm_i2c_read(ACCEL_YOUT_H, val, 2);
    return ((int16_t)val[0] << 8) + val[1];
}

/* Read the original accelerometer Z-axis data */
int16_t getRawAccelerationZ(void) {
    uint8_t val[2] = {0};
    icm_i2c_read(ACCEL_ZOUT_H, val, 2);
    return ((int16_t)val[0] << 8) + val[1];
}

/* Determine if it is AN ICM20607 chip */
void icm20607_who_am_i(void)
{
    uint8_t val, state = 1;
    do
    {
        icm_i2c_read(WHO_AM_I, &val, 1);  // Read ICM20607 ID

        if (ICM20607_ID != val & state)  // If the ID is incorrect, it is only reported once.
        {
            printf("WHO_AM_I=0x%02x\n", val);
            state = 0;
        }
    } while(ICM20607_ID != val);
    printf("WHO_AM_I=0x%02x\n", val);
}

/* Initialize icm20607 chip */
void icm20607_init(void)
{
    uint8_t val = 0x0, res = 1;
    i2c_hardware_init(ICM_ADDRESS); // Initialization
    msleep(10);

    icm_i2c_write(PWR_MGMT_1, 0x80); //Reset device
    msleep(100);
    icm20607_who_am_i();

    do
    { //Wait reset successful
        icm_i2c_read(PWR_MGMT_1, &val, 1);
    } while(0x41 != val);

    icm_i2c_write(PWR_MGMT_1, 0x01);     //clock setting
    icm_i2c_write(PWR_MGMT_2, 0x00);     //Turn on the gyroscope and accelerometer
    icm_i2c_write(CONFIG, 0x01);         //176HZ 1KHZ
    icm_i2c_write(SMPLRT_DIV, 0x07);     //Sampling rate SAMPLE_RATE = INTERNAL_SAMPLE_RATE / (1 + SMPLRT_DIV)
    icm_i2c_write(GYRO_CONFIG, 0x18);    //±2000 dps
    icm_i2c_write(ACCEL_CONFIG, 0x10);   //±8g
    icm_i2c_write(ACCEL_CONFIG_2, 0x23); //Average 8 samples   44.8HZ
    return res;
}

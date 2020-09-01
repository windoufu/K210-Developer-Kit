#include "bsp.h"
#include "i2c_ctl.h"
#include "icm20607.h"
#include "icm_math.h"
#include "stdint.h"


#define DT              (0.005)             

int16_t icm_gyro_x, icm_gyro_y, icm_gyro_z;
int16_t icm_acc_x, icm_acc_y, icm_acc_z;



/*  I2C write data */
static void icm_i2c_write(uint8_t reg, uint8_t data)
{
    i2c_hd_write(ICM_ADDRESS, reg, data);
}

/*I2C read data*/
static void icm_i2c_read(uint8_t reg, uint8_t *data_buf, uint16_t length)
{
    i2c_hd_read(ICM_ADDRESS, reg, data_buf, length);
}

/* Read the original X-axis data of the gyroscope */
int16_t getRawGyroscopeX(void) {
    uint8_t val[2] = {0};
    icm_i2c_read(GYRO_XOUT_H, val, 2);
    return ((int16_t)val[0] << 8) + val[1];
}

/*Read the original Y-axis data of the gyroscope */
int16_t getRawGyroscopeY(void) {
    uint8_t val[2] = {0};
    icm_i2c_read(GYRO_YOUT_H, val, 2);
    return ((int16_t)val[0] << 8) + val[1];
}

/*  Read the original Z-axis data of the gyroscope*/
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

/*Read the original accelerometer Z-axis data*/
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
        icm_i2c_read(WHO_AM_I, &val, 1);  // Determine if it is AN ICM20607 chip 

        if (ICM20607_ID != val & state)  
        {
            printf("ID error! WHO_AM_I=0x%02x\n", val);
            printf("Please press the reset key to reboot\n");
            state = 0;
        }
    } while(ICM20607_ID != val);
    printf("WHO_AM_I=0x%02x\n", val);
}

/* Initialize the icm20607 chip*/
void icm20607_init(void)
{
    uint8_t val = 0x0;
    i2c_hardware_init(ICM_ADDRESS); 
    msleep(10);

    icm_i2c_write(PWR_MGMT_1, 0x80); 
    msleep(100);
    icm20607_who_am_i();

    do
    { 
        icm_i2c_read(PWR_MGMT_1, &val, 1);
    } while(0x41 != val);

    icm_i2c_write(PWR_MGMT_1, 0x01);     
    icm_i2c_write(PWR_MGMT_2, 0x00);     
    icm_i2c_write(CONFIG, 0x01);         //176HZ 1KHZ
    icm_i2c_write(SMPLRT_DIV, 0x07);     //SAMPLE_RATE = INTERNAL_SAMPLE_RATE / (1 + SMPLRT_DIV)
    icm_i2c_write(GYRO_CONFIG, 0x18);    //±2000 dps
    icm_i2c_write(ACCEL_CONFIG, 0x10);   //±8g
    icm_i2c_write(ACCEL_CONFIG_2, 0x23); //Average 8 samples   44.8HZ
}

/* Read the raw data of the gyroscope */
void icm_get_gyro(void)
{
    uint8_t dat[6];

    icm_i2c_read(GYRO_XOUT_H, dat, 6);
    icm_gyro_x = (int16_t)(((uint16_t)dat[0] << 8 | dat[1]));
    icm_gyro_y = (int16_t)(((uint16_t)dat[2] << 8 | dat[3]));
    icm_gyro_z = (int16_t)(((uint16_t)dat[4] << 8 | dat[5]));
}

/* Read the raw data of the accelerometer */
void icm_get_acc(void)
{
    uint8_t dat[6];

    icm_i2c_read(ACCEL_XOUT_H, dat, 6);
    icm_acc_x = (int16_t)(((uint16_t)dat[0] << 8 | dat[1]));
    icm_acc_y = (int16_t)(((uint16_t)dat[2] << 8 | dat[3]));
    icm_acc_z = (int16_t)(((uint16_t)dat[4] << 8 | dat[5]));
}

/* Read ICM attitude angle */
void get_icm_attitude(void)
{
    icm_get_gyro();
    icm_get_acc();

    g_icm20607.accX = icm_acc_x;
    g_icm20607.accY = icm_acc_y;
    g_icm20607.accZ = icm_acc_z;
    g_icm20607.gyroX = icm_gyro_x;
    g_icm20607.gyroY = icm_gyro_y;
    g_icm20607.gyroZ = icm_gyro_z;

    get_attitude_angle(&g_icm20607, &g_attitude, DT); // Four-element algorithm
}

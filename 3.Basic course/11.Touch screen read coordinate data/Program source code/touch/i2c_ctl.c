
#include "i2c_ctl.h"
#include "i2c.h"
#include "stdio.h"

static uint16_t _current_addr = 0x00;

/* Hardware initialization I2C, set slave address, data bit width, I2C communication rate*/
void i2c_hardware_init(uint16_t addr)
{
    i2c_init(I2C_DEVICE_0, addr, ADDRESS_WIDTH, I2C_CLK_SPEED);
    _current_addr = addr;
}

/* Write data to register reg, return 0 on write, return non-0 on failure*/
uint16_t i2c_hd_write(uint8_t addr, uint8_t reg, uint8_t data)
{
    if (_current_addr != addr)
    {
        i2c_hardware_init(addr);
    }
    uint8_t cmd[2];
    cmd[0] = reg;
    cmd[1] = data;
    uint16_t error = 1;
    error = i2c_send_data(I2C_DEVICE_0, cmd, 2);
    return error;
}

/* Read length data from register reg and save it to data_buf, return 0 if read successfully, non-zero if failed */
uint16_t i2c_hd_read(uint8_t addr, uint8_t reg, uint8_t *data_buf, uint16_t length)
{
    if (_current_addr != addr)
    {
        i2c_hardware_init(addr);
    }
    uint16_t error = 1;
    error = i2c_recv_data(I2C_DEVICE_0, &reg, 1, data_buf, length);
    return error;
}

/* Print the current I2C device address on the I2C bus*/
void i2c_read_addr(void)
{
    uint16_t error = 1;

    uint8_t cmd[2];
    cmd[0] = 0x00;
    cmd[1] = 0x01;
    
    for (int i = 0; i < 255; i++)
    {
        i2c_init(I2C_DEVICE_0, i, ADDRESS_WIDTH, I2C_CLK_SPEED);
        error = i2c_send_data(I2C_DEVICE_0, cmd, 2);
        if (error == 0)
        {
            // printf("I2C DEVICE FOUND:");
            printf("0x");
            printf("%x", i);
        }
        else{
            printf(".");
        }
    }
    printf(" \n");
}

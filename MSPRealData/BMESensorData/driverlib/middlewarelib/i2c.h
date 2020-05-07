//
// 02/11/2019, V 1.1
// Paolo Santinelli
//
#ifndef _I2C_H // is myheader.h already included?
#define _I2C_H // define this so we know it's included

typedef struct
{
    uint16_t baseAddress;
    uint16_t slaveAddress;
} i2c_device;

typedef struct
{
    uint16_t tx_len;
    void * tx_buf;
    uint16_t rx_len;
    void * rx_buf;

} i2c_transaction;

#include "driverlib.h"
void initI2c(uint8_t slaveAddress);
int i2c_transfer(const i2c_device *dev, i2c_transaction *data);
#endif

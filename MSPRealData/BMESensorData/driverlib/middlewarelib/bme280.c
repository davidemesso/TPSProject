//
// 01/12/2019, V 1.2
// Paolo Santinelli
//
#include "driverlib.h"
#include "bme280.h"
#include "i2c.h"

//******************************************************************************
//
// getBme280IdValue()
//
// Read the chip identification number
//
//******************************************************************************
//------------------------------------------------------------------------------
void getBme280IdValue(uint8_t * id)
{
        //Send single byte data.
        //USCI_B_I2C_masterSendSingleByte(USCI_B0_BASE, transmitData); // 1
        
        //Set in transmit mode
        USCI_B_I2C_setMode(USCI_B0_BASE, USCI_B_I2C_TRANSMIT_MODE); // 2
        USCI_B_I2C_masterSendStart (USCI_B0_BASE); // 2
        USCI_B_I2C_masterSendMultiByteNext (USCI_B0_BASE, ID); // 2
        USCI_B_I2C_masterSendMultiByteStop (USCI_B0_BASE); // 2 
        //Delay until transmission completes
        while (USCI_B_I2C_isBusBusy(USCI_B0_BASE)) ; // 1
        __delay_cycles(50);
        //Set Master in receive mode
        USCI_B_I2C_setMode(USCI_B0_BASE, USCI_B_I2C_RECEIVE_MODE);
        USCI_B_I2C_masterSendStart(USCI_B0_BASE);
        *id = USCI_B_I2C_masterReceiveSingle(USCI_B0_BASE);
        USCI_B_I2C_masterSendMultiByteStop(USCI_B0_BASE);
}
//------------------------------------------------------------------------------
//******************************************************************************
//
// configBme280 I2C routine.
//
// ctrl_hum:
//  addr = 0xF2
//  value= 0x01
//  osrs_h[2:0] = 001 --> oversampling x 1
//
//  | b7  |  b6  |  b5  |  b4  |  b3  |  b2    b1    b0 |
//                                    |   osrs_h[2:0]   |
//
// ctrl_meas:
//  addr = 0xF4
//  value= 0x27
//  osrs_t[2:0] = 001 --> oversampling x 1
//  osrs_p[2:0] = 001 --> oversampling x 1
//  mode  [1:0] =  11 --> normal mode
//
//  | b7  |  b6  |  b5  |  b4  |  b3  |  b2  |  b1    b0 |
//  |    osrs_t[2:0]    |     osrs_p[2:0]    | mode[1:0] |
//
//******************************************************************************

int configBme280(void)
{

        uint8_t dataValues[6], retVal;
        i2c_transaction i2cTrans;
        i2c_device i2cDev;

        i2cDev.baseAddress  = USCI_B0_BASE;
        i2cDev.slaveAddress = SLAVE_ADDRESS;

        //Set CONFIG = 0xA0, sampling rate 1 sec
        dataValues[0] = CONFIG;
        dataValues[1] = 0xA0;

        //Set CTRL_HUM = 0x01, humidity control register
        dataValues[2] = CTRL_HUM;
        dataValues[3] = 0x01;// 0x00

        //Set CTRL_MEAS = 0x27, pressure and temperature data acquisition options
        dataValues[4] = CTRL_MEAS;
        dataValues[5] = 0x27; // 0x03

        i2cTrans.tx_buf = dataValues;
        i2cTrans.tx_len = 6;
        i2cTrans.rx_buf = (void*) 0;
        i2cTrans.rx_len = 0;


        retVal = i2c_transfer(&i2cDev, &i2cTrans);

        return retVal;

}
//------------------------------------------------------------------------------
//******************************************************************************
//
// int getI2cValues(uint8_t slaveAddr,uint8_t firstRegAddr,uint8_t dataDim,uint8_t*values)
//
// Read dataDim bytes from the i2c slave device starting from the register of
// address firstRegAddr.
//
//******************************************************************************
//-------------------------------------------------------------------------------
int getI2cValues(uint8_t slaveAddr, uint8_t firstRegAddr, uint8_t dataDim,  uint8_t*values )
{
    uint8_t startRegAddr = firstRegAddr, retVal;
    i2c_transaction i2cTrans;
    i2c_device i2cDev;

    i2cDev.baseAddress  = USCI_B0_BASE;
    i2cDev.slaveAddress = slaveAddr;

    i2cTrans.tx_buf = &startRegAddr;
    i2cTrans.tx_len = 1;
    i2cTrans.rx_buf = values;
    i2cTrans.rx_len = dataDim;

    retVal = i2c_transfer(&i2cDev, &i2cTrans);

    return retVal;
}
//------------------------------------------------------------------------------
//******************************************************************************
//
// int getCalibrationParameter
//
// This function reads the bme280 calibration parameter from the salve and 
// populates the calibrPar.
//
//******************************************************************************
//------------------------------------------------------------------------------
int getBme280CalibrationParameters(uint8_t slaveAddr, calibrPar *par)
{
    
    uint8_t digTPH[33]; 
    getI2cValues(slaveAddr, ADDR_CALIB_00, 26, digTPH);
    getI2cValues(slaveAddr, ADDR_CALIB_13, 7, (digTPH+26*sizeof(uint8_t)));
    
    par->dig_T1 = ((uint16_t) (digTPH[1]))  <<8 | ((uint16_t)  (digTPH[0]));
    par->dig_T2 = ((int16_t)  (digTPH[3]))  <<8 | ((uint16_t)  (digTPH[2]));
    par->dig_T3 = ((int16_t)  (digTPH[5]))  <<8 | ((uint16_t)  (digTPH[4]));
    par->dig_P1 = ((uint16_t) (digTPH[7]))  <<8 | ((uint16_t)  (digTPH[6]));
    par->dig_P2 = ((int16_t)  (digTPH[9]))  <<8 | ((uint16_t)  (digTPH[8]));
    par->dig_P3 = ((int16_t)  (digTPH[11])) <<8 | ((uint16_t)  (digTPH[10]));
    par->dig_P4 = ((int16_t)  (digTPH[13])) <<8 | ((uint16_t)  (digTPH[12]));
    par->dig_P5 = ((int16_t)  (digTPH[15])) <<8 | ((uint16_t)  (digTPH[14]));
    par->dig_P6 = ((int16_t)  (digTPH[17])) <<8 | ((uint16_t)  (digTPH[16]));
    par->dig_P7 = ((int16_t)  (digTPH[19])) <<8 | ((uint16_t)  (digTPH[18]));
    par->dig_P8 = ((int16_t)  (digTPH[21])) <<8 | ((uint16_t)  (digTPH[20]));
    par->dig_P9 = ((int16_t)  (digTPH[23])) <<8 | ((uint16_t)  (digTPH[22]));
    par->dig_H1 = ((uint8_t)  (digTPH[25]));
    par->dig_H2 = ((int16_t)  (digTPH[27])) <<8 | ((uint16_t)  (digTPH[26]));
    par->dig_H3 = ((uint8_t)  (digTPH[28]));
    par->dig_H4 = ((int16_t)  (digTPH[29])) <<4 | ((uint16_t)  (0x0f & digTPH[30]));
    par->dig_H5 = ((int16_t)  (digTPH[31])) <<4 | ((uint16_t)  (digTPH[30]>>4));
    par->dig_H6 = ((int8_t)   (digTPH[32]));
    return 0;
}
//------------------------------------------------------------------------------
//******************************************************************************
//
// int32_t BME280_compensate_T_int32(int32_t adc_T, calibrPar *dig)
//
// Returns temperature in DegC, resolution is 0.01 DegC. Output value of “5123”
// equals 51.23 DegC. t_fine carries fine temperature as global value
//
//******************************************************************************
//------------------------------------------------------------------------------

int32_t BME280_compensate_T_int32(uint32_t adc_T, calibrPar *dig)
{
    int32_t var1;
    int32_t var2;
    int32_t temperature;
    int32_t temperature_min = -4000;
    int32_t temperature_max = 8500;

    var1 = (int32_t)((adc_T / 8)-((int32_t)dig->dig_T1 * 2));
    var1 = (var1 * ((int32_t)dig->dig_T2)) / 2048;
    var2 = (int32_t)((adc_T / 16)-((int32_t)dig->dig_T1));
    var2 = (((var2 * var2) / 4096) * ((int32_t)dig->dig_T3)) / 16384;
    dig->t_fine = var1 + var2;
    temperature = (dig->t_fine * 5 + 128) / 256;
    if (temperature < temperature_min)
    {
        temperature = temperature_min;
    }
    else if (temperature > temperature_max)
    {
        temperature = temperature_max;
    }

    return temperature;
}

//------------------------------------------------------------------------------
//******************************************************************************
//
// uint32_t  BME280_compensate_H_int32 (int32_t adc_H, calibrPar *dig)
//
// Return humidity in %RH as unsigned 32 bit integer in Q22.10 format
// (22 integer and 10 fractional bits).
// Output value of "47455" represents 47445/1024=46.333 %RH
//
//******************************************************************************
//------------------------------------------------------------------------------
uint32_t  BME280_compensate_H_int32 (uint32_t adc_H, calibrPar *dig)
{
    int32_t var1;
    int32_t var2;
    int32_t var3;
    int32_t var4;
    int32_t var5;
    uint32_t humidity;
    uint32_t humidity_max = 102400;

    var1 = dig->t_fine - ((int32_t)76800);
    var2 = (int32_t)(adc_H * 16384);
    var3 = (int32_t)(((int32_t)dig->dig_H4) * 1048576);
    var4 = ((int32_t)dig->dig_H5) * var1;
    var5 = (((var2 - var3) - var4) + (int32_t)16384) / 32768;
    var2 = (var1 * ((int32_t)dig->dig_H6)) / 1024;
    var3 = (var1 * ((int32_t)dig->dig_H3)) / 2048;
    var4 = ((var2 * (var3 + (int32_t)32768)) / 1024) + (int32_t)2097152;
    var2 = ((var4 * ((int32_t)dig->dig_H2)) + 8192) / 16384;
    var3 = var5 * var2;
    var4 = ((var3 / 32768) * (var3 / 32768)) / 128;
    var5 = var3 - ((var4 * ((int32_t)dig->dig_H1)) / 16);
    var5 = (var5 < 0 ? 0 : var5);
    var5 = (var5 > 419430400 ? 419430400 : var5);
    humidity = (uint32_t)(var5 / 4096);
    if (humidity > humidity_max)
    {
        humidity = humidity_max;
    }

    return humidity;
}
//------------------------------------------------------------------------------
//******************************************************************************
//
// uint32_t BME280_compensate_P_int32(int32_t adc_P, calibrPar *dig )
//
// return pressure in Pa as unsigned 32 bit integer in Q24.8 format
// (24 integer bits and 8 fractional bits).
// Output value of "24674867" represents 24674867/256 =96386.2 Pa = 963.862 hPa
//
//******************************************************************************
//------------------------------------------------------------------------------
uint32_t BME280_compensate_P_int32(uint32_t adc_P, calibrPar *dig )

{
    int32_t var1;
    int32_t var2;
    int32_t var3;
    int32_t var4;
    uint32_t var5;
    uint32_t pressure;
    uint32_t pressure_min = 30000;
    uint32_t pressure_max = 110000;

    var1 = (((int32_t)dig->t_fine) / 2) - (int32_t)64000;
    var2 = (((var1 / 4) * (var1 / 4)) / 2048) * ((int32_t)dig->dig_P6);
    var2 = var2 + ((var1 * ((int32_t)dig->dig_P5)) * 2);
    var2 = (var2 / 4) + (((int32_t)dig->dig_P4) * 65536);
    var3 = (dig->dig_P3 * (((var1 / 4) * (var1 / 4)) / 8192)) / 8;
    var4 = (((int32_t)dig->dig_P2) * var1) / 2;
    var1 = (var3 + var4) / 262144;
    var1 = (((32768 + var1)) * ((int32_t)dig->dig_P1)) / 32768;

    // avoid exception caused by division by zero
    if (var1)
    {
        var5 = (uint32_t)((uint32_t)1048576) - adc_P;
        pressure = ((uint32_t)(var5 - (uint32_t)(var2 / 4096))) * 3125;
        if (pressure < 0x80000000)
        {
            pressure = (pressure << 1) / ((uint32_t)var1);
        }
        else
        {
            pressure = (pressure / (uint32_t)var1) * 2;
        }
        var1 = (((int32_t)dig->dig_P9) * ((int32_t)(((pressure / 8) * (pressure / 8)) / 8192))) / 4096;
        var2 = (((int32_t)(pressure / 4)) * ((int32_t)dig->dig_P8)) / 8192;
        pressure = (uint32_t)((int32_t)pressure + ((var1 + var2 + dig->dig_P7) / 16));
        if (pressure < pressure_min)
        {
            pressure = pressure_min;
        }
        else if (pressure > pressure_max)
        {
            pressure = pressure_max;
        }
    }
    else
    {
        pressure = pressure_min;
    }

    return pressure;
}


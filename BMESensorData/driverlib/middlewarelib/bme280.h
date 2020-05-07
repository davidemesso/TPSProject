//
// 01/12/2019, V 1.2
// Paolo Santinelli
//
#ifndef _BME280_H // is myheader.h already included?
#define _BME280_H // define this so we know it's included

#include "driverlib.h"

#define POOLLING_I2C_READ
#define SLAVE_ADDRESS 0x76
// Registers
#define ID  0xD0
#define CTRL_HUM      0xF2  // Sets the humidity data acquisition options
#define CTRL_MEAS     0xF4  // Sets the pressure and temperature data acquisition options
#define CONFIG        0xF5  // Sets the rate, filter and interface options of the device
#define ADDR_DATA_Low 0xF7  // Raw pressure, temperature humidity data
#define ADDR_CALIB_00 0x88  // Calibration data
#define ADDR_CALIB_13 0xE1  // Calibration data

typedef struct              // Data struct to hold bme280 calibration parameters
{
  uint16_t dig_T1;
  int16_t  dig_T2;
  int16_t  dig_T3;
  uint16_t dig_P1;
  int16_t  dig_P2;
  int16_t  dig_P3;
  int16_t  dig_P4;
  int16_t  dig_P5;
  int16_t  dig_P6;
  int16_t  dig_P7;
  int16_t  dig_P8;
  int16_t  dig_P9;
  uint8_t  dig_H1;
  int16_t  dig_H2;
  uint8_t  dig_H3;
  int16_t  dig_H4;
  int16_t  dig_H5;
  int8_t   dig_H6;
  int32_t  t_fine;
} calibrPar;


int configBme280(void);
void getBme280IdValue(uint8_t * id);
int getI2cValues( uint8_t slaveAddr, uint8_t firstRegAddr, uint8_t dataDim,  uint8_t*values );
int getBme280CalibrationParameters(uint8_t slaveAddr, calibrPar *par);
int32_t  BME280_compensate_T_int32(uint32_t adc_T, calibrPar *dig);
uint32_t BME280_compensate_H_int32 (uint32_t adc_H, calibrPar *dig);
uint32_t BME280_compensate_P_int32(uint32_t adc_P,  calibrPar *dig );
#endif

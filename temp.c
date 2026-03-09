#include "temp.h"
#include "stm32f4xx_hal.h"
#include "Driver_I2C.h"

/* I2C Port number */
#ifndef TEMP_I2C_PORT
#define TEMP_I2C_PORT 3
#endif

/* 7-bit I2C address */
#define TEMP_I2C_ADDR 0x48

/* I2C Driver macros */
#define _I2C_Driver_(n) Driver_I2C##n
#define I2C_Driver_(n) _I2C_Driver_(n)

extern ARM_DRIVER_I2C I2C_Driver_(TEMP_I2C_PORT);
#define ptrI2C (&I2C_Driver_(TEMP_I2C_PORT))

int32_t Temp_Read(uint8_t *val) {
    uint8_t reg = 0;
    uint8_t data[1];

    data[0] = reg;

    ptrI2C->MasterTransmit(TEMP_I2C_ADDR, data, 1, true);
    while (ptrI2C->GetStatus().busy);
    if (ptrI2C->GetDataCount() != 1) return -1;

    ptrI2C->MasterReceive(TEMP_I2C_ADDR, val, 1, false);
    while (ptrI2C->GetStatus().busy);
    if (ptrI2C->GetDataCount() != 1) return -1;

    return 0;
}
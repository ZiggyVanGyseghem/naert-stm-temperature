
#include "temp.h"
#include "stm32f4xx_hal.h"
#include "Driver_I2C.h"

/* I2C Port number - The STM32 pins used (PC9 and PA8) map to I2C3 */
#ifndef TEMP_I2C_PORT
#define TEMP_I2C_PORT 3
#endif

/* 7-bit I2C address of the TC74 sensor 
 * (0x48 is the default factory hardware address for the TC74) */
#define TEMP_I2C_ADDR 0x48

/* I2C Driver macros - This dynamically links our code to Keil's underlying I2C3 hardware driver */
#define _I2C_Driver_(n) Driver_I2C##n
#define I2C_Driver_(n) _I2C_Driver_(n)

extern ARM_DRIVER_I2C I2C_Driver_(TEMP_I2C_PORT);
#define ptrI2C (&I2C_Driver_(TEMP_I2C_PORT)) // Create a handy pointer to the I2C interface

/**
 * @brief Reads the current temperature from the TC74 sensor.
 * @param val Pointer to a variable where the temperature will be stored.
 * @return 0 on success, -1 on failure.
 */
int32_t Temp_Read(uint8_t *val) {
    uint8_t reg = 0; // The TC74 internal register address for "Read Temperature" is 0x00
    uint8_t data[1];

    data[0] = reg;

    // --- STEP 1: TELL THE SENSOR WHAT WE WANT TO READ ---
    // MasterTransmit sends the register address (0x00) to the TC74.
    // 'true' means we keep the I2C connection open (no STOP condition) for a Repeated Start.
    ptrI2C->MasterTransmit(TEMP_I2C_ADDR, data, 1, true);
    
    // Polling loop: Wait in a blocking state until the hardware finishes sending the byte
    while (ptrI2C->GetStatus().busy);
    
    // Safety check: Did we actually successfully send 1 byte? If not, return an error (-1).
    if (ptrI2C->GetDataCount() != 1) return -1;

    // --- STEP 2: READ THE ANSWER FROM THE SENSOR ---
    // MasterReceive asks the TC74 to send back the temperature data from the register.
    // 'false' means we send a STOP condition after receiving, closing the I2C connection.
    ptrI2C->MasterReceive(TEMP_I2C_ADDR, val, 1, false);
    
    // Polling loop: Wait until the hardware finishes receiving the byte
    while (ptrI2C->GetStatus().busy);
    
    // Safety check: Did we actually successfully receive 1 byte?
    if (ptrI2C->GetDataCount() != 1) return -1;

    // Success! The temperature is now stored in the memory address pointed to by 'val'.
    return 0;
}
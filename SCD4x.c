/*
 * SCD4x.c
 * Minimalistic (optimized) access to SCD40/SCD41 CO₂ sensor
 * datasheet: https://sensirion.com/media/documents/48C4B7FB/64C134E7/Sensirion_SCD4x_Datasheet.pdf
 * Copyright (c) 2024 Klaus Keppler - https://github.com/keppler/co2
 */

#include <stddef.h>
#define __DELAY_BACKWARD_COMPATIBLE__ /* see https://www.nongnu.org/avr-libc/user-manual/group__util__delay.html */
#include <util/delay.h>
#include "SCD4x.h"
#include "i2cmaster.h"

#define SCD4x_ADDRESS ((0x62) << 1)

#define SCD4x_COMMAND_GET_FEATURE_SET_VERSION                 0x202F // execution time: 1ms
#define SCD4x_COMMAND_START_PERIODIC_MEASUREMENT              0x21b1 // execution time: 0ms
#define SCD4x_COMMAND_STOP_PERIODIC_MEASUREMENT               0x3f86 // execution time: 500ms
#define SCD4x_COMMAND_GET_SERIAL_NUMBER                       0x3682 // execution time: 1ms
#define SCD4x_COMMAND_GET_DATA_READY_STATUS                   0xe4b8 // execution time: 1ms
#define SCD4x_COMMAND_READ_MEASUREMENT                        0xec05 // execution time: 1ms

uint16_t SCD4x_VALUE_co2 = 0;
int16_t SCD4x_VALUE_temp = 0;
uint16_t SCD4x_VALUE_humidity = 0;

static uint8_t _computeCRC8(const uint8_t *data, uint8_t len) {
    uint8_t crc = 0xFF; // initialize with 0xff

    for (uint8_t x = 0; x < len; x++) {
        crc ^= data[x]; // XOR-in the next input byte
        for (uint8_t i = 0; i < 8; i++) {
            if ((crc & 0x80) != 0) {
                crc = (uint8_t)((crc << 1) ^ 0x31);
            } else {
                crc <<= 1;
            }
        }
    }

    return crc;
}

// Gets two bytes from SCD4x plus CRC.
// Returns true if endTransmission returns zero _and_ the CRC check is valid
static uint8_t _readRegister(uint16_t registerAddress, uint16_t *response, uint8_t count, uint16_t delayMillis) {

    i2c_start_wait(SCD4x_ADDRESS + I2C_WRITE);
    i2c_write(registerAddress >> 8);   // MSB
    i2c_write(registerAddress & 0xFF); // LSB
    i2c_stop();

    _delay_ms(delayMillis);

    if (response == NULL || count == 0) return 0;

    uint8_t ret = 0;
    uint8_t data[2];
    i2c_rep_start(SCD4x_ADDRESS + I2C_READ);
    for (uint8_t i=0; i < count; i++) {
        data[0] = i2c_readAck();
        data[1] = i2c_readAck();
        response[i] = (data[0] << 8) | data[1];
        uint8_t crc = i2c_read(i == count-1 ? 0 : 1);   /* last read: NACK, else ACK */
        if (crc != _computeCRC8(data, 2)) ret = 1;
    }
    i2c_stop();
    return ret;
}

uint8_t SCD4x_startPeriodicMeasurement(void) {
    return _readRegister(SCD4x_COMMAND_START_PERIODIC_MEASUREMENT, NULL, 0, 0);
}

uint8_t SCD4x_stopPeriodicMeasurement(void) {
    return _readRegister(SCD4x_COMMAND_STOP_PERIODIC_MEASUREMENT, NULL, 0, 500);
}

uint8_t SCD4x_getSerialNumber(uint8_t serial[6]) {
    return _readRegister(SCD4x_COMMAND_GET_SERIAL_NUMBER, (uint16_t*)serial, 3, 1);
}

scd4x_sensor_type_t SCD4x_getSensorType(void) {
    /* use "GetFeatureSet" command to detect sensor type */
    uint16_t featureSet;
    if (_readRegister(SCD4x_COMMAND_GET_FEATURE_SET_VERSION, &featureSet, 1, 1) != 0) {
        // error
        return SCD4x_SENSOR_ERROR;
    }
    uint8_t sensor = ((featureSet & 0x1000) >> 12);
    if (sensor == 0x00) return SCD4x_SENSOR_SCD40;
    if (sensor == 0x01) return SCD4x_SENSOR_SCD41;
    return SCD4x_SENSOR_UNKNOWN;
}

uint8_t SCD4x_getData(void) {
    uint8_t ret;
    uint16_t data[3];
    if ((ret = _readRegister(SCD4x_COMMAND_GET_DATA_READY_STATUS, data, 1, 1)) != 0) {
        /* error while reading */
        return ret;
    }
    if (data[0] == 0) return 0xFF; /* no data available */

    /* now read data */
    if ((ret = _readRegister(SCD4x_COMMAND_READ_MEASUREMENT, data, 3, 1)) != 0) {
        /* error while reading */
        return ret;
    }

    SCD4x_VALUE_co2 = data[0];
    SCD4x_VALUE_temp = -450 + (((float)data[1]) * 175 / 6553.6);
    SCD4x_VALUE_humidity = ((float)data[2]) * 100 / 65536;

    return 0;
}

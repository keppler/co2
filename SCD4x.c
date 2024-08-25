/*         ___    ___
 *  __ ___|_  )__/ __| ___ _ _  ___ ___ _ _
 * / _/ _ \/ /___\__ \/ -_) ' \(_-</ _ \ '_|
 *_\__\___/___|  |___/\___|_||_/__/\___/_|_________________________________
 * CO₂ Sensor for Caving -- https://github.com/keppler/co2
 * Minimalistic (optimized) access to SCD40/SCD41 CO₂ sensor
 * Datasheet: https://sensirion.com/media/documents/48C4B7FB/64C134E7/Sensirion_SCD4x_Datasheet.pdf
 * Copyright (c) 2024 Klaus Keppler - https://github.com/keppler/co2
 */

#include <stddef.h>
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
#define SCD4x_COMMAND_GET_SENSOR_ALTITUDE                     0x2322 // execution time: 1ms
#define SCD4x_COMMAND_SET_SENSOR_ALTITUDE                     0x2427 // execution time: 1ms
#define SCD4x_COMMAND_GET_AUTOMATIC_SELF_CALIBRATION          0x2313 // execution time: 1ms
#define SCD4x_COMMAND_SET_AUTOMATIC_SELF_CALIBRATION          0x2416 // execution time: 1ms
#define SCD4x_COMMAND_PERFORM_FORCED_RECALIBRATION            0x362f // execution time: 400ms
#define SCD4x_COMMAND_PERFORM_SELF_TEST                       0x3639 // execution time: 10000ms
#define SCD4x_COMMAND_POWER_DOWN                              0x36e0 // execution time: 1ms
#define SCD4x_COMMAND_WAKE_UP                                 0x36f6 // execution time: 20ms
#define SCD4x_COMMAND_PERSIST_SETTINGS                        0x3615 // execution time: 800ms

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
static uint8_t _readRegister(uint16_t registerAddress, const uint16_t *data, uint8_t dataCount, uint16_t *response, uint8_t responseCount, uint16_t delayMillis) {
    uint8_t buf[2];

    i2c_start_wait(SCD4x_ADDRESS + I2C_WRITE);
    i2c_write(registerAddress >> 8);   // MSB
    i2c_write(registerAddress & 0xFF); // LSB

    // send data (if given)
    for (uint8_t i=0; i < dataCount && data != NULL; i++) {
        buf[0] = data[i] >> 8;      // MSB
        buf[1] = data[i] & 0xFF;    // LSB
        i2c_write(buf[0]);
        i2c_write(buf[1]);
        i2c_write(_computeCRC8(buf, 2)); // CRC
    }

    i2c_stop();

    while (delayMillis > 0) {
        // workaround against overflow on large delays (i.e. 10000ms on self test)
        uint16_t wait = delayMillis > 1000 ? 1000 : delayMillis;
        _delay_ms(wait);
        delayMillis -= wait;
    }

    if (response == NULL || responseCount == 0) return 0;

    uint8_t ret = 0;
    i2c_rep_start(SCD4x_ADDRESS + I2C_READ);
    for (uint8_t i=0; i < responseCount; i++) {
        buf[0] = i2c_readAck();
        buf[1] = i2c_readAck();
        response[i] = (buf[0] << 8) | buf[1];
        uint8_t crc = i2c_read(i == responseCount-1 ? 0 : 1);   /* last read: NACK, else ACK */
        if (crc != _computeCRC8(buf, 2)) ret = 1;
    }
    i2c_stop();
    return ret;
}

uint8_t SCD4x_startPeriodicMeasurement(void) {
    return _readRegister(SCD4x_COMMAND_START_PERIODIC_MEASUREMENT, NULL, 0, NULL, 0, 0);
}

uint8_t SCD4x_stopPeriodicMeasurement(void) {
    return _readRegister(SCD4x_COMMAND_STOP_PERIODIC_MEASUREMENT, NULL, 0, NULL, 0, 500);
}

uint8_t SCD4x_getSerialNumber(uint8_t serial[6]) {
    return _readRegister(SCD4x_COMMAND_GET_SERIAL_NUMBER, NULL, 0, (uint16_t*)serial, 3, 1);
}

scd4x_sensor_type_t SCD4x_getSensorType(void) {
    /* use "GetFeatureSet" command to detect sensor type */
    uint16_t featureSet;
    if (_readRegister(SCD4x_COMMAND_GET_FEATURE_SET_VERSION, NULL, 0, &featureSet, 1, 1) != 0) {
        // error
        return SCD4x_SENSOR_ERROR;
    }
    uint8_t sensor = ((featureSet & 0x1000) >> 12);
    if (sensor == 0x00) return SCD4x_SENSOR_SCD40;
    if (sensor == 0x01) return SCD4x_SENSOR_SCD41;
    return SCD4x_SENSOR_UNKNOWN;
}

uint16_t SCD4x_getSensorAltitude(void) {
    uint16_t data;
    if (_readRegister(SCD4x_COMMAND_GET_SENSOR_ALTITUDE, NULL, 0, &data, 1, 1) != 0) {
        /* error while reading */
        return 9999;
    }
    return data;
}

void SCD4x_setSensorAltitude(uint16_t alt) {
    _readRegister(SCD4x_COMMAND_SET_SENSOR_ALTITUDE, &alt, 1, NULL, 0, 1);
}

uint8_t SCD4x_getData(void) {
    uint8_t ret;
    uint16_t data[3];
    if ((ret = _readRegister(SCD4x_COMMAND_GET_DATA_READY_STATUS, NULL, 0, data, 1, 1)) != 0) {
        /* error while reading */
        return ret;
    }
    if (data[0] == 0) return 0xFF; /* no data available */

    /* now read data */
    if ((ret = _readRegister(SCD4x_COMMAND_READ_MEASUREMENT, NULL, 0, data, 3, 1)) != 0) {
        /* error while reading */
        return ret;
    }

    SCD4x_VALUE_co2 = data[0];
    SCD4x_VALUE_temp = -450 + (((float)data[1]) * 175 / 6553.6);
    SCD4x_VALUE_humidity = ((float)data[2]) * 100 / 65536;

    return 0;
}

scd4x_asc_enabled_t SCD4x_getAutomaticSelfCalibration(void) {
    uint16_t data;
    if (_readRegister(SCD4x_COMMAND_GET_AUTOMATIC_SELF_CALIBRATION, NULL, 0, &data, 1, 1) != 0) {
        /* error while reading, i.e. CRC error */
        return SCD4x_ASC_UNKNOWN;
    }
    switch (data) {
        case 0x00: return SCD4x_ASC_DISABLED;
        case 0x01: return SCD4x_ASC_ENABLED;
        default: return SCD4x_ASC_UNKNOWN;
    }
}

void SCD4x_setAutomaticSelfCalibration(scd4x_asc_enabled_t asc) {
    uint16_t data = asc == SCD4x_ASC_ENABLED ? 0x01 : 0x00;
    _readRegister(SCD4x_COMMAND_SET_AUTOMATIC_SELF_CALIBRATION, &data, 1, NULL, 0, 1);
}

uint16_t SCD4x_performForcedRecalibration(void) {
    uint16_t data = 420;    // set to 420 ppm co2 -- see https://keelingcurve.ucsd.edu/
    if (_readRegister(SCD4x_COMMAND_PERFORM_FORCED_RECALIBRATION, &data, 1, &data, 1, 400) != 0) {
        /* error while reading, i.e. CRC error */
        return 0xFFFF;
    }
    return data;
}

uint16_t SCD4x_performSelfTest(void) {
    uint16_t data;
    if (_readRegister(SCD4x_COMMAND_PERFORM_SELF_TEST, NULL, 0, &data, 1, 10000) != 0) {
        /* error while reading, i.e. CRC error */
        return 0xFFFF;
    }
    return data;
}

void SCD4x_powerDown(void) {
    _readRegister(SCD4x_COMMAND_POWER_DOWN, NULL, 0, NULL, 0, 0);
}

void SCD4x_wakeUp(void) {
    _readRegister(SCD4x_COMMAND_WAKE_UP, NULL, 0, NULL, 0, 0);
}

void SCD4x_persistSettings(void) {
    _readRegister(SCD4x_COMMAND_PERSIST_SETTINGS, NULL, 0, NULL, 0, 0);
}

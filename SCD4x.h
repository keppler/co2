/*         ___    ___
 *  __ ___|_  )__/ __| ___ _ _  ___ ___ _ _
 * / _/ _ \/ /___\__ \/ -_) ' \(_-</ _ \ '_|
 *_\__\___/___|  |___/\___|_||_/__/\___/_|_________________________________
 * CO₂ Sensor for Caving -- https://github.com/keppler/co2
 * Minimalistic (optimized) access to SCD40/SCD41 CO₂ sensor
 * Datasheet: https://sensirion.com/media/documents/48C4B7FB/64C134E7/Sensirion_SCD4x_Datasheet.pdf
 * Copyright (c) 2024 Klaus Keppler - https://github.com/keppler/co2
 */

#ifndef _SCD4X_H
#define _SCD4X_H

#include <stdint.h>

typedef enum {
    SCD4x_SENSOR_SCD40 = 0x00,
    SCD4x_SENSOR_SCD41 = 0x01,
    SCD4x_SENSOR_ERROR = 0xfe,
    SCD4x_SENSOR_UNKNOWN = 0xff
} scd4x_sensor_type_t;

typedef enum {
    SCD4x_ASC_DISABLED = 0x00,
    SCD4x_ASC_ENABLED = 0x01,
    SCD4x_ASC_UNKNOWN = 0xff
} scd4x_asc_enabled_t;

extern uint16_t SCD4x_VALUE_co2;
extern int16_t SCD4x_VALUE_temp;
extern uint16_t SCD4x_VALUE_humidity;

uint8_t SCD4x_startPeriodicMeasurement(void);
uint8_t SCD4x_stopPeriodicMeasurement(void);
uint8_t SCD4x_getSerialNumber(uint8_t serial[6]);
scd4x_sensor_type_t SCD4x_getSensorType(void);
uint8_t SCD4x_getData(void);
uint16_t SCD4x_getSensorAltitude(void);
void SCD4x_setSensorAltitude(uint16_t alt);
scd4x_asc_enabled_t SCD4x_getAutomaticSelfCalibration(void);
void SCD4x_setAutomaticSelfCalibration(scd4x_asc_enabled_t asc);
uint16_t SCD4x_performForcedRecalibration(void);
uint16_t SCD4x_performSelfTest(void);
void SCD4x_powerDown(void);
void SCD4x_wakeUp(void);
void SCD4x_persistSettings(void);

#endif // _SCD4X_H

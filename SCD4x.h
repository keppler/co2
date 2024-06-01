#ifndef _SCD4X_H
#define _SCD4X_H

#include <stdint.h>

typedef enum {
    SCD4x_SENSOR_SCD40 = 0x00,
    SCD4x_SENSOR_SCD41 = 0x01,
    SCD4x_SENSOR_ERROR = 0xfe,
    SCD4x_SENSOR_UNKNOWN = 0xff
} scd4x_sensor_type_t;

extern uint16_t SCD4x_VALUE_co2;
extern int16_t SCD4x_VALUE_temp;
extern uint16_t SCD4x_VALUE_humidity;

uint8_t SCD4x_startPeriodicMeasurement(void);
uint8_t SCD4x_stopPeriodicMeasurement(void);
uint8_t SCD4x_getSerialNumber(uint8_t serial[6]);
scd4x_sensor_type_t SCD4x_getSensorType(void);
uint8_t SCD4x_getData(void);

#endif // _SCD4X_H

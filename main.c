// 1MHz: c:avrdude -c usbasp -p t85 -P usb -v -U lfuse:w:0x62:m -U hfuse:w:0xdf:m -U efuse:w:0xff:m -U flash:w:co2-scd41.hex:i

#include <avr/pgmspace.h>
#include <util/delay.h>
#include "i2cmaster.h"
#include "SSD1306.h"
#include "SCD4x.h"
#include "VCC.h"

#define VCC_MIN 280 // minimum voltage: 2.80V
#define VCC_MAX 370 // maximum voltage: 3.70V

void sensor_error(uint16_t err) {
    SSD1306_writeString(0, 0, PSTR("ERROR:"), 1);
    SSD1306_writeInt(7, 0, (int16_t)err, 10, 0x00, 0);
    while(1);
}

static uint8_t oldPct = 0;
static void writeBattery(uint8_t pct) {
    if (oldPct == pct) return; // nothing has changed
    oldPct = pct;
    //SSD1306_writeString(0, 0, PSTR("       "), 1);
    //uint8_t img[] = {0x18, 0x7e, 0x42, 0x42, 0x7e, 0x7e, 0x7e, 0x7e, 0x7e, 0x7e, 0x7e, 0x7e, 0x7e};
    uint8_t img[] = {0x18, 0x7e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7e};
    for (uint8_t x=0; x<10; x++) {
        img[11-x] = pct > x*10 ? 0x7e : 0x42;
    }
    SSD1306_writeImg(13, 8, img, 0);
    uint8_t x = SSD1306_writeInt(2, 0, pct, 10, 0x00, 0);
    SSD1306_writeChar(x++, 0, '%', 0);
    while (x < 6) SSD1306_writeChar(x++, 0, ' ', 0);
}

int main(void) {
    uint8_t err;

    /* LED (for test purposes)
    DDRB |= (1 << DDB4);
    PORTB &= ~(1 << PB4);
    */

    /* give components enough time to start up */
    _delay_ms(30);

    /* initialize I²C bus */
    i2c_init();

    /* initialize display */
    SSD1306_init();
    SSD1306_clear();
    SSD1306_on();
    SSD1306_writeString(0, 0, PSTR("V23 - 2024-05-12"), 1);
    SSD1306_writeInt(15, 7, 1, 10, 0x00, 0);

    _delay_ms(500);

    if ((err = SCD4x_stopPeriodicMeasurement()) != 0) {
        SSD1306_writeInt(14, 1, err, 16, 0x00, 0);
    }

    SSD1306_writeInt(15, 7, 2, 10, 0x00, 0); // DEBUG

    /* detect sensor type */
    scd4x_sensor_type_t sensorType = SCD4x_getSensorType();
    SSD1306_writeString(0, 1, PSTR("SENSOR:"), 1);
    if (sensorType == SCD4x_SENSOR_SCD40) {
        SSD1306_writeString(8, 1, PSTR("SCD40"), 1);
    } else if (sensorType == SCD4x_SENSOR_SCD41) {
        SSD1306_writeString(8, 1, PSTR("SCD41"), 1);
    } else if (sensorType == SCD4x_SENSOR_ERROR) {
        SSD1306_writeString(8, 1, PSTR("ERROR"), 1);
    } else {
        SSD1306_writeString(8, 1, PSTR("UNKNOWN"), 1);
    }

    /* read serial number */
    {
        /* do this within separate block, to free memory afterwards */
        uint8_t serial[6];
        SSD1306_writeString(0, 2, PSTR("SERIAL:"), 1);
        if ((err = SCD4x_getSerialNumber(serial)) != 0) {
            /* error while reading serial */
            SSD1306_writeString(0, 3, PSTR("CRC ERROR"), 1);
        } else {
            uint8_t i;
            for (i=0; i<3; i++) {
                uint8_t u = serial[i*2];
                if (u < 0x10) {
                    SSD1306_writeInt(i*5, 3, 0, 16, 0x00, 0);
                    SSD1306_writeInt((i*5)+1, 3, u, 16, 0x00, 0);
                } else {
                    SSD1306_writeInt(i*5, 3, u, 16, 0x00, 0);
                }
                u = serial[(i*2)+1];
                if (u < 0x10) {
                    SSD1306_writeInt((i*5)+2, 3, 0, 16, 0x00, 0);
                    SSD1306_writeInt((i*5)+3, 3, u, 16, 0x00, 0);
                } else {
                    SSD1306_writeInt((i*5)+2, 3, u, 16, 0x00, 0);
                }
            }
        }
    }

    {
        SSD1306_writeString(0, 5, PSTR("VCC:"), 1);
        uint16_t vcc = VCC_get();
        uint8_t x;
        x = SSD1306_writeInt(5, 5, vcc / 100, 10, 0x00, 0);
        SSD1306_writeChar(x++, 5, '.', 0x00);
        x = SSD1306_writeInt(x, 5, vcc % 100, 10, 0x00, 0);
        SSD1306_writeChar(x++, 5, 'V', 0x00);
    }

    _delay_ms(3000);
    SSD1306_clear();

    if (SCD4x_startPeriodicMeasurement() != 0) {
        SSD1306_writeString(0, 2, PSTR("START ERROR"), 1);
    }

    writeBattery(0);

    SSD1306_writeString(4, 3, PSTR("."), 1);
    SSD1306_writeString(7, 2, PSTR("^C"), 1);
    SSD1306_writeString(14, 2, PSTR("%"), 1);
    SSD1306_writeString(14, 3, PSTR("RH"), 1);

    SSD1306_writeString(0, 5, PSTR("CO2 MAX:"), 1);
    SSD1306_writeString(12, 6, PSTR("CO2"), 1);
    SSD1306_writeString(12, 7, PSTR("PPM"), 1);

    uint8_t tick = 0;
    uint16_t co2max = 0;
    static const char tickChars[] = {'[','\\',']','\\'};
    while(1) {
        err = SCD4x_getData();
        if (err > 1) {
            SSD1306_writeString(0, 3, PSTR("ERR:       "), 1);
            SSD1306_writeInt(5, 3, err, 16, 0x00, 0);
        } else if (err == 0) {
            if (SCD4x_VALUE_co2 > co2max) co2max = SCD4x_VALUE_co2;
            SSD1306_writeInt(9, 5, co2max, 10, 0, 0);
            SSD1306_writeInt(1, 6, SCD4x_VALUE_co2, 10, 0x04, 5);
            SSD1306_writeInt(0, 2, SCD4x_VALUE_temp / 10, 10, 0x04, 2);
            SSD1306_writeInt(5, 2, SCD4x_VALUE_temp % 10, 10, 0x04, 0);
            SSD1306_writeInt(10, 2, SCD4x_VALUE_humidity, 10, 0x04, 2);
        }
        _delay_ms(1000);
        SSD1306_writeChar(15, 0, tickChars[++tick % 4], 0);
        if (tick % 10 == 0) {
            // update VCC display every ~10 seconds
            uint16_t vcc = VCC_get();
            // SSD1306_writeInt(8, 0, vcc, 10, 0x00, 0); // DEBUG output raw voltage
            if (vcc < VCC_MIN) vcc = VCC_MIN;
            else if (vcc > VCC_MAX) vcc = VCC_MAX;
            writeBattery(((vcc - VCC_MIN) * 100) / (VCC_MAX-VCC_MIN));
        }
    }
}

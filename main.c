/*         ___    ___
 *  __ ___|_  )__/ __| ___ _ _  ___ ___ _ _
 * / _/ _ \/ /___\__ \/ -_) ' \(_-</ _ \ '_|
 *_\__\___/___|  |___/\___|_||_/__/\___/_|_________________________________
 * CO₂ Sensor for Caving -- https://github.com/keppler/co2
 * Program entry and main measurement loop
 */

#include <avr/pgmspace.h>
#include <util/delay.h>
#include "button.h"
#include "i2cmaster.h"
#include "menu.h"
#include "timer.h"
#include "SSD1306.h"
#include "SCD4x.h"
#include "VCC.h"
#include "main.h"

#define VCC_MIN 280 // minimum voltage: 2.80V
#define VCC_MAX 370 // maximum voltage: 3.70V

const char app_version[] PROGMEM = "V25 - 2024-12-17";

// show battery status
static uint8_t oldPct = 0;
static uint8_t vccPct = 0;
static void writeBattery(uint8_t pct) {
    if (oldPct == pct) return; // nothing has changed
    oldPct = pct;
    //uint8_t img[] = {0x18, 0x7e, 0x42, 0x42, 0x7e, 0x7e, 0x7e, 0x7e, 0x7e, 0x7e, 0x7e, 0x7e, 0x7e};
    uint8_t img[13];
    img[0] = 0x18;
    img[1] = img[12] = 0x7e;
    for (uint8_t x=0; x<10; x++) {
        img[11-x] = pct > x*10 ? 0x7e : 0x42;
    }
    SSD1306_writeImg(13, 8, img, 0);
    uint8_t x = SSD1306_writeInt(2, 0, pct, 10, 0x00, 0);
    SSD1306_writeChar(x++, 0, '%', 0);
    while (x < 6) SSD1306_writeChar(x++, 0, ' ', 0);
}

static uint8_t tick;
static uint16_t co2max = 0;
static uint16_t lastThreshold = 2000;
static uint8_t belowThresholdSecs = 0;

static void app_beep(uint16_t ms) {
    PORTB |= (1 << PB3);
    //uint64_t old_ms = timer_millis();
    //while (timer_millis() - old_ms < 100);
    while (ms >= 100) { _delay_ms(100); ms -= 100; }
    PORTB &= ~(1 << PB3);
}

static void main_enter(void) {
    tick = 0;
    SSD1306_clear();

    if (SCD4x_startPeriodicMeasurement() != 0) {
        SSD1306_writeString(0, 2, PSTR("START ERROR"), 1);
    }

    oldPct = 0xff; // force update
    writeBattery(vccPct);

    SSD1306_writeString(4, 3, PSTR("."), 1);
    SSD1306_writeString(7, 2, PSTR("^C"), 1);
    SSD1306_writeString(14, 2, PSTR("%"), 1);
    SSD1306_writeString(14, 3, PSTR("RH"), 1);

    SSD1306_writeString(0, 5, PSTR("CO2 MAX:"), 1);
    SSD1306_writeInt(9, 5, co2max, 10, 0, 0);
    SSD1306_writeString(12, 6, PSTR("CO2"), 1);
    SSD1306_writeString(12, 7, PSTR("PPM"), 1);
}

static void main_leave(void) {
    SCD4x_stopPeriodicMeasurement();
}

static void main_loop(void) {
    static const char tickChars[] = {'[','\\',']','\\'};
    static uint64_t old_ms = 0;
    uint8_t err;

    uint8_t btn = button_pressed();
    if (btn == 1) {
        app_state_next(MENU);
        return;
    }

    if (timer_millis() - old_ms >= 1000) {
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

            /* check threshold */
            if (SCD4x_VALUE_co2 > lastThreshold + 2000) {
                /* beep. */
                // ... 6000-8000: 1x, 10.000-18.000: 2x, 20.000-24.000 3x, >=24.000: 4x
                uint8_t cnt = 1;
                if (SCD4x_VALUE_co2 >= 10000) cnt++;
                if (SCD4x_VALUE_co2 >= 20000) cnt++;
                if (SCD4x_VALUE_co2 >= 24000) cnt++;
                while (cnt-- > 0) {
                    app_beep(400);
                    _delay_ms(200);
                }

                /* update threshold - use 2000ppm steps */
                lastThreshold = SCD4x_VALUE_co2 - (SCD4x_VALUE_co2 % 2000);
            } else if (SCD4x_VALUE_co2 < lastThreshold - 2000) {
                if (belowThresholdSecs > 0) {
                    belowThresholdSecs -= 5;    /* remember: we have one valid measurement every five seconds! */
                    if (belowThresholdSecs == 0) {
                        /* if less than last threshold for more than 60sec, reduce threshold by 2000ppm */
                        if (lastThreshold >= 6000) lastThreshold -= 2000;
                        /* ToDo: maybe beep short (100ms) to signal reducing co2 level? */
                        app_beep(100);
                    }
                }
            } else {
                /* within range of lastThreshold +/- 1999, reset reduce counter */
                belowThresholdSecs = 60;
            }
        }
        if (tick % 10 == 0) {
            // update VCC display every ~10 seconds
            uint16_t vcc = VCC_get();
            // SSD1306_writeInt(8, 0, vcc, 10, 0x00, 0); // DEBUG output raw voltage
            if (vcc < VCC_MIN) vcc = VCC_MIN;
            else if (vcc > VCC_MAX) vcc = VCC_MAX;
            vccPct = ((vcc - VCC_MIN) * 100) / (VCC_MAX - VCC_MIN);
            writeBattery(vccPct);
        }
        SSD1306_writeChar(15, 0, tickChars[++tick % 4], 0);
        old_ms = timer_millis();
    }

}

static enum app_state_t app_state = MAINLOOP, app_lastState = MAINLOOP;

void app_state_next(enum app_state_t next) {
    app_lastState = app_state;
    app_state = next;
}

void app_wakeup(uint8_t initial) {
    uint8_t err;

    /* initialize display */
    SSD1306_init();
    SSD1306_clear();
    SSD1306_on();
    SSD1306_writeString(0, 0, app_version, 1);

    // beep.
    app_beep(100);

    //_delay_ms(400);

    if ((err = SCD4x_stopPeriodicMeasurement()) != 0) {
        SSD1306_writeInt(14, 1, err, 16, 0x00, 0);
    }

    /* detect sensor type */
    scd4x_sensor_type_t sensorType = SCD4x_getSensorType();
    SSD1306_writeString(0, 1, PSTR("SENSOR:"), 1);
    if (sensorType == SCD4x_SENSOR_SCD40) {
        SSD1306_writeString(8, 1, PSTR("SCD40"), 1);
    } else if (sensorType == SCD4x_SENSOR_SCD41) {
        SSD1306_writeString(8, 1, PSTR("SCD41"), 1);
    } else if (sensorType == SCD4x_SENSOR_ERROR) {
        SSD1306_writeString(8, 1, PSTR("ERROR"), 1);
        while(1);
    } else {
        SSD1306_writeString(8, 1, PSTR("UNKNOWN"), 1);
        while(1);
    }

    /* read serial number */
    if (initial) {
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

    /* reset max and threshold values after power-off */
    lastThreshold = 2000;
    belowThresholdSecs = 0;
    co2max = 0;

    _delay_ms(3000);
}

int main(void) {
    /* initialize time functions */
    timer_init();

    button_init();

    // init buzzer
    DDRB |= (1 << DDB3);
    PORTB &= ~(1 << PB3);

    /* give components enough time to start up */
    _delay_ms(30);

    /* initialize I²C bus */
    i2c_init();

    app_wakeup(1);

    main_enter();

    while(1) {
        button_read();
        if (app_lastState != app_state) {
            // state transition
            switch (app_lastState) {
                case MAINLOOP: main_leave(); break;
                default: break;
            }
            switch (app_state) {
                case MAINLOOP: main_enter(); break;
                case MENU: menu_enter(); break;
            }
            app_lastState = app_state;
        }
        switch (app_state) {
            case MAINLOOP: main_loop(); break;
            case MENU: menu_loop(); break;
        }

    }
}

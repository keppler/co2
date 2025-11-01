/*         ___    ___
 *  __ ___|_  )__/ __| ___ _ _  ___ ___ _ _
 * / _/ _ \/ /___\__ \/ -_) ' \(_-</ _ \ '_|
 *_\__\___/___|  |___/\___|_||_/__/\___/_|_________________________________
 * CO₂ Sensor for Caving -- https://github.com/keppler/co2
 * Program entry and main measurement loop
 */

#include <avr/pgmspace.h>
#include <util/delay.h>
#include "beep.h"
#include "button.h"
#include "i2cmaster.h"
#include "menu.h"
#include "splash.h"
#include "timer.h"
#include "SSD1306.h"
#include "SCD4x.h"
#include "VCC.h"
#include "main.h"

#define VCC_MIN 280 /* minimum voltage: 2.80V */
#define VCC_MAX 370 /* maximum voltage: 3.70V */

const char app_version[] PROGMEM = "V34 - 2025-10-26";

// show battery status
static uint8_t oldPct = 0;
static uint8_t vccPct = 0;
static void writeBattery(uint8_t pct) {
    if (oldPct == pct) return; /* nothing has changed */
    oldPct = pct;
    /* uint8_t img[] = {0x18, 0x7e, 0x42, 0x42, 0x7e, 0x7e, 0x7e, 0x7e, 0x7e, 0x7e, 0x7e, 0x7e, 0x7e}; */
    uint8_t img[13];
    img[0] = 0x18;
    img[1] = img[12] = 0x7e;
    for (uint8_t x=0; x<10; x++) {
        img[11-x] = pct > x*10 ? 0x7e : 0x42;
    }
    SSD1306_writeImg(0, 0, 13, 8, img, 0);
    uint8_t x = SSD1306_writeInt(2, 0, pct, 10, 0x00, 0);
    SSD1306_writeChar(x++, 0, '%', 0);
    while (x < 6) SSD1306_writeChar(x++, 0, ' ', 0);
}

static uint8_t tick;
static uint16_t co2max = 0;
static uint16_t lastThreshold = 2000;
static uint8_t belowThresholdSecs = 0;

typedef enum {
    MAIN_STATE_EMPTY,
    MAIN_STATE_STARTING,
    MAIN_STATE_RUNNING,
} main_state_t;
static main_state_t main_state = MAIN_STATE_EMPTY;

static void main_enter(void) {
    tick = 0;
    SSD1306_clear();

    if (SCD4x_startPeriodicMeasurement() != 0) {
        SSD1306_writeString(0, 2, PSTR("START ERROR"), 1);
    }

    oldPct = 0xff; // force update
    writeBattery(vccPct);

    main_state = MAIN_STATE_EMPTY;
}

static void main_leave(void) {
    SCD4x_stopPeriodicMeasurement();
}

static void main_loop(void) {
    static const char tickChars[] = {'<','=','>','='};
    static uint64_t old_ms = 0;
    uint8_t err;

    uint8_t btn = button_pressed();
    if (btn == 1) {
        app_state_next(MENU);
        return;
    }

    if (timer_millis() - old_ms >= 1000) {
        if (main_state == MAIN_STATE_STARTING) {
            SSD1306_writeInt(6, 5, 90 - (timer_millis() / 1000), 10, 0, 2);
        }
        err = SCD4x_getData();
        if (err == 0) {
            if (main_state == MAIN_STATE_EMPTY) {
                SSD1306_writeString(4, 3, PSTR("."), 1);
                SSD1306_writeString(7, 2, PSTR("[C"), 1);   /* '[' is displayed as '°' */
                SSD1306_writeString(14, 2, PSTR("%"), 1);
                SSD1306_writeString(14, 3, PSTR("RH"), 1);

                if (main_state == MAIN_STATE_RUNNING || timer_millis() > 90000) {
                    SSD1306_writeString(0, 5, PSTR("CO2 MAX:"), 1);
                    SSD1306_writeInt(9, 5, co2max, 10, 0, 0);
                } else {
                    SSD1306_writeString(0, 5, PSTR("INIT:"), 1);
                    SSD1306_writeInt(6, 5, 90 - (timer_millis() / 1000), 10, 0, 2);
                }
                SSD1306_writeString(12, 6, PSTR("CO2"), 1);
                SSD1306_writeString(12, 7, PSTR("PPM"), 1);
                main_state = MAIN_STATE_STARTING;
            }
            if (main_state == MAIN_STATE_RUNNING || timer_millis() > 90000) {
                if (main_state == MAIN_STATE_STARTING) SSD1306_writeString(0, 5, PSTR("CO2 MAX:"), 1);
                main_state = MAIN_STATE_RUNNING;
            }

            if (main_state == MAIN_STATE_RUNNING) {
                if (SCD4x_VALUE_co2 > co2max) co2max = SCD4x_VALUE_co2;
                SSD1306_writeInt(9, 5, co2max, 10, 0, 0);
            }
            uint8_t flags = SSD1306_FLAG_DOUBLE;
            if (main_state == MAIN_STATE_STARTING) flags |= SSD1306_FLAG_LIGHT;
            SSD1306_writeInt(1, 6, SCD4x_VALUE_co2, 10, flags, 5);
            SSD1306_writeInt(0, 2, SCD4x_VALUE_temp / 10, 10, flags, 2);
            SSD1306_writeInt(5, 2, SCD4x_VALUE_temp % 10, 10, flags, 0);
            SSD1306_writeInt(10, 2, SCD4x_VALUE_humidity, 10, flags, 2);

            /* check threshold */
            if (SCD4x_VALUE_co2 > lastThreshold + 2000) {
                /* beep. */
                // ... 6000-8000: 1x, 10.000-18.000: 2x, 20.000-24.000 3x, >=24.000: 4x
                uint8_t cnt = 1;
                if (SCD4x_VALUE_co2 >= 10000) cnt++;
                if (SCD4x_VALUE_co2 >= 20000) cnt++;
                if (SCD4x_VALUE_co2 >= 24000) cnt++;
                while (cnt-- > 0) {
                    beep(BEEP_WARN);
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
                        beep(BEEP_RELAX);
                    }
                }
            } else {
                /* within range of lastThreshold +/- 1999, reset reduce counter */
                belowThresholdSecs = 60;
            }
        } else if (err != 0xFF) {
            /* ignore case of 0xff (no data available) */
            SSD1306_writeString(0, 3, PSTR("ERR:       "), 1);
            SSD1306_writeInt(5, 3, err, 16, 0x00, 0);
        }
        if (tick % 10 == 0) {
            /* update VCC display every ~10 seconds */
            uint16_t vcc = VCC_get();
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

    SSD1306_writeImg(5, 1, splash_width, splash_height, splash_data, 2);
    SSD1306_writeString(0, 5, app_version, SSD1306_FLAG_PGM);

    /* beep. */
    beep(BEEP_STARTUP);

    if ((err = SCD4x_stopPeriodicMeasurement()) != 0) {
        SSD1306_writeInt(14, 0, err, 16, 0x00, 0);
    }

    /* detect sensor type */
    scd4x_sensor_type_t sensorType = SCD4x_getSensorType();
    if (sensorType == SCD4x_SENSOR_SCD40) {
        SSD1306_writeString(0, 6, PSTR("SCD40"), 1);
    } else if (sensorType == SCD4x_SENSOR_SCD41) {
        SSD1306_writeString(0, 6, PSTR("SCD41"), 1);
    } else if (sensorType == SCD4x_SENSOR_ERROR) {
        SSD1306_writeString(0, 6, PSTR("ERROR"), 1);
        while(1);
    } else {
        SSD1306_writeString(0, 6, PSTR("UNKNW"), 1);
        while(1);
    }

    {
        SSD1306_writeString(7, 6, PSTR("VCC 0.00V"), 1);
        uint16_t vcc = VCC_get();
        uint8_t x;
        x = SSD1306_writeInt(11, 6, vcc / 100, 10, 0x00, 0);
        //SSD1306_writeChar(x++, 5, '.', 0x00);
        x = SSD1306_writeInt(++x, 6, vcc % 100, 10, SSD1306_FLAG_FILL_ZERO, 2);
        //SSD1306_writeChar(x++, 5, 'V', 0x00);
    }

/* disabled display of serial number to save precious memory on ATtiny85 */
#if 0
    /* read serial number */
    if (initial) {
        uint8_t serial[6];
        if ((err = SCD4x_getSerialNumber(serial)) != 0) {
            /* error while reading serial */
            SSD1306_writeString(0, 7, PSTR("CRC ERROR"), 1);
        } else {
            uint8_t i;
            for (i=0; i<3; i++) {
                uint8_t u = serial[i*2];
                if (u < 0x10) {
                    SSD1306_writeInt(i*5, 7, 0, 16, 0x00, 0);
                    SSD1306_writeInt((i*5)+1, 7, u, 16, 0x00, 0);
                } else {
                    SSD1306_writeInt(i*5, 7, u, 16, 0x00, 0);
                }
                u = serial[(i*2)+1];
                if (u < 0x10) {
                    SSD1306_writeInt((i*5)+2, 7, 0, 16, 0x00, 0);
                    SSD1306_writeInt((i*5)+3, 7, u, 16, 0x00, 0);
                } else {
                    SSD1306_writeInt((i*5)+2, 7, u, 16, 0x00, 0);
                }
            }
        }
    }
#endif

    /* reset max and threshold values after power-off */
    lastThreshold = 2000;
    belowThresholdSecs = 0;
    co2max = 0;

    _delay_ms(3000);
}

int main(void) {
    /* PB3: sensor power */
    DDRB |= (1 << DDB3);    /* set port mode to OUTPUT */
    PORTB |= (1 << PB3);    /* set ON */

    /* initialize time functions */
    timer_init();

    button_init();

    /* init sound transducer */
    beep_init();

    /* initialize I²C bus */
    i2c_init();

    app_wakeup(1);

    main_enter();

    for (;;) {
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

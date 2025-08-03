/*         ___    ___
 *  __ ___|_  )__/ __| ___ _ _  ___ ___ _ _
 * / _/ _ \/ /___\__ \/ -_) ' \(_-</ _ \ '_|
 *_\__\___/___|  |___/\___|_||_/__/\___/_|_________________________________
 * CO₂ Sensor for Caving -- https://github.com/keppler/co2
 * Configuration menu
 * Copyright (c) 2024 Klaus Keppler - https://github.com/keppler/co2
 */

#include <stdint.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <avr/power.h>
#include <avr/sleep.h>
#include <util/delay.h>
#include "SCD4x.h"
#include "SSD1306.h"
#include "beep.h"
#include "button.h"
#include "main.h"
#include "timer.h"
#include "menu.h"

static uint8_t cursor;
static scd4x_asc_enabled_t asc_status = SCD4x_ASC_UNKNOWN;
static uint64_t timeout_ms;
static uint16_t altitude;

static void do_asc(void) {
    // toggle ASC setting
    SCD4x_setAutomaticSelfCalibration(asc_status == SCD4x_ASC_DISABLED ? SCD4x_ASC_ENABLED : SCD4x_ASC_DISABLED);
    asc_status = SCD4x_getAutomaticSelfCalibration();
    switch (asc_status) {
        case SCD4x_ASC_DISABLED: SSD1306_writeString(13, 0, PSTR("OFF"), 1); break;
        case SCD4x_ASC_ENABLED: SSD1306_writeString(13, 0, PSTR("ON "), 1); break;
        default: SSD1306_writeString(13, 1, PSTR("???"), 1); break;
    }
    if (asc_status != SCD4x_ASC_UNKNOWN) SCD4x_persistSettings();
}

static void do_forced_recalibration(void) {
    SSD1306_clear();
    SSD1306_writeString(0, 0, PSTR("FORCE CALIBRATE"), 1);
    SSD1306_writeString(0, 1, PSTR("TO 420 PPM CO2 ?"), 1);
    SSD1306_writeString(1, 3, PSTR("CONTINUE"), 1);
    SSD1306_writeString(0, 4, PSTR("*CANCEL"), 1);
    timeout_ms = timer_millis();
    uint8_t subCursor = 1;
    while(1) {
        button_read();
        uint8_t btn = button_pressed();
        if (btn == 1) {
            SSD1306_writeString(0, 3+subCursor, PSTR(" "), 1);
            subCursor++;
            subCursor %= 2; /* if (subCursor == 2) subCursor = 0; */
            SSD1306_writeString(0, 3+subCursor, PSTR("*"), 1);
        } else if (btn == 2) {
            // long press...
            if (subCursor == 1) return;
            // else: do recalibration...
            SSD1306_writeString(1, 3, PSTR("SAVING..."), 1);
            _delay_ms(500);
            uint16_t res = SCD4x_performForcedRecalibration();
            SSD1306_writeString(1, 3, PSTR("DONE:    "), 1);
            SSD1306_writeInt(7, 3, (int32_t)res - 0x8000, 10, 0x00, 0);
            _delay_ms(2000);
            return;
        }
        if (timer_millis() - timeout_ms > 5000) return;
    }
}

static void do_altitude(void) {
    SSD1306_writeInt(11, 2, altitude, 10, 0x02, 4);
    while(1) {
       button_read();
       uint8_t btn = button_pressed();
       if (btn == 1) {
           // increase & update value
           altitude += 100;
           if (altitude > 3000) altitude = 0;
           SSD1306_writeInt(11, 2, altitude, 10, 0x02, 4);
       } else if (btn == 2) {
           // save new altitude
           SCD4x_setSensorAltitude(altitude);
           SCD4x_persistSettings();
           // leave loop
           break;
       }
    }
    // write non-inverted
    SSD1306_writeInt(11, 2, altitude, 10, 0x00, 4);
}

static void do_selftest(void) {
    SSD1306_clear();
    SSD1306_writeString(0, 0, PSTR("TESTING..."), 1);
    uint16_t status = SCD4x_performSelfTest();
    SSD1306_writeString(0, 0, PSTR("DONE.     "), 1);
    SSD1306_writeString(0, 1, PSTR("STATUS:"), 1);
    if (status == 0) {
        SSD1306_writeString(8, 1, PSTR("OK"), 1);
    } else {
        SSD1306_writeString(8, 1, PSTR("ERR"), 1);
        SSD1306_writeInt(12, 1, status, 10, 0x00, 0);
    }
    _delay_ms(2000);
}

static void do_poweroff(void) {
    uint64_t btn_ts;

    SSD1306_clear();
    SSD1306_writeString(0, 0, PSTR("-- POWER OFF --"), 1);
    SCD4x_powerDown();
    _delay_ms(500);
    beep(BEEP_SHUTDOWN);
    _delay_ms(1000);
    SSD1306_off();

DO_SLEEP:
    set_sleep_mode(SLEEP_MODE_PWR_DOWN);
    power_all_disable();
    sleep_mode();

    // when we get here, we've been woken up
    power_all_enable();
    timer_reset();
    button_reset();
    btn_ts = timer_millis();
    while(1) {
        button_read();
        uint8_t btn = button_pressed();
        if (btn == 2) break;    // long press
        if (timer_millis() - btn_ts > 2000) goto DO_SLEEP;
    }

    // short beep here (to signal that device is powered up; it takes some time unless display is showing something)
    beep(BEEP_SHORT);

    SCD4x_wakeUp();
    app_wakeup(0);
}

void menu_enter(void) {
    SSD1306_clear();
    SSD1306_writeString(1, 0, PSTR("AUTO-CALIB:"), 1);
    asc_status = SCD4x_getAutomaticSelfCalibration();
    switch (asc_status) {
        case SCD4x_ASC_DISABLED: SSD1306_writeString(13, 0, PSTR("OFF"), 1); break;
        case SCD4x_ASC_ENABLED: SSD1306_writeString(13, 0, PSTR("ON"), 1); break;
        default: SSD1306_writeString(13, 0, PSTR("???"), 1); break;
    }
    SSD1306_writeString(1, 1, PSTR("FORCE CALIBRATE"), 1);
    SSD1306_writeString(1, 2, PSTR("ALTITUDE:"), 1);
    altitude = SCD4x_getSensorAltitude();
    SSD1306_writeInt(11, 2, altitude, 10, 0x00, 4);

    SSD1306_writeString(1, 3, PSTR("SELF TEST"), 1);
    SSD1306_writeString(1, 4, PSTR("POWER OFF"), 1);
    SSD1306_writeString(1, 5, PSTR("BACK"), 1);
    cursor = 5;
    SSD1306_writeString(0, 5, PSTR("*"), 1);
    timeout_ms = timer_millis();
}

void menu_loop(void) {
    uint8_t btn = button_pressed();
    if (btn > 0) timeout_ms = timer_millis();
    if (btn == 1) {
        SSD1306_writeString(0, cursor, PSTR(" "), 1);
        cursor++;
        cursor %= 6; /* if (cursor == 6) cursor = 0; */
        SSD1306_writeString(0, cursor, PSTR("*"), 1);
    } else if (btn == 2) {
        if (cursor == 0) {
            // set ASC
            do_asc();
            timeout_ms = timer_millis();
        } else if (cursor == 1) {
            // force calibration
            do_forced_recalibration();
            menu_enter();
        } else if (cursor == 2) {
            // set altitude test
            do_altitude();
            timeout_ms = timer_millis();
        } else if (cursor == 3) {
            // self test
            do_selftest();
            menu_enter();
        } else if (cursor == 4) {
            // power off
            do_poweroff();
            // returning here means, device was woken up
            app_state_next(MAINLOOP);
        } else if (cursor == 5) {
            // back
            app_state_next(MAINLOOP);
        }
        return;
    }
    if (timer_millis() - timeout_ms > 10000) {
        // timeout
        app_state_next(MAINLOOP);
    }
}

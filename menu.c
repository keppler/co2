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
#include <avr/sleep.h>
#include <util/delay.h>
#include "SCD4x.h"
#include "SSD1306.h"
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
        case SCD4x_ASC_DISABLED: SSD1306_writeString(13, 1, PSTR("OFF"), 1); break;
        case SCD4x_ASC_ENABLED: SSD1306_writeString(13, 1, PSTR("ON "), 1); break;
        default: SSD1306_writeString(13, 1, PSTR("???"), 1); break;
    }
    if (asc_status != SCD4x_ASC_UNKNOWN) SCD4x_persistSettings();
}

static void do_forced_recalibration(void) {
    SSD1306_clear();
    SSD1306_writeString(0, 0, PSTR("==CALIBRATION=="), 1);
    SSD1306_writeString(0, 2, PSTR("FORCE CALIBRATE"), 1);
    SSD1306_writeString(0, 3, PSTR("TO 420 PPM CO2 ?"), 1);
    SSD1306_writeString(1, 5, PSTR("CONTINUE"), 1);
    SSD1306_writeString(0, 6, PSTR("*CANCEL"), 1);
    timeout_ms = timer_millis();
    uint8_t subCursor = 1;
    while(1) {
        button_read();
        uint8_t btn = button_pressed();
        if (btn == 1) {
            SSD1306_writeString(0, 5+subCursor, PSTR(" "), 1);
            subCursor++;
            if (subCursor == 2) subCursor = 0;
            SSD1306_writeString(0, 5+subCursor, PSTR("*"), 1);
        } else if (btn == 2) {
            // long press...
            if (subCursor == 1) return;
            // else: do recalibration...
            SSD1306_writeString(1, 5, PSTR("SAVING..."), 1);
            _delay_ms(500);
            uint16_t res = SCD4x_performForcedRecalibration();
            SSD1306_writeString(1, 5, PSTR("DONE:    "), 1);
            SSD1306_writeInt(7, 5, (int32_t)res - 0x8000, 10, 0x00, 0);
            _delay_ms(2000);
            return;
        }
        if (timer_millis() - timeout_ms > 5000) return;
    }
}

static void do_altitude(void) {
    SSD1306_writeInt(11, 3, altitude, 10, 0x02, 4);
    while(1) {
       button_read();
       uint8_t btn = button_pressed();
       if (btn == 1) {
           // increase & update value
           altitude += 100;
           if (altitude > 3000) altitude = 0;
           SSD1306_writeInt(11, 3, altitude, 10, 0x02, 4);
       } else if (btn == 2) {
           // save new altitude
           SCD4x_setSensorAltitude(altitude);
           SCD4x_persistSettings();
           // leave loop
           break;
       }
    }
    // write non-inverted
    SSD1306_writeInt(11, 3, altitude, 10, 0x00, 4);
}

static void do_selftest(void) {
    SSD1306_clear();
    SSD1306_writeString(0, 0, PSTR("== SELF TEST =="), 1);
    _delay_ms(500);
    SSD1306_writeString(0, 2, PSTR("TESTING..."), 1);
    uint16_t status = SCD4x_performSelfTest();
    SSD1306_writeString(0, 2, PSTR("DONE.     "), 1);
    SSD1306_writeString(0, 3, PSTR("STATUS:"), 1);
    if (status == 0) {
        SSD1306_writeString(8, 3, PSTR("OK"), 1);
    } else {
        SSD1306_writeString(8, 3, PSTR("ERR"), 1);
        SSD1306_writeInt(12, 3, status, 10, 0x00, 0);
    }
    _delay_ms(2000);
}

static void do_poweroff(void) {
    uint64_t btn_ts;

    SSD1306_clear();
    SSD1306_writeString(0, 0, PSTR("== POWER OFF =="), 1);
    _delay_ms(2000);
    SSD1306_off();
    SCD4x_powerDown();
DO_SLEEP:
    cli();
    set_sleep_mode(SLEEP_MODE_PWR_DOWN);
    sleep_enable();
    sei();
    sleep_cpu();
    sleep_disable();
    // when we get here, we've been woken up
    btn_ts = timer_millis();
    while(1) {
        button_read();
        uint8_t btn = button_pressed();
        if (btn == 2) break;    // long press
        if (timer_millis() - btn_ts > 2000) goto DO_SLEEP;
    }
    SSD1306_on();
    SCD4x_wakeUp();
}

void menu_enter(void) {
    SSD1306_clear();
    SSD1306_writeString(1, 0, PSTR("INTV: 5 SEC"), 1);
    SSD1306_writeString(1, 1, PSTR("AUTO-CALIB:"), 1);
    asc_status = SCD4x_getAutomaticSelfCalibration();
    switch (asc_status) {
        case SCD4x_ASC_DISABLED: SSD1306_writeString(13, 1, PSTR("OFF"), 1); break;
        case SCD4x_ASC_ENABLED: SSD1306_writeString(13, 1, PSTR("ON"), 1); break;
        default: SSD1306_writeString(13, 1, PSTR("???"), 1); break;
    }
    SSD1306_writeString(1, 2, PSTR("FORCE CALIBRATE"), 1);
    SSD1306_writeString(1, 3, PSTR("ALTITUDE:"), 1);
    altitude = SCD4x_getSensorAltitude();
    SSD1306_writeInt(11, 3, altitude, 10, 0x00, 4);

    SSD1306_writeString(1, 4, PSTR("SELF TEST"), 1);
    SSD1306_writeString(1, 5, PSTR("POWER OFF"), 1);
    SSD1306_writeString(1, 6, PSTR("BACK"), 1);
    cursor = 6;
    SSD1306_writeString(0, 6, PSTR("*"), 1);
    timeout_ms = timer_millis();
}

void menu_loop(void) {
    uint8_t btn = button_pressed();
    if (btn > 0) timeout_ms = timer_millis();
    if (btn == 1) {
        SSD1306_writeString(0, cursor, PSTR(" "), 1);
        cursor++;
        if (cursor == 7) cursor = 0;
        if (cursor == 0) cursor++; // skip over non-implemented items...
        SSD1306_writeString(0, cursor, PSTR("*"), 1);
    } else if (btn == 2) {
        if (cursor == 1) {
            // set ASC
            do_asc();
            timeout_ms = timer_millis();
            return;
        } else if (cursor == 2) {
            // force calibration
            do_forced_recalibration();
            menu_enter();
            return;
        } else if (cursor == 3) {
            // set altitude test
            do_altitude();
            timeout_ms = timer_millis();
            return;
        } else if (cursor == 4) {
            // self test
            do_selftest();
            menu_enter();
            return;
        } else if (cursor == 5) {
            // power off
            do_poweroff();
            // returning here means, device was woken up
            app_state_next(MAINLOOP);
            return;
        } else if (cursor == 6) {
            // back
            app_state_next(MAINLOOP);
            return;
        }
    }
    if (timer_millis() - timeout_ms > 10000) {
        // timeout
        app_state_next(MAINLOOP);
    }
}
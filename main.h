/*         ___    ___
 *  __ ___|_  )__/ __| ___ _ _  ___ ___ _ _
 * / _/ _ \/ /___\__ \/ -_) ' \(_-</ _ \ '_|
 *_\__\___/___|  |___/\___|_||_/__/\___/_|_________________________________
 * CO₂ Sensor for Caving -- https://github.com/keppler/co2
 * Global declarations (application state)
 */

#ifndef _MAIN_H
#define _MAIN_H

#include <avr/pgmspace.h>

enum app_state_t {
    MAINLOOP,
    MENU
};

extern const char app_version[] PROGMEM;
void app_state_next(enum app_state_t next);
void app_wakeup(uint8_t initial);

#endif // _MAIN_H

/*         ___    ___
 *  __ ___|_  )__/ __| ___ _ _  ___ ___ _ _
 * / _/ _ \/ /___\__ \/ -_) ' \(_-</ _ \ '_|
 *_\__\___/___|  |___/\___|_||_/__/\___/_|_________________________________
 * CO₂ Sensor for Caving -- https://github.com/keppler/co2
 * Global declarations (application state)
 */

#ifndef _MAIN_H
#define _MAIN_H

enum app_state_t {
    MAINLOOP,
    MENU
};

void app_state_next(enum app_state_t next);

#endif // _MAIN_H

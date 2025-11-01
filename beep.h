/*         ___    ___
 *  __ ___|_  )__/ __| ___ _ _  ___ ___ _ _
 * / _/ _ \/ /___\__ \/ -_) ' \(_-</ _ \ '_|
 *_\__\___/___|  |___/\___|_||_/__/\___/_|_________________________________
 * CO₂ Sensor for Caving -- https://github.com/keppler/co2
 * Driver for sound transducer
 */

#ifndef _BEEP_H
#define _BEEP_H

typedef enum {
    BEEP_SHORT = 0,
    BEEP_RELAX = 1,
    BEEP_WARN = 2,
    BEEP_STARTUP = 3,
    BEEP_SHUTDOWN = 4,
} beep_t;

extern uint8_t beep_volume;
void beep_init(void);
void beep(const beep_t t);

#endif /* _BEEP_H */

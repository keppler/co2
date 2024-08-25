/*         ___    ___
 *  __ ___|_  )__/ __| ___ _ _  ___ ___ _ _
 * / _/ _ \/ /___\__ \/ -_) ' \(_-</ _ \ '_|
 *_\__\___/___|  |___/\___|_||_/__/\___/_|_________________________________
 * CO₂ Sensor for Caving -- https://github.com/keppler/co2
 * Timer utility (count milliseconds using interrupt)
 */

#ifndef _TIMER_H
#define _TIMER_H

#include <stdint.h>

void timer_init(void);
uint32_t timer_millis(void);

#endif // _TIMER_H

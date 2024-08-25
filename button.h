/*         ___    ___
 *  __ ___|_  )__/ __| ___ _ _  ___ ___ _ _
 * / _/ _ \/ /___\__ \/ -_) ' \(_-</ _ \ '_|
 *_\__\___/___|  |___/\___|_||_/__/\___/_|_________________________________
 * CO₂ Sensor for Caving -- https://github.com/keppler/co2
 * Debounced input button with short (50ms) and long (1s) detection
 * Copyright (c) 2024 Klaus Keppler - https://github.com/keppler/co2
 */

#ifndef _BUTTON_H
#define _BUTTON_H

#include <stdint.h>

void button_init(void);
void button_read(void);
uint8_t button_pressed(void);

#endif // _BUTTON_H

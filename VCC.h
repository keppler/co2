/*         ___    ___
 *  __ ___|_  )__/ __| ___ _ _  ___ ___ _ _
 * / _/ _ \/ /___\__ \/ -_) ' \(_-</ _ \ '_|
 *_\__\___/___|  |___/\___|_||_/__/\___/_|_________________________________
 * CO₂ Sensor for Caving -- https://github.com/keppler/co2
 * Read power supply voltage
 */

#ifndef _VCC_H
#define _VCC_H

#include <stdint.h>

uint16_t VCC_get(void);

#endif /* !_VCC_H */

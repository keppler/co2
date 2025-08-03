/*         ___    ___
 *  __ ___|_  )__/ __| ___ _ _  ___ ___ _ _
 * / _/ _ \/ /___\__ \/ -_) ' \(_-</ _ \ '_|
 *_\__\___/___|  |___/\___|_||_/__/\___/_|_________________________________
 * CO₂ Sensor for Caving -- https://github.com/keppler/co2
 * Driver for sound transducer
 */

#include <stdint.h>
#include <avr/pgmspace.h>
#include <util/delay.h>
#include "beep.h"

static const uint8_t melody[] PROGMEM = {
    6, 8, 12, 16, 30, 44,  /* index to melodies */
    250, 100,              /* short */
    220, 250, 250, 180,    /* relax */
    250, 180, 220, 250,    /* warn */
    250, 200, 220, 140, 190, 140, 0, 60, 250, 60, 0, 60, 250, 60,   /* startup */
    190, 200, 220, 140, 250, 140, 0, 60, 250, 60, 0, 60, 250, 60    /* shutdown */
};

void beep_init(void) {
    /* PB4: PWM with OC1B pin (PB4) */
    DDRB |= (1 << DDB4);
    PORTB &= ~(1 << PB4);
}

void beep(const beep_t t) {
    uint8_t len;
    TCCR1 = 1<<CS10;
    GTCCR = 1 << PWM1B | 1 << COM1B1;

    uint8_t pos;
    for (pos=pgm_read_byte(melody + t); pos < pgm_read_byte(melody + t + 1); pos+=2) {
        OCR1C = pgm_read_byte(melody + pos);
        OCR1B = OCR1C/2; /* OCR1C/2 for 50% duty cycle */
        for (len=pgm_read_byte(melody + pos + 1); len>0; len-=20) _delay_ms(20);
    }

    TCCR1 = 0;
    GTCCR = 0;
}

/*         ___    ___
 *  __ ___|_  )__/ __| ___ _ _  ___ ___ _ _
 * / _/ _ \/ /___\__ \/ -_) ' \(_-</ _ \ '_|
 *_\__\___/___|  |___/\___|_||_/__/\___/_|_________________________________
 * CO₂ Sensor for Caving -- https://github.com/keppler/co2
 * Timer utility (count milliseconds using interrupt)
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include "timer.h"

static uint64_t _millis = 0;
#if F_CPU == 8000000
static uint16_t _cnt = 0;
#endif

ISR(TIM0_COMPA_vect) {
#if F_CPU == 1000000
    _millis++;
#elif F_CPU == 8000000
    if (++_cnt >= 8) {
        _cnt = 0;
        _millis++;
    }
#else
    #error F_CPU value not supported
#endif
}

void timer_init(void) {
    // interrupt every 1024th clock cycle
    // 1MHz: 1,000,000 Hz / 1024 = 976.5625 Hz = 1.024 ms
    // 8MHz: 8,000,000 Hz / 1024 = 7812.5 Hz = 0.128 ms
    // TCCR0x: CTC mode, set prescaler to 1024
    TCCR0A = 1<<WGM01;
    TCCR0B = 1<<CS02 | 1<<CS00;
    OCR0A = 0x00;

    // enable timer compare interrupt
    TIMSK = 1<<OCIE0A;

    sei();
}

void timer_reset(void) {
    _millis = 0;
}

uint32_t timer_millis(void) {
    uint64_t m;
    cli();
    m = _millis;
    sei();
    return m;
}

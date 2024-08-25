/*         ___    ___
 *  __ ___|_  )__/ __| ___ _ _  ___ ___ _ _
 * / _/ _ \/ /___\__ \/ -_) ' \(_-</ _ \ '_|
 *_\__\___/___|  |___/\___|_||_/__/\___/_|_________________________________
 * CO₂ Sensor for Caving -- https://github.com/keppler/co2
 * Debounced input button with short (50ms) and long (1s) detection
 * Copyright (c) 2024 Klaus Keppler - https://github.com/keppler/co2
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include "button.h"
#include "timer.h"

#define BUTTON_DEBOUNCE_DELAY 50
#define BUTTON_LONG 1000

#define BTN_PIN PB1

static uint32_t debounce = 0;
static uint32_t startPress = 0;
static uint8_t last_state = (1 << BTN_PIN); // initialize with HIGH
static uint8_t pressed = 0;
static uint8_t state = (1 << BTN_PIN); // initialize with HIGH

ISR(PCINT0_vect) {
    // (pseudo) interrupt handler to wake up device from sleep mode
}

void button_init(void) {
    // initialize button
    DDRB &= ~(1 << DDB1);       // set port mode to INPUT
    PORTB |= (1 << BTN_PIN);    // enable pull-up

    GIMSK |= (1 << PCIE);       // enable pin change interrupts
    PCMSK |= (1 << BTN_PIN);    // enable PCINTx interrupt
}

void button_read(void) {
    uint8_t r = PINB & (1 << BTN_PIN);
    if (r != last_state) {
        debounce = timer_millis();
    }
    if ((timer_millis() - debounce) > BUTTON_DEBOUNCE_DELAY) {
        if (r != state) {
            state = r;
            if (state == 0) {
                // button pressed
                startPress = timer_millis();
            } else {
                // button released
                if (startPress > 0) pressed = 1;
            }
        } else if (state == 0 && startPress > 0 && timer_millis() - startPress >= BUTTON_LONG) {
            pressed = 2;
            startPress = 0;
        }
    }
    last_state = r;
}

uint8_t button_pressed(void) {
    uint8_t ret = pressed;
    pressed = 0;
    return ret;
}

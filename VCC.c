#include <avr/io.h>
#include <util/delay.h>
#include "VCC.h"

/*
 * Get Battery Voltage
 * SOURCES:
 * - https://wp.josh.com/2014/11/06/battery-fuel-guage-with-zero-parts-and-zero-pins-on-avr/
 * - Microchip Application Note AN2447 (https://ww1.microchip.com/downloads/en/Appnotes/00002447A.pdf)
 */
uint16_t VCC_get(void) {
	/*
	By default, the successive approximation circuitry requires an input clock frequency between 50
	kHz and 200 kHz to get maximum resolution.
	*/

	// Enable ADC, set prescaller to /8 which will give an ADC clock of 1mHz/8 = 125kHz
	ADCSRA = _BV(ADEN) | _BV(ADPS1) | _BV(ADPS0);

	/* Select ADC inputs */
#if defined(__AVR_ATtiny44__) || defined(__AVR_ATtiny84__)
    // BITS:  76543210
    // REFS = 00       = Vcc used as Vref
	// MUX  =   100001 = Single ended, 1.1V (Internal Ref) as Vin
	ADMUX = 0x21;   // 0b00100001 for ATtinyX4
#elif defined(__AVR_ATtiny45__) || defined(__AVR_ATtiny85__)
    // BITS:  76543210
    // REFS = 00X0     = Vcc used as Vref
    // MUX  =     1100 = Single ended, 1.1V (Internal Ref) as Vin
    ADMUX = 0x0C;   // 0b00001100 for ATtinyX5
#else
#error Target not supported
#endif

	/*
		After switching to internal voltage reference the ADC requires a settling time of 1ms before
		measurements are stable. Conversions starting before this may not be reliable. The ADC must
		be enabled during the settling time.
	*/
	_delay_ms(1);
				
	/*
		The first conversion after switching voltage source may be inaccurate, and the user is advised to discard this result.
	*/
	ADCSRA |= _BV(ADSC);				// Start a conversion

	while( ADCSRA & _BV( ADSC) ) ;		// Wait for 1st conversion to be ready...
										// ... and ignore the result

	/*
		After the conversion is complete (ADIF is high), the conversion result can be found in the ADC
		Result Registers (ADCL, ADCH).		
		
		When an ADC conversion is complete, the result is found in these two registers.
		When ADCL is read, the ADC Data Register is not updated until ADCH is read.		
	*/
	
	// Note we could have used ADLAR left adjust mode and then only needed to read a single byte here
		
	uint8_t low  = ADCL;
	uint8_t high = ADCH;

	uint16_t adc = (high << 8) | low;		// 0<= result <=1023
			
	// Compute a fixed point with 1 decimal place (i.e. 5v= 50)
	//
	// Vcc   =  (1.1v * 1024) / ADC
	// Vcc10 = ((1.1v * 1024) / ADC ) * 10				->convert to 1 decimal fixed point
	// Vcc10 = ((11   * 1024) / ADC )				->simplify to all 16-bit integer math
				
	uint16_t vccx100 = (uint16_t) ( (110L * 1024L) / adc);
	
	/*	
		Note that the ADC will not automatically be turned off when entering other sleep modes than Idle
		mode and ADC Noise Reduction mode. The user is advised to write zero to ADEN before entering such
		sleep modes to avoid excessive power consumption.
	*/
	
	ADCSRA &= ~_BV( ADEN );			// Disable ADC to save power
	
	return( vccx100 );
}

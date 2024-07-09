#include <avr/io.h>
#include <stdint.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#include "serial.h"

/**
 * @file
 *
 * Implementation of serial transmission (8-N-1) at any host supported rate,
 * using Timer1 and an interrupt, allowing the main program to continue
 * running during communication.
 *
 * This is the preferred serial implementation because it has relatively
 * precise control of bit timing (within about 0.5%), especially at high speeds
 * (230,400 bps is comfortably supported).
 * A simple calibration step will tune this delay to the particular chip in use.
 * Jitter in bit times is minimised as the interrupt is timer driven.
 */

enum {
    // States 0..7 indicate the data bit index (0 = LSB) that should
    // define the line level when the next timer interrupt occurs
    STOP_BIT = 8, // Timer interrupt will set line high
    SENDING_STOP,
    IDLE // Indicates transmission is complete and line is ready for new transmission
};

/**
 * The data byte being transmitted.
 */
static volatile uint8_t data;

/**
 * Represents the next "thing to do" when the timer interrupt
 * is serviced. E.g. state = 0 means "send bit zero (LSB) of
 * current data byte".
 */
static volatile uint8_t state = IDLE;

// From ATtiny25/45/85 [DATASHEET] page 89,
// Table 12-5, Timer/Counter1 Prescale Select
enum {
    NO_PRESCALE      = 0b0001,
    PRESCALE_BY_2    = 0b0010,
    PRESCALE_BY_4    = 0b0011,
    PRESCALE_BY_8    = 0b0100,
    PRESCALE_BY_32   = 0b0110,
    PRESCALE_BY_256  = 0b1001,
    PRESCALE_BY_1024 = 0b1011
};

void serial_timer_init() {
    // run Timer1 in async mode, 64MHz clock source.

    // 300 bps:    /1024 prescale: Measured 202..223; mid 212
    // 1200 bps:   /256 prescale: Measured 202..223; mid 212
    // 9600 bps:   /32 prescale: Calculated delay is 207 (64M / 9600 / 32 - 1)
    //             Measured delay on my TINY85 board / Polulu serial: 201..223. mid: 212
    //             Measured with TINY85 + PL2303 USB/serial: 204..226. mid: 215
    // 38400 bps:  /8 prescale: Calculated delay is 207 (64M / 38400 / 8 - 1)
    //             Measured: 200..222; mid 211
    // 115200 bps: /4 prescale: Calculated delay is 137 (64M / 115200 / 4 - 1)
    //             Measured 131..144; mid 137
    //             Measured with TINY85 + PL2303 USB/serial: 133..148; mid 140
    // 230400 bps: /2 prescale. Measured: 129..142; mid 135
    // higher rates don't seem to be easily possible in OS X


    // "To set Timer/Counter1 in asynchronous mode
    //  first enable PLL and then wait 100 μs for PLL to stabilize.
    //  Next, poll the PLOCK bit until it is set
    //  and then set the PCKE bit."

    PLLCSR = 0b00000010; // enable PLL in async high speed mode (64MHz)
    _delay_us(100);
    while(!(PLLCSR & 1))
        ; // wait until PLOCK set
    PLLCSR |= 0b100; // set PCKE (PCK Enable)

    // "In PWM mode, the Timer Overflow Flag - TOV1 is set
    //  when the TCNT1 counts to the OCR1C value
    //  and the TCNT1 is reset to $00. The Timer Overflow Interrupt1
    //  is executed when TOV1 is set provided that Timer Overflow
    //  Interrupt and global interrupts are enabled.
    //  This also applies to the Timer Output Compare flags
    //  and interrupts."

    // PWM mode must be enabled:
    TCCR1 = (1 << PWM1A) | PRESCALE_BY_32; // CHANGE THIS TO CORRECT PRESCALER for desired data rate
    GTCCR = 0;
    OCR1C = 212; // CHANGE THIS TO TESTED TIMER LIMIT for given data rate
    TIMSK = 1 << TOIE1; // TOIE1 (Timer/Counter1 Overflow) int enabled

    state = IDLE;
    SERIAL_HI();

    sei();
}

ISR(TIMER1_OVF_vect) {
    // Transmit the stop bit or next data bit
    if (state >= STOP_BIT || (data & 1)) {
        SERIAL_HI();
    } else {
        SERIAL_LO();
    }

    if (state < IDLE) {
        ++state;
        data >>= 1;
    } else {
        TIMSK &= ~(1 << TOIE1); // transmit done. disable timer interrupt
    }
}

/**
 * Spinwaits until serial line is idle, then begins
 * transmission of `c` as 8 bits. Function returns immediately,
 * does not wait for transmission to occur.
 */
void sendt(uint8_t c) {
    while (state != IDLE)
        ;

    TCNT1 = 0; // start counting
    SERIAL_LO(); // start bit
    // bits are sent LSB to MSB
    data = c;
    state = 0;

    TIMSK |= 1 << TOIE1; // enable timer interrupt
}

void serial_timer_delay_test() {
   // try different values of delay to find the range that works

   for(uint8_t d = 1; d; ++d){
      PORTB ^= 0b10; // toggle LED

      OCR1C = d; // Update timer interrupt frequency

      sendt('.'); sendt('o'); sendt('O'); sendt('(');
      sendt('0'+(d/1000));
      sendt('0'+((d/100)%10));
      sendt('0'+((d/10)%10));
      sendt('0'+(d%10));
      sendt(')');
      sendt('\r'); sendt('\n');
   }
}

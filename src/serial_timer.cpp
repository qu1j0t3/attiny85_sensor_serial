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

// If 1, then the timer interrupt will always be enabled
// If 0, then interrupt is only enabled as long as necessary (during serial transmission)
//       -- this mode is buggy, results in garbled serial output, not clear why (FIXME)
#define INT_ALWAYS 1

enum {
    // States 0..7 indicate the data bit index (0 = LSB) that should
    // define the line level when the next timer interrupt occurs
    STOP_BIT = 8, // Next timer interrupt will set line high
    SENDING_STOP, // Holds off any line change until the stop bit is complete
    IDLE // Indicates transmission is complete and line is ready for new transmission
};

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

// Example measured timer settings using my Digispark and serial interface:
//
//        Bps   Prescale           Timer
//     ------   ----------------   -----
//        300   PRESCALE_BY_1024   212
//       1200   PRESCALE_BY_256    212
//       9600   PRESCALE_BY_32     214
//      38400   PRESCALE_BY_8      211
//     115200   PRESCALE_BY_4      137
//     230400   PRESCALE_BY_2      135
//
// Calibration is done by running "chirp" test (serial_timer_delay_test()),
// and taking midpoint of first and last uncorrupted timer values received.

/**
 * Set to correct prescale ratio for the desired baud rate
 * according to calibration test.
 */
static const uint8_t prescale = PRESCALE_BY_32;

/**
 * Set to timer count for the desired baud rate
 * according to calibration test.
 */
static const uint8_t divider = 214;

void serial_timer_init() {
    // run Timer1 in async mode, 64MHz clock source.

    // "To set Timer/Counter1 in asynchronous mode
    //  first enable PLL and then wait 100 μs for PLL to stabilize.
    //  Next, poll the PLOCK bit until it is set
    //  and then set the PCKE bit."

    PLLCSR = 1 << PLLE; // enable PLL in async high speed mode (64MHz)

    _delay_us(100);

    while(!(PLLCSR & (1 << PLOCK)))
        ; // wait until PLL is locked

    PLLCSR |= 1 << PCKE;

    TCCR1 = (1 << PWM1A) | prescale;
    GTCCR = 0;
    OCR1C = divider;

    state = IDLE;
    SERIAL_HI();

    TIMSK = INT_ALWAYS*(1 << TOIE1); // Timer/Counter1 Overflow interrupt

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
    } else if (! INT_ALWAYS) {
        TIMSK &= ~(1 << TOIE1); // transmit done. disable timer interrupt
    }
}

/**
 * Wait until serial communication is finished
 * and line is idle. At this point the timer interrupt
 * will be disabled.
 */
void flush_serial() {
    while (/*(TIMSK & (1 << TOIE1)) &&*/ state != IDLE)
        ;
}

/**
 * Spinwaits until serial line is idle, then begins
 * transmission of `c` as 8 bits. Function returns immediately,
 * does not wait for transmission to occur.
 *
 * TODO: Using this function only allows limited concurrency (after calling sendt(),
 *       main program will block at the next call to sendt(), until first is done).
 *       Need to implement a buffer that can be filled at once and consumed by ISR,
 *       so that the main program continues during the entire buffer transmission.
 */
void sendt(uint8_t c) {
    flush_serial();

    TCNT1 = 0;   // start counting
    SERIAL_LO(); // begin the start bit
    data = c;    // bits of `data` follow,
    state = 0;   // bit zero is next

    if (! INT_ALWAYS) TIMSK |= 1 << TOIE1; // enable timer interrupt
}

void serial_timer_delay_test() {
   // try different values of delay to find the range that works

   for(uint8_t d = 1; d; ++d){
      TOGGLE_LED();

      OCR1C = d; // Update timer interrupt frequency

      sendt('.'); sendt('o'); sendt('O'); sendt('(');
      sendt('0'+((d/100)%10));
      sendt('0'+((d/10)%10));
      sendt('0'+(d%10));
      sendt(')');
      sendt('\r'); sendt('\n');
   }

   OCR1C = divider; // reset to configured value
}

#include <avr/io.h>
#include <stdint.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#include "serial.h"

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
volatile uint8_t data;

/**
 * Represents the next "thing to do" when the timer interrupt
 * is serviced. E.g. state = 0 means "send bit zero (LSB) of
 * current data byte".
 */
volatile uint8_t state = IDLE;

// From table 12-5, Timer/Counter1 Prescale Select
#define PRESCALE_BY_32 0b0110
#define PRESCALE_BY_8  0b0100
#define PRESCALE_BY_4  0b0011


void serial_timer_init() {
    // run Timer1 in async mode, 64MHz clock source,
    // for 115200 bps, this means 555.555 ticks per bit,
    // and a /4 prescaler brings this to 138.888 counts per interrupt.
    // for 9600 bps, a /32 prescaler results in 208.333 counts per interrupt.

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

    // PWM mode must be enabled for the Timer Interrupt
    // to occur when timer reaches OCR1C.
    TCCR1 = 0b01000000 | PRESCALE_BY_4;
    GTCCR = 0;
    // 9600 bps: /32 prescale: Calculated delay is 207 (64M / 9600 / 32 - 1)
    //           Measured delay on my TINY85 board: low 201, high 223. mid: 212
    // 38400 bps: /8 prescale: Calculated delay is 207 (64M / 38400 / 8 - 1)
    //            Measured: 200..222; mid 211
    // 115200 bps: /4 prescale: Calculated delay is 137 (64M / 115200 / 4 - 1)
    //             Measured 131..144; mid 137
    OCR1C = 137;
    TIMSK = 0b00000100; // TOIE1 (Timer/Counter1 Overflow) int enabled

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
        data >>= 1;
        ++state;
    }

    //static uint8_t ctr;
    //if (! ctr++) TOGGLE_LED();
}

/**
 * Spinwaits until serial line is idle, then begins
 * transmission of `c` as 8 bits, using Timer1.
 */
void sendt(uint8_t c) {
    while (state != IDLE)
        ;

    TCNT1 = 0; // start counting
    SERIAL_LO(); // start bit
    // bits are sent LSB to MSB
    data = c;
    state = 0;
}

void serial_timer_delay_test() {
   // try different values of delay to find the range that works

   for(uint8_t d = 1;d;){
      PORTB ^= 0b10; // toggle LED

      OCR1C = d; // Update timer interrupt frequency

      sendt('.'); sendt('o'); sendt('O'); sendt('(');
      sendt('0'+(d/1000));
      sendt('0'+((d/100)%10));
      sendt('0'+((d/10)%10));
      sendt('0'+(d%10));
      sendt(')');
      sendt('\r'); sendt('\n');

      ++d;
   }
}



#include <avr/io.h>
#include <stdint.h>
#include <util/delay.h>

#include "serial.h"

// N.B. clock is 16.5MHz
//uint16_t d = 421; // for 9600. range 400..443. measured by testing - see serial_test()
//uint16_t d = 3408; // for 1200. range 3231..3585. measured by testing - see serial_test(). computed
//uint16_t d = 101; // for 38400. range 96..107 measured by testing
static uint16_t d = 31; // for 115200. range 30..33 measured by testing

// It's possible to get a 3 cycle loop if k were 8 bits
void delay4(uint16_t k) {
   while(k--) { // 4 cycles per iteration?
      __builtin_avr_nops(0);
   }
}

void send(uint8_t b) {
   SERIAL_LO(); // Start bit
   delay4(d);

   for(uint8_t i = 0; i < 8; ++i) {
      if(b & 1) {
         SERIAL_HI();
      } else {
         SERIAL_LO();
      }
      b >>= 1;
      delay4(d);
   }

   SERIAL_HI(); // Stop bit
   delay4(d);
}

void serial_delay_test() {
   // try different values of delay to find the range that works
   // on my Digispark Tiny, the working range is 201..222 inclusive.

   for(d = 0;d < 40000;){ // 110 baud would be a bit time of 4 x 37500 cycles so this limit is safe
      PORTB ^= 0b10; // toggle LED

      send('.'); send('o'); send('O'); send('(');
      send('0'+(d/1000));
      send('0'+((d/100)%10));
      send('0'+((d/10)%10));
      send('0'+(d%10));
      send(')');
      send('\r'); send('\n');

      ++d;
      _delay_ms(500);
   }
}

#include <avr/io.h>
#include <stdint.h>
#include <util/delay.h>

#include "serial.h"

// N.B. clock is 16.5MHz
static uint16_t d = 423; // for 9600. range 401..445. measured by testing - see serial_test()
//static uint16_t d = 101; // for 38400. range 96..107 measured by testing
//static uint16_t d = 31; // for 115200. range 30..33 measured by testing

// It's possible to get a 3 cycle loop if k were 8 bits
void delay4(uint16_t k) {
   while(k--) { // 4 cycles per iteration?
      __builtin_avr_nops(0);
   }
}

void sendstr(char *str) {
   while(*str) {
      send(*str++);
   }
}

void send(uint8_t b) {
   SERIAL_LO(); // Start bit
   delay4(d);

   for(uint8_t i = 8; i--;) {
      /*
      if(b & 1) {     // if() construct introduces jitter because
         SERIAL_HI(); // 1 bit takes 7 cycles and 0 bit takes 4 cycles
      } else {        // so worst case difference is 8 x 3 = 24 cycles
         SERIAL_LO();
      }*/
      PORTB = (PORTB & ~RXMASK) | ((b & 1) * RXMASK); // compiles to deterministic sbrc,sbi,sbrs,cbi
      b >>= 1;
      delay4(d);
   }

   SERIAL_HI(); // Stop bit
   delay4(d);
}

void serial_delay_test() {
   // try different values of delay to find the range that works

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

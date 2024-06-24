#include <avr/io.h>
#include <stdint.h>

#include "usi_i2c_master.h"

/*
 ┏━━━━━━━━━━━┓
 ┃ 2   4   6 ┃  ISP header socket
 ┃ 1   3   5 ┃  on Digispark carrier protoboard
 ┗━━━     ━━━┛
 See: https://www.pololu.com/docs/0J36/all
 1:       = MISO        = Target pin 1
 2: Red   = Target VDD 5V
 3: Green = SCK         = pin 2
 4: Black = MOSI/SDA    = pin 0
 5:       = _RST = pin 5
 6: White = GND

 ┏━━━━━━━━━━━┓
 ┃ R  Blk  W ┃  Sensirion
 ┃ -   G   - ┃  connections
 ┗━━━     ━━━┛
 */

// I/O Map
// PB5: _RESET; avoid for i/o
// PB4:
// PB3: ISP RX (to computer)
// PB2: sensor SCK
// PB1:
// PB0: sensor SDA

enum {
   SCD41_ADDRESS = 0x62
};

static char i2c_get_serial_number[] = {
   (SCD41_ADDRESS << 1) | 1, // ??? low bit 0 = master write, otherwise master read; see USI_I2C_Master_Transceiver_Start()
   0x36, 0x82
};

#define TOGGLE_LED() if(1){ PORTB ^= 0b10; }

#define RXMASK 0b1000

// N.B. clock is 16.5MHz
//uint16_t d = 421; // for 9600. range 400..443. measured by testing - see serial_test()
//uint16_t d = 3408; // for 1200. range 3231..3585. measured by testing - see serial_test(). computed
//uint16_t d = 101; // for 38400. range 96..107 measured by testing
uint16_t d = 30; // for 115200. range 29..31 measured by testing

// It's possible to get a 3 cycle loop if k were 8 bits
void delay4(uint16_t k) {
   while(k--) { // 4 cycles per iteration?
      __builtin_avr_nops(0);
   }
}

#define SERIAL_LO() if(1){ PORTB &= ~RXMASK; }
#define SERIAL_HI() if(1){ PORTB |= RXMASK; }

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

   for(;d < 40000;){ // 110 baud would be a bit time of 4 x 37500 cycles so this limit is safe
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

void serial_stream_test() {
   uint8_t jsf8(void);

   while(1) {
      for(uint8_t i = 16; i--;) {
         uint8_t b = jsf8();

         send('0'+(b/100));
         send('0'+((b/10)%10));
         send('0'+(b%10));
         send(',');
      }
      send('\r'); send('\n');
      TOGGLE_LED();
   }
}

int main() {
   DDRB = 0b1010; // set serial RX and LED pin to output
   SERIAL_HI(); // set line idle
   _delay_ms(200);

   char buf[9] = { (SCD41_ADDRESS << 1) | 0 /*read*/ };

   //USI_I2C_Master_Start_Transmission(i2c_get_serial_number, sizeof(i2c_get_serial_number));

   //USI_I2C_Master_Start_Transmission(buf, sizeof(buf));

   send('O'); send('K'); send('.'); send('\r'); send('\n');

#ifndef ARDUINO_avrdd
#endif
}
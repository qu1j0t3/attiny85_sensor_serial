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

#define RXMASK 0b1000

// N.B. clock is 16.5MHz
uint8_t d = 211; // discovered by testing - see serial_test()

void delay(uint8_t k) {
   while(k--) { // target 8 cycles per iteration
      __builtin_avr_nops(3); // + about 5 cycles of loop overhead
   }
}

#define SERIAL_LO() if(1){ PORTB &= ~RXMASK; }
#define SERIAL_HI() if(1){ PORTB |= RXMASK; }

void send(uint8_t b) {
   SERIAL_LO(); // Start bit
   delay(d); // 9600bps: 104.17µs per bit

   for(uint8_t i = 0; i < 8; ++i) {
      if(b & 1) {
         SERIAL_HI();
      } else {
         SERIAL_LO();
      }
      b >>= 1;
      delay(d);
   }

   SERIAL_HI(); // Stop bit
   delay(d);
}

void serial_test() {
   // try different values of delay to find the range that works
   // on my Digispark Tiny, the working range is 201..222 inclusive.

   for(d=1;d;){
      PORTB ^= 0b10; // toggle LED
      for(uint8_t i = 0; i < 2; ++i) {
         send('.'); send('.'); send('.'); send('(');
         send('0'+(d/100));
         send('0'+((d/10)%10));
         send('0'+(d%10));
         send(')');
         send('\r'); send('\n');
         _delay_us(500);
      }
      ++d;
      _delay_ms(500);
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
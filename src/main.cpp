#include <avr/io.h>
#include <util/delay.h>
#include <stdint.h>
//#include <stdlib.h>
#include <stddef.h>

#include <TinyWireM.h>

#include "serial.h"

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

 Header pins below carrier protoboard;
 seen from top, with ISP header upwards

 How to connect PL2303 USB adapter (flying leads, White, Black, Red, Green)
 [NC] [NC] [NC] [White] [Black] [NC]
 --
 Corresponding to Polulu PGM03A ISP pins:
 [A]  [B]  [TX] [RX]    [GND]   [VBUS (+5V)] ; "TX" & "RX" relative to host computer
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

inline void TOGGLE_LED() {
   PINB |= 1 << PINB1;
}

/**
 * Generates an endless stream of 8 bit random numbers,
 * and prints character by character as ASCII lines of 16 numbers,
 * using the provided function.
 */
void serial_stream_test(void (*send_byte_func)(uint8_t)) {
   uint8_t jsf8(void);

   send_byte_func('O');
   send_byte_func('K');
   send_byte_func('.');
   send_byte_func('\r');
   send_byte_func('\n');

   while(1) {
      for(uint8_t i = 16; i--;) {
         uint8_t b = jsf8();

         send_byte_func('0'+(b/100));
         send_byte_func('0'+((b/10)%10));
         send_byte_func('0'+(b%10));
         send_byte_func(',');
      }
      send_byte_func('\r');
      send_byte_func('\n');

      TOGGLE_LED();
   }
}

int main() {
   DDRB = 0b1010; // set serial RX and LED pin to output

   serial_timer_init();

   //_delay_ms(200); // Wait a little bit before using serial output

   //serial_timer_delay_test();
   //serial_stream_test(sendt);

    sendt('I');
    sendt('2');
    sendt('C');
    sendt(':');
    sendt('\r');
    sendt('\n');

    static USI_TWI TinyWireM; // N.B. We cannot use C++ `new` on AVR

    TinyWireM.begin();

   while(1) {
      TinyWireM.beginTransmission(SCD41_ADDRESS);
      TinyWireM.send(0x36);
      TinyWireM.send(0x82);
      TinyWireM.endTransmission();

      _delay_ms(2);

      TinyWireM.requestFrom(SCD41_ADDRESS, sizeof(9));

      for(uint8_t i = 0; i < 9; ++i) {
         uint8_t b = TinyWireM.receive();          // get the temperature
         sendt('0'+(b/100));
         sendt('0'+((b/10)%10));
         sendt('0'+(b%10));
         sendt(',');
      }
      sendt('\r');
      sendt('\n');

      _delay_ms(1000);
   }

#ifndef ARDUINO_avrdd
#endif
}
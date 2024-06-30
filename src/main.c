#include <avr/io.h>
#include <stdint.h>

#include "serial.h"
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
   SERIAL_HI(); // set line idle
   _delay_ms(200);

   char buf[9] = { (SCD41_ADDRESS << 1) | 0 /*read*/ };

#if SERIAL_DELAY
   //serial_delay_test();

   sendstr("OK.\r\n");

   serial_stream_test();
#else
   serial_timer_init();
   //serial_timer_delay_test();
   serial_stream_test(sendt);
#endif


   USI_I2C_Master_Start_Transmission(i2c_get_serial_number, sizeof(i2c_get_serial_number));

   USI_I2C_Master_Start_Transmission(buf, sizeof(buf));


#ifndef ARDUINO_avrdd
#endif
}
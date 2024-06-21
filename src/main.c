#include <avr/io.h>
#include <stdint.h>

#include "usi_i2c_master.h"

/*
 ┏━━━━━━━━━━━┓
 ┃ 2   4   6 ┃  ISP header socket
 ┃ 1   3   5 ┃  on Digispark carrier protoboard
 ┗━━━     ━━━┛
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
// PB3:
// PB2: SCK: serial clock input
// PB1: MISO: SPI Host Data Input / Client Data Output
// PB0: MOSI: SPI Host Data Output / Client Data Input

enum __attribute__ ((__packed__)) {
   SCD41_ADDRESS = 0x62
};

static char i2c_get_serial_number[] = {
   (SCD41_ADDRESS << 1) | 1, // ??? low bit 0 = master write, otherwise master read; see USI_I2C_Master_Transceiver_Start()
   0x36, 0x82
};

int main() {
   char buf[9] = { (SCD41_ADDRESS << 1) | 0 /*read*/ };

   USI_I2C_Master_Start_Transmission(i2c_get_serial_number, sizeof(i2c_get_serial_number));

   USI_I2C_Master_Start_Transmission(buf, sizeof(buf));


   for(;;);

   #ifndef ARDUINO_avrdd
#endif
}
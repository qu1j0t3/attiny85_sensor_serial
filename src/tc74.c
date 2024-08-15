#include <stdint.h>

#include "fleury_i2cmaster/i2cmaster.h"
#include "serial.h"

enum {
   // Note TC74 part # : TC74XX-YYZAA
   //    where XX is the address code, e.g. A0 = 0b1001000
   TC74_ADDRESS = 0b1001000
};// i2c_addresses;

enum {
  TC74_RTR_COMMAND = 0,
  TC74_RWCR_COMMAND
};

enum {
   TC74_NORMAL_MODE = 0,
   TC74_STANDBY_MODE = 1<<7, // bit written to CR
   TC74_DATA_READY = 1<<6 // bit read from CR
};

void tc74_normal_mode() {
   uint8_t ret = i2c_start((TC74_ADDRESS << 1) | I2C_WRITE);       // set device address and write mode
   if ( ret ) { // failed to issue start condition, possibly no device found
      sendnum('A', ret);
   } else {// issuing start condition ok, device accessible
      i2c_write(TC74_RWCR_COMMAND);
      i2c_write(TC74_NORMAL_MODE); // turn off Standby mode
   }
   i2c_stop();
}

void tc74_temp() {
    uint8_t ret = i2c_start((TC74_ADDRESS << 1) | I2C_WRITE);       // set device address and write mode

    if ( ret ) { // failed to issue start condition, possibly no device found
        i2c_stop();
        sendnum('B', ret);
    } else {// issuing start condition ok, device accessible
        i2c_write(TC74_RWCR_COMMAND);
        i2c_stop();

        i2c_start((TC74_ADDRESS << 1) | I2C_READ);     // set device address and write mode
        uint8_t cr = i2c_readNak();                    // read one byte
        i2c_stop();

        if (cr & TC74_DATA_READY) {
            uint8_t ret = i2c_start((TC74_ADDRESS << 1) | I2C_WRITE);       // set device address and write mode

            if ( ret ) { // failed to issue start condition, possibly no device found
                    i2c_stop();
                    sendnum('C', ret);
            } else { // issuing start condition ok, device accessible
                    i2c_write(TC74_RTR_COMMAND);
                    i2c_stop();

                    i2c_start((TC74_ADDRESS << 1) | I2C_READ);     // set device address and write mode
                    uint8_t temp = i2c_readNak();                    // read one byte
                    i2c_stop();

                    sendnum('T', temp);
            }
        }
    }
}
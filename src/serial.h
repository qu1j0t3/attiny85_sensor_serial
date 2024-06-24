#ifndef __SERIAL_H__
#define __SERIAL_H__

#include <avr/io.h>

#define RXMASK 0b1000

#define SERIAL_LO() if(1){ PORTB &= ~RXMASK; }
#define SERIAL_HI() if(1){ PORTB |= RXMASK; }

void send(uint8_t b);
void serial_delay_test();

#endif

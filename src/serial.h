#ifndef SERIAL_H_
#define SERIAL_H_

#include <avr/io.h>

#define RXMASK 0b1000

static inline void SERIAL_LO() {
    PORTB &= ~RXMASK;
}

static inline void SERIAL_HI() {
    PORTB |= RXMASK;
}

void send(uint8_t b);
void sendstr(char *str);
void serial_delay_test();

void serial_timer_init();
void serial_timer_delay_test();
void sendt(uint8_t b);
void flush_serial();

void TOGGLE_LED();

#endif // SERIAL_H_

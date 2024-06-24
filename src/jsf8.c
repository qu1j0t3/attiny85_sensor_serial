#include <stdint.h>

#define rot8(x,k) (((x) << (k))|((x) >> (8 - (k))))
uint8_t jsf8(void) {
	static uint8_t a = 0xf1;
	static uint8_t b = 0xee, c = 0xee, d = 0xee;

	uint8_t e = a - rot8(b, 1);
	a = b ^ rot8(c, 4);
	b = c + d;
	c = d + e;
	return d = e + a;
}

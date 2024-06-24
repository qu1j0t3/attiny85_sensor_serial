#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

int main(int argc, char *argv[]) {

   uint8_t jsf8(void);

   unsigned n = argc > 1 ? atoi(argv[1]) : 0;

   for(; argc <= 1 || n; --n) {
      for(unsigned i = 16; i--;) {
         unsigned b = jsf8();
         printf("%03d,", b);
      }
      putchar('\n');
   }

   return EXIT_SUCCESS;
}
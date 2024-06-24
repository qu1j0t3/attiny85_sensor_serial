BIN = ~/.platformio/packages/toolchain-atmelavr/bin/

.PHONY : test dump

dump : .pio/build/tiny13a/firmware.elf
	$(BIN)/avr-objdump -d -S $<

.pio/build/tiny13a/firmware.elf : platformio.ini src/main.c
	platformio build


stream_test : stream_test.o src/jsf8.o
	$(CC) -o $@ $^

test : stream_test
	./stream_test 10
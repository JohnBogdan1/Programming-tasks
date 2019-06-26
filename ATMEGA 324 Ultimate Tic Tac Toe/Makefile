.PHONY: all clean

all: main.hex

main.hex: main.elf
	avr-objcopy  -j .text -j .data -O ihex main.elf main.hex
	avr-size main.elf

main.elf: *.c
	avr-gcc -mmcu=atmega324a -DF_CPU=16000000 -Wno-deprecated-declarations -Wall -Wextra -Os -o $@ $^

clean:
	rm -rf main.elf main.hex

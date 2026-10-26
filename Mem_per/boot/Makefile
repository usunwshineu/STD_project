# Boot loader for the 8 KB BRAM ROM -> main.hex
# (icestorm/Makefile copies it to code.hex and patches it in with icebram)
# Uses the drivers in ../c/ unchanged.

CROSS = riscv64-unknown-elf-
CC = $(CROSS)gcc
OBJCOPY = $(CROSS)objcopy
OBJDUMP = $(CROSS)objdump
SIZE = $(CROSS)size
HEXDUMP = hexdump
HEXDUMP_ARGS = -v -e '1/4 "%08x" "\n"'

C = ../c
CFLAGS = -Wall -Os -march=rv32i_zicsr -mabi=ilp32 -ffreestanding -nostartfiles -nostdlib \
         -ffunction-sections -fdata-sections -I$(C) -I/usr/lib/picolibc/riscv64-unknown-elf/include

SOURCES = start.S boot.c $(C)/acia.c $(C)/spi.c $(C)/flash.c

all: main.hex

main.elf: lnk-boot.lds $(SOURCES) $(wildcard $(C)/*.h)
	$(CC) $(CFLAGS) -Wl,--gc-sections,-T,lnk-boot.lds,-Map,boot.map -o $@ $(SOURCES) -lgcc
	$(SIZE) $@

main.bin: main.elf
	$(OBJCOPY) -O binary -j .text -j .data $< $@

main.hex: main.bin
	$(HEXDUMP) $(HEXDUMP_ARGS) $< > $@

disassemble: main.elf
	$(OBJDUMP) -d $< > main.dis

clean:
	rm -f *.bin *.hex *.elf *.dis *.map

.PHONY: all clean disassemble

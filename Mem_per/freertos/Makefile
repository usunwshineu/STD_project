# FreeRTOS application -> app.img, loaded from flash @ 128 KB by ../boot
#
#   make            build app.img
#   make clean
# Writing it to flash: see icestorm/Makefile (make freertos / flash_freertos).
#
# Needs the kernel next to this folder:
#   cd .. && git clone https://github.com/FreeRTOS/FreeRTOS-Kernel

CROSS    = riscv64-unknown-elf-
CC       = $(CROSS)gcc
OBJCOPY  = $(CROSS)objcopy
OBJDUMP  = $(CROSS)objdump
SIZE     = $(CROSS)size
PYTHON  ?= python3

FREERTOS ?= ../FreeRTOS-Kernel
C         = ../c
PORT      = $(FREERTOS)/portable/GCC/RISC-V

# no -flto: portASM.S references kernel symbols that LTO could drop
CFLAGS = -Wall -Os -g -march=rv32i_zicsr -mabi=ilp32 -ffreestanding -nostartfiles -nostdlib \
         -ffunction-sections -fdata-sections \
         -I. -I$(C) -I$(FREERTOS)/include -I$(PORT) \
         -I$(PORT)/chip_specific_extensions/RV32I_CLINT_no_extensions \
         -I/usr/lib/picolibc/riscv64-unknown-elf/include

SOURCES = start.S main.c mini_libc.c $(C)/acia.c $(C)/printf.c \
          $(FREERTOS)/tasks.c $(FREERTOS)/list.c $(FREERTOS)/queue.c \
          $(FREERTOS)/portable/MemMang/heap_4.c \
          $(PORT)/port.c $(PORT)/portASM.S

all: app.img

app.elf: lnk-app.lds FreeRTOSConfig.h $(SOURCES) $(wildcard $(C)/*.h)
	$(CC) $(CFLAGS) -Wl,--gc-sections,-T,lnk-app.lds,-Map,app.map -o $@ $(SOURCES) -lgcc
	$(SIZE) $@

app.bin: app.elf
	$(OBJCOPY) -O binary -j .text -j .data $< $@

app.img: app.bin
	$(PYTHON) mkimage.py $< $@

disassemble: app.elf
	$(OBJDUMP) -d -S $< > app.dis

clean:
	rm -f *.elf *.bin *.img *.map *.dis

.PHONY: all clean disassemble

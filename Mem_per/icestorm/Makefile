	# Makefile for icestorm tools + yosys + nextpnr
# Modified from examples in icestorm distribution
# 01-16-18 E. Brombaugh

# src directory
VPATH = ../src:../picorv32

SRC =	up5k_riscv.v ../src/system.v ../src/spram_16kx32.v \
		../src/acia.v ../src/acia_rx.v ../src/acia_tx.v \
		../src/wb_bus.v ../src/wb_master.v \
		../src/vex_bridge.v ../src/VexRiscv.v ../src/mtimer.v

# preparing the machine code
FAKE_HEX =	rom.hex
REAL_HEX =  code.hex

# what goes into the 8 KB ROM:
#   boot (default) = FreeRTOS loader, copies ../freertos/app.img from flash @128 KB to SPRAM
#   c              = the bare-metal programs in ../c, as before:  make recode ROMCODE=c
ROMCODE ?= boot
APP_IMG = ../freertos/app.img

# project stuff
PROJ = up5k_riscv
PIN_DEF = up5k_riscv.pcf
SDC = up5k_riscv.sdc
DEVICE = up5k

YOSYS = /usr/bin/yosys
NEXTPNR = nextpnr-ice40
NEXTPNR_ARGS = --pre-pack $(SDC) --placer heap --timing-allow-fail
ICEPACK = icepack
ICETIME = icetime
ICEPROG = iceprog
ICEBRAM = icebram
CDCPROG = cdc_prog
VERILATOR = verilator
TECH_LIB = /usr/local/share/yosys/ice40/cells_sim.v

all: $(PROJ).bin

$(FAKE_HEX):
	$(ICEBRAM) -g 32 2048 > $(FAKE_HEX)

%.json: $(SRC) $(FAKE_HEX)
	$(YOSYS) -p 'synth_ice40 -dsp -top $(PROJ) -json $@' $(SRC)

%.asc: %.json $(PIN_DEF) 
	$(NEXTPNR) $(NEXTPNR_ARGS) --$(DEVICE) --json $< --pcf $(PIN_DEF) --asc $@

# always re-checked, so an old code.hex (e.g. from ../c) is never reused by mistake;
# only icebram/icepack re-run, not yosys/nextpnr
$(REAL_HEX): FORCE
	$(MAKE) -C ../$(ROMCODE)/ main.hex
	cp ../$(ROMCODE)/main.hex ./$(REAL_HEX)
		
%.bin: %.asc $(REAL_HEX)
	$(ICEBRAM) $(FAKE_HEX) $(REAL_HEX) < $< > temp.asc
	$(ICEPACK) temp.asc $@

%.rpt: %.asc
	$(ICETIME) -d $(DEVICE) -mtr $@ $<

prog: $(PROJ).bin
	$(ICEPROG) -S $<

recode:
	rm -f $(REAL_HEX) $(PROJ).bin
	$(MAKE) prog
    
flash: $(PROJ).bin
	$(ICEPROG) $<

# FreeRTOS: bitstream + app image in ONE flash file (app at 128 KB)
#   make freertos          build up5k_riscv_freertos.bin
#   make flash_freertos    write it to flash (FPGA boots, loader starts FreeRTOS)
# After changing only the app, this does not re-run yosys/nextpnr.
$(APP_IMG): FORCE
	$(MAKE) -C ../freertos app.img

$(PROJ)_freertos.bin: $(PROJ).bin $(APP_IMG)
	python3 ../freertos/mkimage.py --flash $(PROJ).bin $(APP_IMG) $@

freertos: $(PROJ)_freertos.bin

flash_freertos: $(PROJ)_freertos.bin
	$(ICEPROG) $<

lint: $(SRC)
	$(VERILATOR) --lint-only -Wall --top-module $(PROJ) $(TECH_LIB) $(SRC)

clean:
	$(MAKE) -C ../c/ clean
	$(MAKE) -C ../boot/ clean
	$(MAKE) -C ../freertos/ clean
	rm -f *.json *.asc *.rpt *.bin *.hex

.SECONDARY:
.PHONY: all prog clean freertos flash_freertos FORCE

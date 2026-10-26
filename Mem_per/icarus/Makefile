# Makefile for Icarus Verilog simulation
# 02-11-2019 E. Brombaugh

# sources
SOURCES = 	tb_system.v ../src/system.v ../src/spram_16kx32.v \
			../src/acia.v ../src/acia_rx.v ../src/acia_tx.v \
			../src/wb_bus.v ../src/wb_master.v \
			../src/vex_bridge.v ../src/VexRiscv.v ../src/mtimer.v

# preparing the machine code
HEX = rom.hex

# top level
TOP = tb_system
			
# Executables
VLOG = iverilog
HEXDUMP = hexdump
WAVE = gtkwave
TECH_LIB = /usr/local/share/yosys/ice40/cells_sim.v

# targets
all: $(TOP).vcd

$(HEX):
	$(MAKE) -C ../c/ main.hex
	cp ../c/main.hex ./$(HEX)
			
wave: $(TOP).vcd $(TOP).gtkw
	$(WAVE) $(TOP).gtkw
	
$(TOP).vcd: $(TOP)
	./$(TOP)

$(TOP): $(SOURCES) $(HEX)
	$(VLOG) -D icarus -l $(TECH_LIB) -o $(TOP) $(SOURCES)
	
clean:
	$(MAKE) -C ../c/ clean
	rm -rf a.out *.obj $(HEX) $(RPT) $(TOP) $(TOP).vcd tb_freertos fr

# ---------------------------------------------------------------------------
# FreeRTOS run (make freertos MS=1300): prints the UART output + LED edges.
# The boot loader is skipped (no SB_SPI model): rom.hex just jumps to SPRAM,
# which is preloaded with ../freertos/app.img. SPI/I2C are stubbed.
# Runs in ./fr so its rom.hex does not clash with the normal flow above.
MS ?= 1300
FR_SOURCES = tb_freertos.v sim_freertos_models.v ../src/system.v \
			../src/acia.v ../src/acia_rx.v ../src/acia_tx.v \
			../src/vex_bridge.v ../src/VexRiscv.v ../src/mtimer.v

tb_freertos: $(FR_SOURCES)
	$(VLOG) -g2012 -o $@ $(FR_SOURCES)

freertos: tb_freertos
	$(MAKE) -C ../freertos app.img
	mkdir -p fr
	$(HEXDUMP) -v -e '1/4 "%08x" "\n"' ../freertos/app.img > fr/app.hex
	printf '100002b7\n00028067\n' > fr/rom.hex
	cd fr && vvp -n ../tb_freertos +ms=$(MS)

.PHONY: freertos

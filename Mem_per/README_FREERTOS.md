# FreeRTOS on Mem_per

Copy these files over your `Mem_per` folder. Only the files listed here are new or changed.

## Changed
| File | Change |
|---|---|
| `src/system.v` | `tmr_sel` added to the read mux and to `mem_rdy`. Without it the CPU hangs on the first timer read. |
| `src/acia.v` | `clk_freq = 12000000` (was 24 MHz, so the UART ran at 57600 instead of 115200). |
| `src/mtimer.v` | Replace yours with this one. Port names match `system.v`, and it doesn't use `new`/`old` as names (SystemVerilog keywords). |
| `icestorm/up5k_riscv.v` | TX `SB_IO` on pin 10 un-commented, so the USB-UART converter gets data. |
| `icestorm/Makefile` | ROM content is now `../boot` (FreeRTOS loader). New targets: `freertos`, `flash_freertos`. `make recode ROMCODE=c` puts your old bare-metal `c/` program back. |
| `icarus/Makefile` | New target `freertos`. |
| `c/up5k_riscv.h` | Additions only: mtimer registers, LED bit names. |

## New
| | |
|---|---|
| `boot/` | 1.5 KB loader for the 8 KB ROM. It copies `app.img` from flash @ 128 KB into SPRAM, checks the checksum and jumps to it. Uses `c/acia.c`, `c/spi.c` and `c/flash.c` unchanged. |
| `freertos/` | The FreeRTOS application (3 test tasks), `FreeRTOSConfig.h`, `mkimage.py`. |
| `icarus/tb_freertos.v`, `icarus/sim_freertos_models.v` | Simulation of FreeRTOS on your real `src/VexRiscv.v`. |
| `GenUp5k.scala` | Adds `mhartid = 0`, which the FreeRTOS port reads. Regenerate `src/VexRiscv.v` with it. |

## Use
```
git clone https://github.com/FreeRTOS/FreeRTOS-Kernel     # inside Mem_per/
cd icestorm
make freertos          # bitstream + app in one file: up5k_riscv_freertos.bin
make flash_freertos    # iceprog up5k_riscv_freertos.bin
```
After changing only the FreeRTOS code, `make flash_freertos` runs again without yosys/nextpnr.

Flash layout: bitstream at 0 (104 090 bytes), app at 0x20000 (128 KB).

## What you should see (no UART needed)
d1-d4 are **active-low** on this board: LED OFF = 1, LED ON = 0.

1. Lamp test: all d-LEDs switch one way for about 0.4 s, then the other way for about 0.4 s. This proves the CPU runs from ROM.
2. d1-d4 show the boot stage in binary (d1 = bit 0, d4 = bit 3; an OFF LED is a 1):

| stage | LEDs OFF | meaning |
|---|---|---|
| 1 | d1 | CPU running the loader |
| 2 | d2 | SPI0 initialised |
| 3 | d1 d2 | flash answered (valid ID) |
| 4 | d3 | app header found at 128 KB |
| 5 | d1 d3 | app copied to SPRAM |
| 6 | d2 d3 | checksum ok, jumping to app |
| 7 | d1 d2 d3 | FreeRTOS `main()` running |
| 8 | d4 | scheduler starting |

   A steady pattern means the loader is stuck at that stage. If the pattern blinks, the loader failed at that stage.
3. While FreeRTOS runs: d1 blinks at 1 Hz (real-time task) and d2 at 2 Hz (background task). d4 stays off.
4. d3 blinking fast while FreeRTOS runs means an error: an assert, stack overflow, malloc failure or trap.

UART, 115200 8N1 (FPGA pin 10 = TX → converter RX, plus GND):
```
FreeRTOS V11.1.0+ on VexRiscv / iCE40UP5K, 12000000 Hz clock, 1000 Hz tick
t=1s  rt latency last=210us max=210us  heap free=13360 min=13360  stack free (words) rt=77 rep=189 bg=85
```
The loader also prints `up5k boot`, the flash ID and the number of bytes loaded.

## Simulation
`cd icarus && make freertos MS=300` takes about 3 minutes, `MS=1100` about 10 minutes
(1.1 s simulated, enough to see the first report line).

/*
 * boot.c - stage-1 boot ROM
 *
 * FreeRTOS does not fit in the 8 KB BRAM ROM, so this small loader lives
 * there instead. It copies the application image from the configuration
 * SPI flash (SPI0) into SPRAM at 0x10000000, verifies it and jumps to it.
 *
 * Benefit: changing the application only needs re-flashing the image,
 * not re-running yosys/nextpnr (see icestorm/Makefile: make freertos).
 *
 * Image header (first 16 bytes, written by start.S + tools/mkimage.py):
 *   +0  j reset        (so the image is directly executable at its start)
 *   +4  magic "FRTS"
 *   +8  image size in bytes (multiple of 4)
 *   +12 checksum: 32-bit sum of all image words with this field taken as 0
 *
 * LEDs (no UART needed):
 *   power-up lamp test: d1-d4 + RGB all ON ~0.4 s, all OFF ~0.4 s
 *     -> tells you the LED polarity and proves the CPU runs from ROM
 *   then d1-d4 show the boot STAGE in binary (d1 = bit 0 ... d4 = bit 3):
 *     1 CPU running          2 SPI0 initialised      3 flash answered (ID ok)
 *     4 app header ok        5 app copied to SPRAM   6 checksum ok, jumping
 *     7/8 set by the FreeRTOS app (main entered / scheduler starting)
 *   steady pattern  = stuck at that stage
 *   pattern blinking (alternating with all-LEDs-the-other-way) = boot failed at that stage
 *   NOTE: on this board d1-d4 are active-low: LED OFF = bit 1, LED ON = bit 0.
 */

#include "up5k_riscv.h"	/* ../c/ */
#include "acia.h"
#include "spi.h"
#include "flash.h"

#define APP_FLASH_ADDR  0x00020000UL	/* 128 KB: just past the 104 KB UP5K bitstream */
#define APP_RAM_ADDR    0x10000000UL
#define APP_MAX_SIZE    0x0000F000UL	/* top 4 KB of SPRAM belongs to the bootloader */
#define APP_MAGIC       0x53545246UL	/* "FRTS" little-endian */
#define CHUNK           1024

static void delay(volatile uint32_t n)
{
	while (n--)
		;
}

#define LED_STAGE_MASK  0xFu		/* d1..d4 = gp_out[3:0] */

static void stage(uint32_t n)
{
	gp_out = (gp_out & ~LED_STAGE_MASK) | (n & LED_STAGE_MASK);
}

static void puthex(uint32_t v)
{
	static const char hex[] = "0123456789ABCDEF";
	int i;

	acia_puts("0x");
	for (i = 28; i >= 0; i -= 4)
		acia_putc(hex[(v >> i) & 0xF]);
}

static void fail(char *msg)
{
	acia_puts("BOOT FAIL: ");
	acia_puts(msg);
	acia_puts("\r\n");
	gp_out &= LED_STAGE_MASK;	/* keep the stage code, RGB off */
	for (;;) {
		gp_out ^= LED_STAGE_MASK | LED_RGB2;	/* blink the stage pattern */
		delay(100000);
	}
}

void boot_main(void)
{
	uint32_t hdr[4], size, sum, off, n, i, id;
	uint32_t *img = (uint32_t *)APP_RAM_ADDR;

	/* lamp test (busy-wait timing, roughly 0.4 s each at 12 MHz) */
	gp_out = LED_STAGE_MASK | LED_RGB0 | LED_RGB1 | LED_RGB2;
	delay(150000);
	gp_out = 0;
	delay(150000);

	gp_out = LED_RGB0;
	stage(1);
	acia_puts("\r\n\r\nup5k boot\r\n");

	spi_init(SPI0);
	flash_init(SPI0);		/* release flash from deep power-down */
	delay(1000);			/* tRES1 >= 3 us */
	stage(2);

	id = flash_id(SPI0);
	acia_puts("flash id: ");
	puthex(id);
	acia_puts("\r\n");
	if (id == 0 || id == 0xFFFFFF)
		fail("flash does not answer (ID 000000/FFFFFF)");
	stage(3);

	/* header */
	flash_read(SPI0, (uint8_t *)hdr, APP_FLASH_ADDR, sizeof(hdr));
	if (hdr[1] != APP_MAGIC)
		fail("no app image at 128 KB (bad magic)");
	size = hdr[2];
	if (size < sizeof(hdr) || size > APP_MAX_SIZE || (size & 3))
		fail("bad image size");
	stage(4);

	/* body */
	for (off = 0; off < size; off += n) {
		n = size - off;
		if (n > CHUNK)
			n = CHUNK;
		flash_read(SPI0, (uint8_t *)APP_RAM_ADDR + off, APP_FLASH_ADDR + off, n);
	}

	stage(5);

	/* verify */
	sum = 0;
	for (i = 0; i < size / 4; i++)
		sum += (i == 3) ? 0 : img[i];
	if (sum != hdr[3])
		fail("checksum mismatch");

	acia_puts("loaded ");
	puthex(size);
	acia_puts(" bytes, jumping to app\r\n");
	gp_out = 0;
	stage(6);

	((void (*)(void))APP_RAM_ADDR)();
}

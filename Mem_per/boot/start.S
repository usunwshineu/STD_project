/*
 * start.S - boot ROM entry (runs from the 8 KB BRAM at 0x00000000)
 *
 * VexRiscv does NOT preset sp at reset (PicoRV32 did, via STACKADDR),
 * so it must be set here before any C code runs.
 */
	.section .text.start, "ax"
	.global _start
_start:
	la   sp, _stack_top

	// copy .data from ROM to RAM
	la   a0, _sidata
	la   a1, _sdata
	la   a2, _edata
1:	bgeu a1, a2, 2f
	lw   a3, 0(a0)
	sw   a3, 0(a1)
	addi a0, a0, 4
	addi a1, a1, 4
	j    1b
2:
	// zero .bss
	la   a0, _sbss
	la   a1, _ebss
3:	bgeu a0, a1, 4f
	sw   zero, 0(a0)
	addi a0, a0, 4
	j    3b
4:
	call boot_main
5:	j    5b

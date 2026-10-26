/*
 * start.S - application entry, linked at 0x10000000 (SPRAM)
 *
 * The boot loader (../boot) copies the image here and jumps to its first word.
 * .data is already in place (the image is loaded at its link address),
 * so only .bss needs clearing.
 */
	.section .text.start, "ax"
	.global _start
_start:
	j    reset
	.word 0x53545246		// magic "FRTS"
	.word 0				// image size  - patched by tools/mkimage.py
	.word 0				// checksum    - patched by tools/mkimage.py

reset:
	la   sp, _stack_top		// main()'s stack; reused by nothing after the
					// scheduler starts (ISR stack is xISRStack)
	// zero .bss
	la   a0, _sbss
	la   a1, _ebss
1:	bgeu a0, a1, 2f
	sw   zero, 0(a0)
	addi a0, a0, 4
	j    1b
2:
	// All traps (tick, ecall from portYIELD, exceptions) go to FreeRTOS.
	// xPortStartScheduler() does NOT set mtvec itself.
	la   t0, freertos_risc_v_trap_handler
	csrw mtvec, t0

	call main
3:	j    3b

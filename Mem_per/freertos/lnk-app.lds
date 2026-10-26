/*
 * lnk-app.lds - FreeRTOS application in SPRAM (64 KB at 0x10000000)
 *
 *   0x10000000  .text .rodata .data   <- loaded from flash by the boot ROM
 *               .bss (incl. FreeRTOS heap ucHeap[], xISRStack[])
 *   ...         main() stack, grows down from the top
 *   0x10010000
 *
 * The loaded part must end below 0x1000F000, where the boot ROM keeps its
 * stack while copying. .bss may extend into that area: it is zeroed after
 * the jump, when the boot ROM is finished.
 */
MEMORY
{
    RAM (rwx) : ORIGIN = 0x10000000, LENGTH = 0x10000
}
ENTRY(_start)
SECTIONS
{
    .text :
    {
        KEEP(*(.text.start))
        *(.text .text.*)
        *(.rodata .rodata.* .srodata .srodata.*)
        . = ALIGN(4);
    } > RAM

    .data :
    {
        *(.data .data.* .sdata .sdata.*)
        . = ALIGN(4);
        _image_end = .;
    } > RAM

    .bss (NOLOAD) :
    {
        _sbss = .;
        *(.bss .bss.* .sbss .sbss.* COMMON)
        . = ALIGN(4);
        _ebss = .;
    } > RAM

    _stack_top = ORIGIN(RAM) + LENGTH(RAM);

    ASSERT(_image_end <= 0x1000F000, "app image too big: overlaps boot ROM stack area")
    ASSERT(_ebss <= _stack_top - 1024, "RAM full: less than 1 KB left for main() stack - reduce configTOTAL_HEAP_SIZE")
}

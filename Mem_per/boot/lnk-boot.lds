/*
 * lnk-boot.lds - boot ROM linker script
 * Code in the 8 KB BRAM ROM. Data/bss/stack in the TOP 4 KB of SPRAM,
 * which the application image is not allowed to occupy (see lnk-app.lds),
 * so loading the app cannot overwrite the bootloader's own stack.
 */
MEMORY
{
    ROM  (rx)  : ORIGIN = 0x00000000, LENGTH = 0x2000
    RAM  (rw)  : ORIGIN = 0x1000F000, LENGTH = 0x1000
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
        _sidata = .;
    } > ROM

    .data : AT(_sidata)
    {
        . = ALIGN(4);
        _sdata = .;
        *(.data .data.* .sdata .sdata.*)
        . = ALIGN(4);
        _edata = .;
    } > RAM

    .bss (NOLOAD) :
    {
        _sbss = .;
        *(.bss .bss.* .sbss .sbss.* COMMON)
        . = ALIGN(4);
        _ebss = .;
    } > RAM

    _stack_top = ORIGIN(RAM) + LENGTH(RAM);
}

/* GBA linker script — Pokémon Red Remaster
   This is superseded by the -specs=gba.specs from devkitARM in most cases,
   but kept here as a reference and for custom section placement. */

OUTPUT_FORMAT("elf32-littlearm")
OUTPUT_ARCH(arm)
ENTRY(_start)

MEMORY {
    ROM  (rx)  : ORIGIN = 0x08000000, LENGTH = 32M
    EWRAM(rwx) : ORIGIN = 0x02000000, LENGTH = 256K
    IWRAM(rwx) : ORIGIN = 0x03000000, LENGTH = 32K
}

SECTIONS {
    .text : {
        *(.text.start)
        *(.text*)
        *(.rodata*)
    } > ROM

    .data : {
        *(.data*)
    } > IWRAM AT > ROM

    .bss : {
        *(.bss*)
        *(COMMON)
    } > EWRAM
}

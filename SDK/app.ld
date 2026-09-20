ENTRY(_davis_start)
PHDRS { text PT_LOAD FLAGS(5); ro PT_LOAD FLAGS(4); data PT_LOAD FLAGS(6); }
SECTIONS {
 . = 0x40000000;
 .text : ALIGN(4096) { *(.text.entry) *(.text .text.*) } :text
 . = ALIGN(4096);
 .rodata : { *(.rodata .rodata.*) } :ro
 . = 0x50000000;
 .data : ALIGN(4096) { *(.data .data.*) } :data
 .bss : { *(.bss .bss.*) *(COMMON) } :data
 /DISCARD/ : { *(.eh_frame*) *(.comment*) *(.note*) }
}

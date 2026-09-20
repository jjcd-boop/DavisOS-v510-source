ENTRY(_davis_start)
PHDRS { text PT_LOAD FLAGS(5); ro PT_LOAD FLAGS(4); data PT_LOAD FLAGS(6); }
SECTIONS {
 . = 0x40000000; .text : ALIGN(0x1000) { *(.text.entry) *(.text*) } :text
 .rodata : ALIGN(0x1000) { *(.rodata*) } :ro
 . = 0x50000000; .data : ALIGN(0x1000) { *(.data*) } :data
 .bss : ALIGN(16) { *(.bss*) *(COMMON) } :data
 /DISCARD/ : { *(.comment) *(.eh_frame*) *(.note*) }
}

ENTRY(efi_main)
SECTIONS {
 . = 0;
 .text : { *(.text*) }
 .data : { *(.data*) }
 .rodata : { *(.rodata*) }
 .bss : { *(.bss*) *(COMMON) }
}

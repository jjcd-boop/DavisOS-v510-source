ENTRY(DavisKernelMain)
SECTIONS {
 . = 0x02000000;
 __kernel_start = .;
 .text : ALIGN(4096) { __text_start = .; *(.text.entry) *(.text*) __text_end = .; }
 .rodata : ALIGN(4096) { __rodata_start = .; *(.rodata*) __rodata_end = .; }
 .data : ALIGN(4096) { __data_start = .; *(.data*) __data_end = .; }
 .bss : ALIGN(4096) { __bss_start = .; *(.bss*) *(COMMON) __bss_end = .; }
 __kernel_end = .;
}

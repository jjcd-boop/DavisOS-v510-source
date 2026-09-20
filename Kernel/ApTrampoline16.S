/* Davis OS x86 AP bootstrap trampoline. Copied below 1 MiB and entered by SIPI. */
.section .rodata.ap_trampoline,"a",@progbits
.balign 16
.global DavisApTrampolineBlobStart
.global DavisApTrampolineBlobEnd
DavisApTrampolineBlobStart:
.code16
    cli
    cld
    pushw %cs
    popw %ds
    pushw %cs
    popw %es

    /* Derive this SIPI segment's physical base and patch 32-bit GDT bases. */
    xorl %eax,%eax
    movw %cs,%ax
    shll $4,%eax
    movl %eax,%ebx
    movw %ax, (gdt32_code_base_lo-DavisApTrampolineBlobStart)
    shrl $16,%eax
    movb %al, (gdt32_code_base_mid-DavisApTrampolineBlobStart)
    movb %ah, (gdt32_code_base_hi-DavisApTrampolineBlobStart)
    movl %ebx,%eax
    movw %ax, (gdt32_data_base_lo-DavisApTrampolineBlobStart)
    shrl $16,%eax
    movb %al, (gdt32_data_base_mid-DavisApTrampolineBlobStart)
    movb %ah, (gdt32_data_base_hi-DavisApTrampolineBlobStart)

    movl %ebx,%eax
    addl $(gdt-DavisApTrampolineBlobStart),%eax
    movl %eax,(gdt_ptr_base-DavisApTrampolineBlobStart)
    lgdt (gdt_ptr-DavisApTrampolineBlobStart)
    movl %cr0,%eax
    orl $1,%eax
    movl %eax,%cr0
    ljmpl $0x08,$(pm32-DavisApTrampolineBlobStart)

.code32
pm32:
    movw $0x10,%ax
    movw %ax,%ds
    movw %ax,%es
    movw %ax,%ss

    /* Fail closed before CR3 is trusted. Handoff begins at image +512.
       magic=0x44564150484F4646, version=2, bytes=76, cr3@+24.
       A 32-bit seal authenticates all fields consumed before C++ validation. */
    cmpl $0x484f4646,512
    jne ap_halt32
    cmpl $0x44564150,516
    jne ap_halt32
    cmpl $2,520
    jne ap_halt32
    cmpl $76,524
    jne ap_halt32
    /* Verify the pre-paging seal over the first 64 handoff bytes.
       Must exactly match ApTrampoline::ComputePrePagingSeal(). */
    movl $0xD415A55A,%eax
    xorl %ecx,%ecx
seal_loop:
    xorl 512(,%ecx,4),%eax
    roll $5,%eax
    addl $0x9e3779b9,%eax
    incl %ecx
    cmpl $16,%ecx
    jb seal_loop
    cmpl 576,%eax                  /* Handoff.prePagingSeal @ +64 */
    jne ap_halt32

    cmpl $0,540                    /* high CR3 dword must be zero in .code32 */
    jne ap_halt32
    movl 536,%eax
    testl %eax,%eax
    jz ap_halt32
    testl $0xfff,%eax              /* CR3 root must be 4 KiB aligned */
    jnz ap_halt32
    movl %eax,%cr3
    movl %cr4,%eax
    orl $0x20,%eax                 /* CR4.PAE */
    movl %eax,%cr4
    movl $0xC0000080,%ecx          /* IA32_EFER */
    rdmsr
    orl $0x100,%eax                /* EFER.LME */
    wrmsr
    movl %cr0,%eax
    orl $0x80000000,%eax           /* CR0.PG */
    movl %eax,%cr0

    /* Long-mode far target is a linear address, so add SIPI physical base. */
    movl %ebx,%eax
    addl $(lm64-DavisApTrampolineBlobStart),%eax
    movl %eax,(lm_ptr-DavisApTrampolineBlobStart)
    ljmpl *(lm_ptr-DavisApTrampolineBlobStart)

ap_halt32:
    cli
1:  hlt
    jmp 1b

.code64
lm64:
    movw $0x20,%ax
    movw %ax,%ds
    movw %ax,%es
    movw %ax,%ss
    /* RIP-relative references remain valid after the trampoline is copied. */
    movl %ebx,%esi
    addq $512,%rsi
    movq 32(%rsi),%rsp             /* Handoff.stackTop */
    andq $-16,%rsp
    movq 40(%rsi),%rax             /* Handoff.entryPoint */
    testq %rax,%rax
    jz ap_halt64
    /* System V x86-64: pass Handoff* in RDI to the kernel AP entry. */
    movq %rsi,%rdi
    jmp *%rax
ap_halt64:
    cli
2:  hlt
    jmp 2b

.balign 8
lm_ptr:
    .long 0
    .word 0x18

.balign 8
gdt:
    .quad 0
    /* selector 0x08: 32-bit code, base patched at runtime */
    .word 0xffff
gdt32_code_base_lo: .word 0
gdt32_code_base_mid: .byte 0
    .byte 0x9a
    .byte 0xcf
gdt32_code_base_hi: .byte 0
    /* selector 0x10: 32-bit data, base patched at runtime */
    .word 0xffff
gdt32_data_base_lo: .word 0
gdt32_data_base_mid: .byte 0
    .byte 0x92
    .byte 0xcf
gdt32_data_base_hi: .byte 0
    /* selector 0x18: 64-bit code, base ignored */
    .quad 0x00af9a000000ffff
    /* selector 0x20: data */
    .quad 0x00cf92000000ffff
gdt_end:
gdt_ptr:
    .word gdt_end-gdt-1
gdt_ptr_base:
    .long 0

.global DavisApTrampolineBlobEnd
DavisApTrampolineBlobEnd:
.code64

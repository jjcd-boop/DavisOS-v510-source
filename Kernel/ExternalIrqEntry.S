.text
.extern DavisExternalIrqDispatch
.macro IRQSTUB name, vec
.global \name
.type \name,@function
\name:
    cld
    pushq %rax
    pushq %rcx
    pushq %rdx
    pushq %rsi
    pushq %rdi
    pushq %r8
    pushq %r9
    pushq %r10
    pushq %r11
    pushq %r12
    movq $\vec,%rdi
    movq %rsp,%r12
    andq $-16,%rsp
    call DavisExternalIrqDispatch
    movq %r12,%rsp
    popq %r12
    popq %r11
    popq %r10
    popq %r9
    popq %r8
    popq %rdi
    popq %rsi
    popq %rdx
    popq %rcx
    popq %rax
    iretq
.size \name,.-\name
.endm
IRQSTUB DavisIrq50,0x50
IRQSTUB DavisIrq51,0x51
IRQSTUB DavisIrq52,0x52
IRQSTUB DavisIrq53,0x53
IRQSTUB DavisIrq54,0x54
IRQSTUB DavisIrq55,0x55
IRQSTUB DavisIrq56,0x56
IRQSTUB DavisIrq57,0x57
IRQSTUB DavisIrq58,0x58
IRQSTUB DavisIrq59,0x59
IRQSTUB DavisIrq5A,0x5a
IRQSTUB DavisIrq5B,0x5b
IRQSTUB DavisIrq5C,0x5c
IRQSTUB DavisIrq5D,0x5d
IRQSTUB DavisIrq5E,0x5e
IRQSTUB DavisIrq5F,0x5f

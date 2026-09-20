.text
.extern DavisExceptionDispatch
.extern DavisExceptionSelectReturn
.extern DavisCommitContextSwitch
.extern DavisTrapShouldAbandon
.extern DavisTrapAbandonToKernel
.extern DavisCommitFaultActive
.extern DavisEmergencyCommitHalt
.extern DavisSwitchToKernelAddressSpace
.extern DavisRestoreUserAddressSpaceIfCpl3
.macro SAVE_AND_CALL
    cld
    pushq %rax; pushq %rbx; pushq %rcx; pushq %rdx; pushq %rsi; pushq %rdi; pushq %rbp
    pushq %r8; pushq %r9; pushq %r10; pushq %r11; pushq %r12; pushq %r13; pushq %r14; pushq %r15
    movq %rsp,%r12
    andq $-16,%rsp
    call DavisSwitchToKernelAddressSpace
    movq %r12,%rdi
    call DavisExceptionDispatch
    call DavisCommitFaultActive
    testq %rax,%rax
    jnz DavisEmergencyCommitHalt
    movq %r12,%rdi
    call DavisExceptionSelectReturn
    testq %rax,%rax
    jz .Lno_recovery\@
    movq %rax,%rdi
    call DavisCommitContextSwitch
    movq %rax,%rsp
    jmp .Lrestore\@
.Lno_recovery\@:
    call DavisTrapShouldAbandon
    testq %rax,%rax
    jnz DavisTrapAbandonToKernel
    movq 144(%r12),%rdi
    call DavisRestoreUserAddressSpaceIfCpl3
    movq %r12,%rsp
.Lrestore\@:
    popq %r15; popq %r14; popq %r13; popq %r12; popq %r11; popq %r10; popq %r9; popq %r8
    popq %rbp; popq %rdi; popq %rsi; popq %rdx; popq %rcx; popq %rbx; popq %rax
    addq $16,%rsp
    iretq
.endm
.macro NOERR name vec
.global \name
.type \name,@function
\name: pushq $0; pushq $\vec; SAVE_AND_CALL
.endm
.macro ERR name vec
.global \name
.type \name,@function
\name: pushq $\vec; SAVE_AND_CALL
.endm
NOERR DavisExceptionDE,0
NOERR DavisExceptionNMI,2
NOERR DavisExceptionBP,3
NOERR DavisExceptionOF,4
NOERR DavisExceptionBR,5
NOERR DavisExceptionUD,6
NOERR DavisExceptionNM,7
ERR DavisExceptionNP,11
ERR DavisExceptionSS,12
ERR DavisExceptionGP,13
ERR DavisExceptionPF,14
ERR DavisExceptionAC,17
.global DavisExceptionMC
.type DavisExceptionMC,@function
DavisExceptionMC:
    pushq $0
    pushq $18
    cld
    pushq %rax; pushq %rbx; pushq %rcx; pushq %rdx; pushq %rsi; pushq %rdi; pushq %rbp
    pushq %r8; pushq %r9; pushq %r10; pushq %r11; pushq %r12; pushq %r13; pushq %r14; pushq %r15
    movq %rsp,%rdi
    andq $-16,%rsp
    call DavisExceptionDispatch
.Lmc_halt:
    cli
    hlt
    jmp .Lmc_halt
NOERR DavisExceptionXM,19

.text
.extern DavisTrapShouldAbandon
.extern DavisTrapAbandonToKernel
.extern DavisSwitchToKernelAddressSpace
.extern DavisRestoreUserAddressSpaceIfCpl3
.global DavisTimerEntry
.type DavisTimerEntry,@function
// External LAPIC timer IRQ, vector 0x40 (64). Hardware supplies RIP/CS/RFLAGS and, only on
// privilege change, RSP/SS. v0.68 deliberately preempts only CPL3 frames.
DavisTimerEntry:
    cld
    pushq $0
    pushq $64
    pushq %rax
    pushq %rbx
    pushq %rcx
    pushq %rdx
    pushq %rsi
    pushq %rdi
    pushq %rbp
    pushq %r8
    pushq %r9
    pushq %r10
    pushq %r11
    pushq %r12
    pushq %r13
    pushq %r14
    pushq %r15
    movq %rsp,%r12
    andq $-16,%rsp
    call DavisSwitchToKernelAddressSpace
    movq %r12,%rdi
    call DavisTimerDispatch
    movq %rax,%r12
    andq $-16,%rsp
    call DavisTimerEoi
    movq %r12,%rdi
    call DavisCommitContextSwitch
    movq %rax,%r12
    call DavisTrapShouldAbandon
    testq %rax,%rax
    jnz DavisTrapAbandonToKernel
    # CpuContext::Frame.cs is the 19th qword (index 18) of the saved frame.
    movq 144(%r12),%rdi
    call DavisRestoreUserAddressSpaceIfCpl3
    movq %r12,%rsp
    popq %r15
    popq %r14
    popq %r13
    popq %r12
    popq %r11
    popq %r10
    popq %r9
    popq %r8
    popq %rbp
    popq %rdi
    popq %rsi
    popq %rdx
    popq %rcx
    popq %rbx
    popq %rax
    addq $16,%rsp
    iretq
.size DavisTimerEntry,.-DavisTimerEntry

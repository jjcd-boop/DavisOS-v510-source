.text
.extern DavisTrapShouldAbandon
.extern DavisTrapAbandonToKernel
.extern DavisSwitchToKernelAddressSpace
.extern DavisRestoreUserAddressSpaceIfCpl3
.global DavisInt80Entry
.type DavisInt80Entry,@function
DavisInt80Entry:
    cld
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
    call DavisSwitchToKernelAddressSpace
    movq %rsp,%rdi
    call DavisInt80Dispatch
    call DavisTrapShouldAbandon
    testq %rax,%rax
    jnz DavisTrapAbandonToKernel
    movq $0x23,%rdi
    call DavisRestoreUserAddressSpaceIfCpl3
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
    iretq

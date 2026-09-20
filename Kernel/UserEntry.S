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
    # Frame is on TSS.RSP0 (mapped in the process CR3). Switch to the kernel
    # CR3 before C++ so SYS_ALLOC / CopyFromUser can identity-map page tables.
    call DavisSwitchToKernelAddressSpace
    movq %rsp,%rdi
    call DavisInt80Dispatch
    call DavisTrapShouldAbandon
    testq %rax,%rax
    jnz DavisTrapAbandonToKernel
    # int $0x80 is always from CPL3. CS sits at 15*8(%rsp) after the 15 pushes
    # (hardware RIP/CS/RFLAGS/RSP/SS). Restore user CR3 before iretq.
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
.size DavisInt80Entry,.-DavisInt80Entry

.global DavisEnterUser
.type DavisEnterUser,@function
DavisEnterUser:
    # SysV: rdi=rip,rsi=rsp,rdx=rflags,rcx=cs,r8=ss
    movq %r8,%rax
    pushq %rax
    pushq %rsi
    pushq %rdx
    pushq %rcx
    pushq %rdi
    iretq
1:  cli
    hlt
    jmp 1b
.size DavisEnterUser,.-DavisEnterUser

.global DavisReloadSegments
.type DavisReloadSegments,@function
DavisReloadSegments:
    # SysV: rdi=kernel code selector, rsi=kernel data selector
    movw %si,%ax
    movw %ax,%ds
    movw %ax,%es
    movw %ax,%ss
    pushq %rdi
    leaq 1f(%rip),%rax
    pushq %rax
    lretq
1:  ret
.size DavisReloadSegments,.-DavisReloadSegments

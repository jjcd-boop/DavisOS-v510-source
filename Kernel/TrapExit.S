.data
.align 8
.global DavisSavedKernelRsp
DavisSavedKernelRsp: .quad 0
.global DavisSavedKernelCr3
DavisSavedKernelCr3: .quad 0
.global DavisKernelContinuation
DavisKernelContinuation: .quad 0
.global DavisSavedUserCr3
DavisSavedUserCr3: .quad 0
.global DavisPendingUserCr3
DavisPendingUserCr3: .quad 0
.global DavisPendingKernelRsp0
DavisPendingKernelRsp0: .quad 0
.text
.global DavisPrepareTrapReturn
.type DavisPrepareTrapReturn,@function
DavisPrepareTrapReturn:
    # SysV: rdi=kernel rsp, rsi=kernel cr3, rdx=continuation
    movq %rdi,DavisSavedKernelRsp(%rip)
    movq %rsi,DavisSavedKernelCr3(%rip)
    movq %rdx,DavisKernelContinuation(%rip)
    ret

# Switch from the process CR3 to the kernel/UEFI CR3 so page-table walks,
# the physical allocator, and CopyFromUser identity maps are valid.
# Must be called only after the trap frame lives on a stack mapped in both
# address spaces (TSS.RSP0). Clobbers rax.
.global DavisSwitchToKernelAddressSpace
.type DavisSwitchToKernelAddressSpace,@function
DavisSwitchToKernelAddressSpace:
    movq %cr3,%rax
    movq %rax,DavisSavedUserCr3(%rip)
    movq DavisSavedKernelCr3(%rip),%rax
    testq %rax,%rax
    jz 1f
    cmpq DavisSavedUserCr3(%rip),%rax
    je 1f
    movq %rax,%cr3
1:  ret

# If returning to CPL3 and we are still on the kernel CR3 (no context switch
# already loaded a process CR3), restore the saved user CR3. rdi=CS.
.global DavisRestoreUserAddressSpaceIfCpl3
.type DavisRestoreUserAddressSpaceIfCpl3,@function
DavisRestoreUserAddressSpaceIfCpl3:
    testq $3,%rdi
    jz 1f
    movq DavisSavedKernelCr3(%rip),%rax
    testq %rax,%rax
    jz 2f
    movq %cr3,%rcx
    cmpq %rax,%rcx
    jne 1f
2:  movq DavisSavedUserCr3(%rip),%rax
    testq %rax,%rax
    jz 1f
    movq %rax,%cr3
1:  ret

.global DavisTrapAbandonToKernel
.type DavisTrapAbandonToKernel,@function
DavisTrapAbandonToKernel:
    cli
    movq DavisSavedKernelCr3(%rip),%rax
    testq %rax,%rax
    jz 1f
    movq %rax,%cr3
1:  movq DavisSavedKernelRsp(%rip),%rsp
    movq DavisKernelContinuation(%rip),%rax
    testq %rax,%rax
    jz 2f
    jmp *%rax
2:  hlt
    jmp 2b

.global DavisEnterPreparedUser
.type DavisEnterPreparedUser,@function
DavisEnterPreparedUser:
    # SysV: rdi rip, rsi rsp, rdx rflags, rcx cs, r8 ss, r9 new cr3
    # Optional DavisPendingKernelRsp0 is a stack mapped in the app CR3.
    pushq %rbx
    pushq %r12
    pushq %r13
    pushq %r14
    pushq %r15
    movq %rdi,%rbx
    movq %rsi,%r12
    movq %rdx,%r13
    movq %rcx,%r14
    movq %r8,%r15
    movq %r9,DavisPendingUserCr3(%rip)
    movq %cr3,%rsi
    movq %rsp,%rdi
    leaq .Luser_return(%rip),%rdx
    call DavisPrepareTrapReturn
    cli
    movq DavisPendingKernelRsp0(%rip),%rax
    testq %rax,%rax
    jz 1f
    movq %rax,%rsp
1:  movq DavisPendingUserCr3(%rip),%rax
    movq %rax,%cr3
    movq %r15,%rax
    pushq %rax
    pushq %r12
    pushq %r13
    pushq %r14
    pushq %rbx
    iretq
.Luser_return:
    popq %r15
    popq %r14
    popq %r13
    popq %r12
    popq %rbx
    ret
.size DavisEnterPreparedUser,.-DavisEnterPreparedUser

.global DavisResumePreparedUser
.type DavisResumePreparedUser,@function
DavisResumePreparedUser:
    # SysV: rdi=CpuContext::Frame*, rsi=user cr3
    # Frame lives in kernel BSS (mapped in the app CR3). Interrupts must stay
    # off from the CR3 write until iretq loads the user RFLAGS — a timer IRQ
    # in the one-instruction window with CR3=user and RSP=UEFI stack double-faults.
    pushq %rbx
    pushq %r12
    movq %rdi,%rbx
    movq %rsi,%r12
    movq %cr3,%rsi
    movq %rsp,%rdi
    leaq .Lresume_return(%rip),%rdx
    call DavisPrepareTrapReturn
    cli
    movq %r12,%cr3
    movq %rbx,%rsp
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
    addq $16,%rsp       # vector + error
    iretq
.Lresume_return:
    popq %r12
    popq %rbx
    ret
.size DavisResumePreparedUser,.-DavisResumePreparedUser

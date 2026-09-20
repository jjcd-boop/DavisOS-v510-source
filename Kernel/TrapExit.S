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
    movq %rdi,DavisSavedKernelRsp(%rip)
    movq %rsi,DavisSavedKernelCr3(%rip)
    movq %rdx,DavisKernelContinuation(%rip)
    ret
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

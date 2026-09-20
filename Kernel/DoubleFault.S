.text
.extern DavisDoubleFaultDispatch
.global DavisDoubleFault
.type DavisDoubleFault,@function
DavisDoubleFault:
    cld
    # #DF always carries a hardware error code. IST1 has already replaced RSP.
    movq %rsp,%rdi
    andq $-16,%rsp
    call DavisDoubleFaultDispatch
.Ldf_halt:
    cli
    hlt
    jmp .Ldf_halt

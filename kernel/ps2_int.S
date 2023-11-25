    .extern PS2InterruptHandler
    .global PS2InterruptHandler_
PS2InterruptHandler_:
    pusha
    push %esp
    call PS2InterruptHandler
    pop %esp
    popa
    iret


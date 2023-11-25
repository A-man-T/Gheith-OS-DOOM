	# outb(uint16_t port, uint8_t val)
	.global outb
outb:
	push %edx
	mov 8(%esp),%dx
	mov 12(%esp),%al
	outb %al,%dx
	pop %edx
	ret

	# outl(uint16_t port, uint32_t val)
        .global outl
outl:
        mov 4(%esp),%dx
        mov 8(%esp),%eax
        outl %eax,%dx
        ret

	# uint8_t inb(uint16_t port)
	.global inb
inb:
	push %edx
	mov 8(%esp),%dx
	inb %dx,%al
	pop %edx
	and $0xff,%eax
	ret

	# uint32_t inl(uint16_t port)
	.global inl
inl:
	push %edx
	mov 8(%esp),%dx
	inl %dx,%eax
	pop %edx
	ret

    .globl rdmsr
    # uint64_t rdmsr(uint32_t id)
rdmsr:
    mov 4(%esp), %ecx
    rdmsr
    ret

    .globl wrmsr
    # wrmsr (uint32_t id, uint64_t value)
wrmsr:
    mov 4(%esp), %ecx
    mov 8(%esp), %eax
    mov 12(%esp), %edx
    wrmsr
    ret

    .global spuriousHandler_
spuriousHandler_:
    iret

    .extern apitHandler
    .global apitHandler_
apitHandler_:
    // TODO: XMM, MMX, FP, ...
    pusha
    push %esp
    call apitHandler
    pop %esp
    popa
    iret

    .global sti
sti:
    sti
    ret

    .global cli
cli:
    cli
    ret

    .global getFlags
getFlags:
    pushf
    pop %eax
    ret

	/* memcpy(void* dest, void* src, size_t n) */
        .global memcpy
memcpy:
        mov 4(%esp),%eax       # dest
        mov 8(%esp),%edx       # src
        mov 12(%esp),%ecx      # n
        push %ebx
1:
        add $-1,%ecx
        jl 1f
        movb (%edx),%bl
        movb %bl,(%eax)
        add $1,%edx
        add $1,%eax
        jmp 1b
1:
        pop %ebx
        ret

     /* bzero(void* dest, size_t n) */
    .global bzero
bzero:
    mov 4(%esp),%eax       # dest
    mov 8(%esp),%ecx      # n
1:
    add $-1,%ecx
    jl 1f
    movb $0,(%eax)
    add $1,%eax
    jmp 1b
1:
    ret

	# ltr(uint32_t tr)
	.global ltr
ltr:
	mov 4(%esp),%eax
	ltr %ax
	ret

    # monitor(void* ptr)
    .global monitor
monitor:
    mov 4(%esp),%eax
    xor %ecx,%ecx
    xor %edx,%edx
    monitor
    ret

    # mwait()
    .global mwait
mwait:
    xor %eax,%eax
    xor %ecx,%ecx
    xor %edx,%edx
    mwait
    ret

    # cpuid(long eax, cpuid_out* out)
    #          12              16
    .global cpuid
cpuid:
    push %ebx          // moves everything by 4
    push %esi          // moves everything by 4+4=8
    mov 12(%esp),%eax
    cpuid
    mov 16(%esp),%esi
    mov %eax,0(%esi)
    mov %ebx,4(%esi)
    mov %ecx,8(%esi)
    mov %edx,12(%esi)
    pop %esi
    pop %ebx
    ret


    # start_process(Process process)
        .global start_process
start_process:
        mov 4(%esp), %eax       # struct base pointer
        # push interrupt frame
        push userSS             # user ss
        push 8(%eax)            # user esp
        push 4(%eax)            # user eflags
        push userCS             # user cs
        push 0(%eax)            # user eip
        # restore registers
        mov 12(%eax), %edi
        mov 16(%eax), %esi
        mov 20(%eax), %ebp
        mov 24(%eax), %ebx
        mov 28(%eax), %edx
        mov 32(%eax), %ecx
        mov 36(%eax), %eax
        # go to user process
        iret

	.global sysHandler_
sysHandler_:
        pusha
        push %esp
        push %eax
        .extern sysHandler
        call sysHandler
        add $40,%esp # move esp back to where it started
        iret

/* invlpg(uint32_t va) */
    .global invlpg
invlpg:
    mov 4(%esp),%eax
    invlpg (%eax)
    ret

    .global pageFaultHandler_
pageFaultHandler_:
    pusha

    mov %esp,%eax
    push %eax       /* second argument */
    mov %cr2,%eax   /* address */
    push %eax       /* first argument (va) */

    .extern vmm_pageFault
    call vmm_pageFault   /* (va, sp) */

    add $8,%esp    /* pop arguments */

    popa

    add $4,%esp   /* pop error */
    iret

    /* vmm_on(uint32_t pd) */
    .global vmm_on
vmm_on:
    mov 4(%esp),%eax
    mov %eax,%cr3

    mov %cr0,%eax
    # enable virtual memory and write protection in kernel mode
    or $0x80010000,%eax
    mov %eax,%cr0
    ret

    /* vmm_global_on() */
    .global vmm_global_on
vmm_global_on:
    # set the PGE bit in cr4 to enable global optimization on TLB
    mov %cr4, %eax
    or $0b10000000, %eax
    mov %eax, %cr4
    ret

    /* uint32_t getCR3() */
    .global getCR3
getCR3:
    mov %cr3,%eax
    ret


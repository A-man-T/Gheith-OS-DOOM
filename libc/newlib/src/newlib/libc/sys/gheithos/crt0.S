	.extern main
    .extern exit

	.global _start
_start:
	call main
	push %eax
loop:
	call exit
	jmp loop


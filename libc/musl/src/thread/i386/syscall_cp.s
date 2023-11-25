.text
.global __cp_begin
.hidden __cp_begin
.global __cp_end
.hidden __cp_end
.global __cp_cancel
.hidden __cp_cancel
.hidden __cancel
.global __syscall_cp_asm
.hidden __syscall_cp_asm
.type   __syscall_cp_asm,@function
__syscall_cp_asm:
	mov 4(%esp),%ecx
	pushl %ebx
	pushl %esi
	pushl %edi
	pushl %ebp
__cp_begin:
	movl (%ecx),%eax
	testl %eax,%eax
	jnz __cp_cancel
	add $28, %esp
	mov (%esp), %eax
	int $48
	sub $28, %esp
__cp_end:
	popl %ebp
	popl %edi
	popl %esi
	popl %ebx
	ret
__cp_cancel:
	popl %ebp
	popl %edi
	popl %esi
	popl %ebx
	jmp __cancel

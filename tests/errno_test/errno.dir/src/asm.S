.global _highEsp
_highEsp:
    mov 4(%esp), %eax # sysnum

    # Save state
    push %ebx
    push %esi
    push %edi
    push %ebp
    mov %esp, %esi # Save original esp for later

    # Set up the test
    mov $0xFFFFFFF0, %esp

    # Do the syscall
    int $48

    # Resume state
    mov %esi, %esp # Get saved esp from earlier
    pop %ebp
    pop %edi
    pop %esi
    pop %ebx
    ret
.global _lowEsp
_lowEsp:
    mov 4(%esp), %eax # sysnum

    # Save state
    push %ebx
    push %esi
    push %edi
    push %ebp
    mov %esp, %esi # Save original esp for later

    # Set up the test
    mov $0, %esp

    # Do the syscall
    int $48

    # Resume state
    mov %esi, %esp # Get saved esp from earlier
    pop %ebp
    pop %edi
    pop %esi
    pop %ebx
    ret
.global _unmappedEsp
_unmappedEsp:
    mov 4(%esp), %eax # sysnum

    # Save state
    push %ebx
    push %esi
    push %edi
    push %ebp
    mov %esp, %esi # Save original esp for later

    # Set up the test
    mov $0xA0000000, %esp

    # Do the syscall
    int $48

    # Resume state
    mov %esi, %esp # Get saved esp from earlier
    pop %ebp
    pop %edi
    pop %esi
    pop %ebx
    ret

.global _sigreturn
_sigreturn:
    # Save state
    push %ebx
    push %esi
    push %edi
    push %ebp

    # Do the syscall
    mov $1006, %eax
    int $48

    # Resume state
    pop %ebp
    pop %edi
    pop %esi
    pop %ebx
    ret
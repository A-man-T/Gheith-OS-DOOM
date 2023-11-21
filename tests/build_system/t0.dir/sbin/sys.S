	#
	# user-side system calls
	#
	# System calls use a special convention:
        #     %eax  -  system call number
        #
        #

	# void exit(int status)
	.global exit
exit:
	mov $0,%eax
	int $48
	ret

	# ssize_t write(int fd, void* buf, size_t nbyte)
	.global write
write:
	mov $1,%eax
	int $48
	ret

	# int execl(const char *pathname, const char *arg, ...
        #               /* (char  *) NULL */);
execl:
	mov $1000,%eax
	int $48
	ret


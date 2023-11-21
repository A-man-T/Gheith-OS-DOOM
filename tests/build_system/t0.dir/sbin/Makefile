UTILS = init

CFLAGS = -std=c99 -m32 -nostdlib -nostdinc -g -O2 -Wall -Werror

all : $(UTILS)

OFILES = sys.o crt0.o libc.o heap.o machine.o

# keep all files
.SECONDARY :

%.o :  Makefile %.c
	gcc -c -MD $(CFLAGS) $*.c

%.o :  Makefile %.S
	gcc -MD -m32 -c $*.S

%.o :  Makefile %.s
	gcc -MD -m32 -c $*.s

$(UTILS) : % : Makefile %.o $(OFILES)
	ld -N -m elf_i386 -e start -Ttext=0x00600000 -o $@  $*.o $(OFILES)

clean ::
	rm -f *.o
	rm -f *.d
	#rm -f $(UTILS)

-include *.d

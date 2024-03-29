# This version of config.mak was generated by:
# ./configure --target=i386 --disable-shared
# Any changes made here will be lost if configure is re-run
AR = ar
RANLIB = ranlib
ARCH = i386
SUBARCH =
ASMSUBARCH =
srcdir = .
prefix = ${HOME}/musl
exec_prefix = $(prefix)
bindir = $(exec_prefix)/bin
libdir = $(prefix)/lib
includedir = $(prefix)/include
syslibdir = /lib
CC = gcc -m32
CFLAGS = -march=i686 -D__thread="^-^"
CFLAGS_AUTO = -O2 -fno-align-jumps -fno-align-functions -fno-align-loops -fno-align-labels -fira-region=one -fira-hoist-pressure -freorder-blocks-algorithm=simple -fno-prefetch-loop-arrays -fno-tree-ch -pipe -fomit-frame-pointer -fno-unwind-tables -fno-asynchronous-unwind-tables -ffunction-sections -fdata-sections -mtune=generic -Wno-pointer-to-int-cast -Werror=implicit-function-declaration -Werror=implicit-int -Werror=pointer-sign -Werror=pointer-arith -Werror=int-conversion -Werror=incompatible-pointer-types -Werror=discarded-qualifiers -Werror=discarded-array-qualifiers -Waddress -Warray-bounds -Wchar-subscripts -Wduplicate-decl-specifier -Winit-self -Wreturn-type -Wsequence-point -Wstrict-aliasing -Wunused-function -Wunused-label -Wunused-variable
CFLAGS_C99FSE = -std=c99 -nostdinc -ffreestanding -fexcess-precision=standard -frounding-math -fno-strict-aliasing -Wa,--noexecstack
CFLAGS_MEMOPS = -fno-tree-loop-distribute-patterns
CFLAGS_NOSSP = -fno-stack-protector
CPPFLAGS =
LDFLAGS = -m32
LDFLAGS_AUTO = -Wl,--sort-section,alignment -Wl,--sort-common -Wl,--gc-sections -Wl,--hash-style=both -Wl,--no-undefined -Wl,--exclude-libs=ALL -Wl,--dynamic-list=./dynamic.list
CROSS_COMPILE = i386-
LIBCC = -lgcc -lgcc_eh
OPTIMIZE_GLOBS = internal/*.c malloc/*.c string/*.c
ALL_TOOLS =  obj/musl-gcc
TOOL_LIBS =  lib/musl-gcc.specs
ADD_CFI = no
MALLOC_DIR = mallocng
SHARED_LIBS =
WRAPCC_GCC = $(CC)
AOBJS = $(LOBJS)

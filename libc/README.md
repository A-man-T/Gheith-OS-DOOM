# Libc Instructions

## Newlib

## Musl

The musl here is not fully working in our OS and should probably not be used; Newlib is preferred. It depends on many Linux-specific behaviors that we have not yet implemented. However, it does build and is able to compile a test case, so this is left here for future reference if we ever want to try again.

Build steps:

-   Do NOT run `./configure`. The lab machines will not generate a working `config.mak`, but the included one will work correctly on the lab machines. We have not debugged why the lab machines don't work, but the command used to generate the working `config.mak` was based on the following command, but with a few manual edits:
    ```
    CC=gcc RANLIB=ranlib AR=ar ./configure --target=i386 --disabled-shared
    ```
-   To build musl, simply run `make` in the folder, then `make install` to install. The provided configuration will place the built files in your home directory in a folder named `musl`.
-   To build a test case, use the provided `libc/gheithos.ld` linker script with the command:
    ```
    gcc -T gheithos.ld -Wl,-melf_i386 -static -specs $HOME/musl/lib/musl-gcc.specs -m32 -o <output binary> <input file(s)>
    ```

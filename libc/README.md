# Libc Instructions

## Newlib

Newlib is our preferred libc to use here. Building it is a more involved process than musl, but it works better with our OS's available features.

For most use you should not need the instructions here, and can just use the `make libc` command in the top level folder. It will take a while to run the first time, but then will go faster.

The build system in Newlib uses Autotools for everything, which will work with regular make once the Makefiles are generated, but to use Autotools to generate these requires a specific version, 2.69. If you need to make an edit to the Autotools files but don't want to install this version, please contact the Libc team and we can help regenerate these files. This should only be necessary if you need to edit any of the build system for whatever reason; editing existing files will work fine. If you do edit Automake files and want to set it up, be sure to add the Autotools 2.69 to your PATH, and then run `autoreconf` from the `libc/newlib/src/newlib` folder.

### Adding Syscalls

-   Find the `libc/newlib/src/newlib/libc/sys/gheithos` folder
-   If the syscall is nonstandard, add it to the `sys/gheithos.h` header
-   Implement the syscall in `syscalls.c`, see other syscalls for reference
-   Rebuild libc with `make libc` in the top level Makefile

### Build Steps

-   First, ensure that your current folder is `libc/newlib`, and CD into it if not.
-   Then, set up your PATH to include the ` libc/newlib/build_tools` folder. This is necessary because Newlib expects to find the build tools with an `i386-pc-gheithos-` prefix, but we don't need any special cross compilers, so these scripts just pass through to the regular host x86 build tools, with some extra options for 32 bit. This can be done like so:
    ```
    $ export PATH=$PWD/build_tools:$PATH
    ```
-   Next, you need to set up the build folder and CD into it:
    ```
    $ mkdir build
    $ cd build
    ```
-   Now, configure Newlib with the following command:
    ```
    $ ../src/configure --target=i386-pc-gheithos --disable-multilib --prefix=$PWD/../install
    ```
-   Finally, build newlib by running `make`, and install it with `make install`.
-   To compile a program using our build of Newlib, use the provided script in `libc/gheithos-gcc`, which will add the proper options to build with our libc. Your user code files should look like a normal libc program you would run on Linux (same headers). There is no provided separate linker; just use gcc as the linker by passing in object files, and if you need separate linker flags, use `-Wl,--linker-flag`. Example usage:

    ```
    $ ./libc/gheithos-gcc -o testing testing.c
    ```

## musl

The musl here is not fully working in our OS and should probably not be used; Newlib is preferred. It depends on many Linux-specific behaviors that we have not yet implemented. However, it does build and is able to compile a test case, so this is left here for future reference if we ever want to try again.

### Build Steps

-   Do NOT run `./configure`. The lab machines will not generate a working `config.mak`, but the included one will work correctly on the lab machines. We have not debugged why the lab machines don't work, but the command used to generate the working `config.mak` was based on the following command, but with a few manual edits:
    ```
    $ CC=gcc RANLIB=ranlib AR=ar ./configure --target=i386 --disabled-shared
    ```
-   To build musl, simply run `make` in the folder, then `make install` to install. The provided configuration will place the built files in your home directory in a folder named `musl`.
-   To build a test case, use the provided `libc/gheithos.ld` linker script with the command:
    ```
    $ gcc -T gheithos.ld -Wl,-melf_i386 -static -specs $HOME/musl/lib/musl-gcc.specs -m32 -o <output binary> <input file(s)>
    ```

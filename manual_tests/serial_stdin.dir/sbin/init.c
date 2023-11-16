#include "libc.h"

/**
 * This is just a simple program that allows us to test that we can read from serial input
 * I did not put this with the unit_tests since it requires user input and validation
 * If someone wants to figure out a way to automate this (shell scripts?) feel free
*/
int main(int argc, char** argv) {
    char str[256] = {};
    printf("%s\n", gets(str));
    shutdown();
}
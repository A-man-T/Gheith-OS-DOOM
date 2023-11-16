#include "libc.h"

/**
 * This is just a simple program that allows us to test that we can read from serial input
 * Can be automated with the pytest in manual_tests/test_run_manuals.py
 *   (looks at manual_tests/serial_stdin.dir/files/sample_input.txt)
 */
int main(int argc, char** argv) {
    char str[256] = {};
    printf("%s\n", gets(str));
    shutdown();
}
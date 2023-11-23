#include "libc.h"

int putchar(int c) {
    char t = (char)c;
    return write(1,&t,1);
}

int puts(const char* p) {
    char c;
    int count = 0;
    while ((c = *p++) != 0) {
        int n = putchar(c); 
        if (n < 0) return n;
        count ++;
    }
    putchar('\n');
    
    return count+1;
}

/**
 * A very simple/naive approach to getchar
 * Converts carriage returns from the enter key to a \n but that's as fancy as this gets
 * Backspace not an option right now so don't mess up
 * Since we do not have a terminal right now we will just print the incoming characters as
 * the user types so they they get visual feedback that it is working
 */
int getchar() {
    char c;
    read(0, &c, 1);
    if (c == '\r') {
        c = '\n';
    }
    putchar(c);
    return c;
}

char *gets(char *s) {
    size_t i = 0;
    char c = getchar();
    while (c != '\n' && c != '\0') {
        s[i++] = c;
        c = getchar();
    }
    s[i] = '\0';
    return s;
}
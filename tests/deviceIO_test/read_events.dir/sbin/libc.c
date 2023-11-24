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

void print_ev(int i){
    printf("event: %i\n", i);
    printf("\ttype: %i\n", (i >> 30) & 0x3);
    printf("\tcode: %i\n", (i >> 25) & 0x1F);
    printf("\tvalue: %i\n", (i >> 16) & 0x1FF);
    printf("\ttime: %i\n", i & 0xFFFF);
}
#include <stdio.h>
#include <stdlib.h>
#include <sys/gheithos.h>


#define ASSERT(c) do {                                       \
    if (!(c)) {                                              \
        printf("*** failure at %s:%d\n",__FILE__, __LINE__); \
        exit(-1);                                            \
    }                                                        \
} while (0)
    
void print_ev(int i){
    printf("event: %i\n", i);
    printf("\ttype: %i\n", (i >> 30) & 0x3);
    printf("\tcode: %i\n", (i >> 25) & 0x1F);
    printf("\tvalue: %i\n", (i >> 16) & 0x1FF);
    printf("\ttime: %i\n", i & 0xFFFF);
}

int main(int argc, char** argv) {
    printf("*** start\n");
    while(1){
        int x = read_key_event();
        if (x != 0){
            print_ev(x);
        }
        x = read_mouse_event();
        if (x != 0){
            print_ev(x);
        }
    }
    shutdown();
    return 0;
}

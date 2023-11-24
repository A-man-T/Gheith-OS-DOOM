#include "libc.h"

#define ASSERT(c) do {                                       \
    if (!(c)) {                                              \
        printf("*** failure at %s:%d\n",__FILE__, __LINE__); \
        exit(-1);                                            \
    }                                                        \
} while (0)
    


int main(int argc, char** argv) {
    printf("*** start\n");
    while(1){
        int x = read_key();
        if (x != 0){
            print_ev(x);
        }
        x = read_mouse();
        if (x != 0){
            print_ev(x);
        }
    }
    shutdown();
    return 0;
}

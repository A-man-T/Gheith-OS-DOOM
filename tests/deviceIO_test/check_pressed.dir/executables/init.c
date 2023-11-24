#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/gheithos.h>

#define ASSERT(c) do {                                       \
    if (!(c)) {                                              \
        printf("*** failure at %s:%d\n",__FILE__, __LINE__); \
        exit(-1);                                            \
    }                                                        \
} while (0)

int main(int argc, char** argv) {
    printf("*** start\n");
    bool last_pressed = false;
    bool last_held = false;
    bool pressed;
    bool held;
    while(1){
        pressed = (is_pressed(30) != 0); //checks if A is pressed 
        held = (is_held(30) != 0); //checks if A is held 

        if (last_held != held){
            if (held){
                printf("A held\n");
            } else {
                printf("A not held\n");
            }
        }
        if (last_pressed != pressed){
            if (pressed){
                printf("A pressed\n");
            } else {
                printf("A not pressed\n");
            }
        }
        last_held = held;
        last_pressed = pressed;
    }
    shutdown();
    return 0;
}

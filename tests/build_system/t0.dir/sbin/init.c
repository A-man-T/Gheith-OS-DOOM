#include "libc.h"

int main(int argc, char** argv) {
    puts("*** hello, user");
    char* ptr = malloc(100);
    memcpy(ptr,"*** looking normal\n",19);
    int togo = 19;
    while (togo > 0) {
        int cnt = write(1,ptr,togo);
        togo -= cnt;
        ptr += cnt;
    }
    return 0;
}

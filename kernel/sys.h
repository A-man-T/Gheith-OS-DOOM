#ifndef _SYS_H_
#define _SYS_H_

#include "stdint.h"

class SYS {
   public:
    static void init(void);
    static int32_t execve(const char *pathname, const char *const argv[], const char *const envp[], bool is_initial = false);
    static void exit_process(uint32_t exit_code);
};

#endif

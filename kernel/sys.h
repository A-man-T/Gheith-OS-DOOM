#ifndef _SYS_H_
#define _SYS_H_

#include "stdint.h"

class SYS {
   public:
    static void init(void);
    static int32_t execl(const char* path, const char** args, bool is_initial = false);
    static void exit_process(uint32_t exit_code);
};

#endif

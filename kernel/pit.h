#ifndef _PIT_H_
#define _PIT_H_

#include "atomic.h"
#include "debug.h"
#include "smp.h"
#include "stdint.h"

extern PerCPU<bool> kernel_interrupted;

class Pit {
    static uint32_t jiffiesPerSecond;
    static uint32_t apitCounter;

   public:
    static volatile uint32_t jiffies;
    static void calibrate(uint32_t hz);
    static void init();
    static uint32_t secondsToJiffies(uint32_t secs) {
        return jiffiesPerSecond * secs;
    }
    static uint32_t seconds(void) {
        return jiffies / jiffiesPerSecond;
        return 0;
    }
};

#endif

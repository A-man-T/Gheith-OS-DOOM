#ifndef _VMM_H_
#define _VMM_H_

#include "stdint.h"

namespace VMM {

// Called (on the initial core) to initialize data structures, etc
void global_init();

// Called on each core to do per-core initialization
void per_core_init();

uint32_t new_directory();

uint32_t set_up_stack(uint32_t n_args, uint32_t total_length, const char** args, uint32_t directory);

uint32_t clone_directory(uint32_t other);

void unmap_pages(uint32_t start, uint32_t size, uint32_t directory);

void free_directory(uint32_t directory);

void vmm_off();

}  // namespace VMM

#endif

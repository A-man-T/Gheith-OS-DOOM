#pragma once

#include "stdint.h"

namespace COW {

void init();
uint32_t ref(uint32_t* entry, bool ref_only = false);
void copy(uint32_t* entry, bool is_page_table);
void unref(uint32_t* entry, bool is_page_table);

}
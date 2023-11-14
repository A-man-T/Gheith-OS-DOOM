#include "cow.h"

#include "atomic.h"
#include "config.h"
#include "physmem.h"
#include "stdint.h"

namespace COW {

constexpr uint32_t FLAG_PR = 1 << 0;
constexpr uint32_t FLAG_RW = 1 << 1;
constexpr uint32_t PG_NUM_MASK = 0xFFFF'FFFF / PhysMem::FRAME_SIZE * PhysMem::FRAME_SIZE;
constexpr uint32_t FLAGS_MASK = ~PG_NUM_MASK;
constexpr uint32_t pointers_per_frame = PhysMem::FRAME_SIZE / 4;

static Atomic<uint32_t>* ref_counts;
static Atomic<uint8_t>* page_locks;

void init() {
    ref_counts = new Atomic<uint32_t>[kConfig.memSize / PhysMem::FRAME_SIZE];
    page_locks = new Atomic<uint8_t>[kConfig.memSize / PhysMem::FRAME_SIZE];
}

uint32_t ref(uint32_t* entry, bool ref_only) {
    if (!ref_only && (*entry & FLAG_PR) == 0) {
        return 0;
    }

    uint32_t page_number = *entry / PhysMem::FRAME_SIZE;

    if (!ref_only) {
        // reference to a valid entry
        if ((*entry & FLAG_RW) > 0) {
            ref_counts[page_number].add_fetch(1);
        }

        *entry &= ~FLAG_RW;
    }

    ref_counts[page_number].add_fetch(1);

    return *entry;
}

void copy(uint32_t* entry, bool is_page_table) {
    if ((*entry & FLAG_PR) == 0 || (*entry & FLAG_RW) > 0) {
        return;
    }

    uint32_t page_number = *entry / PhysMem::FRAME_SIZE;

    page_locks[page_number].add_fetch(1);

    if (ref_counts[page_number].add_fetch(-1) == 0) {
        page_locks[page_number].add_fetch(-1);
        while (page_locks[page_number].get() > 0) {
            pause();
        }

        // let copiers finish and then flip to RW
        *entry |= FLAG_RW;
    } else {
        // still references remaining on the page table, copy it
        uint32_t new_frame = PhysMem::alloc_frame();
        uint32_t* new_data = (uint32_t*)new_frame;
        uint32_t* current_data = (uint32_t*)(*entry & PG_NUM_MASK);

        if (is_page_table) {
            for (uint32_t i = 0; i < pointers_per_frame; i++) {
                new_data[i] = ref(current_data + i);
            }
        } else {
            memcpy(new_data, current_data, PhysMem::FRAME_SIZE);
        }

        page_locks[page_number].add_fetch(-1);
        *entry = new_frame | (*entry & FLAGS_MASK) | FLAG_RW;
    }
}

void unref(uint32_t* entry, bool is_page_table) {
    if ((*entry & FLAG_PR) == 0) {
        return;
    }

    uint32_t page_number = *entry / PhysMem::FRAME_SIZE;

    while (page_locks[page_number].get() > 0) {
        pause();
    }

    if ((*entry & FLAG_RW) > 0 || ref_counts[page_number].add_fetch(-1) == 0) {
        uint32_t address = *entry & PG_NUM_MASK;

        if (is_page_table) {
            uint32_t* table = (uint32_t*)address;
            for (uint32_t i = 0; i < pointers_per_frame; i++) {
                unref(table + i, false);
            }
        }

        PhysMem::dealloc_frame(address);
    }

    *entry = 0;
}

}  // namespace COW
#include "vmm.h"

#include "config.h"
#include "cow.h"
#include "debug.h"
#include "idt.h"
#include "kernel.h"
#include "libk.h"
#include "machine.h"
#include "physmem.h"
#include "sys.h"

// bit constants for PTE/PDE
constexpr uint32_t FLAG_PR = 1 << 0;   // present, 1 means the page table exists
constexpr uint32_t FLAG_RW = 1 << 1;   // read/write, 1 allows write
constexpr uint32_t FLAG_US = 1 << 2;   // user/supervisor, 1 allows user
constexpr uint32_t FLAG_PCD = 1 << 4;  // page cache disable, 1 disables cache
constexpr uint32_t FLAG_GL = 1 << 8;   // global; 1 means it's global, use for identity mapping

// bit masks/shifts for PTE/PDE
constexpr uint32_t PG_NUM_MASK = 0xFFFF'FFFF / PhysMem::FRAME_SIZE * PhysMem::FRAME_SIZE;
constexpr uint32_t FLAGS_MASK = ~PG_NUM_MASK;

namespace VMM {

// size constants
constexpr uint32_t pointers_per_frame = PhysMem::FRAME_SIZE / 4;
constexpr uint32_t ram_size = 128 * 1024 * 1024;
constexpr uint32_t ram_pages = ram_size / PhysMem::FRAME_SIZE;
constexpr uint32_t ram_page_dirs = ram_pages / pointers_per_frame;
constexpr uint32_t reserved_pages = 1;   // null pointer
constexpr uint32_t extra_page_dirs = 2;  // shared page, Local/IO APIC

// excludes shared page since these are used for functions that wouldn't copy/free it
constexpr uint32_t user_page_dir_start = USER_MEM_START / PhysMem::FRAME_SIZE / pointers_per_frame;
constexpr uint32_t user_page_dir_end = SHARED_ADDRESS / PhysMem::FRAME_SIZE / pointers_per_frame;

// align these so they're on page boundaries
__attribute__((aligned(PhysMem::FRAME_SIZE))) static uint32_t shared_frame[PhysMem::FRAME_SIZE / 4];
__attribute__((aligned(PhysMem::FRAME_SIZE))) static uint32_t zero_frame[PhysMem::FRAME_SIZE / 4];
__attribute__((aligned(PhysMem::FRAME_SIZE))) static uint32_t base_directory[pointers_per_frame];
__attribute__((aligned(PhysMem::FRAME_SIZE))) static uint32_t base_page_tables[ram_page_dirs + extra_page_dirs][pointers_per_frame];

uint32_t* get_page_directory_entry(uint32_t page, uint32_t* directory) {
    uint32_t dentry_number = page / pointers_per_frame;

    // this function is only ever used right before writing the page table, so
    // make sure it is copied
    COW::copy(directory + dentry_number, true);

    return directory + dentry_number;
}

uint32_t* get_page_table(uint32_t page, uint32_t* directory) {
    uint32_t entry = *get_page_directory_entry(page, directory);
    if (entry & FLAG_PR) {
        return (uint32_t*)(entry & PG_NUM_MASK);
    } else {
        return nullptr;
    }
}

void set_page_table(uint32_t page, uint32_t* page_table, uint32_t flags, uint32_t* directory) {
    *get_page_directory_entry(page, directory) = (uint32_t)page_table | flags;
}

void map_address(uint32_t from_address, uint32_t to_address, uint32_t flags, uint32_t* directory) {
    uint32_t from = from_address / PhysMem::FRAME_SIZE;

    uint32_t* page_table = get_page_table(from, directory);
    if (page_table == nullptr) {
        page_table = (uint32_t*)PhysMem::alloc_frame();
        set_page_table(from, page_table, flags, directory);
    }

    page_table[from % pointers_per_frame] = (to_address & PG_NUM_MASK) | flags;
}

void set_up_identity_dir(uint32_t start_page, uint32_t end_page, uint32_t* page_table) {
    // enable read/write, kernel mode only
    set_page_table(start_page, page_table, FLAG_PR | FLAG_RW, base_directory);

    for (uint32_t i = start_page; i < end_page; i++) {
        map_address(i * PhysMem::FRAME_SIZE, i * PhysMem::FRAME_SIZE, FLAG_PR | FLAG_RW | FLAG_GL, base_directory);
    }
}

void ref_zero_frame() {
    uint32_t frame = (uint32_t)zero_frame;
    COW::ref(&frame, true);
}

void global_init() {
    uint32_t base_pt_index = 0;
    uint32_t full_ram_page_tables = PhysMem::frameup(kConfig.memSize) / PhysMem::FRAME_SIZE;
    uint32_t full_ram_page_dirs = (full_ram_page_tables + pointers_per_frame - 1) / pointers_per_frame;

    for (uint32_t i = 0; i < full_ram_page_dirs; i++) {
        uint32_t start_page = i * pointers_per_frame;
        uint32_t end_page = start_page + pointers_per_frame;
        if (i == 0) {
            start_page += reserved_pages;
        }

        uint32_t* page_table =
            i < ram_page_dirs
                ? base_page_tables[base_pt_index++]
                : (uint32_t*)PhysMem::alloc_frame();
        set_up_identity_dir(start_page, end_page, page_table);
    }

    set_page_table(SHARED_ADDRESS / PhysMem::FRAME_SIZE, base_page_tables[base_pt_index++], FLAG_PR | FLAG_RW | FLAG_US, base_directory);
    map_address(SHARED_ADDRESS, (uint32_t)shared_frame, FLAG_PR | FLAG_RW | FLAG_US | FLAG_GL, base_directory);

    set_page_table(kConfig.localAPIC / PhysMem::FRAME_SIZE, base_page_tables[base_pt_index++], FLAG_PR | FLAG_RW | FLAG_PCD, base_directory);
    map_address(kConfig.localAPIC, kConfig.localAPIC, FLAG_PR | FLAG_RW | FLAG_PCD | FLAG_GL, base_directory);
    map_address(kConfig.ioAPIC, kConfig.ioAPIC, FLAG_PR | FLAG_RW | FLAG_PCD | FLAG_GL, base_directory);

    COW::init();

    // ensure that someone always keeps a ref so zero page isn't freed
    ref_zero_frame();
}

void per_core_init() {
    vmm_on((uint32_t)base_directory);
    vmm_global_on();
}

uint32_t new_directory() {
    uint32_t directory = PhysMem::alloc_frame();
    memcpy((char*)directory, base_directory, PhysMem::FRAME_SIZE);
    return directory;
}

template <typename T>
void write_stack_data(T data, uint32_t& address, uint32_t& current_block, uint32_t* directory) {
    if (current_block == 0 || address % PhysMem::FRAME_SIZE == 0) {
        uint32_t page = address / PhysMem::FRAME_SIZE;
        uint32_t* page_table = get_page_table(page, directory);
        uint32_t page_table_entry = page % pointers_per_frame;
        current_block = 0;
        if (page_table != nullptr && (page_table[page_table_entry] & FLAG_PR) > 0) {
            current_block = page_table[page_table_entry] & PG_NUM_MASK;
        }

        if (current_block == 0) {
            current_block = PhysMem::alloc_frame();
            map_address(address, current_block, FLAG_PR | FLAG_US | FLAG_RW, directory);
        }
    }

    // it is fine to assume page alignment here; guaranteed by set_up_stack
    ((T*)current_block)[(address & ~PG_NUM_MASK) / sizeof(T)] = data;
    address += sizeof(T);
}

uint32_t set_up_stack(uint32_t directory_address, uint32_t total_length, char* const argv[], uint32_t n_args, char* const envp[], uint32_t n_envs) {
    uint32_t* directory = (uint32_t*)directory_address;

    // argc, argv**, envp**, argv*[i] + nullptr, envp*[i] + nullptr
    uint32_t pointer_stack_space = 4 + 4 + 4 + (4 * (n_args + 1)) + (4 * (n_envs + 1));
    uint32_t stack_space_needed = pointer_stack_space + total_length;
    uint32_t stack_pointer = (USER_STACK_START - stack_space_needed) & 0xFFFF'FFF0;

    uint32_t current_pointer_block = 0;
    uint32_t current_data_block = 0;
    uint32_t pointer_address = stack_pointer;
    uint32_t data_address = stack_pointer + pointer_stack_space;

    write_stack_data(n_args, pointer_address, current_pointer_block, directory);                                   // argc
    write_stack_data(stack_pointer + 12, pointer_address, current_pointer_block, directory);                       // argv**
    write_stack_data(stack_pointer + 12 + (4 * (n_args + 1)), pointer_address, current_pointer_block, directory);  // envp**

    for (uint32_t i = 0; i < n_args; i++) {
        write_stack_data(data_address, pointer_address, current_pointer_block, directory);  // argv*

        for (uint32_t j = 0; argv[i][j] != 0; j++) {
            write_stack_data(argv[i][j], data_address, current_data_block, directory);  // char* data
        }
        write_stack_data<char>(0, data_address, current_data_block, directory);  // char* null terminator
    }
    write_stack_data<uint32_t>(0, pointer_address, current_pointer_block, directory);  // argv null terminator

    for (uint32_t i = 0; i < n_envs; i++) {
        write_stack_data(data_address, pointer_address, current_pointer_block, directory);  // argv*

        for (uint32_t j = 0; envp[i][j] != 0; j++) {
            write_stack_data(envp[i][j], data_address, current_data_block, directory);  // char* data
        }
        write_stack_data<char>(0, data_address, current_data_block, directory);  // char* null terminator
    }
    write_stack_data<uint32_t>(0, pointer_address, current_pointer_block, directory);  // envp null terminator

    return stack_pointer;
}

uint32_t clone_directory_no_cow(uint32_t other) {
    other &= PG_NUM_MASK;
    uint32_t* current_dir = (uint32_t*)other;
    uint32_t* new_dir = (uint32_t*)PhysMem::alloc_frame();

    for (uint32_t i = 0; i < pointers_per_frame; i++) {
        uint32_t dir_entry = current_dir[i];

        if ((dir_entry & FLAG_PR) == 0 || i < user_page_dir_start || i >= user_page_dir_end) {
            new_dir[i] = dir_entry;
        } else {
            uint32_t new_pt_frame = PhysMem::alloc_frame();
            uint32_t* current_pt = (uint32_t*)(dir_entry & PG_NUM_MASK);
            uint32_t* new_pt = (uint32_t*)new_pt_frame;

            for (uint32_t j = 0; j < pointers_per_frame; j++) {
                uint32_t pt_entry = current_pt[j];

                if ((pt_entry & FLAG_PR) == 0) {
                    new_pt[j] = current_pt[j];
                } else {
                    uint32_t new_data_frame = PhysMem::alloc_frame();
                    memcpy((char*)new_data_frame, (char*)(pt_entry & PG_NUM_MASK), PhysMem::FRAME_SIZE);
                    new_pt[j] = new_data_frame | (pt_entry & FLAGS_MASK);
                }
            }

            new_dir[i] = new_pt_frame | (dir_entry & FLAGS_MASK);
        }
    }

    return (uint32_t)new_dir;
}

uint32_t clone_directory(uint32_t other) {
    other &= PG_NUM_MASK;
    uint32_t* current_dir = (uint32_t*)other;
    uint32_t* new_dir = (uint32_t*)PhysMem::alloc_frame();

    for (uint32_t i = 0; i < pointers_per_frame; i++) {
        uint32_t dir_entry = current_dir[i];

        if ((dir_entry & FLAG_PR) == 0 || i < user_page_dir_start || i >= user_page_dir_end) {
            new_dir[i] = dir_entry;
        } else {
            new_dir[i] = COW::ref(current_dir + i);
        }
    }

    // invalidate the TLB entries for userspace; they are now read-only
    vmm_on(other);

    return (uint32_t)new_dir;
}

void unmap_pages(uint32_t start, uint32_t size, uint32_t directory) {
    if (size == 0) {
        return;
    }

    uint32_t end = start + size - 1;
    uint32_t start_page = start / PhysMem::FRAME_SIZE;
    uint32_t end_page = end / PhysMem::FRAME_SIZE;
    uint32_t start_dir = start_page / pointers_per_frame;
    uint32_t end_dir = end_page / pointers_per_frame;

    uint32_t* tables = (uint32_t*)directory;

    for (uint32_t pdi = start_dir; pdi <= end_dir; pdi++) {
        // only the start and end might be partial
        if (pdi == start_dir || pdi == end_dir) {
            // find the first and last PTE that are included in this range within the current PT
            uint32_t pt_start_page = pdi * pointers_per_frame;
            uint32_t pt_end_page = (pdi + 1) * pointers_per_frame - 1;
            uint32_t start_pte = (start_page > pt_start_page ? start_page : pt_start_page) % pointers_per_frame;
            uint32_t end_pte = (end_page < pt_end_page ? end_page : pt_end_page) % pointers_per_frame;

            // if there is a page table here, check if any entries need to be preserved before copying
            if ((tables[pdi] & FLAG_PR) > 0) {
                uint32_t out_of_range = 0;
                uint32_t* table = (uint32_t*)(tables[pdi] & PG_NUM_MASK);

                for (uint32_t pti = 0; pti < pointers_per_frame; pti++) {
                    if (pti >= start_pte && pti <= end_pte) {
                        continue;
                    }
                    if ((table[pti] & FLAG_PR) > 0) {
                        out_of_range++;
                    }
                }

                // only copy if there is important data, otherwise unref all
                if (out_of_range > 0) {
                    COW::copy(tables + pdi, true);
                    for (uint32_t pti = start_pte; pti <= end_pte; pti++) {
                        COW::unref(table + pti, false);
                    }
                    continue;
                }
            }
        }

        // if it was not partial, free the full dir
        COW::unref(tables + pdi, true);
    }

    // flush TLB after unmapping
    vmm_on(getCR3());
}

void free_directory(uint32_t directory_address) {
    directory_address &= PG_NUM_MASK;
    uint32_t* directory = (uint32_t*)directory_address;
    if (directory == nullptr || directory == base_directory) {
        return;
    }

    for (uint32_t i = user_page_dir_start; i < user_page_dir_end; i++) {
        COW::unref(directory + i, true);
    }

    PhysMem::dealloc_frame(directory_address);
}

void vmm_off() {
    vmm_on((uint32_t)base_directory);
}

}  // namespace VMM

void segfault(uint32_t address) {
    *(uint32_t*)FAULT_ADDRESS = address;
    active_processes.mine()->run_signal_handler(1, address);
    SYS::exit_process(139);
}

constexpr uint32_t FAULT_FLAG_PR = 1 << 0;  // 1 if the page was present; fault occurred for another reason
constexpr uint32_t FAULT_FLAG_WR = 1 << 1;  // 1 if faulted on write
constexpr uint32_t FAULT_FLAG_US = 1 << 2;  // 1 if user mode code triggered the fault

void wrapped_page_fault(uint32_t address, uint32_t fault, uint32_t* interrupt_frame) {
    auto process = active_processes.mine();
    bool is_present = fault & FAULT_FLAG_PR;
    bool is_write = fault & FAULT_FLAG_WR;
    bool is_user = fault & FAULT_FLAG_US;

    // Debug::printf("fault: %d %d %d %x (from: %x)\n", is_present, is_write, is_user, address, interrupt_frame[0]);

    if (address < USER_MEM_START || address >= USER_MEM_END) {
        if (is_user) {
            segfault(address);
        } else {
            Debug::panic("kernel shouldn't page fault... (addr: %x, from: %x)\n", address, interrupt_frame[0]);
        }
    } else {
        uint32_t* directory = (uint32_t*)(getCR3() & PG_NUM_MASK);

        if (is_present && is_write) {
            // copy on write
            uint32_t page_number = address / PhysMem::FRAME_SIZE;
            uint32_t* page_table = VMM::get_page_table(page_number, directory);

            COW::copy(page_table + (page_number % VMM::pointers_per_frame), false);
        } else {
            auto result = process->memory_mapping.check_address(address, true);
            if (!result.exists) {
                if (is_user) {
                    segfault(address);
                } else {
                    // return value would've already been set to -1, or a different error code
                    process->run();
                }
            }

            if (is_write || result.contents != 0) {
                // on a write or if the contents came from a file, map the page as writable
                if (result.contents == 0) {
                    result.contents = PhysMem::alloc_frame();
                }
                VMM::map_address(address, result.contents, FLAG_PR | FLAG_RW | FLAG_US, directory);
            } else {
                // if the fault was on a read, point it to the zero frame instead of allocating a new frame
                VMM::ref_zero_frame();
                VMM::map_address(address, (uint32_t)VMM::zero_frame, FLAG_PR | FLAG_US, directory);
            }
        }
    }
}

extern "C" void vmm_pageFault(uint32_t address, uint32_t* frame) {
    auto process = active_processes.mine();
    uint32_t* interrupt_frame = frame + 9;
    uint32_t* register_frame = frame;
    uint32_t fault = frame[8];

    bool is_user = fault & FAULT_FLAG_US;
    if (is_user) {
        process->save_state(interrupt_frame, register_frame);
    }

    invlpg(address);
    wrapped_page_fault(address, fault, interrupt_frame);

    if (is_user) {
        process->run();
    }
}

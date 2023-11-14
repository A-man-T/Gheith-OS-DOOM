#ifndef _KERNEL_H_
#define _KERNEL_H_

#include "elf.h"
#include "ext2.h"
#include "future.h"
#include "physmem.h"
#include "process.h"

inline constexpr uint32_t USER_MEM_START = 0x8000'0000;
inline constexpr uint32_t SHARED_ADDRESS = 0xF000'0000;
inline constexpr uint32_t FAULT_ADDRESS = SHARED_ADDRESS + 0x800;
inline constexpr uint32_t USER_MEM_END = SHARED_ADDRESS + PhysMem::FRAME_SIZE;

inline constexpr uint32_t USER_STACK_SIZE = 1 * 1024 * 1024;
inline constexpr uint32_t USER_STACK_START = SHARED_ADDRESS;
inline constexpr uint32_t USER_STACK_END = USER_STACK_START - USER_STACK_SIZE;

extern Ide* d;
extern Ext2* fs;
extern PerCPU<Process*> active_processes;

bool check_bounds(uint32_t address, uint32_t offset, uint32_t scale, uint32_t lower_bound, uint32_t upper_bound);
bool validate_address(uint32_t address, uint32_t offset = 0, uint32_t scale = 1);

template <typename T>
bool validate_address(const T* pointer, uint32_t offset = 1) {
    uint32_t address = reinterpret_cast<uint32_t>(pointer);

    return validate_address(address, 0) && (offset == 0 || validate_address(address + sizeof(T) - 1, offset - 1, sizeof(T)));
}

Node* find_fs_node(const char* path, Node* relative_path = nullptr, uint32_t depth = 40);
Future<int> kernelMain(void);

#endif

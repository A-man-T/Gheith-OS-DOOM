#pragma once

#include "bit_tree.h"
#include "file_descriptor.h"
#include "physmem.h"
#include "shared.h"
#include "stdint.h"

class MemoryMapping {
   public:
    struct VirtualMemoryEntry {
        uint32_t start;
        uint32_t size;

        Shared<FileDescriptor> file;
        uint32_t file_offset;
        uint32_t file_size;
    };

    struct UnmapResult {
        uint32_t start{0};
        uint32_t size{0};
    };

    struct CheckResult {
        bool exists{false};
        uint32_t contents{0};
    };

   private:
    BitTree<VirtualMemoryEntry, 4, 0x8000'0000, PhysMem::FRAME_SIZE> vme_tree;

   public:
    uint32_t map(uint32_t start, uint32_t size);
    uint32_t map(uint32_t start, uint32_t size, Shared<FileDescriptor> fd, uint32_t file_offset, uint32_t file_size);
    UnmapResult unmap(uint32_t address);
    CheckResult check_address(uint32_t address, bool get_contents);

    void clear();
};
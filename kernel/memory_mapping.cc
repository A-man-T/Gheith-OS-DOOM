#include "memory_mapping.h"

#include "errno.h"
#include "kernel.h"
#include "physmem.h"
#include "unique_ptr.h"

constexpr uint32_t PG_NUM_MASK = 0xFFFF'FFFF / PhysMem::FRAME_SIZE * PhysMem::FRAME_SIZE;

uint32_t MemoryMapping::map(uint32_t start, uint32_t size) {
    return map(start, size, {}, 0, 0);
}

uint32_t MemoryMapping::map(uint32_t start, uint32_t size, Shared<FileDescriptor> fd, uint32_t file_offset, uint32_t file_size) {
    if (size == 0 || start % PhysMem::FRAME_SIZE != 0 || size % PhysMem::FRAME_SIZE != 0) {
        return -1 * EINVAL;
    }

    if (start != 0) {
        // user addresses only
        if (start < USER_MEM_START || !check_bounds(start, size, 1, USER_MEM_START, SHARED_ADDRESS + 1)) {
            return -1 * EINVAL;
        }

        if (check_address(start, false).exists) {
            return -1 * EINVAL;
        }
        if (check_address(start + size - 1, false).exists) {
            return -1 * EINVAL;
        }
    } else {
        uint32_t page = USER_MEM_START;
        while (page + size > size && page + size <= SHARED_ADDRESS) {
            auto entry = vme_tree.lookup_ceil(page);
            if (entry == nullptr || page + size <= entry->start) {
                start = page;
                break;
            }
            page = entry->start + entry->size;
        }
        if (start == 0) {
            return -1 * ENOMEM;
        }
    }

    vme_tree.insert(start,
                    new VirtualMemoryEntry(
                        start, size,
                        fd, file_offset, file_size));

    return start;
}

MemoryMapping::UnmapResult MemoryMapping::unmap(uint32_t address) {
    auto entry = vme_tree.lookup_floor(address);
    if (entry == nullptr || address >= entry->start + entry->size) {
        return {};
    }

    uint32_t start = entry->start;
    uint32_t size = entry->size;
    delete vme_tree.remove(start);

    return {start, size};
}

MemoryMapping::CheckResult MemoryMapping::check_address(uint32_t address, bool get_contents) {
    auto entry = vme_tree.lookup_floor(address);
    bool exists = entry != nullptr && address < entry->start + entry->size;
    if (!get_contents || !exists) {
        return {exists};
    }

    uint32_t page = address & PG_NUM_MASK;
    uint32_t begin_page = entry->start & PG_NUM_MASK;
    uint32_t end_page = (entry->start + entry->file_size - 1) & PG_NUM_MASK;

    if (entry->file_size > 0 && page >= begin_page && page <= end_page) {
        uint32_t file_read_start = entry->file_offset + (page - entry->start);
        uint32_t memory_offset = 0;

        if (page < entry->start) {
            file_read_start = entry->file_offset;
            memory_offset = entry->start - page;
        }

        uint32_t file_read_end = file_read_start + PhysMem::FRAME_SIZE - memory_offset;
        if (file_read_end > entry->file_offset + entry->file_size) {
            file_read_end = entry->file_offset + entry->file_size;
        }

        uint32_t contents = PhysMem::alloc_frame();
        entry->file->read(
            (char*)(contents + memory_offset),
            file_read_end - file_read_start,
            [](auto _) { MISSING(); },
            file_read_start);

        return {true, contents};
    }

    return {true, 0};
}

void MemoryMapping::clear() {
    vme_tree.clear();
}
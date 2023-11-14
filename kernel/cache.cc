#include "cache.h"

CachedBlockIO::CachedBlockIO(BlockIO* device)
    : BlockIO(device->block_size), device(device), data{new char[block_size * CACHE_SIZE]} {}

uint32_t CachedBlockIO::size_in_bytes() {
    return device->size_in_bytes();
}

void CachedBlockIO::read_block(uint32_t block_number, char* buffer) {
    uint32_t index = block_number % CACHE_SIZE;
    auto& entry = cache[index];
    char* cached = data + index * block_size;

    {
        LockGuard _{entry.lock};
        if (entry.block == block_number) {
            memcpy(buffer, cached, block_size);
            return;
        }
    }

    device->read_block(block_number, buffer);

    {
        LockGuard _{entry.lock};
        entry.block = block_number;
        memcpy(cached, buffer, block_size);
    }
}

CachedBlockIO::~CachedBlockIO() {
    delete[] data;
}
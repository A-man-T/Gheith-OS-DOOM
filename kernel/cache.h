#pragma once

#include "atomic.h"
#include "ide.h"
#include "stdint.h"

class CachedBlockIO : public BlockIO {
   private:
    static constexpr uint32_t CACHE_SIZE = 32;

    struct CacheEntry {
        SpinLock lock;
        uint32_t block{0xFFFF'FFFF};
    };

    BlockIO* device;
    CacheEntry cache[CACHE_SIZE];
    char* data;

   public:
    CachedBlockIO(BlockIO* ide);

    uint32_t size_in_bytes() override;

    void read_block(uint32_t block_number, char* buffer) override;

    virtual ~CachedBlockIO();
};
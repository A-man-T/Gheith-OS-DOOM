#include "kernel.h"

#include "elf.h"
#include "ext2.h"
#include "ide.h"
#include "libk.h"
#include "machine.h"
#include "physmem.h"
#include "sys.h"
#include "unique_ptr.h"

Ide* d;
Ext2* fs;
PerCPU<Process*> active_processes;

bool check_bounds(uint32_t base, uint32_t index, uint32_t scale, uint32_t lower_bound, uint32_t upper_bound) {
    // check for scale overflow
    uint32_t scaled = index * scale;
    if (scaled / scale != index) {
        return false;
    }

    // check for offset overflow
    uint32_t final_value = base + scaled;
    if (final_value < base) {
        return false;
    }

    return final_value >= lower_bound && final_value < upper_bound;
}

bool validate_address(uint32_t address, uint32_t offset, uint32_t scale) {
    if (offset > 0 && !validate_address(address)) {
        return false;
    }

    return check_bounds(address, offset, scale, USER_MEM_START, USER_MEM_END);
}

Node* find_fs_node(const char* path, Node* relative_path, uint32_t depth) {
    if (depth == 0 || path[0] == 0) {
        return nullptr;
    }

    if (path[0] == '/') {
        relative_path = fs->root;
        path++;
    } else if (relative_path == nullptr) {
        return nullptr;
    }

    auto current_dir = UniquePtr(new Node(relative_path));
    auto* current_start = path;
    for (auto* c = path; c == path || c[-1] != 0; c++) {
        if (*c == 0 || *c == '/') {
            if (!current_dir->is_dir()) {
                return nullptr;
            }

            auto length = c - current_start;

            // handle double slash case
            if (length == 0) {
                current_start = c + 1;
                continue;
            }

            if (length == 1 && *current_start == '.') {
                current_start = c + 1;
                continue;
            }

            auto name = UniquePtr<char, char[]>(new char[length + 1]);
            memcpy(name, current_start, length);
            name[length] = 0;
            current_start = c + 1;

            auto next_dir = UniquePtr(fs->find(current_dir, name));
            while (true) {
                if (next_dir == nullptr) {
                    return nullptr;
                } else if (next_dir->is_symlink()) {
                    uint32_t symbol_size = next_dir->size_in_bytes();
                    auto symbol = UniquePtr<char, char[]>(new char[symbol_size + 1]);
                    symbol[symbol_size] = 0;
                    next_dir->get_symbol(symbol);
                    next_dir = find_fs_node(symbol, current_dir, depth - 1);
                } else {
                    break;
                }
            }

            current_dir = next_dir;
        }
    }

    return current_dir.release();
}

Future<int> kernelMain(void) {
    d = new Ide(1);
    fs = new Ext2(d);

    const char* args[] = {"/sbin/init", nullptr};
    SYS::execl("/sbin/init", args, true);

    co_return -1;
}

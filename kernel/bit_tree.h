#pragma once

#include "atomic.h"
#include "stdint.h"

// credit to Ronit Shah for the idea of this data structure

struct BitTreeNode {
    SpinLock lock;
    uint32_t ref_count{1};
    uint32_t present_bits{0};
    void* pointers[32]{nullptr};

    BitTreeNode() {}

    BitTreeNode(BitTreeNode* other, bool is_last_level = false) : present_bits{other->present_bits} {
        for (uint32_t i = 0; i < 32; i++) {
            pointers[i] = other->pointers[i];
            auto child = (BitTreeNode*)pointers[i];
            if (!is_last_level && child != nullptr) {
                LockGuard _{child->lock};
                child->ref_count++;
            }
        }
    }

    template <typename FreeData>
    bool free(uint32_t level, FreeData free_data) {
        LockGuard _{lock};
        if (--ref_count > 0) {
            return false;
        }

        for (uint32_t i = 0; i < 32; i++) {
            if (pointers[i] == nullptr) {
                continue;
            }

            if (level == 1) {
                free_data(pointers[i]);
            } else {
                auto child = (BitTreeNode*)pointers[i];
                if (child->free(level - 1, free_data)) {
                    delete child;
                }
            }
        }

        return true;
    }
};

template <typename T, uint32_t Levels, uint32_t Offset = 0, uint32_t Divisor = 1>
class BitTree {
   private:
    enum class GetEntryAction {
        Insert,
        Lookup,
        Delete
    };

    BitTreeNode root;
    uint32_t size{Levels == 0 ? 0 : 1};

   public:
    BitTree() {
        for (uint32_t i = 0; i < Levels; i++) {
            size *= 32;
        }
    }

    BitTree(BitTree& other)
        : root{&other.root}, size{other.size} {}

    ~BitTree() {
        free_node(&root, Levels);
    }

    void clear() {
        free_node(&root, Levels);
        root = {};
    }

    // return previous value
    T* insert(uint32_t index, T* item) {
        if (!adjust_index(index)) {
            return nullptr;
        }

        T** ref = get_entry_ref(index, GetEntryAction::Insert);
        T* previous = *ref;
        *ref = item;

        return previous;
    }

    T* lookup_exact(uint32_t index) {
        if (!adjust_index(index)) {
            return nullptr;
        }

        T** ref = get_entry_ref(index, GetEntryAction::Lookup);
        if (ref == nullptr) {
            return nullptr;
        }

        return *ref;
    }

    T* lookup_floor(uint32_t index) {
        if (!adjust_index(index)) {
            return nullptr;
        }

        return run_query(index, true, &root);
    }

    T* lookup_ceil(uint32_t index) {
        if (!adjust_index(index)) {
            return nullptr;
        }

        return run_query(index, false, &root);
    }

    T* remove(uint32_t index) {
        if (!adjust_index(index)) {
            return nullptr;
        }

        T** ref = get_entry_ref(index, GetEntryAction::Delete);
        if (ref == nullptr) {
            return nullptr;
        }

        T* previous = *ref;
        *ref = nullptr;

        return previous;
    }

   private:
    bool adjust_index(uint32_t& index) {
        if (index < Offset) {
            return false;
        }
        index = (index - Offset) / Divisor;
        return index < size;
    }

    T** get_entry_ref(uint32_t index, GetEntryAction action, uint32_t level = Levels, BitTreeNode** node_ref = nullptr) {
        BitTreeNode* root_node = &root;
        if (node_ref == nullptr) {
            node_ref = &root_node;
        }
        BitTreeNode* node = *node_ref;

        bool is_adding = action == GetEntryAction::Insert;
        bool is_removing = action == GetEntryAction::Delete;
        bool is_editing = is_adding || is_removing;

        // copy on write
        if (is_editing) {
            LockGuard _{node->lock};
            if (node->ref_count > 1) {
                node->ref_count--;
                node = new BitTreeNode(node, level == 1);
                if (level == 1) {
                    for (uint32_t i = 0; i < 32; i++) {
                        if (node->pointers[i] != nullptr) {
                            node->pointers[i] = new T(*(T*)node->pointers[i]);
                        }
                    }
                }
                *node_ref = node;
            }
        }

        uint32_t level_index = get_level_index(index, level);

        if (level == 1) {
            if (is_adding) {
                node->present_bits |= 1 << level_index;
            } else if (is_removing) {
                node->present_bits &= ~(1 << level_index);
            }

            return (T**)(node->pointers + level_index);
        }

        auto next_ref = (BitTreeNode**)(node->pointers + level_index);
        if (node->pointers[level_index] == nullptr) {
            if (!is_adding) {
                return nullptr;
            }

            BitTreeNode* new_node = new BitTreeNode();
            node->present_bits |= 1 << level_index;
            node->pointers[level_index] = new_node;
        }

        T** result = get_entry_ref(index, action, level - 1, next_ref);
        if (is_removing) {
            if ((*next_ref)->present_bits == 0) {
                node->present_bits &= ~(1 << level_index);
                free_node((BitTreeNode*)node->pointers[level_index], level - 1, false);
                node->pointers[level_index] = nullptr;
            }
        }
        return result;
    }

    T* run_query(uint32_t index, bool is_floor, BitTreeNode* node, uint32_t level = Levels) {
        if (node == nullptr) {
            return nullptr;
        }

        uint32_t level_index = get_level_index(index, level);
        uint32_t query_index = compute_index(node->present_bits, level_index, is_floor);
        if (query_index == 32) {
            return nullptr;
        }

        if (level == 1) {
            return (T*)node->pointers[query_index];
        }

        uint32_t move_index = is_floor ? 0xFFFF'FFFF : 0;
        uint32_t new_index = level_index == query_index ? index : move_index;

        T* result = run_query(new_index, is_floor, (BitTreeNode*)node->pointers[query_index], level - 1);
        if (result == nullptr) {
            uint32_t retry_index = compute_index(node->present_bits, query_index + (is_floor ? -1 : 1), is_floor);
            if (retry_index == 32) {
                return nullptr;
            }
            return run_query(move_index, is_floor, (BitTreeNode*)node->pointers[retry_index], level - 1);
        } else {
            return result;
        }
    }

    uint32_t get_level_index(uint32_t index, uint32_t level) {
        return (index >> (5 * (level - 1))) & 0b11111;
    }

    uint32_t compute_index(uint32_t bits, uint32_t index, bool is_floor) {
        if (index > 31) {
            return 32;
        }

        uint32_t masked = mask_bits(bits, index, is_floor);
        if (masked == 0) {
            return 32;
        }

        if (is_floor) {
            return 31 - __builtin_clz(masked);
        } else {
            return __builtin_ctz(masked);
        }
    }

    uint32_t mask_bits(uint32_t bits, uint32_t index, bool is_floor) {
        if (is_floor) {
            index = 31 - index;
            return bits << index >> index;
        } else {
            return bits >> index << index;
        }
    }

    void free_node(BitTreeNode* node, uint32_t level, bool free_data = true) {
        node->free(
            level,
            [free_data](void* pointer) {
                if (free_data) {
                    delete (T*)pointer;
                }
            });

        if (node != &root) {
            delete node;
        }
    }
};
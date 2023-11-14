#include "ext2.h"

#include "libk.h"

constexpr uint32_t root_inode = 2;

constexpr uint32_t superblock_offset = 1024;
constexpr uint32_t sb_inode_count_offset = 0;
constexpr uint32_t sb_block_size_offset = 24;
constexpr uint32_t sb_inodes_per_group_offset = 40;
constexpr uint32_t sb_inode_size_offset = 88;

constexpr uint32_t block_group_descriptor_size = 32;
constexpr uint32_t bg_inode_block_offset = 8;

template <typename T>
void get_val(char* buffer, uint32_t offset, T& output) {
    memcpy(&output, buffer + offset, sizeof(output));
}

Ext2::Ext2(Ide* ide) : disk(ide) {
    char buffer[sb_inode_size_offset + sizeof(inode_size)];
    disk.read(superblock_offset, buffer);

    uint32_t n_inodes;
    get_val(buffer, sb_inode_count_offset, n_inodes);
    get_val(buffer, sb_inodes_per_group_offset, n_inodes_per_block_group);
    get_val(buffer, sb_block_size_offset, block_size);
    block_size = 1024 << block_size;
    get_val(buffer, sb_inode_size_offset, inode_size);
    n_block_groups = (n_inodes + n_inodes_per_block_group - 1) / n_inodes_per_block_group;

    // block group descriptors start in the first block after the superblock
    uint32_t bgdt_start = get_block_offset(1024 / block_size + 1);

    auto bgdt_buffer = new char[disk.block_size];
    uint32_t processed = 0;
    group_inode_start_blocks = new uint32_t[n_block_groups];
    for (uint32_t i = 0; i < n_block_groups; i++) {
        uint32_t bgd_offset = i * block_group_descriptor_size;

        // this should work because block size should always be divisible by 32
        if (processed % disk.block_size == 0) {
            disk.read(bgdt_start + bgd_offset, disk.block_size, bgdt_buffer);
        }

        get_val(bgdt_buffer, bgd_offset - processed + bg_inode_block_offset, group_inode_start_blocks[i]);
        processed += block_group_descriptor_size;
    }
    delete[] bgdt_buffer;

    root = get_node(root_inode);
}

uint32_t Ext2::get_block_size() {
    return block_size;
}

uint32_t Ext2::get_inode_size() {
    return inode_size;
}

Node* Ext2::get_node(uint32_t number) {
    if (number == 0) {
        return nullptr;
    } else {
        return new Node(this, number);
    }
}

Node* Ext2::find(Node* dir, const char* name) {
    return get_node(dir->find(name));
}

uint32_t Ext2::get_block_offset(uint32_t block_number) {
    return block_number * block_size;
}

uint32_t Ext2::get_inode_offset(uint32_t inode_number) {
    // 1 indexing
    inode_number--;

    uint32_t block_group = inode_number / n_inodes_per_block_group;
    uint32_t block_group_offset = group_inode_start_blocks[block_group];
    uint32_t local_index = inode_number - block_group * n_inodes_per_block_group;

    return block_group_offset * block_size + local_index * inode_size;
}

Ext2::~Ext2() {
    delete root;
    delete group_inode_start_blocks;
}

///////////// Node /////////////

constexpr uint32_t node_type_offset = 0;
constexpr uint32_t node_size_offset = 4;
constexpr uint32_t node_link_count_offset = 26;
constexpr uint32_t node_block_arr_offset = 40;

Node::Node(Ext2* fs, uint32_t number) : BlockIO(fs->get_block_size()), fs(fs), number(number) {
    uint32_t offset = fs->get_inode_offset(number);
    char data[node_block_arr_offset + sizeof(block_pointers)];
    fs->disk.read(offset, data);

    get_val(data, node_type_offset, type);
    get_val(data, node_size_offset, size);
    get_val(data, node_link_count_offset, link_count);
    get_val(data, node_block_arr_offset, block_pointers);

    type >>= 12;
}

Node::Node(Node* other)
    : BlockIO(other->fs->get_block_size()),
      fs(other->fs),
      type(other->type),
      link_count(other->link_count),
      size(other->size),
      saved_entry_count(other->saved_entry_count),
      number(other->number) {
    memcpy(block_pointers, other->block_pointers, sizeof(block_pointers));
}

uint32_t Node::size_in_bytes() {
    return size;
}

void Node::read_block(uint32_t number, char* buffer) {
    uint32_t real_number = get_data_block(number);
    if (real_number != 0) {
        fs->disk.read_all(fs->get_block_offset(real_number), block_size, buffer);
    } else {
        for (uint32_t i = 0; i < block_size; i++) {
            buffer[i] = 0;
        }
    }
}

uint16_t Node::get_type() {
    return type;
}

bool Node::is_dir() {
    return type == 0x4;
}

bool Node::is_file() {
    return type == 0x8;
}

bool Node::is_symlink() {
    return type == 0xA;
}

void Node::get_symbol(char* buffer) {
    ASSERT(is_symlink());
    if (size >= 60) {
        read_all(0, size, buffer);
    } else {
        memcpy(buffer, block_pointers, size);
    }
}

uint32_t Node::n_links() {
    return link_count;
}

// this function is UB and calling it is invalid...
void Node::show(const char* msg) {
    MISSING();
}

struct FolderLLNode {
    uint32_t inode;
    uint16_t node_length;
    uint16_t name_length;
};

// if name is nullptr, then count the entries instead
//   reduces code repetition in the count function
uint32_t Node::find(const char* name) {
    ASSERT(is_dir());

    uint32_t entries = 0;
    uint32_t processed = 0;
    FolderLLNode node;

    uint32_t desired_length = 0;
    if (name != nullptr) {
        desired_length = K::strlen(name);
    }

    auto buffer = new char[block_size];
    for (uint32_t i = 0; processed < size; i++) {
        uint32_t local_processed = 0;
        read_block(i, buffer);

        while (local_processed < block_size && processed < size) {
            get_val(buffer, local_processed, node);

            char* file_name = buffer + local_processed + sizeof(node);
            processed += node.node_length;
            local_processed += node.node_length;

            if (node.inode == 0) {
                continue;
            }
            entries++;

            if (name != nullptr) {
                if (node.name_length == desired_length) {
                    bool matches = true;
                    for (uint32_t i = 0; i < desired_length; i++) {
                        if (file_name[i] != name[i]) {
                            matches = false;
                            break;
                        }
                    }
                    if (matches) {
                        delete[] buffer;
                        return node.inode;
                    }
                }
            }
        }
    }
    delete[] buffer;

    saved_entry_count = entries;

    if (name != nullptr) {
        return 0;
    } else {
        return entries;
    }
}

uint32_t Node::entry_count() {
    if (saved_entry_count == 0xFFFF'FFFF) {
        find(nullptr);
    }

    return saved_entry_count;
}

constexpr uint32_t direct_cutoff = 12;
constexpr uint32_t max_indirection = 3;

uint32_t Node::get_data_block(uint32_t block) {
    uint32_t pointers_per_block = fs->get_block_size() / 4;

    if (block < direct_cutoff) {
        return block_pointers[block];
    }
    block -= direct_cutoff;

    uint32_t counter = pointers_per_block;
    uint32_t index = 0;
    while (index < max_indirection) {
        if (block < counter) {
            return recurse_radix_tree(block, block_pointers[index + direct_cutoff], index);
        }
        block -= counter;
        counter *= pointers_per_block;
        index++;
    }

    return 0;
}

uint32_t Node::recurse_radix_tree(uint32_t block, uint32_t start, uint32_t depth) {
    uint32_t pointers_per_block = fs->get_block_size() / 4;

    if (start == 0) {
        return 0;
    }

    uint32_t counter = 1;
    for (uint32_t i = 0; i < depth; i++) {
        counter *= pointers_per_block;
    }

    uint32_t result;
    fs->disk.read(fs->get_block_offset(start) + (block / counter) * 4, result);

    if (depth == 0) {
        return result;
    } else {
        return recurse_radix_tree(block % counter, result, depth - 1);
    }

    return 0;
}

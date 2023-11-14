#ifndef _ext2_h_
#define _ext2_h_

#include "atomic.h"
#include "cache.h"
#include "ide.h"

class Ext2;

// A wrapper around an i-node
class Node : public BlockIO {  // we implement BlockIO because we
                               // represent data
   private:
    Ext2* fs;
    uint16_t type;
    uint16_t link_count;
    uint32_t size;
    uint32_t block_pointers[15];
    uint32_t saved_entry_count{0xFFFF'FFFF};

   public:
    // i-number of this node
    const uint32_t number;

    Node(Ext2* fs, uint32_t number);
    Node(Node* other);

    virtual ~Node() {}

    // How many bytes does this i-node represent
    //    - for a file, the size of the file
    //    - for a directory, implementation dependent
    //    - for a symbolic link, the length of the name
    uint32_t size_in_bytes() override;

    // read the given block (panics if the block number is not valid)
    // remember that block size is defined by the file system not the device
    void read_block(uint32_t number, char* buffer) override;

    uint16_t get_type();

    // true if this node is a directory
    bool is_dir();

    // true if this node is a file
    bool is_file();

    // true if this node is a symbolic link
    bool is_symlink();

    // If this node is a symbolic link, fill the buffer with
    // the name the link referes to.
    //
    // Panics if the node is not a symbolic link
    //
    // The buffer needs to be at least as big as the the value
    // returned by size_in_byte()
    void get_symbol(char* buffer);

    // Returns the number of hard links to this node
    uint32_t n_links();

    void show(const char* msg);

    uint32_t find(const char* name);

    // Returns the number of entries in a directory node
    //
    // Panics if not a directory
    uint32_t entry_count();

   private:
    uint32_t get_data_block(uint32_t block);
    uint32_t recurse_radix_tree(uint32_t block, uint32_t start, uint32_t depth);
};

// This class encapsulates the implementation of the Ext2 file system
class Ext2 {
    uint32_t n_block_groups;
    uint32_t n_inodes_per_block_group;
    uint32_t block_size;
    uint16_t inode_size;

    uint32_t* group_inode_start_blocks{nullptr};

   public:
    // The root directory for this file system
    Node* root{nullptr};

    // The device on which the file system resides
    CachedBlockIO disk;

   public:
    // Mount an existing file system residing on the given device
    // Panics if the file system is invalid
    Ext2(Ide* ide);

    // Returns the block size of the file system. Doesn't have
    // to match that of the underlying device
    uint32_t get_block_size();

    // Returns the actual size of an i-node. Ext2 specifies that
    // an i-node will have a minimum size of 128B but could have
    // more bytes for extended attributes
    uint32_t get_inode_size();

    // Returns the node with the given i-number
    Node* get_node(uint32_t number);

    // If the given node is a directory, return a reference to the
    // node linked to that name in the directory.
    //
    // Returns a null reference if "name" doesn't exist in the directory
    //
    // Panics if "dir" is not a directory
    Node* find(Node* dir, const char* name);

    uint32_t get_block_offset(uint32_t block_number);

    uint32_t get_inode_offset(uint32_t inode_number);

    ~Ext2();
};

#endif

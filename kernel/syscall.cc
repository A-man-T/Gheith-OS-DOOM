#include "syscalls.h"
#include "kernel.h"
#include "utils.h"
#include "process.h"
#include "file_descriptor.h"


// Seeks to a position in a file, based on whence. 
// Returns the resulting offset location as measured in bytes from the beginning of the file,
// or -1 if the offset is negative and/or on an error (NOT offset - 1)
int lseek(const int fd, const int offset, const int whence)
{
    Process* process = active_processes.mine();
    auto descriptor_pointer = process->file_descriptors.get(fd);
    if (descriptor_pointer == nullptr) {
        return -1;
    }
    auto& descriptor = *descriptor_pointer;
    if (descriptor == nullptr)
    {
        return -1;
    }

    if (!(descriptor->supports_offset()))
    {
        return -1;
    }

    int new_offset = -1;
    switch (whence)
    {
        case SEEK_SET:
        {
            new_offset = offset;
            break;
        }
        case SEEK_CUR:
        {
            new_offset = descriptor->get_offset() + offset;
            break;
        }
        case SEEK_END:
        {
            new_offset = descriptor->get_length() + offset;
            break;
        }
        default:
        {
            return -1;
        }
    }

    // Check if offset is negative
    if (new_offset < 0) {
        return -1;
    }
    
    descriptor->set_offset(new_offset);
    return new_offset;
}

bool isatty(const int fd_num){
    Process* process = active_processes.mine();
    auto descriptor_pointer = process->file_descriptors.get(fd_num);
    if (descriptor_pointer == nullptr) {
        return 0;
    }
    auto& fd = *descriptor_pointer;
    if (fd == nullptr)
    {
        return 0;
    }
    return fd->is_tty();
}
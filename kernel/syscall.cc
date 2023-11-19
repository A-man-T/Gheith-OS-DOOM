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
    Process *process = active_processes.mine();
    Shared<FileDescriptor> descriptor = process->file_descriptors.get(fd);
    if (descriptor == nullptr)
    {
        //Debug::printf("passed in invalid file descriptor into lseek\n");
        return -1;
    }

    if (!(descriptor->supports_offset()))
    {
        //Debug::printf("file descriptor does not support offset\n");
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
            //Debug::printf("passed in invalid whence flag into lseek\n");
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
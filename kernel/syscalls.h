#include "debug.h"
#include "file_descriptor.h"

// Define constants
// TODO: remove hardcodes when libc is finished
const int HARDCODED_OCLOEXEC = 524288;
const int HARDCODED_SEEK_SET = 0;
const int HARDCODED_SEEK_CUR = 1;
const int HARDCODED_SEEK_END = 2;


// Seeks to a position in a file, based on whence.
// Returns the resulting offset location as measured in bytes from the beginning of the file, or -1 on an error (NOT offset - 1)
int lseek(const int fd, const int offset, const int whence);
#include "debug.h"
#include "file_descriptor.h"

// Define constants
inline constexpr int SEEK_SET = 0;
inline constexpr int SEEK_CUR = 1;
inline constexpr int SEEK_END = 2;


// Seeks to a position in a file, based on whence.
// Returns the resulting offset location as measured in bytes from the beginning of the file,
// or -1 if the offset is negative and/or on an error (NOT offset - 1)
int lseek(const int fd, const int offset, const int whence);

// Determines whether the file descriptor uses the terminal. Stdin, stdout, and stderr are all terminals.
int isatty(const int fd_num);
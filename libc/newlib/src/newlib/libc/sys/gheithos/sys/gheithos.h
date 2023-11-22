#pragma once

// This file contains headers for syscalls that are special to our OS

void shutdown(void);
void yield(void);
int join(void);

// semaphores
int sem(unsigned int initial_value);
int up(unsigned int num);
int down(unsigned int num);
int sem_close(unsigned int num);

// signals
void simple_signal(void (*handler)(int, unsigned int));

// mmap
void* simple_mmap(void* address, unsigned int size, int fd, unsigned int offset);
int simple_munmap(void* address);

// files
int len(int fd);
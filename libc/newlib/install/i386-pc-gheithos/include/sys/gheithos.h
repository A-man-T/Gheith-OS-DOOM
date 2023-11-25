#pragma once

#ifdef __cplusplus
extern "C" {
#endif

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
int simple_signal(void (*handler)(int, unsigned int));
int kill(int pid, int sig);

// mmap
void* simple_mmap(void* address, unsigned int size, int fd, unsigned int offset);
int simple_munmap(void* address);

// files
int len(int fd);

// device IO
int read_mouse_event(void);
int read_key_event(void);
int is_pressed(unsigned int key);
int is_held(unsigned int key);

#ifdef __cplusplus
}
#endif
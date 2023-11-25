#ifndef _SYS_H_
#define _SYS_H_


#include "stdint.h"
/****************/
/* System calls */
/****************/

typedef int ssize_t;
typedef unsigned int size_t;

/* exit */
extern void exit(int rc);

/* write */
extern ssize_t write(int fd, void* buf, size_t nbyte);

/* fork */
extern int fork();

/* execve */
extern int execve(const char *pathname, char *const argv[], char *const envp[]);

/* shutdown */
extern void shutdown(void);

/* join */
extern int join(void);

/* sem */
extern int sem(unsigned int);

/* up */
extern int up(unsigned int);

/* down */
extern int down(unsigned int);

/* simple_signal */
extern void simple_signal(void (*pf)(int, unsigned int));

/* simple_mmap */
extern uint32_t* simple_mmap(void* addr, unsigned size, int fd, unsigned offset);

/* simple_munmap */
extern uint32_t simple_munmap(void* addr);

/* sigreturn */
extern int sigreturn(void);



extern int dup(int fd);

/* FILES */

extern void chdir(char* path);

extern int open(char* path);

extern int close(int fd);

extern int len(int fd);

extern int read(int fd, char* buffer, int count);

extern int pipe(int* write_fd, int* read_fd);

extern int seek(int fd, int offset, int whence);

extern int kill(unsigned v);

#endif

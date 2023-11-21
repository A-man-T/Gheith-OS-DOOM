#ifndef _SYS_H_
#define _SYS_H_

/****************/
/* System calls */
/****************/

typedef int ssize_t;
typedef unsigned int size_t;

/* exit */
extern void exit(int rc);

/* write */
extern ssize_t write(int fd, void* buf, size_t nbyte);

/* execl */
extern int execl(const char *pathname, const char *arg, ...
                       /* (char  *) NULL */);

#endif

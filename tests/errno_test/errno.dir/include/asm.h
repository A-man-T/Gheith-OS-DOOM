/**
 * Attempts to trigger an EFAULT error by putting %esp above user space
 * @param syssum the syscall number to get the efault on
 * @returns the return value of the syscall (not intercepted by libc)
 */
extern int _highEsp(int sysnum);
/**
 * Attempts to trigger an EFAULT error by putting %esp below user space
 * @param syssum the syscall number to get the efault on
 * @returns the return value of the syscall (not intercepted by libc)
 */
extern int _lowEsp(int sysnum);
/**
 * Attempts to trigger an EFAULT error by putting %esp into unmapped user space
 * @param syssum the syscall number to get the efault on
 * @returns the return value of the syscall (not intercepted by libc)
 */
extern int _unmappedEsp(int sysnum);

/**
 * Calls sigreturn directly since libc obscures it from the user
 * @return the return value of the syscall (not intercepted by libc)
 */
extern int _sigreturn();

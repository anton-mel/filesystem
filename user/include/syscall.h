#ifndef _USER_SYSCALL_H_
#define _USER_SYSCALL_H_

#include <lib/syscall.h>

#include <debug.h>
#include <gcc.h>
#include <proc.h>
#include <types.h>
#include <x86.h>
#include <file.h>

static gcc_inline void sys_puts(const char *s, size_t len)
{
    asm volatile ("int %0"
                  :: "i" (T_SYSCALL),
                     "a" (SYS_puts),
                     "b" (s),
                     "c" (len)
                  : "cc", "memory");
}

static gcc_inline pid_t sys_spawn(unsigned int elf_id, unsigned int quota)
{
    int errno;
    pid_t pid;

    asm volatile ("int %2"
                  : "=a" (errno), "=b" (pid)
                  : "i" (T_SYSCALL),
                    "a" (SYS_spawn),
                    "b" (elf_id),
                    "c" (quota)
                  : "cc", "memory");

    return errno ? -1 : pid;
}

static gcc_inline void sys_yield(void)
{
    asm volatile ("int %0"
                  :: "i" (T_SYSCALL),
                     "a" (SYS_yield)
                  : "cc", "memory");
}

static gcc_inline int sys_read(int fd, char *buf, size_t n)
{
    int errno;
    size_t ret;

    asm volatile ("int %2"
                  : "=a" (errno), "=b" (ret)
                  : "i" (T_SYSCALL),
                    "a" (SYS_read),
                    "b" (fd),
                    "c" (buf),
                    "d" (n)
                  : "cc", "memory");

    return errno ? -1 : ret;
}

static gcc_inline int sys_write(int fd, char *p, int n)
{
    int errno;
    size_t ret;

    asm volatile ("int %2"
                  : "=a" (errno), "=b" (ret)
                  : "i" (T_SYSCALL),
                    "a" (SYS_write),
                    "b" (fd),
                    "c" (p),
                    "d" (n)
                  : "cc", "memory");

    return errno ? -1 : ret;
}

static gcc_inline int sys_close(int fd)
{
    int errno;
    int ret;

    asm volatile ("int %2"
                  : "=a" (errno), "=b" (ret)
                  : "i" (T_SYSCALL),
                    "a" (SYS_close),
                    "b" (fd)
                  : "cc", "memory");

    return errno ? -1 : 0;
}

static gcc_inline int sys_fstat(int fd, struct file_stat *st)
{
    int errno;
    int ret;

    asm volatile ("int %2"
                  : "=a" (errno), "=b" (ret)
                  : "i" (T_SYSCALL),
                    "a" (SYS_stat),
                    "b" (fd),
                    "c" (st)
                  : "cc", "memory");

    return errno ? -1 : 0;
}

static gcc_inline int sys_link(char *old, char *new)
{
    int errno, ret;
    int old_len = strlen(old);
    int new_len = strlen(new);

    // for the arg4 need to use Source reg
    // since all a b c d are used now
    asm volatile ("int %2"
                  : "=a" (errno), "=b" (ret)
                  : "i" (T_SYSCALL),
                    "a" (SYS_link),    // syscall number
                    "b" (old),         // arg1: old path
                    "c" (new),         // arg2: new path
                    "d" (old_len),     // arg3: old path length
                    "S" (new_len)      // arg4: new path length
                  : "cc", "memory");

    return errno ? -1 : 0;
}

static gcc_inline int sys_unlink(char *path)
{
    int errno, ret;
    int path_len = strlen(path);

    asm volatile ("int %2"
                  : "=a" (errno), "=b" (ret)
                  : "i" (T_SYSCALL),
                    "a" (SYS_unlink),  // syscall number
                    "b" (path),        // arg1: path
                    "c" (path_len)     // arg2: path length
                  : "cc", "memory");

    return errno ? -1 : 0;
}

static gcc_inline int sys_open(char *path, int omode)
{
    int errno, fd;
    int path_len = strlen(path);

    asm volatile ("int %2"
                  : "=a" (errno), "=b" (fd)
                  : "i" (T_SYSCALL),
                    "a" (SYS_open),   // syscall number
                    "b" (path),       // arg1: path pointer
                    "c" (omode),      // arg2: open mode
                    "d" (path_len)    // arg3: path length
                  : "cc", "memory");

    return errno ? -1 : fd;
}

static gcc_inline int sys_mkdir(char *path)
{
    int errno, ret;
    int path_len = strlen(path);

    asm volatile ("int %2"
                  : "=a" (errno), "=b" (ret)
                  : "i" (T_SYSCALL),
                    "a" (SYS_mkdir),  // syscall number
                    "b" (path),       // arg1: path pointer
                    "c" (path_len)    // arg2: path length
                  : "cc", "memory");

    return errno ? -1 : 0;
}

static gcc_inline int sys_chdir(char *path)
{
    int errno, ret;
    int path_len = strlen(path);

    asm volatile ("int %2"
                  : "=a" (errno), "=b" (ret)
                  : "i" (T_SYSCALL),
                    "a" (SYS_chdir),  // syscall number
                    "b" (path),       // arg1: path pointer
                    "c" (path_len)    // arg2: path length
                  : "cc", "memory");

    return errno ? -1 : 0;
}

static gcc_inline int sys_readline(char *path)
{
    int errno, ret;

    asm volatile ("int %2"
                  : "=a" (errno), "=b" (ret)
                  : "i" (T_SYSCALL),
                    "a" (SYS_readline), // syscall number
                    "b" (path)          // arg1: path pointer
                  : "cc", "memory");

    return errno ? -1 : 0;
}

#endif  /* !_USER_SYSCALL_H_ */

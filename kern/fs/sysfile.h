// File-system system calls.
// Mostly argument checking, since we don't trust
// user code, and calls into inode.c, dir.c, path.c and file.c.

#ifndef _KERN_FS_SYSFILE_H_
#define _KERN_FS_SYSFILE_H_

#ifdef _KERN_

void sys_read(tf_t *tf);
void sys_write(tf_t *tf);
void sys_close(tf_t *tf);
void sys_fstat(tf_t *tf);
void sys_link(tf_t *tf);
void sys_unlink(tf_t *tf);
void sys_open(tf_t *tf);
void sys_mkdir(tf_t *tf);
void sys_chdir(tf_t *tf);

// Helper function for sys_read
#include "file.h"
static int perform_file_read(struct file *file_ptr, char *kernel_buf, unsigned int n);
static bool validate_read_args(int fd, unsigned int buffer, unsigned int n);
static int copy_to_user(char *kernel_buf, unsigned int user_buf, int len);
static void set_syscall_success(tf_t *tf, int bytes_read);
static void set_syscall_failure(tf_t *tf);
static bool validate_write_args(int fd, unsigned int buffer, unsigned int n);
static int copy_from_user(unsigned int user_buf, unsigned int len);
static int perform_file_write(struct file *file_ptr, unsigned int n);
static bool validate_close_fd(int fd);
static bool validate_fstat_args(int fd, struct file_stat *user_stat);

#define SIZE_BUFF 10000

#endif  /* _KERN_ */

#endif  /* !_KERN_FS_SYSFILE_H_ */

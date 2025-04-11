// File-system system calls.

#include <kern/lib/types.h>
#include <kern/lib/debug.h>
#include <kern/lib/pmap.h>
#include <kern/lib/string.h>
#include <kern/lib/trap.h>
#include <kern/fs/sysfile.h>
#include <kern/lib/syscall.h>
#include <kern/lib/spinlock.h>
#include <kern/thread/PCurID/export.h>
#include <kern/thread/PTCBIntro/export.h>
#include <kern/trap/TSyscallArg/export.h>

#include "dir.h"
#include "path.h"
#include "fcntl.h"
#include "log.h"

char glob_buffer[SIZE_BUFF];
static spinlock_t Block;

/**
 * This function is not a system call handler, but an auxiliary function
 * used by sys_open.
 * Allocate a file descriptor for the given file.
 * You should scan the list of open files for the current thread
 * and find the first file descriptor that is available.
 * Return the found descriptor or -1 if none of them is free.
 */
static int fdalloc(struct file *f)
{
    // TODO
    unsigned int tid = get_curid();
    struct file **file_table = tcb_get_openfiles(tid);

    for (int fd_index = 0; fd_index < NOFILE; fd_index++)
    {
        if (file_table[fd_index] == NULL)
        {
            tcb_set_openfiles(tid, fd_index, f);
            return fd_index;
        }
    }

    return -1;  // No available file descriptor
}

/**
 * From the file indexed by the given file descriptor, read n bytes and save them
 * into the buffer in the user. As explained in the assignment specification,
 * you should first write to a kernel buffer then copy the data into user buffer
 * with pt_copyout.
 * Return Value: Upon successful completion, read() shall return a non-negative
 * integer indicating the number of bytes actually read. Otherwise, the
 * functions shall return -1 and set errno E_BADF to indicate the error.
 */
void sys_read(tf_t *tf)
{
    spinlock_acquire(&Block);
    
    // TODO
    // get args from the syscall
    int fd = syscall_get_arg2(tf);
    unsigned int user_buffer = syscall_get_arg3(tf);
    unsigned int n = syscall_get_arg4(tf);

    // validate input arguments
    if (!validate_read_args(fd, user_buffer, n)) {
        set_syscall_failure(tf);
        spinlock_release(&Block);
        return;
    }

    // resolve file pointer from current thread
    struct file *file_ptr = tcb_get_openfiles(get_curid())[fd];
    if (file_ptr == NULL) {
        set_syscall_failure(tf);
        spinlock_release(&Block);
        return;
    }

    // zero kernel-side buffer before read
    memzero(glob_buffer, SIZE_BUFF);

    // read into kernel buffer
    int bytes_read = perform_file_read(file_ptr, glob_buffer, n);
    if (bytes_read < 0) {
        set_syscall_failure(tf);
        spinlock_release(&Block);
        return;
    }

    // copy data to user space
    int copied = copy_to_user(glob_buffer, user_buffer, bytes_read);
    set_syscall_success(tf, copied);
    spinlock_release(&Block);
}

/* Helper functions */

static bool validate_read_args(int fd, unsigned int buffer, unsigned int n) {
    if ( fd < 0 || fd >= NOFILE || n > SIZE_BUFF ) {
        return 0;
    }
    return 1;
}

static int perform_file_read(struct file *file_ptr, char *kernel_buf, unsigned int n) {
    return file_read(file_ptr, kernel_buf, n);
}

static int copy_to_user(char *kernel_buf, unsigned int user_buf, int len) {
    return pt_copyout(kernel_buf, get_curid(), user_buf, len);
}

// helper function to manage syscall rax
static void set_syscall_failure(tf_t *tf) {
    syscall_set_errno(tf, E_BADF);
    syscall_set_retval1(tf, -1);
}

// helper function to manage syscall rax
static void set_syscall_success(tf_t *tf, int bytes) {
    syscall_set_errno(tf, E_SUCC);
    syscall_set_retval1(tf, bytes);
}

/**
 * Write n bytes of data in the user's buffer into the file indexed by the file descriptor.
 * You should first copy the data info an in-kernel buffer with pt_copyin and then
 * pass this buffer to appropriate file manipulation function.
 * Upon successful completion, write() shall return the number of bytes actually
 * written to the file associated with f. This number shall never be greater
 * than nbyte. Otherwise, -1 shall be returned and errno E_BADF set to indicate the
 * error.
 */
void sys_write(tf_t *tf)
{
    spinlock_acquire(&Block);
    // TODO
    // get syscall argument first
    int fd = syscall_get_arg2(tf);
    unsigned int user_buffer = syscall_get_arg3(tf);
    unsigned int n = syscall_get_arg4(tf);

    // basic argument checks
    if (!validate_write_args(fd, user_buffer, n)) {
        set_syscall_failure(tf);
        spinlock_release(&Block);
        return;
    }

    // resolve the file from the current TCB
    struct file *file_ptr = tcb_get_openfiles(get_curid())[fd];
    if (file_ptr == NULL) {
        set_syscall_failure(tf);
        spinlock_release(&Block);
        return;
    }

    // zero kernel-side buffer before write
    memzero(glob_buffer, SIZE_BUFF);

    // copy back from user to kernel
    int copied = copy_from_user(user_buffer, n);
    // if (copied < 0) {
    //     set_syscall_failure(tf);
    //     spinlock_release(&Block);
    //     return;
    // }

    // write to file from kernel buffer
    int written = perform_file_write(file_ptr, copied);
    if (written < 0) {
        set_syscall_failure(tf);
        spinlock_release(&Block);
        return;
    }

    set_syscall_success(tf, written);
    spinlock_release(&Block);
}

/* Helper functions */

static bool validate_write_args(int fd, unsigned int buffer, unsigned int n) {
    if ( fd < 0 || fd >= NOFILE || n > SIZE_BUFF ) {
        return 0;
    }
    unsigned int end_of_write = buffer + n;
    // important!!! validate write to user space
    if (end_of_write > VM_USERHI || buffer < VM_USERLO) {
        return 0;
    }
    return 1;
}

static int copy_from_user(unsigned int user_buf, unsigned int len) {
    return pt_copyin(get_curid(), user_buf, glob_buffer, len);
}

static int perform_file_write(struct file *file_ptr, unsigned int n) {
    return file_write(file_ptr, glob_buffer, n);
}

/**
 * Return Value: Upon successful completion, 0 shall be returned; otherwise, -1
 * shall be returned and errno E_BADF set to indicate the error.
 */
void sys_close(tf_t *tf)
{
    // TODO
    // no locking since no buffer access
    // same idea: get sys args
    int fd = syscall_get_arg2(tf);

    // validate: if closed without failure
    if (!validate_close_fd(fd)) {
        set_syscall_failure(tf);
        return;
    }

    // access the file given fd for a current thread
    struct file *file_ptr = tcb_get_openfiles(get_curid())[fd];
    if (file_ptr == NULL) {
        set_syscall_failure(tf);
        return;
    }

    // remove file from open file table
    // From ED stem: we should not remove the file itself!!!
    tcb_set_openfiles(get_curid(), fd, NULL);

    // just properly close the file
    file_close(file_ptr);
    // success
    set_syscall_success(tf, 0);
}

/* Helper functions */

static bool validate_close_fd(int fd) {
    return fd >= 0;
}

/**
 * Return Value: Upon successful completion, 0 shall be returned. Otherwise, -1
 * shall be returned and errno E_BADF set to indicate the error.
 */
void sys_fstat(tf_t *tf)
{
    // TODO
    // no locking since no buffer access
    // fetch
    int fd = syscall_get_arg2(tf);
    struct file_stat *user_stat = (struct file_stat *)syscall_get_arg3(tf);

    // validate
    if (!validate_fstat_args(fd, user_stat)) {
        set_syscall_failure(tf);
        return;
    }

    // access
    struct file *file_ptr = tcb_get_openfiles(get_curid())[fd];
    if (file_ptr == NULL) {
        set_syscall_failure(tf);
        return;
    }

    // get stats 
    int result = file_stat(file_ptr, user_stat);
    if (result != 0) {
        set_syscall_failure(tf);
        return;
    }
    set_syscall_success(tf, 0);
}


/* Helpers */

static bool is_valid_user_buffer(uintptr_t addr, size_t len) {
    return addr >= VM_USERLO && (addr + len) <= VM_USERHI;
}

static bool validate_fstat_args(int fd, struct file_stat *user_stat) {
    return fd >= 0 && user_stat != NULL;
}

/**
 * Create the path new as a link to the same inode as old.
 */
void sys_link(tf_t * tf)
{
    char name[DIRSIZ], path_new[128], path_old[128];
    struct inode *dp, *ip;

    uintptr_t old_ptr = syscall_get_arg2(tf);
    uintptr_t new_ptr = syscall_get_arg3(tf);
    size_t old_size = syscall_get_arg4(tf);
    size_t new_size = syscall_get_arg5(tf);

    if (!check_user_buffer(tf, old_ptr, old_size, 128) || 
        !check_user_buffer(tf, new_ptr, new_size, 128)) {
        return;
    }

    // Copy user-provided paths into kernel memory
    pt_copyin(get_curid(), old_ptr, path_old, old_size);
    pt_copyin(get_curid(), new_ptr, path_new, new_size);

    if ((ip = namei(path_old)) == 0) {
        syscall_set_errno(tf, E_NEXIST);
        return;
    }

    begin_trans();

    inode_lock(ip);
    if (ip->type == T_DIR) {
        inode_unlockput(ip);
        commit_trans();
        syscall_set_errno(tf, E_DISK_OP);
        return;
    }

    ip->nlink++;
    inode_update(ip);
    inode_unlock(ip);

    if ((dp = nameiparent(path_new, name)) == 0)
        goto bad;
    inode_lock(dp);
    if (dp->dev != ip->dev || dir_link(dp, name, ip->inum) < 0) {
        inode_unlockput(dp);
        goto bad;
    }
    inode_unlockput(dp);
    inode_put(ip);

    commit_trans();

    syscall_set_errno(tf, E_SUCC);
    return;

bad:
    inode_lock(ip);
    ip->nlink--;
    inode_update(ip);
    inode_unlockput(ip);
    commit_trans();
    syscall_set_errno(tf, E_DISK_OP);
    return;
}

/* Helpers */

static bool check_user_buffer(tf_t *tf, uintptr_t buf, size_t len, size_t maxlen) {
    if (!is_valid_user_buffer(buf, len)) {
        syscall_set_errno(tf, E_INVAL_ADDR);
        syscall_set_retval1(tf, -1);
        return FALSE;
    }

    if (maxlen > 0 && len >= maxlen) {
        syscall_set_errno(tf, E_INVAL_ADDR);
        syscall_set_retval1(tf, -1);
        return FALSE;
    }

    return TRUE;
}

/**
 * Is the directory dp empty except for "." and ".." ?
 */
static int isdirempty(struct inode *dp)
{
    int off;
    struct dirent de;

    for (off = 2 * sizeof(de); off < dp->size; off += sizeof(de)) {
        if (inode_read(dp, (char *) &de, off, sizeof(de)) != sizeof(de))
            KERN_PANIC("isdirempty: readi");
        if (de.inum != 0)
            return 0;
    }
    return 1;
}

void sys_unlink(tf_t *tf)
{
    struct inode *ip, *dp;
    struct dirent de;
    char name[DIRSIZ], path[128];
    uint32_t off;

    uintptr_t buffer = syscall_get_arg2(tf);
    size_t length = syscall_get_arg3(tf);

    if (!check_user_buffer(tf, buffer, length, 128)) {
        return;
    }

    pt_copyin(get_curid(), buffer, path, 128);

    if ((dp = nameiparent(path, name)) == 0) {
        syscall_set_errno(tf, E_DISK_OP);
        return;
    }

    begin_trans();

    inode_lock(dp);

    // Cannot unlink "." or "..".
    if (dir_namecmp(name, ".") == 0 || dir_namecmp(name, "..") == 0)
        goto bad;

    if ((ip = dir_lookup(dp, name, &off)) == 0)
        goto bad;
    inode_lock(ip);

    if (ip->nlink < 1)
        KERN_PANIC("unlink: nlink < 1");
    if (ip->type == T_DIR && !isdirempty(ip)) {
        inode_unlockput(ip);
        goto bad;
    }

    memset(&de, 0, sizeof(de));
    if (inode_write(dp, (char *) &de, off, sizeof(de)) != sizeof(de))
        KERN_PANIC("unlink: writei");
    if (ip->type == T_DIR) {
        dp->nlink--;
        inode_update(dp);
    }
    inode_unlockput(dp);

    ip->nlink--;
    inode_update(ip);
    inode_unlockput(ip);

    commit_trans();

    syscall_set_errno(tf, E_SUCC);
    return;

bad:
    inode_unlockput(dp);
    commit_trans();
    syscall_set_errno(tf, E_DISK_OP);
    return;
}

static struct inode *create(char *path, short type, short major, short minor)
{
    uint32_t off;
    struct inode *ip, *dp;
    char name[DIRSIZ];

    if ((dp = nameiparent(path, name)) == 0)
        return 0;
    inode_lock(dp);

    if ((ip = dir_lookup(dp, name, &off)) != 0) {
        inode_unlockput(dp);
        inode_lock(ip);
        if (type == T_FILE && ip->type == T_FILE)
            return ip;
        inode_unlockput(ip);
        return 0;
    }

    if ((ip = inode_alloc(dp->dev, type)) == 0)
        KERN_PANIC("create: ialloc");

    inode_lock(ip);
    ip->major = major;
    ip->minor = minor;
    ip->nlink = 1;
    inode_update(ip);

    if (type == T_DIR) {  // Create . and .. entries.
        dp->nlink++;      // for ".."
        inode_update(dp);
        // No ip->nlink++ for ".": avoid cyclic ref count.
        if (dir_link(ip, ".", ip->inum) < 0
            || dir_link(ip, "..", dp->inum) < 0)
            KERN_PANIC("create dots");
    }

    if (dir_link(dp, name, ip->inum) < 0)
        KERN_PANIC("create: dir_link");

    inode_unlockput(dp);
    return ip;
}

void sys_open(tf_t *tf)
{
    char path[128];
    int fd, omode;
    struct file *f;
    struct inode *ip;

    uintptr_t buffer = syscall_get_arg2(tf);
    omode = syscall_get_arg3(tf);
    size_t length = syscall_get_arg4(tf);

    if (!check_user_buffer(tf, buffer, length, 128)) {
        return;
    } else {
        pt_copyin(get_curid(), buffer, path, 128);
    }

    if (omode & O_CREATE) {
        begin_trans();
        ip = create(path, T_FILE, 0, 0);
        commit_trans();
        if (ip == 0) {
            syscall_set_retval1(tf, -1);
            syscall_set_errno(tf, E_CREATE);
            return;
        }
    } else {
        if ((ip = namei(path)) == 0) {
            syscall_set_retval1(tf, -1);
            syscall_set_errno(tf, E_NEXIST);
            return;
        }
        inode_lock(ip);
        if (ip->type == T_DIR && omode != O_RDONLY) {
            inode_unlockput(ip);
            syscall_set_retval1(tf, -1);
            syscall_set_errno(tf, E_DISK_OP);
            return;
        }
    }

    if ((f = file_alloc()) == 0 || (fd = fdalloc(f)) < 0) {
        if (f)
            file_close(f);
        inode_unlockput(ip);
        syscall_set_retval1(tf, -1);
        syscall_set_errno(tf, E_DISK_OP);
        return;
    }
    inode_unlock(ip);

    f->type = FD_INODE;
    f->ip = ip;
    f->off = 0;
    f->readable = !(omode & O_WRONLY);
    f->writable = (omode & O_WRONLY) || (omode & O_RDWR);
    syscall_set_retval1(tf, fd);
    syscall_set_errno(tf, E_SUCC);
}

void sys_mkdir(tf_t *tf)
{
    char path[128];
    struct inode *ip;

    uintptr_t buffer = syscall_get_arg2(tf);
    size_t length = syscall_get_arg3(tf);

    if (!check_user_buffer(tf, buffer, length, 128)) {
        return;
    } else {
        pt_copyin(get_curid(), buffer, path, 128);
    }

    begin_trans();
    if ((ip = (struct inode *) create(path, T_DIR, 0, 0)) == 0) {
        commit_trans();
        syscall_set_errno(tf, E_DISK_OP);
        return;
    }
    inode_unlockput(ip);
    commit_trans();
    syscall_set_errno(tf, E_SUCC);
}

void sys_chdir(tf_t *tf)
{
    char path[128];
    struct inode *ip;
    int pid = get_curid();

    uintptr_t buffer = syscall_get_arg2(tf);
    size_t length = syscall_get_arg3(tf);

    if (!check_user_buffer(tf, buffer, length, 128)) {
        return;
    } else {
        pt_copyin(get_curid(), buffer, path, 128);
    }

    if ((ip = namei(path)) == 0) {
        syscall_set_errno(tf, E_DISK_OP);
        return;
    }
    inode_lock(ip);
    if (ip->type != T_DIR) {
        inode_unlockput(ip);
        syscall_set_errno(tf, E_DISK_OP);
        return;
    }
    inode_unlock(ip);
    inode_put(tcb_get_cwd(pid));
    tcb_set_cwd(pid, ip);
    syscall_set_errno(tf, E_SUCC);
}

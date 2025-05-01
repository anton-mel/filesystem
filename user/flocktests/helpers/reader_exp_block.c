// ELF ID 8

#include <proc.h>
#include <stdio.h>
#include <syscall.h>
#include <file.h>

int main (void) {
    int fd = open(FLOCK_TEST_PATH, O_RDONLY);

    if (flock(fd, LOCK_SH | LOCK_NB) == 0)  sys_exit(1);

    flock(fd, LOCK_UN);
    close(fd);
    sys_exit(0);
}
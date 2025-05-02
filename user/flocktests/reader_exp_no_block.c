// ELF ID 7

#include <proc.h>
#include <stdio.h>
#include <syscall.h>
#include <file.h>

#include "flocktests_common.h"

#define exit(...) return -1

int main (void) {
    int fd = open(FLOCK_TEST_PATH, O_RDONLY);

    if (flock(fd, LOCK_SH | LOCK_NB) < 0)  exit();

    flock(fd, LOCK_UN);
    close(fd);
    exit();
}
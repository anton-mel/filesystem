// ELF ID 10

#include <proc.h>
#include <stdio.h>
#include <syscall.h>
#include <file.h>

#include "flocktests_common.h"

int main (void) {
    int fd = open(FLOCK_TEST_PATH, O_RDONLY);

    if (flock(fd, LOCK_EX | LOCK_NB) == 0)  return 1;

    flock(fd, LOCK_UN);
    close(fd);
}
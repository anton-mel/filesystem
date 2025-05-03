// ELF ID 9
// user/flocktests/helpers/writer_exp_no_block.c

#include <proc.h>
#include <stdio.h>
#include <syscall.h>
#include <file.h>
#include "flocktests_common.h"

#define exit(...) return -1

int main(void) {
    int fd, ret;

    fd = open(FLOCK_TEST_PATH, O_CREATE | O_RDWR);
    if (fd < 0) {
        printf("writer_exp_no_block: open failed\n");
        exit();
    }

    // should succeed immediately
    ret = flock(fd, LOCK_EX | LOCK_NB);
    if (ret != 0) {
        printf("writer_exp_no_block: failed to get exclusive lock\n");
        exit();
    }

    // signal driver that we got it
    produce(1);

    flock(fd, LOCK_UN);
    close(fd);
    return 0;
}

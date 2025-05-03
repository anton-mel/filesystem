// ELF ID 7

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
        printf("reader_exp_no_block: open failed\n");
        exit();
    }

    // should succeed immediately
    ret = flock(fd, LOCK_SH | LOCK_NB);
    if (ret != 0) {
        printf("reader_exp_no_block: failed to get shared lock\n");
        exit();
    }

    // signal driver that we got the lock
    produce(1);

    /* do some writes under lock */
    for (int i = 0; i < 50; i++) {
        if (write(fd, "oliver", 6) != 6)
            exit();
    }

    /* wait until the tester has tried its non‐blocking lock */
    consume();

    /* release the lock */
    if (flock(fd, LOCK_UN) != 0)
        exit();

    close(fd);
    return 0;
}


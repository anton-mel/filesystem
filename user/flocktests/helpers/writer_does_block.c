// ELF ID 11

#include <proc.h>
#include <stdio.h>
#include <syscall.h>
#include <file.h>
#include "flocktests_common.h"

#define exit(...) return -1

int main(void) {
    int fd;

    /* open (or create) the file for write-only */
    if ((fd = open(FLOCK_TEST_PATH, O_CREATE | O_WRONLY)) < 0)
        exit();

    /* acquire exclusive lock */
    if (flock(fd, LOCK_EX) != 0)
        exit();

    /* tell the non‐blocking tester we’ve got the lock */
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

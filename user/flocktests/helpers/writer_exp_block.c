// ELF ID 10
// user/flocktests/helpers/writer_exp_block.c

#include <proc.h>
#include <stdio.h>
#include <syscall.h>
#include <file.h>
#include "flocktests_common.h"

#define exit(...) return -1

int main(void) {
    int fd, ret;

    // wait for driver to say “go”
    consume();

    fd = open(FLOCK_TEST_PATH, O_CREATE | O_RDWR);
    if (fd < 0) {
        printf("writer_exp_block: open failed\n");
        exit();
    }

    // should fail immediately (would block)
    ret = flock(fd, LOCK_EX | LOCK_NB);
    if (ret == 0) {
        printf("writer_exp_block: unexpectedly got exclusive lock\n");
        exit();
    }

    // tell driver we tried
    produce(1);

    close(fd);
    return 0;
}

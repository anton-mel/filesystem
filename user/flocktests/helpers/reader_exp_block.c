// ELF ID 8
// user/flocktests/helpers/reader_exp_block.c

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

    fd = open(FLOCK_TEST_PATH, O_CREATE | O_RDONLY);
    if (fd < 0) {
        printf("reader_exp_block: open failed\n");
        exit();
    }

    // should fail immediately (would block)
    ret = flock(fd, LOCK_SH | LOCK_NB);
    if (ret == 0) {
        printf("reader_exp_block: unexpectedly got shared lock\n");
        exit();
    }

    // tell driver we tried
    produce(1);

    close(fd);
    return 0;
}

// ELF ID 7

#include <proc.h>
#include <stdio.h>
#include <syscall.h>
#include <file.h>

#include "flocktests_common.h"

#define exit(...) return -1
#define SYNC_BEFORE_CHILD() produce(1)
#define SYNC_AFTER_CHILD()  consume()

int main(void) {
    int fd;

    SYNC_AFTER_CHILD(); // Wait for main test to say: "you may try locking"

    fd = open("flockfile", O_RDWR);
    if (fd < 0) {
        printf("reader_exp_no_block ERROR: open failed\n");
        exit();
    }

    int ret = flock(fd, LOCK_SH | LOCK_NB);
    if (ret == 0) {
        printf("reader_exp_no_block: got shared lock (non-blocking)\n");
    } else {
        printf("reader_exp_no_block: failed to get shared lock (non-blocking)\n");
    }

    produce(1); // Signal to test: done trying

    if (ret == 0) {
        flock(fd, LOCK_UN); // release if we held it
    }

    close(fd);
    return 0;
}

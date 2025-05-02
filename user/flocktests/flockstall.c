
/*
* flockstall.c — Sync-Staller for flock() Testing
*
* This helper process coordinates with another test process to demonstrate
* flock() non-blocking behavior (LOCK_NB). It acquires a lock, performs simulated
* activity, and releases the lock only after receiving an explicit signal.
*
* Note:
* The stall duration is driven by synchronization, not time.
* The staller blocks at consume() until the paired test process calls produce().
*/

#include <proc.h>
#include <stdio.h>
#include <syscall.h>
#include <x86.h>
#include <file.h>
#include <gcc.h>

#include "flocktests_common.h"

#define exit(...) return -1

char buf[8192];

int main(int argc, char *argv[]) {
    int fd;

    printf("[D] flockstall: initiating stall sequence\n");

    fd = open(FLOCK_TEST_PATH, O_WRONLY);
    if (fd < 0) {
        printf("flockstall ERROR: could not open file.\n");
        exit();
    }

    if (flock(fd, LOCK_EX) < 0) {
        printf("flockstall ERROR: could not obtain exclusive lock.\n");
        close(fd);
        exit();
    }

    /* Notify: lock acquired */
    produce(1);

    /* Simulate workload */
    for (int i = 0; i < 80; ++i) {
        if (write(fd, "STALLLOOP--", 20) != 20 || write(fd, "..PAUSE..", 20) != 20) {
            printf("flockstall write error at iteration %d\n", i);
            close(fd);
            exit();
        }
    }

    /* Wait for coordination signal */
    consume();

    if (flock(fd, LOCK_UN) < 0) {
        printf("flockstall ERROR: could not release lock.\n");
        close(fd);
        exit();
    }

    close(fd);

    printf("[D] flockstall: completed successfully\n");
    return 0;
}

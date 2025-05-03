#include <proc.h>
#include <stdio.h>
#include <syscall.h>
#include <file.h>
#include "flocktests_common.h"

#define FAIL(msg) do { printf("FAIL: %s\n", msg); return 1; } while (0)
#define PASS()    do { printf("PASS\n"); return 0; } while (0)

#define exit(...) return -1

/* sync macros from common: */
#define SYNC_BEFORE_CHILD()  produce(1)
#define SYNC_AFTER_CHILD()   consume()

/*
 * test_single_writer: verify that a single process can successfully
 * acquire and then release an exclusive lock on a file when there is
 * no contention.
 *
 * This ensures basic correctness of LOCK_EX and LOCK_UN in the
 * uncontended case.
 */
int test_single_writer(void) {
    int fd = open(FLOCK_TEST_PATH, O_CREATE|O_RDWR);
    if (fd < 0) FAIL("open");
    if (flock(fd, LOCK_EX) != 0) FAIL("flock ex");
    if (flock(fd, LOCK_UN) != 0) FAIL("flock un");
    close(fd);
    PASS();
}

/*
 * test_multiple_readers: verify that multiple processes can concurrently
 * acquire shared locks on the same file without blocking each other.
 *
 * This ensures that LOCK_SH allows simultaneous read access by more than
 * one holder when no exclusive lock is held.
 */
int test_multiple_readers(void) {
    pid_t child;
    int fd = open(FLOCK_TEST_PATH, O_CREATE | O_RDWR);
    if (fd < 0) FAIL("open");

    /* spawn first reader (ELF ID 7 = reader_exp_no_block) */
    if ((child = spawn(R_EXP_NB, 500)) < 0)
        FAIL("spawn reader_exp_no_block");
    consume();  /* wait for first reader’s produce(1) */

    /* spawn second reader (same ELF ID 7) */
    if ((child = spawn(R_EXP_NB, 500)) < 0)
        FAIL("spawn reader_exp_no_block");
    consume();  /* wait for second reader’s produce(1) */

    /* spawn third reader (same ELF ID 7) */
    if ((child = spawn(R_EXP_NB, 500)) < 0)
    FAIL("spawn reader_exp_no_block");
    consume();  /* wait for second reader’s produce(1) */

    close(fd);
    PASS();
}

/*
 * test_ewouldblock: verify that a non-blocking exclusive flock
 * correctly fails with EWOULDBLOCK when another process already
 * holds an exclusive lock on the same file.
 *
 * This ensures that LOCK_EX | LOCK_NB does not block the caller,
 * but instead immediately returns an error if the resource is busy.
 */
int test_ewouldblock(void) {
    int fd;
    pid_t child;

    /* PART 2: spawn the writer (ELF ID 11) which takes an exclusive lock */
    if ((child = spawn(W_DOES_B, 500)) == -1) {
        printf("ERROR in test_ewouldblock: failed to spawn writer (ELF 11)\n");
        exit();
    }

    /* wait until the writer has successfully acquired its LOCK_EX */
    consume();

    /* open the same flockfile for read/write */
    if ((fd = open(FLOCK_TEST_PATH, O_RDWR)) < 0) {
        printf("ERROR in test_ewouldblock: open flockfile failed\n");
        exit();
    }

    /* this should fail immediately with EWOULDBLOCK */
    if (flock(fd, LOCK_EX | LOCK_NB) != -1) {
        printf("ERROR in test_ewouldblock: flock unexpectedly succeeded\n");
        close(fd);
        produce(1);       /* let writer finish */
        exit();
    }

    /* tell the writer it can now release and exit */
    produce(1);

    close(fd);
    PASS();
}

/* ---- writer excludes reader -------------------------------------- */
int test_writer_excludes_reader(void) {
    int fd = open(FLOCK_TEST_PATH, O_CREATE|O_RDWR);
    if (fd < 0) FAIL("open");

    if (flock(fd, LOCK_EX) != 0) FAIL("parent LOCK_EX");
    SYNC_BEFORE_CHILD();  /* allow helper to try */
    pid_t child = spawn(R_EXP_B, 500);
    if (child < 0) FAIL("spawn reader_block");
    SYNC_AFTER_CHILD();   /* wait helper finish */
    flock(fd, LOCK_UN);
    close(fd);
    PASS();
}

/* ---- writer excludes writer -------------------------------------- */
int test_writer_excludes_writer(void) {
    int fd = open(FLOCK_TEST_PATH, O_CREATE|O_RDWR);
    if (fd < 0) FAIL("open");

    if (flock(fd, LOCK_EX) != 0) FAIL("parent LOCK_EX");
    SYNC_BEFORE_CHILD();
    pid_t child = spawn(10, 500);
    if (child < 0) FAIL("spawn writer_exp_block");
    SYNC_AFTER_CHILD();
    flock(fd, LOCK_UN);
    close(fd);
    PASS();
}

/* ---- reader excludes writer -------------------------------------- */
int test_reader_excludes_writer(void) {
    int fd = open(FLOCK_TEST_PATH, O_CREATE|O_RDWR);
    if (fd < 0) FAIL("open");

    if (flock(fd, LOCK_SH) != 0) FAIL("parent LOCK_SH");
    SYNC_BEFORE_CHILD();
    pid_t child = spawn(10, 500);
    if (child < 0) FAIL("spawn writer_exp_block");
    SYNC_AFTER_CHILD();
    flock(fd, LOCK_UN);
    close(fd);
    PASS();
}

/* ---- queued writer doesn’t block later readers ------------------- */
int test_queued_writer_doesnt_block(void) {
    int fd = open(FLOCK_TEST_PATH, O_CREATE|O_RDWR);
    if (fd < 0) FAIL("open");

    if (flock(fd, LOCK_SH) != 0) FAIL("parent LOCK_SH");
    SYNC_BEFORE_CHILD();
    if (spawn(12, 500) < 0) FAIL("spawn writer_does_block");
    consume();  /* writer took EX */
    SYNC_BEFORE_CHILD();
    if (spawn(R_EXP_NB, 500) < 0) FAIL("spawn reader_no_block");
    consume();
    flock(fd, LOCK_UN);
    PASS();
}

/*
 * test_bad_fd: ensure flock returns an error when called
 * on an invalid file descriptor, verifying that the syscall
 * properly rejects bad FDs instead of succeeding.
 */
int test_bad_fd(void) {
    if (flock(999, LOCK_SH) == 0) FAIL("flock on bad fd");
    PASS();
}

/*
 * test_upgrade_flock: verify that upgrading an existing shared lock to
 * an exclusive lock succeeds when there are no other lock holders.
 * This test ensures that flock(fd, LOCK_EX) upgrades the lock in-place.
 */
int test_upgrade_flock(void) {
    int fd;

    /* open (or create) the file for read/write */
    if ((fd = open(FLOCK_TEST_PATH, O_CREATE | O_RDWR)) < 0) {
        printf("ERROR in test_upgrade_flock: cannot create/open file\n");
        exit();
    }

    /* acquire a shared lock */
    if (flock(fd, LOCK_SH) == -1) {
        printf("ERROR in test_upgrade_flock: could not acquire shared lock\n");
        close(fd);
        exit();
    }

    /* upgrade to an exclusive lock */
    if (flock(fd, LOCK_EX) == -1) {
        printf("ERROR in test_upgrade_flock: could not upgrade to exclusive lock\n");
        close(fd);
        exit();
    }

    /* write some data under the exclusive lock */
    for (int i = 0; i < 50; i++) {
        if (write(fd, "anton", 5) != 5) {
            printf("ERROR in test_upgrade_flock: write aa %d failed\n", i);
            exit();
        }
        if (write(fd, "oliver", 6) != 6) {
            printf("ERROR in test_upgrade_flock: write bb %d failed\n", i);
            exit();
        }
    }

    /* release the exclusive lock */
    if (flock(fd, LOCK_UN) == -1) {
        printf("ERROR in test_upgrade_flock: could not release lock\n");
        close(fd);
        exit();
    }

    close(fd);
    PASS();
}

/*
 * test_downgrade_flock: verify that downgrading an exclusive lock to
 * a shared lock allows data written under the exclusive lock to be
 * read correctly, and that the shared lock behaves as expected.
 * This test ensures that flock(fd, LOCK_SH) downgrades the lock in-place.
 */
int test_downgrade_flock(void) {
    int fd, n;
    char buf[100];

    unlink(FLOCK_TEST_PATH);

    /* 1) Create file and take exclusive lock */
    fd = open(FLOCK_TEST_PATH, O_CREATE | O_RDWR);
    if (fd < 0) FAIL("open failed");
    if (flock(fd, LOCK_EX) != 0) FAIL("initial exclusive lock failed");

    /* 2) Write 100 bytes of known data under the exclusive lock */
    for (int i = 0; i < 100; i++) {
        if (write(fd, "x", 1) != 1)
            FAIL("write failed");
    }

    /* 3) Unlock and close */
    if (flock(fd, LOCK_UN) != 0) FAIL("unlock failed");
    close(fd);

    /* 4) Reopen, take exclusive lock again */
    fd = open(FLOCK_TEST_PATH, O_RDWR);
    if (fd < 0) FAIL("reopen failed");
    if (flock(fd, LOCK_EX) != 0) FAIL("exclusive lock failed");

    /* 5) Downgrade to shared lock */
    if (flock(fd, LOCK_SH) != 0) FAIL("downgrade to shared failed");

    /* 6) Read back the 100 bytes */
    n = read(fd, buf, 100);
    if (n != 100) FAIL("read wrong count");
    for (int i = 0; i < 100; i++) {
        if (buf[i] != 'x') FAIL("data mismatch");
    }

    /* 7) Final unlock and close */
    if (flock(fd, LOCK_UN) != 0) FAIL("unlock failed");
    close(fd);
    PASS();
}

int main(void) {
    struct { const char *name; int (*fn)(void); } tests[] = {
        { "test_ewouldblock",           test_ewouldblock },
        { "single_writer",              test_single_writer },
        { "multiple_readers",           test_multiple_readers },
        // { "writer_excludes_reader",     test_writer_excludes_reader },
        // { "writer_excludes_writer",     test_writer_excludes_writer },
        // { "reader_excludes_writer",     test_reader_excludes_writer },
        // { "queued_writer_doesnt_block", test_queued_writer_doesnt_block },
        { "bad_fd",                     test_bad_fd },
        { "upgrade_flock",              test_upgrade_flock },
        { "downgrade_flock",            test_downgrade_flock },
    };

    int failures = 0;
    for (int i = 0; i < sizeof(tests)/sizeof(*tests); i++) {
        unlink(FLOCK_TEST_PATH);
        printf("\n=== %s ===\n", tests[i].name);
        if (tests[i].fn()) failures++;
    }

    printf("\nSUMMARY: %d failure(s)\n", failures);
    return failures;
}

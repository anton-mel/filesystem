#include <proc.h>
#include <stdio.h>
#include <syscall.h>
#include <file.h>
#include "flocktests_common.h"

#define FAIL(msg) do { printf("FAIL: %s\n", msg); return 1; } while (0)
#define PASS() do { printf("PASS\n"); return 0; } while (0)

#define SYNC_BEFORE_CHILD() produce(1)
#define SYNC_AFTER_CHILD()  consume()

/* ---- Basic correctness ----------------------------------------------- */

int test_single_writer (void) {
    printf("(single writer)...\n");

    int fd1 = open(FLOCK_TEST_PATH, O_CREATE | O_RDWR);
    if (fd1 < 0) FAIL("open failed");
    
    if (flock(fd1, FLOCK_EX) != 0) FAIL("flock exclusive lock failed");
    if (flock(fd1, FLOCK_UN) != 0) FAIL("flock unlock failed");

    PASS();
}

// int test_multiple_readers (void) {
//     printf("(multiple readers)...\n");

//     int fd1 = open(FLOCK_TEST_PATH, O_CREATE);
//     if (fd1 < 0) FAIL("open failed");

//     int children[NUM_IDS];
//     for (int i = 0; i < NUM_IDS; i++) {
//         children[i] = sys_spawn(R_EXP_NB, 8);
//         if (children[i] == NUM_IDS) FAIL("spawn failed");
//     }

//     // int status;
//     for (int i = 0; i < NUM_IDS; i++) {
//         // sys_wait(children[i], &status);
//         // if (status != 0) FAIL("reader was blocked");
//     }

//     if (flock(fd1, FLOCK_UN) != 0) FAIL("flock unlock failed");

//     PASS();
// }

/* ---- Mutual Exclusion ---------------------------------------------- */

int test_writer_excludes_reader (void) {
    printf("(writer excludes reader)...\n");

    int fd1 = open(FLOCK_TEST_PATH, O_CREATE | O_RDWR);
    if (fd1 < 0) FAIL("open failed");

    if (flock(fd1, FLOCK_EX) != 0) FAIL("flock exclusive lock failed");

    SYNC_BEFORE_CHILD(); // tell reader we're ready

    int child_pid = sys_spawn(R_EXP_B, 1000);
    if (child_pid == NUM_IDS) FAIL("spawn failed");

    SYNC_AFTER_CHILD(); // wait for child to attempt lock
    
    if (flock(fd1, FLOCK_UN) != 0) FAIL("flock unlock failed");

    PASS();
}

int test_writer_excludes_writer (void) {
    printf("(writer excludes writer)...\n");

    int fd1 = open(FLOCK_TEST_PATH, O_CREATE | O_RDWR);
    if (fd1 < 0) FAIL("open failed");

    if (flock(fd1, FLOCK_EX) != 0) FAIL("flock exclusive lock failed");

    SYNC_BEFORE_CHILD();
    int child_pid = sys_spawn(W_EXP_B, 1000);
    if (child_pid == NUM_IDS) FAIL("spawn failed");
    SYNC_AFTER_CHILD();
    
    if (flock(fd1, FLOCK_UN) != 0) FAIL("flock unlock failed");

    PASS();
}

int test_reader_excludes_writer (void) {
    printf("(reader_excludes_writer)...\n");

    int fd1 = open(FLOCK_TEST_PATH, O_CREATE);
    if (fd1 < 0) FAIL("open failed");

    if (flock(fd1, FLOCK_SH) != 0) FAIL("flock shared lock failed");

    SYNC_BEFORE_CHILD();
    int child_pid = sys_spawn(W_EXP_B, 1000);
    if (child_pid == NUM_IDS) FAIL("spawn failed");
    SYNC_AFTER_CHILD();
    
    if (flock(fd1, FLOCK_UN) != 0) FAIL("flock unlock failed");

    PASS();
}

int test_queued_writer_doesnt_block (void) {
    printf("(queued_writer_doesnt_block)...\n");

    int fd1 = open(FLOCK_TEST_PATH, O_CREATE | O_RDWR);
    if (fd1 < 0) FAIL("open failed");

    // Reader 1 acquires shared lock
    if (flock(fd1, FLOCK_SH) != 0) FAIL("flock shared lock failed");

    SYNC_BEFORE_CHILD();
    int child_pid_1 = sys_spawn(W_DOES_B, 1000);
    if (child_pid_1 == NUM_IDS) FAIL("spawn failed");

    SYNC_BEFORE_CHILD();
    int child_pid_2 = sys_spawn(R_EXP_NB, 1000);
    if (child_pid_2 == NUM_IDS) FAIL("spawn failed");
    SYNC_AFTER_CHILD(); // Reader 2 done

    if (flock(fd1, FLOCK_UN) != 0) FAIL("flock unlock failed");

    SYNC_AFTER_CHILD(); // Writer proceeds

    PASS();
}

int test_bad_fd (void) {
    printf("(bad fd)...\n");

    if (flock(16, FLOCK_SH) == 0) FAIL("flock should've failed");

    PASS();
}

int test_upgrade_flock(void) {
    printf("(upgrade flock)...\n");

    int fd1 = open(FLOCK_TEST_PATH, O_CREATE | O_RDWR);
    if (fd1 < 0) FAIL("open failed");

    if (flock(fd1, FLOCK_SH) != 0) FAIL("flock shared lock failed");

    SYNC_BEFORE_CHILD();
    int child_pid_1 = sys_spawn(R_EXP_NB, 1000);
    if (child_pid_1 == NUM_IDS) FAIL("spawn failed");
    SYNC_AFTER_CHILD();

    if (flock(fd1, FLOCK_EX) != 0) FAIL("flock upgrade to exclusive failed");

    SYNC_BEFORE_CHILD();
    child_pid_1 = sys_spawn(R_EXP_B, 1000);
    if (child_pid_1 == NUM_IDS) FAIL("spawn failed");
    SYNC_AFTER_CHILD();

    if (flock(fd1, FLOCK_UN) != 0) FAIL("flock unlock failed");

    PASS();
}

int test_downgrade_flock(void) {
    printf("(downgrade flock)...\n");

    int fd1 = open(FLOCK_TEST_PATH, O_CREATE | O_RDWR);
    if (fd1 < 0) FAIL("open failed");

    if (flock(fd1, FLOCK_EX) != 0) FAIL("flock exclusive lock failed");

    SYNC_BEFORE_CHILD();
    int child_pid_1 = sys_spawn(R_EXP_B, 1000);
    if (child_pid_1 == NUM_IDS) FAIL("spawn failed");
    SYNC_AFTER_CHILD();

    if (flock(fd1, FLOCK_SH) != 0) FAIL("flock downgrade to shared failed");

    SYNC_BEFORE_CHILD();
    child_pid_1 = sys_spawn(R_EXP_NB, 1000);
    if (child_pid_1 == NUM_IDS) FAIL("spawn failed");
    SYNC_AFTER_CHILD();

    if (flock(fd1, FLOCK_UN) != 0) FAIL("flock unlock failed");

    PASS();
}



/* ---- Test harness ------------------------------------------------- */
static int run_test(const char *label, int (*fn)(void)) {
    // Ensure a pristine environment: remove stale file before & after
    unlink(FLOCK_TEST_PATH);
    printf("[TEST] %s\n", label);
    int rc = fn();
    unlink(FLOCK_TEST_PATH);
    return rc;
}

int main(void)
{
    int failures = 0;

    failures += run_test("single_writer",               test_single_writer);
    // failures += run_test("multiple_readers",            test_multiple_readers); // not sure how to do this with my approach
    failures += run_test("writer_excludes_reader",      test_writer_excludes_reader);
    failures += run_test("writer_excludes_writer",      test_writer_excludes_writer);
    failures += run_test("reader_excludes_writer",      test_reader_excludes_writer);
    failures += run_test("queued_writer_doesnt_block",  test_queued_writer_doesnt_block);
    failures += run_test("bad_fd",                      test_bad_fd);
    failures += run_test("upgrade_flock",               test_upgrade_flock);
    failures += run_test("downgrade_flock",             test_downgrade_flock);

    printf("\n========== Summary =========\n");
    if (failures == 0)
    printf("All tests passed!\n");
    else
    printf("%d test(s) failed.\n", failures);
    printf("============================\n");
    
    // Use sys_exit so parent harness can see success/failure
    // sys_exit(failures);
    return failures; // not reached
}

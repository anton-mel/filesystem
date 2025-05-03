#include <proc.h>
#include <stdio.h>
#include <syscall.h>
#include <file.h>
#include "flocktests_common.h"

// Oliver TODO:
// Check how they did the tests in ftests
// do the same linking, create tests
// that covers every possible case to prove
// that it is working. If the FLOCK is not working
// debug it. My implemntation might be wrong.

#define FAIL(msg) do { printf("FAIL: %s\n", msg); return 1; } while (0)
#define PASS() do { printf("PASS\n"); return 0; } while (0)

/* ---- Basic correctness ----------------------------------------------- */

int test_single_writer (void) {
    printf("(single writer)...\n");

    int fd1 = open(FLOCK_TEST_PATH, O_CREATE | O_RDWR);
    if (fd1 < 0) FAIL("open failed");
    
    if (flock(fd1, FLOCK_EX) != 0) FAIL("flock exclusive lock failed");
    if (flock(fd1, FLOCK_UN) != 0) FAIL("flock unlock failed");

    PASS();
}

int test_multiple_readers (void) {
    printf("(multiple readers)...\n");

    int fd1 = open(FLOCK_TEST_PATH, O_CREATE);
    if (fd1 < 0) FAIL("open failed");

    int children_count = 5;
    int children[children_count];
    for (int i = 0; i < children_count; i++) {
        children[i] = spawn(R_EXP_NB, 100);
        printf("spawned %d\n", children[i]);
        if (children[i] < 0) FAIL("spawn failed");
    }

    int status = 10;
    for (int i = 0; i < children_count; i++) {
        status = 10;
        wait(children[i], &status);
        if (status != 0) FAIL("reader was blocked");
    }

    PASS();
}

/* ---- Mutual Exclusion ---------------------------------------------- */

int test_writer_excludes_reader (void) {
    printf("(writer excludes reader)...\n");

    int fd1 = open(FLOCK_TEST_PATH, O_CREATE);
    if (fd1 < 0) FAIL("open failed");

    if (flock(fd1, FLOCK_EX) != 0) FAIL("flock exclusive lock failed");
    int child_pid = spawn(R_EXP_B, 100);
    if (child_pid < 0) FAIL("spawn failed");

    int status = 10;
    wait(child_pid, &status);
    printf("status: %d\n", status);
    if (status != 0) FAIL("reader was not blocked");
    
    if (flock(fd1, FLOCK_UN) != 0) FAIL("flock unlock failed");

    PASS();
}

int test_writer_excludes_writer (void) {
    printf("(writer excludes writer)...\n");

    int fd1 = open(FLOCK_TEST_PATH, O_CREATE);
    if (fd1 < 0) FAIL("open failed");

    if (flock(fd1, FLOCK_EX) != 0) FAIL("flock exclusive lock failed");
    int child_pid = spawn(W_EXP_B, 100);
    if (child_pid < 0) FAIL("spawn failed");

    int status = 10;
    wait(child_pid, &status);
    if (status != 0) FAIL("writer was not blocked");
    
    if (flock(fd1, FLOCK_UN) != 0) FAIL("flock unlock failed");

    PASS();
}

int test_reader_excludes_writer (void) {
    printf("(reader_excludes_writer)...\n");

    int fd1 = open(FLOCK_TEST_PATH, O_CREATE);
    if (fd1 < 0) FAIL("open failed");

    if (flock(fd1, FLOCK_SH) != 0) FAIL("flock shared lock failed");
    int child_pid = spawn(W_EXP_B, 100);
    if (child_pid < 0) FAIL("spawn failed");

    int status = 10;
    wait(child_pid, &status);
    if (status != 0) FAIL("writer was not blocked");
    
    if (flock(fd1, FLOCK_UN) != 0) FAIL("flock unlock failed");

    PASS();
}

int test_queued_writer_does_block (void) {
    printf("(queued_writer_does_block)...\n");

    int fd1 = open(FLOCK_TEST_PATH, O_CREATE);
    if (fd1 < 0) FAIL("open failed");

    // create reader 1
    if (flock(fd1, FLOCK_SH) != 0) FAIL("flock shared lock failed");

    // create writer that will actually block and wait
    int child_pid_1 = spawn(W_DOES_B, 100);
    if (child_pid_1 < 0) FAIL("spawn failed");

    // create reader 2 that should block
    int child_pid_2 = spawn(R_EXP_B, 100);
    if (child_pid_2 < 0) FAIL("spawn failed");

    // make sure reader 2 was blocked
    int status = 10;
    wait(child_pid_2, &status);
    if (status != 0) FAIL("reader wasn't blocked");

    // unlock reader 1
    if (flock(fd1, FLOCK_UN) != 0) FAIL("flock unlock failed");

    // now writer should be able to get access
    status = 10;
    wait(child_pid_1, &status);
    if (status != 0) FAIL("reader was blocked"); // this wont be reached if this test fails

    PASS();
}

int test_bad_fd (void) {
    printf("(bad fd)...\n");

    if (flock(16, FLOCK_SH) == 0) FAIL("flock should've failed");

    PASS();
}

int test_upgrade_flock(void) {
    printf("(upgrade flock)...\n");

    int fd1 = open(FLOCK_TEST_PATH, O_CREATE);
    if (fd1 < 0) FAIL("open failed");

    if (flock(fd1, FLOCK_SH) != 0) FAIL("flock shared lock failed");

    int child_pid_1 = spawn(R_EXP_NB, 100);
    if (child_pid_1 < 0) FAIL("spawn failed");

    int status = 10;
    wait(child_pid_1, &status);
    if (status != 0) FAIL("reader was blocked");

    if (flock(fd1, FLOCK_EX) != 0) FAIL("flock shared lock failed");

    child_pid_1 = spawn(R_EXP_B, 100);
    if (child_pid_1 < 0) FAIL("spawn failed");

    status = 10;
    wait(child_pid_1, &status);
    if (status != 0) FAIL("reader wasn't blocked");

    if (flock(fd1, FLOCK_UN) != 0) FAIL("flock unlock failed");
    PASS();
}

int test_downgrade_flock(void) {
    printf("(downgrade flock)...\n");

    int fd1 = open(FLOCK_TEST_PATH, O_CREATE);
    if (fd1 < 0) FAIL("open failed");

    if (flock(fd1, FLOCK_EX) != 0) FAIL("flock shared lock failed");

    int child_pid_1 = spawn(R_EXP_B, 100);
    if (child_pid_1 < 0) FAIL("spawn failed");

    int status = 10;
    wait(child_pid_1, &status);
    if (status != 0) FAIL("reader wasn't blocked");

    if (flock(fd1, FLOCK_SH) != 0) FAIL("flock shared lock failed");

    child_pid_1 = spawn(R_EXP_NB, 100);
    if (child_pid_1 < 0) FAIL("spawn failed");

    status = 10;
    wait(child_pid_1, &status);
    if (status != 0) FAIL("reader was blocked");

    if (flock(fd1, FLOCK_UN) != 0) FAIL("flock unlock failed");
    PASS();
}



/* ---- Test harness ------------------------------------------------- */
static int run_test(const char *label, int (*fn)(void)) {
    // Ensure a pristine environment: remove stale file before & after
    unlink(FLOCK_TEST_PATH);
    printf("\n\n[TEST] %s\n", label);
    int rc = fn();
    unlink(FLOCK_TEST_PATH);
    return rc;
}

int main(void)
{
    int failures = 0;

    mkdir("/tmp");
    failures += run_test("single_writer",           test_single_writer);
    failures += run_test("multiple_readers",        test_multiple_readers);
    failures += run_test("writer_excludes_reader",  test_writer_excludes_reader);
    failures += run_test("writer_excludes_writer",  test_writer_excludes_writer);
    failures += run_test("reader_excludes_writer",  test_reader_excludes_writer);
    failures += run_test("queued_writer_does_block", test_queued_writer_does_block);
    failures += run_test("bad_fd",                  test_bad_fd);
    failures += run_test("upgrade_flock",           test_upgrade_flock);
    failures += run_test("downgrade_flock",         test_downgrade_flock);

    printf("\n====== Summary ======\n");
    if (failures == 0)
        printf("All tests passed!\n");
    else
        printf("%d test(s) failed.\n", failures);

    // Use sys_exit so parent harness can see success/failure
    // sys_exit(failures);
    return failures; // not reached
}

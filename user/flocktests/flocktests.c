#include <proc.h>
#include <stdio.h>
#include <syscall.h>
#include <file.h>

// Oliver TODO:
// Check how they did the tests in ftests
// do the same linking, create tests
// that covers every possible case to prove
// that it is working. If the FLOCK is not working
// debug it. My implemntation might be wrong.

#define TMO   2000   /* ms to wait before declaring a hang                */
#define RETRY 10000  /* iterations in stress/fuzzer loops                 */

/* ---- Basic correctness ----------------------------------------------- */

void single_writer (void) {
    int fd1 = open("single_writer.dat", O_CREATE | O_RDWR);
    if (fd1 < 0) { printf("open#1 failed\n"); exit(); }
    
}

/* ---- Mutual Exclusion ---------------------------------------------- */

/* ---- Non-Blocking --------------------------------------------------- */

/* ---- Re-entrancy --------------------------------------------------- */

/* ---- Upgrade Deadlock --------------------------------------------------- */

/* ---- Fairness/Starvation --------------------------------------------------- */

/* ---- Inheritance --------------------------------------------------- */

/* ---- Signal Interrupt --------------------------------------------------- */

/* ---- Edge Cases --------------------------------------------------- */

int
main(void)
{
    // This function is written by ChatGPT as the simpliest test
    // we are covering. It passes. Remove this (use the same style as in ftests).

    printf("[ex-nb] test start\n");

    int fd1 = open("flock_ex_nb.dat", O_CREATE | O_RDWR);
    if (fd1 < 0) { printf("open#1 failed\n"); return 1; }

    /* first handle takes the EX lock */
    if (flock(fd1, LOCK_EX) != 0) { printf("LOCK_EX(fd1) failed\n"); return 1; }

    int fd2 = open("flock_ex_nb.dat", O_RDWR);
    if (fd2 < 0) { printf("open#2 failed\n"); return 1; }

    int rc = flock(fd2, LOCK_EX | LOCK_NB);

    /* -------- verdict -------- */
    if (rc < 0)
        printf("PASS ✓  got EWOULDBLOCK\n");
    else
        printf("FAIL ✗  got %d, expected -2\n", rc);

    flock(fd1, LOCK_UN);
    close(fd2);
    close(fd1);
    
    printf("[ex-nb] test end\n");
    return 0;
}

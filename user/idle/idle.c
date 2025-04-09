#include <proc.h>
#include <stdio.h>
#include <syscall.h>
#include <x86.h>

int main(int argc, char **argv)
{
    printf("idle\n");

    while (1) {
        yield();
    }

    // NOTE: uncomment this to be able to spawn the tests,
    // but avoid spawning this along the bash. We cannot
    // sync it up yet in mCertikOS to execute after or after
    // so we decide to keep it simple and run 1 at a time.

    // pid_t fstest_pid;
    // if ((fstest_pid = spawn(4, 1000)) != -1)
    //     printf("fstest in process %d.\n", fstest_pid);
    // else
    //     printf("Failed to launch fstest.\n");

    return 0;
}

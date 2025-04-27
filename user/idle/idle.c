#include <proc.h>
#include <stdio.h>
#include <syscall.h>
#include <x86.h>

int main(int argc, char **argv)
{
    printf("idle\n");

    pid_t tests_pid;
    if ((tests_pid = spawn(6, 2000)) != -1)
        printf("fstest in process %d.\n", tests_pid);
    else
        printf("Failed to launch fstest.\n");

    return 0;
}

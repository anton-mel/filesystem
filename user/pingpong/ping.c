#include <proc.h>
#include <stdio.h>
#include <syscall.h>

int main(int argc, char **argv)
{
    printf("ping started.\n");

    pid_t pong_pid = spawn(2, 100);
    printf("ping spawned pong with pid %d.\n", pong_pid);

    int status= 5;
    wait(pong_pid, &status);
    printf("pong exited with status %d.\n", status);

    return 12;
}

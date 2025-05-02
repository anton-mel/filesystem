#include <proc.h>
#include <stdio.h>
#include <syscall.h>
#include <x86.h>

int main(int argc, char **argv)
{
    printf("idle\n");

    pid_t ping_pid = spawn(1, 1000);

    // printf("ping exited with status %d.\n", status);

    // for (int i = 0; i < 20; i++) {
    //     printf("\n\n\niteration %d\n", i);
    //     for (int j = 0; j < 5; j++) {
    //         if ((ping_pid = spawn(1, 10)) != -1)
    //             printf("ping in process %d.\n", ping_pid);
    //         else
    //             printf("Failed to launch ping.\n");
    //     }
    //     sys_yield();
    // }
}

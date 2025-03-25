#include <proc.h>
#include <stdio.h>
#include <syscall.h>

int main(int argc, char **argv)
{
    unsigned int i;
    printf("ping started.\n");

    // fast producing
    for (i = 0; i < 10; i++)
        produce();

    // slow producing
    for (i = 0; i < 40; i++)
    {
        if (i % 4 == 0)
            produce();
    }

    // Add for Clarity (Produce Task is Completed).
    printf("\033[1;33mPRODUCER EXITED! [POSSIBLY LAST LINE]\033[0m\n");
    return 0;
}

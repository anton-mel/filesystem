#include <proc.h>
#include <stdio.h>
#include <syscall.h>

int main(int argc, char **argv)
{
    unsigned int i;
    printf("pong started.\n");

    for (i = 0; i < 40; i++)
    {
        if (i % 2 == 0)
            consume();
    }

    // Add for Clarity (Consumer Task is Completed).
    printf("\033[1;33mCONSUMER EXITED! [POSSIBLY LAST LINE]\033[0m\n");
    return 0;
}

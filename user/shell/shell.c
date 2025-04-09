#include <proc.h>
#include <stdio.h>
#include <syscall.h>
#include <string.h>
#include <x86.h>
#include <file.h>
#include <gcc.h>

#define SIZE_BUF 1025
char buf [SIZE_BUF];



static int execute(char *buf) {
    if (strcmp(buf, "ls") == 0) {
        printf("ls\n");
    } else if (strcmp(buf, "cd") == 0) {
        // handle cd
    } else if (strcmp(buf, "pwd") == 0) {
        // handle pwd
    } else {
        printf("Unknown command: %s\n", buf);
    }

    return 0; // return -1 on error
}



int main(int argc, char *argv[])
{
    printf("*******shell starting*******\n");

    while (1) {
        sys_readline(buf);
        if (buf != NULL) {
            if (execute(buf) < 0) {
                printf("shell failed!\n");
                break;
            }
        }
    }

    printf("*******end of shell*******\n");
    
    return 0;
}




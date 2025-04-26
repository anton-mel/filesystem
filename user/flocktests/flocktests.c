#include <proc.h>
#include <stdio.h>
#include <syscall.h>
#include <x86.h>
#include <file.h>
#include <gcc.h>

#define exit(...) return __VA_ARGS__

char buf[8192];
char name[3];
char *echoargv[] = {"echo", "ALL", "TESTS", "PASSED", 0};

// Simple file system tests

int main(int argc, char *argv[])
{
    printf("*******usertests starting [flock]*******\n\n");

    printf("=====test file usertests.ran does not exists=====\n");

    if (open("usertests.ran", O_RDONLY) >= 0)
    {
        printf("already ran user tests (file usertests.ran exists) "
               "-- recreate certikos_disk.img\n");
        exit(1);
    }
    printf("=====test file usertests.ran does not exists: ok\n\n");
    close(open("usertests.ran", O_CREATE));
    
    printf("*******end of tests*******\n");

    return 0;
}

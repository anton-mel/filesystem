#include <proc.h>
#include <file.h>
#include <stdio.h>
#include <syscall.h>
#include <types.h>
#include <string.h>
#include <x86.h>
#include <gcc.h>

#define SIZE_BUF 1025
#define MAX_ARGS 16

char buf [SIZE_BUF];


/* Commands Logic */

static void exec_ls(int argc, char **argv) {
    // char *path = (argc < 2) ? "." : argv[1];

    // int fd = sys_open(path, 0);
    // struct stat st;
    // fstat(fd, &st);
  
    // if(st.type == T_DIR) {
    //   struct dirent de;
    //   while(read(fd, &de, sizeof(de)) == sizeof(de)) {
    //     if(de.inum == 0) continue;
    //     printf("%s\n", de.name);
    //   }
    // }
    // close(fd);
    // exit(0);
  
}

static void exec_pwd(int argc, char **argv) {

}

static void exec_cd(int argc, char *argv[]) {
    
}

static void exec_cp(int argc, char *argv[]) {
    
}

static void exec_rm(int argc, char *argv[]) {
    if (argc < 2) {
        printf("rm: missing operand\n");
        return;
    }

    for (int i = 1; i < argc; i++) {
        char *path = argv[i];
        int fd = open(path, O_RDONLY);
        if (fd < 0) {
            printf("rm: cannot open %s\n", path);
            continue;
        }

        struct file_stat stat;
        if (fstat(fd, &stat) < 0) {
            printf("rm: cannot stat %s\n", path);
            close(fd);
            continue;
        }

        if (stat.type == T_DIR) {
            printf("rm: %s is a directory (not removed)\n", path);
            close(fd);
            continue;
        }

        if (unlink(path) < 0) {
            printf("rm: failed to remove %s\n", path);
        }

        close(fd);
    }
}

static void exec_mkdir(int argc, char *argv[]) {
    
}

static void exec_cat(int argc, char *argv[]) {
    
}

static void exec_touch(int argc, char *argv[]) {
    
}


/* General Logic */

static int parse(char *line, char *argv[]) {
    int argc = 0;
    while (*line != '\0') {
        // Skip leading spaces
        while (*line == ' ') line++;
        if (*line == '\0') break;

        argv[argc++] = line;
        if (argc >= MAX_ARGS) break;

        // Find the end of the word
        while (*line && *line != ' ') line++;
        if (*line) *line++ = '\0';  // Null-terminate
    }
    argv[argc] = NULL;
    return argc;
}

static int execute(char *buf) {
    // first parse the input
    char *argv[MAX_ARGS];
    int argc = parse(buf, argv);
    if (argc == 0) return 0;

    if (strcmp(buf, "ls") == 0) {
        // either use the dir passed or
        // point to the current dir
        exec_ls(argc, argv);
    } else if (strcmp(buf, "cd") == 0) {
        if (argc < 2) {
            printf("cd: missing directory\n");
        } else {
            if (chdir(argv[1]) < 0) {
                printf("cd: failed to change to %s\n", argv[1]);
            }
        }
    } else if (strcmp(buf, "pwd") == 0) {
        return -1;
    } else if (strcmp(buf, "rm") == 0) {
        return -1;
    } else {
        printf("Unknown command: %s\n", buf);
    }

    return 0;
    // return -1 on error
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




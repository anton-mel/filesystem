#ifndef _USER_COMMON_H_
#define _USER_COMMON_H_

#include <proc.h>
#include <file.h>
#include <stdio.h>
#include <syscall.h>
#include <types.h>
#include <string.h>
#include <stdarg.h>
#include <x86.h>
#include <gcc.h>

#include "commands.h"

// Note sys_readline can handle 
// at most 1024 chars + 1 EOL.
#define SIZE_BUF 1025
#define MAX_ARGS 16

typedef status_t (*cmd_func_t)(int argc, char *argv[]);
typedef struct {
    const char *name;
    cmd_func_t func;
    int min_args;
} command_entry_t;

/* General Logic */
int parse(char *line, char *argv[]);
void print_status_error(status_t code);
void perror_msg(const char *fmt, ...);

#define false 0
#define true  1

#endif  /* _USER_COMMON_H_ */

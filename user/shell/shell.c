/* shell.c */

#include "common.h"
#include <fstat.h>
#include <dirent.h>

#define SIZE_BUF 1025
char buf [SIZE_BUF];

static int ls(char *path) {
    int fd = sys_open(path, 0); // read only
    struct file_stat st;
    sys_fstat(fd, &st);

    struct dirent de;
    if (st.type == T_DIR) {
        while (sys_read(fd, (char *) &de, sizeof(de)) == sizeof(de)) {
            printf("%s ", de.name);
        }
    }

    sys_close(fd);
    return 0;
}


// Command table
command_entry_t commands[] = {
    // cmd      fn_ptr       min_arg
    { "ls",     exec_ls,     0 },  // ls [optional_path]
    { "pwd",    exec_pwd,    0 },  // pwd
    { "cd",     exec_cd,     0 },  // cd [optional_path]
    { "cp",     exec_cp,     2 },  // cp <src> <dst>
    { "mv",     exec_mv,     2 },  // mv <src> <dst>
    { "rm",     exec_rm,     1 },  // rm <target>
    { "mkdir",  exec_mkdir,  1 },  // mkdir <dirname>
    { "cat",    exec_cat,    1 },  // cat <file>
    { "touch",  exec_touch,  1 },  // touch <file>
    { "write",  exec_write,  2 },  // write <file> <content>
    { "append", exec_append, 2 },  // append <file> <content>
    { "help",   exec_help,   0 },  // help
    { NULL,     NULL,        0 }
};

/* General Logic */

static status_t execute(char *line) {
    char *argv[MAX_ARGS];
    int argc = parse(line, argv);
    if (argc == 0) return SH_OK;

    const char *cmd = argv[0];

    for (command_entry_t *entry = commands; entry->name != NULL; entry++) {
        if (strcmp(cmd, entry->name) == 0) {
            // Handle the argument count internaly
            // to avoid copy pasting the code
            if ((argc - 1) < entry->min_args) {
                return SH_TOO_FEW_ARGS;
            }

            return entry->func(argc, argv);
        }
    }

    return SH_UNKNOWN_CMD;
}

int main(int argc, char *argv[])
{
    printf("\033[33m\n*****************************************\n\n");
    printf("Welcome to the mCertikOS SHELL interface!\n");
    printf("\n*****************************************\n\033[0m\n");
    printf("Type 'help' for a list of commands.\n\n");

    while (1) {
        sys_readline(buf);
        if (buf != NULL) {
            status_t status = execute(buf);
            if (status < 0) {
                print_status_error(status);
            }
        }
    }
    
    return 0;
}

/* Error Handling */

void print_status_error(status_t code) {
    if (code == SH_OK) return;

    for (int i = 0; status_messages[i].message != NULL; i++) {
        if (status_messages[i].code == code) {
            printf("\033[31m[ERROR] %s\033[0m\n", status_messages[i].message);
            return;
        }
    }

    // Fallback for unknown status code
    printf("\033[31m[ERROR] Unknown error code: %d\033[0m\n", code);
}

// Usage: perror_msg("%s", msg);
void perror_msg(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    printf("\033[31m[ERROR] ");
    vcprintf(fmt, args);
    printf("\033[0m\n");
    va_end(args);
}

// Print all available commands
status_t exec_help(int argc, char *argv[]) {
    printf("Available commands:\n");
    for (int i = 0; commands[i].name != NULL; i++) {
        printf("  %-6s  (min args: %d)\n", commands[i].name, commands[i].min_args);
    }
    return SH_OK;
}

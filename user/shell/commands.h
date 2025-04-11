#ifndef _USER_COMMANDS_H_
#define _USER_COMMANDS_H_

#include <file.h>
#include <stdio.h>

typedef enum {
    SH_OK = 0,
    SH_FAIL = -1,
    SH_TOO_FEW_ARGS = -2,
    SH_TOO_MANY_ARGS = -3,
    SH_UNKNOWN_CMD = -4,
    SH_IO_ERROR = -5,
    SH_CMD_NOT_DONE = -6
    // Add more as needed
} status_t;

typedef struct {
    status_t code;
    const char *message;
} status_msg_t;

static const status_msg_t status_messages[] = {
    { SH_FAIL,          "General failure occurred." },
    { SH_TOO_FEW_ARGS,  "Too few arguments provided." },
    { SH_TOO_MANY_ARGS, "Too many arguments provided." },
    { SH_UNKNOWN_CMD,   "Unknown command entered." },
    { SH_IO_ERROR,      "Input/output error occurred." },
    { SH_CMD_NOT_DONE,  "This command is not implemented yet." },
    { SH_OK,            NULL } // SH_OK should not print
};

/* Commands */
status_t exec_pwd(int argc, char *argv[]);
status_t exec_ls(int argc, char *argv[]);
status_t exec_cd(int argc, char *argv[]);
status_t exec_cp(int argc, char *argv[]);
status_t exec_rm(int argc, char *argv[]);
status_t exec_mkdir(int argc, char *argv[]);
status_t exec_cat(int argc, char *argv[]);
status_t exec_touch(int argc, char *argv[]);
status_t exec_help(int argc, char *argv[]);

#endif  /* _USER_COMMANDS_H_ */

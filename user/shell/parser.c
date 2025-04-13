/* parser.c */

#include "common.h"

/// We worked 3 hours, but realized this is not needed. Cry-cry.
// status_t recursive_exec(const char *path, status_t (*func_ptr)(char *)) {
//     int i = 0;
//     int MAX_TOKEN_LEN = 128;
//     char token[MAX_TOKEN_LEN];

//     while (path[i] != '\0') {
//         int j = 0;
//         while (path[i] != '\0' && path[i] != '/') {
//             ASSERT(j != MAX_TOKEN_LEN); // panic
//             token[j++] = path[i++];
//         }
//         token[j] = '\0';
//         if (path[i] == '/')
//             i++;

//         if (j > 0) {
//             status_t status = func_ptr(token);
//             if (status < 0) {
//                 return status;
//             }
//         }
//     }

//     return SH_OK;
// }

int parse(char *line, char *argv[]) {
    int argc = 0;

    while (*line != '\0') {
        // Skip leading spaces
        while (*line == ' ') line++;
        if (*line == '\0') break;

        if (*line == '"') {
            // Start of a quoted argument
            line++;  // Skip the opening quote
            argv[argc++] = line;
            if (argc >= MAX_ARGS) break;

            // Move to closing quote or end
            while (*line && *line != '"') line++;
            if (*line == '"') {
                *line = '\0'; // Null-terminate argument
                line++;       // Move past closing quote
            }
        } else {
            // Unquoted argument
            argv[argc++] = line;
            if (argc >= MAX_ARGS) break;

            while (*line && *line != ' ') line++;
            if (*line) *line++ = '\0';
        }
    }

    argv[argc] = NULL;
    return argc;
}

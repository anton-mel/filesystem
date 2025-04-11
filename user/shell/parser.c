/* parser.c */

#include "common.h"

int parse(char *line, char *argv[]) {
    // TODO
    // this is horrible function
    // please, redo it...
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

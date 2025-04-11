/* commands.c */

#include "common.h"

/* Command Logic */

status_t exec_ls(int argc, char *argv[]) {
    // TODO: this function is failing to locate itself
    // I will leave this to finish to @oliver.
    char *target = (argc < 2) ? "." : argv[1];
    int fd = open(target, O_RDONLY);

    if (fd < 0) {
        perror_msg("ls: cannot open %s", target);
        return SH_IO_ERROR;
    }

    struct file_stat st;
    if (fstat(fd, &st) < 0) {
        perror_msg("ls: cannot stat %s", target);
        close(fd);
        return SH_IO_ERROR;
    }

    if ((st.type == T_NONE)) {
        // Debug sys_create
        perror_msg("ls: file %s type if not set!", target);
        return SH_FAIL;
    } else if (st.type == T_FILE) {
        printf("%s\n", target);
    } else if (st.type == T_DIR) {
        struct dirent de;
        while (read(fd, (char *)&de, sizeof(de)) == sizeof(de)) {
            if (de.inum == 0) continue;
            printf("%s\n", de.name);
        }
    } else {
        perror_msg("ls: unsupported type for %s", target);
        close(fd);
        return SH_FAIL;
    }

    close(fd);
    return SH_OK;
}

status_t exec_pwd(int argc, char *argv[]) {
    // TODO
    return SH_CMD_NOT_DONE;
}

status_t exec_cd(int argc, char *argv[]) {
    // TODO
    return SH_CMD_NOT_DONE;
}

status_t exec_cp(int argc, char *argv[]) {
    // TODO
    return SH_CMD_NOT_DONE;
}

status_t exec_rm(int argc, char *argv[]) {
    for (int i = 1; i < argc; i++) {
        char *path = argv[i];
        int fd = open(path, O_RDONLY);
        if (fd < 0) {
            perror_msg("rm: cannot open %s", path);
            continue;
        }

        struct file_stat stat;
        if (fstat(fd, &stat) < 0) {
            perror_msg("rm: cannot stat %s", path);
            close(fd);
            continue;
        }

        if (stat.type == T_DIR) {
            perror_msg("rm: %s is a directory (not removed)", path);
            close(fd);
            continue;
        }

        if (unlink(path) < 0) {
            perror_msg("rm: failed to remove %s", path);
        }

        close(fd);
    }

    return SH_OK;
}

status_t exec_mkdir(int argc, char *argv[]) {
    for (int i = 1; i < argc; i++) {
        char *path = argv[i];

        if (mkdir(path) < 0) {
            perror_msg("mkdir: failed to create %s", path);
        }
    }

    return SH_OK;
}

status_t exec_cat(int argc, char *argv[]) {
    for (int i = 1; i < argc; i++) {
        char *path = argv[i];
        int fd = open(path, O_RDONLY);

        if (fd < 0) {
            perror_msg("cat: cannot open %s", path);
            continue;
        }

        struct file_stat stat;
        if (fstat(fd, &stat) < 0) {
            perror_msg("cat: cannot stat %s", path);
            close(fd);
            continue;
        }

        if (stat.type != T_FILE && stat.type != 0) {
            perror_msg("cat: %s is not a regular file", path);
            close(fd);
            continue;
        }

        char read_buf[1024];
        // char print_buf[1025];  // +1 for null terminator
        int n;
        bool present = false;

        while ((n = read(fd, read_buf, sizeof(read_buf))) > 0) {
            // Copy into null-terminated buffer for safe printf
            // strncpy(print_buf, read_buf, n);
            // print_buf[n] = '\0';
            // printf("%s\n", print_buf);
            present = true;
            puts(read_buf, n);
        }

        if (present) {
            // if content is present
            // space out the newline
            printf("\n");
        }

        if (close(fd) < 0) {
            perror_msg("cat: failed to close %s", path);
        }
    }

    return SH_OK;
}

// write <data> <file>
status_t exec_write(int argc, char *argv[]) {
    // TODO: fix this to be called as from > to
    if (argc < 3) {
        // requires a string to write and a target filename
        printf("Usage: write [from data] [to file]\n");
        return 0;
    }

    char *from_path = argv[2];
    char *to_path = argv[1];
    size_t to_path_len = strlen(to_path);

    int fd = open(from_path, O_CREATE | O_RDWR);
    if (fd < 0) {
        perror_msg("write: cannot open <file to> %s", from_path);
        return 0;
    }

    int n = write(fd, to_path, to_path_len);
    if (n < 0) {
        perror_msg("write: failed to write to file '%s'", to_path);
    } else if (n < to_path_len) {
        perror_msg("write: only wrote %d out of %d bytes to file '%s'", n, to_path_len, to_path);
    }

    if (close(fd) < 0) {
        perror_msg("touch: failed to close %s", to_path);
    }

    return SH_OK;
}

status_t exec_touch(int argc, char *argv[]) {
    for (int i = 1; i < argc; i++) {
        char *path = argv[i];
        int fd = open(path, O_RDONLY);

        if (fd < 0) {
            // File doesn't exist, try to create it
            fd = open(path, O_CREATE);
            if (fd < 0) {
                perror_msg("touch: cannot create %s", path);
                continue;
            }
        }

        if (close(fd) < 0) {
            perror_msg("touch: failed to close %s", path);
        }
    }

    return SH_OK;
}

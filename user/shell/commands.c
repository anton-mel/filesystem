/* commands.c */

#include "common.h"

/* Command Logic */

status_t exec_ls(int argc, char *argv[]) {
    // TODO: this function is failing to locate itself
    // I will leave this to finish to @oliver.
    const char *target = (argc < 2) ? "." : argv[1];
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

    if (st.type == T_FILE) {
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

        if (stat.type != T_FILE) {
            perror_msg("cat: %s is not a file", path);
            close(fd);
            continue;
        }

        int n;
        
        char buffer[1024];
        while ((n = read(fd, buffer, sizeof(buffer))) > 0) {
            for (int i = 0; i < n; i++) {
                // need to implement custom putc in userspace
                // can probably copy from the putc in the bootloader
            }
        }

        close(fd);
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

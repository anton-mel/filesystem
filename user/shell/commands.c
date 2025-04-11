/* commands.c */

#include "common.h"

/* Command Logic */

status_t exec_ls(int argc, char *argv[]) {
    // TODO
    return SH_CMD_NOT_DONE;

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
    // TODO
    return SH_CMD_NOT_DONE;
}

status_t exec_cat(int argc, char *argv[]) {
    // TODO
    return SH_CMD_NOT_DONE;
}

status_t exec_touch(int argc, char *argv[]) {
    // TODO
    return SH_CMD_NOT_DONE;
}

/* commands.c */

#include "common.h"

// Default Global Path Varibales
char cwd_path[MAX_PATH_LEN] = "/";

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
            if (strcmp(de.name, ".") == 0 || strcmp(de.name, "..") == 0)
                continue;
            printf("%s\t", de.name);
        }
        printf("\n");
    } else {
        perror_msg("ls: unsupported type for %s", target);
        close(fd);
        return SH_FAIL;
    }

    close(fd);
    return SH_OK;
}

status_t exec_pwd(int argc, char *argv[]) {
    printf("%s\n", cwd_path);
    return SH_OK;
}


status_t exec_cd(int argc, char *argv[]) {
    const char *target = (argc < 2) ? "/" : argv[1];

    // Open the target directory using O_DIRECTORY to ensure it's a directory.
    int fd = open(target, O_RDONLY);
    if (fd < 0) {
        perror_msg("cd: could not open directory: %s", target);
        return SH_IO_ERROR;
    }

    // Use fstat() on the open file descriptor to verify it is a directory.
    struct file_stat st;
    if (fstat(fd, &st) < 0) {
        perror_msg("cd: fstat failed for directory: %s", target);
        close(fd);
        return SH_IO_ERROR;
    }
    
    if (st.type != T_DIR) {
        perror_msg("cd: target is not a directory: %s, it is %d", target, st.type);
        close(fd);
        return SH_IO_ERROR;
    }
    
    // Once verified, close the descriptor.
    close(fd);

    // update both on syscall
    if (chdir((char *)target) < 0) {
        perror_msg("cd: could not change directories into: %s", target);
        return SH_IO_ERROR;
    }
    
    setCurrentDirectory(target);

    return SH_OK;
}

const char *get_basename(const char *path) {
    const char *base = strrchr(path, '/');
    return base ? base + 1 : path;
}

void remove_basename(char *path) {
    char *base = strrchr(path, '/');
    if (base != NULL) {
        *base = '\0';  // Truncate the string at the last '/'
    }
}

bool is_subdirectory(const char *src_path, const char *dst_path) {
    size_t len = strlen(src_path);
    return (strncmp(src_path, dst_path, len) == 0) && (dst_path[len] == '/' || dst_path[len] == '\0');
}

status_t copy_file(int src_fd, int dst_fd) {
    char buf[4096];

    ssize_t bytes;
    while ((bytes = read(src_fd, buf, sizeof(buf))) > 0) {
        if (write(dst_fd, buf, bytes) != bytes) {
            perror_msg("cp: write error");
            close(src_fd);
            close(dst_fd);
            return SH_IO_ERROR;
        }
    }

    if (bytes < 0)
    {
        perror_msg("cp: read error");
        return SH_IO_ERROR;
    }

    close(src_fd);
    close(dst_fd);

    return SH_OK;
}


status_t cp(const char *src_path, const char *dst_path, const char *original_dst_path, bool recursive)
{
    if (strcmp(src_path, original_dst_path) == 0) {
        return SH_OK;
    }

    int src_fd = open(src_path, O_RDONLY);
    if (src_fd < 0) {
        perror_msg("cp: cannot open source: %s", src_path);
        return SH_IO_ERROR;
    }

    struct file_stat src_st;
    if (fstat(src_fd, &src_st) < 0) {
        perror_msg("cp: cannot fstat source: %s", src_path);
        close(src_fd);
        return SH_IO_ERROR;
    }

    int dst_fd = open(dst_path, O_RDONLY);
    struct file_stat dst_st;
    bool dst_exists = (dst_fd >= 0);

    if (!dst_exists) {
        // case 1: Source is a file
        if (src_st.type == T_FILE) {
            dst_fd = open(dst_path, O_CREATE | O_WRONLY);
            if (dst_fd < 0) {
                perror_msg("cp: cannot create destination file: %s", dst_path);
                close(src_fd);
                return SH_IO_ERROR;
            }
            status_t ret = copy_file(src_fd, dst_fd);
            close(src_fd);
            close(dst_fd);
            return ret;
        }
        // case 2: Source is a directory
        else if (src_st.type == T_DIR) {
            if (!recursive) {
                perror_msg("cp: -r not specified; cannot copy directory");
                close(src_fd);
                return SH_FAIL;
            }

            if (mkdir(dst_path) < 0) {
                perror_msg("cp: mkdir failed: %s", dst_path);
                close(src_fd);
                return SH_IO_ERROR;
            }

            close(src_fd);
            src_fd = open(src_path, O_RDONLY);
            if (src_fd < 0) {
                perror_msg("cp: cannot reopen source dir: %s", src_path);
                return SH_IO_ERROR;
            }

            dst_fd = open(dst_path, O_RDONLY);
            if (dst_fd < 0) {
                perror_msg("cp: cannot open new destination dir: %s", dst_path);
                close(src_fd);
                return SH_IO_ERROR;
            }

            if (fstat(dst_fd, &dst_st) < 0) {
                perror_msg("cp: cannot fstat new destination dir: %s", dst_path);
                close(src_fd);
                close(dst_fd);
                return SH_IO_ERROR;
            }

            struct dirent de;
            while (read(src_fd, (char *)&de, sizeof(de)) == sizeof(de)) {
                if (de.inum == 0) continue;
                if (!strcmp(de.name, ".") || !strcmp(de.name, "..")) continue;

                char child_src[128];
                char child_dst[128];
                snprintf(child_src, sizeof(child_src), "%s/%s", src_path, de.name);
                snprintf(child_dst, sizeof(child_dst), "%s/%s", dst_path, de.name);

                status_t ret = cp(child_src, child_dst, original_dst_path, recursive);
                if (ret != SH_OK) {
                    close(src_fd);
                    close(dst_fd);
                    return ret;
                }
            }

            close(src_fd);
            close(dst_fd);
            return SH_OK;
        }
        else {
            // Some unsupported file type
            perror_msg("cp: unsupported source type");
            close(src_fd);
            return SH_FAIL;
        }
    }
    else {
        if (fstat(dst_fd, &dst_st) < 0) {
            perror_msg("cp: cannot fstat destination: %s", dst_path);
            close(src_fd);
            close(dst_fd);
            return SH_IO_ERROR;
        }

        // case 1: Source is a file
        if (src_st.type == T_FILE) {
            if (dst_st.type == T_FILE) {
                close(dst_fd); 
                // re-open for writing:
                int new_dst_fd = open(dst_path, O_WRONLY);
                if (new_dst_fd < 0) {
                    perror_msg("cp: cannot open destination for writing: %s", dst_path);
                    close(src_fd);
                    return SH_IO_ERROR;
                }
                status_t ret = copy_file(src_fd, new_dst_fd);
                close(src_fd);
                close(new_dst_fd);
                return ret;
            }
            else if (dst_st.type == T_DIR) {
                const char *base = get_basename(src_path);
                char new_dst_path[128];
                snprintf(new_dst_path, sizeof(new_dst_path), "%s/%s", dst_path, base);

                int new_dst_fd = open(new_dst_path, O_CREATE | O_WRONLY);
                if (new_dst_fd < 0) {
                    perror_msg("cp: cannot create file: %s", new_dst_path);
                    close(src_fd);
                    close(dst_fd);
                    return SH_IO_ERROR;
                }

                status_t ret = copy_file(src_fd, new_dst_fd);
                close(src_fd);
                close(dst_fd);
                close(new_dst_fd);
                return ret;
            }
            else {
                perror_msg("cp: destination has unsupported file type");
                close(src_fd);
                close(dst_fd);
                return SH_FAIL;
            }
        }
        // case 2: Source is directory
        else if (src_st.type == T_DIR) {
            if (!recursive) {
                perror_msg("cp: -r not specified; cannot copy directory");
                close(src_fd);
                close(dst_fd);
                return SH_FAIL;
            }

            if (dst_st.type == T_FILE) {
                perror_msg("cp: cannot copy directory into a file");
                close(src_fd);
                close(dst_fd);
                return SH_FAIL;
            }

            else if (dst_st.type == T_DIR) {
                const char *base = get_basename(src_path);
                char new_dst_dir[128];
                snprintf(new_dst_dir, sizeof(new_dst_dir), "%s/%s", dst_path, base);

                if (mkdir(new_dst_dir) < 0) {
                    perror_msg("cp: mkdir failed: %s", new_dst_dir);
                    close(src_fd);
                    close(dst_fd);
                    return SH_IO_ERROR;
                }

                // Reopen src to read from it again (since we did fstat, we need a fresh FD pointer)
                close(src_fd);
                src_fd = open(src_path, O_RDONLY);
                if (src_fd < 0) {
                    perror_msg("cp: cannot reopen source dir: %s", src_path);
                    close(dst_fd);
                    return SH_IO_ERROR;
                }

                struct dirent de;
                while (read(src_fd, (char *)&de, sizeof(de)) == sizeof(de)) {
                    if (de.inum == 0) continue;
                    if (!strcmp(de.name, ".") || !strcmp(de.name, "..")) continue;

                    char child_src[128];
                    char child_dst[128];
                    snprintf(child_src, sizeof(child_src), "%s/%s", src_path, de.name);
                    snprintf(child_dst, sizeof(child_dst), "%s/%s", new_dst_dir, de.name);

                    status_t ret = cp(child_src, child_dst, original_dst_path, recursive);
                    if (ret != SH_OK) {
                        close(src_fd);
                        close(dst_fd);
                        return ret;
                    }
                }

                close(src_fd);
                close(dst_fd);
                return SH_OK;
            } 
            else {
                perror_msg("cp: destination has unsupported file type");
                close(src_fd);
                close(dst_fd);
                return SH_FAIL;
            }
        }
        else {
            perror_msg("cp: unsupported source type");
            close(src_fd);
            close(dst_fd);
            return SH_FAIL;
        }
    }

    // Should never get here
    return SH_OK;
}

status_t exec_cp(int argc, char *argv[]) {
    // TODO
    bool recursive = false;
    if (argc > 4) {
        // limit on the maximum # of arguments
        perror_msg("Usage: cp [-r] <source> <destination>\n");
        return SH_TOO_MANY_ARGS;
    } else if (argc == 4) {
        if (strcmp(argv[1], "-r") != 0) {
            perror_msg("Usage: cp [-r] <source> <destination>\n");
            return SH_INVALID_ARGS;    
        }

        recursive = true;
    }

    const char *src = argv[argc - 2];
    const char *dst = argv[argc - 1];

    const char *base = get_basename(src);
    char new_dst_path[128];
    // TODO: Assure this new concatenation is within PATH_MAX length
    snprintf(new_dst_path, sizeof(new_dst_path), "%s/%s", dst, base);

    return cp(src, dst, new_dst_path, recursive);
}

status_t exec_mv(int argc, char *argv[]) {
    // TODO
    if (argc > 3) {
        // limit on the maximum # of arguments
        printf("Usage: mv <source> <destination>\n");
        return SH_TOO_MANY_ARGS;
    }

    const char *src = argv[1];
    const char *dst = argv[2];

    int src_fd = open((char *)src, O_RDONLY);
    if (src_fd < 0) {
        perror_msg("mv: cannot open source %s", src);
        return SH_IO_ERROR;
    }

    int dst_fd = open((char *)dst, O_RDONLY);
    if (dst_fd < 0) {
        perror_msg("mv: cannot open destination %s", src);
        return SH_IO_ERROR;
    }

    // Move by creating a new link 
    // and removing the old one
    // Need some way to recoursively go
    // through the global CWD path...
    return SH_OK;
}

/**
 * Recursively remove the directory specified by 'dirpath'.
 * First removes all contained files/directories recursively, 
 * then removes the directory itself.
 */
static status_t rm_dir(const char *dirpath) {
    int fd = open((char *) dirpath, O_RDONLY);
    if (fd < 0) {
        return SH_IO_ERROR;
    }

    struct dirent de;
    char fullpath[MAX_PATH_LEN];
    status_t ret = SH_OK;

    while (read(fd, (char *)&de, sizeof(de))) {
        if (de.inum != 0 && strcmp(de.name, ".") != 0 && strcmp(de.name, "..") != 0) {
            concatenatePaths(fullpath, dirpath, de.name);

            // Dummy call
            char *sub_argv[2];
            sub_argv[0] = "rm";
            sub_argv[1] = fullpath;
            
            ret = exec_rm(2, sub_argv);
            if (ret != SH_OK) {
                close(fd);
                return ret;
            }
        }
    }
    close(fd);

    if (unlink((char *) dirpath) < 0) {
        return SH_IO_ERROR;
    }
    return SH_OK;
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
            status_t ret = rm_dir(path);
            if (ret != SH_OK) {
                perror_msg("rm: failed to remove directory %s", path);
            }
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
    if (argc < 3) {
        printf("Usage: write [from data] [to file]\n");
        return SH_TOO_FEW_ARGS;
    }

    char *data = argv[1];
    char *filename = argv[2];
    size_t data_len = strlen(data);

    // Unfortunately keeping track of EOF
    // might be a bit challenging, so we
    // just remove the file completely.
    unlink(filename);

    // Create a new file (empty)
    int fd = open(filename, O_CREATE | O_RDWR);
    if (fd < 0) {
        perror_msg("write: cannot create file %s", filename);
        return SH_IO_ERROR;
    }

    int n = write(fd, data, data_len);
    if (n < 0) {
        perror_msg("write: failed to write to file '%s'", filename);
        close(fd);
        return SH_IO_ERROR;
    } else if ((size_t)n < data_len) {
        perror_msg("write: only wrote %d out of %lu bytes to file '%s'", n, data_len, filename);
    }

    if (close(fd) < 0) {
        perror_msg("write: failed to close file %s", filename);
        return SH_IO_ERROR;
    }

    return SH_OK;
}

// append <data> <file>
status_t exec_append(int argc, char *argv[]) {
    if (argc < 3) {
        printf("Usage: append [from data] [to file]\n");
        return SH_TOO_FEW_ARGS;
    }

    char *data   = argv[1];
    char *target = argv[2];
    size_t data_len = strlen(data);

    int fd = open(target, O_CREATE | O_RDWR);
    if (fd < 0) {
        perror_msg("append: cannot open file %s", target);
        return SH_IO_ERROR;
    }

    size_t offset = 0;
    char buf[256];
    ssize_t n;
    while ((n = read(fd, buf, sizeof(buf))) > 0) {
        offset += n;
    }

    close(fd);
    fd = open(target, O_CREATE | O_RDWR);
    if (fd < 0) {
        perror_msg("append: cannot reopen file %s", target);
        return SH_IO_ERROR;
    }

    size_t moved = 0;
    while (moved < offset) {
        size_t chunk = (offset - moved) > sizeof(buf) ? sizeof(buf) : (offset - moved);
        if (read(fd, buf, chunk) <= 0) {
            break;
        }
        moved += chunk;
    }

    n = write(fd, data, data_len);
    if (n < 0) {
        perror_msg("append: failed to write to file '%s'", target);
        close(fd);
        return SH_IO_ERROR;
    } else if ((size_t)n < data_len) {
        perror_msg("append: only wrote %d out of %lu bytes to file '%s'", n, data_len, target);
    }

    if (close(fd) < 0) {
        perror_msg("append: failed to close file %s", target);
        return SH_IO_ERROR;
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


/****** Helpers ******/

// Updates the global current directory given an input path.
void setCurrentDirectory(const char *inputPath) {
    if (inputPath == NULL || *inputPath == '\0') {
        strncpy(cwd_path, "/", MAX_PATH_LEN);
        cwd_path[MAX_PATH_LEN - 1] = '\0';
        return;
    }

    if (inputPath[0] == '/') {
        // Input is an absolute path – overwrite
        size_t pathSize = strlen(inputPath);
        if (pathSize >= MAX_PATH_LEN) {
            PANIC("cwd path too long.");
            return;
        }
        memcpy(cwd_path, inputPath, pathSize);
        cwd_path[pathSize] = '\0';
    } else {
        // Input is a relative path – append.
        size_t currentLen = strlen(cwd_path);
        size_t addLen = strlen(inputPath);

        // Ensure room for an extra '/'
        if (currentLen + 1 + addLen >= MAX_PATH_LEN) {
            PANIC("cwd path too long.");
            return;
        }

        // Add a trailing '/' if one is not already present.
        if (cwd_path[currentLen - 1] != '/') {
            cwd_path[currentLen] = '/';
            currentLen++;
        }

        // Append the new path filename.
        for (size_t i = 0; i < addLen; i++) {
            cwd_path[currentLen + i] = inputPath[i];
        }
        cwd_path[currentLen + addLen] = '\0';
    }

    fixPathFormatting(cwd_path);
}

// Concatenates two path segments into destination.
// 'base' and 'addition' are joined with a '/' as needed.
void concatenatePaths(char *dest, const char *base, const char *addition) {
    if (dest != base) {
        strcpy(dest, base);
    }

    size_t baseLen = strlen(base);
    dest += baseLen;
    if (baseLen > 0 && *(dest - 1) != '/') {
        *dest = '/';
        dest++;
    }

    strcpy(dest, addition);
}

// Extracts the first segment from a relative path and returns a pointer to the remainder.
// The extracted segment is terminated by inserting a '\0' in place of the first '/'.
char *extractSegment(char *pathStr) {
    char *tokenPtr = pathStr;
    while (*tokenPtr && *tokenPtr != '/') {
        tokenPtr++;
    }

    if (*tokenPtr == '/') {
        *tokenPtr = '\0';
        char *remainder = tokenPtr + 1;

        // Skip any consecutive '/'
        while (*remainder == '/' && *remainder != '\0') {
            remainder++;
        }
        if (*remainder == '\0') {
            remainder = NULL;
        }
        return remainder;
    }

    return NULL;
}

// Normalizes the given absolute 
// path by resolving '.' and '..' tokens.
void fixPathFormatting(char *pathStr) {
    int originalSize = strlen(pathStr);
    char *tempBuffer = (char *)user_alloc(originalSize + 1);
    if (!tempBuffer) {
        return;
    }
    char *bufEnd = tempBuffer;
    char *originalPtr = pathStr;  // final normalized string

    // The input must begin with a '/'
    *bufEnd++ = '/';
    pathStr++;

    while (pathStr != NULL) {
        // Tokenize the next component from the path.
        char *nextPart = extractSegment(pathStr);

        if (strcmp(pathStr, ".") == 0) {
            // Skip current directory tokens.
        } else if (strcmp(pathStr, "..") == 0) {
            while (bufEnd > tempBuffer && *(bufEnd - 1) != '/') {
                bufEnd--;
            }
        } else if (pathStr[0] != '\0') {
            if (*(bufEnd - 1) != '/') {
                *bufEnd++ = '/';
            }
            strcpy(bufEnd, pathStr);
            bufEnd += strlen(pathStr);
        }

        pathStr = nextPart;
    }

    // Remove trailing slash unless it's the root "/"
    if (bufEnd > tempBuffer + 1 && *(bufEnd - 1) == '/') {
        bufEnd--;
    }

    *bufEnd = '\0';
    strcpy(originalPtr, tempBuffer);
}

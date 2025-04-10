#ifndef _FSTAT_H_
#define _FSTAT_H_

#define T_DIR  1
#define T_FILE 2
#define T_DEV  3

struct file_stat {
    short type;
    unsigned int dev;
    unsigned int ino;
    unsigned short nlink;
    unsigned int size;
};

#endif
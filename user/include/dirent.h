#ifndef _DIRENT_H_
#define _DIRENT_H_

// Directory is a file containing a sequence of dirent structures.
#define DIRSIZ 14

struct dirent {
    uint16_t inum;
    char name[DIRSIZ];
};

#endif
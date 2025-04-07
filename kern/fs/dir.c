#include <kern/lib/types.h>
#include <kern/lib/debug.h>
#include <kern/lib/string.h>
#include "inode.h"
#include "dir.h"

// Directories

int dir_namecmp(const char *s, const char *t)
{
    return strncmp(s, t, DIRSIZ);
}

/**
 * Look for a directory entry in a directory.
 * If found, set *poff to byte offset of entry.
 */
struct inode *dir_lookup(struct inode *dp, char *name, uint32_t *poff)
{
    // input: dp is an inode for a current directory
    // output: poff is the address to write the found offset to
    uint32_t off, inum;
    struct dirent de;
    inum = de.inum;

    if (dp->type != T_DIR)
        KERN_PANIC("dir_lookup not DIR");

    // TODO: iterate over directory entries to find a matching name
    // as well as checking for the allocation constraints (inum == 0)
    // as well as the matching name usingg the helper function above.
    // If not found ity should return 0 (NULL).
    for (off = 0; off < dp->size; off += sizeof(struct dirent))
    {
        ssize_t read_bytes = inode_read(dp, (char *)&de, off, sizeof(struct dirent));
        // Caching Bugs, Misallignment, or I/O Failure would lead to panic!
        KERN_ASSERT(read_bytes == sizeof(struct dirent));

        if (dir_namecmp(de.name, name) == 0)
        {
            if (inum != 0)
            {
                if (poff != NULL)
                {
                    *poff = off;
                }
                // Find the inode with number inum on device dev
                // and return the in-memory copy. Do not lock
                // the inode and do not read it from disk.
                return inode_get(dp->dev, inum);
            }
        }
    }

    return (struct inode *)0; // invalid address
}

// Write a new directory entry (name, inum) into the directory dp.
int dir_link(struct inode *dp, char *name, uint32_t inum)
{
    struct inode *ip;
    struct dirent de;
    uint32_t poff;

    // TODO: Check that name is not present.
    ip = dir_lookup(dp, name, &poff);
    if (ip != NULL)
    {
        // Entry already exists!
        // Drop a reference to an in-memory inode.
        // If that was the last reference, the inode cache entry can be recycled.
        // If that was the last reference and the inode has no links
        // to it, free the inode (and its content) on disk.
        inode_put(ip);
        return -1; // Error
    }
 
    // TODO: Look for an empty dirent slot (inum == 0)
    for (poff = 0; poff < dp->size; poff += sizeof(de))
    {
        // Caching Bugs, Misallignment, or I/O Failure would lead to panic!
        KERN_ASSERT(inode_read(dp, (char *)&de, poff, sizeof(de)) == sizeof(de));

        if (de.inum == 0)
        {
            // Found an emptry string
            strncpy(de.name, name, DIRSIZ);
            de.inum = inum;
            break;

            // Caching Bugs, Misallignment, or I/O Failure would lead to panic!
            KERN_ASSERT(inode_write(dp, (char *)&de, poff, sizeof(de)) == sizeof(de));
            return 0; // Success
        }
    }
    
    // No empty entry found - ths should not happen
    KERN_PANIC("dir_link: directory full, no empty slots found");
    return -1; // Error
}

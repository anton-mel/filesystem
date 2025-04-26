/* ---------------------------------------------------------------------- */
/* flock.h — minimal in-kernel BSD-flock definitions                      */
/* ---------------------------------------------------------------------- */

#ifndef _KERN_FLOCK_H_
#define _KERN_FLOCK_H_

#include <lib/spinlock.h>
#include <lib/condvar.h>

/* user-visible flags */
#define LOCK_SH  1    /* shared */
#define LOCK_EX  2    /* exclusive */
#define LOCK_NB  4    /* don’t block */
#define LOCK_UN  8    /* unlock   */

/* internal flag */
#define FL_SLEEP 0x04 /* request may sleep */

/* forward declare your file structure */
typedef struct file file_t;

/* one outstanding lock request (granted or waiting) */
typedef struct file_lock {
    file_t            *fl_file;       /* file this lock is on */
    void              *fl_owner;      /* open-file-description identifier */
    unsigned int       fl_pid;        /* tgid of the owner */
    unsigned char      fl_type;       /* LOCK_SH, LOCK_EX, or LOCK_UN */
    unsigned char      fl_flags;      /* FL_SLEEP when blocking allowed */
    unsigned long      fl_start;      /* normally 0 */
    unsigned long      fl_end;        /* normally ~0 (whole file) */

    /* simple singly-linked lists */
    struct file_lock  *next_granted;  /* next in granted list */
    struct file_lock  *next_blocked;  /* next in blocked list */

    CV                 fl_wait;       /* CV to sleep on if blocked */
} file_lock_t;

/* per-inode (or global) lock context */
typedef struct file_lock_context {
    spinlock_t         lock;          /* protects both lists */
    file_lock_t       *granted;       /* head of granted locks */
    file_lock_t       *blocked;       /* head of waiting requests */
} file_lock_context_t;

#endif /* _KERN_FLOCK_H_ */

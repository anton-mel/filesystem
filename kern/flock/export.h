#ifndef _KERN_FLOCK_H_
#define _KERN_FLOCK_H_

#ifdef _KERN_

#include <fs/file.h>

/*
 * Initialize the flock subsystem.
 *   Call once at boot (or module load) before any flock_acquire()/release().
 */
void flock_init(struct flock *flock);

/*
 * Acquire or change a BSD flock on file descriptor fd.
 *   op is one of LOCK_SH, LOCK_EX, or LOCK_UN, possibly OR’d with LOCK_NB.
 *   Returns 0 on success, –1 on error (errno set by caller).
 */
int flock_acquire(struct flock *flock, unsigned operation);

/*
 * Alias for unlocking: you can also call flock_release(fd).
 */
int flock_release(struct flock *flock);


#endif  /* _KERN_ */

#endif  /* !_KERN_FLOCK_H_ */

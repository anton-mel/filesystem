/*----------------------------------------------------------------------
 * flock.h  —  minimal reader / writer lock for the kernel file layer
 *
 *  •   any number of concurrent SHARED (reader) holders
 *  •   exactly one EXCLUSIVE (writer) holder
 *  •   FIFO fairness so queued writers don’t starve
 *----------------------------------------------------------------------*/
#ifndef KERN_FLOCK_H_
#define KERN_FLOCK_H_

#include <lib/spinlock.h>
#include <lib/condvar.h>

/* ---- public API flags ------------------------------------------------ */
#define FLOCK_SH  (1U << 0)   /* shared/read            */
#define FLOCK_EX  (1U << 1)   /* exclusive/write        */
#define FLOCK_UN  (1U << 2)   /* unlock                 */
#define FLOCK_NB  (1U << 3)   /* non-blocking acquire   */

#define FLOCK_EWOULDBLOCK  (-2)

/* ---- internal state -------------------------------------------------- */
enum lock_state {
    LOCK_IDLE = 0,
    LOCK_SHARED,
    LOCK_EXCLUSIVE
};

struct wait_queue {
    int  count;   /* number of sleepers in this queue          */
    CV   cv;      /* condition variable used to block / wake   */
};

struct flock {
    /* current ownership ------------------------------------------------ */
    int            active_readers;   /* # readers holding the lock        */
    bool           active_writer;    /* true if an exclusive holder exists */
    enum lock_state state;           /* fast discriminator                */

    /* synchronisation primitives --------------------------------------- */
    spinlock_t     mtx;              /* protects entire struct            */
    struct wait_queue wq_readers;    /* waiting readers                   */
    struct wait_queue wq_writers;    /* waiting writers                   */
};

/* ===================  interface  ===================================== */
void flock_init      (struct flock *lk);
int  flock_acquire   (struct flock *lk, unsigned op);
int  flock_release   (struct flock *lk);

/* Convenience wrapper: returns 0 on success, FLOCK_EWOULDBLOCK on failure */
static inline int
flock_try_acquire(struct flock *lk, unsigned op)
{
    return flock_acquire(lk, op | FLOCK_NB);
}

#endif /* KERN_FLOCK_H_ */

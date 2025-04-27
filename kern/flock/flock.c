/*----------------------------------------------------------------------
 * flock.c  —  implementation of reader / writer flock
 *----------------------------------------------------------------------*/
#include "flock.h"
#include <kern/lib/debug.h>

/* ---- small helpers --------------------------------------------------- */
static inline bool want_writer(unsigned op) { return op & FLOCK_EX; }
static inline bool want_reader(unsigned op) { return op & FLOCK_SH; }
static inline bool is_nonblock(unsigned op) { return op & FLOCK_NB; }

/* ---- init ------------------------------------------------------------ */
void
flock_init(struct flock *lk)
{
    lk->active_readers = 0;
    lk->active_writer  = FALSE;
    lk->state          = LOCK_IDLE;

    lk->wq_readers.count = 0;
    lk->wq_writers.count = 0;

    spinlock_init(&lk->mtx);
    CV_init(&lk->wq_readers.cv);
    CV_init(&lk->wq_writers.cv);
}

/* ---- acquire --------------------------------------------------------- */
int
flock_acquire(struct flock *lk, unsigned op)
{
    /* reject requests that ask for neither SH nor EX, or for both */
    if (!(want_reader(op) ^ want_writer(op)))
        return -1;

    spinlock_acquire(&lk->mtx);

    /* ===== writer ===================================================== */
    if (want_writer(op)) {
        if (lk->state == LOCK_IDLE) {
            lk->state         = LOCK_EXCLUSIVE;
            lk->active_writer = TRUE;
            spinlock_release(&lk->mtx);
            return 0;
        }
        if (is_nonblock(op)) {
            spinlock_release(&lk->mtx);
            return FLOCK_EWOULDBLOCK;
        }

        /* wait until lock becomes idle */
        lk->wq_writers.count++;
        while (lk->state != LOCK_IDLE)
            CV_wait(&lk->wq_writers.cv, &lk->mtx);
        lk->wq_writers.count--;

        lk->state         = LOCK_EXCLUSIVE;
        lk->active_writer = TRUE;
        spinlock_release(&lk->mtx);
        return 0;
    }

    /* ===== reader ===================================================== */
    if (lk->state != LOCK_EXCLUSIVE && lk->wq_writers.count == 0) {
        /* fast path */
        lk->state = LOCK_SHARED;
        lk->active_readers++;
        spinlock_release(&lk->mtx);
        return 0;
    }

    if (is_nonblock(op)) {
        spinlock_release(&lk->mtx);
        return FLOCK_EWOULDBLOCK;
    }

    /* slow path: wait while a writer is active or queued */
    lk->wq_readers.count++;
    while (lk->state == LOCK_EXCLUSIVE || lk->wq_writers.count)
        CV_wait(&lk->wq_readers.cv, &lk->mtx);
    lk->wq_readers.count--;

    lk->state = LOCK_SHARED;
    lk->active_readers++;
    spinlock_release(&lk->mtx);
    return 0;
}

/* ---- release --------------------------------------------------------- */
int
flock_release(struct flock *lk)
{
    spinlock_acquire(&lk->mtx);

    if (lk->state == LOCK_IDLE) {        /* nothing to release */
        spinlock_release(&lk->mtx);
        return -1;
    }

    if (lk->state == LOCK_EXCLUSIVE) {   /* writer releasing */
        lk->active_writer = FALSE;
        lk->state         = LOCK_IDLE;
    } else {                             /* reader releasing */
        if (--lk->active_readers == 0)
            lk->state = LOCK_IDLE;
    }

    /* wake next in FIFO order: writers first, else all readers */
    if (lk->state == LOCK_IDLE) {
        if (lk->wq_writers.count)
            CV_signal(&lk->wq_writers.cv);
        else
            CV_broadcast(&lk->wq_readers.cv);
    }

    spinlock_release(&lk->mtx);
    return 0;
}

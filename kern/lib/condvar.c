// Bouded Buffer
//
//      Given book definition.

#include <lib/condvar.h>
#include <lib/debug.h>
#include <lib/string.h>
#include <lib/thread.h>
#include <dev/intr.h>
#include <thread/PCurID/export.h>
#include <thread/PThread/export.h>

void CV_init(CV *cv)
{
    cv->tail = 0;
    memzero(cv->queue, sizeof(cv->queue));
}

static void CV_enqueue(CV *cv, unsigned int pid)
{
    DIS_INTR({
        KERN_ASSERT(pid > 0 && pid < NUM_IDS);
        KERN_ASSERT(cv->tail < NUM_IDS);
        KERN_ASSERT(cv->queue[cv->tail] == 0);

        cv->queue[cv->tail] = pid;
        cv->tail = pid;
    });
}

static unsigned int CV_dequeue(CV *cv)
{
    unsigned int pid = cv->queue[0];

    if (pid != 0)
    {
        cv->queue[0] = cv->queue[pid];
        cv->queue[pid] = 0;
        if (cv->queue[0] == 0)
        {
            cv->tail = 0;
        }
    }

    return pid;
}

void CV_wait(CV *cv, spinlock_t *lock)
{
    unsigned int cur_pid = get_curid();
    DIS_INTR({
        KERN_ASSERT(spinlock_holding(lock));
        CV_enqueue(cv, cur_pid);
        thread_suspend(lock, cur_pid);
    });

    spinlock_acquire(lock);
}

void CV_signal(CV *cv)
{
    unsigned int pid = CV_dequeue(cv);
    if (pid)
    {
        DIS_INTR(thread_ready(pid));
    }
}

void CV_broadcast(CV *cv)
{
    unsigned int pid;
    while ((pid = CV_dequeue(cv)) != 0)
    {
        DIS_INTR(thread_ready(pid));
    }
}

void BB_init(BoundedBuffer *bb)
{
    memzero(bb->buf, sizeof(bb->buf));
    bb->head = bb->size = 0;
    spinlock_init(&bb->lock);
    CV_init(&bb->empty);
    CV_init(&bb->full);
}

bool BB_is_empty(const BoundedBuffer *bb)
{
    return bb->size == 0;
}

bool BB_is_full(const BoundedBuffer *bb)
{
    return bb->size == BUFFER_CAPACITY;
}

void BB_enqueue(BoundedBuffer *bb, unsigned int val)
{
    spinlock_acquire(&bb->lock);

    while (BB_is_full(bb))
    {
        CV_wait(&bb->full, &bb->lock);
    }

    bb->buf[(bb->head + bb->size) % BUFFER_CAPACITY] = val;
    bb->size++;

    CV_signal(&bb->empty);
    spinlock_release(&bb->lock);
}

unsigned int BB_dequeue(BoundedBuffer *bb)
{
    spinlock_acquire(&bb->lock);

    while (BB_is_empty(bb))
    {
        CV_wait(&bb->empty, &bb->lock);
    }

    unsigned int val = bb->buf[bb->head];
    bb->head = (bb->head + 1) % BUFFER_CAPACITY;
    bb->size--;

    CV_signal(&bb->full);
    spinlock_release(&bb->lock);
    return val;
}

// Conditional Variable & Bounded-Buffer Library
//
//      CPSC 422 @ System Programming Course
//      by Anton Melnychuk <anton.melnychuk@yale.edu>

#include <lib/condvar.h>
#include <lib/debug.h>
#include <lib/string.h>
#include <lib/thread.h>
#include <dev/intr.h>
#include <thread/PCurID/export.h>
#include <pcpu/PCPUIntro/export.h>
#include <thread/PThread/export.h>


// ----------- Condition Variable Implementation -----------

void CV_init(CV *cv)
{
    cv->tail = 0;
    memzero(cv->queue, sizeof(cv->queue));
}

static void CV_enqueue(CV *cv, unsigned int pid)
{
    DISI(
        // Ensure proper placement.
        KERN_ASSERT(pid > 0 && pid < NUM_IDS);
        KERN_ASSERT(cv->queue[cv->tail] == 0);
        KERN_ASSERT(cv->tail < NUM_IDS)
    );

    cv->queue[cv->tail] = pid;
    cv->tail = pid;
}

static unsigned int CV_dequeue(CV *cv)
{
    unsigned int pid = cv->queue[0];

    if (cv->queue[pid] == 0) {
        cv->tail = 0;
    }
    cv->queue[0] = cv->queue[pid];
    cv->queue[pid] = 0;

    return pid;
}

void CV_wait(CV *cv, spinlock_t *lock)
{
    unsigned int cur_pid = get_curid();
    DISI(
        KERN_ASSERT(spinlock_holding(lock))
        // BB problem definition.
    );
    CV_enqueue(cv, cur_pid);
    DISI(
        thread_suspend(lock, cur_pid)
    );

    spinlock_acquire(lock);
}

void CV_signal(CV *cv)
{
    unsigned int pid = CV_dequeue(cv);
    if (pid)
    {
        // Wake up upper thread.
        DISI(thread_ready(pid));
    }
}

void CV_broadcast(CV *cv)
{
    unsigned int pid;
    while ((pid = CV_dequeue(cv)) != 0)
    {
        // Wake up all threads.
        DISI(thread_ready(pid));
    }
}


// ----------- Bounded Buffer Implementation -----------

void BB_init(BoundedBuffer *bb)
{
    memzero(bb->buf, sizeof(bb->buf));
    spinlock_init(&bb->lock);

    bb->head = 0;
    bb->size = 0;

    CV_init(&bb->empty);
    CV_init(&bb->full);
}

void BB_enqueue(BoundedBuffer *bb, unsigned int val)
{
    spinlock_acquire(&bb->lock);

    while (is_BB_full(bb))
    {
        CV_wait(&bb->full, &bb->lock);
    }

    bb->buf[(bb->head + bb->size) % BUFFER_CAPACITY] = val;
    bb->size++;

    // unsigned int cpu_idx = get_pcpu_idx();
    DISI(KERN_DEBUG("\033[0;32mEnqueued %u: Used %u\\%d\033[0m\n", val, bb->size, BUFFER_CAPACITY));

    CV_signal(&bb->empty);
    spinlock_release(&bb->lock);
}

unsigned int BB_dequeue(BoundedBuffer *bb)
{
    spinlock_acquire(&bb->lock);

    while (is_BB_empty(bb))
    {
        CV_wait(&bb->empty, &bb->lock);
    }

    unsigned int val = bb->buf[bb->head];
    bb->head = (bb->head + 1) % BUFFER_CAPACITY;
    bb->size--;

    // unsigned int cpu_idx = get_pcpu_idx();
    DISI(KERN_DEBUG("\033[0;31mDequeued %u: Used %u\\%d\033[0m\n", val, bb->size, BUFFER_CAPACITY));

    CV_signal(&bb->full);
    spinlock_release(&bb->lock);
    return val;
}


// ----------- Helper Functions -----------

bool is_BB_empty(const BoundedBuffer *bb)
{
    return (bb->size == 0);
}

bool is_BB_full(const BoundedBuffer *bb)
{
    return (bb->size == BUFFER_CAPACITY);
}

#include <lib/x86.h>
#include <lib/thread.h>
#include <lib/spinlock.h>
#include <lib/debug.h>
#include <dev/lapic.h>
#include <pcpu/PCPUIntro/export.h>
#include <kern/thread/PTCBIntro/export.h>

#include "import.h"

static spinlock_t sched_lk;

unsigned int sched_ticks[NUM_CPUS];

void thread_init(unsigned int mbi_addr)
{
    unsigned int i;
    for (i = 0; i < NUM_CPUS; i++)
    {
        sched_ticks[i] = 0;
    }

    spinlock_init(&sched_lk);
    tqueue_init(mbi_addr);
    set_curid(0);
    tcb_set_state(0, TSTATE_RUN);
}

/**
 * Allocates a new child thread context, sets the state of the new child thread
 * to ready, and pushes it to the ready queue.
 * It returns the child thread id.
 */
unsigned int thread_spawn(void *entry, unsigned int id, unsigned int quota)
{
    unsigned int pid;

    spinlock_acquire(&sched_lk);

    pid = kctx_new(entry, id, quota);
    if (pid != NUM_IDS)
    {
        tcb_set_state(pid, TSTATE_READY);
        tqueue_enqueue(NUM_IDS, pid);
    }

    spinlock_release(&sched_lk);

    return pid;
}

/**
 * Yield to the next thread in the ready queue.
 * You should set the currently running thread state as ready,
 * and push it back to the ready queue.
 * Then set the state of the popped thread as running, set the
 * current thread id, and switch to the new kernel context.
 * Hint: If you are the only thread that is ready to run,
 * do you need to switch to yourself?
 */
void thread_yield(void)
{
    unsigned int old_cur_pid;
    unsigned int new_cur_pid;

    spinlock_acquire(&sched_lk);

    old_cur_pid = get_curid();
    tcb_set_state(old_cur_pid, TSTATE_READY);
    tqueue_enqueue(NUM_IDS, old_cur_pid);

    new_cur_pid = tqueue_dequeue(NUM_IDS);
    tcb_set_state(new_cur_pid, TSTATE_RUN);
    set_curid(new_cur_pid);

    if (old_cur_pid != new_cur_pid)
    {
        spinlock_release(&sched_lk);
        kctx_switch(old_cur_pid, new_cur_pid);
    }
    else
    {
        spinlock_release(&sched_lk);
    }
}

void sched_update(void)
{
    spinlock_acquire(&sched_lk);
    sched_ticks[get_pcpu_idx()] += 1000 / LAPIC_TIMER_INTR_FREQ;
    if (sched_ticks[get_pcpu_idx()] >= SCHED_SLICE)
    {
        sched_ticks[get_pcpu_idx()] = 0;
        spinlock_release(&sched_lk);
        thread_yield();
    }
    else
    {
        spinlock_release(&sched_lk);
    }
}

/**
 * Atomically release lock and sleep on chan.
 * Reacquires lock when awakened.
 * Recall this function is needed to allow
 * thread waiting for the file system block
 * resource release. Note, only one thread
 * should access the file system at a time
 * to prevent data races and ensure consistency
 * of shared on-disk structures.
 */
void thread_sleep(void *chan, spinlock_t *lk)
{
    KERN_DEBUG("thread_sleep: pid = %d, chan = %p\n", get_curid(), chan);

    // TODO: your local variables here.
    unsigned int old_cur_pid;
    unsigned int new_cur_pid;

    if (lk == 0)
        KERN_PANIC("sleep without lock");

    // TODO:
    // Must acquire sched_lk in order to change the current thread's state and
    // then switch. Once we hold sched_lk, we can be guaranteed that we won't
    // miss any wakeup (wakeup runs with sched_lk locked), so it's okay to
    // release lock.

    spinlock_acquire(&sched_lk);
    spinlock_release(lk);

    // TODO: Go to sleep.
    old_cur_pid = get_curid();
    new_cur_pid = tqueue_dequeue(NUM_IDS);
    // The queue is never empty as there is always inserted `idle`
    // process. NUM_IDS return would happen only if all threads are sleeping
    // right now or dead which would meaning we forgot to wake up threads
    // before sleeping, so in reality there should not happen this case.
    // We should just panic:
    KERN_ASSERT(new_cur_pid != NUM_IDS);

    set_curid(new_cur_pid);
    tcb_set_state(old_cur_pid, TSTATE_SLEEP);
    tcb_set_state(new_cur_pid, TSTATE_RUN);
    tcb_set_chan(old_cur_pid, chan);

    // TODO: Context switch.
    spinlock_release(&sched_lk);
    kctx_switch(old_cur_pid, new_cur_pid);
    spinlock_acquire(&sched_lk);

    // TODO: Tidy up.
    tcb_set_chan(old_cur_pid, (void *)0);

    // TODO: Reacquire original lock.
    spinlock_acquire(lk);
    spinlock_release(&sched_lk);
}

/**
 * Wake up all processes sleeping on chan.
 * Multiple threads may be waiting for different
 * non-conflicting file system operations, so we
 * need to wake all them up at once to be safe.
 */
void thread_wakeup(void *chan)
{
    // TODO
    unsigned int old_cur_pid;
    unsigned int new_cur_pid;

    spinlock_acquire(&sched_lk);
    for (new_cur_pid = 0; new_cur_pid < NUM_IDS; new_cur_pid++)
    {
        if (tcb_get_state(new_cur_pid) == TSTATE_SLEEP &&
            tcb_get_chan(new_cur_pid) == chan)
        {
            old_cur_pid = get_curid();
            KERN_DEBUG("thread_wakeup: caller pid= %d, calle pid=%d, chan=%p\n", old_cur_pid, new_cur_pid, chan);
            tcb_set_state(new_cur_pid, TSTATE_READY);
            tqueue_enqueue(NUM_IDS, new_cur_pid);
            tcb_set_chan(new_cur_pid, (void *)0);
        }
    }
    spinlock_release(&sched_lk);
}

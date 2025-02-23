#include <lib/x86.h>
#include <lib/thread.h>

#include "import.h"

void thread_init(unsigned int mbi_addr)
{
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
    // TODO
    unsigned int child_id = kctx_new(entry, id, quota);
    tcb_set_state(child_id, TSTATE_READY);
    tqueue_enqueue(child_id, id);
    return child_id;
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
    // TODO
    unsigned int cur = get_curid();
    tcb_set_state(cur, TSTATE_READY);
    tqueue_enqueue(NUM_IDS, cur);

    unsigned int next = tqueue_dequeue(NUM_IDS);

    // if the two threads are the same,
    // we just keep it running; to avoid
    // the context switch with itself
    if (next == cur) {
        tcb_set_state(cur, TSTATE_RUN);
        return;
    }

    tcb_set_state(next, TSTATE_RUN);
    set_curid(next);
    kctx_switch(cur, next);
}

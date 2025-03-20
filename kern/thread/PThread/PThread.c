#include <lib/x86.h>
#include <lib/thread.h>
#include <lib/debug.h>
#include <dev/lapic.h>
#include <lib/spinlock.h>
#include <thread/PTCBIntro/export.h>
#include <pcpu/PCPUIntro/export.h>

#include "import.h"

// IMPORTANT: Per-CPU locks (part1)
static spinlock_t ready_queue_lock[NUM_CPUS];

// @anton-mel: (part2)
static int elapsed_time[NUM_CPUS] = {0};
// @anton-mel: to verify part2, inclease this value
// let's keep it 10ms for the responsiveness.
#define LAPIC_MS_INTR 10000 / LAPIC_TIMER_INTR_FREQ

void thread_init(unsigned int mbi_addr)
{
    for (int i = 0; i < NUM_CPUS; i++)
    {
        // @anton-mel
        elapsed_time[i] = 0; // just to be safe...
        spinlock_init(&ready_queue_lock[i]);
    }

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
    // handle within lock...
    // unsigned int cpu_idx = get_pcpu_idx();

    spinlock_acquire(&ready_queue_lock[get_pcpu_idx()]);
    pid = kctx_new(entry, id, quota);

    if (pid != NUM_IDS)
    {
        tcb_set_cpu(pid, get_pcpu_idx());
        tcb_set_state(pid, TSTATE_READY);
        tqueue_enqueue(NUM_IDS + get_pcpu_idx(), pid);
    }
    spinlock_release(&ready_queue_lock[get_pcpu_idx()]);

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
    unsigned int new_cur_pid;
    unsigned int old_cur_pid;
    // same problem...
    // unsigned int old_cur_pid = get_curid();
    // unsigned int cpu_idx = get_pcpu_idx();

    // check here
    // @anton-mel: failed if held before ctx switch
    int is_already_aquired = spinlock_try_acquire(&ready_queue_lock[get_pcpu_idx()]);

    if (is_already_aquired == 1)
    {
        return;
    }

    old_cur_pid = get_curid();

    tcb_set_state(old_cur_pid, TSTATE_READY);
    tqueue_enqueue(NUM_IDS + get_pcpu_idx(), old_cur_pid);

    new_cur_pid = tqueue_dequeue(NUM_IDS + get_pcpu_idx());

    tcb_set_state(new_cur_pid, TSTATE_RUN);
    set_curid(new_cur_pid);

    spinlock_release(&ready_queue_lock[get_pcpu_idx()]);

    if (old_cur_pid != new_cur_pid)
    {
        // @anton-mel: release before the ctx switch
        kctx_switch(old_cur_pid, new_cur_pid);
    }
}

// @anton-mel: HELPER FUNCTIONS [PART 4]
// ------------------------------------------------ //
void thread_suspend(spinlock_t *lk, unsigned int prev_pid)
{
    unsigned int next_pid;
    spinlock_acquire(&ready_queue_lock[get_pcpu_idx()]);
    KERN_ASSERT(prev_pid == get_curid());

    next_pid = tqueue_dequeue(NUM_IDS + get_pcpu_idx());
    KERN_ASSERT(next_pid != NUM_IDS);

    spinlock_release(lk);

    tcb_set_state(prev_pid, TSTATE_SLEEP); // Suspend current thread
    tcb_set_state(next_pid, TSTATE_RUN);   // Set next thread to running
    set_curid(next_pid);

    spinlock_release(&ready_queue_lock[get_pcpu_idx()]);

    kctx_switch(prev_pid, next_pid);
}

void thread_ready(unsigned int pid)
{
    spinlock_acquire(&ready_queue_lock[tcb_get_cpu(pid)]);
    tcb_set_state(pid, TSTATE_READY);                // Set thread to ready state
    tqueue_enqueue(NUM_IDS + tcb_get_cpu(pid), pid); // Add thread to ready queue
    spinlock_release(&ready_queue_lock[tcb_get_cpu(pid)]);
}
// ------------------------------------------------ //

/**
 * This function keeps track of the elapsed time since the last thread switch
 * for each CPU. When the elapsed time reaches the defined scheduling slice
 * (SCHED_SLICE), it triggers a thread yield to allow another thread to run.
 * This mechanism enables preemptive multitasking by ensuring that no single
 * thread monopolizes the CPU indefinitely.
 */
void sched_update(void)
{
    int cpu_idx = get_pcpu_idx();
    elapsed_time[cpu_idx] += LAPIC_MS_INTR;
    if (elapsed_time[cpu_idx] >= SCHED_SLICE)
    {
        // KERN_DEBUG("[CPU %d] 8253 Programmable Interval Timer\n", cpu_idx);
        elapsed_time[cpu_idx] = 0;
        thread_yield();
    }
}

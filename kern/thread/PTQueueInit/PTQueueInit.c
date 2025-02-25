#include "lib/x86.h"

#include "import.h"

/**
 * Initializes all the thread queues with tqueue_init_at_id.
 */
void tqueue_init(unsigned int mbi_addr)
{
    // TODO: define your local variables here.

    tcb_init(mbi_addr);

    for (int chid = 0; chid < NUM_IDS + 1; chid++) {
        tqueue_init_at_id(chid);
    }
}

/**
 * Insert the TCB #pid into the tail of the thread queue #chid.
 * Recall that the doubly linked list is index based.
 * So you only need to insert the index.
 * Hint: there are multiple cases in this function.
 */
void tqueue_enqueue(unsigned int chid, unsigned int pid)
{
    // Assumes that next and prev on pid is currently pointing to NUM_IDS
    unsigned int q_tail = tqueue_get_tail(chid);

    if (q_tail == NUM_IDS) {
        tqueue_set_tail(chid, pid);
        tqueue_set_head(chid, pid);
    } else {
        tcb_set_next(q_tail, pid);
        tcb_set_prev(pid, q_tail);
        
        tqueue_set_tail(chid, pid);
    }
}

/**
 * Reverse action of tqueue_enqueue, i.e. pops a TCB from the head of the specified queue.
 * It returns the popped thread's id, or NUM_IDS if the queue is empty.
 * Hint: there are multiple cases in this function.
 */
unsigned int tqueue_dequeue(unsigned int chid)
{
    unsigned int q_head = tqueue_get_head(chid);

    if (q_head == NUM_IDS) {
        return NUM_IDS;
    }

    unsigned int next = tcb_get_next(q_head);
    if (next == NUM_IDS) { // if only one node in list
        tqueue_set_head(chid, NUM_IDS);
        tqueue_set_tail(chid, NUM_IDS);
    } else { // if more than one node in list
        tcb_set_prev(next, NUM_IDS);
        tqueue_set_head(chid, next);
    }

    tcb_set_next(q_head, NUM_IDS);
    tcb_set_prev(q_head, NUM_IDS);

    return q_head;
}

/**
 * Removes the TCB #pid from the queue #chid.
 * Hint: there are many cases in this function.
 */
void tqueue_remove(unsigned int chid, unsigned int pid)
{
    unsigned int next = tcb_get_next(pid);
    unsigned int prev = tcb_get_prev(pid);
    unsigned int q_head = tqueue_get_head(chid);

    if (q_head == NUM_IDS) {
        return;
    }

    if (next == NUM_IDS) {
        tqueue_set_tail(chid, prev);
    }

    if (prev == NUM_IDS) {
        tqueue_set_head(chid, next);
    }

    if (next != NUM_IDS) {
        tcb_set_prev(next, prev);
    }

    if (prev != NUM_IDS) {
        tcb_set_next(prev, next);
    }

    tcb_set_next(pid, NUM_IDS);
    tcb_set_prev(pid, NUM_IDS);
}

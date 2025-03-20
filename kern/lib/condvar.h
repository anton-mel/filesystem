// Bouded Buffer
//
//      Given book definition.

#ifndef _KERN_LIB_CONDVAR_H_
#define _KERN_LIB_CONDVAR_H_

#ifdef _KERN_

#include <lib/spinlock.h>
#include <lib/x86.h>

typedef struct
{
    unsigned int queue[NUM_IDS];
    unsigned int tail;
} CV;

typedef struct
{
#define BUFFER_CAPACITY 3
    unsigned int buf[BUFFER_CAPACITY];
    unsigned int head;
    unsigned int size;
    spinlock_t lock;
    CV empty;
    CV full;
} BoundedBuffer;

void CV_init(CV *cv);
void CV_wait(CV *cv, spinlock_t *lock);
void CV_signal(CV *cv);
void CV_broadcast(CV *cv);

void BB_init(BoundedBuffer *bb);
bool BB_is_empty(const BoundedBuffer *bb);
bool BB_is_full(const BoundedBuffer *bb);
void BB_enqueue(BoundedBuffer *bb, unsigned int val);
unsigned int BB_dequeue(BoundedBuffer *bb);

#endif /* _KERN_ */

#endif /* !_KERN_LIB_CONDVAR_H_ */

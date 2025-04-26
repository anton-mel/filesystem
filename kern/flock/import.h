#ifndef _KERN_FLOCK_IMPORT_H_
#define _KERN_FLOCK_IMPORT_H_

#ifdef _KERN_

void spinlock_init(spinlock_t *lk);
void spinlock_acquire(spinlock_t *lk);
void spinlock_release(spinlock_t *lk);
bool spinlock_holding(spinlock_t *lk);
int spinlock_try_acquire(spinlock_t *lk);

void CV_init(CV *cv); 
void CV_wait(CV *cv, spinlock_t *lock);
void CV_broadcast(CV *cv); 
void CV_signal(CV *cv);

#endif  /* _KERN_ */

#endif  /* !_KERN_FLOCK_IMPORT_H_ */

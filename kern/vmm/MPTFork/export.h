#ifndef _KERN_VMM_MPTFORK_H_
#define _KERN_VMM_MPTFORK_H_

#ifdef _KERN_

unsigned int proc_fork(void);
void copy_cow_page(unsigned int pid, unsigned int vaddr);

#endif  /* _KERN_ */

#endif  /* !_KERN_VMM_MPTFORK_H_ */

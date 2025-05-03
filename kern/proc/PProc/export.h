#ifndef _KERN_PROC_PPROC_H_
#define _KERN_PROC_PPROC_H_

#ifdef _KERN_

unsigned int proc_create(void *elf_addr, unsigned int quota);
void proc_start_user(void);

void proc_exit(unsigned int status);
int proc_wait(unsigned int pid, int *status);

#endif  /* _KERN_ */

#endif  /* !_KERN_PROC_PPROC_H_ */

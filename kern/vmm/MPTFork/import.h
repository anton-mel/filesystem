#ifndef _KERN_VMM_MPTFORK_H_
#define _KERN_VMM_MPTFORK_H_

#ifdef _KERN_

unsigned int get_curid(void);
void proc_start_user(void);
unsigned int thread_spawn(void *entry, unsigned int id,
                          unsigned int quota);
unsigned int unmap_page(unsigned int proc_index, unsigned int vaddr);
void rmv_ptbl_entry(unsigned int proc_index, unsigned int pde_index,
                    unsigned int pte_index);
void set_ptbl_entry(unsigned int proc_index, unsigned int pde_index,
                    unsigned int pte_index, unsigned int page_index,
                    unsigned int perm);
void set_pdir_entry(unsigned int proc_index, unsigned int pde_index,
                    unsigned int page_index);
unsigned int container_alloc(unsigned int id);
unsigned int container_get_usage(unsigned int id);
unsigned int container_get_quota(unsigned int id);
unsigned int map_page(unsigned int proc_index, unsigned int vaddr,
                      unsigned int page_index, unsigned int perm);
unsigned int alloc_mem_quota(unsigned int id, unsigned int quota);
void set_cr3(unsigned int **pdir);  // sets the CR3 register
unsigned int get_ptbl_entry(unsigned int proc_index, unsigned int pde_index,
                            unsigned int pte_index);

#endif  /* _KERN_ */

#endif  /* !_KERN_VMM_MPTFORK_H_ */

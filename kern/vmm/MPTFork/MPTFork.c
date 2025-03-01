/**
 * This file implements page copying for fork
 * and other helper function for page fault handling.
 */

#include <lib/x86.h>
#include <lib/string.h>
#include <lib/debug.h>
#include <lib/trap.h>
#include "import.h"

#define PDE_ADDR(x) (x >> 22)
#define PTE_ADDR(x) ((x >> 12) & 0x3ff)

#define PAGESIZE      4096
#define PDIRSIZE      (PAGESIZE * 1024)
#define VM_USERLO_PDE (VM_USERLO / PDIRSIZE)
#define VM_USERHI_PDE (VM_USERHI / PDIRSIZE)
#define COWPERM(pte)  ((pte) & (PTE_P | PTE_W | PTE_U | PTE_COW))

extern tf_t uctx_pool[NUM_IDS];
extern unsigned int *PDirPool[NUM_IDS][1024];
// Bonus Point: Have not verified the correctness.
// static unsigned int ref_count[NUM_IDS * 1024] = {0};

/**
 * Copy parent’s page directory and page tables to child process.
 * Mark shared pages as read-only + COW.
 */
void copy_pagetables(unsigned int parent_pid, unsigned int child_pid) {
    // [BUG TO FIXED] seems like copying can happen
    // for a non-valid parrent and child processes,
    // so we should handle carefully
    if (parent_pid == 0 || 
        child_pid == 0) {
        return;
    }

    // copy over the page table directory and setup the map
    for (unsigned int pde_index = VM_USERLO_PDE; pde_index < VM_USERHI_PDE; pde_index++) {
        unsigned int new_page = container_alloc(child_pid);

        if (new_page == 0) {
            // no more memory available
            // handle gracefully
            return;
        }

        // set the page directory entry for the child 
        // process, pointing it to the newly allocated page
        set_pdir_entry(child_pid, pde_index, new_page);

        // iterate through the page table entries (PTE) 
        // for this page directory entry (PDE)
        for (unsigned int pte_index = 0; pte_index < 1024; pte_index++) {
            unsigned int pbtl_entry = get_ptbl_entry(parent_pid, pde_index, pte_index);
            unsigned int page_index = pbtl_entry >> 12;
            unsigned int perms = COWPERM(pbtl_entry);

            if (perms & PTE_U) {
                if (perms & PTE_W) {
                    // Mark writable pages as COW
                    perms = (perms & ~PTE_W) | PTE_COW;
                }
                // set the same COW permissions in both tables
                set_ptbl_entry(child_pid, pde_index, pte_index, page_index, perms);
                set_ptbl_entry(parent_pid, pde_index, pte_index, page_index, perms);
            }
        }
    }
}


/**
 * Copy the page when a COW fault occurs.
 */
void copy_cow_page(unsigned int pid, unsigned int vaddr) {
    unsigned int pde_index = PDE_ADDR(vaddr);
    unsigned int pte_index = PTE_ADDR(vaddr);

    // Get the current page table entry (PTE) for the faulting address
    unsigned int pte_entry = get_ptbl_entry(pid, pde_index, pte_index);
    unsigned int page_index = pte_entry >> 12; // Physical page index
    unsigned int perm = pte_entry & ~0xFFF;    // Permissions (without the physical page index)

    // Allocate a new page for the process
    unsigned int new_page = container_alloc(pid);
    if (new_page == 0) {
        // Handle out-of-memory error
        return;
    }

    // copy from the faulting address to a new page allocation
    memcpy((void *) (new_page * PAGESIZE), (void *) (page_index * PAGESIZE), PAGESIZE);

    // remove the old page table entry (after, right?)
    rmv_ptbl_entry(pid, pde_index, pte_index);
    set_cr3(PDirPool[pid]);

    // map the new page with write permissions
    // clear the COW bit!
    unsigned int new_perm = (perm & ~PTE_COW) | PTE_W;
    set_ptbl_entry(pid, pde_index, pte_index, new_page, new_perm);

}


/**
 * proc_fork - Creates a child process by 
 * copying parent’s memory and user context.
 */
unsigned int proc_fork(void) {
    unsigned int pid, parent_pid;
    // parent process
    parent_pid = get_curid();
    
    // calculate the available quota for a child
    // for the child process (half of the remaining quota)
    // since the container does not handle this for us.
    unsigned int child_quota = (container_get_quota(parent_pid) - container_get_usage(parent_pid)) / 2;

    // child process
    unsigned int child_pid = thread_spawn((void *) proc_start_user, parent_pid, child_quota);

    if (child_pid != NUM_IDS) {
        // pagetable copying
        copy_pagetables(parent_pid, child_pid);
        // copy the parent’s CPU user context
        uctx_pool[child_pid] = uctx_pool[parent_pid];
        // standard return value for fork (child)
        uctx_pool[child_pid].regs.eax = 0;
    }

    return child_pid;
}


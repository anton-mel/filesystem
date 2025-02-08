#include <lib/x86.h>

#include "import.h"

/**
 * For each process from id 0 to NUM_IDS - 1,
 * set up the page directory entries so that the kernel portion of the map is
 * the identity map, and the rest of the page directories are unmapped.
 */
void pdir_init(unsigned int mbi_addr)
{
    unsigned int vaddr, proc_index, pde_index;

    // Initialize the identity page table
    idptbl_init(mbi_addr);

    for (proc_index = 0; proc_index < NUM_IDS; proc_index++) {
        for (pde_index = 0; pde_index < 1024; pde_index++) {
            vaddr = pde_index << 22;

            // Map kernel/system regions, unmap user space
            if (vaddr < 0x40000000 || vaddr >= 0xF0000000) {
                // Kernel/System memory: Identity map for all processes
                set_pdir_entry_identity(proc_index, pde_index);
            } else {
                // User space: Explicitly unmap the page directory entry
                rmv_pdir_entry(proc_index, pde_index);
            }
        }
    }
}


/**
 * Allocates a page (with container_alloc) for the page table,
 * and registers it in the page directory for the given virtual address,
 * and clears (set to 0) all page table entries for this newly mapped page table.
 * It returns the page index of the newly allocated physical page.
 * In the case when there's no physical page available, it returns 0.
 */
unsigned int alloc_ptbl(unsigned int proc_index, unsigned int vaddr)
{
    unsigned int page_index = container_alloc(proc_index);
    if (page_index == 0) {
        return 0; // no physical page available
    }
    
    // create a new page table
    set_pdir_entry_by_va(proc_index, vaddr, (page_index << 12) | PTE_P | PTE_W);

    // default out the page table entries
    unsigned int pde_index = vaddr >> 22;
    for (unsigned int pte_index = 0; pte_index < 1024; pte_index++) {
        rmv_ptbl_entry(proc_index, pde_index, pte_index);
    }
    
    return page_index;
}

// Reverse operation of alloc_ptbl.
// Removes corresponding the page directory entry,
// and frees the page for the page table entries (with container_free).
void free_ptbl(unsigned int proc_index, unsigned int vaddr)
{
    // TODO
    // first, get the pdir entry to get the PP index
    unsigned int pdir_entry = get_pdir_entry_by_va(proc_index, vaddr);
    
    if (!(pdir_entry & PTE_P)) {
        // make sure it even exists
        return;
    }

    unsigned int page_index = pdir_entry >> 12; // PPN
    rmv_pdir_entry_by_va(proc_index, vaddr); // free PDir entry
    container_free(proc_index, page_index); // free PhyPage
}


#include <lib/x86.h>

#include "import.h"

/**
 * For each process from id 0 to NUM_IDS - 1,
 * set up the page directory entries so that the kernel portion of the map is
 * the identity map, and the rest of the page directories are unmapped.
 */
void pdir_init(unsigned int mbi_addr)
{
    // TODO
    // LINEAR ADDR => | 31-22 | 21-12 | 11-0 |
    unsigned int vaddr, proc_index, pde_index;
    // Initialize the identity page table
    idptbl_init(mbi_addr);

    // for each process and its pagetable size of PAGE
    for (proc_index = 0; proc_index < NUM_IDS; proc_index++) {
        // 4KB (PAGE) => 1024 unsigned int addresses or 2^10 combinations => 10 bits
        for (pde_index = 0; pde_index < 1024; pde_index++) {
            vaddr = pde_index << 22; // simulate VA
            // map kernel/system regions & unmap user space
            if (vaddr < 0x40000000 || vaddr >= 0xF0000000) {
                set_pdir_entry_identity(proc_index, pde_index);
            } else {
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
    // TODO
    // LINEAR ADDR => | 31-22 | 21-12 | 11-0 |
    // first, allocate the PM for the pagetable (1 PAGE)
    unsigned int page_index = container_alloc(proc_index);
    // PP unavailable/ exceeded the quota
    if (page_index == 0) {
        return 0; 
    }

    // register the map from [vaddr] to PP # [page_index] in the P_DIR
    // avoid global permissions, since non-kernel processes should not share state
    set_pdir_entry_by_va(proc_index, vaddr, (page_index << 12) | PTE_P | PTE_W);

    // default out the page table entries
    unsigned int pde_index = vaddr >> 22;
    // since PT is size of PAGE => 2^10 entries => 1024 unsigned int addrs
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
    // PAGE DIRECTORY ENTRY => | 31-12 | 11-0 |
    // first, get the P_DIR entry to get the PP index
    unsigned int pdir_entry = get_pdir_entry_by_va(proc_index, vaddr);
    
    if (!(pdir_entry & PTE_P)) {
        return;
    }

    // given 4GB PM space, we get 2^20 bits addr
    unsigned int page_index = pdir_entry >> 12;

    rmv_pdir_entry_by_va(proc_index, vaddr); // free PDir entry
    container_free(proc_index, page_index); // free PhyPage
}

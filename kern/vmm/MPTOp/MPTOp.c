#include <lib/x86.h>

#include "import.h"

/**
 * Returns the page table entry corresponding to the virtual address,
 * according to the page structure of process # [proc_index].
 * Returns 0 if the mapping does not exist.
 */
unsigned int get_ptbl_entry_by_va(unsigned int proc_index, unsigned int vaddr)
{
    // TODO
    // Considering Little Endian.
    unsigned int pde = vaddr >> 22;
    unsigned int pte = (vaddr >> 12) & 0x3FF;
    return get_ptbl_entry(proc_index, pde, pte);
}

// Returns the page directory entry corresponding to the given virtual address.
unsigned int get_pdir_entry_by_va(unsigned int proc_index, unsigned int vaddr)
{
    // TODO
    unsigned int pde = vaddr >> 22;
    return get_pdir_entry(proc_index, pde);
}

// Removes the page table entry for the given virtual address.
void rmv_ptbl_entry_by_va(unsigned int proc_index, unsigned int vaddr)
{
    // TODO
    unsigned int pde = vaddr >> 22;
    unsigned int pte = (vaddr >> 12) & 0x3FF;
    rmv_ptbl_entry(proc_index, pde, pte);
}

// Removes the page directory entry for the given virtual address.
void rmv_pdir_entry_by_va(unsigned int proc_index, unsigned int vaddr)
{
    // TODO
    unsigned int pde = vaddr >> 22;
    rmv_pdir_entry(proc_index, pde);
}

// Maps the virtual address [vaddr] to the physical page # [page_index] with permission [perm].
// You do not need to worry about the page directory entry. just map the page table entry.
void set_ptbl_entry_by_va(unsigned int proc_index, unsigned int vaddr,
                          unsigned int page_index, unsigned int perm)
{
    // TODO
    unsigned int pde = vaddr >> 22;
    unsigned int pte = (vaddr >> 12) & 0x3FF;
    set_ptbl_entry(proc_index, pde, pte, page_index, perm);
}

// Registers the mapping from [vaddr] to physical page # [page_index] in the page directory.
void set_pdir_entry_by_va(unsigned int proc_index, unsigned int vaddr,
                          unsigned int page_index)
{
    // TODO
    unsigned int pde = vaddr >> 22;
    set_pdir_entry(proc_index, pde, page_index);
}

// Initializes the identity page table.
// The permission for the kernel memory should be PTE_P, PTE_W, and PTE_G,
// While the permission for the rest should be PTE_P and PTE_W.
void idptbl_init(unsigned int mbi_addr)
{
    unsigned int pde_index, pte_index; 
    unsigned int addr, perm;

    container_init(mbi_addr);

    for (pde_index = 0; pde_index < 1024; pde_index++) {
        for (pte_index = 0; pte_index < 1024; pte_index++) {
            addr = (pde_index << 22) | (pte_index << 12);
            // elf.c: VM_USERLO = 0x40000000 & VM_USERHI = 0xF0000000
            if (addr < 0x40000000 || addr >= 0xF0000000) {
                perm = PTE_P | PTE_W | PTE_G;
            } else {
                perm = PTE_P | PTE_W;
            }
            set_ptbl_entry_identity(pde_index, pte_index, perm);
        }
    }
}

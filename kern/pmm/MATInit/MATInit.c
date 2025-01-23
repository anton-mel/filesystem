#include <lib/debug.h>
#include "import.h"

#define PAGESIZE     4096
#define VM_USERLO    0x40000000
#define VM_USERHI    0xF0000000
#define VM_USERLO_PI (VM_USERLO / PAGESIZE)
#define VM_USERHI_PI (VM_USERHI / PAGESIZE)

/**
 * The initialization function for the allocation table AT.
 * It contains two major parts:
 * 1. Calculate the actual physical memory of the machine, and sets the number
 *    of physical pages (NUM_PAGES).
 * 2. Initializes the physical allocation table (AT) implemented in the MATIntro layer
 *    based on the information available in the physical memory map table.
 *    Review import.h in the current directory for the list of available
 *    getter and setter functions.
 */
void pmem_init(unsigned int mbi_addr)
{
    /// @anton-mel
    // TLDR. Physical Memory Map table provides a description of the physical memory 
    // layout of a machine, as detected by the bootloader (or BIOS). It tells the 
    // kernel which regions of memory are available for use, which are reserved, 
    // and which are allocated for special purposes (e.g., BIOS data, devices, etc.).
    // This is NOT related to Virtual Memory, but still part of the Memory Management.

    unsigned int nps = 0;                   // Total number of physical pages
    unsigned int highest_addr = 0;          // Highest address available for PM
    unsigned int num_ranges = get_size();   // Number of memory map entries

    // Calls the lower layer initialization primitive e.g. device drivers or interrupts.
    // The parameter mbi_addr should not be used in the further code.
    devinit(mbi_addr);

    /**
     * Calculate the total number of physical pages provided by the hardware and
     * store it into the local variable nps.
     * Hint: Think of it as the highest address in the ranges of the memory map table,
     *       divided by the page size.
     */

    // Find the highest address available for physical memory
    for (unsigned int i = 0; i < num_ranges; i++) {
        if (is_usable(i)) { // Only consider usable ranges, otherwise cannot allocate
            unsigned int range_end = get_mms(i) + get_mml(i);
            if (range_end > highest_addr) {
                highest_addr = range_end;
            }
        }
    }

    nps = highest_addr / PAGESIZE;
    set_nps(nps);   // Setting the value computed above to NUM_PAGES.

    /**
     * Initialization of the physical allocation table (AT).
     *
     * In CertiKOS, all addresses < VM_USERLO or >= VM_USERHI are reserved by the kernel.
     * That corresponds to the physical pages from 0 to VM_USERLO_PI - 1,
     * and from VM_USERHI_PI to NUM_PAGES - 1.
     * The rest of the pages that correspond to addresses [VM_USERLO, VM_USERHI)
     * can be used freely ONLY IF the entire page falls into one of the ranges in
     * the memory map table with the permission marked as usable.
     *
     * Hint:
     * 1. You have to initialize AT for all the page indices from 0 to NPS - 1.
     * 2. For the pages that are reserved by the kernel, simply set its permission to 1.
     *    Recall that the setter at_set_perm also marks the page as unallocated.
     *    Thus, you don't have to call another function to set the allocation flag.
     * 3. For the rest of the pages, explore the memory map table to set its permission
     *    accordingly. The permission should be set to 2 only if there is a range
     *    containing the entire page that is marked as available in the memory map table.
     *    Otherwise, it should be set to 0. Note that the ranges in the memory map are
     *    not aligned by pages, so it may be possible that for some pages, only some of
     *    the addresses are in a usable range. Currently, we do not utilize partial pages,
     *    so in that case, you should consider those pages as unavailable.
     */
    
    for (unsigned int i = 0; i < nps; i++) {
        if (i < VM_USERLO_PI || i >= VM_USERHI_PI) {
            // Kernel reserved pages
            at_set_perm(i, 1);
        } else {

            /// @anton-mel
            // Physical Memory Layout
            // ---------------------------------------------------------
            // | Reserved | Usable Range 1 | Reserved | Usable Range 2 |
            // ---------------------------------------------------------
            //         ^                ^          ^                ^
            //     range_start         range_end    range_start     range_end
            //
            // Page to Check
            // -----------------
            // |    Page i     |
            // -----------------
            // ^                ^
            // page_start      page_end
            //
            // Condition:
            // - If `page_start >= range_start` AND `page_end <= range_end`,
            // then the page is fully within the usable range.

            int is_page_usable = 0;
            for (unsigned int j = 0; j < num_ranges; j++) {
                if (is_usable(j)) { // For each memory range to place a table
                    unsigned int range_start = get_mms(j);
                    unsigned int range_end = range_start + get_mml(j);
                    unsigned int page_start = i * PAGESIZE;
                    unsigned int page_end = page_start + PAGESIZE;

                    // Check if the entire page falls within the usable range
                    if (page_start >= range_start && page_end <= range_end) {
                        is_page_usable = 1;
                        break; // Success
                    }
                }
            }

            if (is_page_usable) {
                at_set_perm(i, 2); // Normal page
            } else {
                at_set_perm(i, 0); // Unusable page
            }
        }
    }
}

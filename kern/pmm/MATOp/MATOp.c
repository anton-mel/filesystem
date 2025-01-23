#include <lib/debug.h>
#include "import.h"

#define PAGESIZE     4096
#define VM_USERLO    0x40000000
#define VM_USERHI    0xF0000000
#define VM_USERLO_PI (VM_USERLO / PAGESIZE)
#define VM_USERHI_PI (VM_USERHI / PAGESIZE)

// Memoization: Remember the last allocated page
// since stack is growing downward consequently.
static unsigned int memoized_index = VM_USERLO_PI;

/**
 * Allocate a physical page.
 *
 * 1. First, implement a naive page allocator that scans the allocation table (AT)
 *    using the functions defined in import.h to find the first unallocated page
 *    with normal permissions.
 *    (Q: Do you have to scan the allocation table from index 0? Recall how you have
 *    initialized the table in pmem_init.)
 *    Then mark the page as allocated in the allocation table and return the page
 *    index of the page found. In the case when there is no available page found,
 *    return 0.
 * 2. Optimize the code using memoization so that you do not have to
 *    scan the allocation table from scratch every time.
 */
unsigned int palloc()
{
    unsigned int nps = get_nps();
    for (unsigned int i = memoized_index; i < nps; i++) {
        if (at_is_norm(i) && !at_is_allocated(i)) {
            at_set_allocated(i, 1);
            memoized_index = i + 1;
            return i;
        }
    }

    // Optional: Wrap around if no page was found in the current search
    // since the next function can potentially break consequent order
    for (unsigned int i = VM_USERLO_PI; i < memoized_index; i++) {
        if (at_is_norm(i) && !at_is_allocated(i)) {
            at_set_allocated(i, 1);
            memoized_index = i + 1;
            return i;
        }
    }

    return 0;
}

/**
 * Free a physical page.
 *
 * This function marks the page with given index as unallocated
 * in the allocation table.
 *
 * Hint: Simple.
 */
void pfree(unsigned int pfree_index)
{
    at_set_allocated(pfree_index, 0);
    // Optional: Reset memoized index to 
    // avoid skipping freed pages?
    if (pfree_index < memoized_index) {
        memoized_index = pfree_index;
    }
}

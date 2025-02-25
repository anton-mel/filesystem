#include <lib/trap.h>
#include <lib/x86.h>

#include "import.h"

extern tf_t uctx_pool[NUM_IDS];

// copied over from trap.h:
// uctx_pool[curid].regs format
// typedef struct pushregs {
//     uint32_t edi;
//     uint32_t esi;
//     uint32_t ebp;
//     uint32_t oesp;  /* Useless */
//     uint32_t ebx;
//     uint32_t edx;
//     uint32_t ecx;
//     uint32_t eax;
// } pushregs;

// !NOTE: The order is defined as per the spec.
// The system call number will go in %eax, and the 
// arguments (up to five of them) will go in %ebx, 
// %ecx, %edx, %esi, and %edi, respectively.

/**
 * Retrieves the system call arguments from uctx_pool that get
 * passed in from the current running process' system call.
 */
unsigned int syscall_get_arg1(void)
{
    // TODO
    unsigned int curid = get_curid();
    return uctx_pool[curid].regs.eax;
}

unsigned int syscall_get_arg2(void)
{
    // TODO
    unsigned int curid = get_curid();
    return uctx_pool[curid].regs.ebx;
}

unsigned int syscall_get_arg3(void)
{
    // TODO
    unsigned int curid = get_curid();
    return uctx_pool[curid].regs.ecx;
}

unsigned int syscall_get_arg4(void)
{
    // TODO
    unsigned int curid = get_curid();
    return uctx_pool[curid].regs.edx;
}

unsigned int syscall_get_arg5(void)
{
    // TODO
    unsigned int curid = get_curid();
    return uctx_pool[curid].regs.esi;
}

unsigned int syscall_get_arg6(void)
{
    // TODO
    unsigned int curid = get_curid();
    return uctx_pool[curid].regs.edi;
}

/**
 * Sets the error number in uctx_pool that gets passed
 * to the current running process when we return to it.
 */
void syscall_set_errno(unsigned int errno)
{
    // TODO
    // !NOTE: Following the spec documentation.
    // A system call always returns with an error number 
    // via register EAX. All valid error numbers are 
    // listed in __error_nr defined in kern/lib/syscall.h.
    unsigned int curid = get_curid();
    uctx_pool[curid].regs.eax = errno;
    // E_SUCC indicates success (no errors).
}

// !NOTE: Following the order, do the opposite here.
// A system call can return at most 5 32-bit values 
// via registers EBX, ECX, EDX, ESI and EDI. When the 
// trap happens, we first save the corresponding trap 
// frame (the register values of the user process) into 
// memory (uctx_pool), and we restores the register 
// values based on the saved ones.

/**
 * Sets the return values in uctx_pool that get passed
 * to the current running process when we return to it.
 */
void syscall_set_retval1(unsigned int retval)
{
    // TODO
    unsigned int curid = get_curid();
    uctx_pool[curid].regs.ebx = retval;
}

void syscall_set_retval2(unsigned int retval)
{
    // TODO
    unsigned int curid = get_curid();
    uctx_pool[curid].regs.ecx = retval;
}

void syscall_set_retval3(unsigned int retval)
{
    // TODO
    unsigned int curid = get_curid();
    uctx_pool[curid].regs.edx = retval;
}

void syscall_set_retval4(unsigned int retval)
{
    // TODO
    unsigned int curid = get_curid();
    uctx_pool[curid].regs.esi = retval;
}

void syscall_set_retval5(unsigned int retval)
{
    // TODO
    unsigned int curid = get_curid();
    uctx_pool[curid].regs.edi = retval;
}

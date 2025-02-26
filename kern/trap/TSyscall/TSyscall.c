#include <lib/debug.h>
#include <lib/pmap.h>
#include <lib/types.h>
#include <lib/x86.h>
#include <lib/trap.h>
#include <lib/syscall.h>

#include "import.h"

static char sys_buf[NUM_IDS][PAGESIZE];

/**
 * Copies a string from user into buffer and prints it to the screen.
 * This is called by the user level "printf" library as a system call.
 */
void sys_puts(void)
{
    // @anton-mel: this function is needed for the 
    // VGA buffer handling over the syscall.
    unsigned int cur_pid;
    unsigned int str_uva, str_len;
    unsigned int remain, cur_pos, nbytes;

    cur_pid = get_curid();
    str_uva = syscall_get_arg2();
    str_len = syscall_get_arg3();

    if (!(VM_USERLO <= str_uva && str_uva + str_len <= VM_USERHI)) {
        syscall_set_errno(E_INVAL_ADDR);
        return;
    }

    remain = str_len;
    cur_pos = str_uva;

    while (remain) {
        if (remain < PAGESIZE - 1)
            nbytes = remain;
        else
            nbytes = PAGESIZE - 1;

        if (pt_copyin(cur_pid, cur_pos, sys_buf[cur_pid], nbytes) != nbytes) {
            syscall_set_errno(E_MEM);
            return;
        }

        sys_buf[cur_pid][nbytes] = '\0';
        KERN_INFO("%s", sys_buf[cur_pid]);

        remain -= nbytes;
        cur_pos += nbytes;
    }

    syscall_set_errno(E_SUCC);
}

extern uint8_t _binary___obj_user_pingpong_ping_start[];
extern uint8_t _binary___obj_user_pingpong_pong_start[];
extern uint8_t _binary___obj_user_pingpong_ding_start[];
extern uint8_t _binary___obj_user_fork_test_fork_test_start[];


// !NOTE: (defined in user/include/syscall.h)
// static gcc_inline pid_t sys_spawn(unsigned int elf_id, unsigned int quota)
// {
//     int errno;
//     pid_t pid;
//
//     asm volatile ("int %2"                    // IDT syscall handler #2
//                   : "=a" (errno), "`=b" (pid) // operand %0: output eax to errno + output from EBX to pid
//                   : "i" (T_SYSCALL),          // operand %1: immediate constant for the interrupt vector
//                     "a" (SYS_spawn),          // operand %2: placed in EAX before the call
//                     "b" (elf_id),             // operand %3: placed in EBX before the call
//                     "c" (quota)               // operand %4: placed in ECX before the call
//                   : "cc", "memory");          // "cc" means modified + "memory" means may have been altered
//
//     return errno ? -1 : pid;
// }

/**
 * Spawns a new child process.
 * The user level library function sys_spawn (defined in user/include/syscall.h)
 * takes two arguments [elf_id] and [quota], and returns the new child process id
 * or NUM_IDS (as failure), with appropriate error number.
 * Currently, we have three user processes defined in user/pingpong/ directory,
 * ping, pong, and ding.
 * The linker ELF addresses for those compiled binaries are defined above.
 * Since we do not yet have a file system implemented in mCertiKOS,
 * we statically load the ELF binaries into the memory based on the
 * first parameter [elf_id].
 * For example, ping, pong, ding and fork_test correspond to the elf_ids
 * 1, 2, 3, and 4, respectively.
 * If the parameter [elf_id] is none of these, then it should return
 * NUM_IDS with the error number E_INVAL_PID. The same error case apply
 * when the proc_create fails.
 * Otherwise, you should mark it as successful, and return the new child process id.
 */
void sys_spawn(void)
{
    // TODO
    // So to implement this function, first we looked at the gcc inline assembly 
    // provided as a header to the userspace. Thus, we follow the calling conventions
    // predefined and able to correctly extract the registers values (look above struct).
    // Given syscall predefined order for getters: EAX, EBX, ECX, EDX, ESI and EDI:

    unsigned int elf_id = syscall_get_arg2(); // elf_id is passed in EBX
    unsigned int quota = syscall_get_arg3(); // quota is passed in ECX
    unsigned int child_pid;
    void *elf_addr = NULL;

    // !NOTE: objects for the ping/pong/ding is imported above.
    // Map elf_id to the corresponding ELF binary since there 
    // is no filesystem, so we have to do it manually.

    switch (elf_id) {
        case 1:
            elf_addr = _binary___obj_user_pingpong_ping_start;
            break;
        case 2:
            elf_addr = _binary___obj_user_pingpong_pong_start;
            break;
        case 3:
            elf_addr = _binary___obj_user_pingpong_ding_start;
            break;
        case 4:
            elf_addr = _binary___obj_user_fork_test_fork_test_start;
            break;
        default:
            // failure
            syscall_set_errno(E_INVAL_PID);
            syscall_set_retval1(NUM_IDS);
            return;
    }

    // Create the new process
    // !NOTE: syscall_set_retval1 fn sets the ret values in uctx_pool
    // that get passed to the current running prc when returned.

    child_pid = proc_create(elf_addr, quota);
    if (child_pid == NUM_IDS) {
        // failure
        syscall_set_errno(E_INVAL_PID);
        syscall_set_retval1(NUM_IDS);
    } else {
        // success
        syscall_set_errno(E_SUCC);
        syscall_set_retval1(child_pid);
    }
}


// !NOTE: (defined in user/include/syscall.h)
// static gcc_inline void sys_yield(void)
// {
//     asm volatile ("int %0"            // IDT syscall handler #0
//                   :: "i" (T_SYSCALL), // operand %0: immediate constant for the interrupt vector
//                      "a" (SYS_yield)  // operand %1: place SYS_yield into EAX before the call
//                   : "cc", "memory");  // "cc" means modified + "memory" means may have been altered
// }

/**
 * Yields to another thread/process.
 * The user level library function sys_yield (defined in user/include/syscall.h)
 * does not take any argument and does not have any return values.
 * Do not forget to set the error number as E_SUCC.
 */
void sys_yield(void)
{
    // TODO
    thread_yield();
    syscall_set_errno(E_SUCC);
}


// !NOTE: (defined in user/include/syscall.h)
// static gcc_inline pid_t sys_fork(void)
// {
//     int errno;
//     pid_t pid;

//     asm volatile ("int %2"                   // IDT syscall handler #2
//                   : "=a" (errno), "=b" (pid) // operand %0: output eax to errno + output from EBX to pid
//                   : "i" (T_SYSCALL),         // operand %1: immediate constant for the interrupt vector
//                     "a" (SYS_fork)           // operand %1: immediate constant for the interrupt vector
//                   : "cc", "memory");         // "cc" means modified + "memory" means may have been altered

//     return pid;
// }

// Your implementation of fork
void sys_fork(void)
{
    unsigned int cur_pid = get_curid();
    unsigned int child_pid;

    // first, define the child given the 
    // parent current process (elf copied over)
    child_pid = proc_create((void *)uctx_pool[cur_pid].regs.eip, 0);
    if (child_pid == NUM_IDS) {
        // failure
        syscall_set_errno(E_INVAL_PID);
        syscall_set_retval1(NUM_IDS);
    } else {
        // Success
        syscall_set_errno(E_SUCC);
        syscall_set_retval1(child_pid);
    }
}

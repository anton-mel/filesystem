#include <lib/elf.h>
#include <lib/debug.h>
#include <lib/gcc.h>
#include <lib/spinlock.h>
#include <lib/seg.h>
#include <lib/trap.h>
#include <lib/x86.h>
#include <pcpu/PCPUIntro/export.h>

#include "import.h"

extern tf_t uctx_pool[NUM_IDS];

extern unsigned int last_active[NUM_CPUS];

static bool exited[NUM_IDS];
static int return_value[NUM_IDS];
static spinlock_t process_lks[NUM_IDS];

void log_init();

void proc_start_user(void)
{
    unsigned int cur_pid = get_curid();
    unsigned int cpu_idx = get_pcpu_idx();

    static int started = FALSE;

    if (get_curid() != 1 && started == FALSE) {
        started = TRUE;
        log_init();
    }

    kstack_switch(cur_pid);
    set_pdir_base(cur_pid);
    last_active[cpu_idx] = cur_pid;

    trap_return((void *) &uctx_pool[cur_pid]);
}

unsigned int proc_create(void *elf_addr, unsigned int quota)
{
    unsigned int pid, id;

    id = get_curid();
    pid = thread_spawn((void *) proc_start_user, id, quota);
    exited[pid] = FALSE;
    return_value[pid] = 0;

    // KERN_INFO("proc_create: pid %d, id %d\n", pid, id);
    spinlock_init(&process_lks[pid]);

    if (pid != NUM_IDS) {
        elf_load(elf_addr, pid);

        uctx_pool[pid].es = CPU_GDT_UDATA | 3;
        uctx_pool[pid].ds = CPU_GDT_UDATA | 3;
        uctx_pool[pid].cs = CPU_GDT_UCODE | 3;
        uctx_pool[pid].ss = CPU_GDT_UDATA | 3;
        uctx_pool[pid].esp = VM_USERHI;
        uctx_pool[pid].eflags = FL_IF;
        uctx_pool[pid].eip = elf_entry(elf_addr);

        seg_init_proc(get_pcpu_idx(), pid);
    }

    return pid;
}

void proc_exit(unsigned int status)
{
    unsigned int cur_pid = get_curid();
    // KERN_INFO("proc_exit: pid %d, status %d\n", cur_pid, status);
    spinlock_acquire(&process_lks[cur_pid]);
    exited[cur_pid] = TRUE;
    return_value[cur_pid] = status;
    thread_wakeup(&return_value[cur_pid]);
    spinlock_release(&process_lks[cur_pid]);
}

int proc_wait(unsigned int pid, int *status)
{
    unsigned int cur_pid = get_curid();
    // KERN_INFO("proc_wait: cur_pid %d, pid %d\n", cur_pid, pid);

    if (pid >= NUM_IDS || pid == cur_pid) {
        return -1;
    }

    spinlock_acquire(&process_lks[pid]);
    while (exited[pid] == FALSE) {
        thread_sleep(&return_value[pid], &process_lks[pid]);
    }
    *status = return_value[pid];
    spinlock_release(&process_lks[pid]);

    return 0;
}
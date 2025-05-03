#include <proc.h>
#include <syscall.h>
#include <types.h>

pid_t spawn(uintptr_t exec, unsigned int quota)
{
    return sys_spawn(exec, quota);
}

void yield(void)
{
    sys_yield();
}

void produce(int val)
{
    sys_produce(val);
}

int consume(void)
{
    return sys_consume();
}

int wait(int pid, int *statusp)
{
    return sys_wait(pid, statusp);
}
#include <lib/debug.h>
#include <lib/gcc.h>
#include <lib/stdarg.h>
#include <lib/x86.h>

#include <lib/types.h>
#include <lib/spinlock.h>
#include <lib/reentrant_lock.h>

// @anton-mel: fine-grained lock
// static spinlock_t debug_lock;
// on debug_panic, should recover.
// Plus, double lock with vdprintf.
static reentrantlock debug_lock;

void debug_init(void)
{
    // @anton-mel: init local spinlock
    // spinlock_init(&debug_lock);
    reentrantlock_init(&debug_lock);
}

extern int vdprintf(const char *fmt, va_list ap);

void debug_info(const char *fmt, ...)
{
#ifdef DEBUG_MSG
    // @anton-mel: lock VA write
reentrantlock_acquire(&debug_lock);
    va_list ap;
    va_start(ap, fmt);
    vdprintf(fmt, ap);
    va_end(ap);
reentrantlock_release(&debug_lock);
#endif
}

void lockdeb(void)
{
    // go down recursive
    reentrantlock_acquire(&debug_lock);
}

void unlockdeb(void)
{
    // go down recursive
    reentrantlock_release(&debug_lock);
}

#ifdef DEBUG_MSG

void debug_normal(const char *file, int line, const char *fmt, ...)
{
// @anton-mel: lock VGA & VA write
reentrantlock_acquire(&debug_lock);
    dprintf("[D] %s:%d: ", file, line);
    va_list ap;
    va_start(ap, fmt);
    vdprintf(fmt, ap);
    va_end(ap);
reentrantlock_release(&debug_lock);
}

#define DEBUG_TRACEFRAMES 10

static void debug_trace(uintptr_t ebp, uintptr_t *eips)
{
    int i;
    uintptr_t *frame = (uintptr_t *)ebp;

    for (i = 0; i < DEBUG_TRACEFRAMES && frame; i++)
    {
        eips[i] = frame[1];            /* saved %eip */
        frame = (uintptr_t *)frame[0]; /* saved %ebp */
    }
    for (; i < DEBUG_TRACEFRAMES; i++)
        eips[i] = 0;
}

gcc_noinline void debug_panic(const char *file, int line, const char *fmt, ...)
{
    int i;
    uintptr_t eips[DEBUG_TRACEFRAMES];
    va_list ap;

// @anton-mel: lock VGA & VA write
reentrantlock_acquire(&debug_lock);
    dprintf("[P] %s:%d: ", file, line);

    va_start(ap, fmt);
    vdprintf(fmt, ap);
    va_end(ap);

debug_trace(read_ebp(), eips);
    for (i = 0; i < DEBUG_TRACEFRAMES && eips[i] != 0; i++)
        dprintf("\tfrom 0x%08x\n", eips[i]);

    dprintf("Kernel Panic !!!\n");

    // @anton-mel: Note, I assume this is impossible to recover from the halt(),
    // so the lock is unreleased. Best practice is to dissable interrupts and
    // keep it released. Thus, halt() will stop execution.
    // EDIT: just use reentrant lock!
    reentrantlock_release(&debug_lock);
    halt();
}

void debug_warn(const char *file, int line, const char *fmt, ...)
{
// @anton-mel: lock VGA & VA write
reentrantlock_acquire(&debug_lock);
    dprintf("[W] %s:%d: ", file, line);
    va_list ap;
    va_start(ap, fmt);
    vdprintf(fmt, ap);
    va_end(ap);
reentrantlock_release(&debug_lock);
}

#endif /* DEBUG_MSG */

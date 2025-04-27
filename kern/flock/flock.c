#include "flock.h"
#include "import.h"

/* -------- Global stand-in for “per-inode” context ------------------ */
static file_lock_context_t g_ctx;   /* zero-filled by the BSS loader   */
static bool                g_ready = false;   /* lazy-init flag        */

/* one-shot init helper */
static inline void
ctx_init_once(void)
{
    if (!g_ready) {
        spinlock_init(&g_ctx.lock);
        g_ctx.granted = NULL;
        g_ctx.blocked = NULL;
        g_ready = true;
    }
}

/* true iff “a” conflicts with “b” (whole-file semantics) */
static bool
lock_conflict(const file_lock_t *a, const file_lock_t *b)
{
    if (a->fl_type == LOCK_UN || b->fl_type == LOCK_UN)
        return false;

    /* shared ↔ shared never conflicts */
    if (a->fl_type == LOCK_SH && b->fl_type == LOCK_SH)
        return false;

    /* same owner upgrading from SH→EX is allowed (POSIX permits) */
    if (a->fl_owner == b->fl_owner)
        return false;

    return true;        /* everything else conflicts */
}

/* move req from blocked → granted and wake anyone sleeping on it */
static void
grant_lock(file_lock_context_t *ctx, file_lock_t *req)
{
    /* unlink from blocked list (singly-linked) */
    file_lock_t **p = &ctx->blocked;
    while (*p && *p != req) p = &(*p)->next_blocked;
    if (*p) *p = req->next_blocked;

    /* push on granted list */
    req->next_granted   = ctx->granted;
    ctx->granted        = req;
    req->next_blocked   = NULL;

    CV_broadcast(&req->fl_wait);      /* wake the lucky thread */
}

/*
 * flock_op() – handle LOCK_SH / LOCK_EX / LOCK_UN / LOCK_NB
 * Return 0 on success, -EWOULDBLOCK if NB set & lock unavailable,
 * or -EINVAL for bad flags.
 */
int flock_op(file_t *filp __attribute__((unused)),
             void   *owner,
             int     op)
{
    ctx_init_once();                             /* NEW: prepare g_ctx */
    file_lock_context_t *ctx = &g_ctx;

    /* sanity on op bits */
    if ((op & (LOCK_SH | LOCK_EX | LOCK_UN)) == 0)
        return -EINVAL;

    spinlock_acquire(&ctx->lock);

    /* ------------ UNLOCK branch (unchanged except indentation) ----- */
    if (op & LOCK_UN) {
        file_lock_t **pp = &ctx->granted;
        while (*pp) {
            if ((*pp)->fl_owner == owner) {
                file_lock_t *dead = *pp;
                *pp = dead->next_granted;
                CV_broadcast(&dead->fl_wait);
            } else {
                pp = &(*pp)->next_granted;
            }
        }

    again:
        for (file_lock_t *req = ctx->blocked; req; req = req->next_blocked) {
            bool clash = false;
            for (file_lock_t *g = ctx->granted; g; g = g->next_granted)
                if (lock_conflict(g, req)) { clash = true; break; }

            if (!clash) {
                grant_lock(ctx, req);
                goto again;                    /* list changed */
            }
        }
        spinlock_release(&ctx->lock);
        return 0;
    }

    /* ------------ LOCK branch (identical to your version) ---------- */
    file_lock_t local_req = {
        .fl_file  = filp,
        .fl_owner = owner,
        .fl_pid   = get_curid(),
        .fl_type  = (op & LOCK_EX) ? LOCK_EX : LOCK_SH,
        .fl_flags = 0,
        .fl_start = 0,
        .fl_end   = ~0UL,
    };
    CV_init(&local_req.fl_wait);

    bool conflict = false;
    for (file_lock_t *g = ctx->granted; g; g = g->next_granted)
        if (lock_conflict(g, &local_req)) { conflict = true; break; }

    if (!conflict) {
        local_req.next_granted = ctx->granted;
        ctx->granted = &local_req;
        spinlock_release(&ctx->lock);
        return 0;
    }

    if (op & LOCK_NB) {
        spinlock_release(&ctx->lock);
        return -EWOULDBLOCK;
    }

    local_req.next_blocked = ctx->blocked;
    ctx->blocked = &local_req;
    local_req.fl_flags |= FL_SLEEP;

    while (true) {
        CV_wait(&local_req.fl_wait, &ctx->lock);

        bool granted = false;
        for (file_lock_t *g = ctx->granted; g; g = g->next_granted)
            if (g == &local_req) { granted = true; break; }

        if (granted) {
            spinlock_release(&ctx->lock);
            return 0;
        }
    }
}

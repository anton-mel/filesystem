<!-- Compile: make / make all
Run tests: make clean && make TEST=1
Run in qemu: make qemu / make qemu-nox
Debug with gdb: make qemu-gdb / make qemu-nox-gdb
                (in another terminal) gdb -->

<!-- List here the following info:
1. who you have worked with
2. whether you coded this assignment together, and if not, who worked on which part
3. brief description of what you have implemented
4. and anything else you would like us to know -->

# Final Project: Advanced Synchronization - File Sharing

Team: Anton Melnychuk & Oliver Li

If multiple user threads open one file, the inconsistency may occur due to the lack of mutual exclusion. One solution to avoid this issue is to provide the file lock. In Linux, the file lock is called flock, which provides the functions of applying or removing an advisory lock on an open file. The details of the flock can be found in its [manual page](https://man7.org/linux/man-pages/man2/flock.2.html). This project ports this **Linux‑style `flock(2)` advisory lock** to the simplified mCertiKOS kernel.

```
man 2 flock
```

*Reference man‑page:* [https://man7.org/linux/man-pages/man2/flock.2.html](https://man7.org/linux/man-pages/man2/flock.2.html)

---

## 1 · What the API Looks Like

```c
/* user/include/file.h */

int flock(int fd, int op);
/* op is a bit‑mask:
 *   LOCK_SH           shared (reader) lock
 *   LOCK_EX           exclusive (writer) lock
 *   LOCK_UN           unlock
 *   LOCK_NB           non‑blocking modifier
 *
 * returns  0                  success
 *          ‑EWOULDBLOCK       would block with LOCK_NB
 *          ‑1                 bad combo / bad fd / nothing to unlock
 */
```

Calling convention is identical to Linux.

---

## 2 · How It Works Internally

* File’s **inode** stores a `struct flock` (state + two wait‑queues).
* Two kinds of owners
   `active_writer` **or** `active_readers > 0` (never both).
* **Writer‑preferred policy**
   As soon as a writer queues, *new* shared locks are blocked — avoids writer starvation.
* Each `flock_acquire()` / `flock_release()` section is protected by a **spin‑lock**; sleeping is done with two CVs (`cv_readers`, `cv_writers`).

State diagram

```
        LOCK_IDLE
      ↙           ↘
 LOCK_SHARED   LOCK_EXCLUSIVE
```

Up‑/Downgrade is allowed but not atomic (mirrors Linux semantics).

---

## 3 · Features & Edge‑Cases

| case                                 | behaviour                                           |                                     |
| ------------------------------------ | --------------------------------------------------- | ----------------------------------- |
| \`LOCK\_EX                           | LOCK\_NB\` on busy lock                             | returns ‑EWOULDBLOCK (never sleeps) |
| multiple `LOCK_SH` holders           | all proceed until a writer queues                   |                                     |
| queued writer then new reader        | reader waits → writer runs first (writer‑preferred) |                                     |
| downgrade `EX → SH`                  | allowed; wakes queued readers                       |                                     |
| upgrade `SH → EX`                    | allowed if last reader; else blocks/‑EWOULDBLOCK    |                                     |
| invalid op (both SH & EX or neither) | returns ‑1                                          |                                     |
| close(fd) while locked               | implicit `LOCK_UN` just like Linux                  |                                     |

Maximum simultaneous readers is limited only by thread count; no hardcoded buffer limits are imposed by the lock.

## 4. Flock Notes & Design Choices

`flock()` is an **built-in library** to use: a thread calls `flock(fd, OP)` with `OP = LOCK_SH` (shared), `LOCK_EX` (exclusive), or `LOCK_UN` (unlock). If the requested lock is available it is granted instantly; otherwise the kernel parks the caller on a condition variable and lets other threads run until the lock is released. Appending `LOCK_NB` flips the behaviour: the call becomes “try once,” returning immediately with `‑EWOULDBLOCK` instead of blocking.

In out case, the lock lives in the inode—not the file descriptor—so every descriptor that refers to the same file synchronises on a single shared structure, while descriptors that refer to different inodes never interfere. Each file descriptor simply records the *kind* of lock the calling thread currently holds. A given inode is therefore always in one of three states: **idle**, **shared**, or **exclusive**; it can never hold shared and exclusive ownership at the same time.

When the inode is in the exclusive state, every subsequent lock request—shared or exclusive—blocks (or returns `‑EWOULDBLOCK` if `LOCK_NB` was supplied). When in the shared state, additional shared requests are admitted immediately, but any incoming exclusive request is queued and, from that point on, no further readers are allowed until the writer has run—this writer‑first policy prevents writer starvation. Up‑ and down‑grading (`SH → EX` or `EX → SH`) follow the Linux rule: the caller briefly unlocks and re‑locks, so another thread could slip in between, but the internal state transitions and wake‑ups remain atomic and race‑free.

## 5. Testing

In the flocktests, we try demonstrate our flock in a variety of situations. The first section is a basic correctness test. It shows that the flock call will actually succeed as intended. The second "mutual exclusion" section shows the difference between the exclusive and shared lock. It shows how the exclusive lock will block if other locks are present. The third section "starvation" is a very primitive proof of concept of starvation. Our flock implementation is writer-preferential (as opposed to Linux's design which is reader-preferential). Our design will block future readers from reading if a writer is in the waiting queue. Linux's design would have the reader fast track through the queue even if a writer was waiting. With our design, readers will starve more frequently because they will be blocked whenever a writer is queued or executing. The last section just tests edge cases.

We needed to implement sys_exit and sys_wait in order to time operations between flocks and show the actual functionality. Our sys_exit is very primitive and doesn't actually clean up the process. That was deemed to be out of the scope of our project and ancilliary to the main purpose of just demonstrating flock. We also changed how sys_spawn works so that any open container can be the child of a process rather than the mathematical way of finding a child before. These two changes allowed the flock tests to actually work.

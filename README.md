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

If multiple user threads open one file, the inconsistency may occur due to the lack of mutual exclusion. One solution to avoid this issue is to provide the file lock. In Linux, the file lock is called flock, which provides the functions of applying or removing an advisory lock on an open file. The details of the flock can be found in its [manual page](https://man7.org/linux/man-pages/man2/flock.2.html):

```
man 2 flock
```

A file could be shared in 2 styles:

- **Exclusive access:** when the file is locked, all the attempts to access this file would be put into the waiting list. Those threads will not proceed until the lock is released.
- **Shared access:** shared access is used to simulate the multiple readers one writer situation, while if all the threads hold the shared lock, they could access the file without any conflicts, but if one of them upgrade its lock into the exclusive lock, only the one holds the exclusive lock could access to the file, others have to wait.

In the flocktests, we try demonstrate our flock in a variety of situations. The first section is a basic correctness test. It shows that the flock call will actually succeed as intended. The second "mutual exclusion" section shows the difference between the exclusive and shared lock. It shows how the exclusive lock will block if other locks are present. The third section "starvation" is a very primitive proof of concept of starvation. Our flock implementation is writer-preferential (as opposed to Linux's design which is reader-preferential). Our design will block future readers from reading if a writer is in the waiting queue. Linux's design would have the reader fast track through the queue even if a writer was waiting. With our design, readers will starve more frequently because they will be blocked whenever a writer is queued or executing. The last section just tests edge cases.

Since fork does not work in this OS, our flocktests spawn small hard-coded helper binaries. These are found in the user/flocktests/helpers folder. They are used like building blocks in our flocktests when we need to spawn new processes to demonstrate some purpose.

We needed to implement sys_exit and sys_wait in order to time operations between flocks and show the actual functionality. Our sys_exit is very primitive and doesn't actually clean up the process. That was deemed to be out of the scope of our project and ancilliary to the main purpose of just demonstrating flock. We also changed how sys_spawn works so that any open container can be the child of a process rather than the mathematical way of finding a child before. These two changes allowed the flock tests to actually work. 
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

# 🧪 Final Project: Advanced Synchronization - File Sharing

If multiple user threads open one file, the inconsistency may occur due to the lack of mutual exclusion. One solution to avoid this issue is to provide the file lock. In Linux, the file lock is called flock, which provides the functions of applying or removing an advisory lock on an open file. The details of the flock can be found in its [manual page](https://man7.org/linux/man-pages/man2/flock.2.html):

```
man 2 flock
```

A file could be shared in 2 styles:

- **Exclusive access:** when the file is locked, all the attempts to access this file would be put into the waiting list. Those threads will not proceed until the lock is released.
- **Shared access:** shared access is used to simulate the multiple readers one writer situation, while if all the threads hold the shared lock, they could access the file without any conflicts, but if one of them upgrade its lock into the exclusive lock, only the one holds the exclusive lock could access to the file, others have to wait.

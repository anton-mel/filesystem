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

# 🧪 Lab 4: Multicore and Preemption

In the first part of this lab, we add multiprocessor support to mCertiKOS. Next, we implement a preemptive scheduler, and make some designated parts of the kernel preemtable by turning on interrupts during those parts of kernel code. Last, we designed and implemented the producer-consumer problem (also known as the bounded-buffer problem) with shared objects and condition variables.

## 👥 Team: Anton Melnychuk and Oliver Li

Everyone contributed equally to the assignment.

### 📋 Workload Distribution

| Name           | Tasks                                                          |
|----------------|----------------------------------------------------------------|
| Anton Melnychuk| - Read Chapter 5 [Synchronizing Access to Shared Objects]      |
|                | - Part 1: Multicore Support (Exercise 2)                       |
|                | - Part 1: Multicore Support (Exercise 3-5)                     |
|                | - Part 2: Preemptive Multitasking (Exercise 6)                 |
|                | - Part 3: Preempting Kernel Execution (Exercise 7, 8, 10)      |
|                | - Part 4: The Producer-Consumer Problem                        |
| Oliver Li      | - Read Chapter 5 [Synchronizing Access to Shared Objects]      |

### 🛠️ How to Run

This assignment is organized into four parts, each located in a separate branch: `lab4part1`, `lab4part2`, `lab4part3`, and `lab4part4`. To view the implementation of a specific part, please switch to the corresponding branch.

To see the output of the final part, run:

```sh
make && make qemu-nox
```

Refer to the screenshot below for the expected output.

> [!NOTE]
> If the kernel occasionally panics in th initialization stage, this error is not related to this solution and was present in the starter code. Read https://edstem.org/us/courses/72435/discussion/6421756 post.

### ✅ Final Results

![Final Output](./static/proof.png)

There are two producer and two consumer processes launched in `kern/init.c`, along with an idle process for each CPU. This results in a total of **six processes** that the scheduler preempts and manages over 2 CPUs. We aimed to optimize the system by putting the idle thread to sleep until the producer-consumer tasks finished, but due to limitations in the OS (aka, lack of proper synchronization and wake-up hooks for the userspace), we’ve deferred this optimization for future work.

At the end of execution, we can observe that the **bounded buffer is completely free**, which confirms correct behavior — since we have a balanced number of system calls for both production and consumption. If one side had an imbalance, it would result in a stuck state, with one thread waiting indefinitely. The output screenshot matches the expected results from the assignment: the number of producer/consumer processes created equals the number exited, indicating successful completion **without deadlocks or race conditions**.

There are occasional slowdowns in behavior, but we suspect they are unrelated to the core correctness of the producer-consumer implementation and thus considered out of scope for this assignment.

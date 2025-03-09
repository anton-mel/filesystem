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

# Lab 4: Multicore and Preemption

In the first part of this lab, we add multiprocessor support to mCertiKOS. Next, we implement a preemptive scheduler, and make some designated parts of the kernel preemtable by turning on interrupts during those parts of kernel code. Last, we designed and implemented the producer-consumer problem (also known as the bounded-buffer problem) with shared objects and condition variables.

## Team: Anton Melnychuk and Oliver Li

Everyone contributed equally to the assignment.

### Workload Distribution

| Name           | Tasks                                                          |
|----------------|----------------------------------------------------------------|
| Anton Melnychuk| - Read Chapter 5 [Synchronizing Access to Shared Objects]      |
|                | - Part 1: Multicore Support (Exercise 2)                       |
|                | - Part 1: Multicore Support (Exercise 3-5)                     |
|                | - Part 2: Preemptive Multitasking (Exercise 6)                 |
| Oliver Li      | - Read Chapter 5 [Synchronizing Access to Shared Objects]      |

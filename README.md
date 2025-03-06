## Instructions to Run

Compile: make / make all

Run tests: make clean && make TEST=1

Run in qemu: make qemu / make qemu-nox

Debug with gdb: make qemu-gdb / make qemu-nox-gdb

(in another terminal) gdb

## Contributions
Team: Anton Melnychuk (am3785) and Oliver Li (fl468)

Pair Programming (Part 1 & 2 & 3)

Oliver Li -- Debugging Part1

Anton Melnychuk -- Debugging Part3

## Unit Testing 

For this lab, we have developed a set of custom fork unit tests (user-space programs) alongside the provided `fork_test` and `idle` processes. We modified the Makefiles and integrated everything by manually adding additional `sys_spawn(4, 1000)`, `sys_spawn(5, 1000)`, and `sys_spawn(6, 1000)` test cases.

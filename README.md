[![Review Assignment Due Date](https://classroom.github.com/assets/deadline-readme-button-22041afd0340ce965d47ae6ef1cefeee28c7c493a6346c4f15d667ab976d596c.svg)](https://classroom.github.com/a/2YrP84_e)

Compile: make / make all <br>
Run tests: make clean && make TEST=1 <br>
Run in qemu: make qemu / make qemu-nox <br>
Debug with gdb: make qemu-gdb / make qemu-nox-gdb <br>
                (in another terminal) gdb

# Team: Anton Melnychuk and Oliver Li
Everyone contributed equally to the assignment.


## Question A

1.1 **What is the first instruction that the processor executes in 32-bit mode?**

The first instruction executed is (line 115)
```assembly
movw $PROT_MODE_DSEG, %ax
``` 
after the OS kernel has transitioned from a real-mode using a CR0 register described below.

Here `PROT_MODE_DSEG = 0x10` is a kernel data segment selector. This further loads `%ax` with the selector value, which is later used to initialize various general-purpose segment registers (`%ds`, `%es`, `%fs`, `%gs`, `%ss`) to to the same protected mode segments as defined in the Global Descriptor Table (GDT). This way OS sets up a single, flat data segment that covers the entire memory range and transitions mostly to a pagging system (VA to PA), making segmentation unnecessary.

1.2 **What exactly causes the switch from 16- to 32-bit mode?**

There are 3 stages. First, we load the GDT using `lgdt gdtdesc` instruction. Then, immediately after setting the PE flag of the MSW in CR0 (boo1, line 106): 
```
movl %cr0, %eax
orl $CR0_PE_ON, %eax
movl %eax, %cr0
```
the initialization code must flush the processor's instruction prefetch queue by executing a JMP instruction. The 80386 fetches and decodes instructions and addresses before they are used; however, after a change into protected mode, the prefetched instruction information (which pertains to real-address mode) is no longer valid. A JMP forces the processor to discard the invalid information [from Intel 80386 Manual]. So we should consider the line after that as a valid starting point of the 32-bit mode (answer part a).

2.1 **What is the last instruction of the boot loader executed?**

The last instruction executed by the boot loader is after we exit boot1
```
    call 0x8f9b <exec_kernel>
```
and enter bootloader’s handoff to the kernel
```
	.set MBOOT_INFO_MAGIC, 0x2badb002

	.globl exec_kernel
	.code32
exec_kernel:
	cli
	movl	$MBOOT_INFO_MAGIC, %eax
	movl	8(%esp), %ebx
	movl	4(%esp), %edx
	jmp	*%edx
```

It starts by disabling interrupts with cli to prevent any disruption during the setup. It then places a predefined magic number (0x2badb002) into the EAX register as a validation marker, ensuring the kernel recognizes that it was booted correctly after jump. Next, it retrieves in runtime two values from the stack–one into EBX and another into EDX—which contain boot-related parameters (like multiboot information and the kernel’s entry address). Finally, it jumps to the address in EDX, effectively transferring control to the kernel’s initialization code, where the magic key is asserted to ensure successful boot. Othwerwise it enters a halt condition spin (kern/init/entry.S):

```
start:
	cli

	/* check whether the bootloader provide multiboot information */
	cmpl	$MULTIBOOT_BOOTLOADER_MAGIC, %eax
	jne	spin
```

So the last instruction is
```
	jmp	*%edx
```

2.2 **What is the first instruction of the kernel it just loaded?**

```
    cli
```

needed to clean up the interrupt flags; disables interrupts. This is essential because the kernel needs to gain full control of the system and prevent interruptions during critical initialization. After that the kernel initialization is proceeded. OS sets up physical memory and written in the following part of the lab and jumps to `kernel_main` to execute provided test cases.

3.1 **Where is the first instruction of the kernel loaded in memory?**

Given the kernel is loaded and the entry address is returned as output, OS passes this value along with bootloader infotable to the `exec_kernel` function. It useS this entry addresses received as an argument to further make a jump. As we mentioned in the previous question the first instruction of this loaded kernel is the `cli`, while its PA address is gdb is `0x102944`.

Another easy way to check it is using the `kerninfo` command:

```
$> kerninfo
Special kernel symbols:
  start  00102944
  etext  00102f21
  edata  0010654c
  end    0094dc44
Kernel executable memory footprint: 8493KB
```

> NOTE
> I created a helper function in a bootloader which may potentially case the result to be slightly different.

### Exercise 3. Trace through the first few instructions of the boot loader again and identify the first instruction that would “break” or otherwise do the wrong thing if you were to get the boot loader’s link address wrong. Then change the link address in boot/boot0/Makefile.inc to something wrong, execute make clean, recompile the lab with make, and trace into the bootloader again to see what happens. Don’t forget to change the link address back and execute make clean again afterward!

[Provided additionaly to the assignment, not graded] Although GDB does not allow me to reach the faulting line, if the `boot1` loader's link address is set incorrectly, the first issue we would assume likely occur at `int $0x13`, command after all the configurations such as LBA, buffer addresses, size od DAP etc have been pushed to the stack. `Boot0` invoked by BIOS calls interrupt 13 to read the sector from the disk, but because linking is pointing to invalid code or data or non-existent address, this jump could lead to variety of problems starting falting silently, some undefined behavior, or thirple faults (if protection is not implemented) with rebooting, depending what CPU actually finds. On the other hand, if the boot0 loader’s link is invalid, we will not be even able to see the “Start boot0 …” message, as it likely will fail immediately at the `int %10` as boo0 is located at a fixed address. 

## Question B

1.1 **How does the boot loader decide how many sectors it must read in order to fetch the entire kernel from disk? Where does it find this information?**

The boot loader determines how many sectors to read by examining the ELF header. The program header information, which contains the location and size of the segments to load, is accessed via the e_phoff field, which points to the offset of the program header in the ELF file. The loader uses this information to further decide ph and eph to calculate how many sectors are required to load the kernel into memory. In particular one would just iterate over and read every section with its entries in the table; each entry gives information such as the section name, the section size, and so on. Look at the example below to understand how mCertiKOS does it:

```
    // is this a valid ELF?
    if (ELFHDR->e_magic != ELF_MAGIC)
        panic("Kernel is not a valid elf.");

    // load each program segment (ignores ph flags)
    ph = (proghdr *) ((uint8_t *) ELFHDR + ELFHDR->e_phoff);
    eph = ph + ELFHDR->e_phnum;

    for (; ph < eph; ph++) {
        readsection(ph->p_va, ph->p_memsz, ph->p_offset, dkernel);
    }
```

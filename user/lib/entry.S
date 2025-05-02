	.text
	.globl _start
_start:
	/*
	 * If there are arguments on the stack, then the current stack will not
	 * be aligned to a nice big power-of-two boundary.
	 */
	testl	$0x0fffffff, %esp
	jnz	args_exist

noargs:
	/* If no arguments are on the stack, push two dummy zero. */
	pushl	$0
	pushl	$0

args_exist:
	/* Jump to the C part. */
	call	main
	movl	%eax, %ebx
	movl	$16,  %eax

	int 	$0x30

spin:
	jmp spin
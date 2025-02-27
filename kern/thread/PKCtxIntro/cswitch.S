/*
 * void cswitch(struct kctx *from, struct kctx *to);
 */
	.globl cswitch
cswitch:
	movl	4(%esp), %eax	/* %eax <- from */ 
	movl	8(%esp), %edx	/* %edx <- to */

	/* TODO: save the old kernel context */
	lea 	4(%esp), %ecx
	movl	%ecx, (%eax) /*from.esp = esp */

	movl	%edi, 4(%eax)
	movl	%esi, 8(%eax)
	movl	%ebx, 12(%eax)
	movl	%ebp, 16(%eax)

	movl	(%esp), %ecx
	movl	%ecx, 20(%eax) /* from.eip = current stack's return address */

	/* TODO: load the new kernel context */
	movl	(%edx), %esp
	movl	4(%edx), %edi
	movl	8(%edx), %esi
	movl	12(%edx), %ebx
	movl	16(%edx), %ebp

	xor	%eax, %eax
	jmp 	*20(%edx)

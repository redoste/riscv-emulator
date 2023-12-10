.section .text

.globl dr_entry
dr_entry:
	/* R8 points to x16 to make sure all the registers are available
	 * with a [-128;127] disp
	 */
	lea 16*8(%rsi), %r8
	mov %rdx, %r9
	lea dr_exit(%rip), %r10

	/* NOTE : for now we don't clobber any callee-saved registers
	 *        if we end up using one of them, make sure to backup
	 *        them properly
	 */
	//push %rbx
	//push %rbp
	//push %r12
	//push %r13
	//push %r14
	//push %r15

	jmp *%rdi

dr_exit:
	/* TODO : dispatch without changing context when the code is
	 *        already cached
	 */
	mov %r9, %rax

	//pop %r15
	//pop %r14
	//pop %r13
	//pop %r12
	//pop %rbp
	//pop %rbx

	ret

.section .note.GNU-stack, "", %progbits

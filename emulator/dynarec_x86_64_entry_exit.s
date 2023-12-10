.section .text

.globl dr_entry
dr_entry:
	/* R8 points to x16 to make sure all the registers are available
	 * with a [-128;127] disp
	 */
	lea 16*8(%rdx), %r8
	mov %rcx, %r9
	lea dr_exit(%rip), %r10
	lea dr_emu_functions(%rip), %r11

	/* NOTE : for now we only clobber one callee-saved register
	 *        if we end up using other ones, make sure to backup
	 *        them properly
	 */
	//push %rbx
	//push %rbp
	push %r12
	//push %r13
	//push %r14
	//push %r15

	mov %rdi, %r12

	jmp *%rsi

dr_exit:
	/* TODO : dispatch without changing context when the code is
	 *        already cached
	 */
	mov %r9, %rax

	//pop %r15
	//pop %r14
	//pop %r13
	pop %r12
	//pop %rbp
	//pop %rbx

	ret

.section .data
dr_emu_functions:
	.quad emu_w8
	.quad emu_w16
	.quad emu_w32
	.quad emu_w64
	.quad emu_r8
	.quad emu_r16
	.quad emu_r32
	.quad emu_r64
	.quad emu_ecall
	.quad emu_ebreak

.section .note.GNU-stack, "", %progbits

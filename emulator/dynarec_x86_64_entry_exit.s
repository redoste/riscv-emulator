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

	/* NOTE : for now we only clobber a few callee-saved registers
	 *        if we end up using other ones, make sure to backup
	 *        them properly
	 */
	//push %rbx
	//push %rbp
	push %r12
	//push %r13
	push %r14
	push %r15

	mov %rdi, %r12

	jmp *%rsi

dr_exit:
	/* TODO : dispatch without changing context when the code is
	 *        already cached
	 */
	mov %r9, %rax

	pop %r15
	pop %r14
	//pop %r13
	pop %r12
	//pop %rbp
	//pop %rbx

	ret

.macro DR_WX_WRAPPER size
dr_w\size\()_wrapper:
	/* We need to keep the stack properly aligned, i.e. it is 16-byte aligned
	 * on the `call` instruction and the push of the return address will make it
	 * 8-byte aligned
	 */
	sub $8, %rsp
	call emu_w\size
	add $8, %rsp

	test %al, %al
	jnz dr_w\size\()_wrapper.1
	ret
dr_w\size\()_wrapper.1:
	/* If some cache entry were invalidated, we drop the return pointer and
	 * short-circuit back to `dr_exit` as the current block might have been
	 * munmaped in the case of self-modifying code
	 */
	add $8, %rsp
	pop %r11
	pop %r10
	pop %r9
	pop %r8

	// And we don't forget to increment PC as the write is now done
	add $4, %r9
	jmp *%r10
.endm

DR_WX_WRAPPER 8
DR_WX_WRAPPER 16
DR_WX_WRAPPER 32
DR_WX_WRAPPER 64

.section .data
dr_emu_functions:
	.quad dr_w8_wrapper      /* [0]  */
	.quad dr_w16_wrapper     /* [1]  */
	.quad dr_w32_wrapper     /* [2]  */
	.quad dr_w64_wrapper     /* [3]  */
	.quad emu_r8             /* [4]  */
	.quad emu_r16            /* [5]  */
	.quad emu_r32            /* [6]  */
	.quad emu_r64            /* [7]  */
	.quad emu_ecall          /* [8]  */
	.quad emu_ebreak         /* [9]  */
	.quad cpu_csr_read       /* [10] */
	.quad cpu_csr_write      /* [11] */
	.quad cpu_csr_exchange   /* [12] */
	.quad cpu_csr_set_bits   /* [13] */
	.quad cpu_csr_clear_bits /* [14] */

.section .note.GNU-stack, "", %progbits

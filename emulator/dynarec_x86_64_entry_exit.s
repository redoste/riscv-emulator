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
dr_emu_w\size\()_wrapper:
	// We save the current PC to emu->cpu.pc
	mov %r9, 0(%r12)
	// We pass the emu as the first argument
	mov %r12, %rdi

	push %r8
	push %r9
	push %r10
	push %r11

	/* We need to keep the stack properly aligned, i.e. it is 16-byte aligned
	 * on the `call` instruction and the push of the return address will make it
	 * 8-byte aligned
	 */
	sub $8, %rsp
	call emu_w\size
	add $8, %rsp

	mov 8(%r12), %dil /* jump_pending */
	or 9(%r12), %dil  /* exception_pending */
	test %dil, %dil
	jnz dr_wrappers_short_circuit

	test %al, %al
	jnz dr_wrappers_write_done_short_circuit

	pop %r11
	pop %r10
	pop %r9
	pop %r8
	ret
.endm

dr_wrappers_write_done_short_circuit:
	/* If some cache entry were invalidated, we drop the return pointer and
	 * short-circuit back to `dr_exit` as the current block might have been
	 * munmaped in the case of self-modifying code
	 */
	pop %r11
	pop %r10
	pop %r9
	pop %r8
	add $8, %rsp

	// And we don't forget to increment PC as the write is now done
	add $4, %r9
	jmp *%r10

.macro DR_WRAPPER name
dr_\name\()_wrapper:
	// We save the current PC to emu->cpu.pc
	mov %r9, 0(%r12)
	// We pass the emu as the first argument
	mov %r12, %rdi

	push %r8
	push %r9
	push %r10
	push %r11

	/* We need to keep the stack properly aligned, i.e. it is 16-byte aligned
	 * on the `call` instruction and the push of the return address will make it
	 * 8-byte aligned
	 */
	sub $8, %rsp
	call \name
	add $8, %rsp

	mov 8(%r12), %dil /* jump_pending */
	or 9(%r12), %dil  /* exception_pending */
	test %dil, %dil
	jnz dr_wrappers_short_circuit

	pop %r11
	pop %r10
	pop %r9
	pop %r8
	ret
.endm

dr_wrappers_short_circuit:
	/* If `jump_pending` or `exception_pending` were set by an emulator function,
	 * we short-circuit back to `dr_exit` to redispatch to a new dynarec code block
	 */
	pop %r11
	pop %r10
	pop %r9
	pop %r8
	add $8, %rsp

	// We read back the new PC to r9
	mov 0(%r12), %r9
	jmp *%r10

DR_WX_WRAPPER 8
DR_WX_WRAPPER 16
DR_WX_WRAPPER 32
DR_WX_WRAPPER 64

DR_WRAPPER emu_r8
DR_WRAPPER emu_r16
DR_WRAPPER emu_r32
DR_WRAPPER emu_r64
DR_WRAPPER emu_ecall
DR_WRAPPER emu_ebreak
DR_WRAPPER cpu_csr_read
DR_WRAPPER cpu_csr_write
DR_WRAPPER cpu_csr_exchange
DR_WRAPPER cpu_csr_set_bits
DR_WRAPPER cpu_csr_clear_bits
DR_WRAPPER cpu_mret

.section .data
dr_emu_functions:
	.quad dr_emu_w8_wrapper             /* [0]  */
	.quad dr_emu_w16_wrapper            /* [1]  */
	.quad dr_emu_w32_wrapper            /* [2]  */
	.quad dr_emu_w64_wrapper            /* [3]  */
	.quad dr_emu_r8_wrapper             /* [4]  */
	.quad dr_emu_r16_wrapper            /* [5]  */
	.quad dr_emu_r32_wrapper            /* [6]  */
	.quad dr_emu_r64_wrapper            /* [7]  */
	.quad dr_emu_ecall_wrapper          /* [8]  */
	.quad dr_emu_ebreak_wrapper         /* [9]  */
	.quad dr_cpu_csr_read_wrapper       /* [10] */
	.quad dr_cpu_csr_write_wrapper      /* [11] */
	.quad dr_cpu_csr_exchange_wrapper   /* [12] */
	.quad dr_cpu_csr_set_bits_wrapper   /* [13] */
	.quad dr_cpu_csr_clear_bits_wrapper /* [14] */
	.quad dr_cpu_mret_wrapper           /* [15] */

.section .note.GNU-stack, "", %progbits

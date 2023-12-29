#ifndef CPU_CSR_H
#define CPU_CSR_H

#include "isa.h"

// NOTE : forward declaration to deal with a cyclic dependency with emulator.h
typedef struct emulator_t emulator_t;

/* cpu_csr_read : read a control and status register
 *                returns the value of the CSR
 *     emulator_t* emu   : pointer to the emulator
 *     guest_reg csr_num : CSR number
 */
guest_reg cpu_csr_read(emulator_t* emu, guest_reg csr_num);

/* cpu_csr_write : write a control and status register
 *     emulator_t* emu   : pointer to the emulator
 *     guest_reg csr_num : CSR number
 *     guest_reg value   : CSR value
 */
void cpu_csr_write(emulator_t* emu, guest_reg csr_num, guest_reg value);

/* cpu_csr_exchange : write a control and status register
 *                    returns the old value of the CSR
 *     emulator_t* emu   : pointer to the emulator
 *     guest_reg csr_num : CSR number
 *     guest_reg value   : CSR value
 */
guest_reg cpu_csr_exchange(emulator_t* emu, guest_reg csr_num, guest_reg value);

/* cpu_csr_set_bits : set bits in a control and status register
 *                    returns the old value of the CSR
 *     emulator_t* emu   : pointer to the emulator
 *     guest_reg csr_num : CSR number
 *     guest_reg mask    : mask of the bits to set in the CSR
 */
guest_reg cpu_csr_set_bits(emulator_t* emu, guest_reg csr_num, guest_reg mask);

/* cpu_csr_clear_bits : clear bits in a control and status register
 *                      returns the old value of the CSR
 *     emulator_t* emu   : pointer to the emulator
 *     guest_reg csr_num : CSR number
 *     guest_reg mask    : mask of the bits to clear in the CSR
 */
guest_reg cpu_csr_clear_bits(emulator_t* emu, guest_reg csr_num, guest_reg mask);

/* X_CSRS : X-macro describing all the CSRs supported by the emulator
 *     For the sake of simplicity all 3 types of CSRs are treated similarly as it's not
 *     required by the spec to raise an exception when an illegal value is written to a WLRL
 *         WPRI : reserved writes preserve values, reads ignore values
 *         WLRL : write/read only legal values
 *         WARL : write any values, read legal values
 *
 *     X_RW(NUM, NAME, MASK, BASE) : Read-write CSR
 *     X_RO(NUM, VALUE)            : Read-only CSR
 */
#define X_CSRS                                                                                           \
	/* Machine information registers */                                                              \
	X_RO(CSR_MVENDORID, 0)                                                                           \
	X_RO(CSR_MARCHID, 0)                                                                             \
	X_RO(CSR_MIMPID, 0)                                                                              \
	X_RO(CSR_MHARTID, 0)                                                                             \
	X_RO(CSR_MCONFIGPTR, 0)                                                                          \
                                                                                                         \
	/* Machine trap setup */                                                                         \
	/* TODO : detect invalid privilege modes in xPP */                                               \
	X_RW(CSR_MSTATUS, mstatus, (1 << 22) |         /* TSR : Trap sret */                             \
					   (1 << 21) | /* TW : Timeout wait */                           \
					   (1 << 20) | /* TVM : Trap virtual memory */                   \
					   (1 << 19) | /* MXR : Make executable readable */              \
					   (1 << 18) | /* SUM : Supervisor user memory access */         \
					   (1 << 17) | /* MPRV : Modify privilege */                     \
					   (3 << 11) | /* MPP : Machine previous privilege mode */       \
					   (1 << 8) |  /* SPP : Supervisor previous privilege mode */    \
					   (1 << 7) |  /* MPIE : Machine previous interrupt-enable */    \
					   (1 << 5) |  /* SPIE : Supervisor previous interrupt-enable */ \
					   (1 << 3) |  /* MIE : Machine interrupt-enable */              \
					   (1 << 1),   /* SIE : Supervisor interrupt-enable */           \
	     (2ll << 34) | (2ll << 32))                /* SXL & UXL : XLEN=64 */                         \
	X_RO(CSR_MISA, (2ll << 62) |                   /* MXL : XLEN=64 */                               \
			       (1 << 20) |             /* U mode */                                      \
			       (1 << 18) |             /* S mode */                                      \
			       (1 << 12) |             /* M extension */                                 \
			       (1 << 8) |              /* I base ISA */                                  \
			       (1 << 0))               /* A extension */                                 \
	X_RW(CSR_MEDELEG, medeleg, ~(1 << 11) /* ecall from M-mode can't be delegated */, 0)             \
	X_RW(CSR_MIDELEG, mideleg, ~0, 0)                                                                \
	X_RW(CSR_MIE, mie, (1 << 11) |        /* MEIE : Machine    external interrupt enabled */         \
				   (1 << 9) | /* SEIE : Supervisor external interrupt enabled */         \
				   (1 << 7) | /* MTIE : Machine    timer    interrupt enabled */         \
				   (1 << 5) | /* STIE : Supervisor timer    interrupt enabled */         \
				   (1 << 3) | /* MSIE : Machine    software interrupt enabled */         \
				   (1 << 1),  /* SSIE : Supervisor software interrupt enabled */         \
	     0)                                                                                          \
	X_RW(CSR_MTVEC, mtvec, ~2, 0)                                                                    \
	X_RW(CSR_MCOUNTEREN, mcounteren, 0xffffffff, 0)                                                  \
                                                                                                         \
	/* Machine trap handling */                                                                      \
	X_RW(CSR_MSCRATCH, mscratch, ~0, 0)                                                              \
	X_RW(CSR_MEPC, mepc, ~3, 0)                                                                      \
	X_RW(CSR_MCAUSE, mcause, (1ll << 63) | 0x3f, 0)                                                  \
	X_RW(CSR_MTVAL, mtval, ~0, 0)                                                                    \
	X_RW(CSR_MIP, mip, (1 << 11) |        /* MEIP : Machine    external interrupt pending */         \
				   (1 << 9) | /* SEIP : Supervisor external interrupt pending */         \
				   (1 << 7) | /* MTIP : Machine    timer    interrupt pending */         \
				   (1 << 5) | /* STIP : Supervisor timer    interrupt pending */         \
				   (1 << 3) | /* MSIP : Machine    software interrupt pending */         \
				   (1 << 1),  /* SSIP : Supervisor software interrupt pending */         \
	     0)                                                                                          \
                                                                                                         \
	/* Machine configuration */                                                                      \
	X_RW(CSR_MENVCFG, menvcfg, (3ll << 62) | (0xf << 4) | 1, 0)                                      \
                                                                                                         \
	/* Machine memory protection */                                                                  \
	/* TODO : find a way to declare ranges easily */                                                 \
	X_RO(CSR_PMPCFG0 + 0, 0)                                                                         \
	X_RO(CSR_PMPCFG0 + 2, 0)                                                                         \
	X_RO(CSR_PMPCFG0 + 4, 0)                                                                         \
	X_RO(CSR_PMPCFG0 + 6, 0)                                                                         \
	X_RO(CSR_PMPCFG0 + 8, 0)                                                                         \
	X_RO(CSR_PMPCFG0 + 10, 0)                                                                        \
	X_RO(CSR_PMPCFG0 + 12, 0)                                                                        \
	X_RO(CSR_PMPCFG0 + 14, 0)                                                                        \
	X_RO(CSR_PMPADDR0 + 0, 0)                                                                        \
	X_RO(CSR_PMPADDR0 + 1, 0)                                                                        \
	X_RO(CSR_PMPADDR0 + 2, 0)                                                                        \
	X_RO(CSR_PMPADDR0 + 3, 0)                                                                        \
	X_RO(CSR_PMPADDR0 + 4, 0)                                                                        \
	X_RO(CSR_PMPADDR0 + 5, 0)                                                                        \
	X_RO(CSR_PMPADDR0 + 6, 0)                                                                        \
	X_RO(CSR_PMPADDR0 + 7, 0)                                                                        \
	X_RO(CSR_PMPADDR0 + 8, 0)                                                                        \
	X_RO(CSR_PMPADDR0 + 9, 0)                                                                        \
	X_RO(CSR_PMPADDR0 + 10, 0)                                                                       \
	X_RO(CSR_PMPADDR0 + 11, 0)                                                                       \
	X_RO(CSR_PMPADDR0 + 12, 0)                                                                       \
	X_RO(CSR_PMPADDR0 + 13, 0)                                                                       \
	X_RO(CSR_PMPADDR0 + 14, 0)                                                                       \
	X_RO(CSR_PMPADDR0 + 15, 0)                                                                       \
	X_RO(CSR_PMPADDR0 + 16, 0)                                                                       \
	X_RO(CSR_PMPADDR0 + 17, 0)                                                                       \
	X_RO(CSR_PMPADDR0 + 18, 0)                                                                       \
	X_RO(CSR_PMPADDR0 + 19, 0)                                                                       \
	X_RO(CSR_PMPADDR0 + 20, 0)                                                                       \
	X_RO(CSR_PMPADDR0 + 21, 0)                                                                       \
	X_RO(CSR_PMPADDR0 + 22, 0)                                                                       \
	X_RO(CSR_PMPADDR0 + 23, 0)                                                                       \
	X_RO(CSR_PMPADDR0 + 24, 0)                                                                       \
	X_RO(CSR_PMPADDR0 + 25, 0)                                                                       \
	X_RO(CSR_PMPADDR0 + 26, 0)                                                                       \
	X_RO(CSR_PMPADDR0 + 27, 0)                                                                       \
	X_RO(CSR_PMPADDR0 + 28, 0)                                                                       \
	X_RO(CSR_PMPADDR0 + 29, 0)                                                                       \
	X_RO(CSR_PMPADDR0 + 30, 0)                                                                       \
	X_RO(CSR_PMPADDR0 + 31, 0)                                                                       \
	X_RO(CSR_PMPADDR0 + 32, 0)                                                                       \
	X_RO(CSR_PMPADDR0 + 33, 0)                                                                       \
	X_RO(CSR_PMPADDR0 + 34, 0)                                                                       \
	X_RO(CSR_PMPADDR0 + 35, 0)                                                                       \
	X_RO(CSR_PMPADDR0 + 36, 0)                                                                       \
	X_RO(CSR_PMPADDR0 + 37, 0)                                                                       \
	X_RO(CSR_PMPADDR0 + 38, 0)                                                                       \
	X_RO(CSR_PMPADDR0 + 39, 0)                                                                       \
	X_RO(CSR_PMPADDR0 + 40, 0)                                                                       \
	X_RO(CSR_PMPADDR0 + 41, 0)                                                                       \
	X_RO(CSR_PMPADDR0 + 42, 0)                                                                       \
	X_RO(CSR_PMPADDR0 + 43, 0)                                                                       \
	X_RO(CSR_PMPADDR0 + 44, 0)                                                                       \
	X_RO(CSR_PMPADDR0 + 45, 0)                                                                       \
	X_RO(CSR_PMPADDR0 + 46, 0)                                                                       \
	X_RO(CSR_PMPADDR0 + 47, 0)                                                                       \
	X_RO(CSR_PMPADDR0 + 48, 0)                                                                       \
	X_RO(CSR_PMPADDR0 + 49, 0)                                                                       \
	X_RO(CSR_PMPADDR0 + 50, 0)                                                                       \
	X_RO(CSR_PMPADDR0 + 51, 0)                                                                       \
	X_RO(CSR_PMPADDR0 + 52, 0)                                                                       \
	X_RO(CSR_PMPADDR0 + 53, 0)                                                                       \
	X_RO(CSR_PMPADDR0 + 54, 0)                                                                       \
	X_RO(CSR_PMPADDR0 + 55, 0)                                                                       \
	X_RO(CSR_PMPADDR0 + 56, 0)                                                                       \
	X_RO(CSR_PMPADDR0 + 57, 0)                                                                       \
	X_RO(CSR_PMPADDR0 + 58, 0)                                                                       \
	X_RO(CSR_PMPADDR0 + 59, 0)                                                                       \
	X_RO(CSR_PMPADDR0 + 60, 0)                                                                       \
	X_RO(CSR_PMPADDR0 + 61, 0)                                                                       \
	X_RO(CSR_PMPADDR0 + 62, 0)                                                                       \
	X_RO(CSR_PMPADDR0 + 63, 0)                                                                       \
                                                                                                         \
	/* Machine counter/timers */                                                                     \
	X_RW(CSR_MCYCLE, mcycle, ~0, 0)                                                                  \
	X_RW(CSR_MINSTRET, minstret, ~0, 0)                                                              \
	X_RO(CSR_MHPMCOUNTER3 + 0, 0)                                                                    \
	X_RO(CSR_MHPMCOUNTER3 + 1, 0)                                                                    \
	X_RO(CSR_MHPMCOUNTER3 + 2, 0)                                                                    \
	X_RO(CSR_MHPMCOUNTER3 + 3, 0)                                                                    \
	X_RO(CSR_MHPMCOUNTER3 + 4, 0)                                                                    \
	X_RO(CSR_MHPMCOUNTER3 + 5, 0)                                                                    \
	X_RO(CSR_MHPMCOUNTER3 + 6, 0)                                                                    \
	X_RO(CSR_MHPMCOUNTER3 + 7, 0)                                                                    \
	X_RO(CSR_MHPMCOUNTER3 + 8, 0)                                                                    \
	X_RO(CSR_MHPMCOUNTER3 + 9, 0)                                                                    \
	X_RO(CSR_MHPMCOUNTER3 + 10, 0)                                                                   \
	X_RO(CSR_MHPMCOUNTER3 + 11, 0)                                                                   \
	X_RO(CSR_MHPMCOUNTER3 + 12, 0)                                                                   \
	X_RO(CSR_MHPMCOUNTER3 + 13, 0)                                                                   \
	X_RO(CSR_MHPMCOUNTER3 + 14, 0)                                                                   \
	X_RO(CSR_MHPMCOUNTER3 + 15, 0)                                                                   \
	X_RO(CSR_MHPMCOUNTER3 + 16, 0)                                                                   \
	X_RO(CSR_MHPMCOUNTER3 + 17, 0)                                                                   \
	X_RO(CSR_MHPMCOUNTER3 + 18, 0)                                                                   \
	X_RO(CSR_MHPMCOUNTER3 + 19, 0)                                                                   \
	X_RO(CSR_MHPMCOUNTER3 + 20, 0)                                                                   \
	X_RO(CSR_MHPMCOUNTER3 + 21, 0)                                                                   \
	X_RO(CSR_MHPMCOUNTER3 + 22, 0)                                                                   \
	X_RO(CSR_MHPMCOUNTER3 + 23, 0)                                                                   \
	X_RO(CSR_MHPMCOUNTER3 + 24, 0)                                                                   \
	X_RO(CSR_MHPMCOUNTER3 + 25, 0)                                                                   \
	X_RO(CSR_MHPMCOUNTER3 + 26, 0)                                                                   \
	X_RO(CSR_MHPMCOUNTER3 + 27, 0)                                                                   \
	X_RO(CSR_MHPMCOUNTER3 + 28, 0)                                                                   \
                                                                                                         \
	/* Machine counter setup */                                                                      \
	X_RO(CSR_MCOUNTINHIBIT, 0)                                                                       \
	X_RO(CSR_MHPMEVENT3 + 0, 0)                                                                      \
	X_RO(CSR_MHPMEVENT3 + 1, 0)                                                                      \
	X_RO(CSR_MHPMEVENT3 + 2, 0)                                                                      \
	X_RO(CSR_MHPMEVENT3 + 3, 0)                                                                      \
	X_RO(CSR_MHPMEVENT3 + 4, 0)                                                                      \
	X_RO(CSR_MHPMEVENT3 + 5, 0)                                                                      \
	X_RO(CSR_MHPMEVENT3 + 6, 0)                                                                      \
	X_RO(CSR_MHPMEVENT3 + 7, 0)                                                                      \
	X_RO(CSR_MHPMEVENT3 + 8, 0)                                                                      \
	X_RO(CSR_MHPMEVENT3 + 9, 0)                                                                      \
	X_RO(CSR_MHPMEVENT3 + 10, 0)                                                                     \
	X_RO(CSR_MHPMEVENT3 + 11, 0)                                                                     \
	X_RO(CSR_MHPMEVENT3 + 12, 0)                                                                     \
	X_RO(CSR_MHPMEVENT3 + 13, 0)                                                                     \
	X_RO(CSR_MHPMEVENT3 + 14, 0)                                                                     \
	X_RO(CSR_MHPMEVENT3 + 15, 0)                                                                     \
	X_RO(CSR_MHPMEVENT3 + 16, 0)                                                                     \
	X_RO(CSR_MHPMEVENT3 + 17, 0)                                                                     \
	X_RO(CSR_MHPMEVENT3 + 18, 0)                                                                     \
	X_RO(CSR_MHPMEVENT3 + 19, 0)                                                                     \
	X_RO(CSR_MHPMEVENT3 + 20, 0)                                                                     \
	X_RO(CSR_MHPMEVENT3 + 21, 0)                                                                     \
	X_RO(CSR_MHPMEVENT3 + 22, 0)                                                                     \
	X_RO(CSR_MHPMEVENT3 + 23, 0)                                                                     \
	X_RO(CSR_MHPMEVENT3 + 24, 0)                                                                     \
	X_RO(CSR_MHPMEVENT3 + 25, 0)                                                                     \
	X_RO(CSR_MHPMEVENT3 + 26, 0)                                                                     \
	X_RO(CSR_MHPMEVENT3 + 27, 0)                                                                     \
	X_RO(CSR_MHPMEVENT3 + 28, 0)

/* cpu_csrs_t : structure storing the current value of the CSRs
 */
typedef struct cpu_csrs_t {
#define X_RW(NUM, NAME, MASK, BASE) guest_reg NAME;
#define X_RO(NUM, VALUE)
	X_CSRS
#undef X_RW
#undef X_RO
} cpu_csrs_t;

#endif

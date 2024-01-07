#include <assert.h>
#include <endian.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "emulator.h"
#include "isa.h"
#include "mmu_paging_guest_to_guest.h"
#include "mmu_paging_guest_to_host.h"

static bool mmu_vg2pg_r64(emulator_t* emu, guest_paddr paddr, uint64_t* value) {
	size_t offset = paddr & MMU_PG2H_OFFSET_MASK;
	assert((offset & 7) == 0);

	mmu_pg2h_pte pte;
	if (!mmu_pg2h_get_pte(emu, paddr, &pte)) {
		return false;
	}

	if (pte & MMU_PG2H_PTE_TYPE_MMIO) {
		return false;
	} else {
		uint8_t* pool = (uint8_t*)(pte & MMU_PG2H_PAGE_MASK);
		*value = le64toh(*(uint64_t*)&pool[offset]);
		return true;
	}
}

static bool mmu_vg2pg_w64(emulator_t* emu, guest_paddr paddr, uint64_t value) {
	size_t offset = paddr & MMU_PG2H_OFFSET_MASK;
	assert((offset & 7) == 0);

	mmu_pg2h_pte pte;
	if (!mmu_pg2h_get_pte(emu, paddr, &pte)) {
		return false;
	}

	if (pte & MMU_PG2H_PTE_TYPE_MMIO) {
		return false;
	} else {
		uint8_t* pool = (uint8_t*)(pte & MMU_PG2H_PAGE_MASK);
		uint64_t* host_addr = (uint64_t*)&pool[offset];
		*host_addr = htole64(value);
		return true;
	}
}

static bool mmu_vg2pg_walk(emulator_t* emu, guest_vaddr vaddr, mmu_vg2pg_pte* pte_out, ssize_t* levels_remaining, guest_paddr* pte_paddr) {
	if (MMU_SV39_VPN_TOP(vaddr) != MMU_SV39_VPN_TOPP &&
	    MMU_SV39_VPN_TOP(vaddr) != MMU_SV39_VPN_TOPN) {
		// The virtual address isn't cannonical
		return false;
	}

	guest_vaddr vpn[] = {MMU_SV39_VPN_0(vaddr), MMU_SV39_VPN_1(vaddr), MMU_SV39_VPN_2(vaddr)};

	/* The translation algorithm follows the steps described in the RISC-V supervisor spec :
	 * 4.3.2 Virtual Address Translation Process
	 */

	// 1. Let a be satp.ppn * PAGESIZE, and let i = LEVELS - 1.
	guest_paddr a = (emu->cpu.csrs.satp & 0xfffffffffff) << MMU_VG2PG_PAGE_SHIFT;
	ssize_t i = 3 - 1;
	mmu_vg2pg_pte pte;

	while (1) {
		// 2. Let pte be the value of the PTE at address a+va.vpn[i]*PTESIZE.
		if (!mmu_vg2pg_r64(emu, a + vpn[i] * sizeof(mmu_vg2pg_pte), &pte)) {
			return false;
		}

		/* 3. If pte.v = 0, or if pte.r = 0 and pte.w = 1, or if any bits or encodings that are reserved for
		 *    future standard use are set within pte, stop and raise a page-fault exception corresponding
		 *    to the original access type
		 */
		if (!(pte & MMU_VG2PG_PTE_VALID) ||
		    (!(pte & MMU_VG2PG_PTE_READ) && (pte & MMU_VG2PG_PTE_WRITE)) ||
		    (pte & MMU_VG2PG_PTE_PBMT) != 0 ||
		    (pte & MMU_VG2PG_PTE_N) != 0) {
			return false;
		}

		/* 4. Otherwise, the PTE is valid. If pte.r = 1 or pte.x = 1, go to step 5. Otherwise, this PTE is a
		 *    pointer to the next level of the page table. Let i = i - 1. If i < 0, stop and raise a page-fault
		 *    exception corresponding to the original access type. Otherwise, let a = pte.ppn * PAGESIZE
		 *    and go to step 2.
		 */
		if ((pte & MMU_VG2PG_PTE_READ) || (pte & MMU_VG2PG_PTE_EXEC)) {
			break;
		} else {
			i--;
			if (i < 0) {
				return false;
			}
			a = MMU_SV39_PTE_PPN(pte) << MMU_VG2PG_PAGE_SHIFT;
		}
	}

	*pte_out = pte;
	*levels_remaining = i;
	*pte_paddr = a + vpn[i] * sizeof(mmu_vg2pg_pte);
	return true;
}

bool mmu_vg2pg_translate(emulator_t* emu, mmu_vg2pg_access_type_t access_type, guest_vaddr vaddr, guest_paddr* paddr) {
	guest_vaddr vpn[] = {MMU_SV39_VPN_0(vaddr), MMU_SV39_VPN_1(vaddr), MMU_SV39_VPN_2(vaddr)};

	size_t tlb_index = (vaddr >> MMU_VG2PG_PAGE_SHIFT) & emu->cpu.vg2pg_tlb_mask;
	mmu_vg2pg_tlb_entry_t* tlb_entry = &emu->cpu.vg2pg_tlb[tlb_index];

	mmu_vg2pg_pte pte;
	ssize_t levels_remaining;
	guest_paddr pte_paddr;

	if ((tlb_entry->tag & MMU_VG2PG_PAGE_MASK) == (vaddr & MMU_VG2PG_PAGE_MASK) &&
	    (tlb_entry->tag & MMU_VG2PG_OFFSET_MASK) != 0) {
		pte = tlb_entry->pte;
		levels_remaining = (tlb_entry->tag & MMU_VG2PG_OFFSET_MASK) - 1;
		pte_paddr = (guest_paddr)-1;
	} else {
		if (!mmu_vg2pg_walk(emu, vaddr, &pte, &levels_remaining, &pte_paddr)) {
			return false;
		}

		tlb_entry->tag = (vaddr & MMU_VG2PG_PAGE_MASK) |
				 ((levels_remaining + 1) & MMU_VG2PG_OFFSET_MASK);
		tlb_entry->pte = pte;
	}

	/* 5. A leaf PTE has been found. Determine if the requested memory access is allowed by the
	 *    pte.r, pte.w, pte.x, and pte.u bits, given the current privilege mode and the value of the
	 *    SUM and MXR fields of the mstatus register. If not, stop and raise a page-fault exception
	 *    corresponding to the original access type
	 */
	bool mprv = (emu->cpu.csrs.mstatus >> 17) & 1;
	bool sum = (emu->cpu.csrs.mstatus >> 18) & 1;
	bool mxr = (emu->cpu.csrs.mstatus >> 19) & 1;

	privilege_mode_t effective_priv_mode = emu->cpu.priv_mode;
	if (emu->cpu.priv_mode == M_MODE && mprv) {
		assert(access_type != MMU_VG2PG_ACCESS_EXEC);
		effective_priv_mode = (emu->cpu.csrs.mstatus >> 11) & 3;  // MPP
	}
	assert(effective_priv_mode != M_MODE);

	if (effective_priv_mode == U_MODE && !(pte & MMU_VG2PG_PTE_USER)) {
		return false;
	} else if (effective_priv_mode == S_MODE &&
		   (pte & MMU_VG2PG_PTE_USER) &&
		   (!sum || (access_type == MMU_VG2PG_ACCESS_EXEC))) {
		return false;
	}

	if (access_type == MMU_VG2PG_ACCESS_READ) {
		if (!mxr && !(pte & MMU_VG2PG_PTE_READ)) {
			return false;
		} else if (mxr && !(pte & MMU_VG2PG_PTE_READ) && !(pte & MMU_VG2PG_PTE_EXEC)) {
			return false;
		}
	} else if (access_type == MMU_VG2PG_ACCESS_WRITE && !(pte & MMU_VG2PG_PTE_WRITE)) {
		return false;
	} else if (access_type == MMU_VG2PG_ACCESS_EXEC && !(pte & MMU_VG2PG_PTE_EXEC)) {
		return false;
	}

	/* 6. If i > 0 and pte.ppn[i - 1 : 0] != 0, this is a misaligned superpage; stop and raise a page-fault
	 *    exception corresponding to the original access type.
	 */
	if (levels_remaining == 2 && (MMU_SV39_PTE_PPN_1(pte) != 0 || MMU_SV39_PTE_PPN_0(pte) != 0)) {
		return false;
	} else if (levels_remaining == 1 && MMU_SV39_PTE_PPN_0(pte) != 0) {
		return false;
	}

	/* 7. If pte.a = 0, or if the original memory access is a store and pte.d = 0, either raise a page-fault
	 *    exception corresponding to the original access type, or [...] set pte.a to 1 and, if the original
	 *    memory access is a store, also set pte.d to 1
	 */
	if (!(pte & MMU_VG2PG_PTE_ACCESSED) ||
	    (access_type == MMU_VG2PG_ACCESS_WRITE && !(pte & MMU_VG2PG_PTE_DIRTY))) {
		bool update_pte;
		if (pte_paddr == (guest_paddr)-1) {
			mmu_vg2pg_pte new_pte;
			update_pte = mmu_vg2pg_walk(emu, vaddr, &new_pte, &levels_remaining, &pte_paddr);
			update_pte &= new_pte == pte;
		} else {
			update_pte = true;
		}

		if (update_pte) {
			assert(pte_paddr != (guest_paddr)-1);
			pte |= MMU_VG2PG_PTE_ACCESSED | ((access_type == MMU_VG2PG_ACCESS_WRITE) ? MMU_VG2PG_PTE_DIRTY : 0);
			if (!mmu_vg2pg_w64(emu, pte_paddr, pte)) {
				return false;
			}
			tlb_entry->pte = pte;
		}
	}

	/* 8. The translation is successful. The translated physical address is given as follows:
	 *     * pa.pgoff = va.pgoff.
	 *     * If i > 0, then this is a superpage translation and pa.ppn[i - 1 : 0] = va.vpn[i - 1 : 0].
	 *     * pa.ppn[LEVELS - 1 : i] = pte.ppn[LEVELS - 1 : i].
	 */
	*paddr = (MMU_SV39_PTE_PPN(pte) << MMU_VG2PG_PAGE_SHIFT) |
		 (levels_remaining >= 2 ? vpn[1] << 21 : 0) |
		 (levels_remaining >= 1 ? vpn[0] << 12 : 0) |
		 (vaddr & MMU_VG2PG_OFFSET_MASK);
	return true;
}

void mmu_vg2pg_flush_tlb(emulator_t* emu) {
	size_t tlb_size = emu->cpu.vg2pg_tlb_mask + 1;
	emu->cpu.tlb_or_cache_flush_pending = true;
	memset(emu->cpu.vg2pg_tlb, 0, tlb_size * sizeof(emu->cpu.vg2pg_tlb[0]));
}

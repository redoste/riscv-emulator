#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/mman.h>

#include "emulator.h"
#include "isa.h"
#include "mmu_paging_guest_to_host.h"

static void* mmu_pg2h_map_page_table(void) {
	void* page = mmap(NULL, MMU_PG2H_PAGE_SIZE, PROT_READ | PROT_WRITE,
			  MAP_PRIVATE | MAP_ANONYMOUS, 0, 0);
	if (page == MAP_FAILED) {
		perror("mmap");
	} else {
		assert(((uintptr_t)page & MMU_PG2H_OFFSET_MASK) == 0);
	}
	return page;
}

bool mmu_pg2h_map(emulator_t* emu, guest_paddr guest_physical_page, void* host_page) {
	uintptr_t host_page_int = (uintptr_t)host_page;

	if ((guest_physical_page & MMU_PG2H_OFFSET_MASK) != 0 ||
	    (host_page_int & MMU_PG2H_OFFSET_MASK) != 0) {
		// The guest physical address or the host address aren't aligned to a page boundary
		return false;
	}
	if (MMU_PG2H_PPN_TOP(guest_physical_page) != MMU_PG2H_PPN_TOPP &&
	    MMU_PG2H_PPN_TOP(guest_physical_page) != MMU_PG2H_PPN_TOPN) {
		// The address isn't cannonical, its top bits aren't equal to the top bit of its PPN[2]
		return false;
	}

	guest_paddr ppn[] = {
		MMU_PG2H_PPN_0(guest_physical_page),
		MMU_PG2H_PPN_1(guest_physical_page),
		MMU_PG2H_PPN_2(guest_physical_page),
	};

	mmu_pg2h_pte* previous_level_entry = &emu->pg2h_paging_table;
	mmu_pg2h_pte current_level_pte, *current_level_table;
	for (ssize_t i = 2; i >= 0; i--) {
		current_level_pte = *previous_level_entry;
		current_level_table = (mmu_pg2h_pte*)(current_level_pte & MMU_PG2H_PAGE_MASK);
		if (!(current_level_pte & MMU_PG2H_PTE_VALID)) {
			current_level_table = mmu_pg2h_map_page_table();
			if (current_level_table == MAP_FAILED) {
				return false;
			}
			current_level_pte = (uintptr_t)current_level_table | MMU_PG2H_PTE_VALID;
			*previous_level_entry = current_level_pte;
		}
		previous_level_entry = &current_level_table[ppn[i]];
	}

	mmu_pg2h_pte* level0_entry = previous_level_entry;
	if (*level0_entry & MMU_PG2H_PTE_VALID) {
		// This guest physical page is already mapped
		return false;
	}
	*level0_entry = host_page_int | MMU_PG2H_PTE_VALID | MMU_PG2H_PTE_TYPE_POOL;
	return true;
}

static bool mmu_pg2h_walk(emulator_t* emu, guest_paddr guest_physical_page, mmu_pg2h_pte** pte) {
	if ((guest_physical_page & MMU_PG2H_OFFSET_MASK) != 0) {
		// The guest physical address isn't aligned to a page boundary
		return false;
	}

	if (MMU_PG2H_PPN_TOP(guest_physical_page) != MMU_PG2H_PPN_TOPP &&
	    MMU_PG2H_PPN_TOP(guest_physical_page) != MMU_PG2H_PPN_TOPN) {
		// The address isn't cannonical, its top bits aren't equal to the top bit of its PPN[2]
		return false;
	}

	guest_paddr ppn[] = {
		MMU_PG2H_PPN_0(guest_physical_page),
		MMU_PG2H_PPN_1(guest_physical_page),
		MMU_PG2H_PPN_2(guest_physical_page),
	};

	mmu_pg2h_pte* previous_level_entry = &emu->pg2h_paging_table;
	mmu_pg2h_pte current_level_pte, *current_level_table;
	for (ssize_t i = 2; i >= 0; i--) {
		current_level_pte = *previous_level_entry;
		current_level_table = (mmu_pg2h_pte*)(current_level_pte & MMU_PG2H_PAGE_MASK);
		if (!(current_level_pte & MMU_PG2H_PTE_VALID)) {
			return false;
		}
		previous_level_entry = &current_level_table[ppn[i]];
	}

	*pte = previous_level_entry;
	return true;
}

bool mmu_pg2h_unmap(emulator_t* emu, guest_paddr guest_physical_page) {
	mmu_pg2h_pte* level0_entry;
	if (!mmu_pg2h_walk(emu, guest_physical_page, &level0_entry)) {
		return false;
	}

	if (!(*level0_entry & MMU_PG2H_PTE_VALID)) {
		// This guest physical page isn't mapped
		return false;
	}
	if (!(*level0_entry & MMU_PG2H_PTE_TYPE_MMIO)) {
		munmap((void*)(*level0_entry & MMU_PG2H_PAGE_MASK), MMU_PG2H_PAGE_SIZE);
	}
	*level0_entry = 0;
	return true;
}

bool mmu_pg2h_get_pte(emulator_t* emu, guest_paddr guest_physical_page, mmu_pg2h_pte* pte) {
	mmu_pg2h_pte* level0_entry;
	if (!mmu_pg2h_walk(emu, guest_physical_page & MMU_PG2H_PAGE_MASK, &level0_entry)) {
		return false;
	}

	if (!(*level0_entry & MMU_PG2H_PTE_VALID)) {
		return false;
	}
	*pte = *level0_entry;
	return true;
}

static void mmu_pg2h_free_level(mmu_pg2h_pte* table, size_t level) {
	if (level == 0) {
		munmap(table, MMU_PG2H_PAGE_SIZE);
		return;
	}

	for (size_t i = 0; i < MMU_PG2H_PAGE_SIZE / sizeof(mmu_pg2h_pte); i++) {
		mmu_pg2h_pte entry = table[i];
		if ((entry & MMU_PG2H_PTE_VALID) && !(entry & MMU_PG2H_PTE_TYPE_MMIO)) {
			mmu_pg2h_free_level((mmu_pg2h_pte*)(entry & MMU_PG2H_PAGE_MASK), level - 1);
		}
	}
	munmap(table, MMU_PG2H_PAGE_SIZE);
}

void mmu_pg2h_free(emulator_t* emu) {
	if (emu->pg2h_paging_table & MMU_PG2H_PTE_VALID) {
		mmu_pg2h_free_level((mmu_pg2h_pte*)(emu->pg2h_paging_table & MMU_PG2H_PAGE_MASK), 3);
	}
}

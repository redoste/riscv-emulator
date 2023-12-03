#ifndef MMU_PG2H
#define MMU_PG2H

#include <stdbool.h>
#include <stdint.h>

#include "emulator.h"
#include "isa.h"

/* MMU_PG2H_OFFSET_MASK : mask to extract the offset part of a guest physical
 *                        address
 */
#define MMU_PG2H_OFFSET_MASK 0xfff

/* MMU_PG2H_OFFSET_SIZE : size of a page in the guest physical to host page table
 */
#define MMU_PG2H_PAGE_SIZE (MMU_PG2H_OFFSET_MASK + 1)

/* MMU_PG2H_PAGE_MASK : mask to extract the physical page number part of a
 *                      guest physical address
 */
#define MMU_PG2H_PAGE_MASK ~MMU_PG2H_OFFSET_MASK

/* MMU_PG2H_PAGE_SHIFT : number of bits to shift to get the physical page number
 *                       of a guest physical address
 */
#define MMU_PG2H_PAGE_SHIFT 12

/* MMU_PG2H_PPN_n : macros used to extract the n-th component of the
 *                  physical page number
 */
#define MMU_PG2H_PPN_0(x) (((x) >> 12) & 0x1ff)
#define MMU_PG2H_PPN_1(x) (((x) >> 21) & 0x1ff)
#define MMU_PG2H_PPN_2(x) (((x) >> 30) & 0x1ff)

/* MMU_PG2H_PPN_TOP : macro used to extract the top 26 bits of a guest physical
 *                    address
 *     because they aren't used by the physical guest to host paging, they must
 *     be equal to all-ones (MMU_PG2H_PPN_TOPN) or all-zeros (MMU_PG2H_PPN_TOPP)
 *     for the address to be considered cannonical
 */
#define MMU_PG2H_PPN_TOP(x) ((x) & ~0x3fffffffffull)
#define MMU_PG2H_PPN_TOPN   ~0x3fffffffffull
#define MMU_PG2H_PPN_TOPP   0ull

/* mmu_pg2h_pte : typedef used to represent a page table entry in the physical guest to
 *                host page table
 */
typedef uintptr_t mmu_pg2h_pte;

/* MMU_PG2H_PTE_VALID : flag marking a PTE as valid in the PG2H page table
 */
#define MMU_PG2H_PTE_VALID (1 << 0)

/* MMU_PG2H_PTE_TYPE_POOL : flag marking a PTE as a memory pool (i.e. it points
 *                          to a raw chunk of mmaped host memory)
 */
#define MMU_PG2H_PTE_TYPE_POOL (0 << 1)

/* MMU_PG2H_PTE_TYPE_MMIO : flag marking a PTE as used for MMIO
 */
#define MMU_PG2H_PTE_TYPE_MMIO (1 << 1)

/* mmu_pg2h_map : map a new guest physical page to a host page
 *                returns true if the page was successfully mapped
 *                returns false otherwise
 *     emulator_t* emu                 : pointer to the emulator
 *     guest_paddr guest_physical_page : guest physical page to map
 *     void* host_page                 : host page to map
 */
bool mmu_pg2h_map(emulator_t* emu, guest_paddr guest_physical_page, void* host_page);

/* mmu_pg2h_unmap : unmap a guest physical page
 *                  returns true if the page was successfully unmapped
 *                  returns false otherwise
 *     emulator_t* emu                 : pointer to the emulator
 *     guest_paddr guest_physical_page : guest physical page to unmap
 */
bool mmu_pg2h_unmap(emulator_t* emu, guest_paddr guest_physical_page);

/* mmu_pg2h_get_pte : get the PTE of a page in the physical guest to host page table
 *                    returns true if the PTE is present and valid
 *                    returns false otherwise
 *     emulator_t* emu                 : pointer to the emulator
 *     guest_paddr guest_physical_page : guest physical page corresponding to the requested PTE
 *     mmu_pg2h_pte* pte               : pointer to the mmu_pg2h_pte to fill with the read PTE
 */
bool mmu_pg2h_get_pte(emulator_t* emu, guest_paddr guest_physical_page, mmu_pg2h_pte* pte);

/* mmu_pg2h_free : free all the allocated memory used by the page table
 *     emulator_t* emu : pointer to the emulator
 */
void mmu_pg2h_free(emulator_t* emu);

#endif
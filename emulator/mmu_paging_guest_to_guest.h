#ifndef MMU_VG2PG
#define MMU_VG2PG

#include <assert.h>
#include <stdbool.h>

#include "isa.h"
#include "mmu_paging_guest_to_host.h"

/* MMU_VG2PG_OFFSET_MASK : mask to extract the offset part of a guest virtual
 *                         address
 */
#define MMU_VG2PG_OFFSET_MASK 0xfff

/* MMU_VG2PG_PAGE_SIZE : size of a page in the guest page table
 */
#define MMU_VG2PG_PAGE_SIZE (MMU_VG2PG_OFFSET_MASK + 1)

/* MMU_VG2PG_PAGE_MASK : mask to extract the virtual page number part of a
 *                       guest virtual address
 */
#define MMU_VG2PG_PAGE_MASK ~MMU_VG2PG_OFFSET_MASK

/* MMU_VG2PG_PAGE_SHIFT : number of bits to shift to get the virtual page number
 *                        of a guest virtual address
 */
#define MMU_VG2PG_PAGE_SHIFT 12

static_assert(MMU_VG2PG_OFFSET_MASK == MMU_PG2H_OFFSET_MASK &&
		      MMU_VG2PG_PAGE_SHIFT == MMU_PG2H_PAGE_SHIFT,
	      "The VG2PG and PG2H page tables should have the same page size");

/* MMU_SV39_VPN_n : macros used to extract the n-th component of the
 *                  virtual page number
 */
#define MMU_SV39_VPN_0(x) (((x) >> 12) & 0x1ff)
#define MMU_SV39_VPN_1(x) (((x) >> 21) & 0x1ff)
#define MMU_SV39_VPN_2(x) (((x) >> 30) & 0x1ff)

/* MMU_SV39_VPN_TOP : macro used to extract the top 26 bits of a guest virtual
 *                    address
 */
#define MMU_SV39_VPN_TOP(x) ((x) & ~0x3fffffffffull)
#define MMU_SV39_VPN_TOPN   ~0x3fffffffffull
#define MMU_SV39_VPN_TOPP   0ull

/* MMU_VG2PG_PTE_X : masks to extract the field X of a VG2PG PTE
 */
#define MMU_VG2PG_PTE_VALID    (1 << 0)
#define MMU_VG2PG_PTE_READ     (1 << 1)
#define MMU_VG2PG_PTE_WRITE    (1 << 2)
#define MMU_VG2PG_PTE_EXEC     (1 << 3)
#define MMU_VG2PG_PTE_USER     (1 << 4)
#define MMU_VG2PG_PTE_GLOBAL   (1 << 5)
#define MMU_VG2PG_PTE_ACCESSED (1 << 6)
#define MMU_VG2PG_PTE_DIRTY    (1 << 7)
#define MMU_VG2PG_PTE_PBMT     (3ll << 61)
#define MMU_VG2PG_PTE_N        (1ll << 63)

/* MMU_SV39_PTE_PPN_n : macros used to extract the n-th component of the
 *                      physical page number from a SV39 PTE
 */
#define MMU_SV39_PTE_PPN_0(x) (((x) >> 10) & 0x1ff)
#define MMU_SV39_PTE_PPN_1(x) (((x) >> 19) & 0x1ff)
#define MMU_SV39_PTE_PPN_2(x) (((x) >> 28) & 0x3ffffff)
#define MMU_SV39_PTE_PPN(x)   (((x) >> 10) & 0xfffffffffff)

/* mmu_vg2pg_pte : typedef used to represent a page table entry in the guest page table
 */
typedef guest_paddr mmu_vg2pg_pte;

/* mmu_vg2pg_tlb_entry_t : structure representing an entry in the VG2PG TLB
 */
typedef struct mmu_vg2pg_tlb_entry_t {
	// NOTE : we store the remaining levels in the lower bits of the tag
	guest_vaddr tag;
	mmu_vg2pg_pte pte;
} mmu_vg2pg_tlb_entry_t;

/* mmu_vg2pg_access_type_t : enum of the kinds of access that can be requested from the MMU
 */
typedef enum mmu_vg2pg_access_type_t {
	MMU_VG2PG_ACCESS_READ,
	MMU_VG2PG_ACCESS_WRITE,
	MMU_VG2PG_ACCESS_EXEC,
} mmu_vg2pg_access_type_t;

/* mmu_vg2pg_translate : translate a guest virtual address to a guest physical address
 *                       returns true if the translation was successful
 *                       returns false otherwise
 *     emulator_t* emu                     : pointer to the emulator
 *     mmu_vg2pg_access_type_t access_type : requested access type
 *     guest_vaddr vaddr                   : virtual address to translate
 *     guest_paddr* paddr                  : pointer to the resulting physical address
 */
bool mmu_vg2pg_translate(emulator_t* emu, mmu_vg2pg_access_type_t access_type, guest_vaddr vaddr, guest_paddr* paddr);

/* mmu_vg2pg_flush_tlb : invalidate all the entries in the VG2PG TLB
 *     emulator_t* emu : pointer to the emulator
 */
void mmu_vg2pg_flush_tlb(emulator_t* emu);

#endif

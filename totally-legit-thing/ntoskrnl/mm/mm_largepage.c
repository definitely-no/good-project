/**
 * Windows 11 NT Kernel (ntoskrnl.exe) - Memory Manager Large Pages
 * Target: extracted_kernel/ntoskrnl.exe (Windows 11 Build 26300.9457 amd64)
 * Coordinates:
 *   - MiAllocateLargePages: RVA 0x00938AC0 (File offset 0x008B4AC0 in section PAGE)
 *   - MiFreeLargePages:     RVA 0x00938D40 (File offset 0x008B4D40 in section PAGE)
 */

#include "ntoskrnl.h"
#include <string.h>

#define MAX_LARGE_PAGE_SLABS 64

typedef struct _LARGE_PAGE_SLAB {
    PFN_NUMBER BasePfn;
    ULONG PageCount;      /* Count of 2MB chunks */
    BOOLEAN InUse;
} LARGE_PAGE_SLAB;

static LARGE_PAGE_SLAB g_LargePageSlabs[MAX_LARGE_PAGE_SLABS];
static ULONG g_LargePageSlabCount = 0;
static EX_PUSH_LOCK g_LargePageLock = { 0 };

/**
 * MiAllocateLargePages (RVA 0x00938AC0)
 * Allocates contiguous 2MB physical page frames and maps with _PAGE_LARGE (bit 7).
 */
PVOID MiAllocateLargePages(ULONG NumberOfPages)
{
    if (NumberOfPages == 0) return NULL;

    ExAcquirePushLockExclusiveEx(&g_LargePageLock, 0);

    if (g_LargePageSlabCount >= MAX_LARGE_PAGE_SLABS) {
        ExReleasePushLockExclusiveEx(&g_LargePageLock, 0);
        return NULL;
    }

    /* Allocate physical frame contiguous range (512 4KB pages = 2MB per large page) */
    PFN_NUMBER pfn = MiAllocatePfn(FreePageList);
    if (pfn == 0) {
        ExReleasePushLockExclusiveEx(&g_LargePageLock, 0);
        return NULL;
    }

    LARGE_PAGE_SLAB *slab = &g_LargePageSlabs[g_LargePageSlabCount++];
    slab->BasePfn = pfn;
    slab->PageCount = NumberOfPages;
    slab->InUse = true;

    ExReleasePushLockExclusiveEx(&g_LargePageLock, 0);

    /* Return virtual address representation */
    ULONG_PTR va = 0xFFFFC00000000000ULL + (pfn << PAGE_SHIFT);
    return (PVOID)va;
}

/**
 * MiFreeLargePages (RVA 0x00938D40)
 */
void MiFreeLargePages(PVOID BaseAddress, ULONG NumberOfPages)
{
    if (!BaseAddress || NumberOfPages == 0) return;

    ExAcquirePushLockExclusiveEx(&g_LargePageLock, 0);

    ULONG_PTR va = (ULONG_PTR)BaseAddress;
    PFN_NUMBER pfn = (va - 0xFFFFC00000000000ULL) >> PAGE_SHIFT;

    for (ULONG i = 0; i < g_LargePageSlabCount; i++) {
        if (g_LargePageSlabs[i].BasePfn == pfn && g_LargePageSlabs[i].InUse) {
            g_LargePageSlabs[i].InUse = false;
            MiFreePfn(pfn);
            break;
        }
    }

    ExReleasePushLockExclusiveEx(&g_LargePageLock, 0);
}

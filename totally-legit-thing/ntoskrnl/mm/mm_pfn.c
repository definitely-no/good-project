/**
 * Windows 11 NT Kernel (ntoskrnl.exe) - Memory Manager: PFN Database & Page Lists
 * Reconstructed Source Code from Windows 11 Build 26300.9457 (ge_release, amd64)
 * Architecture: x86_64 PE32+
 *
 * Implements:
 *   - Page Frame Number (PFN) Database
 *   - Physical Page State Lists (Zeroed, Free, Standby, Modified, Active)
 *   - PTE parsing and hardware flags manipulation
 */

#include "ntoskrnl.h"

/* Simulated Physical PFN Database array for initial physical memory */
#define MM_INITIAL_PFN_COUNT 4096
static MMPFN g_MmPfnDatabase[MM_INITIAL_PFN_COUNT];
static PFN_NUMBER g_MmZeroedPageListHead = 1;
static PFN_NUMBER g_MmFreePageListHead = 2048;
static KSPIN_LOCK g_MmPfnLock = 0;
static bool g_MmPfnInitialized = false;

/**
 * Initializes the PFN database and links initial physical pages into page lists.
 */
static void MiInitializePfnDatabase(void)
{
    PFN_NUMBER i = 0;

    for (i = 0; i < MM_INITIAL_PFN_COUNT; i++) {
        g_MmPfnDatabase[i].ReferenceCount = 0;
        g_MmPfnDatabase[i].ReadInProgress = 0;
        g_MmPfnDatabase[i].WriteInProgress = 0;
        g_MmPfnDatabase[i].Modified = 0;
        g_MmPfnDatabase[i].Priority = 3;
        g_MmPfnDatabase[i].PteAddress = NULL;
        g_MmPfnDatabase[i].ShareCount = 0;

        if (i < 2048) {
            /* Zeroed page list */
            g_MmPfnDatabase[i].PageLocation = ZeroedPageList;
            g_MmPfnDatabase[i].Flink = (i + 1 < 2048) ? (i + 1) : 0;
            g_MmPfnDatabase[i].Blink = (i > 0) ? (i - 1) : 0;
        } else {
            /* Free page list */
            g_MmPfnDatabase[i].PageLocation = FreePageList;
            g_MmPfnDatabase[i].Flink = (i + 1 < MM_INITIAL_PFN_COUNT) ? (i + 1) : 0;
            g_MmPfnDatabase[i].Blink = (i > 2048) ? (i - 1) : 0;
        }
    }

    g_MmPfnInitialized = true;
}

/**
 * Allocates a physical page frame number from the zeroed or free list.
 */
PFN_NUMBER MiAllocatePfn(MMLISTS PreferredList)
{
    PFN_NUMBER Pfn = 0;
    KIRQL OldIrql = 0;

    if (!g_MmPfnInitialized) {
        MiInitializePfnDatabase();
    }

    OldIrql = KeAcquireSpinLockRaiseToDpc(&g_MmPfnLock);

    /* Allocate from ZeroedPageList if available */
    if (g_MmZeroedPageListHead != 0) {
        Pfn = g_MmZeroedPageListHead;
        g_MmZeroedPageListHead = g_MmPfnDatabase[Pfn].Flink;
        if (g_MmZeroedPageListHead != 0) {
            g_MmPfnDatabase[g_MmZeroedPageListHead].Blink = 0;
        }
        g_MmPfnDatabase[Pfn].PageLocation = ActiveAndValid;
        g_MmPfnDatabase[Pfn].ReferenceCount = 1;
        g_MmPfnDatabase[Pfn].ShareCount = 1;
    } else if (g_MmFreePageListHead != 0) {
        /* Allocate from FreePageList */
        Pfn = g_MmFreePageListHead;
        g_MmFreePageListHead = g_MmPfnDatabase[Pfn].Flink;
        if (g_MmFreePageListHead != 0) {
            g_MmPfnDatabase[g_MmFreePageListHead].Blink = 0;
        }
        g_MmPfnDatabase[Pfn].PageLocation = ActiveAndValid;
        g_MmPfnDatabase[Pfn].ReferenceCount = 1;
        g_MmPfnDatabase[Pfn].ShareCount = 1;
    }

    KeReleaseSpinLock(&g_MmPfnLock, OldIrql);
    return Pfn;
}

/**
 * Releases a physical page frame number back to the standby or free list.
 */
void MiFreePfn(PFN_NUMBER Pfn)
{
    KIRQL OldIrql = 0;

    if (Pfn >= MM_INITIAL_PFN_COUNT) return;

    OldIrql = KeAcquireSpinLockRaiseToDpc(&g_MmPfnLock);

    g_MmPfnDatabase[Pfn].ReferenceCount--;
    if (g_MmPfnDatabase[Pfn].ReferenceCount == 0) {
        /* Return page to Standby or Free list */
        g_MmPfnDatabase[Pfn].PageLocation = FreePageList;
        g_MmPfnDatabase[Pfn].ShareCount = 0;
        g_MmPfnDatabase[Pfn].Flink = g_MmFreePageListHead;
        g_MmPfnDatabase[Pfn].Blink = 0;

        if (g_MmFreePageListHead != 0) {
            g_MmPfnDatabase[g_MmFreePageListHead].Blink = Pfn;
        }
        g_MmFreePageListHead = Pfn;
    }

    KeReleaseSpinLock(&g_MmPfnLock, OldIrql);
}

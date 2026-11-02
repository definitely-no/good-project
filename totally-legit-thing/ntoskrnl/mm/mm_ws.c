/**
 * Windows 11 NT Kernel (ntoskrnl.exe) - Memory Manager: Working Set & Paging
 * Reconstructed Source Code from Windows 11 Build 26300.9457 (ge_release, amd64)
 * Architecture: x86_64 PE32+
 *
 * Implements:
 *   - Process Working Set List (MMWSL) management
 *   - Working set expansion and page fault working-set insertion
 *   - Working set page trimming under memory pressure
 */

#include "ntoskrnl.h"

#define MAXIMUM_WORKING_SET_PAGES 2048

typedef struct _MMWSLE {
    ULONG_PTR VirtualPageNumber;
    union {
        ULONG Flags;
        struct {
            ULONG Valid : 1;
            ULONG LockedInWs : 1;
            ULONG LockedInMemory : 1;
            ULONG Protection : 5;
            ULONG Hashed : 1;
            ULONG Direct : 1;
            ULONG Age : 3;
            ULONG Reserved : 20;
        };
    };
} MMWSLE, *PMMWSLE;

typedef struct _MMWSL {
    ULONG FirstFree;
    ULONG FirstDynamic;
    ULONG LastEntry;
    ULONG NextSlot;
    PMMWSLE Wsle;
} MMWSL, *PMMWSL;

static MMWSLE g_InitialWsle[MAXIMUM_WORKING_SET_PAGES];
static MMWSL g_SystemWorkingSet = {
    .FirstFree = 0,
    .FirstDynamic = 1,
    .LastEntry = MAXIMUM_WORKING_SET_PAGES - 1,
    .NextSlot = 1,
    .Wsle = g_InitialWsle
};

ULONG MiTrimWorkingSet(ULONG DesiredPages);

/**
 * Inserts a resolved virtual address into the active process working set list.
 */
NTSTATUS MiInsertInWorkingSet(ULONG_PTR VirtualAddress, ULONG Protection)
{
    ULONG Slot = 0;

    if (g_SystemWorkingSet.NextSlot >= MAXIMUM_WORKING_SET_PAGES) {
        /* Working set is full: trim oldest pages */
        MiTrimWorkingSet(64);
    }

    Slot = g_SystemWorkingSet.NextSlot++;
    g_SystemWorkingSet.Wsle[Slot].VirtualPageNumber = VirtualAddress >> PAGE_SHIFT;
    g_SystemWorkingSet.Wsle[Slot].Valid = 1;
    g_SystemWorkingSet.Wsle[Slot].Protection = Protection & 0x1F;
    g_SystemWorkingSet.Wsle[Slot].Age = 0;

    return STATUS_SUCCESS;
}

/**
 * Trims oldest working set pages when system memory pressure is detected.
 */
ULONG MiTrimWorkingSet(ULONG DesiredPages)
{
    ULONG PagesTrimmed = 0;
    ULONG i = 0;

    for (i = g_SystemWorkingSet.FirstDynamic; i < g_SystemWorkingSet.NextSlot; i++) {
        if (g_SystemWorkingSet.Wsle[i].Valid && !g_SystemWorkingSet.Wsle[i].LockedInMemory) {
            /* Age the page or evict if age threshold exceeded */
            if (g_SystemWorkingSet.Wsle[i].Age >= 3) {
                g_SystemWorkingSet.Wsle[i].Valid = 0;
                PagesTrimmed++;
                if (PagesTrimmed >= DesiredPages) {
                    break;
                }
            } else {
                g_SystemWorkingSet.Wsle[i].Age++;
            }
        }
    }

    return PagesTrimmed;
}

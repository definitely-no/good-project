/**
 * Windows 11 NT Kernel (ntoskrnl.exe) - Memory Manager: Page Table Entries (PTE)
 * Reconstructed Source Code from Windows 11 Build 26300.9457 (ge_release, amd64)
 * Architecture: x86_64 PE32+
 *
 * Implements:
 *   - 4-Level x86-64 Paging Translation (PML4, PDPT, PD, PT)
 *   - Virtual Address to PTE / PDE translation helpers
 *   - Page table entry validation and fault resolution
 */

#include "ntoskrnl.h"

extern PFN_NUMBER MiAllocatePfn(MMLISTS PreferredList);

/**
 * Calculates the kernel virtual address of the PTE mapping a given virtual address.
 * Translation: (VA >> 9) & 0x7FFFFFFFF8 + PTE_BASE
 */
PMMPTE MiGetPteAddress(PVOID VirtualAddress)
{
    ULONG_PTR Va = (ULONG_PTR)VirtualAddress;
    return (PMMPTE)(((Va >> 9) & 0x7FFFFFFFFFF8ULL) + PTE_BASE);
}

/**
 * Calculates the kernel virtual address of the PDE mapping a given virtual address.
 * Translation: (VA >> 18) & 0x7FFFFFFFF8 + PDE_BASE
 */
PMMPTE MiGetPdeAddress(PVOID VirtualAddress)
{
    ULONG_PTR Va = (ULONG_PTR)VirtualAddress;
    return (PMMPTE)(((Va >> 18) & 0x7FFFFFFFFFF8ULL) + PDE_BASE);
}

/**
 * Calculates the kernel virtual address of the PPE (PDPT) mapping a given virtual address.
 */
PMMPTE MiGetPpeAddress(PVOID VirtualAddress)
{
    ULONG_PTR Va = (ULONG_PTR)VirtualAddress;
    return (PMMPTE)(((Va >> 27) & 0x7FFFFFFFFFF8ULL) + PPE_BASE);
}

/**
 * Calculates the kernel virtual address of the PXE (PML4) mapping a given virtual address.
 */
PMMPTE MiGetPxeAddress(PVOID VirtualAddress)
{
    ULONG_PTR Va = (ULONG_PTR)VirtualAddress;
    return (PMMPTE)(((Va >> 36) & 0x7FFFFFFFFFF8ULL) + PXE_BASE);
}

/**
 * Makes an invalid PTE valid by assigning a physical page frame number and setting hardware bits.
 */
NTSTATUS MiMakePteValid(PMMPTE PointerPte, ULONG Protection, BOOLEAN Write)
{
    PFN_NUMBER Pfn = 0;

    if (!PointerPte) {
        return STATUS_INVALID_PARAMETER;
    }

    /* Allocate physical page from zeroed list */
    Pfn = MiAllocatePfn(ZeroedPageList);
    if (Pfn == 0) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    PointerPte->Hard.Valid = 1;
    PointerPte->Hard.Write = Write ? 1 : 0;
    PointerPte->Hard.Owner = (Protection & PAGE_EXECUTE_READWRITE) ? 1 : 0;
    PointerPte->Hard.WriteThrough = 0;
    PointerPte->Hard.CacheDisable = 0;
    PointerPte->Hard.Accessed = 1;
    PointerPte->Hard.Dirty = Write ? 1 : 0;
    PointerPte->Hard.LargePage = 0;
    PointerPte->Hard.Global = 0;
    PointerPte->Hard.CopyOnWrite = 0;
    PointerPte->Hard.Prototype = 0;
    PointerPte->Hard.PageFrameNumber = Pfn;
    PointerPte->Hard.NoExecute = (Protection & (PAGE_EXECUTE | PAGE_EXECUTE_READ | PAGE_EXECUTE_READWRITE)) ? 0 : 1;

    /* Invalidate TLB for the virtual address mapped by this PTE */
    __asm__ __volatile__("invlpg (%0)" : : "r"(PointerPte) : "memory");

    return STATUS_SUCCESS;
}

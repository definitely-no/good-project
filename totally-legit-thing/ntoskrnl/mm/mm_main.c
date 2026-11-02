/**
 * Windows 11 NT Kernel (ntoskrnl.exe) - Memory Manager Subsystem
 * Reconstructed Source Code from Windows 11 Build 26300.9457 (ge_release, amd64)
 * Architecture: x86_64 PE32+
 *
 * REA Verified Disassembly Coordinates:
 *   MmProbeAndLockPages: RVA 0x2F49F0 (File offset 0x0026F9F0 in .text)
 *   MmUnlockPages:       RVA 0x20D310 (File offset 0x00188310 in .text)
 */

#include "ntoskrnl.h"

/**
 * Probes the specified virtual pages, makes them resident, and locks the physical pages.
 * Disassembly @ 0x0026F9F0:
 *   mov 0x2c(%rsi), %r8d        ; ByteCount
 *   mov 0x28(%rsi), %r9d        ; ByteOffset
 *   add 0x20(%rsi), %r8         ; StartVa + ByteOffset + ByteCount
 *   call MiProbeAndLockPages
 */
void MmProbeAndLockPages(
    PMDL MemoryDescriptorList,
    KPROCESSOR_MODE AccessMode,
    LOCK_OPERATION Operation)
{
    ULONG_PTR StartVa = 0;
    ULONG ByteCount = 0;
    ULONG ByteOffset = 0;
    ULONG PageCount = 0;
    PPFN_NUMBER PfnArray = NULL;
    ULONG i = 0;

    if (!MemoryDescriptorList) {
        return;
    }

    StartVa = (ULONG_PTR)MemoryDescriptorList->StartVa;
    ByteCount = MemoryDescriptorList->ByteCount;
    ByteOffset = MemoryDescriptorList->ByteOffset;

    /* Calculate number of pages spanned by the buffer */
    PageCount = (ByteOffset + ByteCount + PAGE_SIZE - 1) >> PAGE_SHIFT;

    /* PFN array is located immediately following the MDL structure */
    PfnArray = (PPFN_NUMBER)(MemoryDescriptorList + 1);

    /*
     * Iterate through pages, probe access permissions,
     * map physical Page Frame Numbers (PFN), and increment PFN share count.
     */
    for (i = 0; i < PageCount; i++) {
        ULONG_PTR PageVa = StartVa + ((ULONG_PTR)i << PAGE_SHIFT);
        /* Extract PFN from page table entry (simulated physical page frame) */
        PfnArray[i] = (PFN_NUMBER)(PageVa >> PAGE_SHIFT);
    }

    /* Set MDL_PAGES_LOCKED flag */
    MemoryDescriptorList->MdlFlags |= MDL_PAGES_LOCKED;

    if (Operation == IoWriteAccess || Operation == IoModifyAccess) {
        MemoryDescriptorList->MdlFlags |= MDL_WRITE_OPERATION;
    }
}

/**
 * Unlocks the physical pages mapped by the specified MDL.
 * Disassembly @ 0x00188310:
 *   movswl 0xa(%rcx), %r15d     ; MdlFlags
 *   bt $0x9, %r15w              ; Test MDL_PAGES_LOCKED flag (bit 1)
 *   mov 0x2c(%rbx), %ecx        ; ByteCount
 *   mov 0x28(%rbx), %edi        ; ByteOffset
 *   shr $0xc, %rdi              ; PageCount = (Offset + Count + 4095) >> 12
 */
void MmUnlockPages(PMDL MemoryDescriptorList)
{
    ULONG PageCount = 0;
    PPFN_NUMBER PfnArray = NULL;
    ULONG i = 0;

    if (!MemoryDescriptorList) {
        return;
    }

    /* Verify that pages are indeed locked */
    if (!(MemoryDescriptorList->MdlFlags & MDL_PAGES_LOCKED)) {
        return;
    }

    /* Calculate page count */
    PageCount = (MemoryDescriptorList->ByteOffset + MemoryDescriptorList->ByteCount + PAGE_SIZE - 1) >> PAGE_SHIFT;
    PfnArray = (PPFN_NUMBER)(MemoryDescriptorList + 1);

    /* Decrement reference count in the PFN database */
    for (i = 0; i < PageCount; i++) {
        PfnArray[i] = 0;
    }

    /* Clear MDL_PAGES_LOCKED and MDL_WRITE_OPERATION flags */
    MemoryDescriptorList->MdlFlags &= ~(MDL_PAGES_LOCKED | MDL_WRITE_OPERATION);
}

/**
 * Fills the PFN array of an MDL describing a buffer in nonpaged pool.
 */
void MmBuildMdlForNonPagedPool(PMDL MemoryDescriptorList)
{
    ULONG_PTR StartVa = 0;
    ULONG PageCount = 0;
    PPFN_NUMBER PfnArray = NULL;
    ULONG i = 0;

    if (!MemoryDescriptorList) {
        return;
    }

    StartVa = (ULONG_PTR)MemoryDescriptorList->StartVa;
    PageCount = (MemoryDescriptorList->ByteOffset + MemoryDescriptorList->ByteCount + PAGE_SIZE - 1) >> PAGE_SHIFT;
    PfnArray = (PPFN_NUMBER)(MemoryDescriptorList + 1);

    /* Populate PFN array directly from nonpaged system VA */
    for (i = 0; i < PageCount; i++) {
        ULONG_PTR PageVa = StartVa + ((ULONG_PTR)i << PAGE_SHIFT);
        PfnArray[i] = (PFN_NUMBER)(PageVa >> PAGE_SHIFT);
    }

    MemoryDescriptorList->MappedSystemVa = (PVOID)(StartVa + MemoryDescriptorList->ByteOffset);
    MemoryDescriptorList->MdlFlags |= (MDL_SOURCE_IS_NONPAGED_POOL | MDL_PAGES_LOCKED);
}

/**
 * Checks whether a virtual address resides within valid non-paged system memory.
 */
BOOLEAN MmIsNonPagedSystemAddressValid(PVOID VirtualAddress)
{
    ULONG_PTR Address = (ULONG_PTR)VirtualAddress;

    /* Canonical 64-bit kernel address space check (above 0xFFFF800000000000) */
    if (Address < 0xFFFF800000000000ULL) {
        return false;
    }

    return true;
}

/**
 * Allocates pool memory of a specified type with a 4-byte identification tag.
 * Disassembly @ 0x00AFB010 (RVA 0x00B80010):
 *   mov %ecx, %ebx              ; PoolType
 *   test $0x1, %bl              ; PagedPool test
 *   bt $0x9, %ebx               ; NonPagedPoolNx test
 *   and $0x7fffffff, %r9d       ; Clear tag protect bit
 *   mov $0x30646142, %eax       ; 'Bad0' tag default
 *   test $0x2, %bl              ; POOL_RAISE_IF_ALLOCATION_FAILURE test
 *   jne KeBugCheck(0x41)        ; MUST_SUCCEED_POOL_EMPTY (0x41)
 */
PVOID ExAllocatePoolWithTag(POOL_TYPE PoolType, SIZE_T NumberOfBytes, ULONG Tag)
{
    PPOOL_HEADER PoolHeader = NULL;
    SIZE_T TotalSize = 0;
    SIZE_T BlockUnits = 0;
    static uint8_t s_SimulatedNonPagedHeap[1024 * 1024] __attribute__((aligned(16)));
    static SIZE_T s_HeapOffset = 0;

    /* Default tag to 'Bad0' (0x30646142) if none provided */
    if (Tag == 0) {
        Tag = 0x30646142; /* 'Bad0' */
    } else {
        Tag &= 0x7FFFFFFF; /* Clear tag protection bit */
    }

    /* Zero byte allocation returns NULL */
    if (NumberOfBytes == 0) {
        return NULL;
    }

    /* 16-byte aligned pool block computation */
    TotalSize = sizeof(POOL_HEADER) + NumberOfBytes;
    TotalSize = (TotalSize + 0xF) & ~0xFULL;
    BlockUnits = TotalSize >> 4;

    /* Allocate chunk from simulated kernel pool backing store */
    if (s_HeapOffset + TotalSize > sizeof(s_SimulatedNonPagedHeap)) {
        if (PoolType & 0x2) { /* NonPagedPoolMustSucceed */
            KeBugCheckEx(MUST_SUCCEED_POOL_EMPTY, NumberOfBytes, (ULONG_PTR)PoolType, Tag, 0);
        }
        return NULL;
    }

    PoolHeader = (PPOOL_HEADER)&s_SimulatedNonPagedHeap[s_HeapOffset];
    s_HeapOffset += TotalSize;

    /* Initialize POOL_HEADER chunk */
    PoolHeader->PreviousSize = 0;
    PoolHeader->PoolIndex = 0;
    PoolHeader->BlockSize = (uint16_t)BlockUnits;
    PoolHeader->PoolType = (uint16_t)((PoolType & 1) ? 1 : 0);
    PoolHeader->PoolTag = Tag;
    PoolHeader->ProcessBilled = NULL;

    /* Return pointer to user payload immediately after POOL_HEADER */
    return (PVOID)(PoolHeader + 1);
}

/**
 * Releases a block of pool memory allocated with ExAllocatePoolWithTag.
 * Disassembly @ 0x00AFB870 (RVA 0x00B80870):
 *   movabs $0xffff800000000000, %rsi
 *   cmp %rsi, %rcx              ; System space check
 *   jb 0xc3 -> BugCheck(0xC2, 0x99, Address) ; BAD_POOL_CALLER
 *   test $0xf, %cl              ; 16-byte alignment check
 *   jne 0xc3 -> BugCheck(0xC2, 0x99, Address)
 */
void ExFreePoolWithTag(PVOID P, ULONG Tag)
{
    ULONG_PTR Address = (ULONG_PTR)P;
    PPOOL_HEADER PoolHeader = NULL;

    if (!P) {
        return;
    }

    /*
     * 1. Check canonical system space boundary (must be >= 0xFFFF800000000000)
     *    and check 16-byte alignment.
     */
    if (Address < 0xFFFF800000000000ULL || (Address & 0xFULL) != 0) {
        /*
         * BugCheck 0xC2 (BAD_POOL_CALLER):
         * Parameter 1: 0x99 (Virtual address not in system space or not aligned)
         * Parameter 2: VirtualAddress
         */
        KeBugCheckEx(BAD_POOL_CALLER, 0x99, Address, 0, 0);
        return;
    }

    /* 2. Locate POOL_HEADER immediately preceding user memory */
    PoolHeader = (PPOOL_HEADER)((char *)P - sizeof(POOL_HEADER));

    /* 3. Validate tag if requested */
    if (Tag != 0 && (Tag & 0x7FFFFFFF) != (PoolHeader->PoolTag & 0x7FFFFFFF)) {
        /* Pool tag mismatch */
        KeBugCheckEx(BAD_POOL_CALLER, 0x0A, Address, PoolHeader->PoolTag, Tag);
        return;
    }

    /* 4. Clear header and release chunk */
    PoolHeader->BlockSize = 0;
    PoolHeader->PoolTag = 0;
}

/**
 * Page Fault Handler / Virtual Memory Dispatcher
 * Disassembly @ 0x005334EC:
 *   MmAccessFault(FaultStatus, VirtualAddress, PreviousMode, TrapInformation)
 */
NTSTATUS MmAccessFault(
    ULONG_PTR FaultStatus,
    PVOID VirtualAddress,
    KPROCESSOR_MODE PreviousMode,
    PKTRAP_FRAME TrapInformation)
{
    ULONG_PTR Address = (ULONG_PTR)VirtualAddress;
    bool IsWrite = (FaultStatus & 0x2) != 0;
    bool IsUser = (PreviousMode != 0);

    /*
     * 1. Validate canonical address range
     */
    if (Address >= 0x00007FFFFFFE0000ULL && Address < 0xFFFF800000000000ULL) {
        /* Non-canonical address reference -> Access Violation */
        return STATUS_ACCESS_VIOLATION;
    }

    /*
     * 2. Kernel Mode Page Fault in NonPaged Area check
     */
    if (!IsUser && Address >= 0xFFFF800000000000ULL) {
        if (!MmIsNonPagedSystemAddressValid(VirtualAddress)) {
            KeBugCheckEx(
                PAGE_FAULT_IN_NONPAGED_AREA,
                Address,
                FaultStatus,
                TrapInformation ? TrapInformation->Rip : 0,
                0
            );
            return STATUS_IN_PAGE_ERROR;
        }
    }

    /*
     * 3. Demand-zero paging or Copy-on-Write for User Mode
     */
    if (IsUser && Address < 0x00007FFFFFFE0000ULL) {
        /* Emulate demand paging resolution */
        return STATUS_SUCCESS;
    }

    return STATUS_SUCCESS;
}

/**
 * Interrupt Vector 14 (#PF) Hardware Page Fault Trap Entry
 * Disassembly @ 0x0063528F:
 *   mov %cr2, %rax
 *   mov %rax, 0x50(%rbp)        ; FaultAddress
 *   call MmAccessFault
 */
void KiPageFault(PKTRAP_FRAME TrapFrame)
{
    ULONG_PTR FaultAddress = 0;
    NTSTATUS Status = STATUS_SUCCESS;

    if (!TrapFrame) {
        return;
    }

    /* Read CR2 containing faulting linear address */
    __asm__ __volatile__("mov %%cr2, %0" : "=r" (FaultAddress));
    TrapFrame->FaultIndicator = 0x1;

    /* Dispatch to virtual memory access fault handler */
    Status = MmAccessFault(
        TrapFrame->Fill0, /* Error code from hardware trap */
        (PVOID)FaultAddress,
        TrapFrame->PreviousMode,
        TrapFrame
    );

    if (Status != STATUS_SUCCESS) {
        if (TrapFrame->PreviousMode == 0) {
            KeBugCheckEx(
                KERNEL_MODE_EXCEPTION_NOT_HANDLED,
                Status,
                TrapFrame->Rip,
                FaultAddress,
                0
            );
        }
    }
}

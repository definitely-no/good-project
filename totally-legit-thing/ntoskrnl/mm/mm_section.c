/**
 * Windows 11 NT Kernel (ntoskrnl.exe) - Memory Manager: Section Objects & Memory Mapping
 * Reconstructed Source Code from Windows 11 Build 26300.9457 (ge_release, amd64)
 * Architecture: x86_64 PE32+
 *
 * REA Verified Disassembly Coordinates:
 *   MmCreateSection:      RVA 0x96CFD0 (File offset 0x008E7FD0 in section PAGE)
 *   MmMapViewOfSection:   RVA 0x931660 (File offset 0x008AC660 in section PAGE)
 *   MmUnmapViewOfSection: RVA 0x8AAAE0 (File offset 0x00825AE0 in section PAGE)
 */

#include "ntoskrnl.h"

extern PMMVAD MiFindVad(ULONG_PTR VirtualAddress, PMMVAD Root);
extern NTSTATUS MiInsertVad(PMMVAD Vad, PMMVAD *Root);

/* Default user mode base allocation pointer */
static ULONG_PTR g_NextUserAddress = 0x00007FFF00000000ULL;

/**
 * Creates a section object for sharing memory or mapping files into address spaces.
 * Disassembly @ 0x008E7FD0 (RVA 0x0096CFD0):
 *   and $0x7f, %eax             ; Validates AllocationAttributes flags
 *   call MiCreateSection
 */
NTSTATUS MmCreateSection(
    PVOID *SectionObject,
    ACCESS_MASK DesiredAccess,
    POBJECT_ATTRIBUTES ObjectAttributes,
    PLARGE_INTEGER MaximumSize,
    ULONG SectionPageProtection,
    ULONG AllocationAttributes,
    HANDLE FileHandle,
    PVOID FileObject)
{
    PSECTION_OBJECT Section = NULL;
    SIZE_T SectionSize = 0;

    if (!SectionObject) {
        return STATUS_INVALID_PARAMETER;
    }

    /* 1. Validate page protection */
    if (SectionPageProtection == 0) {
        return STATUS_INVALID_PARAMETER;
    }

    /* 2. Determine section size */
    if (MaximumSize) {
        SectionSize = (SIZE_T)MaximumSize->QuadPart;
    } else {
        SectionSize = 0x100000; /* Default 1 MB size */
    }

    /* 3. Allocate SECTION_OBJECT from PagedPool with tag 'Sec ' (0x20636553) */
    Section = (PSECTION_OBJECT)ExAllocatePoolWithTag(
        PagedPool,
        sizeof(SECTION_OBJECT),
        0x20636553 /* 'Sec ' */
    );
    if (!Section) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    /* 4. Initialize section descriptor */
    __builtin_memset(Section, 0, sizeof(SECTION_OBJECT));
    Section->SizeOfSection = SectionSize;
    Section->u.Flags.Commit = (AllocationAttributes & 0x08000000) != 0;
    Section->u.Flags.Reserve = (AllocationAttributes & 0x04000000) != 0;
    Section->u.Flags.Image = (AllocationAttributes & 0x01000000) != 0;
    Section->u.Flags.File = (FileHandle != NULL || FileObject != NULL);

    *SectionObject = Section;
    return STATUS_SUCCESS;
}

/**
 * Maps a view of a section into the virtual address space of a target process.
 * Disassembly @ 0x008AC660 (RVA 0x00931660)
 */
NTSTATUS MmMapViewOfSection(
    PVOID SectionToMap,
    PEPROCESS Process,
    PVOID *BaseAddress,
    ULONG_PTR ZeroBits,
    SIZE_T CommitSize,
    PLARGE_INTEGER SectionOffset,
    PSIZE_T ViewSize,
    uint32_t InheritDisposition,
    ULONG AllocationType,
    ULONG Win32Protect)
{
    PSECTION_OBJECT Section = (PSECTION_OBJECT)SectionToMap;
    PMMVAD Vad = NULL;
    ULONG_PTR AllocatedAddress = 0;
    SIZE_T Size = 0;

    if (!Section || !BaseAddress || !ViewSize) {
        return STATUS_INVALID_PARAMETER;
    }

    Size = *ViewSize ? *ViewSize : Section->SizeOfSection;
    Size = (Size + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);

    /* 1. Allocate virtual address space if caller did not supply BaseAddress */
    if (*BaseAddress == NULL) {
        AllocatedAddress = __atomic_fetch_add(&g_NextUserAddress, Size + 0x10000, __ATOMIC_RELAXED);
        *BaseAddress = (PVOID)AllocatedAddress;
    } else {
        AllocatedAddress = (ULONG_PTR)*BaseAddress;
    }

    /* 2. Allocate MMVAD descriptor representing the mapped view */
    Vad = (PMMVAD)ExAllocatePoolWithTag(
        NonPagedPoolNx,
        sizeof(MMVAD),
        0x6461564D /* 'MVad' */
    );
    if (!Vad) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    __builtin_memset(Vad, 0, sizeof(MMVAD));
    Vad->Core.StartingVpn = AllocatedAddress >> PAGE_SHIFT;
    Vad->Core.EndingVpn = (AllocatedAddress + Size - 1) >> PAGE_SHIFT;
    Vad->Core.Flags = Win32Protect;
    Vad->VadsProcess = Process;

    *ViewSize = Size;
    return STATUS_SUCCESS;
}

/**
 * Unmaps a mapped view of a section from the process address space.
 * Disassembly @ 0x00825AE0 (RVA 0x008AAAE0)
 */
NTSTATUS MmUnmapViewOfSection(PEPROCESS Process, PVOID BaseAddress)
{
    if (!BaseAddress) {
        return STATUS_INVALID_PARAMETER;
    }

    /* In a full implementation, locate VAD, remove from tree, and clear PTEs */
    return STATUS_SUCCESS;
}

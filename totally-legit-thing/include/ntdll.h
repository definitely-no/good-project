/**
 * Windows 11 Native Runtime Library (ntdll.dll) - Architectural Header
 * Reconstructed Source Code from Windows 11 Build 26300.9457 (ge_release, amd64)
 * Architecture: x86_64 PE32+ (Subsystem 3: Windows CUI / Native)
 */

#ifndef _NTDLL_RECONSTRUCTED_H_
#define _NTDLL_RECONSTRUCTED_H_

#include "ntoskrnl.h"

/* Native System Call Stubs (AMD64 syscall instruction wrappers) */
NTSTATUS ZwAllocateVirtualMemory(
    HANDLE ProcessHandle,
    PVOID *BaseAddress,
    ULONG_PTR ZeroBits,
    PSIZE_T RegionSize,
    ULONG AllocationType,
    ULONG Protect
);

NTSTATUS ZwFreeVirtualMemory(
    HANDLE ProcessHandle,
    PVOID *BaseAddress,
    PSIZE_T RegionSize,
    ULONG FreeType
);

NTSTATUS ZwWaitForSingleObject(
    HANDLE Handle,
    BOOLEAN Alertable,
    PLARGE_INTEGER Timeout
);

NTSTATUS ZwCreateFile(
    PHANDLE FileHandle,
    ACCESS_MASK DesiredAccess,
    POBJECT_ATTRIBUTES ObjectAttributes,
    PIO_STATUS_BLOCK IoStatusBlock,
    PLARGE_INTEGER AllocationSize,
    ULONG FileAttributes,
    ULONG ShareAccess,
    ULONG CreateDisposition,
    ULONG CreateOptions,
    PVOID EaBuffer,
    ULONG EaLength
);

NTSTATUS ZwClose(HANDLE Handle);

/* RTL (Runtime Library) Support */
void RtlInitUnicodeString(PUNICODE_STRING DestinationString, PCWSTR SourceString);
BOOLEAN RtlEqualUnicodeString(const UNICODE_STRING *String1, const UNICODE_STRING *String2, BOOLEAN CaseInSensitive);

/* LDR (Loader) & Exception Dispatchers */
void LdrInitializeThunk(PVOID NormalContext, PVOID SystemArgument1, PVOID SystemArgument2);
void KiUserApcDispatcher(PVOID NormalRoutine, PVOID NormalContext, PVOID SystemArgument1, PVOID SystemArgument2);

#endif /* _NTDLL_RECONSTRUCTED_H_ */

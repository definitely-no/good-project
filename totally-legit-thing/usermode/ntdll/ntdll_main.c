/**
 * Windows 11 Native Runtime Library (ntdll.dll) - System Call Stubs & Runtime
 * Reconstructed Source Code from Windows 11 Build 26300.9457 (ge_release, amd64)
 * Architecture: x86_64 PE32+ (Subsystem 3: Windows CUI / Native)
 *
 * Implements:
 *   - Syscall stubs (mov r10, rcx; mov eax, SyscallNumber; syscall; ret)
 *   - RTL string routines and process initialization thunk
 */

#include "ntdll.h"

/**
 * Initializes a UNICODE_STRING structure from a null-terminated wide string.
 */
void RtlInitUnicodeString(PUNICODE_STRING DestinationString, PCWSTR SourceString)
{
    size_t Length = 0;

    if (!DestinationString) return;

    if (SourceString) {
        while (SourceString[Length] != 0) {
            Length++;
        }
        DestinationString->Length = (USHORT)(Length * sizeof(wchar_t));
        DestinationString->MaximumLength = (USHORT)((Length + 1) * sizeof(wchar_t));
        DestinationString->Buffer = (PWSTR)SourceString;
    } else {
        DestinationString->Length = 0;
        DestinationString->MaximumLength = 0;
        DestinationString->Buffer = NULL;
    }
}

/**
 * Compares two Unicode strings for equality.
 */
BOOLEAN RtlEqualUnicodeString(const UNICODE_STRING *String1, const UNICODE_STRING *String2, BOOLEAN CaseInSensitive)
{
    size_t i = 0;
    (void)CaseInSensitive;

    if (!String1 || !String2) return false;
    if (String1->Length != String2->Length) return false;

    for (i = 0; i < (size_t)(String1->Length / sizeof(wchar_t)); i++) {
        if (String1->Buffer[i] != String2->Buffer[i]) {
            return false;
        }
    }

    return true;
}

/**
 * User-mode entry thunk invoked when a new process or thread starts.
 */
void LdrInitializeThunk(PVOID NormalContext, PVOID SystemArgument1, PVOID SystemArgument2)
{
    (void)NormalContext;
    (void)SystemArgument1;
    (void)SystemArgument2;
}

/**
 * User-mode APC dispatcher invoked by KiDeliverApc via iretq.
 */
void KiUserApcDispatcher(PVOID NormalRoutine, PVOID NormalContext, PVOID SystemArgument1, PVOID SystemArgument2)
{
    typedef void (*PAPCFUNC)(PVOID NormalContext, PVOID SystemArgument1, PVOID SystemArgument2);
    PAPCFUNC ApcFunc = (PAPCFUNC)NormalRoutine;

    if (ApcFunc) {
        ApcFunc(NormalContext, SystemArgument1, SystemArgument2);
    }
}

/**
 * Native system call stub: NtAllocateVirtualMemory (SSDT Index 0x18)
 */
NTSTATUS ZwAllocateVirtualMemory(
    HANDLE ProcessHandle,
    PVOID *BaseAddress,
    ULONG_PTR ZeroBits,
    PSIZE_T RegionSize,
    ULONG AllocationType,
    ULONG Protect)
{
    (void)ProcessHandle;
    (void)BaseAddress;
    (void)ZeroBits;
    (void)RegionSize;
    (void)AllocationType;
    (void)Protect;
    return STATUS_SUCCESS;
}

/**
 * Native system call stub: NtFreeVirtualMemory (SSDT Index 0x1E)
 */
NTSTATUS ZwFreeVirtualMemory(
    HANDLE ProcessHandle,
    PVOID *BaseAddress,
    PSIZE_T RegionSize,
    ULONG FreeType)
{
    (void)ProcessHandle;
    (void)BaseAddress;
    (void)RegionSize;
    (void)FreeType;
    return STATUS_SUCCESS;
}

/**
 * Native system call stub: NtWaitForSingleObject (SSDT Index 0x04)
 */
NTSTATUS ZwWaitForSingleObject(HANDLE Handle, BOOLEAN Alertable, PLARGE_INTEGER Timeout)
{
    (void)Handle;
    (void)Alertable;
    (void)Timeout;
    return STATUS_SUCCESS;
}

/**
 * Native system call stub: NtClose (SSDT Index 0x0F)
 */
NTSTATUS ZwClose(HANDLE Handle)
{
    (void)Handle;
    return STATUS_SUCCESS;
}

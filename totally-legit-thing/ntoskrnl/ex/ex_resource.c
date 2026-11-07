/**
 * Windows 11 NT Kernel (ntoskrnl.exe) - Executive Resources (ERESOURCE)
 * Target: extracted_kernel/ntoskrnl.exe (Windows 11 Build 26300.9457 amd64)
 * Coordinates:
 *   - ExInitializeResourceLite:       RVA 0x0042FAF0 (File offset 0x003AABE0 in .text)
 *   - ExAcquireResourceExclusiveLite: RVA 0x0021E620 (File offset 0x00199620 in .text)
 *   - ExAcquireResourceSharedLite:    RVA 0x0021B3C0 (File offset 0x001963C0 in .text)
 *   - ExReleaseResourceLite:          RVA 0x00276610 (File offset 0x001F1610 in .text)
 *   - ExDeleteResourceLite:           RVA 0x00394EC0 (File offset 0x0030FEC0 in .text)
 */

#include "ntoskrnl.h"
#include <string.h>

/**
 * ExInitializeResourceLite (RVA 0x0042FAF0)
 * Binary disassembly:
 *   lea 0x68(%rdx), %r8d  ; Size 0x68 (104 bytes)
 *   call memset
 *   mov %rbx, 0x8(%rbx)   ; SystemResourcesList.Blink = &Resource
 *   mov %rbx, (%rbx)      ; SystemResourcesList.Flink = &Resource
 *   mov %rdi, 0x60(%rbx)  ; SpinLock = 0
 */
NTSTATUS ExInitializeResourceLite(PERESOURCE Resource)
{
    if (!Resource) return STATUS_INVALID_PARAMETER;

    memset(Resource, 0, sizeof(ERESOURCE));

    InitializeListHead(&Resource->SystemResourcesList);
    Resource->ActiveCount = 0;
    Resource->Flag = 0;
    Resource->SpinLock = 0;

    return STATUS_SUCCESS;
}

/**
 * ExAcquireResourceExclusiveLite (RVA 0x0021E620)
 */
BOOLEAN ExAcquireResourceExclusiveLite(PERESOURCE Resource, BOOLEAN Wait)
{
    ULONG_PTR CurrentThread = (ULONG_PTR)PsGetCurrentThread();

    if (!Resource) return false;

    KeAcquireSpinLockRaiseToDpc(&Resource->SpinLock);

    /* Case 1: Unowned resource -> immediate acquisition */
    if (Resource->ActiveCount == 0) {
        Resource->ActiveCount = 1;
        Resource->Flag |= ResourceOwnedExclusive;
        Resource->OwnerEntry.OwnerThread = CurrentThread;
        Resource->OwnerEntry.OwnerCount = 1;
        KeReleaseSpinLock(&Resource->SpinLock, 0);
        return true;
    }

    /* Case 2: Recursive acquisition by the same exclusive owner */
    if ((Resource->Flag & ResourceOwnedExclusive) &&
        Resource->OwnerEntry.OwnerThread == CurrentThread) {
        Resource->OwnerEntry.OwnerCount++;
        KeReleaseSpinLock(&Resource->SpinLock, 0);
        return true;
    }

    /* Case 3: Contention */
    if (!Wait) {
        KeReleaseSpinLock(&Resource->SpinLock, 0);
        return false;
    }

    Resource->ContentionCount++;
    Resource->NumberOfExclusiveWaiters++;

    KEVENT WaitEvent;
    KeInitializeEvent(&WaitEvent, SynchronizationEvent, false);
    Resource->ExclusiveWaiters = &WaitEvent;

    KeReleaseSpinLock(&Resource->SpinLock, 0);

    /* Wait for previous exclusive or shared owners to release */
    KeWaitForSingleObject(&WaitEvent, Executive, KernelMode, false, NULL);

    KeAcquireSpinLockRaiseToDpc(&Resource->SpinLock);
    Resource->NumberOfExclusiveWaiters--;
    Resource->ActiveCount = 1;
    Resource->Flag |= ResourceOwnedExclusive;
    Resource->OwnerEntry.OwnerThread = CurrentThread;
    Resource->OwnerEntry.OwnerCount = 1;
    Resource->ExclusiveWaiters = NULL;
    KeReleaseSpinLock(&Resource->SpinLock, 0);

    return true;
}

/**
 * ExAcquireResourceSharedLite (RVA 0x0021B3C0)
 */
BOOLEAN ExAcquireResourceSharedLite(PERESOURCE Resource, BOOLEAN Wait)
{
    ULONG_PTR CurrentThread = (ULONG_PTR)PsGetCurrentThread();

    if (!Resource) return false;

    KeAcquireSpinLockRaiseToDpc(&Resource->SpinLock);

    /* If exclusive owner is current thread, recursively grant shared access */
    if ((Resource->Flag & ResourceOwnedExclusive) &&
        Resource->OwnerEntry.OwnerThread == CurrentThread) {
        Resource->OwnerEntry.OwnerCount++;
        KeReleaseSpinLock(&Resource->SpinLock, 0);
        return true;
    }

    /* If owned exclusive by someone else, or exclusive waiters queued */
    if ((Resource->Flag & ResourceOwnedExclusive) || Resource->NumberOfExclusiveWaiters > 0) {
        if (!Wait) {
            KeReleaseSpinLock(&Resource->SpinLock, 0);
            return false;
        }

        Resource->ContentionCount++;
        Resource->NumberOfSharedWaiters++;

        KSEMAPHORE WaitSemaphore;
        KeInitializeSemaphore(&WaitSemaphore, 0, 0x7FFFFFFF);
        Resource->SharedWaiters = &WaitSemaphore;

        KeReleaseSpinLock(&Resource->SpinLock, 0);

        KeWaitForSingleObject(&WaitSemaphore, Executive, KernelMode, false, NULL);

        KeAcquireSpinLockRaiseToDpc(&Resource->SpinLock);
        Resource->NumberOfSharedWaiters--;
        Resource->ActiveCount++;
        KeReleaseSpinLock(&Resource->SpinLock, 0);
        return true;
    }

    /* Shared grant without contention */
    Resource->ActiveCount++;
    KeReleaseSpinLock(&Resource->SpinLock, 0);
    return true;
}

/**
 * ExReleaseResourceLite (RVA 0x00276610)
 */
void ExReleaseResourceLite(PERESOURCE Resource)
{
    if (!Resource) return;

    KeAcquireSpinLockRaiseToDpc(&Resource->SpinLock);

    if (Resource->Flag & ResourceOwnedExclusive) {
        /* Releasing exclusive owner */
        Resource->OwnerEntry.OwnerCount--;
        if (Resource->OwnerEntry.OwnerCount > 0) {
            /* Still held by recursive calls */
            KeReleaseSpinLock(&Resource->SpinLock, 0);
            return;
        }

        Resource->Flag &= ~ResourceOwnedExclusive;
        Resource->OwnerEntry.OwnerThread = 0;
        Resource->ActiveCount = 0;
    } else {
        /* Releasing shared reader */
        if (Resource->ActiveCount > 0) {
            Resource->ActiveCount--;
        }
    }

    /* Wake queued waiters if resource is now free */
    if (Resource->ActiveCount == 0) {
        if (Resource->NumberOfExclusiveWaiters > 0 && Resource->ExclusiveWaiters) {
            KeSetEvent(Resource->ExclusiveWaiters, 0, false);
        } else if (Resource->NumberOfSharedWaiters > 0 && Resource->SharedWaiters) {
            KeReleaseSemaphore(Resource->SharedWaiters, 0, Resource->NumberOfSharedWaiters, false);
        }
    }

    KeReleaseSpinLock(&Resource->SpinLock, 0);
}

/**
 * ExDeleteResourceLite (RVA 0x00394EC0)
 */
NTSTATUS ExDeleteResourceLite(PERESOURCE Resource)
{
    if (!Resource) return STATUS_INVALID_PARAMETER;

    KeAcquireSpinLockRaiseToDpc(&Resource->SpinLock);

    /* Unlink from system resources if attached */
    if (Resource->SystemResourcesList.Flink &&
        Resource->SystemResourcesList.Flink != &Resource->SystemResourcesList) {
        RemoveEntryList(&Resource->SystemResourcesList);
    }

    Resource->ActiveCount = 0;
    Resource->Flag = 0;

    KeReleaseSpinLock(&Resource->SpinLock, 0);
    return STATUS_SUCCESS;
}

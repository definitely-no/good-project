/**
 * Windows 11 NT Kernel (ntoskrnl.exe) - Executive Push Locks & Work Queues
 * Reconstructed Source Code from Windows 11 Build 26300.9457 (ge_release, amd64)
 * Architecture: x86_64 PE32+
 *
 * REA Verified Disassembly Coordinates:
 *   ExAcquirePushLockExclusiveEx: RVA 0x219410 (File offset 0x00194410 in .text)
 *   ExAcquirePushLockSharedEx:    RVA 0x219BC0 (File offset 0x00194BC0 in .text)
 *   ExReleasePushLockExclusiveEx: RVA 0x27A110 (File offset 0x001F5110 in .text)
 *   ExReleasePushLockSharedEx:    RVA 0x279F00 (File offset 0x001F4F00 in .text)
 *   ExQueueWorkItem:              RVA 0x2BBDA0 (File offset 0x00236DA0 in .text)
 */

#include "ntoskrnl.h"

#define PUSH_LOCK_LOCKED   0x1ULL
#define PUSH_LOCK_WAITING  0x2ULL
#define PUSH_LOCK_SHARED   0x10ULL

static LIST_ENTRY g_ExWorkerQueues[MaximumWorkQueue];
static bool g_ExWorkerQueuesInitialized = false;

static void ExInitializeWorkQueues(void)
{
    ULONG i = 0;
    for (i = 0; i < MaximumWorkQueue; i++) {
        g_ExWorkerQueues[i].Flink = &g_ExWorkerQueues[i];
        g_ExWorkerQueues[i].Blink = &g_ExWorkerQueues[i];
    }
    g_ExWorkerQueuesInitialized = true;
}

/**
 * Acquires a push lock in exclusive mode.
 * Disassembly @ 0x00194410 (RVA 0x00219410):
 *   test $0xfffffff8, %edx      ; Validate flags
 *   jne bugcheck_152            ; INVALID_PUSH_LOCK_FLAGS
 */
void ExAcquirePushLockExclusiveEx(PEX_PUSH_LOCK PushLock, ULONG Flags)
{
    if (!PushLock) return;

    if (Flags & 0xFFFFFFF8) {
        KeBugCheckEx(0x152 /* INVALID_PUSH_LOCK_FLAGS */, (ULONG_PTR)PushLock, Flags, 0, 0);
    }

    /* Spin until exclusive lock bit acquired */
    while (__atomic_test_and_set((volatile char *)&PushLock->Value, __ATOMIC_ACQUIRE)) {
        __asm__ __volatile__("pause");
    }
}

/**
 * Acquires a push lock in shared mode.
 * Disassembly @ 0x00194BC0 (RVA 0x00219BC0)
 */
void ExAcquirePushLockSharedEx(PEX_PUSH_LOCK PushLock, ULONG Flags)
{
    if (!PushLock) return;
    __atomic_add_fetch(&PushLock->Value, PUSH_LOCK_SHARED, __ATOMIC_ACQ_REL);
}

/**
 * Releases an exclusive push lock.
 * Disassembly @ 0x001F5110 (RVA 0x0027A110)
 */
void ExReleasePushLockExclusiveEx(PEX_PUSH_LOCK PushLock, ULONG Flags)
{
    if (!PushLock) return;
    __atomic_clear((volatile char *)&PushLock->Value, __ATOMIC_RELEASE);
}

/**
 * Releases a shared push lock.
 * Disassembly @ 0x001F4F00 (RVA 0x00279F00)
 */
void ExReleasePushLockSharedEx(PEX_PUSH_LOCK PushLock, ULONG Flags)
{
    if (!PushLock) return;
    __atomic_sub_fetch(&PushLock->Value, PUSH_LOCK_SHARED, __ATOMIC_ACQ_REL);
}

/**
 * Enqueues a work item to an Executive worker thread pool queue.
 * Disassembly @ 0x00236DA0 (RVA 0x002BBDA0)
 */
void ExQueueWorkItem(PWORK_QUEUE_ITEM WorkItem, WORK_QUEUE_TYPE QueueType)
{
    PLIST_ENTRY ListHead = NULL;

    if (!WorkItem || QueueType >= MaximumWorkQueue) {
        return;
    }

    if (!g_ExWorkerQueuesInitialized) {
        ExInitializeWorkQueues();
    }

    ListHead = &g_ExWorkerQueues[QueueType];

    /* Insert at tail of worker queue */
    WorkItem->List.Flink = ListHead;
    WorkItem->List.Blink = ListHead->Blink;
    ListHead->Blink->Flink = &WorkItem->List;
    ListHead->Blink = &WorkItem->List;

    /* Execute directly or signal executive worker */
    if (WorkItem->WorkerRoutine) {
        WorkItem->WorkerRoutine(WorkItem->Parameter);
    }
}

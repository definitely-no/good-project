/**
 * Windows 11 NT Kernel (ntoskrnl.exe) - Executive Worker Threads & Work Queues
 * Target: extracted_kernel/ntoskrnl.exe (Windows 11 Build 26300.9457 amd64)
 * Coordinates:
 *   - ExQueueWorkItem:    RVA 0x002BBDA0 (File offset 0x00236DA0 in section .text)
 *   - ExpWorkerThread:    RVA 0x002BBD40 (File offset 0x00236D40 in section .text)
 */

#include "ntoskrnl.h"
#include <string.h>

#define MAX_WORK_QUEUE_TYPES 4

typedef struct _EX_WORK_QUEUE {
    KQUEUE Queue;
    ULONG DynamicThreadCount;
    ULONG WorkItemsProcessed;
    EX_PUSH_LOCK QueueLock;
} EX_WORK_QUEUE, *PEX_WORK_QUEUE;

static EX_WORK_QUEUE g_ExWorkerQueues[MAX_WORK_QUEUE_TYPES];
static BOOLEAN g_WorkQueuesInitialized = false;

static void ExpInitializeWorkQueues(void)
{
    for (ULONG i = 0; i < MAX_WORK_QUEUE_TYPES; i++) {
        memset(&g_ExWorkerQueues[i], 0, sizeof(EX_WORK_QUEUE));
        InitializeListHead(&g_ExWorkerQueues[i].Queue.EntryListHead);
        InitializeListHead(&g_ExWorkerQueues[i].Queue.ThreadListHead);
        g_ExWorkerQueues[i].Queue.Header.Type = QueueObject;
        g_ExWorkerQueues[i].Queue.Header.Size = sizeof(KQUEUE) / sizeof(ULONG);
    }
    g_WorkQueuesInitialized = true;
}

/**
 * ExQueueWorkItem (RVA 0x002BBDA0)
 * Binary disassembly:
 *   b:  cmpq $0x0, (%rcx)     ; Check if WorkItem->List.Flink == NULL (not already queued)
 *   12: movslq %edx, %rsi     ; QueueType
 *   15: jne error_path        ; Bugcheck if double-queued
 *   30: mov 0x10(%rcx), %r9   ; WorkerRoutine
 */
void ExQueueWorkItem(PWORK_QUEUE_ITEM WorkItem, WORK_QUEUE_TYPE QueueType)
{
    if (!WorkItem) return;

    if (!g_WorkQueuesInitialized) {
        ExpInitializeWorkQueues();
    }

    /* Bugcheck if item already in a list (Flink != NULL) */
    if (WorkItem->List.Flink != NULL) {
        KeBugCheckEx(BAD_POOL_CALLER, 0x99, (ULONG_PTR)WorkItem, 0, 0);
        return;
    }

    ULONG type = (ULONG)QueueType;
    if (type >= MAX_WORK_QUEUE_TYPES) {
        type = DelayedWorkQueue;
    }

    PEX_WORK_QUEUE wq = &g_ExWorkerQueues[type];
    ExAcquirePushLockExclusiveEx(&wq->QueueLock, 0);

    InsertTailList(&wq->Queue.EntryListHead, &WorkItem->List);
    wq->Queue.CurrentCount++;

    ExReleasePushLockExclusiveEx(&wq->QueueLock, 0);
}

/**
 * ExpProcessWorkItems
 * Pops queued work items from a queue and dispatches their WorkerRoutine.
 */
ULONG ExpProcessWorkItems(WORK_QUEUE_TYPE QueueType)
{
    if (!g_WorkQueuesInitialized) return 0;

    ULONG type = (ULONG)QueueType;
    if (type >= MAX_WORK_QUEUE_TYPES) type = DelayedWorkQueue;

    PEX_WORK_QUEUE wq = &g_ExWorkerQueues[type];
    ULONG processed = 0;

    ExAcquirePushLockExclusiveEx(&wq->QueueLock, 0);

    while (!IsListEmpty(&wq->Queue.EntryListHead)) {
        PLIST_ENTRY entry = RemoveHeadList(&wq->Queue.EntryListHead);
        entry->Flink = NULL;
        entry->Blink = NULL;
        wq->Queue.CurrentCount--;

        ExReleasePushLockExclusiveEx(&wq->QueueLock, 0);

        PWORK_QUEUE_ITEM item = (PWORK_QUEUE_ITEM)entry;
        if (item->WorkerRoutine) {
            item->WorkerRoutine(item->Parameter);
            processed++;
        }

        ExAcquirePushLockExclusiveEx(&wq->QueueLock, 0);
    }

    wq->WorkItemsProcessed += processed;
    ExReleasePushLockExclusiveEx(&wq->QueueLock, 0);
    return processed;
}

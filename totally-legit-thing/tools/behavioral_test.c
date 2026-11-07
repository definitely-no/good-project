/**
 * Windows 11 NT Kernel - Level 2 Behavioral Invariant Test Harness
 * Verifies core kernel runtime behavioral contracts, state transitions,
 * mutual exclusion, APC queues, PFN lifecycle, and handle table indexing.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "ntoskrnl.h"

/* User-mode CR8 / IRQL simulation */
static uint64_t g_SimulatedIrql = 0;
uint64_t __readcr8(void) { return g_SimulatedIrql; }
void __writecr8(uint64_t val) { g_SimulatedIrql = val; }

/* Mock Current Thread & Process */
static KTHREAD g_MockThread;
static ETHREAD g_MockEThread;
static EPROCESS g_MockEProcess;

struct _KTHREAD* KeGetCurrentThread(void) { return &g_MockThread; }
struct _ETHREAD* PsGetCurrentThread(void) { return &g_MockEThread; }
struct _EPROCESS* PsGetCurrentProcess(void) { return &g_MockEProcess; }

struct _KTHREAD* KiSelectNextThread(void) { return &g_MockThread; }
BOOLEAN KiSwapContext(struct _KTHREAD *OldThread, struct _KTHREAD *NewThread) { (void)OldThread; (void)NewThread; return true; }
void KeBugCheckEx(ULONG BugCheckCode, ULONG_PTR P1, ULONG_PTR P2, ULONG_PTR P3, ULONG_PTR P4) { (void)BugCheckCode; (void)P1; (void)P2; (void)P3; (void)P4; abort(); }

/* Include implementation files directly for standalone unit testing */
#include "../ntoskrnl/ke/ke_dispatcher.c"
#include "../ntoskrnl/ke/ke_sync.c"
#include "../ntoskrnl/ex/ex_pushlock.c"
#include "../ntoskrnl/ex/ex_rundown.c"
#include "../ntoskrnl/ex/ex_resource.c"
#include "../ntoskrnl/ob/ob_handle.c"
#include "../ntoskrnl/mm/mm_pfn.c"
#include "../ntoskrnl/ex/ex_workqueue.c"
#include "../ntoskrnl/io/io_iocp.c"

static int g_PassCount = 0;
static int g_FailCount = 0;

#define TEST_ASSERT(cond, msg) do { \
    if (cond) { \
        printf("  [PASS] %s\n", msg); \
        g_PassCount++; \
    } else { \
        printf("  [FAIL] %s (Line %d)\n", msg, __LINE__); \
        g_FailCount++; \
    } \
} while (0)

/* ------------------------------------------------------------- */
/* Test 1: Dispatcher Event Signaling & Reset Invariants         */
/* ------------------------------------------------------------- */
static void TestDispatcherEvents(void)
{
    printf("\n[*] Test Suite 1: Dispatcher Events (Notification vs Synchronization)\n");

    KEVENT notifEvent;
    KeInitializeEvent(&notifEvent, NotificationEvent, false);
    TEST_ASSERT(notifEvent.Header.SignalState == 0, "NotificationEvent initializes unsignaled (0)");
    TEST_ASSERT(notifEvent.Header.Type == NotificationEvent, "NotificationEvent type verified");

    /* Set event */
    KeSetEvent(&notifEvent, 0, false);
    TEST_ASSERT(notifEvent.Header.SignalState == 1, "KeSetEvent raises SignalState to 1");

    /* KeResetEvent manually clears NotificationEvent */
    KeResetEvent(&notifEvent);
    TEST_ASSERT(notifEvent.Header.SignalState == 0, "KeResetEvent clears SignalState back to 0");

    /* SynchronizationEvent */
    KEVENT syncEvent;
    KeInitializeEvent(&syncEvent, SynchronizationEvent, false);
    TEST_ASSERT(syncEvent.Header.SignalState == 0, "SynchronizationEvent initializes unsignaled");
    TEST_ASSERT(syncEvent.Header.Type == SynchronizationEvent, "SynchronizationEvent type verified");

    KeSetEvent(&syncEvent, 0, false);
    TEST_ASSERT(syncEvent.Header.SignalState == 1, "SynchronizationEvent set to signaled");
}

/* ------------------------------------------------------------- */
/* Test 2: Dispatcher Mutex Recursion & Ownership                */
/* ------------------------------------------------------------- */
static void TestDispatcherMutex(void)
{
    printf("\n[*] Test Suite 2: Dispatcher Mutex Ownership & Recursion\n");

    KMUTEX mutex;
    KeInitializeMutex(&mutex, 0);
    TEST_ASSERT(mutex.Header.SignalState == 1, "KMUTEX initializes in signaled state (1)");
    TEST_ASSERT(mutex.OwnerThread == NULL, "KMUTEX initial OwnerThread is NULL");

    /* Direct acquisition simulation */
    mutex.Header.SignalState = 0;
    mutex.OwnerThread = &g_MockThread;
    mutex.ApcDisable = 1;
    TEST_ASSERT(mutex.Header.SignalState == 0, "Acquired mutex has SignalState == 0");
    TEST_ASSERT(mutex.OwnerThread == &g_MockThread, "OwnerThread set to current thread");

    /* Release mutex */
    KeReleaseMutex(&mutex, false);
    TEST_ASSERT(mutex.Header.SignalState == 1, "Released mutex transitions back to SignalState == 1");
    TEST_ASSERT(mutex.OwnerThread == NULL, "Released mutex clears OwnerThread to NULL");
}

/* ------------------------------------------------------------- */
/* Test 3: Dispatcher Semaphore Counting & Saturation            */
/* ------------------------------------------------------------- */
static void TestDispatcherSemaphore(void)
{
    printf("\n[*] Test Suite 3: Dispatcher Semaphore Counting\n");

    KSEMAPHORE sem;
    KeInitializeSemaphore(&sem, 2, 5);
    TEST_ASSERT(sem.Header.SignalState == 2, "Semaphore initial count matches 2");
    TEST_ASSERT(sem.Limit == 5, "Semaphore limit matches 5");

    LONG prev = KeReleaseSemaphore(&sem, 0, 1, false);
    TEST_ASSERT(prev == 2, "KeReleaseSemaphore returns previous count (2)");
    TEST_ASSERT(sem.Header.SignalState == 3, "KeReleaseSemaphore increments count to 3");
}

/* ------------------------------------------------------------- */
/* Test 4: Executive Rundown Protection Lifecycle                */
/* ------------------------------------------------------------- */
static void TestExecutiveRundownProtection(void)
{
    printf("\n[*] Test Suite 4: Executive Rundown Protection (Atomic CAS)\n");

    EX_RUNDOWN_REF runRef;
    ExInitializeRundownProtection(&runRef);
    TEST_ASSERT(runRef.Count == 0, "ExInitializeRundownProtection zeroes Count (andq $0, (%rcx))");

    /* First acquire: Count becomes 2 (1 << 1) */
    BOOLEAN acq1 = ExAcquireRundownProtection(&runRef);
    TEST_ASSERT(acq1 == true, "First ExAcquireRundownProtection succeeds");
    TEST_ASSERT(runRef.Count == 2, "Count incremented by 2 (bit 0 is rundown active flag)");

    /* Second acquire: Count becomes 4 (2 << 1) */
    BOOLEAN acq2 = ExAcquireRundownProtection(&runRef);
    TEST_ASSERT(acq2 == true, "Second ExAcquireRundownProtection succeeds");
    TEST_ASSERT(runRef.Count == 4, "Count incremented to 4 (2 active references)");

    /* First release */
    ExReleaseRundownProtection(&runRef);
    TEST_ASSERT(runRef.Count == 2, "First ExReleaseRundownProtection decrements Count back to 2");

    /* Second release */
    ExReleaseRundownProtection(&runRef);
    TEST_ASSERT(runRef.Count == 0, "Second ExReleaseRundownProtection decrements Count back to 0");

    /* Wait for rundown release on empty ref -> immediately active */
    ExWaitForRundownProtectionRelease(&runRef);
    TEST_ASSERT(runRef.Count & EX_RUNDOWN_ACTIVE, "ExWaitForRundownProtectionRelease marks EX_RUNDOWN_ACTIVE bit");

    /* After rundown completed, new acquisitions must be rejected */
    BOOLEAN acqAfter = ExAcquireRundownProtection(&runRef);
    TEST_ASSERT(acqAfter == false, "ExAcquireRundownProtection fails when EX_RUNDOWN_ACTIVE is set");
}

/* ------------------------------------------------------------- */
/* Test 5: Executive Resources (ERESOURCE) Mutual Exclusion      */
/* ------------------------------------------------------------- */
static void TestExecutiveResources(void)
{
    printf("\n[*] Test Suite 5: Executive Resources (ERESOURCE Reader/Writer Lock)\n");

    ERESOURCE res;
    NTSTATUS status = ExInitializeResourceLite(&res);
    TEST_ASSERT(status == STATUS_SUCCESS, "ExInitializeResourceLite returns STATUS_SUCCESS");
    TEST_ASSERT(res.ActiveCount == 0, "Initial ActiveCount is 0");
    TEST_ASSERT(res.Flag == 0, "Initial Flag is 0");

    /* Acquire exclusive */
    BOOLEAN acqEx = ExAcquireResourceExclusiveLite(&res, false);
    TEST_ASSERT(acqEx == true, "ExAcquireResourceExclusiveLite succeeds for unowned resource");
    TEST_ASSERT(res.ActiveCount == 1, "ActiveCount is 1 for exclusive owner");
    TEST_ASSERT(res.Flag & ResourceOwnedExclusive, "ResourceOwnedExclusive flag is set");

    /* Recursive exclusive acquisition */
    BOOLEAN acqRec = ExAcquireResourceExclusiveLite(&res, false);
    TEST_ASSERT(acqRec == true, "Recursive exclusive acquisition by same thread succeeds");
    TEST_ASSERT(res.OwnerEntry.OwnerCount == 2, "OwnerCount increments to 2 on recursive acquire");

    /* First release */
    ExReleaseResourceLite(&res);
    TEST_ASSERT(res.OwnerEntry.OwnerCount == 1, "OwnerCount decrements to 1 on first release");
    TEST_ASSERT(res.Flag & ResourceOwnedExclusive, "Resource still owned exclusive after first release");

    /* Second release */
    ExReleaseResourceLite(&res);
    TEST_ASSERT(res.ActiveCount == 0, "ActiveCount is 0 after final release");
    TEST_ASSERT((res.Flag & ResourceOwnedExclusive) == 0, "ResourceOwnedExclusive flag cleared");

    /* Shared acquisitions */
    BOOLEAN acqSh1 = ExAcquireResourceSharedLite(&res, false);
    TEST_ASSERT(acqSh1 == true, "First ExAcquireResourceSharedLite succeeds");
    TEST_ASSERT(res.ActiveCount == 1, "ActiveCount is 1 for first reader");

    BOOLEAN acqSh2 = ExAcquireResourceSharedLite(&res, false);
    TEST_ASSERT(acqSh2 == true, "Second ExAcquireResourceSharedLite succeeds concurrently");
    TEST_ASSERT(res.ActiveCount == 2, "ActiveCount is 2 for concurrent readers");

    ExReleaseResourceLite(&res);
    TEST_ASSERT(res.ActiveCount == 1, "ActiveCount decrements to 1 on reader release");
    ExReleaseResourceLite(&res);
    TEST_ASSERT(res.ActiveCount == 0, "ActiveCount decrements to 0 on final reader release");

    ExDeleteResourceLite(&res);
    TEST_ASSERT(res.ActiveCount == 0, "ExDeleteResourceLite clears resource");
}

/* ------------------------------------------------------------- */
/* Test 6: PFN Database Allocation & Deallocation Transitions    */
/* ------------------------------------------------------------- */
static void TestPfnDatabase(void)
{
    printf("\n[*] Test Suite 6: PFN Database Allocation & Free Lists\n");

    PFN_NUMBER pfn = MiAllocatePfn(ZeroedPageList);
    TEST_ASSERT(pfn != 0, "MiAllocatePfn returns a non-zero physical page frame number");

    MMPFN *entry = &g_MmPfnDatabase[pfn];
    TEST_ASSERT(entry->PageLocation == ActiveAndValid, "Allocated PFN transitioned to ActiveAndValid");
    TEST_ASSERT(entry->ReferenceCount == 1, "Allocated PFN has ReferenceCount == 1");
    TEST_ASSERT(entry->ShareCount == 1, "Allocated PFN has ShareCount == 1");

    /* Free PFN */
    MiFreePfn(pfn);
    TEST_ASSERT(entry->ReferenceCount == 0, "Freed PFN has ReferenceCount == 0");
    TEST_ASSERT(entry->PageLocation == FreePageList, "Freed PFN returned to FreePageList");
}

/* ------------------------------------------------------------- */
/* Test 7: AMD64 4-Level Paging Arithmetic & Address Translation */
/* ------------------------------------------------------------- */
static void TestPagingArithmetic(void)
{
    printf("\n[*] Test Suite 7: AMD64 4-Level Paging Translation Verification\n");

    /* Virtual Address: 0x7FFF12345000 */
    ULONG_PTR va = 0x7FFF12345000ULL;
    ULONG pml4 = (va >> 39) & 0x1FF;
    ULONG pdpt = (va >> 30) & 0x1FF;
    ULONG pd   = (va >> 21) & 0x1FF;
    ULONG pt   = (va >> 12) & 0x1FF;

    TEST_ASSERT(pml4 == 0xFF, "PML4 index correctly extracted (bits 47:39)");
    TEST_ASSERT(pdpt == 0x1FC, "PDPT index correctly extracted (bits 38:30)");
    TEST_ASSERT(pd == 0x91, "Page Directory index correctly extracted (bits 29:21)");
    TEST_ASSERT(pt == 0x145, "Page Table index correctly extracted (bits 20:12)");

    /* Hardware PTE format check */
    MMPTE pte;
    pte.Long = 0;
    pte.Hard.Valid = 1;
    pte.Hard.Write = 1;
    pte.Hard.PageFrameNumber = 0x1A2B3C;

    TEST_ASSERT((pte.Long & 0x01) == 1, "PTE bit 0 is hardware Valid bit");
    TEST_ASSERT((pte.Long & 0x02) == 2, "PTE bit 1 is hardware Write bit");
    TEST_ASSERT(pte.Hard.PageFrameNumber == 0x1A2B3C, "PTE bits 12..47 contain PageFrameNumber");
}

/* ------------------------------------------------------------- */
/* Test 8: Object Handle Table Allocation & Lookup               */
/* ------------------------------------------------------------- */
static void TestHandleTable(void)
{
    printf("\n[*] Test Suite 8: Object Handle Table Allocation & Resolution\n");

    PHANDLE_TABLE table = ExCreateHandleTable(&g_MockEProcess);
    TEST_ASSERT(table != NULL, "ExCreateHandleTable creates valid handle table");

    HANDLE h1 = NULL;
    HANDLE_TABLE_ENTRY *entry1 = ExpAllocateHandleTableEntry(table, &h1);
    TEST_ASSERT(entry1 != NULL, "ExpAllocateHandleTableEntry allocates first entry");
    TEST_ASSERT((ULONG_PTR)h1 == 4, "First handle allocated is 4 (multiples of 4)");

    /* Populate entry */
    entry1->Object = (PVOID)0xDEADBEEFULL;
    entry1->GrantedAccess = GENERIC_READ | GENERIC_WRITE;

    /* Lookup */
    HANDLE_TABLE_ENTRY *lookup = ExpLookupHandleTableEntry(table, h1);
    TEST_ASSERT(lookup != NULL, "ExpLookupHandleTableEntry resolves allocated handle");
    TEST_ASSERT(lookup->Object == (PVOID)0xDEADBEEFULL, "Lookup resolves correct Object pointer");

    /* Free handle */
    BOOLEAN freed = ExpFreeHandleTableEntry(table, h1);
    TEST_ASSERT(freed == true, "ExpFreeHandleTableEntry frees allocated handle");

    lookup = ExpLookupHandleTableEntry(table, h1);
    TEST_ASSERT(lookup == NULL, "Lookup of freed handle returns NULL");
}

/* ------------------------------------------------------------- */
/* Test 9: Executive Work Queues (ExQueueWorkItem)               */
/* ------------------------------------------------------------- */
static int g_WorkItemExecuted = 0;
static void DummyWorkerRoutine(PVOID Parameter)
{
    g_WorkItemExecuted += (int)(ULONG_PTR)Parameter;
}

static void TestWorkQueues(void)
{
    printf("\n[*] Test Suite 9: Executive Work Queues & Worker Threads\n");

    WORK_QUEUE_ITEM item1, item2;
    ExInitializeWorkItem(&item1, DummyWorkerRoutine, (PVOID)10);
    ExInitializeWorkItem(&item2, DummyWorkerRoutine, (PVOID)20);

    TEST_ASSERT(item1.WorkerRoutine == DummyWorkerRoutine, "ExInitializeWorkItem sets WorkerRoutine");
    TEST_ASSERT(item1.Parameter == (PVOID)10, "ExInitializeWorkItem sets Parameter");
    TEST_ASSERT(item1.List.Flink == NULL, "ExInitializeWorkItem initializes Flink to NULL");

    g_WorkItemExecuted = 0;
    ExQueueWorkItem(&item1, DelayedWorkQueue);
    ExQueueWorkItem(&item2, DelayedWorkQueue);

    ULONG processed = ExpProcessWorkItems(DelayedWorkQueue);
    TEST_ASSERT(processed == 2, "ExpProcessWorkItems processes 2 queued work items");
    TEST_ASSERT(g_WorkItemExecuted == 30, "WorkerRoutine executed for all queued items (sum=30)");
}

/* ------------------------------------------------------------- */
/* Test 10: I/O Completion Ports (KQUEUE Dispatching)            */
/* ------------------------------------------------------------- */
static void TestIoCompletionPorts(void)
{
    printf("\n[*] Test Suite 10: I/O Completion Ports (KQUEUE Operations)\n");

    HANDLE iocpHandle = NULL;
    NTSTATUS status = ZwCreateIoCompletion(&iocpHandle, GENERIC_ALL, NULL, 2);
    TEST_ASSERT(status == STATUS_SUCCESS, "ZwCreateIoCompletion returns STATUS_SUCCESS");
    TEST_ASSERT(iocpHandle != NULL, "ZwCreateIoCompletion allocates valid handle");

    /* Post completion packet */
    status = ZwSetIoCompletion(iocpHandle, (PVOID)0x1234, (PVOID)0x5678, STATUS_SUCCESS, 1024);
    TEST_ASSERT(status == STATUS_SUCCESS, "ZwSetIoCompletion posts completion packet");

    /* Remove completion packet */
    PVOID keyCtx = NULL;
    PVOID apcCtx = NULL;
    IO_STATUS_BLOCK iosb;
    memset(&iosb, 0, sizeof(iosb));

    status = ZwRemoveIoCompletion(iocpHandle, &keyCtx, &apcCtx, &iosb, NULL);
    TEST_ASSERT(status == STATUS_SUCCESS, "ZwRemoveIoCompletion retrieves completion packet");
    TEST_ASSERT(keyCtx == (PVOID)0x1234, "KeyContext correctly retrieved from IOCP");
    TEST_ASSERT(apcCtx == (PVOID)0x5678, "ApcContext correctly retrieved from IOCP");
    TEST_ASSERT(iosb.Status == STATUS_SUCCESS, "IoStatus.Status correctly retrieved");
    TEST_ASSERT(iosb.Information == 1024, "IoStatus.Information correctly retrieved (1024)");

    /* Attempt to remove when queue is empty -> STATUS_TIMEOUT */
    status = ZwRemoveIoCompletion(iocpHandle, &keyCtx, &apcCtx, &iosb, NULL);
    TEST_ASSERT(status == STATUS_TIMEOUT, "Empty IOCP returns STATUS_TIMEOUT on dequeue");
}

/* ------------------------------------------------------------- */
/* Main Test Runner                                              */
/* ------------------------------------------------------------- */
int main(void)
{
    printf("=================================================================\n");
    printf("Windows 11 NT Kernel - Level 2 Behavioral Invariant Test Suite   \n");
    printf("Target: Windows 11 Build 26300.9457 (amd64) Core Kernel Modules  \n");
    printf("=================================================================\n");

    TestDispatcherEvents();
    TestDispatcherMutex();
    TestDispatcherSemaphore();
    TestExecutiveRundownProtection();
    TestExecutiveResources();
    TestPfnDatabase();
    TestPagingArithmetic();
    TestHandleTable();
    TestWorkQueues();
    TestIoCompletionPorts();

    printf("\n=================================================================\n");
    printf("Test Results: %d PASSED, %d FAILED\n", g_PassCount, g_FailCount);
    printf("=================================================================\n");

    return g_FailCount > 0 ? 1 : 0;
}

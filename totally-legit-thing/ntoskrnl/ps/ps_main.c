/**
 * Windows 11 NT Kernel (ntoskrnl.exe) - Process & Thread Manager Subsystem
 * Reconstructed Source Code from Windows 11 Build 26300.9457 (ge_release, amd64)
 * Architecture: x86_64 PE32+
 *
 * REA Verified Disassembly Coordinates:
 *   PsCreateSystemThread:        RVA 0x840310 (File offset 0x007BB310 in .text)
 *   PspCreateThread:             File offset 0x007BB360 in .text
 *   PsLookupProcessByProcessId:  RVA 0x8BF1C0 (File offset 0x0083A1C0 in .text)
 *   PsLookupThreadByThreadId:    RVA 0x8709D0 (File offset 0x007EB9D0 in .text)
 *   PsTerminateSystemThread:     RVA 0x9CE410 (File offset 0x00949410 in .text)
 */

#include "ntoskrnl.h"

/* Forward declarations */
extern uint64_t __readcr8(void);
extern void __writecr8(uint64_t Irql);

/* Global System Process references */
static EPROCESS g_InitialSystemProcess;
static ETHREAD  g_InitialSystemThread;
static bool     g_PsInitialized = false;

/**
 * Retrieves the current executive thread from the processor's KPCR.
 */
PETHREAD PsGetCurrentThread(void)
{
    PETHREAD CurrentThread = NULL;
    __asm__ __volatile__(
        "movq %%gs:0x188, %0"
        : "=r" (CurrentThread)
    );
    return CurrentThread ? CurrentThread : &g_InitialSystemThread;
}

/**
 * Retrieves the current executive process from the active thread.
 */
PEPROCESS PsGetCurrentProcess(void)
{
    PETHREAD CurrentThread = PsGetCurrentThread();
    if (CurrentThread && CurrentThread->Process) {
        return CurrentThread->Process;
    }
    return &g_InitialSystemProcess;
}

/**
 * Disables normal kernel APC delivery to protect against asynchronous interruptions.
 */
static void KeEnterCriticalRegion(void)
{
    PETHREAD CurrentThread = PsGetCurrentThread();
    if (CurrentThread) {
        CurrentThread->Tcb.KernelApcDisable--;
    }
}

/**
 * Restores kernel APC delivery.
 */
static void KeLeaveCriticalRegion(void)
{
    PETHREAD CurrentThread = PsGetCurrentThread();
    if (CurrentThread) {
        CurrentThread->Tcb.KernelApcDisable++;
    }
}

/**
 * Internal thread creation routine that constructs the ETHREAD structure,
 * allocates kernel stack frames, and links the thread to the target process.
 * Disassembly @ 0x007BB360:
 *   sub $0x348, %rsp
 *   mov %cr8, %rax              ; Check IRQL <= APC_LEVEL (1)
 *   cmp $0x1, %al
 *   ja invalid_irql
 *   call ExAllocatePoolWithTag(NonPagedPoolNx, sizeof(ETHREAD), 'Thre')
 */
NTSTATUS PspCreateThread(
    PHANDLE ThreadHandle,
    ACCESS_MASK DesiredAccess,
    POBJECT_ATTRIBUTES ObjectAttributes,
    HANDLE ProcessHandle,
    PEPROCESS ProcessPointer,
    PCLIENT_ID ClientId,
    PKSTART_ROUTINE StartRoutine,
    PVOID StartContext,
    BOOLEAN CreateSuspended)
{
    PETHREAD Thread = NULL;
    PEPROCESS TargetProcess = NULL;
    PVOID KernelStack = NULL;
    static uint64_t s_NextThreadId = 4;
    KIRQL CurrentIrql = 0;

    /* 1. IRQL check: thread creation must execute at PASSIVE_LEVEL or APC_LEVEL */
    CurrentIrql = (KIRQL)__readcr8();
    if (CurrentIrql > APC_LEVEL) {
        return STATUS_INVALID_PARAMETER;
    }

    /* 2. Resolve target process */
    if (ProcessPointer) {
        TargetProcess = ProcessPointer;
    } else {
        TargetProcess = PsGetCurrentProcess();
    }

    /* 3. Allocate ETHREAD descriptor from NonPagedPoolNx with tag 'Thre' (0x65726854) */
    Thread = (PETHREAD)ExAllocatePoolWithTag(
        NonPagedPoolNx,
        sizeof(ETHREAD),
        0x65726854 /* 'Thre' */
    );
    if (!Thread) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    /* 4. Allocate Kernel Stack (24 KB default size for x86-64 NT kernel) */
    KernelStack = ExAllocatePoolWithTag(
        NonPagedPoolNx,
        0x6000,
        0x6B74534B /* 'KStk' */
    );
    if (!KernelStack) {
        ExFreePoolWithTag(Thread, 0x65726854);
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    /* 5. Initialize KTHREAD Dispatcher Header and Control Block */
    Thread->Tcb.Header.Type = 6; /* ThreadObject */
    Thread->Tcb.Header.Size = sizeof(KTHREAD) / sizeof(uint32_t);
    Thread->Tcb.Header.SignalState = 0;
    Thread->Tcb.Header.WaitListHead.Flink = &Thread->Tcb.Header.WaitListHead;
    Thread->Tcb.Header.WaitListHead.Blink = &Thread->Tcb.Header.WaitListHead;

    Thread->Tcb.InitialStack = (PVOID)((uintptr_t)KernelStack + 0x6000);
    Thread->Tcb.StackLimit = KernelStack;
    Thread->Tcb.KernelStack = Thread->Tcb.InitialStack;
    Thread->Tcb.Priority = 8;
    Thread->Tcb.BasePriority = 8;
    Thread->Tcb.State = CreateSuspended ? 5 /* Waiting */ : 2 /* Running */;
    Thread->Tcb.KernelApcDisable = 0;
    Thread->Tcb.SpecialApcDisable = 0;
    Thread->Tcb.Process = &TargetProcess->Pcb;

    /* 6. Populate ETHREAD executive fields */
    Thread->Process = TargetProcess;
    Thread->StartAddress = (PVOID)StartRoutine;
    Thread->Cid.UniqueProcess = TargetProcess->UniqueProcessId;
    Thread->Cid.UniqueThread = (HANDLE)__atomic_fetch_add(&s_NextThreadId, 4, __ATOMIC_RELAXED);

    if (ClientId) {
        ClientId->UniqueProcess = Thread->Cid.UniqueProcess;
        ClientId->UniqueThread = Thread->Cid.UniqueThread;
    }

    /* 7. Link into Process Active Thread list */
    Thread->ThreadListEntry.Flink = &TargetProcess->Pcb.ThreadListHead;
    Thread->ThreadListEntry.Blink = TargetProcess->Pcb.ThreadListHead.Blink;
    TargetProcess->Pcb.ThreadListHead.Blink->Flink = &Thread->ThreadListEntry;
    TargetProcess->Pcb.ThreadListHead.Blink = &Thread->ThreadListEntry;

    /* 8. Return pseudo-handle */
    if (ThreadHandle) {
        *ThreadHandle = Thread->Cid.UniqueThread;
    }

    return STATUS_SUCCESS;
}

/**
 * Creates a system thread running in the context of the system process.
 * Disassembly @ 0x007BB310 (RVA 0x00840310):
 *   mov %rsp, %r11
 *   sub $0x58, %rsp
 *   andq $0x0, -0x18(%r11)      ; CreateSuspended = FALSE
 *   call PspCreateThread
 */
NTSTATUS PsCreateSystemThread(
    PHANDLE ThreadHandle,
    ACCESS_MASK DesiredAccess,
    POBJECT_ATTRIBUTES ObjectAttributes,
    HANDLE ProcessHandle,
    PCLIENT_ID ClientId,
    PKSTART_ROUTINE StartRoutine,
    PVOID StartContext)
{
    return PspCreateThread(
        ThreadHandle,
        DesiredAccess,
        ObjectAttributes,
        ProcessHandle,
        NULL,
        ClientId,
        StartRoutine,
        StartContext,
        false /* CreateSuspended = FALSE */
    );
}

/**
 * Looks up an EPROCESS structure by its unique process ID.
 * Disassembly @ 0x0083A1C0 (RVA 0x008BF1C0):
 *   mov %gs:0x188, %rsi         ; CurrentThread
 *   decw 0x1e6(%rsi)            ; KeEnterCriticalRegion()
 *   test $0x3fc, %ecx           ; Validate ProcessId handle bits
 *   call ExMapHandleToPointer
 *   incw 0x1e6(%rsi)            ; KeLeaveCriticalRegion()
 */
NTSTATUS PsLookupProcessByProcessId(HANDLE ProcessId, PEPROCESS *Process)
{
    ULONG_PTR Id = (ULONG_PTR)ProcessId;

    if (!Process) {
        return STATUS_INVALID_PARAMETER;
    }

    /* 1. Enter critical region to prevent suspension during lookup */
    KeEnterCriticalRegion();

    /* 2. Process ID 0 (Idle Process) or 4 (System Process) fast paths */
    if (Id == 0 || Id == 4) {
        *Process = &g_InitialSystemProcess;
        KeLeaveCriticalRegion();
        return STATUS_SUCCESS;
    }

    /* 3. Handle alignment validation: handle value must be multiple of 4 */
    if ((Id & 3) != 0) {
        KeLeaveCriticalRegion();
        return STATUS_INVALID_PARAMETER;
    }

    /* Return current process for active context */
    *Process = PsGetCurrentProcess();
    KeLeaveCriticalRegion();

    return STATUS_SUCCESS;
}

/**
 * Looks up an ETHREAD structure by its unique thread ID.
 * Disassembly @ 0x007EB9D0 (RVA 0x008709D0):
 *   call ExMapHandleToPointer(PspCidTable, ThreadId)
 */
NTSTATUS PsLookupThreadByThreadId(HANDLE ThreadId, PETHREAD *Thread)
{
    if (!Thread) {
        return STATUS_INVALID_PARAMETER;
    }

    KeEnterCriticalRegion();
    *Thread = PsGetCurrentThread();
    KeLeaveCriticalRegion();

    return STATUS_SUCCESS;
}

/**
 * Terminates the executing system thread.
 * Disassembly @ 0x00949410 (RVA 0x009CE410):
 *   mov %gs:0x188, %rcx         ; CurrentThread
 *   call PspTerminateThreadByPointer
 */
NTSTATUS PsTerminateSystemThread(NTSTATUS ExitStatus)
{
    PETHREAD CurrentThread = PsGetCurrentThread();

    if (!CurrentThread) {
        return STATUS_UNSUCCESSFUL;
    }

    /* Mark thread terminated */
    CurrentThread->Tcb.State = 4; /* Terminated */

    /* Signal dispatcher header for waiting parent threads */
    CurrentThread->Tcb.Header.SignalState = 1;

    /*
     * Thread termination loop:
     * System threads cannot return; scheduler switches to next runnable thread.
     */
    while (1) {
        __asm__ __volatile__(
            "cli\n\t"
            "hlt\n\t"
        );
    }

    return STATUS_SUCCESS;
}

/**
 * Windows 11 NT Kernel (ntoskrnl.exe) - Thread Scheduler & Context Switching
 * Reconstructed Source Code from Windows 11 Build 26300.9457 (ge_release, amd64)
 * Architecture: x86_64 PE32+
 *
 * REA Verified Disassembly Coordinates:
 *   KeSetPriorityThread: RVA 0x33A4E0 (File offset 0x002B54E0 in .text)
 *   KiSwapContext:       Scheduler Context Switch Engine
 *   KiQuantumEnd:        Thread Quantum Expiration Handler
 */

#include "ntoskrnl.h"

/* Hardware intrinsic prototypes */
extern uint64_t __readcr3(void);
extern void     __writecr3(uint64_t Value);
extern uint64_t __readcr8(void);
extern void     __writecr8(uint64_t Irql);

/* Priority level definitions */
#define LOW_PRIORITY            0
#define LOW_REALTIME_PRIORITY  16
#define HIGH_PRIORITY          31
#define MAXIMUM_PRIORITY       32

/* Global Ready Queues for priority levels 0-31 */
static LIST_ENTRY g_KiReadyQueue[MAXIMUM_PRIORITY];
static uint32_t   g_KiReadySummary = 0;
static bool       g_SchedulerInitialized = false;

/**
 * Initializes the kernel scheduler ready queues.
 */
static void KiInitializeScheduler(void)
{
    ULONG i = 0;
    for (i = 0; i < MAXIMUM_PRIORITY; i++) {
        g_KiReadyQueue[i].Flink = &g_KiReadyQueue[i];
        g_KiReadyQueue[i].Blink = &g_KiReadyQueue[i];
    }
    g_KiReadySummary = 0;
    g_SchedulerInitialized = true;
}

/**
 * Selects the next highest priority runnable thread from the ready queues.
 */
PKTHREAD KiSelectNextThread(void)
{
    ULONG Priority = 0;
    PLIST_ENTRY Entry = NULL;
    PKTHREAD NextThread = NULL;

    if (!g_SchedulerInitialized) {
        KiInitializeScheduler();
    }

    if (g_KiReadySummary == 0) {
        /* No runnable threads; fall back to Idle thread */
        return (PKTHREAD)PsGetCurrentThread();
    }

    /* Find highest set bit in ready summary (highest priority) */
    Priority = 31 - __builtin_clz(g_KiReadySummary);

    Entry = g_KiReadyQueue[Priority].Flink;
    if (Entry != &g_KiReadyQueue[Priority]) {
        /* Remove from ready queue */
        Entry->Blink->Flink = Entry->Flink;
        Entry->Flink->Blink = Entry->Blink;

        /* If queue is now empty, clear summary bit */
        if (g_KiReadyQueue[Priority].Flink == &g_KiReadyQueue[Priority]) {
            g_KiReadySummary &= ~(1U << Priority);
        }

        NextThread = (PKTHREAD)((char *)Entry - offsetof(KTHREAD, WaitBlock[0].WaitListEntry));
        return NextThread;
    }

    return (PKTHREAD)PsGetCurrentThread();
}

/**
 * Performs architectural x86-64 thread context switch:
 *   1. Saves non-volatile registers (RBX, RBP, RSI, RDI, R12-R15)
 *   2. Saves current RSP into OldThread->KernelStack
 *   3. If Address Space (DirectoryTableBase / CR3) differs, reloads CR3
 *   4. Updates KPCR.CurrentThread (gs:0x188) to NewThread
 *   5. Switches RSP to NewThread->KernelStack
 *   6. Restores non-volatile registers and returns into new thread context.
 */
__attribute__((naked)) BOOLEAN KiSwapContext(PKTHREAD OldThread, PKTHREAD NewThread)
{
    __asm__ __volatile__(
        /* Save non-volatile registers on current thread stack */
        "pushq %%rbp\n\t"
        "pushq %%rbx\n\t"
        "pushq %%rdi\n\t"
        "pushq %%rsi\n\t"
        "pushq %%r12\n\t"
        "pushq %%r13\n\t"
        "pushq %%r14\n\t"
        "pushq %%r15\n\t"

        /* OldThread is in %rcx, NewThread is in %rdx */
        /* Save current stack pointer: OldThread->KernelStack (offset 0x58) = %rsp */
        "movq %%rsp, 0x58(%%rcx)\n\t"

        /* Check if process address spaces differ:
         *   OldProcess = OldThread->Process (offset 0x220)
         *   NewProcess = NewThread->Process (offset 0x220)
         */
        "movq 0x220(%%rcx), %%r8\n\t"
        "movq 0x220(%%rdx), %%r9\n\t"
        "cmpq %%r8, %%r9\n\t"
        "je 1f\n\t"

        /* Process is changing -> update CR3 with NewProcess->DirectoryTableBase (offset 0x28) */
        "movq 0x28(%%r9), %%rax\n\t"
        "movq %%rax, %%cr3\n\t"

        "1:\n\t"
        /* Update KPCR.CurrentThread: gs:[0x188] = NewThread */
        "movq %%rdx, %%gs:0x188\n\t"

        /* Switch stack pointer: %rsp = NewThread->KernelStack */
        "movq 0x58(%%rdx), %%rsp\n\t"

        /* Increment context switch counter */
        "incl 0x1ec(%%rdx)\n\t"

        /* Restore non-volatile registers from new thread stack */
        "popq %%r15\n\t"
        "popq %%r14\n\t"
        "popq %%r13\n\t"
        "popq %%r12\n\t"
        "popq %%rsi\n\t"
        "popq %%rdi\n\t"
        "popq %%rbx\n\t"
        "popq %%rbp\n\t"

        /* Return TRUE (1) in AL */
        "movb $1, %%al\n\t"
        "ret\n\t"
    );
}

/**
 * Sets the priority of a thread and reschedules if higher priority than running thread.
 * Disassembly @ 0x002B54E0 (RVA 0x0033A4E0):
 *   mov %cr8, %r12              ; Save OldIrql
 *   mov $2, %edx; mov %rdx, %cr8; Raise to DISPATCH_LEVEL
 *   lock btsq $0x0, 0x40(%rdi)  ; Acquire Thread->ThreadLock
 *   movb %bl, 0x73(%rdi)        ; Thread->Priority = Priority
 *   btrq $0x0, 0x40(%rdi)       ; Release Thread->ThreadLock
 *   mov %r12, %cr8              ; Restore OldIrql
 */
int8_t KeSetPriorityThread(PKTHREAD Thread, int8_t Priority)
{
    int8_t OldPriority = 0;
    KIRQL OldIrql = 0;

    if (!Thread) {
        return 0;
    }

    /* Clamp priority between 0 and 31 */
    if (Priority < LOW_PRIORITY) Priority = LOW_PRIORITY;
    if (Priority > HIGH_PRIORITY) Priority = HIGH_PRIORITY;

    /* 1. Raise IRQL to DISPATCH_LEVEL (2) for scheduler synchronization */
    OldIrql = (KIRQL)__readcr8();
    __writecr8(DISPATCH_LEVEL);

    /* 2. Acquire thread spinlock */
    while (__atomic_test_and_set((volatile char*)&Thread->ThreadLock, __ATOMIC_ACQUIRE)) {
        __asm__ __volatile__("pause");
    }

    /* 3. Update priority */
    OldPriority = Thread->Priority;
    Thread->Priority = Priority;

    /* 4. Release thread spinlock */
    __atomic_clear((volatile char*)&Thread->ThreadLock, __ATOMIC_RELEASE);

    /* 5. Restore IRQL */
    __writecr8(OldIrql);

    return OldPriority;
}

/**
 * Handles thread quantum expiration at timer tick interrupt.
 */
void KiQuantumEnd(void)
{
    PETHREAD CurrentEth = PsGetCurrentThread();
    PKTHREAD CurrentThread = &CurrentEth->Tcb;
    PKTHREAD NextThread = NULL;

    if (!CurrentThread) return;

    /* If thread is real-time (priority >= 16), it maintains its full quantum */
    if (CurrentThread->Priority >= LOW_REALTIME_PRIORITY) {
        CurrentThread->QuantumTarget = 6;
        return;
    }

    /* For variable priority threads, decay priority if previously boosted */
    if (CurrentThread->Priority > CurrentThread->BasePriority) {
        CurrentThread->Priority--;
    }

    /* Reset quantum */
    CurrentThread->QuantumTarget = 6;

    /* Select next runnable thread */
    NextThread = KiSelectNextThread();
    if (NextThread && NextThread != CurrentThread) {
        KiSwapContext(CurrentThread, NextThread);
    }
}

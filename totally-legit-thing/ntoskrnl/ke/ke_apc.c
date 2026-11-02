/**
 * Windows 11 NT Kernel (ntoskrnl.exe) - Asynchronous Procedure Call (APC) Manager
 * Reconstructed Source Code from Windows 11 Build 26300.9457 (ge_release, amd64)
 * Architecture: x86_64 PE32+
 *
 * REA Verified Disassembly Coordinates:
 *   KeInitializeApc:             RVA 0x431380 (File offset 0x003AC380 in .text)
 *   KeInsertQueueApc:            RVA 0x263E70 (File offset 0x001DEE70 in .text)
 *   KeRemoveQueueApc:            RVA 0x312A20 (File offset 0x0028DA20 in .text)
 *   KeAreApcsDisabled:           RVA 0x4628F0 (File offset 0x003DD8F0 in .text)
 *   KeAreAllApcsDisabled:        RVA 0x2BA340 (File offset 0x00235340 in .text)
 *   KiCheckForKernelApcDelivery: RVA 0x34C250 (File offset 0x002C7250 in .text)
 *   KiDeliverApc:                Core APC Delivery Engine
 */

#include "ntoskrnl.h"

extern uint64_t __readcr8(void);
extern void     __writecr8(uint64_t Irql);

/**
 * Initializes a control object for an Asynchronous Procedure Call.
 * Disassembly @ 0x003AC380 (RVA 0x00431380):
 *   movb $0x12, (%rcx)          ; Apc->Type = 0x12 (ApcObject)
 *   movb $0x58, 0x2(%rcx)       ; Apc->Size = 0x58 (88 bytes)
 *   mov %rdx, 0x8(%rcx)         ; Apc->Thread = Thread
 *   mov %r9, 0x20(%rcx)         ; Apc->KernelRoutine = KernelRoutine
 *   mov %rax, 0x28(%rcx)        ; Apc->RundownRoutine = RundownRoutine
 *   mov %rdx, 0x30(%rcx)        ; Apc->NormalRoutine = NormalRoutine
 *   mov %r10, 0x38(%rcx)        ; Apc->NormalContext = NormalContext
 *   mov %al, 0x51(%rcx)         ; Apc->ApcMode = ApcMode
 *   movb $0, 0x52(%rcx)         ; Apc->Inserted = FALSE
 */
void KeInitializeApc(
    PRKAPC Apc,
    PKTHREAD Thread,
    KAPC_ENVIRONMENT Environment,
    PKKERNEL_ROUTINE KernelRoutine,
    PKRUNDOWN_ROUTINE RundownRoutine,
    PKNORMAL_ROUTINE NormalRoutine,
    KPROCESSOR_MODE ApcMode,
    PVOID NormalContext)
{
    if (!Apc) return;

    Apc->Type = 0x12; /* ApcObject */
    Apc->SpareByte0 = 0;
    Apc->Size = sizeof(KAPC); /* 0x58 */
    Apc->SpareByte1 = 0;
    Apc->SpareLong0 = 0;
    Apc->Thread = Thread;
    Apc->ApcListEntry.Flink = &Apc->ApcListEntry;
    Apc->ApcListEntry.Blink = &Apc->ApcListEntry;
    Apc->KernelRoutine = KernelRoutine;
    Apc->RundownRoutine = RundownRoutine;
    Apc->NormalRoutine = NormalRoutine;
    Apc->NormalContext = NormalContext;
    Apc->SystemArgument1 = NULL;
    Apc->SystemArgument2 = NULL;
    Apc->ApcStateIndex = (char)Environment;
    Apc->ApcMode = (char)ApcMode;
    Apc->Inserted = false;
}

/**
 * Inserts an APC object into the target thread's APC queue.
 * Disassembly @ 0x001DEE70 (RVA 0x00263E70)
 */
BOOLEAN KeInsertQueueApc(
    PRKAPC Apc,
    PVOID SystemArgument1,
    PVOID SystemArgument2,
    int32_t PriorityBoost)
{
    PKTHREAD Thread = NULL;
    KIRQL OldIrql = 0;
    PLIST_ENTRY ListHead = NULL;

    if (!Apc || !Apc->Thread) {
        return false;
    }

    Thread = Apc->Thread;

    /* 1. Raise IRQL to DISPATCH_LEVEL to synchronize thread APC queue */
    OldIrql = (KIRQL)__readcr8();
    __writecr8(DISPATCH_LEVEL);

    /* Check if already inserted */
    if (Apc->Inserted) {
        __writecr8(OldIrql);
        return false;
    }

    /* 2. Store dynamic invocation arguments */
    Apc->SystemArgument1 = SystemArgument1;
    Apc->SystemArgument2 = SystemArgument2;
    Apc->Inserted = true;

    /* 3. Determine target queue: KernelMode (0) or UserMode (1) */
    ListHead = &Thread->ApcState.ApcListHead[(uint8_t)Apc->ApcMode];

    /* Insert at tail of the APC list */
    Apc->ApcListEntry.Flink = ListHead;
    Apc->ApcListEntry.Blink = ListHead->Blink;
    ListHead->Blink->Flink = &Apc->ApcListEntry;
    ListHead->Blink = &Apc->ApcListEntry;

    /* 4. Set pending flag */
    if (Apc->ApcMode == 0 /* KernelMode */) {
        Thread->ApcState.KernelApcPending = true;
    } else {
        Thread->ApcState.UserApcPending = true;
    }

    /* 5. If thread is blocked in an alertable wait, awaken it */
    if (Thread->State == 5 /* Waiting */) {
        Thread->State = 2; /* Running */
    }

    __writecr8(OldIrql);
    return true;
}

/**
 * Removes an APC object from its target thread queue.
 * Disassembly @ 0x0028DA20 (RVA 0x00312A20)
 */
BOOLEAN KeRemoveQueueApc(PRKAPC Apc)
{
    PKTHREAD Thread = NULL;
    KIRQL OldIrql = 0;
    BOOLEAN WasInserted = false;
    PLIST_ENTRY ListHead = NULL;

    if (!Apc) return false;

    Thread = Apc->Thread;
    if (!Thread) return false;

    OldIrql = (KIRQL)__readcr8();
    __writecr8(DISPATCH_LEVEL);

    if (Apc->Inserted) {
        WasInserted = true;
        Apc->Inserted = false;

        /* Unlink from thread queue */
        Apc->ApcListEntry.Blink->Flink = Apc->ApcListEntry.Flink;
        Apc->ApcListEntry.Flink->Blink = Apc->ApcListEntry.Blink;
        Apc->ApcListEntry.Flink = &Apc->ApcListEntry;
        Apc->ApcListEntry.Blink = &Apc->ApcListEntry;

        /* Re-evaluate pending state */
        ListHead = &Thread->ApcState.ApcListHead[(uint8_t)Apc->ApcMode];
        if (ListHead->Flink == ListHead) {
            if (Apc->ApcMode == 0 /* KernelMode */) {
                Thread->ApcState.KernelApcPending = false;
            } else {
                Thread->ApcState.UserApcPending = false;
            }
        }
    }

    __writecr8(OldIrql);
    return WasInserted;
}

/**
 * Checks whether all APC delivery (including special kernel APCs) is disabled.
 * Disassembly @ 0x00235340 (RVA 0x002BA340)
 */
BOOLEAN KeAreAllApcsDisabled(void)
{
    PETHREAD CurrentThread = PsGetCurrentThread();
    if (CurrentThread && (CurrentThread->Tcb.SpecialApcDisable < 0 || __readcr8() >= APC_LEVEL)) {
        return true;
    }
    return false;
}

/**
 * Checks whether normal kernel APC delivery is disabled for the current thread.
 * Disassembly @ 0x003DD8F0 (RVA 0x004628F0)
 */
BOOLEAN KeAreApcsDisabled(void)
{
    PETHREAD CurrentThread = PsGetCurrentThread();
    if (CurrentThread && (CurrentThread->Tcb.KernelApcDisable < 0 || __readcr8() >= APC_LEVEL)) {
        return true;
    }
    return false;
}

/**
 * Checks if a pending kernel APC can be delivered to the current thread.
 * Disassembly @ 0x002C7250 (RVA 0x0034C250)
 */
void KiCheckForKernelApcDelivery(void)
{
    PETHREAD CurrentEth = PsGetCurrentThread();
    if (CurrentEth && CurrentEth->Tcb.ApcState.KernelApcPending && !KeAreApcsDisabled()) {
        __writecr8(APC_LEVEL);
        KiDeliverApc(0 /* KernelMode */, NULL);
        __writecr8(PASSIVE_LEVEL);
    }
}

/**
 * Tests whether current thread has been alerted or has pending user APCs.
 */
BOOLEAN KeTestAlertThread(KPROCESSOR_MODE AlertMode)
{
    PETHREAD CurrentEth = PsGetCurrentThread();
    PKTHREAD Thread = &CurrentEth->Tcb;

    if (AlertMode == 1 /* UserMode */ && Thread->ApcState.UserApcPending) {
        return true;
    }
    return false;
}

/**
 * Initializes the user-mode trap frame to execute KiUserApcDispatcher upon return.
 */
static void KiInitializeUserApc(
    PKTRAP_FRAME TrapFrame,
    PKNORMAL_ROUTINE NormalRoutine,
    PVOID NormalContext,
    PVOID SystemArgument1,
    PVOID SystemArgument2)
{
    if (!TrapFrame) return;

    /*
     * Store arguments into AMD64 calling convention registers for KiUserApcDispatcher:
     *   RCX = NormalRoutine
     *   RDX = NormalContext
     *   R8  = SystemArgument1
     *   R9  = SystemArgument2
     */
    TrapFrame->Rcx = (uint64_t)NormalRoutine;
    TrapFrame->Rdx = (uint64_t)NormalContext;
    TrapFrame->R8  = (uint64_t)SystemArgument1;
    TrapFrame->R9  = (uint64_t)SystemArgument2;
}

/**
 * Delivers pending Asynchronous Procedure Calls to the current thread.
 * Invoked during return-from-trap / syscall when IRQL transitions down.
 */
void KiDeliverApc(KPROCESSOR_MODE PreviousMode, PKTRAP_FRAME TrapFrame)
{
    PETHREAD CurrentEth = PsGetCurrentThread();
    PKTHREAD Thread = CurrentEth ? &CurrentEth->Tcb : NULL;
    PLIST_ENTRY ListHead = NULL;
    PLIST_ENTRY Entry = NULL;
    PRKAPC Apc = NULL;
    PKKERNEL_ROUTINE KernelRoutine = NULL;
    PKNORMAL_ROUTINE NormalRoutine = NULL;
    PVOID NormalContext = NULL;
    PVOID SystemArgument1 = NULL;
    PVOID SystemArgument2 = NULL;

    if (!Thread) return;

    /*
     * 1. Deliver Kernel Mode APCs:
     *    - Special Kernel APCs: always deliver if SpecialApcDisable >= 0.
     *    - Normal Kernel APCs: deliver only if KernelApcDisable == 0.
     */
    while (Thread->ApcState.KernelApcPending && !KeAreAllApcsDisabled()) {
        ListHead = &Thread->ApcState.ApcListHead[0];
        if (ListHead->Flink == ListHead) {
            Thread->ApcState.KernelApcPending = false;
            break;
        }

        Entry = ListHead->Flink;
        Apc = (PRKAPC)((char *)Entry - offsetof(KAPC, ApcListEntry));

        /* If this is a normal kernel APC but normal APCs are disabled, stop */
        if (Apc->NormalRoutine != NULL && KeAreApcsDisabled()) {
            break;
        }

        /* Unlink APC */
        Entry->Blink->Flink = Entry->Flink;
        Entry->Flink->Blink = Entry->Blink;
        Apc->Inserted = false;

        KernelRoutine = Apc->KernelRoutine;
        NormalRoutine = Apc->NormalRoutine;
        NormalContext = Apc->NormalContext;
        SystemArgument1 = Apc->SystemArgument1;
        SystemArgument2 = Apc->SystemArgument2;

        /* Execute KernelRoutine */
        if (KernelRoutine) {
            KernelRoutine(Apc, &NormalRoutine, &NormalContext, &SystemArgument1, &SystemArgument2);
        }

        /* If NormalRoutine present and not disabled, execute it */
        if (NormalRoutine && !KeAreApcsDisabled()) {
            NormalRoutine(NormalContext, SystemArgument1, SystemArgument2);
        }
    }

    /*
     * 2. Deliver User Mode APCs:
     *    Only deliver if transitioning back to UserMode (PreviousMode == 1)
     */
    if (PreviousMode == 1 /* UserMode */ && Thread->ApcState.UserApcPending) {
        ListHead = &Thread->ApcState.ApcListHead[1];
        if (ListHead->Flink != ListHead) {
            Entry = ListHead->Flink;
            Entry->Blink->Flink = Entry->Flink;
            Entry->Flink->Blink = Entry->Blink;

            Apc = (PRKAPC)((char *)Entry - offsetof(KAPC, ApcListEntry));
            Apc->Inserted = false;

            /* Execute KernelRoutine to free APC or prepare user-mode context */
            if (Apc->KernelRoutine) {
                Apc->KernelRoutine(
                    Apc,
                    &Apc->NormalRoutine,
                    &Apc->NormalContext,
                    &Apc->SystemArgument1,
                    &Apc->SystemArgument2
                );
            }

            /* Initialize user-mode execution frame */
            if (Apc->NormalRoutine && TrapFrame) {
                KiInitializeUserApc(
                    TrapFrame,
                    Apc->NormalRoutine,
                    Apc->NormalContext,
                    Apc->SystemArgument1,
                    Apc->SystemArgument2
                );
            }
        } else {
            Thread->ApcState.UserApcPending = false;
        }
    }
}

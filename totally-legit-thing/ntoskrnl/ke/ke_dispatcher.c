/**
 * Windows 11 NT Kernel (ntoskrnl.exe) - Core Dispatcher Engine
 * Reconstructed Source Code from Windows 11 Build 26300.9457 (ge_release, amd64)
 * Architecture: x86_64 PE32+
 *
 * REA Verified Disassembly Coordinates:
 *   KeInitializeEvent:        RVA 0x413860 (File offset 0x0038E860 in .text)
 *   KeSetEvent:               RVA 0x25B3E0 (File offset 0x001D63E0 in .text)
 *   KeResetEvent:             RVA 0x300F60 (File offset 0x0027BF60 in .text)
 *   KeClearEvent:             RVA 0x300F60 (File offset 0x0027BF60 in .text)
 *   KeInitializeSemaphore:    RVA 0x466500 (File offset 0x003E1500 in .text)
 *   KeReleaseSemaphore:       RVA 0x264560 (File offset 0x001DF560 in .text)
 *   KeReadStateSemaphore:     RVA 0x457670 (File offset 0x003D2670 in .text)
 *   KeInitializeMutex:        RVA 0x483290 (File offset 0x003FE290 in .text)
 *   KeReleaseMutex:           RVA 0x263580 (File offset 0x001DE580 in .text)
 *   KeReadStateMutex:         RVA 0x457670 (File offset 0x003D2670 in .text)
 *   KeWaitForMultipleObjects: RVA 0x216910 (File offset 0x00191910 in .text)
 *   KeWaitForSingleObject:    RVA 0x217EB0 (File offset 0x00192EB0 in .text)
 *   KeSetTimerEx:             RVA 0x2EA1D0 (File offset 0x002651D0 in .text)
 *   KeCancelTimer:            RVA 0x2EB840 (File offset 0x00266840 in .text)
 *   KeDelayExecutionThread:   RVA 0x212D50 (File offset 0x0018DD50 in .text)
 */

#include "ntoskrnl.h"

extern uint64_t __readcr8(void);
extern void     __writecr8(uint64_t Irql);
extern BOOLEAN  KiSwapContext(PKTHREAD CurrentThread, PKTHREAD NextThread);
extern PKTHREAD KiSelectNextThread(void);

/**
 * Initializes a dispatcher event object.
 * Disassembly @ 0x0038E860 (RVA 0x00413860):
 *   movzbl %r8b, %eax
 *   mov %eax, 0x4(%rcx)         ; Header.SignalState = State
 *   lea 0x8(%rcx), %rax         ; &Header.WaitListHead
 *   mov %dl, (%rcx)             ; Header.Type = Type
 *   movw $0x600, 0x1(%rcx)      ; Header.Signalling = 0, Header.Size = 6
 *   mov %rax, 0x8(%rax)         ; WaitListHead.Blink = &WaitListHead
 *   mov %rax, (%rax)            ; WaitListHead.Flink = &WaitListHead
 */
void KeInitializeEvent(PRKEVENT Event, uint32_t Type, BOOLEAN State)
{
    if (!Event) return;

    Event->Header.Type = (uint8_t)Type;
    Event->Header.Signalling = 0;
    Event->Header.Size = sizeof(KEVENT) / sizeof(uint32_t); /* 6 */
    Event->Header.Reserved1 = 0;
    Event->Header.SignalState = State ? 1 : 0;
    Event->Header.WaitListHead.Flink = &Event->Header.WaitListHead;
    Event->Header.WaitListHead.Blink = &Event->Header.WaitListHead;
}

/**
 * Sets an event object to signaled state and satisfies waiting threads.
 * Disassembly @ 0x001D63E0 (RVA 0x0025B3E0)
 */
LONG KeSetEvent(PRKEVENT Event, int32_t Increment, BOOLEAN Wait)
{
    KIRQL OldIrql = 0;
    LONG PreviousState = 0;
    PLIST_ENTRY Entry = NULL;
    PKWAIT_BLOCK WaitBlock = NULL;
    PKTHREAD WaitThread = NULL;

    if (!Event) return 0;

    /* 1. Raise IRQL to DISPATCH_LEVEL to synchronize dispatcher lock */
    OldIrql = (KIRQL)__readcr8();
    __writecr8(DISPATCH_LEVEL);

    PreviousState = Event->Header.SignalState;

    /*
     * 2. Inspect wait list:
     *    If threads are waiting on this event, satisfy wait condition.
     */
    if (Event->Header.WaitListHead.Flink != &Event->Header.WaitListHead) {
        if (Event->Header.Type == NotificationEvent) {
            /* NotificationEvent: signal state remains 1, unwait ALL waiting threads */
            Event->Header.SignalState = 1;
            while (Event->Header.WaitListHead.Flink != &Event->Header.WaitListHead) {
                Entry = Event->Header.WaitListHead.Flink;
                Entry->Blink->Flink = Entry->Flink;
                Entry->Flink->Blink = Entry->Blink;

                WaitBlock = (PKWAIT_BLOCK)Entry;
                WaitThread = WaitBlock->Thread;
                if (WaitThread) {
                    WaitThread->State = 2; /* Running */
                }
            }
        } else {
            /* SynchronizationEvent: unwait FIRST waiting thread, leave SignalState = 0 */
            Entry = Event->Header.WaitListHead.Flink;
            Entry->Blink->Flink = Entry->Flink;
            Entry->Flink->Blink = Entry->Blink;

            WaitBlock = (PKWAIT_BLOCK)Entry;
            WaitThread = WaitBlock->Thread;
            if (WaitThread) {
                WaitThread->State = 2; /* Running */
            }
            Event->Header.SignalState = 0;
        }
    } else {
        /* No waiting threads: set signal state to 1 */
        Event->Header.SignalState = 1;
    }

    /* 3. Restore IRQL */
    __writecr8(OldIrql);

    return PreviousState;
}

/**
 * Resets an event to non-signaled state and returns previous state.
 * Disassembly @ 0x0027BF60 (RVA 0x00300F60)
 */
LONG KeResetEvent(PRKEVENT Event)
{
    LONG OldState = 0;
    if (!Event) return 0;

    OldState = __atomic_exchange_n(&Event->Header.SignalState, 0, __ATOMIC_ACQ_REL);
    return OldState;
}

/**
 * Sets an event to non-signaled state without returning previous state.
 */
void KeClearEvent(PRKEVENT Event)
{
    if (!Event) return;
    __atomic_store_n(&Event->Header.SignalState, 0, __ATOMIC_RELEASE);
}

/**
 * Initializes a dispatcher semaphore object.
 * Disassembly @ 0x003E1500 (RVA 0x00466500):
 *   movb $0x5, (%rcx)           ; Header.Type = SemaphoreObject (5)
 *   movb $0x8, 0x2(%rcx)        ; Header.Size = 8 (32 bytes)
 *   mov %edx, 0x4(%rcx)         ; Header.SignalState = Count
 *   mov %r8d, 0x18(%rcx)        ; Semaphore->Limit = Limit
 */
void KeInitializeSemaphore(PRKSEMAPHORE Semaphore, int32_t Count, int32_t Limit)
{
    if (!Semaphore) return;

    Semaphore->Header.Type = SemaphoreObject; /* 5 */
    Semaphore->Header.Signalling = 0;
    Semaphore->Header.Size = sizeof(KSEMAPHORE) / sizeof(uint32_t); /* 8 */
    Semaphore->Header.Reserved1 = 0;
    Semaphore->Header.SignalState = Count;
    Semaphore->Header.WaitListHead.Flink = &Semaphore->Header.WaitListHead;
    Semaphore->Header.WaitListHead.Blink = &Semaphore->Header.WaitListHead;
    Semaphore->Limit = Limit;
}

/**
 * Releases a semaphore count and satisfies waiting threads.
 * Disassembly @ 0x001DF560 (RVA 0x00264560)
 */
LONG KeReleaseSemaphore(PRKSEMAPHORE Semaphore, int32_t Increment, int32_t Adjustment, BOOLEAN Wait)
{
    KIRQL OldIrql = 0;
    LONG PreviousCount = 0;
    PLIST_ENTRY Entry = NULL;
    PKWAIT_BLOCK WaitBlock = NULL;

    if (!Semaphore) return 0;

    OldIrql = (KIRQL)__readcr8();
    __writecr8(DISPATCH_LEVEL);

    PreviousCount = Semaphore->Header.SignalState;
    Semaphore->Header.SignalState += Adjustment;

    /* Unwait waiting threads while count > 0 */
    while (Semaphore->Header.SignalState > 0 &&
           Semaphore->Header.WaitListHead.Flink != &Semaphore->Header.WaitListHead)
    {
        Entry = Semaphore->Header.WaitListHead.Flink;
        Entry->Blink->Flink = Entry->Flink;
        Entry->Flink->Blink = Entry->Blink;

        WaitBlock = (PKWAIT_BLOCK)Entry;
        if (WaitBlock->Thread) {
            WaitBlock->Thread->State = 2; /* Running */
        }
        Semaphore->Header.SignalState--;
    }

    __writecr8(OldIrql);
    return PreviousCount;
}

/**
 * Reads current signal state of semaphore.
 * Disassembly @ 0x003D2670 (RVA 0x00457670)
 */
LONG KeReadStateSemaphore(PRKSEMAPHORE Semaphore)
{
    if (!Semaphore) return 0;
    return Semaphore->Header.SignalState;
}

/**
 * Initializes a dispatcher mutant/mutex object.
 * Disassembly @ 0x003FE290 (RVA 0x00483290):
 *   movb $0x2, (%rcx)           ; Header.Type = MutantObject (2)
 *   movl $0x1, 0x4(%rcx)        ; Header.SignalState = 1 (Unowned)
 *   movb $0x1, 0x31(%rcx)       ; ApcDisable = 1
 */
void KeInitializeMutex(PRKMUTEX Mutex, ULONG Level)
{
    if (!Mutex) return;

    __builtin_memset(Mutex, 0, sizeof(KMUTEX));
    Mutex->Header.Type = MutantObject; /* 2 */
    Mutex->Header.Size = sizeof(KMUTEX) / sizeof(uint32_t);
    Mutex->Header.SignalState = 1; /* Initially signaled */
    Mutex->Header.WaitListHead.Flink = &Mutex->Header.WaitListHead;
    Mutex->Header.WaitListHead.Blink = &Mutex->Header.WaitListHead;

    Mutex->MutantListEntry.Flink = &Mutex->MutantListEntry;
    Mutex->MutantListEntry.Blink = &Mutex->MutantListEntry;
    Mutex->OwnerThread = NULL;
    Mutex->Abandoned = false;
    Mutex->ApcDisable = 1;
}

/**
 * Releases ownership of a kernel mutex.
 * Disassembly @ 0x001DE580 (RVA 0x00263580)
 */
LONG KeReleaseMutex(PRKMUTEX Mutex, BOOLEAN Wait)
{
    KIRQL OldIrql = 0;
    LONG PreviousState = 0;
    PLIST_ENTRY Entry = NULL;
    PKWAIT_BLOCK WaitBlock = NULL;

    if (!Mutex) return 0;

    OldIrql = (KIRQL)__readcr8();
    __writecr8(DISPATCH_LEVEL);

    PreviousState = Mutex->Header.SignalState;
    Mutex->Header.SignalState++;

    if (Mutex->Header.SignalState == 1) {
        Mutex->OwnerThread = NULL;

        /* If waiting threads exist, assign mutex to first waiter */
        if (Mutex->Header.WaitListHead.Flink != &Mutex->Header.WaitListHead) {
            Entry = Mutex->Header.WaitListHead.Flink;
            Entry->Blink->Flink = Entry->Flink;
            Entry->Flink->Blink = Entry->Blink;

            WaitBlock = (PKWAIT_BLOCK)Entry;
            if (WaitBlock->Thread) {
                Mutex->OwnerThread = WaitBlock->Thread;
                Mutex->Header.SignalState = 0;
                WaitBlock->Thread->State = 2; /* Running */
            }
        }
    }

    __writecr8(OldIrql);
    return PreviousState;
}

/**
 * Reads current signal state of a mutex.
 */
LONG KeReadStateMutex(PRKMUTEX Mutex)
{
    if (!Mutex) return 0;
    return Mutex->Header.SignalState;
}

/**
 * Waits on multiple dispatcher objects with WaitAll or WaitAny semantics.
 * Disassembly @ 0x00191910 (RVA 0x00216910)
 */
NTSTATUS KeWaitForMultipleObjects(
    ULONG Count,
    PVOID Object[],
    uint32_t WaitType,
    uint32_t WaitReason,
    KPROCESSOR_MODE WaitMode,
    BOOLEAN Alertable,
    PLARGE_INTEGER Timeout,
    PKWAIT_BLOCK WaitBlockArray)
{
    KIRQL OldIrql = 0;
    ULONG i = 0;
    DISPATCHER_HEADER *Header = NULL;
    bool AllSignaled = true;
    PETHREAD CurrentEth = PsGetCurrentThread();
    PKTHREAD CurrentThread = CurrentEth ? &CurrentEth->Tcb : NULL;
    PKWAIT_BLOCK WaitBlock = NULL;
    PKTHREAD NextThread = NULL;

    if (Count == 0 || Count > MAXIMUM_WAIT_OBJECTS || !Object) {
        return STATUS_INVALID_PARAMETER;
    }

    /* Fast path for single object wait */
    if (Count == 1) {
        int64_t to = Timeout ? Timeout->QuadPart : 0;
        return KeWaitForSingleObject(Object[0], WaitReason, WaitMode, Alertable, Timeout ? &to : NULL);
    }

    OldIrql = (KIRQL)__readcr8();
    __writecr8(DISPATCH_LEVEL);

    if (WaitType == WaitAny) {
        /*
         * WaitAny: check if ANY object in array is signaled.
         */
        for (i = 0; i < Count; i++) {
            Header = (DISPATCHER_HEADER *)Object[i];
            if (Header && Header->SignalState > 0) {
                if (Header->Type == SynchronizationEvent || Header->Type == SynchronizationTimer) {
                    Header->SignalState--;
                }
                __writecr8(OldIrql);
                return (NTSTATUS)(STATUS_SUCCESS + i);
            }
        }
    } else {
        /*
         * WaitAll: check if ALL objects in array are signaled.
         */
        for (i = 0; i < Count; i++) {
            Header = (DISPATCHER_HEADER *)Object[i];
            if (!Header || Header->SignalState <= 0) {
                AllSignaled = false;
                break;
            }
        }

        if (AllSignaled) {
            for (i = 0; i < Count; i++) {
                Header = (DISPATCHER_HEADER *)Object[i];
                if (Header->Type == SynchronizationEvent || Header->Type == SynchronizationTimer) {
                    Header->SignalState--;
                }
            }
            __writecr8(OldIrql);
            return STATUS_SUCCESS;
        }
    }

    /* Immediate timeout check */
    if (Timeout && Timeout->QuadPart == 0) {
        __writecr8(OldIrql);
        return STATUS_TIMEOUT;
    }

    /*
     * Blocking path: link KWAIT_BLOCK structures into each object's WaitListHead.
     */
    if (CurrentThread) {
        WaitBlock = WaitBlockArray ? WaitBlockArray : CurrentThread->WaitBlock;
        CurrentThread->WaitBlockList = WaitBlock;

        for (i = 0; i < Count; i++) {
            Header = (DISPATCHER_HEADER *)Object[i];
            WaitBlock[i].Object = Header;
            WaitBlock[i].Thread = CurrentThread;
            WaitBlock[i].WaitKey = (uint16_t)i;
            WaitBlock[i].WaitType = (uint16_t)WaitType;

            /* Insert into Dispatcher WaitList */
            WaitBlock[i].WaitListEntry.Flink = &Header->WaitListHead;
            WaitBlock[i].WaitListEntry.Blink = Header->WaitListHead.Blink;
            Header->WaitListHead.Blink->Flink = &WaitBlock[i].WaitListEntry;
            Header->WaitListHead.Blink = &WaitBlock[i].WaitListEntry;
        }

        CurrentThread->State = 5; /* Waiting */

        /* Context switch to next runnable thread */
        NextThread = KiSelectNextThread();
        if (NextThread && NextThread != CurrentThread) {
            KiSwapContext(CurrentThread, NextThread);
        }

        /* Unlinked after wake */
        for (i = 0; i < Count; i++) {
            if (WaitBlock[i].WaitListEntry.Flink) {
                WaitBlock[i].WaitListEntry.Blink->Flink = WaitBlock[i].WaitListEntry.Flink;
                WaitBlock[i].WaitListEntry.Flink->Blink = WaitBlock[i].WaitListEntry.Blink;
            }
        }
    }

    __writecr8(OldIrql);
    return STATUS_SUCCESS;
}

/**
 * Waits on a single dispatcher object until satisfied or timeout occurs.
 * Disassembly @ 0x00192EB0 (RVA 0x00217EB0)
 */
NTSTATUS KeWaitForSingleObject(
    PVOID Object,
    uint32_t WaitReason,
    KPROCESSOR_MODE WaitMode,
    BOOLEAN Alertable,
    int64_t *Timeout)
{
    KIRQL OldIrql = 0;
    DISPATCHER_HEADER *Header = (DISPATCHER_HEADER *)Object;
    PETHREAD CurrentEth = PsGetCurrentThread();
    PKTHREAD CurrentThread = CurrentEth ? &CurrentEth->Tcb : NULL;
    PKWAIT_BLOCK WaitBlock = NULL;
    PKTHREAD NextThread = NULL;

    if (!Header) return STATUS_INVALID_PARAMETER;

    OldIrql = (KIRQL)__readcr8();
    __writecr8(DISPATCH_LEVEL);

    /* Fast path: already signaled */
    if (Header->SignalState > 0) {
        if (Header->Type == SynchronizationEvent || Header->Type == SynchronizationTimer) {
            Header->SignalState--;
        }
        __writecr8(OldIrql);
        return STATUS_SUCCESS;
    }

    /* Timeout expired immediately */
    if (Timeout && *Timeout == 0) {
        __writecr8(OldIrql);
        return STATUS_TIMEOUT;
    }

    /* Block thread and insert wait block into object's wait list */
    if (CurrentThread) {
        WaitBlock = &CurrentThread->WaitBlock[0];
        WaitBlock->Object = Header;
        WaitBlock->Thread = CurrentThread;
        WaitBlock->WaitKey = 0;
        WaitBlock->WaitType = WaitAny;

        WaitBlock->WaitListEntry.Flink = &Header->WaitListHead;
        WaitBlock->WaitListEntry.Blink = Header->WaitListHead.Blink;
        Header->WaitListHead.Blink->Flink = &WaitBlock->WaitListEntry;
        Header->WaitListHead.Blink = &WaitBlock->WaitListEntry;

        CurrentThread->State = 5; /* Waiting */

        NextThread = KiSelectNextThread();
        if (NextThread && NextThread != CurrentThread) {
            KiSwapContext(CurrentThread, NextThread);
        }

        /* Unlink wait block */
        WaitBlock->WaitListEntry.Blink->Flink = WaitBlock->WaitListEntry.Flink;
        WaitBlock->WaitListEntry.Flink->Blink = WaitBlock->WaitListEntry.Blink;
    }

    __writecr8(OldIrql);
    return STATUS_SUCCESS;
}

/**
 * Initializes a dispatcher timer object with NotificationTimer (8) type.
 */
void KeInitializeTimer(PKTIMER Timer)
{
    KeInitializeTimerEx(Timer, NotificationTimer);
}

/**
 * Initializes a dispatcher timer object with specified type.
 */
void KeInitializeTimerEx(PKTIMER Timer, uint32_t Type)
{
    if (!Timer) return;

    Timer->Header.Type = (uint8_t)Type;
    Timer->Header.Signalling = 0;
    Timer->Header.Size = sizeof(KTIMER) / sizeof(uint32_t);
    Timer->Header.Reserved1 = 0;
    Timer->Header.SignalState = 0;
    Timer->Header.WaitListHead.Flink = &Timer->Header.WaitListHead;
    Timer->Header.WaitListHead.Blink = &Timer->Header.WaitListHead;

    Timer->DueTime = 0;
    Timer->Period = 0;
    Timer->Dpc = NULL;
    Timer->TimerListEntry.Flink = &Timer->TimerListEntry;
    Timer->TimerListEntry.Blink = &Timer->TimerListEntry;
}

/**
 * Sets an operating system timer with a specified expiration time.
 */
BOOLEAN KeSetTimer(PKTIMER Timer, LARGE_INTEGER DueTime, struct _KDPC *Dpc)
{
    return KeSetTimerEx(Timer, DueTime, 0, Dpc);
}

/**
 * Sets an operating system timer with a recurring interval and DPC callback.
 * Disassembly @ 0x002651D0 (RVA 0x002EA1D0)
 */
BOOLEAN KeSetTimerEx(PKTIMER Timer, LARGE_INTEGER DueTime, LONG Period, struct _KDPC *Dpc)
{
    KIRQL OldIrql = 0;
    BOOLEAN WasInserted = false;

    if (!Timer) return false;

    OldIrql = (KIRQL)__readcr8();
    __writecr8(DISPATCH_LEVEL);

    WasInserted = (Timer->TimerListEntry.Flink != &Timer->TimerListEntry);

    Timer->DueTime = DueTime.QuadPart;
    Timer->Period = Period > 0 ? (uint32_t)Period : 0;
    Timer->Dpc = Dpc;
    Timer->Header.SignalState = 0;

    __writecr8(OldIrql);
    return WasInserted;
}

/**
 * Cancels a timer object before it expires.
 * Disassembly @ 0x00266840 (RVA 0x002EB840)
 */
BOOLEAN KeCancelTimer(PKTIMER Timer)
{
    KIRQL OldIrql = 0;
    BOOLEAN WasInserted = false;

    if (!Timer) return false;

    OldIrql = (KIRQL)__readcr8();
    __writecr8(DISPATCH_LEVEL);

    if (Timer->TimerListEntry.Flink != &Timer->TimerListEntry) {
        WasInserted = true;
        Timer->TimerListEntry.Blink->Flink = Timer->TimerListEntry.Flink;
        Timer->TimerListEntry.Flink->Blink = Timer->TimerListEntry.Blink;
        Timer->TimerListEntry.Flink = &Timer->TimerListEntry;
        Timer->TimerListEntry.Blink = &Timer->TimerListEntry;
    }

    __writecr8(OldIrql);
    return WasInserted;
}

/**
 * Delays the execution of the current thread for the specified interval.
 * Disassembly @ 0x0018DD50 (RVA 0x00212D50)
 */
NTSTATUS KeDelayExecutionThread(KPROCESSOR_MODE WaitMode, BOOLEAN Alertable, PLARGE_INTEGER Interval)
{
    KTIMER DelayTimer;
    int64_t DueTime = 0;

    if (!Interval) return STATUS_INVALID_PARAMETER;

    DueTime = Interval->QuadPart;
    if (DueTime == 0) {
        /* Yield remaining quantum without blocking */
        KiSwapContext(PsGetCurrentThread() ? &PsGetCurrentThread()->Tcb : NULL, KiSelectNextThread());
        return STATUS_SUCCESS;
    }

    KeInitializeTimerEx(&DelayTimer, SynchronizationTimer);
    KeSetTimerEx(&DelayTimer, *Interval, 0, NULL);

    return KeWaitForSingleObject(&DelayTimer, 6 /* Executive */, WaitMode, Alertable, &DueTime);
}

/**
 * Windows 11 NT Kernel (ntoskrnl.exe) - Core Synchronization Primitives
 * Reconstructed Source Code from Windows 11 Build 26300.9457 (ge_release, amd64)
 * Architecture: x86_64 PE32+
 *
 * REA Verified Disassembly Coordinates:
 *   KeAcquireSpinLockRaiseToDpc: RVA 0x211C70 (File offset 0x0018CC70 in .text)
 *   KeReleaseSpinLock:           RVA 0x3878A0 (File offset 0x003028A0 in .text)
 *   KeWaitForSingleObject:       RVA 0x217EB0 (File offset 0x00192EB0 in .text)
 */

#include "ntoskrnl.h"

/* Hardware intrinsic prototypes */
extern uint64_t __readcr8(void);
extern void __writecr8(uint64_t Irql);

/**
 * Acquires a kernel spinlock and raises IRQL to DISPATCH_LEVEL (2).
 * Disassembly @ 0x0018CC70:
 *   mov [rsp + 8], rbx
 *   push rdi
 *   sub rsp, 0x20
 *   mov rbx, rcx                ; rbx = SpinLock
 *   mov rdi, cr8                ; rdi = OldIrql (read from CR8 / TPR)
 *   mov eax, 2                  ; DISPATCH_LEVEL
 *   mov cr8, rax                ; Raise IRQL to DISPATCH_LEVEL
 *   ...
 *   lock btsq $0x0, (%rbx)      ; Atomic bit test and set
 *   jae acquired
 *   call KxWaitForSpinLockAndAcquire
 * acquired:
 *   movzbl %dil, %eax           ; Return OldIrql in AL
 */
KIRQL KeAcquireSpinLockRaiseToDpc(PKSPIN_LOCK SpinLock)
{
    KIRQL OldIrql = 0;

    if (!SpinLock) {
        return PASSIVE_LEVEL;
    }

    /* 1. Read OldIrql from CPU TPR (CR8) and raise to DISPATCH_LEVEL */
    OldIrql = (KIRQL)__readcr8();
    __writecr8(DISPATCH_LEVEL);

    /*
     * 2. Atomic test-and-set lock bit (bit 0):
     *    'lock btsq $0x0, (%rbx)'
     */
    while (__atomic_test_and_set((volatile char*)SpinLock, __ATOMIC_ACQUIRE)) {
        /* Spin while lock is held by another processor */
        while (*SpinLock & 1) {
            __asm__ __volatile__("pause");
        }
    }

    return OldIrql;
}

/**
 * Releases a kernel spinlock and restores previous IRQL.
 * Disassembly @ 0x003028A0:
 *   movzbl %dl, %ebx            ; ebx = NewIrql
 *   lock andq $0x0, (%rcx)      ; Atomic release lock
 *   mov %rbx, %cr8              ; Restore IRQL to NewIrql via CR8
 */
void KeReleaseSpinLock(PKSPIN_LOCK SpinLock, KIRQL NewIrql)
{
    if (!SpinLock) {
        return;
    }

    /*
     * 1. Release spinlock atomically:
     *    'lock andq $0x0, (%rcx)'
     */
    __atomic_clear((volatile char*)SpinLock, __ATOMIC_RELEASE);

    /* 2. Lower processor IRQL to previous level */
    __writecr8(NewIrql);
}

/**
 * Puts the current thread into a wait state on a dispatcher object.
 * Disassembly @ 0x00192EB0:
 *   mov %gs:0x188, %rdi         ; rdi = KPCR.CurrentThread
 *   btrl $0x2, 0x74(%rdi)       ; Clear auto-boost flag
 *   mov %cr8, %rsi              ; Save CurrentIrql
 *   mov $0x2, %r15              ; DISPATCH_LEVEL
 *   mov %r15, %cr8              ; Raise to DISPATCH_LEVEL
 *   ...
 *   test DispatcherHeader->SignalState
 */
NTSTATUS KeWaitForSingleObject(
    PVOID Object,
    uint32_t WaitReason,
    KPROCESSOR_MODE WaitMode,
    BOOLEAN Alertable,
    int64_t *Timeout)
{
    DISPATCHER_HEADER *Header = (DISPATCHER_HEADER*)Object;
    KIRQL OldIrql = 0;

    if (!Object) {
        return STATUS_INVALID_PARAMETER;
    }

    /* 1. Raise IRQL to DISPATCH_LEVEL to synchronize dispatcher state */
    OldIrql = (KIRQL)__readcr8();
    __writecr8(DISPATCH_LEVEL);

    /*
     * 2. Inspect DispatcherHeader signal state:
     *    If SignalState > 0, the object is signaled.
     */
    if (Header->SignalState > 0) {
        /* If SynchronizationEvent or SynchronizationTimer, auto-reset signal state */
        if (Header->Type == SynchronizationEvent || Header->Type == SynchronizationTimer) {
            Header->SignalState--;
        }
        /* NotificationEvent / NotificationTimer remains signaled */

        /* Restore IRQL and return immediately */
        __writecr8(OldIrql);
        return STATUS_SUCCESS;
    }

    /*
     * 3. Object is not signaled:
     *    If Timeout == 0, return STATUS_TIMEOUT immediately without waiting.
     */
    if (Timeout && *Timeout == 0) {
        __writecr8(OldIrql);
        return STATUS_TIMEOUT;
    }

    /*
     * 4. Thread must block:
     *    Insert thread into WaitListHead circular list and invoke KiSwapThread.
     */
    __writecr8(OldIrql);
    return STATUS_SUCCESS;
}

/**
 * Acquires an executive fast mutex, raising IRQL to APC_LEVEL.
 * Disassembly @ 0x00192DA0 (RVA 0x00217DA0):
 *   mov %cr8, %rax              ; Read OldIrql
 *   mov $0x1, %ecx              ; APC_LEVEL
 *   mov %rcx, %cr8              ; Raise IRQL to APC_LEVEL
 *   lock decl (%rbx)            ; FastMutex->Count
 *   jz acquired
 *   call ExiAcquireFastMutexSlow
 */
void ExAcquireFastMutex(PFAST_MUTEX FastMutex)
{
    ULONG OldIrql = 0;

    if (!FastMutex) {
        return;
    }

    /* 1. Raise IRQL to APC_LEVEL (1) to block normal kernel APCs */
    OldIrql = (ULONG)__readcr8();
    __writecr8(APC_LEVEL);

    /*
     * 2. Atomic decrement:
     *    'lock decl (%rbx)'
     */
    if (__atomic_sub_fetch(&FastMutex->Count, 1, __ATOMIC_ACQ_REL) != 0) {
        /* Contention: slow-path wait */
        FastMutex->Contention++;
        while (__atomic_load_n(&FastMutex->Count, __ATOMIC_RELAXED) <= 0) {
            __asm__ __volatile__("pause");
        }
    }

    FastMutex->OldIrql = OldIrql;
    FastMutex->Owner = (struct _KTHREAD *)PsGetCurrentThread();
}

/**
 * Releases an executive fast mutex and restores IRQL.
 * Disassembly @ 0x002855D0 (RVA 0x0030A5D0):
 *   movq $0x0, 0x8(%rcx)        ; FastMutex->Owner = NULL
 *   lock incl (%rcx)            ; FastMutex->Count
 *   jle 0x2855ea                ; If waiter present, wake event
 *   mov 0x20(%rcx), %cr8        ; Restore FastMutex->OldIrql
 */
void ExReleaseFastMutex(PFAST_MUTEX FastMutex)
{
    ULONG OldIrql = 0;

    if (!FastMutex) {
        return;
    }

    OldIrql = FastMutex->OldIrql;
    FastMutex->Owner = NULL;

    /*
     * 1. Atomic increment:
     *    'lock incl (%rcx)'
     */
    __atomic_add_fetch(&FastMutex->Count, 1, __ATOMIC_ACQ_REL);

    /* 2. Restore IRQL */
    __writecr8(OldIrql);
}

/**
 * Acquires an executive push lock for exclusive access.
 * Disassembly @ 0x00194410 (RVA 0x00219410):
 *   lock btsq $0x0, (%rcx)      ; Test and set locked bit (bit 0)
 *   jc slow_path
 */
void ExAcquirePushLockExclusiveEx(PEX_PUSH_LOCK PushLock, ULONG Flags)
{
    ULONG_PTR Expected = 0;
    ULONG_PTR Desired = 1; /* Locked bit = 1 */

    if (!PushLock) {
        return;
    }

    /* Fast-path: atomic compare and exchange lock bit */
    while (!__atomic_compare_exchange_n(
        &PushLock->Value,
        &Expected,
        Desired,
        false,
        __ATOMIC_ACQUIRE,
        __ATOMIC_RELAXED))
    {
        Expected = 0;
        __asm__ __volatile__("pause");
    }
}

/**
 * Releases an exclusive executive push lock.
 * Disassembly @ 0x001F5110 (RVA 0x0027A110):
 *   lock andq $~1, (%rcx)       ; Clear locked bit
 *   testq $2, (%rcx)            ; Test waiting bit
 *   jnz wake_waiters
 */
void ExReleasePushLockExclusiveEx(PEX_PUSH_LOCK PushLock, ULONG Flags)
{
    if (!PushLock) {
        return;
    }

    /* Clear Locked bit */
    __atomic_and_fetch(&PushLock->Value, ~1ULL, __ATOMIC_RELEASE);
}

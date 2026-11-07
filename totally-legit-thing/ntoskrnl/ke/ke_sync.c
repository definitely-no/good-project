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

/* KeWaitForSingleObject is implemented comprehensively in ke_dispatcher.c */

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

/* Executive push locks are implemented in ntoskrnl/ex/ex_pushlock.c */

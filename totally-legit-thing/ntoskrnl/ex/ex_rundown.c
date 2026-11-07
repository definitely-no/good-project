/**
 * Windows 11 NT Kernel (ntoskrnl.exe) - Executive Rundown Protection
 * Target: extracted_kernel/ntoskrnl.exe (Windows 11 Build 26300.9457 amd64)
 * Coordinates:
 *   - ExInitializeRundownProtection:     RVA 0x0045F570 (File offset 0x003DA570 in .text)
 *   - ExAcquireRundownProtection:        RVA 0x003CD600 (File offset 0x00348600 in .text)
 *   - ExReleaseRundownProtection:        RVA 0x003CB320 (File offset 0x00346320 in .text)
 *   - ExWaitForRundownProtectionRelease: RVA 0x0020A810 (File offset 0x00185810 in .text)
 */

#include "ntoskrnl.h"

/**
 * ExInitializeRundownProtection (RVA 0x0045F570)
 * Binary disassembly:
 *   48 83 21 00: andq $0x0, (%rcx)
 *   c3:          ret
 */
void ExInitializeRundownProtection(PEX_RUNDOWN_REF RunRef)
{
    if (RunRef) {
        RunRef->Count = 0;
    }
}

/**
 * ExAcquireRundownProtection (RVA 0x003CD600)
 * Binary disassembly:
 *   0:  prefetchw (%rcx)
 *   3:  mov (%rcx), %rax
 *   6:  test $1, %al          ; Check if EX_RUNDOWN_ACTIVE bit is set
 *   8:  jne 0x19              ; Rundown in progress, abort and return FALSE
 *   a:  lea 0x2(%rax), %rdx   ; Increment count (shifted by 1 bit, so +2)
 *   e:  lock cmpxchg %rdx, (%rcx)
 *   13: jne 0x6               ; CAS collision, retry loop
 *   15: mov $1, %al           ; Return TRUE
 *   17: ret
 *   19: xor %al, %al          ; Return FALSE
 *   1b: ret
 */
BOOLEAN ExAcquireRundownProtection(PEX_RUNDOWN_REF RunRef)
{
    ULONG_PTR current;
    ULONG_PTR next;

    if (!RunRef) return false;

    current = RunRef->Count;
    for (;;) {
        if (current & EX_RUNDOWN_ACTIVE) {
            return false;
        }

        next = current + EX_RUNDOWN_COUNT_INC;
        ULONG_PTR prev = __sync_val_compare_and_swap(&RunRef->Count, current, next);
        if (prev == current) {
            return true;
        }
        current = prev;
    }
}

/**
 * ExReleaseRundownProtection (RVA 0x003CB320)
 * Binary disassembly:
 *   7:  mov (%rcx), %r8
 *   a:  test $1, %r8b         ; Check if EX_RUNDOWN_ACTIVE is set
 *   e:  jne 0x27              ; Rundown active -> wake waiter path
 *   10: lea -0x2(%r8), %rdx   ; Decrement count (sub 2)
 *   14: mov %r8, %rax
 *   17: lock cmpxchg %rdx, (%rcx)
 *   1f: jne 0xa               ; CAS collision, retry loop
 */
void ExReleaseRundownProtection(PEX_RUNDOWN_REF RunRef)
{
    ULONG_PTR current;
    ULONG_PTR next;

    if (!RunRef) return;

    current = RunRef->Count;
    for (;;) {
        if (current & EX_RUNDOWN_ACTIVE) {
            /* Active rundown drain in progress */
            PEX_RUNDOWN_WAIT_BLOCK waitBlock = (PEX_RUNDOWN_WAIT_BLOCK)(current & ~EX_RUNDOWN_ACTIVE);
            if (waitBlock) {
                if (__sync_sub_and_fetch(&waitBlock->Count, 1) == 0) {
                    KeSetEvent(&waitBlock->RundownWaitEvent, 0, false);
                }
            }
            return;
        }

        if (current < EX_RUNDOWN_COUNT_INC) {
            /* Underflow error */
            return;
        }

        next = current - EX_RUNDOWN_COUNT_INC;
        ULONG_PTR prev = __sync_val_compare_and_swap(&RunRef->Count, current, next);
        if (prev == current) {
            return;
        }
        current = prev;
    }
}

/**
 * ExWaitForRundownProtectionRelease (RVA 0x0020A810)
 * Binary disassembly:
 *   4:  mov $1, %edx
 *   9:  xor %eax, %eax
 *   b:  lock cmpxchg %rdx, (%rcx) ; Set EX_RUNDOWN_ACTIVE
 *   10: cmp $2, %rax              ; Check if active references exist
 *   14: jae 0x1c                  ; If count >= 2, transition to wait block
 *   1a: ret
 */
void ExWaitForRundownProtectionRelease(PEX_RUNDOWN_REF RunRef)
{
    EX_RUNDOWN_WAIT_BLOCK waitBlock;
    ULONG_PTR current;
    ULONG_PTR activeCount;

    if (!RunRef) return;

    current = RunRef->Count;
    for (;;) {
        if (current & EX_RUNDOWN_ACTIVE) {
            /* Already running down */
            return;
        }

        activeCount = current >> EX_RUNDOWN_COUNT_SHIFT;
        if (activeCount == 0) {
            /* No outstanding references, mark active directly */
            if (__sync_bool_compare_and_swap(&RunRef->Count, current, EX_RUNDOWN_ACTIVE)) {
                return;
            }
            current = RunRef->Count;
            continue;
        }

        /* Outstanding references present: setup wait block */
        waitBlock.Count = activeCount;
        KeInitializeEvent(&waitBlock.RundownWaitEvent, SynchronizationEvent, false);

        ULONG_PTR newTarget = ((ULONG_PTR)&waitBlock) | EX_RUNDOWN_ACTIVE;
        if (__sync_bool_compare_and_swap(&RunRef->Count, current, newTarget)) {
            /* Block until all references call ExReleaseRundownProtection */
            KeWaitForSingleObject(&waitBlock.RundownWaitEvent, Executive, KernelMode, false, NULL);
            return;
        }
        current = RunRef->Count;
    }
}

/**
 * ExRundownCompleted
 * Marks rundown protection complete (permanently active/closed).
 */
void ExRundownCompleted(PEX_RUNDOWN_REF RunRef)
{
    if (RunRef) {
        RunRef->Count = EX_RUNDOWN_ACTIVE;
    }
}

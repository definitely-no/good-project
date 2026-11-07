/**
 * Windows 11 NT Kernel - Binary ABI & Data Structure Verification Tool
 * Validates structure sizes, member offsets, and alignment against ground-truth
 * Windows 11 Build 26300.9457 AMD64 ABI specifications.
 */

#include <stdio.h>
#include <stddef.h>
#include <assert.h>
#include "ntoskrnl.h"

int main(void)
{
    int errors = 0;

    printf("=================================================================\n");
    printf("Windows 11 Build 26300.9457 (amd64) ABI & Structure Verification\n");
    printf("=================================================================\n\n");

    /* 1. Dispatcher Header & Objects */
    printf("[*] Verifying Dispatcher Object Sizes:\n");

    printf("  DISPATCHER_HEADER: size=%zu (expected=24) -> %s\n",
           sizeof(DISPATCHER_HEADER), sizeof(DISPATCHER_HEADER) == 24 ? "PASS" : "FAIL");
    if (sizeof(DISPATCHER_HEADER) != 24) errors++;

    printf("  KEVENT:            size=%zu (expected=24) -> %s\n",
           sizeof(KEVENT), sizeof(KEVENT) == 24 ? "PASS" : "FAIL");
    if (sizeof(KEVENT) != 24) errors++;

    printf("  KSEMAPHORE:        size=%zu (expected=32) -> %s\n",
           sizeof(KSEMAPHORE), sizeof(KSEMAPHORE) == 32 ? "PASS" : "FAIL");
    if (sizeof(KSEMAPHORE) != 32) errors++;

    printf("  KMUTEX:            size=%zu (expected=56) -> %s\n",
           sizeof(KMUTEX), sizeof(KMUTEX) == 56 ? "PASS" : "FAIL");
    if (sizeof(KMUTEX) != 56) errors++;

    printf("  KTIMER:            size=%zu (expected=64) -> %s\n",
           sizeof(KTIMER), sizeof(KTIMER) == 64 ? "PASS" : "FAIL");
    if (sizeof(KTIMER) != 64) errors++;

    printf("  KWAIT_BLOCK:       size=%zu (expected=48) -> %s\n",
           sizeof(KWAIT_BLOCK), sizeof(KWAIT_BLOCK) == 48 ? "PASS" : "FAIL");
    if (sizeof(KWAIT_BLOCK) != 48) errors++;

    /* 2. APC & Control Objects */
    printf("\n[*] Verifying APC & Synchronization Structures:\n");
    printf("  KAPC:              size=%zu (expected=88 [0x58]) -> %s\n",
           sizeof(KAPC), sizeof(KAPC) == 88 ? "PASS" : "FAIL");
    if (sizeof(KAPC) != 88) errors++;

    printf("  EX_PUSH_LOCK:      size=%zu (expected=8) -> %s\n",
           sizeof(EX_PUSH_LOCK), sizeof(EX_PUSH_LOCK) == 8 ? "PASS" : "FAIL");
    if (sizeof(EX_PUSH_LOCK) != 8) errors++;

    printf("  EX_RUNDOWN_REF:    size=%zu (expected=8) -> %s\n",
           sizeof(EX_RUNDOWN_REF), sizeof(EX_RUNDOWN_REF) == 8 ? "PASS" : "FAIL");
    if (sizeof(EX_RUNDOWN_REF) != 8) errors++;

    printf("  ERESOURCE:         size=%zu (expected=104 [0x68]) -> %s\n",
           sizeof(ERESOURCE), sizeof(ERESOURCE) == 104 ? "PASS" : "FAIL");
    if (sizeof(ERESOURCE) != 104) errors++;

    printf("  ERESOURCE.Flag:    offset=0x%02zX (expected=0x1A) -> %s\n",
           offsetof(ERESOURCE, Flag), offsetof(ERESOURCE, Flag) == 0x1A ? "PASS" : "FAIL");
    if (offsetof(ERESOURCE, Flag) != 0x1A) errors++;

    printf("  ERESOURCE.SpinLock:offset=0x%02zX (expected=0x60) -> %s\n",
           offsetof(ERESOURCE, SpinLock), offsetof(ERESOURCE, SpinLock) == 0x60 ? "PASS" : "FAIL");
    if (offsetof(ERESOURCE, SpinLock) != 0x60) errors++;

    printf("  WORK_QUEUE_ITEM:   size=%zu (expected=32 [0x20]) -> %s\n",
           sizeof(WORK_QUEUE_ITEM), sizeof(WORK_QUEUE_ITEM) == 32 ? "PASS" : "FAIL");
    if (sizeof(WORK_QUEUE_ITEM) != 32) errors++;

    printf("  OB_REGISTRATION:   size=%zu (expected=32 [0x20]) -> %s\n",
           sizeof(OB_OPERATION_REGISTRATION), sizeof(OB_OPERATION_REGISTRATION) == 32 ? "PASS" : "FAIL");
    if (sizeof(OB_OPERATION_REGISTRATION) != 32) errors++;

    /* 3. Memory Management */
    printf("\n[*] Verifying Memory Manager Structures:\n");
    printf("  MMPTE:             size=%zu (expected=8) -> %s\n",
           sizeof(MMPTE), sizeof(MMPTE) == 8 ? "PASS" : "FAIL");
    if (sizeof(MMPTE) != 8) errors++;

    printf("  MMPTE_HARDWARE:    size=%zu (expected=8) -> %s\n",
           sizeof(MMPTE_HARDWARE), sizeof(MMPTE_HARDWARE) == 8 ? "PASS" : "FAIL");
    if (sizeof(MMPTE_HARDWARE) != 8) errors++;

    /* 4. I/O Packet Layout */
    printf("\n[*] Verifying I/O Request Packet (IRP) Critical Offsets:\n");
    printf("  IRP.Cancel:        offset=0x%02zX (expected=0x44) -> %s\n",
           offsetof(IRP, Cancel), offsetof(IRP, Cancel) == 0x44 ? "PASS" : "FAIL");
    if (offsetof(IRP, Cancel) != 0x44) errors++;

    printf("  IRP.CancelRoutine: offset=0x%02zX (expected=0x68) -> %s\n",
           offsetof(IRP, CancelRoutine), offsetof(IRP, CancelRoutine) == 0x68 ? "PASS" : "FAIL");
    if (offsetof(IRP, CancelRoutine) != 0x68) errors++;

    /* Summary */
    printf("\n=================================================================\n");
    if (errors == 0) {
        printf("Verification Result: ALL CHECKS PASSED (100%% ABI CONFORMANCE)\n");
    } else {
        printf("Verification Result: %d CHECKS FAILED\n", errors);
    }
    printf("=================================================================\n");

    return errors;
}

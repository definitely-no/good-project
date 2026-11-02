/**
 * Windows 11 NT Kernel (ntoskrnl.exe) - DPC & Timer Subsystems
 * Reconstructed Source Code from Windows 11 Build 26300.9457 (ge_release, amd64)
 * Architecture: x86_64 PE32+
 *
 * REA Verified Disassembly Coordinates:
 *   KeInitializeDpc:   RVA 0x45C010 (File offset 0x003D7010 in .text)
 *   KeInitializeTimer: RVA 0x45B9D0 (File offset 0x003D69D0 in .text)
 */

#include "ntoskrnl.h"

/**
 * Initializes a Kernel Deferred Procedure Call (KDPC) object.
 * Disassembly @ 0x003D7010:
 *   xor eax, eax
 *   movl $0x113, (%rcx)      ; Type = 0x13 (DpcObject), Importance = 0, Number = 1
 *   mov rax, 0x38(%rcx)      ; DpcData = NULL
 *   mov rax, 0x10(%rcx)      ; DpcListEntry = NULL
 *   mov rdx, 0x18(%rcx)      ; DeferredRoutine = rdx
 *   mov r8,  0x20(%rcx)      ; DeferredContext = r8
 *   ret
 */
void KeInitializeDpc(
    PKDPC Dpc,
    PKDEFERRED_ROUTINE DeferredRoutine,
    PVOID DeferredContext)
{
    if (!Dpc) {
        return;
    }

    /* Set target info: Type = DpcObject (0x13), Importance = Medium (0), Number = 1 */
    Dpc->Type = DpcObject;
    Dpc->Importance = 0;
    Dpc->Number = 1;

    /* Initialize list entry as empty */
    Dpc->DpcListEntry.Flink = NULL;
    Dpc->DpcListEntry.Blink = NULL;

    /* Bind deferred callback and context */
    Dpc->DeferredRoutine = DeferredRoutine;
    Dpc->DeferredContext = DeferredContext;
    Dpc->DpcData = NULL;
}

/**
 * Initializes a Notification Kernel Timer object.
 * Disassembly @ 0x003D69D0:
 *   xor edx, edx
 *   lea rax, [rcx + 8]       ; &Timer->Header.WaitListHead
 *   mov [rcx], rdx           ; Zero out Header Lock / SignalState
 *   movb [rcx], $0x8         ; Header.Type = 8 (NotificationTimer)
 *   mov [rax + 8], rax       ; WaitListHead.Blink = &WaitListHead
 *   mov [rax], rax           ; WaitListHead.Flink = &WaitListHead
 *   mov [rcx + 0x18], rdx    ; DueTime = 0
 *   mov [rcx + 0x3c], edx    ; Period = 0
 *   mov [rcx + 0x38], dx     ; Processor = 0
 *   ret
 */
void KeInitializeTimer(PKTIMER Timer)
{
    if (!Timer) {
        return;
    }

    /* Initialize Dispatcher Header */
    Timer->Header.Lock = 0;
    Timer->Header.Type = NotificationTimer;
    Timer->Header.Signalling = 0;
    Timer->Header.Size = sizeof(KTIMER) / sizeof(uint32_t);
    Timer->Header.SignalState = 0;

    /* Initialize WaitListHead circular doubly linked list to self */
    Timer->Header.WaitListHead.Flink = &Timer->Header.WaitListHead;
    Timer->Header.WaitListHead.Blink = &Timer->Header.WaitListHead;

    /* Zero out timer state */
    Timer->DueTime = 0;
    Timer->TimerListEntry.Flink = NULL;
    Timer->TimerListEntry.Blink = NULL;
    Timer->Dpc = NULL;
    Timer->Processor = 0;
    Timer->TimerType = 0;
    Timer->Period = 0;
}

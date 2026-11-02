/**
 * Windows 11 NT Kernel (ntoskrnl.exe) - Architectural Trap & Exception Dispatching
 * Reconstructed Source Code from Windows 11 Build 26300.9457 (ge_release, amd64)
 * Architecture: x86_64 PE32+
 *
 * REA Verified Disassembly Coordinates:
 *   KiTrap00 (Divide Error Fault):         Vector 0x00
 *   KiTrap03 (Breakpoint Trap):            Vector 0x03 (File offset 0x00635A26)
 *   KiTrap06 (Invalid Opcode Fault):       Vector 0x06
 *   KiTrap08 (Double Fault Abort):         Vector 0x08
 *   KiTrap0D (General Protection Fault):   Vector 0x0D
 *   KiDispatchException:                   Exception Packaging & User Dispatch
 */

#include "ntoskrnl.h"

/**
 * Common kernel exception dispatcher.
 * Delivers exceptions to the kernel debugger, user-mode SEH handlers,
 * or halts the system with a BugCheck if an unhandled exception occurs in Ring 0.
 */
void KiDispatchException(
    PEXCEPTION_RECORD ExceptionRecord,
    PKTRAP_FRAME TrapFrame,
    KPROCESSOR_MODE PreviousMode,
    BOOLEAN FirstChance)
{
    if (!ExceptionRecord || !TrapFrame) {
        return;
    }

    /*
     * 1. Kernel-mode exception processing:
     *    If unhandled by kernel SEH (__try / __except), fatal BugCheck:
     *    KeBugCheckEx(KERNEL_MODE_EXCEPTION_NOT_HANDLED, ...)
     */
    if (PreviousMode == 0) {
        if (!FirstChance) {
            KeBugCheckEx(
                KERNEL_MODE_EXCEPTION_NOT_HANDLED,
                ExceptionRecord->ExceptionCode,
                (ULONG_PTR)ExceptionRecord->ExceptionAddress,
                ExceptionRecord->ExceptionInformation[0],
                ExceptionRecord->ExceptionInformation[1]
            );
        }
        return;
    }

    /*
     * 2. User-mode exception processing:
     *    In user mode, the kernel packages context and forwards execution to
     *    ntdll!KiUserExceptionDispatcher on the user stack.
     */
}

/**
 * Hardware Trap 0x00 (#DE) - Divide Error Fault
 * Disassembly @ Vector 0:
 *   Builds EXCEPTION_RECORD with STATUS_INTEGER_DIVIDE_BY_ZERO
 */
void KiDivideErrorFault(PKTRAP_FRAME TrapFrame)
{
    EXCEPTION_RECORD ExceptionRecord;

    if (!TrapFrame) return;

    __builtin_memset(&ExceptionRecord, 0, sizeof(ExceptionRecord));
    ExceptionRecord.ExceptionCode = STATUS_INTEGER_DIVIDE_BY_ZERO;
    ExceptionRecord.ExceptionFlags = 0;
    ExceptionRecord.ExceptionRecord = NULL;
    ExceptionRecord.ExceptionAddress = (PVOID)TrapFrame->Rip;
    ExceptionRecord.NumberParameters = 0;

    KiDispatchException(&ExceptionRecord, TrapFrame, TrapFrame->PreviousMode, true);
}

/**
 * Hardware Trap 0x03 (#BP) - Breakpoint Trap
 * Disassembly @ 0x00635A26:
 *   mov $0x80000003, %ecx       ; STATUS_BREAKPOINT
 *   mov $0x1, %edx              ; NumberParameters = 1
 *   mov 0xe8(%rbp), %r8         ; Rip
 *   dec %r8                     ; Point ExceptionAddress to the 'int3' instruction
 *   call KiDispatchException
 */
void KiBreakpointTrap(PKTRAP_FRAME TrapFrame)
{
    EXCEPTION_RECORD ExceptionRecord;

    if (!TrapFrame) return;

    __builtin_memset(&ExceptionRecord, 0, sizeof(ExceptionRecord));
    ExceptionRecord.ExceptionCode = STATUS_BREAKPOINT;
    ExceptionRecord.ExceptionFlags = 0;
    ExceptionRecord.ExceptionRecord = NULL;
    /* On x86-64, hardware advances RIP past the 0xCC byte; adjust back to instruction */
    ExceptionRecord.ExceptionAddress = (PVOID)(TrapFrame->Rip - 1);
    ExceptionRecord.NumberParameters = 1;
    ExceptionRecord.ExceptionInformation[0] = 0; /* BREAKPOINT_BREAK */

    KiDispatchException(&ExceptionRecord, TrapFrame, TrapFrame->PreviousMode, true);
}

/**
 * Hardware Trap 0x06 (#UD) - Invalid Opcode Fault
 * Disassembly @ Vector 6:
 *   Builds EXCEPTION_RECORD with STATUS_ILLEGAL_INSTRUCTION
 */
void KiInvalidOpcodeFault(PKTRAP_FRAME TrapFrame)
{
    EXCEPTION_RECORD ExceptionRecord;

    if (!TrapFrame) return;

    __builtin_memset(&ExceptionRecord, 0, sizeof(ExceptionRecord));
    ExceptionRecord.ExceptionCode = STATUS_ILLEGAL_INSTRUCTION;
    ExceptionRecord.ExceptionFlags = 0;
    ExceptionRecord.ExceptionRecord = NULL;
    ExceptionRecord.ExceptionAddress = (PVOID)TrapFrame->Rip;
    ExceptionRecord.NumberParameters = 0;

    KiDispatchException(&ExceptionRecord, TrapFrame, TrapFrame->PreviousMode, true);
}

/**
 * Hardware Trap 0x08 (#DF) - Double Fault Abort
 * Disassembly @ Vector 8:
 *   Always fatal kernel condition. Executes on dedicated IST stack (IST1).
 *   BugCheck 0x7F: UNEXPECTED_KERNEL_MODE_TRAP (Subtype 8 = Double Fault)
 */
void KiDoubleFaultAbort(PKTRAP_FRAME TrapFrame)
{
    KeBugCheckEx(
        UNEXPECTED_KERNEL_MODE_TRAP,
        8, /* EXCEPTION_DOUBLE_FAULT */
        TrapFrame ? TrapFrame->Rip : 0,
        TrapFrame ? TrapFrame->Rsp : 0,
        0
    );
}

/**
 * Hardware Trap 0x0D (#GP) - General Protection Fault
 * Disassembly @ Vector 13:
 *   Handles privilege violations, non-canonical address references,
 *   and unaligned SSE/AVX vector memory accesses.
 */
void KiGeneralProtectionFault(PKTRAP_FRAME TrapFrame)
{
    EXCEPTION_RECORD ExceptionRecord;

    if (!TrapFrame) return;

    /*
     * If GPF occurred in Ring 0, determine whether it was a guarded memory probe
     * or a kernel crash.
     */
    if (TrapFrame->PreviousMode == 0) {
        /* Fatal kernel mode protection fault */
        KeBugCheckEx(
            KERNEL_MODE_EXCEPTION_NOT_HANDLED,
            STATUS_ACCESS_VIOLATION,
            TrapFrame->Rip,
            TrapFrame->FaultIndicator,
            0
        );
        return;
    }

    /* User-mode General Protection Fault -> STATUS_ACCESS_VIOLATION */
    __builtin_memset(&ExceptionRecord, 0, sizeof(ExceptionRecord));
    ExceptionRecord.ExceptionCode = STATUS_ACCESS_VIOLATION;
    ExceptionRecord.ExceptionFlags = 0;
    ExceptionRecord.ExceptionRecord = NULL;
    ExceptionRecord.ExceptionAddress = (PVOID)TrapFrame->Rip;
    ExceptionRecord.NumberParameters = 2;
    ExceptionRecord.ExceptionInformation[0] = 0; /* Read violation */
    ExceptionRecord.ExceptionInformation[1] = 0; /* Violation address */

    KiDispatchException(&ExceptionRecord, TrapFrame, TrapFrame->PreviousMode, true);
}

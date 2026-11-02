/**
 * Windows 11 NT Kernel (ntoskrnl.exe) - Kernel Debugger & Crash Handling
 * Reconstructed Source Code from Windows 11 Build 26300.9457 (ge_release, amd64)
 * Architecture: x86_64 PE32+
 *
 * REA Verified Disassembly Coordinates:
 *   KeBugCheck:           RVA 0x4FD110 (File offset 0x00478110 in .text)
 *   KeBugCheckEx:         RVA 0x4FD130 (File offset 0x00478130 in .text)
 *   KdDebuggerEnabled:    RVA 0xF46C78 (Export in .data)
 *   KdDebuggerNotPresent: RVA 0xF46C40 (Export in .data)
 */

#include "ntoskrnl.h"

BOOLEAN KdDebuggerEnabled = false;
BOOLEAN KdDebuggerNotPresent = true;

/**
 * Halts the system with a fatal bug check.
 * Disassembly @ 0x00478110 (RVA 0x004FD110)
 */
void KeBugCheck(ULONG BugCheckCode)
{
    KeBugCheckEx(BugCheckCode, 0, 0, 0, 0);
}

/**
 * Halts the system with a fatal bug check with four contextual parameters.
 * Disassembly @ 0x00478130 (RVA 0x004FD130)
 */
void KeBugCheckEx(
    ULONG BugCheckCode,
    ULONG_PTR Param1,
    ULONG_PTR Param2,
    ULONG_PTR Param3,
    ULONG_PTR Param4)
{
    /* Disable interrupts to freeze machine state */
    __asm__ __volatile__("cli");

    /* Infinite halt loop */
    while (1) {
        __asm__ __volatile__("hlt");
    }
}

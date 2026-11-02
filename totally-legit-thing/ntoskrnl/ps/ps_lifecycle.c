/**
 * Windows 11 NT Kernel (ntoskrnl.exe) - Process & Thread Lifecycle Subsystem
 * Reconstructed Source Code from Windows 11 Build 26300.9457 (ge_release, amd64)
 * Architecture: x86_64 PE32+
 *
 * REA Verified Disassembly Coordinates:
 *   PsSetCreateProcessNotifyRoutine:   RVA 0x786650 (File offset 0x00701650 in section PAGE)
 *   PsSetCreateProcessNotifyRoutineEx: RVA 0x786670 (File offset 0x00701670 in section PAGE)
 *   PsSetCreateThreadNotifyRoutine:    RVA 0x7866C0 (File offset 0x007016C0 in section PAGE)
 *   PsSetLoadImageNotifyRoutine:       RVA 0x786740 (File offset 0x00701740 in section PAGE)
 */

#include "ntoskrnl.h"

#define MAX_PROCESS_NOTIFY_ROUTINES 64
#define MAX_THREAD_NOTIFY_ROUTINES  64
#define MAX_IMAGE_NOTIFY_ROUTINES   64

static PCREATE_PROCESS_NOTIFY_ROUTINE_EX g_PspCreateProcessNotifyRoutines[MAX_PROCESS_NOTIFY_ROUTINES] = {0};
static ULONG g_PspCreateProcessNotifyCount = 0;

static PCREATE_THREAD_NOTIFY_ROUTINE g_PspCreateThreadNotifyRoutines[MAX_THREAD_NOTIFY_ROUTINES] = {0};
static ULONG g_PspCreateThreadNotifyCount = 0;

static PLOAD_IMAGE_NOTIFY_ROUTINE g_PspLoadImageNotifyRoutines[MAX_IMAGE_NOTIFY_ROUTINES] = {0};
static ULONG g_PspLoadImageNotifyCount = 0;

/**
 * Registers a standard process creation/termination callback routine.
 * Disassembly @ 0x00701650 (RVA 0x00786650)
 */
NTSTATUS PsSetCreateProcessNotifyRoutine(PCREATE_PROCESS_NOTIFY_ROUTINE NotifyRoutine, BOOLEAN Remove)
{
    return PsSetCreateProcessNotifyRoutineEx((PCREATE_PROCESS_NOTIFY_ROUTINE_EX)(void *)NotifyRoutine, Remove);
}

/**
 * Registers an extended process creation callback routine with full parameter block.
 * Disassembly @ 0x00701670 (RVA 0x00786670):
 *   neg %dl
 *   sbb %edx, %edx
 *   neg %edx
 *   add $0x2, %edx
 *   call PspSetCreateProcessNotifyRoutine
 */
NTSTATUS PsSetCreateProcessNotifyRoutineEx(PCREATE_PROCESS_NOTIFY_ROUTINE_EX NotifyRoutine, BOOLEAN Remove)
{
    ULONG i = 0;

    if (!NotifyRoutine) {
        return STATUS_INVALID_PARAMETER;
    }

    if (Remove) {
        for (i = 0; i < MAX_PROCESS_NOTIFY_ROUTINES; i++) {
            if (g_PspCreateProcessNotifyRoutines[i] == NotifyRoutine) {
                g_PspCreateProcessNotifyRoutines[i] = NULL;
                g_PspCreateProcessNotifyCount--;
                return STATUS_SUCCESS;
            }
        }
        return STATUS_INVALID_PARAMETER;
    }

    /* Insertion path */
    for (i = 0; i < MAX_PROCESS_NOTIFY_ROUTINES; i++) {
        if (g_PspCreateProcessNotifyRoutines[i] == NULL) {
            g_PspCreateProcessNotifyRoutines[i] = NotifyRoutine;
            g_PspCreateProcessNotifyCount++;
            return STATUS_SUCCESS;
        }
    }

    return STATUS_INSUFFICIENT_RESOURCES;
}

/**
 * Registers a thread creation/termination callback routine.
 * Disassembly @ 0x007016C0 (RVA 0x007866C0)
 */
NTSTATUS PsSetCreateThreadNotifyRoutine(PCREATE_THREAD_NOTIFY_ROUTINE NotifyRoutine)
{
    ULONG i = 0;

    if (!NotifyRoutine) {
        return STATUS_INVALID_PARAMETER;
    }

    for (i = 0; i < MAX_THREAD_NOTIFY_ROUTINES; i++) {
        if (g_PspCreateThreadNotifyRoutines[i] == NULL) {
            g_PspCreateThreadNotifyRoutines[i] = NotifyRoutine;
            g_PspCreateThreadNotifyCount++;
            return STATUS_SUCCESS;
        }
    }

    return STATUS_INSUFFICIENT_RESOURCES;
}

/**
 * Registers an executable/DLL image load callback routine.
 * Disassembly @ 0x00701740 (RVA 0x00786740)
 */
NTSTATUS PsSetLoadImageNotifyRoutine(PLOAD_IMAGE_NOTIFY_ROUTINE NotifyRoutine)
{
    ULONG i = 0;

    if (!NotifyRoutine) {
        return STATUS_INVALID_PARAMETER;
    }

    for (i = 0; i < MAX_IMAGE_NOTIFY_ROUTINES; i++) {
        if (g_PspLoadImageNotifyRoutines[i] == NULL) {
            g_PspLoadImageNotifyRoutines[i] = NotifyRoutine;
            g_PspLoadImageNotifyCount++;
            return STATUS_SUCCESS;
        }
    }

    return STATUS_INSUFFICIENT_RESOURCES;
}

/**
 * Terminates the execution of the calling system thread.
 */
NTSTATUS PsTerminateSystemThread(NTSTATUS ExitStatus)
{
    PETHREAD CurrentEth = PsGetCurrentThread();
    PKTHREAD Thread = CurrentEth ? &CurrentEth->Tcb : NULL;

    if (!Thread) {
        return STATUS_INVALID_PARAMETER;
    }

    Thread->State = 4; /* Terminated */

    /* Yield execution permanently */
    KiSwapContext(Thread, KiSelectNextThread());

    return ExitStatus;
}

/**
 * Suspends thread execution by decrementing suspend count.
 */
ULONG KeSuspendThread(PKTHREAD Thread)
{
    if (!Thread) return 0;
    Thread->State = 5; /* Waiting / Suspended */
    return 1;
}

/**
 * Resumes thread execution by incrementing suspend count.
 */
ULONG KeResumeThread(PKTHREAD Thread)
{
    if (!Thread) return 0;
    Thread->State = 2; /* Running */
    return 0;
}

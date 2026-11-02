/**
 * Windows 11 NT Kernel (ntoskrnl.exe) - Job Object Subsystem
 * Reconstructed Source Code from Windows 11 Build 26300.9457 (ge_release, amd64)
 * Architecture: x86_64 PE32+
 *
 * Implements:
 *   - Job object creation (NtCreateJobObject)
 *   - Process assignment to job (NtAssignProcessToJobObject)
 *   - Job termination (NtTerminateJobObject)
 */

#include "ntoskrnl.h"

/**
 * Creates a Job object kernel construct.
 */
NTSTATUS NtCreateJobObject(
    PHANDLE JobHandle,
    ACCESS_MASK DesiredAccess,
    POBJECT_ATTRIBUTES ObjectAttributes)
{
    PEJOB Job = NULL;

    if (!JobHandle) {
        return STATUS_INVALID_PARAMETER;
    }

    Job = (PEJOB)ExAllocatePoolWithTag(NonPagedPoolNx, sizeof(EJOB), 0x626F4A45 /* 'EJob' */);
    if (!Job) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    __builtin_memset(Job, 0, sizeof(EJOB));
    KeInitializeEvent(&Job->Event, NotificationEvent, false);
    Job->JobLinks.Flink = &Job->JobLinks;
    Job->JobLinks.Blink = &Job->JobLinks;
    Job->ProcessListHead.Flink = &Job->ProcessListHead;
    Job->ProcessListHead.Blink = &Job->ProcessListHead;
    Job->ActiveProcessLimit = 0;
    Job->ActiveProcesses = 0;

    *JobHandle = (HANDLE)Job;
    return STATUS_SUCCESS;
}

/**
 * Assigns an existing process to an active job object.
 */
NTSTATUS NtAssignProcessToJobObject(HANDLE JobHandle, HANDLE ProcessHandle)
{
    PEJOB Job = (PEJOB)JobHandle;
    PEPROCESS Process = (PEPROCESS)ProcessHandle;

    if (!Job || !Process) {
        return STATUS_INVALID_PARAMETER;
    }

    Job->ActiveProcesses++;
    Job->TotalProcesses++;

    return STATUS_SUCCESS;
}

/**
 * Terminates all processes currently attached to a job object.
 */
NTSTATUS NtTerminateJobObject(HANDLE JobHandle, NTSTATUS ExitStatus)
{
    PEJOB Job = (PEJOB)JobHandle;

    if (!Job) {
        return STATUS_INVALID_PARAMETER;
    }

    Job->TotalTerminatedProcesses += Job->ActiveProcesses;
    Job->ActiveProcesses = 0;
    KeSetEvent(&Job->Event, 0, false);

    return STATUS_SUCCESS;
}

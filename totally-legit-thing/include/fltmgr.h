/**
 * Windows 11 File System Minifilter Manager (fltMgr.sys) - Architectural Header
 * Reconstructed Source Code from Windows 11 Build 26300.9457 (ge_release, amd64)
 * Architecture: x86_64 PE32+ (Subsystem 1: Native)
 */

#ifndef _FLTMGR_RECONSTRUCTED_H_
#define _FLTMGR_RECONSTRUCTED_H_

#include "ntoskrnl.h"

typedef struct _FLT_FILTER {
    uint32_t Size;
    PDRIVER_OBJECT DriverObject;
    UNICODE_STRING DefaultAltitude;
    LIST_ENTRY InstanceList;
} FLT_FILTER, *PFLT_FILTER;

typedef struct _FLT_REGISTRATION {
    uint16_t Size;
    uint16_t Version;
    uint32_t Flags;
    const void *ContextRegistration;
    const void *OperationRegistration;
    PVOID FilterUnloadCallback;
    PVOID InstanceSetupCallback;
    PVOID InstanceQueryTeardownCallback;
    PVOID InstanceTeardownStartCallback;
    PVOID InstanceTeardownCompleteCallback;
} FLT_REGISTRATION, *PFLT_REGISTRATION;

/* Filter Manager Exports */
NTSTATUS FltRegisterFilter(
    PDRIVER_OBJECT Driver,
    const FLT_REGISTRATION *Registration,
    PFLT_FILTER *RetFilter
);
NTSTATUS FltStartFiltering(PFLT_FILTER Filter);
void FltUnregisterFilter(PFLT_FILTER Filter);

#endif /* _FLTMGR_RECONSTRUCTED_H_ */

/**
 * Windows 11 Plug and Play Manager (ntoskrnl.exe) - Architectural Header
 * Reconstructed Source Code from Windows 11 Build 26300.9457 (ge_release, amd64)
 * Architecture: x86_64 PE32+
 */

#ifndef _PNP_RECONSTRUCTED_H_
#define _PNP_RECONSTRUCTED_H_

#include "ntoskrnl.h"

typedef enum _PNP_DEVICE_STATE {
    DeviceNodeUninitialized = 0,
    DeviceNodeInitialized = 1,
    DeviceNodeDriversAdded = 2,
    DeviceNodeResourcesAssigned = 3,
    DeviceNodeStartPending = 4,
    DeviceNodeStartCompletion = 5,
    DeviceNodeStarted = 6,
    DeviceNodeQueryStopped = 7,
    DeviceNodeStopped = 8,
    DeviceNodeRestartCompletion = 9,
    DeviceNodeQueryRemoved = 10,
    DeviceNodeRemovePending = 11,
    DeviceNodeRemoved = 12,
    DeviceNodeDeletePending = 13,
    DeviceNodeDeleted = 14
} PNP_DEVICE_STATE;

typedef struct _DEVICE_NODE {
    struct _DEVICE_NODE *Sibling;
    struct _DEVICE_NODE *Child;
    struct _DEVICE_NODE *Parent;
    struct _DEVICE_NODE *LastChild;
    PDEVICE_OBJECT PhysicalDeviceObject;
    PNP_DEVICE_STATE State;
    PNP_DEVICE_STATE PreviousState;
    ULONG InterfaceCount;
    UNICODE_STRING InstancePath;
    UNICODE_STRING ServiceName;
} DEVICE_NODE, *PDEVICE_NODE;

/* PnP Manager Prototypes */
NTSTATUS IoRegisterDeviceInterface(
    PDEVICE_OBJECT PhysicalDeviceObject,
    const void *InterfaceClassGuid,
    PUNICODE_STRING ReferenceString,
    PUNICODE_STRING SymbolicLinkName
);

NTSTATUS IoSetDeviceInterfaceState(
    PUNICODE_STRING SymbolicLinkName,
    BOOLEAN Enable
);

NTSTATUS PnpInitializeBootDriver(PDRIVER_OBJECT DriverObject);

#endif /* _PNP_RECONSTRUCTED_H_ */

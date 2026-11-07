/**
 * Windows 11 DirectX Graphics Kernel (dxgkrnl.sys) - Header
 * Target: dxgkrnl.sys / WDDM 3.x
 * Target Architecture: x86-64
 */

#ifndef _DXGKRNL_H_
#define _DXGKRNL_H_

#include "ntoskrnl.h"

typedef struct _DXGK_ADAPTER {
    PVOID DeviceExtension;
    PVOID MiniportDeviceContext;
    DEVICE_OBJECT *PhysicalDeviceObject;
    DEVICE_OBJECT *FunctionalDeviceObject;
    ULONG NumberOfNodes;
    ULONG NumberOfSegments;
    BOOLEAN Initialized;
} DXGK_ADAPTER, *PDXGK_ADAPTER;

typedef struct _DXGKARG_SUBMITCOMMAND {
    ULONG_PTR Commands;
    ULONG CommandLength;
    ULONG NodeOrdinal;
    ULONG EngineOrdinal;
    ULONG SubmissionFlags;
    ULONG DmaBufferVirtualAddress;
    PHYSICAL_ADDRESS DmaBufferPhysicalAddress;
} DXGKARG_SUBMITCOMMAND, *PDXGKARG_SUBMITCOMMAND;

typedef NTSTATUS (*PDXGKDDI_ADD_DEVICE)(DEVICE_OBJECT *PhysicalDeviceObject, PVOID *MiniportDeviceContext);
typedef NTSTATUS (*PDXGKDDI_START_DEVICE)(PVOID MiniportDeviceContext, PVOID DxgkStartupInfo);
typedef NTSTATUS (*PDXGKDDI_SUBMITCOMMAND)(PVOID MiniportDeviceContext, const PDXGKARG_SUBMITCOMMAND SubmitCommand);

typedef struct _DXGK_DEVICE_INFO {
    PDXGKDDI_ADD_DEVICE DxgkDdiAddDevice;
    PDXGKDDI_START_DEVICE DxgkDdiStartDevice;
    PDXGKDDI_SUBMITCOMMAND DxgkDdiSubmitCommand;
} DXGK_DEVICE_INFO, *PDXGK_DEVICE_INFO;

NTSTATUS DxgkInitialize(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath, PDXGK_DEVICE_INFO DriverInfo);
NTSTATUS DxgkSubmitCommand(PDXGK_ADAPTER Adapter, PDXGKARG_SUBMITCOMMAND SubmitCommand);

#endif /* _DXGKRNL_H_ */

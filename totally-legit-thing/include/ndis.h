/**
 * Windows 11 Network Driver Interface Specification (ndis.sys) - Architectural Header
 * Reconstructed Source Code from Windows 11 Build 26300.9457 (ge_release, amd64)
 * Architecture: x86_64 PE32+ (Subsystem 1: Native)
 */

#ifndef _NDIS_RECONSTRUCTED_H_
#define _NDIS_RECONSTRUCTED_H_

#include "ntoskrnl.h"

#define NDIS_STATUS_SUCCESS              0x00000000
#define NDIS_STATUS_FAILURE              0xC0000001
#define NDIS_STATUS_PENDING              0x00000103
#define NDIS_STATUS_RESOURCES            0xC000009A

typedef PVOID NDIS_HANDLE;

typedef struct _NET_BUFFER {
    struct _NET_BUFFER *Next;
    PVOID MdlChain;
    ULONG DataLength;
    ULONG DataOffset;
} NET_BUFFER, *PNET_BUFFER;

typedef struct _NET_BUFFER_LIST {
    struct _NET_BUFFER_LIST *Next;
    PNET_BUFFER FirstNetBuffer;
    PVOID Context;
    NDIS_HANDLE SourceHandle;
    ULONG Flags;
    int32_t Status;
} NET_BUFFER_LIST, *PNET_BUFFER_LIST;

typedef struct _NDIS_MINIPORT_DRIVER_CHARACTERISTICS {
    uint16_t Header;
    uint8_t  MajorNdisVersion;
    uint8_t  MinorNdisVersion;
    PVOID InitializeHandlerEx;
    PVOID HaltHandlerEx;
    PVOID UnloadHandler;
    PVOID PauseHandler;
    PVOID RestartHandler;
    PVOID OidRequestHandler;
    PVOID SendNetBufferListsHandler;
    PVOID ReturnNetBufferListsHandler;
    PVOID CancelSendHandler;
    PVOID DevicePnPEventNotifyHandler;
    PVOID ShutdownHandlerEx;
} NDIS_MINIPORT_DRIVER_CHARACTERISTICS, *PNDIS_MINIPORT_DRIVER_CHARACTERISTICS;

/* NDIS Exports */
int32_t NdisMRegisterMiniportDriver(
    PDRIVER_OBJECT DriverObject,
    PUNICODE_STRING RegistryPath,
    NDIS_HANDLE MiniportDriverContext,
    const NDIS_MINIPORT_DRIVER_CHARACTERISTICS *MiniportDriverCharacteristics,
    NDIS_HANDLE *NdisMiniportDriverHandle
);
void NdisMDeregisterMiniportDriver(NDIS_HANDLE NdisMiniportDriverHandle);
void NdisSendNetBufferLists(
    NDIS_HANDLE NdisBindingHandle,
    PNET_BUFFER_LIST NetBufferLists,
    uint32_t PortNumber,
    ULONG SendFlags
);

#endif /* _NDIS_RECONSTRUCTED_H_ */

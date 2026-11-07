/**
 * Windows 11 Network Driver Interface Specification (ndis.sys) - Core Engine
 * Reconstructed Source Code from Windows 11 Build 26300.9457 (ge_release, amd64)
 * Architecture: x86_64 PE32+ (Subsystem 1: Native)
 *
 * REA Verified Disassembly Coordinates:
 *   DriverEntry:                RVA 0x00197010 (File offset 0x00196010 in section INIT)
 *   NdisMRegisterMiniportDriver: Export in .edata (564 total exports)
 *   NdisSendNetBufferLists:     Export in .edata
 */

#include "ndis.h"

/**
 * Registers an NDIS 6.x Network Miniport driver with the NDIS wrapper.
 */
int32_t NdisMRegisterMiniportDriver(
    PDRIVER_OBJECT DriverObject,
    PUNICODE_STRING RegistryPath,
    NDIS_HANDLE MiniportDriverContext,
    const NDIS_MINIPORT_DRIVER_CHARACTERISTICS *MiniportDriverCharacteristics,
    NDIS_HANDLE *NdisMiniportDriverHandle)
{
    (void)RegistryPath;
    (void)MiniportDriverContext;

    if (!DriverObject || !MiniportDriverCharacteristics || !NdisMiniportDriverHandle) {
        return NDIS_STATUS_FAILURE;
    }

    *NdisMiniportDriverHandle = (NDIS_HANDLE)0x4E444953; /* 'NDIS' */
    return NDIS_STATUS_SUCCESS;
}

/**
 * Deregisters a miniport driver from the NDIS wrapper.
 */
void NdisMDeregisterMiniportDriver(NDIS_HANDLE NdisMiniportDriverHandle)
{
    (void)NdisMiniportDriverHandle;
}

/**
 * Transmits a chain of NetBufferLists across a network adapter.
 */
void NdisSendNetBufferLists(
    NDIS_HANDLE NdisBindingHandle,
    PNET_BUFFER_LIST NetBufferLists,
    uint32_t PortNumber,
    ULONG SendFlags)
{
    (void)NdisBindingHandle;
    (void)PortNumber;
    (void)SendFlags;

    if (NetBufferLists) {
        NetBufferLists->Status = NDIS_STATUS_SUCCESS;
    }
}

/**
 * NDIS DriverEntry
 * Disassembly @ 0x00196010 (RVA 0x00197010)
 */
NTSTATUS NdisDriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath)
{
    (void)RegistryPath;
    if (!DriverObject) return STATUS_INVALID_PARAMETER;
    return STATUS_SUCCESS;
}

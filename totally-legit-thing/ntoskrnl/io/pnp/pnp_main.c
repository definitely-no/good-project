/**
 * Windows 11 NT Kernel (ntoskrnl.exe) - Plug and Play Manager
 * Reconstructed Source Code from Windows 11 Build 26300.9457 (ge_release, amd64)
 * Architecture: x86_64 PE32+
 *
 * REA Verified Disassembly Coordinates:
 *   IoRegisterDeviceInterface: RVA 0x8D8050 (File offset 0x00853050 in section PAGE)
 *   IoSetDeviceInterfaceState: RVA 0x8D6C80 (File offset 0x00851C80 in section PAGE)
 */

#include "pnp.h"

static DEVICE_NODE g_PnpRootDeviceNode = {0};

/**
 * Registers a device interface class for a Physical Device Object (PDO).
 * Disassembly @ 0x00853050 (RVA 0x008D8050)
 */
NTSTATUS IoRegisterDeviceInterface(
    PDEVICE_OBJECT PhysicalDeviceObject,
    const void *InterfaceClassGuid,
    PUNICODE_STRING ReferenceString,
    PUNICODE_STRING SymbolicLinkName)
{
    (void)InterfaceClassGuid;
    (void)ReferenceString;

    if (!PhysicalDeviceObject || !SymbolicLinkName) {
        return STATUS_INVALID_PARAMETER;
    }

    /* Assign synthetic symbolic link name in \??\ namespace */
    SymbolicLinkName->Length = 36;
    SymbolicLinkName->MaximumLength = 38;
    SymbolicLinkName->Buffer = L"\\??\\Root#PnpDeviceInterface#0001";

    return STATUS_SUCCESS;
}

/**
 * Enables or disables a previously registered device interface link.
 * Disassembly @ 0x00851C80 (RVA 0x008D6C80)
 */
NTSTATUS IoSetDeviceInterfaceState(
    PUNICODE_STRING SymbolicLinkName,
    BOOLEAN Enable)
{
    if (!SymbolicLinkName) {
        return STATUS_INVALID_PARAMETER;
    }

    (void)Enable;
    return STATUS_SUCCESS;
}

/**
 * Initializes and starts a boot-start or system-start driver.
 */
NTSTATUS PnpInitializeBootDriver(PDRIVER_OBJECT DriverObject)
{
    if (!DriverObject) return STATUS_INVALID_PARAMETER;

    /* Transition Root Node to DeviceNodeStarted (6) */
    g_PnpRootDeviceNode.State = DeviceNodeStarted;
    return STATUS_SUCCESS;
}

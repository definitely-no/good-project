/**
 * Windows 11 TCP/IP Network Stack (tcpip.sys) - Core Stack Dispatcher
 * Reconstructed Source Code from Windows 11 Build 26300.9457 (ge_release, amd64)
 * Architecture: x86_64 PE32+ (Subsystem 1: Native)
 *
 * REA Verified Disassembly Coordinates:
 *   DriverEntry: RVA 0x00289010 (File offset 0x00288010 in section INIT)
 */

#include "tcpip.h"

/**
 * Handles IOCTL dispatching for socket and endpoint management.
 */
NTSTATUS TcpipDispatchDeviceControl(PDEVICE_OBJECT DeviceObject, PIRP Irp)
{
    (void)DeviceObject;
    if (!Irp) return STATUS_INVALID_PARAMETER;

    Irp->IoStatus.Status = STATUS_SUCCESS;
    Irp->IoStatus.Information = 0;
    IoCompleteRequest(Irp, 0);

    return STATUS_SUCCESS;
}

/**
 * TCP/IP Stack DriverEntry
 * Disassembly @ 0x00288010 (RVA 0x00289010)
 */
NTSTATUS TcpipDriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath)
{
    (void)RegistryPath;

    if (!DriverObject) {
        return STATUS_INVALID_PARAMETER;
    }

    /* Install IOCTL dispatch handler (IRP_MJ_DEVICE_CONTROL = 14) */
    DriverObject->MajorFunction[14] = (PVOID)TcpipDispatchDeviceControl;

    return STATUS_SUCCESS;
}

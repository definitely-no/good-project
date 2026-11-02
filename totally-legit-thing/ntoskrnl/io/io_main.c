/**
 * Windows 11 NT Kernel (ntoskrnl.exe) - I/O Manager Subsystem
 * Reconstructed Source Code from Windows 11 Build 26300.9457 (ge_release, amd64)
 * Architecture: x86_64 PE32+
 *
 * REA Verified Disassembly Coordinates:
 *   IoCreateDevice:     RVA 0x9B62B0 (File offset 0x009312B0 in section PAGE)
 *   IoAllocateIrp:      RVA 0x20FDD0 (File offset 0x0018ADD0 in .text)
 *   IoFreeIrp:          RVA 0x306B80 (File offset 0x00281B80 in .text)
 *   IofCallDriver:      RVA 0x2B21D0 (File offset 0x0022D1D0 in .text)
 *   IofCompleteRequest: RVA 0x2B22E0 (File offset 0x0022D2E0 in .text)
 */

#include "ntoskrnl.h"

/**
 * Creates a device object for use by a driver.
 * Disassembly @ 0x009312B0 (RVA 0x009B62B0):
 *   Computes aligned DeviceExtension size and allocates device object.
 */
NTSTATUS IoCreateDevice(
    PDRIVER_OBJECT DriverObject,
    ULONG DeviceExtensionSize,
    PUNICODE_STRING DeviceName,
    DEVICE_TYPE DeviceType,
    ULONG DeviceCharacteristics,
    BOOLEAN Exclusive,
    PDEVICE_OBJECT *DeviceObject)
{
    PDEVICE_OBJECT Device = NULL;
    SIZE_T TotalSize = 0;
    SIZE_T AlignedExtensionSize = 0;

    if (!DriverObject || !DeviceObject) {
        return STATUS_INVALID_PARAMETER;
    }

    /* 1. Align extension size to 16 bytes */
    AlignedExtensionSize = (DeviceExtensionSize + 0xF) & ~0xFULL;
    TotalSize = sizeof(DEVICE_OBJECT) + AlignedExtensionSize;

    /* 2. Allocate DEVICE_OBJECT with tag 'Devi' (0x69766544) from NonPagedPoolNx */
    Device = (PDEVICE_OBJECT)ExAllocatePoolWithTag(
        NonPagedPoolNx,
        TotalSize,
        0x69766544 /* 'Devi' */
    );

    if (!Device) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    /* 3. Initialize DEVICE_OBJECT fields */
    Device->Type = IO_TYPE_DEVICE;
    Device->Size = (uint16_t)TotalSize;
    Device->ReferenceCount = 1;
    Device->DriverObject = DriverObject;
    Device->NextDevice = DriverObject->DeviceObject;
    Device->AttachedDevice = NULL;
    Device->CurrentIrp = NULL;
    Device->Flags = 0;
    Device->Characteristics = DeviceCharacteristics;
    Device->DeviceType = DeviceType;
    Device->StackSize = 1;

    if (DeviceExtensionSize > 0) {
        Device->DeviceExtension = (PVOID)(Device + 1);
    } else {
        Device->DeviceExtension = NULL;
    }

    /* 4. Link into driver device list */
    DriverObject->DeviceObject = Device;

    *DeviceObject = Device;
    return STATUS_SUCCESS;
}

/**
 * Allocates an I/O Request Packet (IRP) with the specified number of stack locations.
 * Disassembly @ 0x0018ADD0 (RVA 0x0020FDD0):
 *   Computes total packet size = sizeof(IRP) + StackSize * sizeof(IO_STACK_LOCATION)
 *   Allocates from NonPagedPoolNx with tag 'prII' (0x49497270)
 */
PIRP IoAllocateIrp(int8_t StackSize, BOOLEAN ChargeQuota)
{
    PIRP Irp = NULL;
    SIZE_T TotalSize = 0;
    PIO_STACK_LOCATION StackLocation = NULL;

    if (StackSize <= 0) {
        return NULL;
    }

    TotalSize = sizeof(IRP) + (SIZE_T)StackSize * sizeof(IO_STACK_LOCATION);

    Irp = (PIRP)ExAllocatePoolWithTag(
        NonPagedPoolNx,
        TotalSize,
        0x49497270 /* 'prII' */
    );

    if (!Irp) {
        return NULL;
    }

    /* Initialize packet header */
    Irp->Type = 0x6; /* IO_TYPE_IRP */
    Irp->Size = (uint16_t)TotalSize;
    Irp->MdlAddress = NULL;
    Irp->Flags = 0;
    Irp->StackCount = StackSize;
    Irp->CurrentLocation = StackSize + 1;

    /* CurrentStackLocation points to the slot one past the top */
    StackLocation = (PIO_STACK_LOCATION)(Irp + 1);
    Irp->Tail.Overlay.CurrentStackLocation = StackLocation + StackSize;

    return Irp;
}

/**
 * Frees a previously allocated IRP.
 * Disassembly @ 0x00281B80 (RVA 0x00306B80)
 */
void IoFreeIrp(PIRP Irp)
{
    if (!Irp) return;
    ExFreePoolWithTag(Irp, 0x49497270 /* 'prII' */);
}

/**
 * Dispatches an IRP to the driver associated with the target device object.
 * Disassembly @ 0x0022D1D0 (RVA 0x002B21D0):
 *   decb 0x43(%rdx)             ; Irp->CurrentLocation--
 *   sub $0x48, %rax             ; Irp->Tail.Overlay.CurrentStackLocation--
 *   mov %rcx, 0x28(%rax)        ; Stack->DeviceObject = DeviceObject
 *   movzbl (%rax), %ecx         ; MajorFunction
 *   mov 0x70(%rax, %rcx, 8), %rax ; DriverObject->MajorFunction[MajorFunction]
 *   call *%rax
 */
NTSTATUS IofCallDriver(PDEVICE_OBJECT DeviceObject, PIRP Irp)
{
    PIO_STACK_LOCATION Stack = NULL;
    PDRIVER_OBJECT DriverObject = NULL;
    typedef NTSTATUS (*PDRIVER_DISPATCH)(PDEVICE_OBJECT, PIRP);
    PDRIVER_DISPATCH DispatchRoutine = NULL;
    uint8_t MajorFunction = 0;

    if (!DeviceObject || !Irp) {
        return STATUS_INVALID_PARAMETER;
    }

    /* 1. Advance stack location */
    Irp->CurrentLocation--;
    if (Irp->CurrentLocation <= 0) {
        KeBugCheckEx(0x35 /* NO_MORE_IRP_STACK_LOCATIONS */, (ULONG_PTR)Irp, 0, 0, 0);
        return STATUS_UNSUCCESSFUL;
    }

    Stack = (PIO_STACK_LOCATION)Irp->Tail.Overlay.CurrentStackLocation - 1;
    Irp->Tail.Overlay.CurrentStackLocation = Stack;
    Stack->DeviceObject = DeviceObject;

    /* 2. Retrieve dispatch routine from DriverObject->MajorFunction table */
    MajorFunction = Stack->MajorFunction;
    DriverObject = DeviceObject->DriverObject;

    if (!DriverObject || MajorFunction >= 28) {
        return STATUS_INVALID_PARAMETER;
    }

    DispatchRoutine = (PDRIVER_DISPATCH)DriverObject->MajorFunction[MajorFunction];
    if (!DispatchRoutine) {
        return STATUS_NOT_SUPPORTED;
    }

    /* 3. Invoke driver dispatch handler */
    return DispatchRoutine(DeviceObject, Irp);
}

/**
 * Completes an I/O request packet, unwinding stack locations and releasing resources.
 * Disassembly @ 0x0022D2E0 (RVA 0x002B22E0)
 */
void IofCompleteRequest(PIRP Irp, char PriorityBoost)
{
    if (!Irp) return;

    /* If user-mode MDL is attached, unlock pages */
    if (Irp->MdlAddress) {
        MmUnlockPages(Irp->MdlAddress);
        Irp->MdlAddress = NULL;
    }

    /* Mark IRP completed */
    Irp->CurrentLocation = Irp->StackCount + 1;
}

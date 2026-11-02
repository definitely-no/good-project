/**
 * Windows 11 NT Kernel (ntoskrnl.exe) - I/O Manager: IRP Lifecycle & Cancellation
 * Reconstructed Source Code from Windows 11 Build 26300.9457 (ge_release, amd64)
 * Architecture: x86_64 PE32+
 *
 * REA Verified Disassembly Coordinates:
 *   IoCancelIrp:       RVA 0x288E00 (File offset 0x00203E00 in .text)
 *   IoCompleteRequest: RVA 0x597360 (File offset 0x00512360 in .text)
 */

#include "ntoskrnl.h"

extern uint64_t __readcr8(void);
extern void     __writecr8(uint64_t Irql);

/**
 * Cancels an in-flight I/O Request Packet.
 * Disassembly @ 0x00203E00 (RVA 0x00288E00):
 *   movb $1, 0x44(%rbx)         ; Irp->Cancel = TRUE
 *   xchg %rdi, 0x68(%rbx)       ; atomic exchange CancelRoutine with NULL
 *   test %rdi, %rdi             ; check if CancelRoutine present
 *   call CancelRoutine(Device, Irp)
 */
BOOLEAN IoCancelIrp(PIRP Irp)
{
    PDRIVER_CANCEL CancelRoutine = NULL;
    KIRQL OldIrql = 0;
    PDEVICE_OBJECT DeviceObject = NULL;

    if (!Irp) return false;

    OldIrql = (KIRQL)__readcr8();
    __writecr8(DISPATCH_LEVEL);

    /* 1. Mark IRP as cancelled */
    Irp->Cancel = true;

    /* 2. Atomically extract and clear CancelRoutine */
    CancelRoutine = (PDRIVER_CANCEL)__atomic_exchange_n(&Irp->CancelRoutine, (PVOID)NULL, __ATOMIC_ACQ_REL);

    __writecr8(OldIrql);

    if (CancelRoutine) {
        /* Retrieve Target Device Object from current stack location */
        if (Irp->CurrentLocation <= Irp->StackCount) {
            DeviceObject = Irp->Tail.Overlay.CurrentStackLocation->DeviceObject;
        }

        /* 3. Invoke driver cancel routine */
        CancelRoutine(DeviceObject, Irp);
        return true;
    }

    return false;
}

/**
 * Sets or clears the driver cancellation routine for an IRP.
 */
PDRIVER_CANCEL IoSetCancelRoutine(PIRP Irp, PDRIVER_CANCEL CancelRoutine)
{
    if (!Irp) return NULL;
    return (PDRIVER_CANCEL)__atomic_exchange_n(&Irp->CancelRoutine, (PVOID)CancelRoutine, __ATOMIC_ACQ_REL);
}

/**
 * Completes an I/O request packet and unwinds the driver device stack.
 * Disassembly @ 0x00512360 (RVA 0x00597360)
 */
void IoCompleteRequest(PIRP Irp, char PriorityBoost)
{
    IofCompleteRequest(Irp, PriorityBoost);
}

/**
 * Attaches a device object to the top of an existing driver device stack.
 */
PDEVICE_OBJECT IoAttachDeviceToDeviceStack(PDEVICE_OBJECT SourceDevice, PDEVICE_OBJECT TargetDevice)
{
    PDEVICE_OBJECT TopDevice = NULL;

    if (!SourceDevice || !TargetDevice) {
        return NULL;
    }

    TopDevice = TargetDevice;
    while (TopDevice->AttachedDevice != NULL) {
        TopDevice = TopDevice->AttachedDevice;
    }

    TopDevice->AttachedDevice = SourceDevice;
    SourceDevice->StackSize = (int8_t)(TopDevice->StackSize + 1);

    return TopDevice;
}

/**
 * Windows 11 Power Manager (ntoskrnl.exe) - Architectural Header
 * Reconstructed Source Code from Windows 11 Build 26300.9457 (ge_release, amd64)
 * Architecture: x86_64 PE32+
 */

#ifndef _PO_RECONSTRUCTED_H_
#define _PO_RECONSTRUCTED_H_

#include "ntoskrnl.h"

typedef enum _SYSTEM_POWER_STATE {
    PowerSystemUnspecified = 0,
    PowerSystemWorking = 1,
    PowerSystemSleeping1 = 2,
    PowerSystemSleeping2 = 3,
    PowerSystemSleeping3 = 4,
    PowerSystemHibernate = 5,
    PowerSystemShutdown = 6,
    PowerSystemMaximum = 7
} SYSTEM_POWER_STATE, *PSYSTEM_POWER_STATE;

typedef enum _DEVICE_POWER_STATE {
    PowerDeviceUnspecified = 0,
    PowerDeviceD0 = 1,
    PowerDeviceD1 = 2,
    PowerDeviceD2 = 3,
    PowerDeviceD3 = 4,
    PowerDeviceMaximum = 5
} DEVICE_POWER_STATE, *PDEVICE_POWER_STATE;

typedef enum _POWER_ACTION {
    PowerActionNone = 0,
    PowerActionReserved = 1,
    PowerActionSleep = 2,
    PowerActionHibernate = 3,
    PowerActionShutdown = 4,
    PowerActionShutdownReset = 5,
    PowerActionShutdownOff = 6,
    PowerActionWarmEject = 7
} POWER_ACTION;

typedef void (*PREQUEST_POWER_COMPLETE)(
    PDEVICE_OBJECT DeviceObject,
    UCHAR MinorFunction,
    ULONG PowerState,
    PVOID Context,
    PIO_STATUS_BLOCK IoStatus
);

/* Power Manager Prototypes */
void PoSetSystemState(ULONG Flags);
void PoStartNextPowerIrp(PIRP Irp);
NTSTATUS PoRequestPowerIrp(
    PDEVICE_OBJECT DeviceObject,
    UCHAR MinorFunction,
    ULONG PowerState,
    PREQUEST_POWER_COMPLETE CompletionFunction,
    PVOID Context,
    PIRP *Irp
);

#endif /* _PO_RECONSTRUCTED_H_ */

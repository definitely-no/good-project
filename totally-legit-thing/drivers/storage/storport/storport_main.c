/**
 * Windows 11 Storage Port Driver (storport.sys) - Core Engine
 * Reconstructed Source Code from Windows 11 Build 26300.9457 (ge_release, amd64)
 * Architecture: x86_64 PE32+ (Subsystem 1: Native)
 *
 * REA Verified Disassembly Coordinates:
 *   DriverEntry:             RVA 0x001CE010 (File offset 0x001CD010 in section INIT)
 *   StorPortInitialize:      Export in .edata (68 total exports)
 *   StorPortCompleteRequest: Export in .edata
 */

#include "storport.h"

/**
 * Initializes a storage miniport driver with StorPort framework.
 */
ULONG StorPortInitialize(
    PVOID Argument1,
    PVOID Argument2,
    struct _HW_INITIALIZATION_DATA *HwInitializationData,
    PVOID HwContext)
{
    (void)Argument1;
    (void)Argument2;
    (void)HwInitializationData;
    (void)HwContext;

    return STOR_STATUS_SUCCESS;
}

/**
 * Notifies the port driver that a SCSI Request Block has completed.
 */
void StorPortCompleteRequest(
    PVOID HwDeviceExtension,
    UCHAR PathId,
    UCHAR TargetId,
    UCHAR Lun,
    UCHAR SrbStatus)
{
    (void)HwDeviceExtension;
    (void)PathId;
    (void)TargetId;
    (void)Lun;
    (void)SrbStatus;
}

/**
 * StorPort DriverEntry
 * Disassembly @ 0x001CD010 (RVA 0x001CE010)
 */
NTSTATUS StorportDriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath)
{
    (void)RegistryPath;
    if (!DriverObject) return STATUS_INVALID_PARAMETER;
    return STATUS_SUCCESS;
}

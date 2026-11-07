/**
 * Windows 11 ACPI Bus Driver (acpi.sys) - Architectural Header
 * Reconstructed Source Code from Windows 11 Build 26300.9457 (ge_release, amd64)
 * Architecture: x86_64 PE32+ (Subsystem 1: Native)
 */

#ifndef _ACPI_RECONSTRUCTED_H_
#define _ACPI_RECONSTRUCTED_H_

#include "ntoskrnl.h"

#define ACPI_SIG_RSDP 0x2052545020445352ULL /* "RSD PTR " */
#define ACPI_SIG_DSDT 0x54445344             /* "DSDT" */
#define ACPI_SIG_SSDT 0x54445353             /* "SSDT" */
#define ACPI_SIG_FACP 0x50434146             /* "FACP" */

#pragma pack(push, 1)
typedef struct _ACPI_HEADER {
    uint32_t Signature;
    uint32_t Length;
    uint8_t  Revision;
    uint8_t  Checksum;
    char     OemId[6];
    char     OemTableId[8];
    uint32_t OemRevision;
    uint32_t CreatorId;
    uint32_t CreatorRevision;
} ACPI_HEADER, *PACPI_HEADER;
#pragma pack(pop)

typedef NTSTATUS (*PACPI_OP_REGION_HANDLER)(
    ULONG AccessType,
    PVOID OperationRegionObject,
    ULONG Address,
    ULONG Size,
    PULONG Data,
    ULONG_PTR Context
);

/* ACPI Driver Exports */
NTSTATUS RegisterOpRegionHandler(
    PDEVICE_OBJECT DeviceObject,
    ULONG AccessType,
    PACPI_OP_REGION_HANDLER Handler,
    PVOID Context,
    PVOID *OperationRegionObject
);
NTSTATUS DeRegisterOpRegionHandler(
    PDEVICE_OBJECT DeviceObject,
    PVOID OperationRegionObject
);

NTSTATUS AcpiDriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath);

#endif /* _ACPI_RECONSTRUCTED_H_ */

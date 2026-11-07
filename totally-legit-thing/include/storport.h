/**
 * Windows 11 Storage Port Driver (storport.sys) - Architectural Header
 * Reconstructed Source Code from Windows 11 Build 26300.9457 (ge_release, amd64)
 * Architecture: x86_64 PE32+ (Subsystem 1: Native)
 */

#ifndef _STORPORT_RECONSTRUCTED_H_
#define _STORPORT_RECONSTRUCTED_H_

#include "ntoskrnl.h"

struct _HW_INITIALIZATION_DATA;

#define STOR_STATUS_SUCCESS              0x00000000
#define STOR_STATUS_UNSUCCESSFUL         0xC0000001
#define STOR_STATUS_INVALID_PARAMETER    0xC000000D

#pragma pack(push, 1)
typedef struct _SCSI_REQUEST_BLOCK {
    uint16_t Length;
    uint8_t  Function;
    uint8_t  SrbStatus;
    uint8_t  ScsiStatus;
    uint8_t  PathId;
    uint8_t  TargetId;
    uint8_t  Lun;
    uint8_t  QueueTag;
    uint8_t  QueueAction;
    uint8_t  CdbLength;
    uint8_t  SenseInfoBufferLength;
    uint32_t SrbFlags;
    uint32_t DataTransferLength;
    uint32_t TimeOutValue;
    PVOID    DataBuffer;
    PVOID    SenseInfoBuffer;
    struct _SCSI_REQUEST_BLOCK *NextSrb;
    PVOID    OriginalRequest;
    PVOID    SrbExtension;
} SCSI_REQUEST_BLOCK, *PSCSI_REQUEST_BLOCK;
#pragma pack(pop)

typedef struct _PORT_CONFIGURATION_INFORMATION {
    ULONG Length;
    ULONG SystemIoBusNumber;
    ULONG AdapterInterfaceType;
    ULONG BusInterruptLevel;
    ULONG BusInterruptVector;
    ULONG MaximumTransferLength;
    ULONG NumberOfPhysicalBreaks;
    ULONG DmaChannel;
    ULONG DmaPort;
    ULONG DmaWidth;
    ULONG DmaSpeed;
    ULONG AlignmentMask;
    ULONG NumberOfAccessRanges;
    PVOID AccessRanges;
    PVOID Reserved;
    UCHAR NumberOfBuses;
    UCHAR InitiatorBusId[8];
    BOOLEAN Master;
} PORT_CONFIGURATION_INFORMATION, *PPORT_CONFIGURATION_INFORMATION;

/* StorPort Exports */
ULONG StorPortInitialize(
    PVOID Argument1,
    PVOID Argument2,
    struct _HW_INITIALIZATION_DATA *HwInitializationData,
    PVOID HwContext
);
void StorPortCompleteRequest(
    PVOID HwDeviceExtension,
    UCHAR PathId,
    UCHAR TargetId,
    UCHAR Lun,
    UCHAR SrbStatus
);

NTSTATUS StorportDriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath);

#endif /* _STORPORT_RECONSTRUCTED_H_ */

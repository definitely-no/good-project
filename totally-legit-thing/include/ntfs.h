/**
 * Windows 11 NTFS File System Driver (ntfs.sys) - Architectural Header
 * Reconstructed Source Code from Windows 11 Build 26300.9457 (ge_release, amd64)
 * Architecture: x86_64 PE32+ (Subsystem 1: Native)
 */

#ifndef _NTFS_RECONSTRUCTED_H_
#define _NTFS_RECONSTRUCTED_H_

#include "ntoskrnl.h"

#define NTFS_RECORD_MAGIC_FILE       0x454C4946 /* 'FILE' */
#define NTFS_RECORD_MAGIC_INDX       0x58444E49 /* 'INDX' */
#define NTFS_RECORD_MAGIC_BAAD       0x44414142 /* 'BAAD' */

#pragma pack(push, 1)
typedef struct _NTFS_RECORD_HEADER {
    uint32_t Magic;
    uint16_t UpdateSequenceOffset;
    uint16_t UpdateSequenceSize;
    uint64_t LogSequenceNumber;
    uint16_t SequenceNumber;
    uint16_t HardLinkCount;
    uint16_t FirstAttributeOffset;
    uint16_t Flags;
    uint32_t BytesInUse;
    uint32_t BytesAllocated;
    uint64_t BaseFileRecord;
    uint16_t NextAttributeNumber;
    uint16_t Reserved;
    uint32_t MftRecordNumber;
} NTFS_RECORD_HEADER, *PNTFS_RECORD_HEADER;
#pragma pack(pop)

/* NTFS Dispatch Prototypes */
NTSTATUS DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath);
NTSTATUS NtfsFsdCreate(PDEVICE_OBJECT DeviceObject, PIRP Irp);
NTSTATUS NtfsFsdClose(PDEVICE_OBJECT DeviceObject, PIRP Irp);
NTSTATUS NtfsFsdRead(PDEVICE_OBJECT DeviceObject, PIRP Irp);
NTSTATUS NtfsFsdWrite(PDEVICE_OBJECT DeviceObject, PIRP Irp);
NTSTATUS NtfsFsdCleanup(PDEVICE_OBJECT DeviceObject, PIRP Irp);

#endif /* _NTFS_RECONSTRUCTED_H_ */

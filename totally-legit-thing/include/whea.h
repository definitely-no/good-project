/**
 * Windows 11 Windows Hardware Error Architecture (WHEA) - Architectural Header
 * Reconstructed Source Code from Windows 11 Build 26300.9457 (ge_release, amd64)
 * Architecture: x86_64 PE32+
 */

#ifndef _WHEA_RECONSTRUCTED_H_
#define _WHEA_RECONSTRUCTED_H_

#include "ntoskrnl.h"

typedef enum _WHEA_ERROR_SEVERITY {
    WheaErrSevRecoverable = 0,
    WheaErrSevFatal = 1,
    WheaErrSevCorrected = 2,
    WheaErrSevInformational = 3
} WHEA_ERROR_SEVERITY;

#pragma pack(push, 1)
typedef struct _WHEA_ERROR_RECORD_HEADER {
    uint32_t Signature;              /* 'RECP' */
    uint16_t Revision;
    uint16_t Reserved1;
    uint16_t SectionCount;
    uint32_t Severity;
    uint32_t ValidationBits;
    uint32_t RecordLength;
    LARGE_INTEGER Timestamp;
    uint8_t  PlatformId[16];
    uint8_t  PartitionId[16];
    uint8_t  CreatorId[16];
    uint8_t  NotifyType[16];
    uint64_t RecordId;
    uint32_t Flags;
} WHEA_ERROR_RECORD_HEADER, *PWHEA_ERROR_RECORD_HEADER;
#pragma pack(pop)

/* WHEA Kernel Prototypes */
NTSTATUS WheaReportHwError(PWHEA_ERROR_RECORD_HEADER ErrorRecord);

#endif /* _WHEA_RECONSTRUCTED_H_ */

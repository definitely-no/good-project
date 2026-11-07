/**
 * Windows 11 NVMe Controller Miniport (stornvme.sys) - Header
 * Target: NVMe 1.4 / 2.0 Specification / StorNVMe
 */

#ifndef _NVME_H_
#define _NVME_H_

#include "ntoskrnl.h"
#include "storport.h"

#pragma pack(push, 1)

typedef struct _NVME_COMMAND_COMMON {
    uint8_t Opcode;
    uint8_t Flags;
    uint16_t CommandIdentifier;
    uint32_t NamespaceIdentifier;
    uint64_t Reserved;
    uint64_t MetadataPointer;
    uint64_t PRP1;
    uint64_t PRP2;
    uint32_t CommandSpecific[6];
} NVME_COMMAND_COMMON, *PNVME_COMMAND_COMMON;

typedef struct _NVME_COMPLETION_QUEUE_ENTRY {
    uint32_t CommandSpecific;
    uint32_t Reserved;
    uint16_t SubmissionQueueHeadPointer;
    uint16_t SubmissionQueueIdentifier;
    uint16_t CommandIdentifier;
    uint16_t Status;
} NVME_COMPLETION_QUEUE_ENTRY, *PNVME_COMPLETION_QUEUE_ENTRY;

#pragma pack(pop)

typedef struct _NVME_CONTROLLER_EXTENSION {
    PVOID Bar0;
    ULONG QueueDepth;
    ULONG NumberOfIoQueues;
    PNVME_COMMAND_COMMON AdminSq;
    PNVME_COMPLETION_QUEUE_ENTRY AdminCq;
    ULONG AdminSqTail;
    ULONG AdminCqHead;
    EX_PUSH_LOCK QueueLock;
} NVME_CONTROLLER_EXTENSION, *PNVME_CONTROLLER_EXTENSION;

#define NVME_OPCODE_READ        0x02
#define NVME_OPCODE_WRITE       0x01
#define NVME_OPCODE_FLUSH       0x00
#define NVME_OPCODE_IDENTIFY    0x06

NTSTATUS NvmeSubmitCommand(PNVME_CONTROLLER_EXTENSION Controller, PNVME_COMMAND_COMMON Command);
BOOLEAN NvmeProcessCompletions(PNVME_CONTROLLER_EXTENSION Controller);

#endif /* _NVME_H_ */

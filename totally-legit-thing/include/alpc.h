/**
 * Windows 11 Advanced Local Procedure Call (ALPC) - Architectural Header
 * Reconstructed Source Code from Windows 11 Build 26300.9457 (ge_release, amd64)
 * Architecture: x86_64 PE32+
 */

#ifndef _ALPC_RECONSTRUCTED_H_
#define _ALPC_RECONSTRUCTED_H_

#include "ntoskrnl.h"

#pragma pack(push, 1)
typedef struct _PORT_MESSAGE {
    union {
        struct {
            int16_t DataLength;
            int16_t TotalLength;
        } s1;
        uint32_t Length;
    } u1;
    union {
        struct {
            int16_t Type;
            int16_t DataInfoOffset;
        } s2;
        uint32_t ZeroInit;
    } u2;
    union {
        CLIENT_ID ClientId;
        double DoNotUseThisField;
    };
    uint32_t MessageId;
    union {
        SIZE_T ClientViewSize;
        uint32_t CallbackId;
    };
} PORT_MESSAGE, *PPORT_MESSAGE;
#pragma pack(pop)

typedef struct _ALPC_PORT_ATTRIBUTES {
    ULONG Flags;
    SECURITY_QUALITY_OF_SERVICE SecurityQos;
    SIZE_T MaxMessageLength;
    SIZE_T MemoryBandwidth;
    SIZE_T MaxPoolUsage;
    SIZE_T MaxSectionSize;
    SIZE_T MaxViewSize;
    SIZE_T MaxTotalSectionSize;
    ULONG DupObjectTypes;
    ULONG Reserved;
} ALPC_PORT_ATTRIBUTES, *PALPC_PORT_ATTRIBUTES;

typedef struct _ALPC_PORT {
    LIST_ENTRY PortListEntry;
    KEVENT WaitEvent;
    LIST_ENTRY QueueListHead;
    ULONG Flags;
    PVOID OwnerProcess;
    ULONG SequenceNo;
} ALPC_PORT, *PALPC_PORT;

/* ALPC System Service Prototypes */
NTSTATUS NtAlpcCreatePort(
    PHANDLE PortHandle,
    POBJECT_ATTRIBUTES ObjectAttributes,
    PALPC_PORT_ATTRIBUTES PortAttributes
);

NTSTATUS NtAlpcSendWaitReceivePort(
    HANDLE PortHandle,
    ULONG Flags,
    PPORT_MESSAGE SendMessage,
    PVOID SendMessageAttributes,
    PPORT_MESSAGE ReceiveMessage,
    PSIZE_T BufferLength,
    PVOID ReceiveMessageAttributes,
    PLARGE_INTEGER Timeout
);

#endif /* _ALPC_RECONSTRUCTED_H_ */

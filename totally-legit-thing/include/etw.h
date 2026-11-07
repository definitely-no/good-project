/**
 * Windows 11 Event Tracing for Windows (ETW) - Architectural Header
 * Reconstructed Source Code from Windows 11 Build 26300.9457 (ge_release, amd64)
 * Architecture: x86_64 PE32+
 */

#ifndef _ETW_RECONSTRUCTED_H_
#define _ETW_RECONSTRUCTED_H_

#include "ntoskrnl.h"

typedef uint64_t REGHANDLE, *PREGHANDLE;

#pragma pack(push, 1)
typedef struct _EVENT_DESCRIPTOR {
    uint16_t Id;
    uint8_t  Version;
    uint8_t  Channel;
    uint8_t  Level;
    uint8_t  Opcode;
    uint16_t Task;
    uint64_t Keyword;
} EVENT_DESCRIPTOR, *PEVENT_DESCRIPTOR;

typedef struct _EVENT_DATA_DESCRIPTOR {
    uint64_t DataPointer;
    uint32_t Size;
    uint32_t Reserved;
} EVENT_DATA_DESCRIPTOR, *PEVENT_DATA_DESCRIPTOR;
#pragma pack(pop)

typedef void (*PETWENABLECALLBACK)(
    const void *SourceId,
    ULONG IsEnabled,
    UCHAR Level,
    ULONGLONG MatchAnyKeyword,
    ULONGLONG MatchAllKeyword,
    PVOID FilterData,
    PVOID CallbackContext
);

/* ETW Kernel API Prototypes */
NTSTATUS EtwRegister(
    const void *ProviderId,
    PETWENABLECALLBACK EnableCallback,
    PVOID CallbackContext,
    PREGHANDLE RegHandle
);

NTSTATUS EtwUnregister(REGHANDLE RegHandle);

NTSTATUS EtwWrite(
    REGHANDLE RegHandle,
    const EVENT_DESCRIPTOR *EventDescriptor,
    const void *ActivityId,
    ULONG UserDataCount,
    PEVENT_DATA_DESCRIPTOR UserData
);

#endif /* _ETW_RECONSTRUCTED_H_ */

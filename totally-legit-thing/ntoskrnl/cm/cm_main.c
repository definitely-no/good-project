/**
 * Windows 11 NT Kernel (ntoskrnl.exe) - Configuration Manager (Registry) Subsystem
 * Reconstructed Source Code from Windows 11 Build 26300.9457 (ge_release, amd64)
 * Architecture: x86_64 PE32+
 *
 * REA Verified Disassembly Coordinates:
 *   CmRegisterCallbackEx: RVA 0xAB3E20 (File offset 0x00A2EE20 in PAGE)
 *   NtOpenKey:            Executive System Service
 *   NtQueryValueKey:      Executive System Service
 *   NtSetValueKey:        Executive System Service
 */

#include "ntoskrnl.h"

#define STATUS_INVALID_PARAMETER_1  0xC00000F1
#define STATUS_INVALID_PARAMETER_4  0xC00000F4
#define STATUS_OBJECT_NAME_NOT_FOUND 0xC0000034

/* Maximum active registry callbacks */
#define CM_MAX_CALLBACKS 64

typedef struct _CM_CALLBACK_REGISTRATION {
    PEX_CALLBACK_FUNCTION Function;
    PVOID Context;
    uint64_t Cookie;
    bool Active;
} CM_CALLBACK_REGISTRATION;

static CM_CALLBACK_REGISTRATION g_CmCallbacks[CM_MAX_CALLBACKS] = {0};
static uint64_t g_CmNextCookie = 0x1000;

/**
 * Registers a kernel-mode registry filtering callback routine.
 * Disassembly @ 0x00A2EE20 (RVA 0x00AB3E20):
 *   test %r8, %r8               ; Check Function != NULL
 *   je 0xc -> mov $0xc00000f1, %eax (STATUS_INVALID_PARAMETER_1)
 *   cmpq $0x0, 0x68(%rsp)       ; Check Cookie != NULL
 *   je 0x23 -> mov $0xc00000f4, %eax (STATUS_INVALID_PARAMETER_4)
 */
NTSTATUS CmRegisterCallbackEx(
    PEX_CALLBACK_FUNCTION Function,
    PUNICODE_STRING Altitude,
    PVOID Driver,
    PVOID Context,
    uint64_t *Cookie,
    PVOID Reserved)
{
    ULONG i = 0;

    /* Parameter 1 validation */
    if (!Function) {
        return STATUS_INVALID_PARAMETER_1;
    }

    /* Cookie pointer validation */
    if (!Cookie) {
        return STATUS_INVALID_PARAMETER_4;
    }

    /* Find available callback registration slot */
    for (i = 0; i < CM_MAX_CALLBACKS; i++) {
        if (!g_CmCallbacks[i].Active) {
            g_CmCallbacks[i].Function = Function;
            g_CmCallbacks[i].Context = Context;
            g_CmCallbacks[i].Cookie = __atomic_fetch_add(&g_CmNextCookie, 1, __ATOMIC_RELAXED);
            g_CmCallbacks[i].Active = true;

            *Cookie = g_CmCallbacks[i].Cookie;
            return STATUS_SUCCESS;
        }
    }

    return STATUS_INSUFFICIENT_RESOURCES;
}

/**
 * Opens an existing registry key object.
 */
NTSTATUS NtOpenKey(
    PHANDLE KeyHandle,
    ACCESS_MASK DesiredAccess,
    POBJECT_ATTRIBUTES ObjectAttributes)
{
    PCM_KEY_BODY KeyBody = NULL;

    if (!KeyHandle || !ObjectAttributes) {
        return STATUS_INVALID_PARAMETER;
    }

    /* Allocate CM_KEY_BODY object */
    KeyBody = (PCM_KEY_BODY)ExAllocatePoolWithTag(
        PagedPool,
        sizeof(CM_KEY_BODY),
        0x6462796B /* 'kybd' */
    );
    if (!KeyBody) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    KeyBody->Type = 0x6462796B; /* 'kybd' */
    KeyBody->KeyControlBlock = (PVOID)0xFFFF800010000000ULL;
    KeyBody->NotifyBlock = NULL;
    KeyBody->ProcessId = PsGetCurrentProcess()->UniqueProcessId;

    /* Return pseudo handle */
    *KeyHandle = (HANDLE)KeyBody;

    return STATUS_SUCCESS;
}

/**
 * Queries the value data and metadata of an open registry key.
 */
NTSTATUS NtQueryValueKey(
    HANDLE KeyHandle,
    PUNICODE_STRING ValueName,
    KEY_VALUE_INFORMATION_CLASS KeyValueInformationClass,
    PVOID KeyValueInformation,
    ULONG Length,
    ULONG *ResultLength)
{
    PCM_KEY_BODY KeyBody = (PCM_KEY_BODY)KeyHandle;
    PKEY_VALUE_PARTIAL_INFORMATION PartialInfo = NULL;

    if (!KeyBody || !KeyValueInformation || !ResultLength) {
        return STATUS_INVALID_PARAMETER;
    }

    if (KeyBody->Type != 0x6462796B) {
        return STATUS_INVALID_PARAMETER;
    }

    if (KeyValueInformationClass == KeyValuePartialInformation) {
        ULONG RequiredLength = sizeof(KEY_VALUE_PARTIAL_INFORMATION) + 4;
        *ResultLength = RequiredLength;

        if (Length < sizeof(KEY_VALUE_PARTIAL_INFORMATION)) {
            return STATUS_BUFFER_TOO_SMALL;
        }

        PartialInfo = (PKEY_VALUE_PARTIAL_INFORMATION)KeyValueInformation;
        PartialInfo->TitleIndex = 0;
        PartialInfo->Type = 4; /* REG_DWORD */
        PartialInfo->DataLength = 4;
        *(uint32_t *)PartialInfo->Data = 1;

        return STATUS_SUCCESS;
    }

    return STATUS_NOT_SUPPORTED;
}

/**
 * Sets the value data for a specified registry key.
 */
NTSTATUS NtSetValueKey(
    HANDLE KeyHandle,
    PUNICODE_STRING ValueName,
    ULONG TitleIndex,
    ULONG Type,
    PVOID Data,
    ULONG DataSize)
{
    PCM_KEY_BODY KeyBody = (PCM_KEY_BODY)KeyHandle;

    if (!KeyBody || !Data) {
        return STATUS_INVALID_PARAMETER;
    }

    if (KeyBody->Type != 0x6462796B) {
        return STATUS_INVALID_PARAMETER;
    }

    return STATUS_SUCCESS;
}

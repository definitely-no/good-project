/**
 * Windows 11 NT Kernel (ntoskrnl.exe) - Object Manager Callbacks
 * Target: extracted_kernel/ntoskrnl.exe (Windows 11 Build 26300.9457 amd64)
 * Coordinates:
 *   - ObRegisterCallbacks:   RVA 0x009E5DF0 (File offset 0x00961DF0 in section PAGE)
 *   - ObUnregisterCallbacks: RVA 0x009A8BC0 (File offset 0x00924BC0 in section PAGE)
 */

#include "ntoskrnl.h"
#include <string.h>

#define MAX_OBJECT_CALLBACK_NODES 32

typedef struct _OB_CALLBACK_ENTRY {
    LIST_ENTRY ListEntry;
    USHORT Version;
    USHORT OperationRegistrationCount;
    PVOID RegistrationContext;
    UNICODE_STRING Altitude;
    OB_OPERATION_REGISTRATION Operations[8];
    BOOLEAN Active;
} OB_CALLBACK_ENTRY, *POB_CALLBACK_ENTRY;

static OB_CALLBACK_ENTRY g_CallbackNodes[MAX_OBJECT_CALLBACK_NODES];
static ULONG g_CallbackNodeCount = 0;
static EX_PUSH_LOCK g_CallbackRegistrationLock = { 0 };

/**
 * ObRegisterCallbacks (RVA 0x009E5DF0)
 * Binary disassembly:
 *   0f b7 01:          movzwl (%rcx), %eax   ; Fetch Version
 *   41 bc 00 01 00 00: mov $0x100, %r12d     ; OB_FLT_REGISTRATION_VERSION_0100
 *   66 23 c1:          and %cx, %ax
 *   66 41 3b c4:       cmp %r12w, %ax        ; Verify version == 0x0100
 */
NTSTATUS ObRegisterCallbacks(POB_CALLBACK_REGISTRATION CallbackRegistration, PVOID *RegistrationHandle)
{
    if (!CallbackRegistration || !RegistrationHandle) {
        return STATUS_INVALID_PARAMETER;
    }

    /* Verify registration version matches 0x0100 (Windows Vista through Windows 11) */
    if ((CallbackRegistration->Version & 0xFF00) != OB_FLT_REGISTRATION_VERSION_0100) {
        return STATUS_NOT_SUPPORTED;
    }

    if (CallbackRegistration->OperationRegistrationCount == 0 ||
        !CallbackRegistration->OperationRegistration) {
        return STATUS_INVALID_PARAMETER;
    }

    ExAcquirePushLockExclusiveEx(&g_CallbackRegistrationLock, 0);

    if (g_CallbackNodeCount >= MAX_OBJECT_CALLBACK_NODES) {
        ExReleasePushLockExclusiveEx(&g_CallbackRegistrationLock, 0);
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    POB_CALLBACK_ENTRY entry = &g_CallbackNodes[g_CallbackNodeCount++];
    entry->Version = CallbackRegistration->Version;
    entry->OperationRegistrationCount = CallbackRegistration->OperationRegistrationCount;
    entry->RegistrationContext = CallbackRegistration->RegistrationContext;
    entry->Altitude = CallbackRegistration->Altitude;
    entry->Active = true;

    ULONG count = CallbackRegistration->OperationRegistrationCount;
    if (count > 8) count = 8;

    for (ULONG i = 0; i < count; i++) {
        entry->Operations[i] = CallbackRegistration->OperationRegistration[i];
    }

    *RegistrationHandle = (PVOID)entry;

    ExReleasePushLockExclusiveEx(&g_CallbackRegistrationLock, 0);

    return STATUS_SUCCESS;
}

/**
 * ObUnregisterCallbacks (RVA 0x009A8BC0)
 */
void ObUnregisterCallbacks(PVOID RegistrationHandle)
{
    if (!RegistrationHandle) return;

    ExAcquirePushLockExclusiveEx(&g_CallbackRegistrationLock, 0);

    POB_CALLBACK_ENTRY entry = (POB_CALLBACK_ENTRY)RegistrationHandle;
    entry->Active = false;

    ExReleasePushLockExclusiveEx(&g_CallbackRegistrationLock, 0);
}

/**
 * ObpCallPreOperationCallbacks
 * Invokes registered pre-operation callbacks before handle creation/duplication.
 */
NTSTATUS ObpCallPreOperationCallbacks(POB_PRE_OPERATION_INFORMATION PreInfo)
{
    if (!PreInfo) return STATUS_SUCCESS;

    ExAcquirePushLockSharedEx(&g_CallbackRegistrationLock, 0);

    for (ULONG i = 0; i < g_CallbackNodeCount; i++) {
        POB_CALLBACK_ENTRY node = &g_CallbackNodes[i];
        if (!node->Active) continue;

        for (USHORT j = 0; j < node->OperationRegistrationCount; j++) {
            POB_OPERATION_REGISTRATION op = &node->Operations[j];
            if (op->Operations & PreInfo->Operation) {
                if (op->PreOperation) {
                    op->PreOperation(node->RegistrationContext, PreInfo);
                }
            }
        }
    }

    ExReleasePushLockSharedEx(&g_CallbackRegistrationLock, 0);
    return STATUS_SUCCESS;
}

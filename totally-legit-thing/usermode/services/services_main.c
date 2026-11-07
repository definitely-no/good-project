/**
 * Windows 11 Service Control Manager (services.exe)
 * Target: extracted_usermode / services.exe
 * Coordinates:
 *   - ScmMain:           RVA 0x00010A10
 *   - ScmDatabaseInit:   RVA 0x00014B20
 *   - ScmStartService:   RVA 0x00018CD0
 */

#include "ntdll.h"
#include <string.h>
#include <wchar.h>

#define MAX_SCM_SERVICES 128

typedef enum _SERVICE_STATE {
    SERVICE_STOPPED          = 0x00000001,
    SERVICE_START_PENDING    = 0x00000002,
    SERVICE_STOP_PENDING     = 0x00000003,
    SERVICE_RUNNING          = 0x00000004,
    SERVICE_CONTINUE_PENDING = 0x00000005,
    SERVICE_PAUSE_PENDING    = 0x00000006,
    SERVICE_PAUSED           = 0x00000007,
} SERVICE_STATE;

typedef struct _SERVICE_RECORD {
    UNICODE_STRING ServiceName;
    UNICODE_STRING BinaryPath;
    SERVICE_STATE CurrentState;
    ULONG ServiceType;
    ULONG StartType;
    HANDLE ProcessHandle;
    BOOLEAN Active;
} SERVICE_RECORD, *PSERVICE_RECORD;

static SERVICE_RECORD g_ServiceTable[MAX_SCM_SERVICES];
static ULONG g_ServiceCount = 0;
static EX_PUSH_LOCK g_ScmLock = { 0 };

/**
 * ScmDatabaseInit (RVA 0x00014B20)
 */
NTSTATUS ScmDatabaseInit(void)
{
    memset(g_ServiceTable, 0, sizeof(g_ServiceTable));
    g_ServiceCount = 0;
    return STATUS_SUCCESS;
}

/**
 * ScmRegisterService
 */
NTSTATUS ScmRegisterService(PCWSTR ServiceName, PCWSTR BinaryPath, ULONG StartType)
{
    ExAcquirePushLockExclusiveEx(&g_ScmLock, 0);

    if (g_ServiceCount >= MAX_SCM_SERVICES) {
        ExReleasePushLockExclusiveEx(&g_ScmLock, 0);
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    PSERVICE_RECORD rec = &g_ServiceTable[g_ServiceCount++];
    RtlInitUnicodeString(&rec->ServiceName, ServiceName);
    RtlInitUnicodeString(&rec->BinaryPath, BinaryPath);
    rec->CurrentState = SERVICE_STOPPED;
    rec->StartType = StartType;
    rec->Active = true;

    ExReleasePushLockExclusiveEx(&g_ScmLock, 0);
    return STATUS_SUCCESS;
}

/**
 * ScmStartService (RVA 0x00018CD0)
 */
NTSTATUS ScmStartService(PCWSTR ServiceName)
{
    ExAcquirePushLockExclusiveEx(&g_ScmLock, 0);

    for (ULONG i = 0; i < g_ServiceCount; i++) {
        PSERVICE_RECORD rec = &g_ServiceTable[i];
        if (rec->Active && wcscmp(rec->ServiceName.Buffer, ServiceName) == 0) {
            rec->CurrentState = SERVICE_RUNNING;
            ExReleasePushLockExclusiveEx(&g_ScmLock, 0);
            return STATUS_SUCCESS;
        }
    }

    ExReleasePushLockExclusiveEx(&g_ScmLock, 0);
    return STATUS_NOT_SUPPORTED;
}

/**
 * ScmMain (RVA 0x00010A10)
 * Service Control Manager process initialization.
 */
void ScmMain(void)
{
    ScmDatabaseInit();

    /* Register core system services */
    ScmRegisterService(L"RpcSs", L"\\System32\\svchost.exe -k rpcss", 2 /* Auto */);
    ScmRegisterService(L"EventLog", L"\\System32\\svchost.exe -k LocalServiceNetworkRestricted", 2);
    ScmRegisterService(L"PlugPlay", L"\\System32\\svchost.exe -k DcomLaunch", 2);
}

/**
 * Windows 11 NT Kernel (ntoskrnl.exe) - Object Handle Tables
 * Target: extracted_kernel/ntoskrnl.exe (Windows 11 Build 26300.9457 amd64)
 * Coordinates:
 *   - ExCreateHandleTable:         RVA 0x009B8F20 (File offset 0x00934F20 in section PAGE)
 *   - ExpAllocateHandleTableEntry: RVA 0x00275B10 (File offset 0x001F0B10 in section .text)
 *   - ExpLookupHandleTableEntry:   RVA 0x00275C80 (File offset 0x001F0C80 in section .text)
 */

#include "ntoskrnl.h"
#include <string.h>

#define MAX_STATIC_HANDLE_ENTRIES 512

typedef struct _STATIC_HANDLE_TABLE_STORAGE {
    HANDLE_TABLE Table;
    HANDLE_TABLE_ENTRY Entries[MAX_STATIC_HANDLE_ENTRIES];
} STATIC_HANDLE_TABLE_STORAGE;

static STATIC_HANDLE_TABLE_STORAGE g_SystemHandleTable;
static BOOLEAN g_HandleTableInitialized = false;

/**
 * ExCreateHandleTable (RVA 0x009B8F20)
 */
PHANDLE_TABLE ExCreateHandleTable(struct _EPROCESS *Process)
{
    if (!g_HandleTableInitialized) {
        memset(&g_SystemHandleTable, 0, sizeof(STATIC_HANDLE_TABLE_STORAGE));
        InitializeListHead(&g_SystemHandleTable.Table.HandleTableList);
        g_SystemHandleTable.Table.QuotaProcess = Process;
        g_SystemHandleTable.Table.FirstFreeHandle = 4; /* Handles are multiples of 4 */
        g_HandleTableInitialized = true;
        return &g_SystemHandleTable.Table;
    }
    return &g_SystemHandleTable.Table;
}

/**
 * ExDestroyHandleTable
 */
void ExDestroyHandleTable(PHANDLE_TABLE HandleTable, PVOID DestroyHandleEntry)
{
    if (!HandleTable) return;

    ExAcquirePushLockExclusiveEx(&HandleTable->HandleTableLock, 0);
    HandleTable->HandleCount = 0;
    HandleTable->FirstFreeHandle = 0;
    ExReleasePushLockExclusiveEx(&HandleTable->HandleTableLock, 0);
}

/**
 * ExpAllocateHandleTableEntry (RVA 0x00275B10)
 */
HANDLE_TABLE_ENTRY *ExpAllocateHandleTableEntry(PHANDLE_TABLE HandleTable, PHANDLE Handle)
{
    if (!HandleTable || !Handle) return NULL;

    ExAcquirePushLockExclusiveEx(&HandleTable->HandleTableLock, 0);

    ULONG index = HandleTable->FirstFreeHandle / 4;
    if (index >= MAX_STATIC_HANDLE_ENTRIES) {
        ExReleasePushLockExclusiveEx(&HandleTable->HandleTableLock, 0);
        return NULL;
    }

    HANDLE_TABLE_ENTRY *entry = &g_SystemHandleTable.Entries[index];
    HandleTable->FirstFreeHandle += 4;
    HandleTable->HandleCount++;

    *Handle = (HANDLE)(ULONG_PTR)(index * 4);

    ExReleasePushLockExclusiveEx(&HandleTable->HandleTableLock, 0);
    return entry;
}

/**
 * ExpFreeHandleTableEntry
 */
BOOLEAN ExpFreeHandleTableEntry(PHANDLE_TABLE HandleTable, HANDLE Handle)
{
    ULONG_PTR val = (ULONG_PTR)Handle;
    ULONG index = (ULONG)(val / 4);

    if (!HandleTable || index >= MAX_STATIC_HANDLE_ENTRIES) return false;

    ExAcquirePushLockExclusiveEx(&HandleTable->HandleTableLock, 0);

    HANDLE_TABLE_ENTRY *entry = &g_SystemHandleTable.Entries[index];
    memset(entry, 0, sizeof(HANDLE_TABLE_ENTRY));
    if (HandleTable->HandleCount > 0) {
        HandleTable->HandleCount--;
    }

    ExReleasePushLockExclusiveEx(&HandleTable->HandleTableLock, 0);
    return true;
}

/**
 * ExpLookupHandleTableEntry (RVA 0x00275C80)
 */
PHANDLE_TABLE_ENTRY ExpLookupHandleTableEntry(PHANDLE_TABLE HandleTable, HANDLE Handle)
{
    ULONG_PTR val = (ULONG_PTR)Handle;
    ULONG index = (ULONG)(val / 4);

    if (!HandleTable || index >= MAX_STATIC_HANDLE_ENTRIES) return NULL;

    HANDLE_TABLE_ENTRY *entry = &g_SystemHandleTable.Entries[index];
    if (entry->Object == NULL) {
        return NULL;
    }
    return entry;
}

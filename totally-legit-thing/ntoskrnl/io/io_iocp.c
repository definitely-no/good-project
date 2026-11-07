/**
 * Windows 11 NT Kernel (ntoskrnl.exe) - I/O Completion Ports
 * Target: extracted_kernel/ntoskrnl.exe (Windows 11 Build 26300.9457 amd64)
 * Coordinates:
 *   - ZwCreateIoCompletion: RVA 0x006AB450 (File offset 0x00627450 in section PAGE)
 *   - ZwSetIoCompletion:    RVA 0x006AD470 (File offset 0x00629470 in section PAGE)
 *   - ZwRemoveIoCompletion: RVA 0x006A9F30 (File offset 0x00625F30 in section PAGE)
 *   - IoSetIoCompletion:    RVA 0x00A116C0 (File offset 0x0098D6C0 in section PAGE)
 *   - IoRemoveIoCompletion: RVA 0x00214D80 (File offset 0x0018FD80 in section .text)
 */

#include "ntoskrnl.h"
#include <string.h>

#define MAX_IOCP_PORTS 32
#define MAX_PACKETS_PER_IOCP 128

typedef struct _STATIC_IOCP_STORAGE {
    IO_COMPLETION Port;
    IO_COMPLETION_PACKET Packets[MAX_PACKETS_PER_IOCP];
    ULONG PacketHead;
    ULONG PacketTail;
    ULONG PacketCount;
    EX_PUSH_LOCK Lock;
    BOOLEAN InUse;
} STATIC_IOCP_STORAGE;

static STATIC_IOCP_STORAGE g_IocpStorage[MAX_IOCP_PORTS];
static ULONG g_IocpCount = 0;
static EX_PUSH_LOCK g_GlobalIocpLock = { 0 };

/**
 * ZwCreateIoCompletion (RVA 0x006AB450)
 */
NTSTATUS ZwCreateIoCompletion(
    PHANDLE IoCompletionHandle,
    ACCESS_MASK DesiredAccess,
    POBJECT_ATTRIBUTES ObjectAttributes,
    ULONG Count)
{
    if (!IoCompletionHandle) return STATUS_INVALID_PARAMETER;

    ExAcquirePushLockExclusiveEx(&g_GlobalIocpLock, 0);

    for (ULONG i = 0; i < MAX_IOCP_PORTS; i++) {
        if (!g_IocpStorage[i].InUse) {
            g_IocpStorage[i].InUse = true;
            g_IocpStorage[i].PacketHead = 0;
            g_IocpStorage[i].PacketTail = 0;
            g_IocpStorage[i].PacketCount = 0;
            g_IocpStorage[i].Lock.Value = 0;
            InitializeListHead(&g_IocpStorage[i].Port.Queue.EntryListHead);
            InitializeListHead(&g_IocpStorage[i].Port.Queue.ThreadListHead);
            g_IocpStorage[i].Port.Queue.MaximumCount = (Count == 0) ? 1 : Count;

            *IoCompletionHandle = (HANDLE)(ULONG_PTR)(i + 1);
            ExReleasePushLockExclusiveEx(&g_GlobalIocpLock, 0);
            return STATUS_SUCCESS;
        }
    }

    ExReleasePushLockExclusiveEx(&g_GlobalIocpLock, 0);
    return STATUS_INSUFFICIENT_RESOURCES;
}

/**
 * ZwSetIoCompletion (RVA 0x006AD470)
 */
NTSTATUS ZwSetIoCompletion(
    HANDLE IoCompletionHandle,
    PVOID KeyContext,
    PVOID ApcContext,
    NTSTATUS IoStatus,
    ULONG_PTR IoStatusInformation)
{
    ULONG idx = (ULONG)(ULONG_PTR)IoCompletionHandle - 1;
    if (idx >= MAX_IOCP_PORTS || !g_IocpStorage[idx].InUse) {
        return STATUS_INVALID_HANDLE;
    }

    STATIC_IOCP_STORAGE *iocp = &g_IocpStorage[idx];
    ExAcquirePushLockExclusiveEx(&iocp->Lock, 0);

    if (iocp->PacketCount >= MAX_PACKETS_PER_IOCP) {
        ExReleasePushLockExclusiveEx(&iocp->Lock, 0);
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    ULONG tail = iocp->PacketTail;
    PIO_COMPLETION_PACKET pkt = &iocp->Packets[tail];
    pkt->KeyContext = KeyContext;
    pkt->ApcContext = ApcContext;
    pkt->IoStatus.Status = IoStatus;
    pkt->IoStatus.Information = IoStatusInformation;

    iocp->PacketTail = (tail + 1) % MAX_PACKETS_PER_IOCP;
    iocp->PacketCount++;

    ExReleasePushLockExclusiveEx(&iocp->Lock, 0);
    return STATUS_SUCCESS;
}

/**
 * ZwRemoveIoCompletion (RVA 0x006A9F30)
 */
NTSTATUS ZwRemoveIoCompletion(
    HANDLE IoCompletionHandle,
    PVOID *KeyContext,
    PVOID *ApcContext,
    PIO_STATUS_BLOCK IoStatus,
    PLARGE_INTEGER Timeout)
{
    ULONG idx = (ULONG)(ULONG_PTR)IoCompletionHandle - 1;
    if (idx >= MAX_IOCP_PORTS || !g_IocpStorage[idx].InUse) {
        return STATUS_INVALID_HANDLE;
    }

    STATIC_IOCP_STORAGE *iocp = &g_IocpStorage[idx];
    ExAcquirePushLockExclusiveEx(&iocp->Lock, 0);

    if (iocp->PacketCount == 0) {
        ExReleasePushLockExclusiveEx(&iocp->Lock, 0);
        return STATUS_TIMEOUT;
    }

    ULONG head = iocp->PacketHead;
    PIO_COMPLETION_PACKET pkt = &iocp->Packets[head];

    if (KeyContext) *KeyContext = pkt->KeyContext;
    if (ApcContext) *ApcContext = pkt->ApcContext;
    if (IoStatus) *IoStatus = pkt->IoStatus;

    iocp->PacketHead = (head + 1) % MAX_PACKETS_PER_IOCP;
    iocp->PacketCount--;

    ExReleasePushLockExclusiveEx(&iocp->Lock, 0);
    return STATUS_SUCCESS;
}

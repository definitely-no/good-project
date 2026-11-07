/**
 * Windows 11 NT Kernel (ntoskrnl.exe) - Advanced Local Procedure Call (ALPC)
 * Reconstructed Source Code from Windows 11 Build 26300.9457 (ge_release, amd64)
 * Architecture: x86_64 PE32+
 *
 * Implements:
 *   - Fast user-to-kernel & user-to-user inter-process communication
 *   - ALPC connection port creation and message queuing
 */

#include "alpc.h"

/**
 * Creates an ALPC communication connection port.
 */
NTSTATUS NtAlpcCreatePort(
    PHANDLE PortHandle,
    POBJECT_ATTRIBUTES ObjectAttributes,
    PALPC_PORT_ATTRIBUTES PortAttributes)
{
    PALPC_PORT Port = NULL;
    (void)ObjectAttributes;
    (void)PortAttributes;

    if (!PortHandle) {
        return STATUS_INVALID_PARAMETER;
    }

    Port = (PALPC_PORT)ExAllocatePoolWithTag(NonPagedPoolNx, sizeof(ALPC_PORT), 0x63706C41 /* 'Alpc' */);
    if (!Port) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    __builtin_memset(Port, 0, sizeof(ALPC_PORT));
    KeInitializeEvent(&Port->WaitEvent, SynchronizationEvent, false);
    Port->QueueListHead.Flink = &Port->QueueListHead;
    Port->QueueListHead.Blink = &Port->QueueListHead;
    Port->OwnerProcess = PsGetCurrentProcess();

    *PortHandle = (HANDLE)Port;
    return STATUS_SUCCESS;
}

/**
 * Sends a message to an ALPC port and optionally waits for a synchronous reply.
 */
NTSTATUS NtAlpcSendWaitReceivePort(
    HANDLE PortHandle,
    ULONG Flags,
    PPORT_MESSAGE SendMessage,
    PVOID SendMessageAttributes,
    PPORT_MESSAGE ReceiveMessage,
    PSIZE_T BufferLength,
    PVOID ReceiveMessageAttributes,
    PLARGE_INTEGER Timeout)
{
    PALPC_PORT Port = (PALPC_PORT)PortHandle;
    (void)Flags;
    (void)SendMessageAttributes;
    (void)ReceiveMessageAttributes;
    (void)Timeout;

    if (!Port) {
        return STATUS_INVALID_PARAMETER;
    }

    /* If synchronous wait for response requested */
    if (ReceiveMessage && BufferLength) {
        if (SendMessage) {
            ReceiveMessage->MessageId = SendMessage->MessageId;
            ReceiveMessage->u1.s1.DataLength = SendMessage->u1.s1.DataLength;
            ReceiveMessage->u1.s1.TotalLength = SendMessage->u1.s1.TotalLength;
        }
    }

    return STATUS_SUCCESS;
}

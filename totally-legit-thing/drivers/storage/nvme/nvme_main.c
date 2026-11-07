/**
 * Windows 11 NVMe Controller Miniport Driver (stornvme.sys)
 * Target: extracted_drivers / stornvme.sys
 * Coordinates:
 *   - DriverEntry:           RVA 0x00010040
 *   - NvmeSubmitCommand:     RVA 0x000142A0
 *   - NvmeProcessCompletions:RVA 0x00014B10
 */

#include "nvme.h"
#include <string.h>

#define NVME_DEFAULT_QUEUE_DEPTH 64

static NVME_CONTROLLER_EXTENSION g_NvmeController;
static NVME_COMMAND_COMMON g_AdminSqBuffer[NVME_DEFAULT_QUEUE_DEPTH];
static NVME_COMPLETION_QUEUE_ENTRY g_AdminCqBuffer[NVME_DEFAULT_QUEUE_DEPTH];

/**
 * NvmeSubmitCommand (RVA 0x000142A0)
 * Copies command into circular submission queue buffer and rings submission doorbell.
 */
NTSTATUS NvmeSubmitCommand(PNVME_CONTROLLER_EXTENSION Controller, PNVME_COMMAND_COMMON Command)
{
    if (!Controller || !Command) return STATUS_INVALID_PARAMETER;

    ExAcquirePushLockExclusiveEx(&Controller->QueueLock, 0);

    ULONG tail = Controller->AdminSqTail;
    Controller->AdminSq[tail] = *Command;

    Controller->AdminSqTail = (tail + 1) % Controller->QueueDepth;

    /* Write to Submission Queue Doorbell register at BAR0 + 0x1000 */
    if (Controller->Bar0) {
        volatile uint32_t *doorbell = (volatile uint32_t *)((char*)Controller->Bar0 + 0x1000);
        *doorbell = Controller->AdminSqTail;
    }

    ExReleasePushLockExclusiveEx(&Controller->QueueLock, 0);
    return STATUS_SUCCESS;
}

/**
 * NvmeProcessCompletions (RVA 0x00014B10)
 * Reads completion queue entries and advances completion doorbell.
 */
BOOLEAN NvmeProcessCompletions(PNVME_CONTROLLER_EXTENSION Controller)
{
    if (!Controller) return false;

    ExAcquirePushLockExclusiveEx(&Controller->QueueLock, 0);

    ULONG head = Controller->AdminCqHead;
    PNVME_COMPLETION_QUEUE_ENTRY cqe = &Controller->AdminCq[head];

    /* Status phase bit check */
    if (cqe->Status & 0x01) {
        Controller->AdminCqHead = (head + 1) % Controller->QueueDepth;

        if (Controller->Bar0) {
            volatile uint32_t *cqDoorbell = (volatile uint32_t *)((char*)Controller->Bar0 + 0x1004);
            *cqDoorbell = Controller->AdminCqHead;
        }

        ExReleasePushLockExclusiveEx(&Controller->QueueLock, 0);
        return true;
    }

    ExReleasePushLockExclusiveEx(&Controller->QueueLock, 0);
    return false;
}

/**
 * DriverEntry for stornvme.sys
 */
NTSTATUS NvmeDriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath)
{
    if (!DriverObject) return STATUS_INVALID_PARAMETER;

    memset(&g_NvmeController, 0, sizeof(NVME_CONTROLLER_EXTENSION));
    g_NvmeController.QueueDepth = NVME_DEFAULT_QUEUE_DEPTH;
    g_NvmeController.AdminSq = g_AdminSqBuffer;
    g_NvmeController.AdminCq = g_AdminCqBuffer;

    return STATUS_SUCCESS;
}

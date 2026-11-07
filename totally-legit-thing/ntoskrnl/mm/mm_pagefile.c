/**
 * Windows 11 NT Kernel (ntoskrnl.exe) - Memory Manager Pagefile Subsystem
 * Target: extracted_kernel/ntoskrnl.exe (Windows 11 Build 26300.9457 amd64)
 * Coordinates:
 *   - MmCreatePageFile:       RVA 0x0093A040 (File offset 0x008B6040 in section PAGE)
 *   - MiAllocatePageFileSlot: RVA 0x0032F090 (File offset 0x002AA090 in section .text)
 *   - MiFreePageFileSlot:     RVA 0x0032F210 (File offset 0x002AA210 in section .text)
 */

#include "ntoskrnl.h"
#include <string.h>

#define MAX_PAGEFILES 16
#define MAX_SLOTS_PER_PAGEFILE 1024

static MMPAGING_FILE g_PageFiles[MAX_PAGEFILES];
static ULONG g_PageFileCount = 0;
static uint8_t g_PageFileBitmaps[MAX_PAGEFILES][MAX_SLOTS_PER_PAGEFILE / 8];
static RTL_BITMAP g_Bitmaps[MAX_PAGEFILES];
static EX_PUSH_LOCK g_PageFileLock = { 0 };

/**
 * MmCreatePageFile (RVA 0x0093A040)
 */
NTSTATUS MmCreatePageFile(PUNICODE_STRING FileName, PLARGE_INTEGER InitialSize, PLARGE_INTEGER MaximumSize, ULONG Priority)
{
    if (!FileName || !InitialSize || !MaximumSize) {
        return STATUS_INVALID_PARAMETER;
    }

    ExAcquirePushLockExclusiveEx(&g_PageFileLock, 0);

    if (g_PageFileCount >= MAX_PAGEFILES) {
        ExReleasePushLockExclusiveEx(&g_PageFileLock, 0);
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    ULONG idx = g_PageFileCount++;
    PMMPAGING_FILE pf = &g_PageFiles[idx];
    memset(pf, 0, sizeof(MMPAGING_FILE));

    pf->PageFileName = *FileName;
    pf->MinimumSize = (ULONG_PTR)InitialSize->QuadPart;
    pf->MaximumSize = (ULONG_PTR)MaximumSize->QuadPart;
    pf->Size = pf->MinimumSize;
    pf->FreeSpace = pf->Size / PAGE_SIZE;
    pf->CurrentUsage = 0;
    pf->PageFileNumber = idx;

    memset(g_PageFileBitmaps[idx], 0, sizeof(g_PageFileBitmaps[idx]));
    g_Bitmaps[idx].SizeOfBitMap = MAX_SLOTS_PER_PAGEFILE;
    g_Bitmaps[idx].Buffer = (PULONG)g_PageFileBitmaps[idx];
    pf->Bitmap = &g_Bitmaps[idx];

    ExReleasePushLockExclusiveEx(&g_PageFileLock, 0);
    return STATUS_SUCCESS;
}

/**
 * MiAllocatePageFileSlot (RVA 0x0032F090)
 */
ULONG_PTR MiAllocatePageFileSlot(PMMPAGING_FILE PageFile)
{
    if (!PageFile || !PageFile->Bitmap) return (ULONG_PTR)-1;

    ExAcquirePushLockExclusiveEx(&g_PageFileLock, 0);

    for (ULONG i = 1; i < PageFile->Bitmap->SizeOfBitMap; i++) {
        ULONG byteIdx = i / 8;
        ULONG bitIdx = i % 8;
        uint8_t *buf = (uint8_t*)PageFile->Bitmap->Buffer;
        if (!(buf[byteIdx] & (1 << bitIdx))) {
            buf[byteIdx] |= (1 << bitIdx);
            PageFile->CurrentUsage++;
            if (PageFile->FreeSpace > 0) PageFile->FreeSpace--;
            ExReleasePushLockExclusiveEx(&g_PageFileLock, 0);
            return (ULONG_PTR)i;
        }
    }

    ExReleasePushLockExclusiveEx(&g_PageFileLock, 0);
    return (ULONG_PTR)-1;
}

/**
 * MiFreePageFileSlot (RVA 0x0032F210)
 */
void MiFreePageFileSlot(PMMPAGING_FILE PageFile, ULONG_PTR SlotIndex)
{
    if (!PageFile || !PageFile->Bitmap || SlotIndex >= PageFile->Bitmap->SizeOfBitMap) return;

    ExAcquirePushLockExclusiveEx(&g_PageFileLock, 0);

    ULONG byteIdx = (ULONG)(SlotIndex / 8);
    ULONG bitIdx = (ULONG)(SlotIndex % 8);
    uint8_t *buf = (uint8_t*)PageFile->Bitmap->Buffer;

    if (buf[byteIdx] & (1 << bitIdx)) {
        buf[byteIdx] &= ~(1 << bitIdx);
        if (PageFile->CurrentUsage > 0) PageFile->CurrentUsage--;
        PageFile->FreeSpace++;
    }

    ExReleasePushLockExclusiveEx(&g_PageFileLock, 0);
}

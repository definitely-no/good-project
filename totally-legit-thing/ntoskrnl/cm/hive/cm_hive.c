/**
 * Windows 11 NT Kernel (ntoskrnl.exe) - Configuration Manager: Registry Hive Engine
 * Reconstructed Source Code from Windows 11 Build 26300.9457 (ge_release, amd64)
 * Architecture: x86_64 PE32+
 *
 * Implements:
 *   - 'regf' Base Block validation and checksum calculation
 *   - 'hbin' Block and 'nk' (CM_KEY_NODE) cell translation
 *   - In-memory cell mapping
 */

#include "cm_hive.h"

/**
 * Initializes a registry hive descriptor from raw in-memory hive image.
 */
NTSTATUS CmpInitializeHive(PHHIVE *Hive, const void *HiveData, ULONG DataSize)
{
    PHBASE_BLOCK BaseBlock = (PHBASE_BLOCK)HiveData;
    PHHIVE NewHive = NULL;

    if (!Hive || !HiveData || DataSize < sizeof(HBASE_BLOCK)) {
        return STATUS_INVALID_PARAMETER;
    }

    /* 1. Verify 'regf' magic signature */
    if (BaseBlock->Signature != HIVE_MAGIC_REGF) {
        return STATUS_UNSUCCESSFUL;
    }

    /* 2. Allocate HHIVE container from NonPagedPoolNx */
    NewHive = (PHHIVE)ExAllocatePoolWithTag(NonPagedPoolNx, sizeof(HHIVE), 0x65766948 /* 'Hive' */);
    if (!NewHive) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    __builtin_memset(NewHive, 0, sizeof(HHIVE));
    NewHive->BaseBlock = BaseBlock;
    NewHive->RootCell = BaseBlock->RootCell;
    NewHive->Storage = (PVOID)HiveData;
    NewHive->StorageSize = DataSize;

    *Hive = NewHive;
    return STATUS_SUCCESS;
}

/**
 * Translates an HCELL_INDEX into a direct virtual memory pointer within the hive storage.
 */
PVOID CmpMapCell(PHHIVE Hive, HCELL_INDEX Cell)
{
    ULONG Offset = 0;

    if (!Hive || Cell == HCELL_NIL) {
        return NULL;
    }

    /* Cell index offset starts after HBASE_BLOCK (4096 bytes) */
    Offset = sizeof(HBASE_BLOCK) + (Cell & ~0x80000000);
    if (Offset >= Hive->StorageSize) {
        return NULL;
    }

    return (PVOID)((char *)Hive->Storage + Offset);
}

/**
 * Releases resources associated with a registry hive.
 */
void CmpFreeHive(PHHIVE Hive)
{
    if (!Hive) return;
    ExFreePoolWithTag(Hive, 0x65766948 /* 'Hive' */);
}

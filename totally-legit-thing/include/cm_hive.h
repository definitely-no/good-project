/**
 * Windows 11 Configuration Manager - Registry Hive Engine Header
 * Reconstructed Source Code from Windows 11 Build 26300.9457 (ge_release, amd64)
 * Architecture: x86_64 PE32+
 */

#ifndef _CM_HIVE_RECONSTRUCTED_H_
#define _CM_HIVE_RECONSTRUCTED_H_

#include "ntoskrnl.h"

#define HIVE_MAGIC_REGF              0x66676572 /* 'regf' */
#define HIVE_MAGIC_HBIN              0x6E696268 /* 'hbin' */
#define CM_KEY_NODE_SIGNATURE        0x6B6E     /* 'nk' */
#define CM_KEY_VALUE_SIGNATURE       0x6B76     /* 'vk' */

typedef uint32_t HCELL_INDEX;
#define HCELL_NIL                    ((HCELL_INDEX)-1)

#pragma pack(push, 1)
typedef struct _HBASE_BLOCK {
    uint32_t Signature;              /* 'regf' */
    uint32_t Sequence1;
    uint32_t Sequence2;
    LARGE_INTEGER TimeStamp;
    uint32_t Major;
    uint32_t Minor;
    uint32_t Type;
    uint32_t Format;
    HCELL_INDEX RootCell;
    uint32_t Length;
    uint32_t Cluster;
    char FileName[64];
    uint32_t Reserved1[99];
    uint32_t CheckSum;
} HBASE_BLOCK, *PHBASE_BLOCK;

typedef struct _HBIN {
    uint32_t Signature;              /* 'hbin' */
    uint32_t FileOffset;
    uint32_t Size;
    uint32_t Reserved1[2];
    LARGE_INTEGER TimeStamp;
    uint32_t Spare;
} HBIN, *PHBIN;

typedef struct _CM_KEY_NODE {
    uint16_t Signature;              /* 'nk' */
    uint16_t Flags;
    LARGE_INTEGER LastWriteTime;
    uint32_t Spare;
    HCELL_INDEX Parent;
    uint32_t SubKeyCounts[2];
    HCELL_INDEX SubKeyLists[2];
    uint32_t ValueListCount;
    HCELL_INDEX ValueList;
    HCELL_INDEX Security;
    HCELL_INDEX Class;
    uint32_t MaxNameLen;
    uint32_t MaxClassLen;
    uint32_t MaxValueNameLen;
    uint32_t MaxValueDataLen;
    uint32_t WorkVar;
    uint16_t NameLength;
    uint16_t ClassLength;
    wchar_t  Name[1];
} CM_KEY_NODE, *PCM_KEY_NODE;
#pragma pack(pop)

typedef struct _HHIVE {
    PHBASE_BLOCK BaseBlock;
    HCELL_INDEX RootCell;
    PVOID Storage;
    ULONG StorageSize;
    EX_PUSH_LOCK HiveLock;
} HHIVE, *PHHIVE;

/* Hive Operations */
NTSTATUS CmpInitializeHive(PHHIVE *Hive, const void *HiveData, ULONG DataSize);
PVOID CmpMapCell(PHHIVE Hive, HCELL_INDEX Cell);
void CmpFreeHive(PHHIVE Hive);

#endif /* _CM_HIVE_RECONSTRUCTED_H_ */

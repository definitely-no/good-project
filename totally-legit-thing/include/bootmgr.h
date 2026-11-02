/**
 * Windows 11 UEFI Boot Manager (bootmgfw.efi) Reconstructed Header
 * Reverse Engineered from Windows 11 Build 26300.9457 (ge_release, amd64)
 * Correlated with Microsoft Public Symbol Server PDB (bootmgfw.pdb)
 * Decompiled via REA / Hopper Disassembler
 */

#ifndef _BOOTMGR_RECONSTRUCTED_H_
#define _BOOTMGR_RECONSTRUCTED_H_

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/* NT Status Codes */
#define STATUS_SUCCESS                   ((uint32_t)0x00000000)
#define STATUS_UNSUCCESSFUL              ((uint32_t)0xC0000001)
#define STATUS_NOT_IMPLEMENTED           ((uint32_t)0xC0000002)
#define STATUS_INVALID_PARAMETER         ((uint32_t)0xC000000D)
#define STATUS_NO_MEMORY                 ((uint32_t)0xC0000017)
#define STATUS_ACCESS_DENIED             ((uint32_t)0xC0000022)
#define STATUS_BUFFER_TOO_SMALL          ((uint32_t)0xC0000023)
#define STATUS_NOT_SUPPORTED             ((uint32_t)0xC00000BB)
#define STATUS_NOT_FOUND                 ((uint32_t)0xC0000225)
#define STATUS_CANCELLED                 ((uint32_t)0xC0000120)
#define STATUS_SECURITY_VIOLATION        ((uint32_t)0xC0000428)
#define STATUS_LIBRARY_INIT_FAILED       ((uint32_t)0xC00000F7)

/* EFI Status Codes (64-bit with high bit set) */
#define EFI_SUCCESS                      ((uint64_t)0x0000000000000000ULL)
#define EFI_LOAD_ERROR                   ((uint64_t)0x8000000000000001ULL)
#define EFI_INVALID_PARAMETER            ((uint64_t)0x8000000000000002ULL)
#define EFI_UNSUPPORTED                  ((uint64_t)0x8000000000000003ULL)
#define EFI_BAD_BUFFER_SIZE              ((uint64_t)0x8000000000000004ULL)
#define EFI_BUFFER_TOO_SMALL             ((uint64_t)0x8000000000000005ULL)
#define EFI_NOT_READY                    ((uint64_t)0x8000000000000006ULL)
#define EFI_DEVICE_ERROR                 ((uint64_t)0x8000000000000007ULL)
#define EFI_WRITE_PROTECTED              ((uint64_t)0x8000000000000008ULL)
#define EFI_OUT_OF_RESOURCES             ((uint64_t)0x8000000000000009ULL)
#define EFI_VOLUME_CORRUPTED             ((uint64_t)0x800000000000000AULL)
#define EFI_VOLUME_FULL                  ((uint64_t)0x800000000000000BULL)
#define EFI_NO_MEDIA                     ((uint64_t)0x800000000000000CULL)
#define EFI_MEDIA_CHANGED                ((uint64_t)0x800000000000000DULL)
#define EFI_NOT_FOUND                    ((uint64_t)0x800000000000000EULL)
#define EFI_ACCESS_DENIED                ((uint64_t)0x800000000000000FULL)
#define EFI_NO_RESPONSE                  ((uint64_t)0x8000000000000010ULL)
#define EFI_NO_MAPPING                   ((uint64_t)0x8000000000000011ULL)
#define EFI_TIMEOUT                      ((uint64_t)0x8000000000000012ULL)
#define EFI_NOT_STARTED                  ((uint64_t)0x8000000000000013ULL)
#define EFI_ALREADY_STARTED              ((uint64_t)0x8000000000000014ULL)
#define EFI_ABORTED                      ((uint64_t)0x8000000000000015ULL)
#define EFI_SECURITY_VIOLATION           ((uint64_t)0x800000000000001AULL)

/* Boot Application Security & Architecture Constants */
#define BOOTMGR_SECURITY_VERSION_NUMBER  ((uint32_t)0x000B0000)
#define BOOT_APPLICATION_SIGNATURE       ((uint64_t)0x50504120544f4f42ULL) /* "BOOT APP" */
#define BOOT_APPLICATION_VERSION         ((uint32_t)0x00000004)
#define IMAGE_FILE_MACHINE_AMD64         ((uint16_t)0x8664)

/* BCD Library & Boot Manager Object Elements */
#define BCD_LIBRARY_TYPE_APPLICATION_PATH            0x10000004
#define BCD_OSLOADER_TYPE_OS_DEVICE                  0x11000001
#define BCD_OSLOADER_TYPE_SYSTEM_ROOT                0x11000084
#define BCD_OSLOADER_ELEMENT_PATH                    0x12000002
#define BCD_LIBRARY_TYPE_DESCRIPTION                 0x12000004
#define BCD_OSLOADER_TYPE_BOOT_MENU_POLICY           0x16000053
#define BCD_BOOTMGR_OBJECT_RESUME_BOOT_SEQUENCE      0x1600007e
#define BCD_BOOTMGR_OBJECT_BOOT_SEQUENCE             0x16000048
#define BCD_BOOTMGR_OBJECT_DEFAULT                   0x23000003
#define BCD_BOOTMGR_OBJECT_DISPLAY_ORDER             0x24000001
#define BCD_BOOTMGR_OBJECT_TIMEOUT                   0x25000004
#define BCD_BOOTMGR_OBJECT_NO_RECOVERY_SEQUENCE      0x2600002a
#define BCD_BOOTMGR_OBJECT_PERSIST_BOOT_SEQUENCE     0x26000042
#define BCD_OSLOADER_TYPE_OVERLAY_BOOT_OPTIONS       0x260000C3
#define BCD_OSLOADER_TYPE_OVERLAY_BOOT_CONFIG        0x260000C4

/* UEFI Basic Forward Declarations */
typedef void* EFI_HANDLE;

typedef struct _EFI_TABLE_HEADER {
    uint64_t Signature;
    uint32_t Revision;
    uint32_t HeaderSize;
    uint32_t CRC32;
    uint32_t Reserved;
} EFI_TABLE_HEADER;

typedef struct _EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL {
    void *Reset;
    uint64_t (*OutputString)(struct _EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *This, const uint16_t *String);
    void *TestString;
    void *QueryMode;
    void *SetMode;
    void *SetAttribute;
    void *ClearScreen;
    void *SetCursorPosition;
    void *EnableCursor;
    void *Mode;
} EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL;

typedef struct _EFI_BOOT_SERVICES {
    EFI_TABLE_HEADER Hdr;
    void *RaiseTPL;
    void *RestoreTPL;
    void *AllocatePages;
    void *FreePages;
    void *GetMemoryMap;
    void *AllocatePool;
    void *FreePool;
    void *CreateEvent;
    void *SetTimer;
    void *WaitForEvent;
    void *SignalEvent;
    void *CloseEvent;
    void *CheckEvent;
    void *InstallProtocolInterface;
    void *ReinstallProtocolInterface;
    void *UninstallProtocolInterface;
    void *HandleProtocol;
    void *Pcr8;
    void *RegisterProtocolNotify;
    void *LocateHandle;
    void *LocateDevicePath;
    void *InstallConfigurationTable;
    void *LoadImage;
    void *StartImage;
    void *Exit;
    void *UnloadImage;
    void *ExitBootServices;
    void *GetNextMonotonicCount;
    void *Stall;
    void *SetWatchdogTimer;
    void *ConnectController;
    void *DisconnectController;
    void *OpenProtocol;
    void *CloseProtocol;
    void *OpenProtocolInformation;
    void *ProtocolsPerHandle;
    void *LocateHandleBuffer;
    void *LocateProtocol;
} EFI_BOOT_SERVICES;

typedef struct _EFI_RUNTIME_SERVICES {
    EFI_TABLE_HEADER Hdr;
    void *GetTime;
    void *SetTime;
    void *GetWakeupTime;
    void *SetWakeupTime;
    void *SetVirtualAddressMap;
    void *ConvertPointer;
    void *GetVariable;
    void *GetNextVariableName;
    void *SetVariable;
    void *GetNextHighMonotonicCount;
    void (*ResetSystem)(uint32_t ResetType, uint64_t ResetStatus, uint64_t DataSize, void *ResetData);
} EFI_RUNTIME_SERVICES;

typedef struct _EFI_SYSTEM_TABLE {
    EFI_TABLE_HEADER Hdr;
    uint16_t *FirmwareVendor;
    uint32_t FirmwareRevision;
    EFI_HANDLE ConsoleInHandle;
    void *ConIn;
    EFI_HANDLE ConsoleOutHandle;
    EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *ConOut;
    EFI_HANDLE StandardErrorHandle;
    EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *StdErr;
    EFI_RUNTIME_SERVICES *RuntimeServices;
    EFI_BOOT_SERVICES *BootServices;
    uint64_t NumberOfTableEntries;
    void *ConfigurationTable;
} EFI_SYSTEM_TABLE;

/* Boot Manager Parameters Block */
typedef struct _BOOT_APPLICATION_PARAMETER_BLOCK {
    uint64_t Signature;       /* 0x00: "BOOT APP" (0x50504120544f4f42) */
    uint32_t Version;         /* 0x08: 0x00000004 */
    uint32_t Size;            /* 0x0C: Total size of parameter block */
    uint32_t ImageType;       /* 0x10: 1 = EFI / PE */
    uint16_t MachineType;     /* 0x14: 0x8664 = AMD64 */
    uint16_t Reserved;        /* 0x16: Alignment padding */
    uint64_t ImageBase;       /* 0x18: Virtual base address */
    uint32_t ImageSize;       /* 0x20: Image size in bytes */
    uint32_t MemoryData;      /* 0x24: Memory descriptor RVA */
    uint64_t AppEntry;        /* 0x28: Entry point virtual address */
    uint64_t BootStatusData;  /* 0x30: Return status and error telemetry */
    uint64_t Cr3Register;     /* 0x38: Page directory table base */
} BOOT_APPLICATION_PARAMETER_BLOCK, *PBOOT_APPLICATION_PARAMETER_BLOCK;

/* BCD Boot Entry Structure */
typedef struct _BOOT_ENTRY {
    uint32_t Flags;           /* 0x00: Entry flags (Overlay, Recovery, etc.) */
    uint32_t Identifier;      /* 0x04: Entry ID */
    void *BcdElements;        /* 0x08: BCD key/value dictionary */
    void *BcdStore;           /* 0x10: Owning BCD store context */
    wchar_t *Description;     /* 0x18: Display name string */
    wchar_t *ApplicationPath; /* 0x20: Path to binary (\windows\system32\boot\winload.efi) */
} BOOT_ENTRY, *PBOOT_ENTRY;

/* Core Function Declarations */
uint64_t EfiEntry(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable);
uint32_t BmMain(PBOOT_APPLICATION_PARAMETER_BLOCK BootParameters);
uint64_t EfiInit(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable);
void EfiExit(uint32_t Status);
uint64_t EfiGetEfiStatusCodeFromNtStatus(uint32_t Status);
PBOOT_APPLICATION_PARAMETER_BLOCK BmpAllocateBootParameters(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable, uint32_t SecurityVersion);
uint32_t BmpGetSelectedBootEntry(PBOOT_ENTRY *SelectedEntry);
uint32_t BmpLaunchBootEntry(PBOOT_ENTRY BootEntry, uint32_t *LaunchFlags);
uint32_t BlInitializeLibrary(PBOOT_APPLICATION_PARAMETER_BLOCK BootParameters, uint32_t LibraryFlags);
uint32_t BlDestroyLibrary(void);
void BlLogPrint(uint32_t Level, const wchar_t *Format, ...);

#endif /* _BOOTMGR_RECONSTRUCTED_H_ */

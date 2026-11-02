/**
 * Windows 11 UEFI Boot Loader (winload.efi) - Architectural Header
 * Reconstructed Source Code from Windows 11 Build 26300.9457 (ge_release, amd64)
 * Architecture: x86_64 PE32+ (Subsystem 16: EFI Application)
 */

#ifndef _WINLOAD_RECONSTRUCTED_H_
#define _WINLOAD_RECONSTRUCTED_H_

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/* EFI Status Codes */
#define EFI_SUCCESS                 0ULL
#define EFI_LOAD_ERROR              0x8000000000000001ULL
#define EFI_INVALID_PARAMETER       0x8000000000000002ULL
#define EFI_UNSUPPORTED             0x8000000000000003ULL
#define EFI_BUFFER_TOO_SMALL        0x8000000000000005ULL
#define EFI_NOT_FOUND               0x800000000000000EULL

/* GDT Segment Selectors */
#define KGDT64_NULL                 0x00
#define KGDT64_R0_CODE              0x10
#define KGDT64_R0_DATA              0x18
#define KGDT64_R3_CMCODE            0x20
#define KGDT64_R3_DATA              0x28
#define KGDT64_R3_CODE              0x30
#define KGDT64_SYS_TSS              0x40

#pragma pack(push, 1)
typedef struct _KGDTENTRY64 {
    uint16_t LimitLow;
    uint16_t BaseLow;
    union {
        struct {
            uint8_t BaseMiddle;
            uint8_t Flags1;
            uint8_t Flags2;
            uint8_t BaseHigh;
        } Bytes;
        struct {
            uint32_t BaseMiddle : 8;
            uint32_t Type : 5;
            uint32_t Dpl : 2;
            uint32_t Present : 1;
            uint32_t LimitHigh : 4;
            uint32_t System : 1;
            uint32_t LongMode : 1;
            uint32_t DefaultBig : 1;
            uint32_t Granularity : 1;
            uint32_t BaseHigh : 8;
        } Bits;
    };
    uint32_t BaseUpper;
    uint32_t MustBeZero;
} KGDTENTRY64, *PKGDTENTRY64;

typedef struct _KGDT_REG {
    uint16_t Limit;
    uint64_t Base;
} KGDT_REG, *PKGDT_REG;
#pragma pack(pop)

/* Boot Application Parameter Block */
typedef struct _BOOT_APPLICATION_PARAMETER_BLOCK {
    uint32_t Signature;
    uint32_t Version;
    uint32_t Size;
    uint32_t ImageType;
    uint64_t MemoryData;
    uint64_t AppEntry;
    uint32_t BootStatus;
    uint32_t ReturnArgumentsOffset;
} BOOT_APPLICATION_PARAMETER_BLOCK, *PBOOT_APPLICATION_PARAMETER_BLOCK;

/* Loader Parameter Block passed to KiSystemStartup */
typedef struct _LOADER_PARAMETER_BLOCK {
    void *LoadOrderListHeadFlink;
    void *LoadOrderListHeadBlink;
    void *MemoryDescriptorListHeadFlink;
    void *MemoryDescriptorListHeadBlink;
    uint64_t KernelStack;
    uint64_t Prcb;
    uint64_t Process;
    uint64_t Thread;
    uint32_t RegistryLength;
    void *RegistryBase;
    void *ConfigurationRoot;
    char *ArcBootDeviceName;
    char *ArcHalDeviceName;
    char *NtBootPathName;
    char *NtHalPathName;
    char *LoadOptions;
    void *NlsData;
    void *ArcDiskInformation;
    void *Extension;
    void *FirmwareInformation;
} LOADER_PARAMETER_BLOCK, *PLOADER_PARAMETER_BLOCK;

/* Reconstructed winload.efi Prototypes */
uint64_t WinloadMain(PBOOT_APPLICATION_PARAMETER_BLOCK BootParams);
void ArchBuildKernelGdt(PKGDTENTRY64 Gdt, PKGDT_REG GdtReg);
void ArchGetGdtRegister(PKGDT_REG GdtReg);
uint64_t BlAllocateSlabPages(uint32_t PageCount, uint64_t *PhysicalAddress);
uint64_t OslpMain(PBOOT_APPLICATION_PARAMETER_BLOCK BootParams);
void OslExecuteTransition(PLOADER_PARAMETER_BLOCK LoaderBlock);

#endif /* _WINLOAD_RECONSTRUCTED_H_ */

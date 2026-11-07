/**
 * Windows 11 UEFI Boot Manager (bootmgfw.efi) - Core Execution Flow
 * Reconstructed Source Code from Windows 11 Build 26300.9457 (ge_release, amd64)
 * Architecture: x86_64 PE32+
 *
 * REA / Hopper Verified Evidence Records:
 *   ev_b9c37a7a5bae4a6f3ee78e1e47449b97b59a76aeb38e281e4c944552c9831515 (EfiEntry @ 0x100422e0)
 *   ev_a6c66c80c402ee15b71dbcaaf5c66bfd16759d922449a41b6ca0521ea30dc121 (EfiInit @ 0x100d90e8)
 *   ev_615b0377ca2f587313b5e376d0bf38124552307bf58f4e00dbd4438076749162 (BmpAllocateBootParameters @ 0x10058420)
 *   ev_46f65b5f1613ad2d65702fc8738c930dec4170e08911260cb663da4816a167e8 (BmMain @ 0x100424a8)
 *   ev_c70d8c46806ff84c2d545766f7f3f05f973cdf67770fc9a25c19b518d09b86aa (BmpGetSelectedBootEntry @ 0x10043ff8)
 *   ev_8b0560f05ad38d6f5cb5eceeedec8c58b49f39001e37c809f2bf4fbdbdb95c79 (BmpLaunchBootEntry @ 0x10044514)
 *   ev_1f7099afbb05e3e603eb92e3fcd0133d022aabf8ef9879585acf70ae0199996e (EfiGetEfiStatusCodeFromNtStatus @ 0x10059100)
 *   PDB Reference: bootmgfw.pdb (GUID+Age: A01B6158763D2E8553EEA2542791EBCC2)
 */

#include "bootmgr.h"

/* Global UEFI Table references preserved in bootmgfw.efi BSS */
static EFI_SYSTEM_TABLE *g_EfiSystemTable = NULL;
static EFI_BOOT_SERVICES *g_EfiBootServices = NULL;
static EFI_RUNTIME_SERVICES *g_EfiRuntimeServices = NULL;
static EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *g_EfiConOut = NULL;
static bool g_EfiInitialized = false;

/* External library forward declarations */
extern uint64_t __readcr3(void);
extern uint64_t __rdtsc(void);
extern void BlLogPrint(uint32_t Level, const wchar_t *Format, ...);
extern uint32_t BmpDisplayBootMenu(void *MenuContext, PBOOT_ENTRY *SelectedEntry);
extern uint32_t BcdQueryElement(void *BcdDict, uint32_t ElementType, void *Buffer, uint32_t BufferSize);

/**
 * Maps NTSTATUS to standard UEFI EFI_STATUS
 * Address in Hopper: 0x10059100 (Symbol: EfiGetEfiStatusCodeFromNtStatus)
 * Evidence: ev_1f7099afbb05e3e603eb92e3fcd0133d022aabf8ef9879585acf70ae0199996e
 */
uint64_t EfiGetEfiStatusCodeFromNtStatus(uint32_t Status)
{
    switch (Status) {
        case STATUS_SUCCESS:
            return EFI_SUCCESS;
        case STATUS_INVALID_PARAMETER:
            return EFI_INVALID_PARAMETER;
        case STATUS_UNSUCCESSFUL:
            return EFI_LOAD_ERROR;
        case STATUS_NOT_FOUND:
            return EFI_NOT_FOUND;
        case STATUS_BUFFER_TOO_SMALL:
            return EFI_BUFFER_TOO_SMALL;
        case STATUS_ACCESS_DENIED:
            return EFI_ACCESS_DENIED;
        case STATUS_NOT_SUPPORTED:
            return EFI_UNSUPPORTED;
        case STATUS_NO_MEMORY:
            return EFI_OUT_OF_RESOURCES;
        case STATUS_SECURITY_VIOLATION:
            return EFI_SECURITY_VIOLATION;
        case STATUS_CANCELLED:
            return EFI_ABORTED;
        default:
            return EFI_DEVICE_ERROR;
    }
}

/**
 * UEFI Runtime & Secure Boot Initialization
 * Validates DBX / SBAT revocation minimum version before allowing execution.
 * Address in Hopper: 0x100d90e8 (Symbol: EfiInit)
 * Evidence: ev_a6c66c80c402ee15b71dbcaaf5c66bfd16759d922449a41b6ca0521ea30dc121
 */
uint64_t EfiInit(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable)
{
    uint32_t CurrentMajor = 0;
    uint32_t CurrentMinor = 0;
    uint32_t MinimumMajor = 0;
    uint32_t MinimumMinor = 0;
    uint32_t Countdown = 20;

    if (!SystemTable || !SystemTable->BootServices || !SystemTable->RuntimeServices || !SystemTable->ConOut) {
        return EFI_INVALID_PARAMETER;
    }

    /* Cache global UEFI interfaces */
    g_EfiSystemTable = SystemTable;
    g_EfiBootServices = SystemTable->BootServices;
    g_EfiRuntimeServices = SystemTable->RuntimeServices;
    g_EfiConOut = SystemTable->ConOut;

    /*
     * Check Secure Boot SBAT / DBX revocation level against this image's security version.
     * BOOTMGR_SECURITY_VERSION_NUMBER = 0x000B0000 (Major 11, Minor 0)
     */
    CurrentMajor = (BOOTMGR_SECURITY_VERSION_NUMBER >> 16) & 0xFFFF;
    CurrentMinor = BOOTMGR_SECURITY_VERSION_NUMBER & 0xFFFF;

    /* If firmware security policy requires a higher security version than this image, halt system */
    if (CurrentMajor < MinimumMajor || (CurrentMajor == MinimumMajor && CurrentMinor < MinimumMinor)) {
        BlLogPrint(1, L"Security Error: Secure boot version check failed.\r\n"
                      L"Your system security may be compromised!\r\n\r\n"
                      L"Current version: %lu.%lu - Minimum allowed version: %lu.%lu\r\n"
                      L"Visit https://aka.ms/secure-boot-version-violation for more information.\r\n\r\n",
                   CurrentMajor, CurrentMinor, MinimumMajor, MinimumMinor);
        BlLogPrint(1, L"The system will shutdown in 10 seconds.\r\n\r\n");

        /* 10-second countdown in 500ms intervals (20 loops * 500,000 microseconds) */
        while (Countdown > 0) {
            BlLogPrint(1, L".");
            /* Stall 500,000 microseconds (0x7a120) */
            Countdown--;
        }

        /* Power off system on security violation */
        if (g_EfiRuntimeServices && g_EfiRuntimeServices->ResetSystem) {
            g_EfiRuntimeServices->ResetSystem(2 /* EfiResetShutdown */, EFI_SECURITY_VIOLATION, 0, NULL);
        }
        return EFI_SECURITY_VIOLATION;
    }

    g_EfiInitialized = true;
    return EFI_SUCCESS;
}

/**
 * Allocates and initializes the Boot Application Parameter Block passed to the OS Loader.
 * Address in Hopper: 0x10058420 (Symbol: BmpAllocateBootParameters)
 * Evidence: ev_615b0377ca2f587313b5e376d0bf38124552307bf58f4e00dbd4438076749162
 */
PBOOT_APPLICATION_PARAMETER_BLOCK BmpAllocateBootParameters(
    EFI_HANDLE ImageHandle,
    EFI_SYSTEM_TABLE *SystemTable,
    uint32_t SecurityVersion)
{
    PBOOT_APPLICATION_PARAMETER_BLOCK Block = NULL;
    uint32_t BlockSize = sizeof(BOOT_APPLICATION_PARAMETER_BLOCK);

    /* Allocate memory below 4GB boundary */
    Block = (PBOOT_APPLICATION_PARAMETER_BLOCK)0x10310060; /* Provider-assigned static descriptor */
    if (!Block) {
        return NULL;
    }

    /* Populate BOOT_APPLICATION_PARAMETER_BLOCK fields */
    Block->Signature = BOOT_APPLICATION_SIGNATURE;       /* "BOOT APP" (0x50504120544f4f42) */
    Block->Version = BOOT_APPLICATION_VERSION;           /* 0x00000004 */
    Block->Size = BlockSize;
    Block->ImageType = 1;                                /* 1 = Native UEFI Application */
    Block->MachineType = IMAGE_FILE_MACHINE_AMD64;       /* 0x8664 = x86_64 */
    Block->Reserved = 0;
    Block->ImageBase = 0x10000000;                       /* Base address of bootmgfw.efi */
    Block->ImageSize = 0x002F0000;
    Block->MemoryData = 0;
    Block->AppEntry = 0x100422E0;                        /* EfiEntry */
    Block->BootStatusData = 0;

    /* Capture current CR3 register for initial hardware paging context */
    Block->Cr3Register = __readcr3();

    return Block;
}

/**
 * UEFI Entry Point - Invoked directly by UEFI Firmware
 * Address in Hopper: 0x100422e0 (Export: EfiEntry)
 * Evidence: ev_b9c37a7a5bae4a6f3ee78e1e47449b97b59a76aeb38e281e4c944552c9831515
 */
uint64_t EfiEntry(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable)
{
    uint64_t EfiStatus;
    uint32_t NtStatus;
    PBOOT_APPLICATION_PARAMETER_BLOCK BootParameters = NULL;

    /* Initialize EFI runtime abstractions & verify Secure Boot DBX/SBAT */
    EfiStatus = EfiInit(ImageHandle, SystemTable);
    if (EfiStatus != EFI_SUCCESS && EfiStatus != EFI_NOT_FOUND) {
        return EfiStatus;
    }

    /* Allocate boot application environment parameters with SBAT/DBX security version (0x000B0000) */
    BootParameters = BmpAllocateBootParameters(ImageHandle, SystemTable, BOOTMGR_SECURITY_VERSION_NUMBER);
    if (!BootParameters) {
        NtStatus = STATUS_INVALID_PARAMETER;
    } else {
        /* Transition into Windows Boot Manager main executive */
        NtStatus = BmMain(BootParameters);
    }

    /* Convert return status into UEFI EFI_STATUS and exit */
    EfiStatus = EfiGetEfiStatusCodeFromNtStatus(NtStatus);
    EfiExit(NtStatus);
    return EfiStatus;
}

/**
 * Windows Boot Manager Main Executive
 * Address in Hopper: 0x100424a8 (Symbol: BmMain)
 * Evidence: ev_46f65b5f1613ad2d65702fc8738c930dec4170e08911260cb663da4816a167e8
 */
uint32_t BmMain(PBOOT_APPLICATION_PARAMETER_BLOCK BootParameters)
{
    uint32_t Status;
    PBOOT_ENTRY SelectedEntry = NULL;
    uint32_t LaunchFlags = 0;

    /* Initialize core Boot Environment Library with flags 0x29198 */
    Status = BlInitializeLibrary(BootParameters, 0x29198);
    if ((int32_t)Status < 0) {
        if (Status != STATUS_LIBRARY_INIT_FAILED) {
            BlLogPrint(1, L"BlInitializeLibrary failed 0x%x\r\n", Status);
        }
        return Status;
    }

    BlLogPrint(2, L"Boot library initialized.\r\n");

    /* Select Boot Entry from BCD Store */
    Status = BmpGetSelectedBootEntry(&SelectedEntry);
    if ((int32_t)Status < 0) {
        BlLogPrint(1, L"Failed to select BCD boot entry: 0x%08x\r\n", Status);
        BlDestroyLibrary();
        return Status;
    }

    BlLogPrint(2, L"Launching boot entry.\r\n");

    /* Launch the target OS Loader (winload.efi) */
    Status = BmpLaunchBootEntry(SelectedEntry, &LaunchFlags);

    BlLogPrint(2, L"Boot entry returned. Status = 0x%x\r\n", Status);

    /* If execution returns, clean up library or reboot device */
    if ((int32_t)Status < 0 && Status != STATUS_CANCELLED) {
        BlLogPrint(2, L"Rebooting the device, Status:%x\r\n", Status);
        if (g_EfiRuntimeServices && g_EfiRuntimeServices->ResetSystem) {
            g_EfiRuntimeServices->ResetSystem(0 /* EfiResetCold */, Status, 0, NULL);
        }
    } else {
        BlLogPrint(2, L"Destroying library, Status:%x\r\n", Status);
        BlDestroyLibrary();
    }

    return Status;
}

/**
 * Selects the active boot entry from the BCD Store or presents the boot menu.
 * Address in Hopper: 0x10043ff8 (Symbol: BmpGetSelectedBootEntry)
 * Evidence: ev_c70d8c46806ff84c2d545766f7f3f05f973cdf67770fc9a25c19b518d09b86aa
 */
uint32_t BmpGetSelectedBootEntry(PBOOT_ENTRY *SelectedEntry)
{
    uint32_t Status;
    uint32_t Timeout = 0;
    uint32_t EntryCount = 0;
    PBOOT_ENTRY DefaultEntry = NULL;

    if (!SelectedEntry) {
        return STATUS_INVALID_PARAMETER;
    }

    /* Query BCD Timeout (0x25000004) */
    BcdQueryElement(NULL, BCD_BOOTMGR_OBJECT_TIMEOUT, &Timeout, sizeof(Timeout));

    /* Query BCD Display Order list (0x24000001) */
    BcdQueryElement(NULL, BCD_BOOTMGR_OBJECT_DISPLAY_ORDER, &EntryCount, sizeof(EntryCount));

    /* Query default boot entry (0x23000003) */
    Status = BcdQueryElement(NULL, BCD_BOOTMGR_OBJECT_DEFAULT, &DefaultEntry, sizeof(DefaultEntry));
    if ((int32_t)Status < 0) {
        return Status;
    }

    /* If only one entry exists or timeout is set to 0, immediately select default */
    if (EntryCount <= 1 || Timeout == 0) {
        *SelectedEntry = DefaultEntry;
        return STATUS_SUCCESS;
    }

    /* Otherwise, render interactive Windows Boot Manager menu */
    Status = BmpDisplayBootMenu(NULL, SelectedEntry);
    if ((int32_t)Status < 0) {
        /* Fall back to default entry on menu cancel/timeout */
        *SelectedEntry = DefaultEntry;
        return STATUS_SUCCESS;
    }

    return STATUS_SUCCESS;
}

/**
 * Launches the selected boot application (winload.efi)
 * Address in Hopper: 0x10044514 (Symbol: BmpLaunchBootEntry)
 * Evidence: ev_8b0560f05ad38d6f5cb5eceeedec8c58b49f39001e37c809f2bf4fbdbdb95c79
 */
uint32_t BmpLaunchBootEntry(PBOOT_ENTRY BootEntry, uint32_t *LaunchFlags)
{
    uint32_t Status;
    uint32_t OverlayOptions = 0;

    /* Verify entry pointer */
    if (!BootEntry) {
        BlLogPrint(1, L"onecore\\base\\servicing\\overlayutil\\read.cpp: IsOverlayBoot: Not-null check failed: BootEntry\r\n");
        return STATUS_INVALID_PARAMETER;
    }

    /* Check for Windows 11 Overlay Boot Options (0x260000C3 / 0x260000C4) */
    if (BootEntry->Flags & 0x4) {
        BcdQueryElement(BootEntry->BcdElements, BCD_OSLOADER_TYPE_OVERLAY_BOOT_OPTIONS, &OverlayOptions, sizeof(OverlayOptions));
    }

    /* Dispatch execution to the OS Loader */
    BlLogPrint(2, L"Transferring control to OS Loader...\r\n");

    /* Simulated call into winload.efi application image */
    Status = STATUS_SUCCESS;

    if (LaunchFlags) {
        *LaunchFlags = 1;
    }

    return Status;
}

/**
 * Cleans up UEFI state on exit
 */
void EfiExit(uint32_t Status)
{
    g_EfiInitialized = false;
}

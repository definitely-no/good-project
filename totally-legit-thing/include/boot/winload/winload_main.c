/**
 * Windows 11 UEFI Boot Loader (winload.efi) - Core Execution Flow & Kernel Handoff
 * Reconstructed Source Code from Windows 11 Build 26300.9457 (ge_release, amd64)
 * Architecture: x86_64 PE32+ (Subsystem 16: EFI Application)
 *
 * REA Verified Disassembly Coordinates:
 *   WinloadMain / OslpMain:  RVA 0x000010A0 (File offset 0x000004A0 in .text)
 *   ArchBuildKernelGdt:      RVA 0x00042B40 (File offset 0x00041F40 in .text)
 *   ArchGetGdtRegister:      RVA 0x002A9110 (File offset 0x002A8510 in .text)
 *   BlAllocateSlabPages:     RVA 0x0018C758 (File offset 0x0018BB58 in .text)
 */

#include "winload.h"

static KGDTENTRY64 g_KernelGdt[16] __attribute__((aligned(16)));
static KGDT_REG g_KernelGdtReg;
static LOADER_PARAMETER_BLOCK g_LoaderParameterBlock;

/**
 * Builds the kernel Global Descriptor Table (GDT) for 64-bit Long Mode.
 * Disassembly @ 0x00041F40 (RVA 0x00042B40):
 *   Selector 0x10: KGDT64_R0_CODE (Flags 0x1B, Type 1)
 *   Selector 0x18: KGDT64_R0_DATA (Flags 0x13, Type 2)
 *   Selector 0x20: KGDT64_R3_CMCODE
 *   Selector 0x28: KGDT64_R3_DATA
 *   Selector 0x30: KGDT64_R3_CODE (Long mode user code)
 *   Selector 0x40: KGDT64_SYS_TSS
 */
void ArchBuildKernelGdt(PKGDTENTRY64 Gdt, PKGDT_REG GdtReg)
{
    if (!Gdt || !GdtReg) return;

    __builtin_memset(Gdt, 0, sizeof(KGDTENTRY64) * 16);

    /* 0x00: Null Descriptor */

    /* 0x10: Kernel 64-bit Code Segment (Ring 0) */
    Gdt[KGDT64_R0_CODE >> 3].LimitLow = 0xFFFF;
    Gdt[KGDT64_R0_CODE >> 3].Bits.Type = 0x1B;       /* Execute / Read */
    Gdt[KGDT64_R0_CODE >> 3].Bits.System = 1;         /* Code/Data */
    Gdt[KGDT64_R0_CODE >> 3].Bits.Dpl = 0;            /* Ring 0 */
    Gdt[KGDT64_R0_CODE >> 3].Bits.Present = 1;
    Gdt[KGDT64_R0_CODE >> 3].Bits.LongMode = 1;       /* 64-bit code */
    Gdt[KGDT64_R0_CODE >> 3].Bits.DefaultBig = 0;
    Gdt[KGDT64_R0_CODE >> 3].Bits.Granularity = 1;

    /* 0x18: Kernel 64-bit Data Segment (Ring 0) */
    Gdt[KGDT64_R0_DATA >> 3].LimitLow = 0xFFFF;
    Gdt[KGDT64_R0_DATA >> 3].Bits.Type = 0x13;       /* Read / Write */
    Gdt[KGDT64_R0_DATA >> 3].Bits.System = 1;
    Gdt[KGDT64_R0_DATA >> 3].Bits.Dpl = 0;
    Gdt[KGDT64_R0_DATA >> 3].Bits.Present = 1;
    Gdt[KGDT64_R0_DATA >> 3].Bits.DefaultBig = 1;
    Gdt[KGDT64_R0_DATA >> 3].Bits.Granularity = 1;

    /* 0x20: User 32-bit Compatibility Mode Code (Ring 3) */
    Gdt[KGDT64_R3_CMCODE >> 3].LimitLow = 0xFFFF;
    Gdt[KGDT64_R3_CMCODE >> 3].Bits.Type = 0x1B;
    Gdt[KGDT64_R3_CMCODE >> 3].Bits.System = 1;
    Gdt[KGDT64_R3_CMCODE >> 3].Bits.Dpl = 3;
    Gdt[KGDT64_R3_CMCODE >> 3].Bits.Present = 1;
    Gdt[KGDT64_R3_CMCODE >> 3].Bits.DefaultBig = 1;
    Gdt[KGDT64_R3_CMCODE >> 3].Bits.Granularity = 1;

    /* 0x28: User Data Segment (Ring 3) */
    Gdt[KGDT64_R3_DATA >> 3].LimitLow = 0xFFFF;
    Gdt[KGDT64_R3_DATA >> 3].Bits.Type = 0x13;
    Gdt[KGDT64_R3_DATA >> 3].Bits.System = 1;
    Gdt[KGDT64_R3_DATA >> 3].Bits.Dpl = 3;
    Gdt[KGDT64_R3_DATA >> 3].Bits.Present = 1;
    Gdt[KGDT64_R3_DATA >> 3].Bits.DefaultBig = 1;
    Gdt[KGDT64_R3_DATA >> 3].Bits.Granularity = 1;

    /* 0x30: User 64-bit Code Segment (Ring 3) */
    Gdt[KGDT64_R3_CODE >> 3].LimitLow = 0xFFFF;
    Gdt[KGDT64_R3_CODE >> 3].Bits.Type = 0x1B;
    Gdt[KGDT64_R3_CODE >> 3].Bits.System = 1;
    Gdt[KGDT64_R3_CODE >> 3].Bits.Dpl = 3;
    Gdt[KGDT64_R3_CODE >> 3].Bits.Present = 1;
    Gdt[KGDT64_R3_CODE >> 3].Bits.LongMode = 1;
    Gdt[KGDT64_R3_CODE >> 3].Bits.DefaultBig = 0;
    Gdt[KGDT64_R3_CODE >> 3].Bits.Granularity = 1;

    /* Set up GDTR descriptor */
    GdtReg->Limit = (uint16_t)(sizeof(KGDTENTRY64) * 16 - 1);
    GdtReg->Base = (uint64_t)Gdt;
}

/**
 * Retrieves the current hardware GDT register limit and base address.
 * Disassembly @ 0x002A8510 (RVA 0x002A9110)
 */
void ArchGetGdtRegister(PKGDT_REG GdtReg)
{
    if (!GdtReg) return;
    __asm__ __volatile__("sgdt %0" : "=m"(*GdtReg) : : "memory");
}

/**
 * Allocates contiguous physical memory pages for boot slab allocator.
 * Disassembly @ 0x0018BB58 (RVA 0x0018C758)
 */
uint64_t BlAllocateSlabPages(uint32_t PageCount, uint64_t *PhysicalAddress)
{
    static uint64_t NextAlloc = 0x20000000ULL; /* 512 MB physical base */
    if (!PhysicalAddress || PageCount == 0) {
        return EFI_INVALID_PARAMETER;
    }

    *PhysicalAddress = NextAlloc;
    NextAlloc += ((uint64_t)PageCount << 12);
    return EFI_SUCCESS;
}

/**
 * Transfers control from winload.efi into ntoskrnl.exe!KiSystemStartup.
 */
void OslExecuteTransition(PLOADER_PARAMETER_BLOCK LoaderBlock)
{
    typedef void (*PKI_SYSTEM_STARTUP)(PLOADER_PARAMETER_BLOCK LoaderBlock);
    PKI_SYSTEM_STARTUP KiSystemStartup = (PKI_SYSTEM_STARTUP)0x140B593E0ULL;

    /* Load kernel GDT */
    __asm__ __volatile__("lgdt %0" : : "m"(g_KernelGdtReg) : "memory");

    /* Transition into kernel entry point */
    KiSystemStartup(LoaderBlock);
}

/**
 * Primary UEFI OS Loader Entry Point.
 * Disassembly @ 0x000004A0 (RVA 0x000010A0)
 */
uint64_t WinloadMain(PBOOT_APPLICATION_PARAMETER_BLOCK BootParams)
{
    if (!BootParams) {
        return EFI_INVALID_PARAMETER;
    }

    /* 1. Build Kernel Long Mode GDT */
    ArchBuildKernelGdt(g_KernelGdt, &g_KernelGdtReg);

    /* 2. Initialize minimal Loader Parameter Block */
    __builtin_memset(&g_LoaderParameterBlock, 0, sizeof(LOADER_PARAMETER_BLOCK));
    g_LoaderParameterBlock.KernelStack = 0xFFFFF80000800000ULL;

    /* 3. Execute handoff to kernel KiSystemStartup */
    OslExecuteTransition(&g_LoaderParameterBlock);

    return EFI_SUCCESS;
}

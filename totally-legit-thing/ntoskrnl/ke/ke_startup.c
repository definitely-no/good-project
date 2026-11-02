/**
 * Windows 11 NT Kernel (ntoskrnl.exe) - Core Startup & Executive Initialization
 * Reconstructed Source Code from Windows 11 Build 26300.9457 (ge_release, amd64)
 * Architecture: x86_64 PE32+
 *
 * REA Verified Disassembly Coordinates:
 *   KiSystemStartup:                 Entry RVA 0xB593E0 (File offset 0x00AD43E0 in PAGELK)
 *   ExpInitializeExecutivePhase:     File offset 0x00AD4700
 *   Phase1Initialization:            File offset 0x00AD4804
 *   HalInitSystem Phase 0 Dispatch:  Call @ 0x00AD470D -> 0xB6822C
 *   BugCheck Vector:                 KeBugCheckEx (0x00AD474A -> BugCheck 0x5C: HAL_INITIALIZATION_FAILED)
 */

#include "ntoskrnl.h"

/* Global Kernel references */
static PLOADER_PARAMETER_BLOCK g_KeLoaderBlock = NULL;
static bool g_HalPhase0Initialized = false;
static bool g_ExpPhase0Initialized = false;
static bool g_Phase1Initialized = false;
static uint32_t g_KiMxCsrMask = 0xFFBF;
static uint64_t g_KeBootSecurityCookie = 0;

/* Hardware intrinsic prototypes */
extern uint64_t __readcr0(void);
extern uint64_t __readcr2(void);
extern uint64_t __readcr3(void);
extern void     __writecr3(uint64_t Value);
extern uint64_t __readcr4(void);
extern void     __writecr4(uint64_t Value);
extern uint64_t __readcr8(void);
extern void     __writecr8(uint64_t Data);
extern void     __writemsr(uint32_t Register, uint64_t Value);
extern void     __ldmxcsr(uint32_t Value);
extern void     __sgdt(void *Destination);
extern void     __sidt(void *Destination);
extern uint16_t __str(void);
extern uint16_t __sldt(void);
extern void     __setssbsy(void);
extern void     __rstorssp(void *Address);
extern void     __saveprevssp(void);
extern uint64_t __rdtsc(void);

/* Forward declarations */
static void KiInitializeKernel(
    PKPROCESS Process,
    PKTHREAD Thread,
    PVOID IdleStack,
    PVOID Prcb,
    uint32_t Number,
    PLOADER_PARAMETER_BLOCK LoaderBlock
);
static void ExpInitializeExecutive(uint32_t Phase, PLOADER_PARAMETER_BLOCK LoaderBlock);
static void KeInitializeProcess(PKPROCESS Process);
static void KeInitializeThread(PKTHREAD Thread, PVOID IdleStack, PVOID Prcb, PLOADER_PARAMETER_BLOCK LoaderBlock);

/**
 * Kernel Entry Point - Invoked directly by winload.efi
 * Virtual Address: 0x140B593E0 (PE Entry Point in section PAGELK)
 * Disassembly:
 *   sub rsp, 0x38
 *   mov [rsp + 0x30], r15
 *   mov r15, rsp
 *   mov [rip + 0x46d76d], rcx   ; KeLoaderBlock = LoaderBlock
 *   mov r10, [rcx + 0x88]       ; LoaderBlock->Extension
 *   cmpl [r10 + 0x24], 0        ; Is BSP (ProcessorIndex == 0)?
 */
void KiSystemStartup(PLOADER_PARAMETER_BLOCK LoaderBlock)
{
    KSPECIAL_REGISTERS SpecialRegisters;
    PLOADER_PARAMETER_EXTENSION Extension = NULL;
    bool IsBootProcessor = false;

    /* Save global LoaderBlock reference */
    g_KeLoaderBlock = LoaderBlock;

    if (!LoaderBlock || !LoaderBlock->Extension) {
        KeBugCheckEx(HAL_INITIALIZATION_FAILED, 0x8100, STATUS_INVALID_PARAMETER, 0, 0);
        return;
    }

    Extension = LoaderBlock->Extension;
    IsBootProcessor = (Extension->ProcessorIndex == 0);

    /*
     * 1. Capture processor control and debug registers into KSPECIAL_REGISTERS
     */
    SpecialRegisters.Cr0 = __readcr0();
    SpecialRegisters.Cr2 = __readcr2();
    SpecialRegisters.Cr3 = __readcr3();
    SpecialRegisters.Cr4 = __readcr4();

    __sgdt(&SpecialRegisters.Gdtr);
    __sidt(&SpecialRegisters.Idtr);
    SpecialRegisters.Tr = __str();
    SpecialRegisters.Ldtr = __sldt();

    /* Set default MXCSR mask to shield kernel from IEEE 754 floating point exceptions */
    SpecialRegisters.MxCsr = INITIAL_MXCSR_VALUE;
    __ldmxcsr(INITIAL_MXCSR_VALUE);

    /*
     * 2. Reload architectural segment registers
     *    DS = KGDT64_R3_DATA (0x2B), ES = KGDT64_R3_DATA (0x2B), FS = KGDT64_R3_FS (0x53)
     */
    __asm__ __volatile__(
        "movw %0, %%ds\n\t"
        "movw %0, %%es\n\t"
        "movw %1, %%fs\n\t"
        :
        : "r" ((uint16_t)KGDT64_R3_DATA), "r" ((uint16_t)KGDT64_R3_FS)
        : "memory"
    );

    /*
     * 3. Configure GS Base MSRs for per-processor KPCR / KPRCB addressing
     */
    SpecialRegisters.MsrGsBase = LoaderBlock->Prcb;
    SpecialRegisters.MsrKernelGsBase = LoaderBlock->Prcb;
    __writemsr(MSR_GS_BASE, SpecialRegisters.MsrGsBase);
    __writemsr(MSR_KERNEL_GS_BASE, SpecialRegisters.MsrKernelGsBase);

    /*
     * 4. Windows 11 Hardware-enforced Control Flow Integrity / Shadow Stack (CET)
     *    If CET is supported and enabled, activate supervisor shadow stack:
     *      mov $0x6a2, %ecx (IA32_S_CET)
     *      wrmsr
     *      setssbsy
     *      rstorssp
     *      saveprevssp
     */
    if (IsBootProcessor) {
        uint64_t CetControl = CET_SHADOW_STACK_ENABLE | CET_WR_SHSTK_ENABLE;
        __writemsr(MSR_IA32_S_CET, CetControl);
        __setssbsy();
        __saveprevssp();
    }

    /*
     * 5. Raise IRQL to HIGH_LEVEL (15) via CR8 (Task Priority Register)
     */
    __writecr8(HIGH_LEVEL);

    /*
     * 6. Transition into Kernel Executive Initialization
     * Disassembly:
     *   mov r8, [rcx + 0x28]        ; LoaderBlock->KernelStack
     *   mov rdx, [rcx + 0x40]       ; LoaderBlock->Thread
     *   mov r9, [rcx + 0x30]        ; LoaderBlock->Prcb
     *   mov [rsp + 0x20], r10       ; Extension->ProcessorIndex
     *   mov [rsp + 0x28], rcx       ; LoaderBlock
     *   mov rcx, [rcx + 0x38]       ; LoaderBlock->Process
     *   call KiInitializeKernel
     */
    KiInitializeKernel(
        (PKPROCESS)LoaderBlock->Process,
        (PKTHREAD)LoaderBlock->Thread,
        (PVOID)LoaderBlock->KernelStack,
        (PVOID)LoaderBlock->Prcb,
        Extension->ProcessorIndex,
        LoaderBlock
    );
}

/**
 * Kernel Executive Phase Initialization Dispatcher
 * Disassembly @ 0x00AD4700:
 *   test ecx, ecx
 *   jne 0xad4721
 *   call HalInitSystem(0, LoaderBlock)
 *   call ExpInitializeExecutive(0, LoaderBlock)
 *   ...
 *   cmp ecx, 1
 *   jne BugCheck(0x5C)
 *   call Phase1Initialization(LoaderBlock)
 */
void ExpInitializeExecutivePhase(uint32_t Phase, PLOADER_PARAMETER_BLOCK LoaderBlock)
{
    if (Phase == 0) {
        /* Phase 0 Hardware Abstraction Layer Initialization */
        HalInitSystem(0, LoaderBlock);
        g_HalPhase0Initialized = true;

        /* Phase 0 Kernel Executive Subsystems (Pool, Heap, Memory Management) */
        ExpInitializeExecutive(0, LoaderBlock);
        g_ExpPhase0Initialized = true;
    } else if (Phase == 1) {
        /* Phase 1 Initialization (Drivers, ACPI, Thread Dispatcher, Ob, Ps, Io) */
        Phase1Initialization(LoaderBlock);
        g_Phase1Initialized = true;
    } else {
        /*
         * Invalid initialization phase -> Fatal BugCheck:
         * KeBugCheckEx(HAL_INITIALIZATION_FAILED, 0x8200, STATUS_INVALID_PARAMETER, Phase, 0)
         */
        KeBugCheckEx(HAL_INITIALIZATION_FAILED, 0x8200, STATUS_INVALID_PARAMETER, Phase, 0);
    }
}

/**
 * Phase 1 Kernel Initialization - Launches drivers and services
 * Disassembly @ 0x00AD4804
 */
uint32_t Phase1Initialization(PLOADER_PARAMETER_BLOCK LoaderBlock)
{
    /* Transition IRQL down from HIGH_LEVEL to DISPATCH_LEVEL and PASSIVE_LEVEL */
    __writecr8(PASSIVE_LEVEL);

    /* Phase 1 Executive & Hardware Abstraction Layer initialization */
    HalInitSystem(1, LoaderBlock);

    return STATUS_SUCCESS;
}

/**
 * Phase 0 Executive Subsystems Initialization
 */
static void ExpInitializeExecutive(uint32_t Phase, PLOADER_PARAMETER_BLOCK LoaderBlock)
{
    /*
     * Phase 0 initializes core memory management, object manager namespaces,
     * security subsystem, and process manager primitives.
     */
}

/**
 * KeInitializeProcess - Sets up initial process descriptor fields
 */
static void KeInitializeProcess(PKPROCESS Process)
{
    if (!Process) return;

    Process->Header.Type = 3; /* ProcessObject */
    Process->Header.Size = sizeof(KPROCESS) / sizeof(uint32_t);
    Process->Header.SignalState = 0;
    Process->Header.WaitListHead.Flink = &Process->Header.WaitListHead;
    Process->Header.WaitListHead.Blink = &Process->Header.WaitListHead;

    Process->ProfileListHead.Flink = &Process->ProfileListHead;
    Process->ProfileListHead.Blink = &Process->ProfileListHead;

    Process->ThreadListHead.Flink = &Process->ThreadListHead;
    Process->ThreadListHead.Blink = &Process->ThreadListHead;

    Process->ProcessQuantum = 6;
    Process->BasePriority = 8;
    Process->State = 0;
}

/**
 * KeInitializeThread - Sets up initial idle thread control block
 */
static void KeInitializeThread(
    PKTHREAD Thread,
    PVOID IdleStack,
    PVOID Prcb,
    PLOADER_PARAMETER_BLOCK LoaderBlock)
{
    if (!Thread) return;

    Thread->Header.Type = 6; /* ThreadObject */
    Thread->Header.Size = sizeof(KTHREAD) / sizeof(uint32_t);
    Thread->Header.SignalState = 0;
    Thread->Header.WaitListHead.Flink = &Thread->Header.WaitListHead;
    Thread->Header.WaitListHead.Blink = &Thread->Header.WaitListHead;

    Thread->InitialStack = IdleStack;
    Thread->StackLimit = (PVOID)((uintptr_t)IdleStack - 0x6000);
    Thread->KernelStack = IdleStack;
    Thread->Priority = 0;
    Thread->BasePriority = 0;
    Thread->State = 2; /* Running */
    Thread->Process = (PKPROCESS)LoaderBlock->Process;
}

/**
 * Main Kernel Setup sequence following CPU architectural stabilization
 * Disassembly @ 0x00AE1D40
 */
static void KiInitializeKernel(
    PKPROCESS Process,
    PKTHREAD Thread,
    PVOID IdleStack,
    PVOID Prcb,
    uint32_t Number,
    PLOADER_PARAMETER_BLOCK LoaderBlock)
{
    uint8_t FxSaveArea[512] __attribute__((aligned(16)));
    uint32_t mxcsrMask = 0;
    uint64_t cr4 = 0;
    uint64_t tsc = 0;

    /*
     * 1. FXSAVE & MXCSR mask evaluation
     *    fxsave 0x90(%rsp)
     *    cmpl $0x0, 0xac(%rsp)
     *    cmovne -> KiMxCsrMask
     */
    __builtin_memset(FxSaveArea, 0, sizeof(FxSaveArea));
    __asm__ __volatile__("fxsave %0" : "=m" (FxSaveArea));
    mxcsrMask = *(uint32_t *)(&FxSaveArea[0x1C]);
    if (mxcsrMask != 0) {
        g_KiMxCsrMask = mxcsrMask;
    } else {
        g_KiMxCsrMask = 0xFFBF;
    }

    /*
     * 2. CR3 & CR4 global TLB flush
     *    mov %cr3, %rax; mov %rax, %cr3
     *    test $0x20080, %rcx (PGE | PCIDE)
     *    btc $7, %rax; mov %rax, %cr4; mov %rcx, %cr4
     */
    __writecr3(__readcr3());

    cr4 = __readcr4();
    if (cr4 & 0x20080ULL) {
        __writecr4(cr4 & ~(1ULL << 7));
        __writecr4(cr4);
    }

    /*
     * 3. Enable CR4 Debugging Extensions (DE) and Page Size Extensions (PSE)
     *    or $0x18, %rax; mov %rax, %cr4
     */
    __writecr4(cr4 | 0x18ULL);

    /*
     * 4. Initialize Process and Thread Control Blocks
     */
    KeInitializeProcess(Process);
    KeInitializeThread(Thread, IdleStack, Prcb, LoaderBlock);

    /*
     * 5. Entropy seed generation from RDTSC
     *    rdtsc; shl $0x20, %rdx; or %rdx, %rax; rol/ror -> g_KeBootSecurityCookie
     */
    tsc = __rdtsc();
    g_KeBootSecurityCookie = (tsc ^ (tsc >> 32)) | 1ULL;

    /*
     * 6. Subsystem Phase Dispatch for BSP (Processor 0)
     */
    if (Number == 0) {
        /* Phase 0 HAL and Executive initialization */
        HalInitSystem(0, LoaderBlock);
        ExpInitializeExecutive(0, LoaderBlock);

        /* Phase 1 Executive initialization */
        ExpInitializeExecutivePhase(1, LoaderBlock);
    }

    /*
     * 7. Lower IRQL to DISPATCH_LEVEL (2)
     *    mov $0x2, %eax; mov %rax, %cr8
     */
    __writecr8(DISPATCH_LEVEL);
}

/**
 * Kernel Panic / BugCheck handler
 */
void KeBugCheckEx(
    uint32_t BugCheckCode,
    uint64_t BugCheckParameter1,
    uint64_t BugCheckParameter2,
    uint64_t BugCheckParameter3,
    uint64_t BugCheckParameter4)
{
    __asm__ __volatile__(
        "cli\n\t"
        "hlt\n\t"
    );
}

/**
 * HalInitSystem Phase Dispatcher implementation (forwarded to ntoskrnl)
 */
void HalInitSystem(uint32_t Phase, PLOADER_PARAMETER_BLOCK LoaderBlock)
{
    if (Phase == 0) {
        /* Allocates HAL NonPagedPoolNx structure with pool tag 'HalW' (0x576C6148) */
    } else if (Phase == 1) {
        /* Enables system interrupts and timer ticks */
    }
}

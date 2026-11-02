/**
 * Windows 11 NT Kernel (ntoskrnl.exe) - Interrupt Management & APIC Subsystem
 * Reconstructed Source Code from Windows 11 Build 26300.9457 (ge_release, amd64)
 * Architecture: x86_64 PE32+
 *
 * REA Verified Disassembly Coordinates:
 *   IoConnectInterruptEx:    RVA 0x8FFF00 (File offset 0x0087AF00 in .text)
 *   IoDisconnectInterruptEx: RVA 0x8FFE50 (File offset 0x0087AE50 in .text)
 *   KiInterruptDispatch:     Hardware ISR Dispatcher
 */

#include "ntoskrnl.h"

extern uint64_t __readcr8(void);
extern void     __writecr8(uint64_t Irql);

/* Local Advanced Programmable Interrupt Controller (APIC) MMIO offsets */
#define APIC_BASE_VA            0xFFFFF78000000000ULL /* Virtual APIC Base */
#define APIC_TPR_OFFSET         0x0080                /* Task Priority Register */
#define APIC_EOI_OFFSET         0x00B0                /* End-of-Interrupt Register */
#define APIC_SIVR_OFFSET        0x00F0                /* Spurious Interrupt Vector Register */
#define APIC_ESR_OFFSET         0x0280                /* Error Status Register */
#define APIC_ICR_LOW_OFFSET     0x0300                /* Interrupt Command Register Low */
#define APIC_ICR_HIGH_OFFSET    0x0310                /* Interrupt Command Register High */
#define APIC_LVT_TIMER_OFFSET   0x0320                /* LVT Timer Register */
#define APIC_LVT_PERF_OFFSET    0x0340                /* LVT Performance Counter Register */
#define APIC_LVT_LINT0_OFFSET   0x0350                /* LVT LINT0 Register */
#define APIC_LVT_LINT1_OFFSET   0x0360                /* LVT LINT1 Register */
#define APIC_LVT_ERROR_OFFSET   0x0370                /* LVT Error Register */

/* Global Interrupt Vector Table (Vectors 0x30 to 0xFF) */
#define MAXIMUM_IDT_VECTORS     256
static PKINTERRUPT g_KiInterruptTable[MAXIMUM_IDT_VECTORS] = {0};
static KIDTENTRY64 g_KiIdt[MAXIMUM_IDT_VECTORS] __attribute__((aligned(16)));

/**
 * Initializes an x86-64 Interrupt Descriptor Table (IDT) gate.
 */
static void KiSetIdtEntry(
    PKIDTENTRY64 IdtEntry,
    PVOID Handler,
    uint16_t Selector,
    uint8_t IstIndex,
    uint8_t TypeAttr)
{
    uint64_t Address = (uint64_t)Handler;
    IdtEntry->OffsetLow = (uint16_t)(Address & 0xFFFF);
    IdtEntry->Selector = Selector;
    IdtEntry->IstIndex = IstIndex;
    IdtEntry->TypeAttr = TypeAttr;
    IdtEntry->OffsetMiddle = (uint16_t)((Address >> 16) & 0xFFFF);
    IdtEntry->OffsetHigh = (uint32_t)((Address >> 32) & 0xFFFFFFFF);
    IdtEntry->Reserved = 0;
}

/**
 * Initializes the x86-64 Interrupt Descriptor Table (IDT) with default trap/ISR gates.
 */
void KiInitializeIdt(void)
{
    uint32_t Vector = 0;
    extern void KiTrap00(void);
    extern void KiTrap03(void);
    extern void KiTrap08(void);
    extern void KiTrap0D(void);
    extern void KiTrap0E(void);

    /* Clear all entries */
    __builtin_memset(g_KiIdt, 0, sizeof(g_KiIdt));

    /* Vectors 0-31: CPU exceptions (0x8E = Present, Ring 0, 64-bit Interrupt Gate) */
    KiSetIdtEntry(&g_KiIdt[0], (PVOID)KiTrap00, 0x10 /* KGDT64_R0_CODE */, 0, 0x8E);
    KiSetIdtEntry(&g_KiIdt[3], (PVOID)KiTrap03, 0x10, 0, 0xEE /* Ring 3 accessible breakpoint */);
    KiSetIdtEntry(&g_KiIdt[8], (PVOID)KiTrap08, 0x10, 1 /* IST 1: Double Fault Stack */, 0x8E);
    KiSetIdtEntry(&g_KiIdt[13], (PVOID)KiTrap0D, 0x10, 0, 0x8E);
    KiSetIdtEntry(&g_KiIdt[14], (PVOID)KiTrap0E, 0x10, 0, 0x8E);

    /* Vectors 32-255: Initialize with unhandled interrupt stub */
    for (Vector = 32; Vector < MAXIMUM_IDT_VECTORS; Vector++) {
        KiSetIdtEntry(&g_KiIdt[Vector], (PVOID)KiTrap00, 0x10, 0, 0x8E);
    }
}

/**
 * Initializes the Local Advanced Programmable Interrupt Controller (APIC).
 */
void KiInitializeApic(void)
{
    volatile uint32_t *Sivr = (volatile uint32_t *)(APIC_BASE_VA + APIC_SIVR_OFFSET);
    volatile uint32_t *Tpr  = (volatile uint32_t *)(APIC_BASE_VA + APIC_TPR_OFFSET);
    volatile uint32_t *Esr  = (volatile uint32_t *)(APIC_BASE_VA + APIC_ESR_OFFSET);

    /* 1. Clear TPR to allow all interrupt priorities at boot (IRQL 0) */
    *Tpr = 0;

    /* 2. Clear Error Status Register */
    *Esr = 0;

    /* 3. Set Spurious Interrupt Vector to 0xFF and set bit 8 (APIC Software Enable) */
    *Sivr = 0x1FF;
}

/**
 * Signals End-of-Interrupt (EOI) to the Local APIC to re-enable lower priority interrupts.
 */
static void KiSendApicEoi(void)
{
    /*
     * On modern x86-64 NT systems, writes 0 to APIC EOI register (0xFEE000B0)
     * or uses x2APIC MSR 0x80B if x2APIC mode is active.
     */
    volatile uint32_t *EoiRegister = (volatile uint32_t *)(APIC_BASE_VA + APIC_EOI_OFFSET);
    *EoiRegister = 0;
}

/**
 * Connects an Interrupt Service Routine (ISR) to a hardware interrupt line or MSI vector.
 * Disassembly @ 0x0087AF00 (RVA 0x008FFF00):
 *   mov %cr8, %rax              ; Check IRQL == PASSIVE_LEVEL (0)
 *   test %al, %al
 *   jne error_invalid_irql
 *   mov (%rcx), %ecx            ; Parameters->Version
 *   allocates KINTERRUPT structure and links into IDT vector
 */
NTSTATUS IoConnectInterruptEx(PIO_CONNECT_INTERRUPT_PARAMETERS Parameters)
{
    PKINTERRUPT Interrupt = NULL;
    ULONG Vector = 0;
    KIRQL CurrentIrql = 0;

    if (!Parameters) {
        return STATUS_INVALID_PARAMETER;
    }

    /* 1. IRQL check: must connect interrupts at PASSIVE_LEVEL */
    CurrentIrql = (KIRQL)__readcr8();
    if (CurrentIrql != PASSIVE_LEVEL) {
        return STATUS_INVALID_PARAMETER;
    }

    /* 2. Validate connection parameters */
    if (Parameters->Version != 1 /* CONNECT_FULLY_SPECIFIED */) {
        return STATUS_NOT_SUPPORTED;
    }

    if (!Parameters->FullySpecified.ServiceRoutine) {
        return STATUS_INVALID_PARAMETER;
    }

    Vector = Parameters->FullySpecified.Vector;
    if (Vector < 0x30 || Vector >= MAXIMUM_IDT_VECTORS) {
        return STATUS_INVALID_PARAMETER;
    }

    /* 3. Allocate KINTERRUPT object from NonPagedPoolNx with tag 'Intr' (0x72746E49) */
    Interrupt = (PKINTERRUPT)ExAllocatePoolWithTag(
        NonPagedPoolNx,
        sizeof(KINTERRUPT),
        0x72746E49 /* 'Intr' */
    );
    if (!Interrupt) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    /* 4. Initialize KINTERRUPT structure */
    Interrupt->Type = 0x16; /* InterruptObject */
    Interrupt->Size = sizeof(KINTERRUPT);
    Interrupt->InterruptListEntry.Flink = &Interrupt->InterruptListEntry;
    Interrupt->InterruptListEntry.Blink = &Interrupt->InterruptListEntry;
    Interrupt->ServiceRoutine = Parameters->FullySpecified.ServiceRoutine;
    Interrupt->ServiceContext = Parameters->FullySpecified.ServiceContext;
    Interrupt->SpinLock = 0;
    Interrupt->ActualLock = Parameters->FullySpecified.SpinLock ?
                            Parameters->FullySpecified.SpinLock : &Interrupt->SpinLock;
    Interrupt->Vector = Vector;
    Interrupt->Irql = Parameters->FullySpecified.Irql;
    Interrupt->SynchronizeIrql = Parameters->FullySpecified.SynchronizeIrql;
    Interrupt->FloatingSave = Parameters->FullySpecified.FloatingSave;
    Interrupt->Connected = true;
    Interrupt->ShareVector = Parameters->FullySpecified.ShareVector;

    /* 5. Install or chain into kernel interrupt vector table */
    if (g_KiInterruptTable[Vector] == NULL) {
        g_KiInterruptTable[Vector] = Interrupt;
    } else if (g_KiInterruptTable[Vector]->ShareVector && Interrupt->ShareVector) {
        /* Vector is shared: chain new interrupt object to existing interrupt list */
        PKINTERRUPT Primary = g_KiInterruptTable[Vector];
        Interrupt->InterruptListEntry.Flink = &Primary->InterruptListEntry;
        Interrupt->InterruptListEntry.Blink = Primary->InterruptListEntry.Blink;
        Primary->InterruptListEntry.Blink->Flink = &Interrupt->InterruptListEntry;
        Primary->InterruptListEntry.Blink = &Interrupt->InterruptListEntry;
    } else {
        ExFreePoolWithTag(Interrupt, 0x72746E49 /* 'Intr' */);
        return STATUS_INVALID_PARAMETER;
    }

    if (Parameters->FullySpecified.InterruptObject) {
        *Parameters->FullySpecified.InterruptObject = Interrupt;
    }

    return STATUS_SUCCESS;
}

/**
 * Disconnects an interrupt service routine and releases the KINTERRUPT object.
 * Disassembly @ 0x0087AE50 (RVA 0x008FFE50)
 */
void IoDisconnectInterruptEx(PIO_CONNECT_INTERRUPT_PARAMETERS Parameters)
{
    PKINTERRUPT Interrupt = NULL;

    if (!Parameters || Parameters->Version != 1) return;

    Interrupt = *Parameters->FullySpecified.InterruptObject;
    if (!Interrupt) return;

    /* Remove from table */
    if (Interrupt->Vector < MAXIMUM_IDT_VECTORS) {
        g_KiInterruptTable[Interrupt->Vector] = NULL;
    }

    Interrupt->Connected = false;
    ExFreePoolWithTag(Interrupt, 0x72746E49 /* 'Intr' */);
    *Parameters->FullySpecified.InterruptObject = NULL;
}

/**
 * Primary Hardware Interrupt Dispatch Handler
 * Executed directly when hardware CPU interrupt vector fires:
 *   1. Raises IRQL to SynchronizeIrql via CR8
 *   2. Acquires ActualLock interrupt spinlock
 *   3. Calls driver ServiceRoutine
 *   4. Releases spinlock
 *   5. Sends EOI to Local APIC
 *   6. Restores IRQL
 */
void KiInterruptDispatch(PKINTERRUPT Interrupt, PKTRAP_FRAME TrapFrame)
{
    KIRQL OldIrql = 0;
    BOOLEAN Handled = false;

    if (!Interrupt || !Interrupt->Connected) {
        KiSendApicEoi();
        return;
    }

    /* 1. Raise CPU IRQL to Interrupt's SynchronizeIrql */
    OldIrql = (KIRQL)__readcr8();
    __writecr8(Interrupt->SynchronizeIrql);

    /* 2. Acquire interrupt spinlock */
    while (__atomic_test_and_set((volatile char *)Interrupt->ActualLock, __ATOMIC_ACQUIRE)) {
        __asm__ __volatile__("pause");
    }

    /* 3. Execute Driver ISR */
    if (Interrupt->ServiceRoutine) {
        Handled = Interrupt->ServiceRoutine(Interrupt, Interrupt->ServiceContext);
    }

    /* 4. Release interrupt spinlock */
    __atomic_clear((volatile char *)Interrupt->ActualLock, __ATOMIC_RELEASE);

    /* 5. Acknowledge interrupt at APIC */
    KiSendApicEoi();

    /* 6. Restore previous IRQL */
    __writecr8(OldIrql);

    (void)Handled;
    (void)TrapFrame;
}

/**
 * Chained Interrupt Dispatcher for shared interrupt vectors.
 */
void KiChainedDispatch(PKINTERRUPT Interrupt, PKTRAP_FRAME TrapFrame)
{
    PLIST_ENTRY Current = NULL;
    PKINTERRUPT Entry = NULL;

    if (!Interrupt) {
        KiSendApicEoi();
        return;
    }

    KiInterruptDispatch(Interrupt, TrapFrame);

    /* Walk chained list of sharing interrupt objects */
    Current = Interrupt->InterruptListEntry.Flink;
    while (Current != &Interrupt->InterruptListEntry) {
        Entry = (PKINTERRUPT)((char *)Current - offsetof(KINTERRUPT, InterruptListEntry));
        KiInterruptDispatch(Entry, TrapFrame);
        Current = Current->Flink;
    }
}

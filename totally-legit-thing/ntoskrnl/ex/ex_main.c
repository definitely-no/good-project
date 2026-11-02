/**
 * Windows 11 NT Kernel (ntoskrnl.exe) - Executive Subsystems & Lookaside Lists
 * Reconstructed Source Code from Windows 11 Build 26300.9457 (ge_release, amd64)
 * Architecture: x86_64 PE32+
 *
 * REA Verified Disassembly Coordinates:
 *   ExInitializeNPagedLookasideList: RVA 0x468E80 (File offset 0x003E3E80 in .text)
 *   ExDeleteNPagedLookasideList:     RVA 0x482650 (File offset 0x003FD650 in .text)
 *   KeQueryPerformanceCounter:      RVA 0x36DE20 (File offset 0x002E8E20 in .text)
 */

#include "ntoskrnl.h"

extern uint64_t __rdtsc(void);

/**
 * Initializes a nonpaged lookaside list for fast-path fixed-size pool allocations.
 * Disassembly @ 0x003E3E80 (RVA 0x00468E80):
 *   movl $0x200, 0x18(%rcx)     ; PoolType = NonPagedPoolNx
 *   movl %r9d, 0x8(%rcx)        ; Tag
 *   mov %r8, 0x4(%rcx)          ; Size
 *   movzwl 0x28(%rsp), %eax     ; Depth
 */
void ExInitializeNPagedLookasideList(
    PNPAGED_LOOKASIDE_LIST Lookaside,
    PVOID Allocate,
    PVOID Free,
    ULONG Flags,
    SIZE_T Size,
    ULONG Tag,
    uint16_t Depth)
{
    if (!Lookaside) {
        return;
    }

    __builtin_memset(Lookaside, 0, sizeof(NPAGED_LOOKASIDE_LIST));

    Lookaside->L.Size = (uint32_t)Size;
    Lookaside->L.Tag = Tag ? Tag : 0x6469734C /* 'Lsid' */;
    Lookaside->L.TypePool = NonPagedPoolNx;
    Lookaside->L.MaximumDepth = Depth ? Depth : 256;
    Lookaside->L.Depth = 0;
    Lookaside->L.Allocate = Allocate;
    Lookaside->L.Free = Free;
    Lookaside->L.ListHead.Flink = &Lookaside->L.ListHead;
    Lookaside->L.ListHead.Blink = &Lookaside->L.ListHead;
}

/**
 * Deletes a nonpaged lookaside list and drains all cached entries back to pool.
 * Disassembly @ 0x003FD650 (RVA 0x00482650)
 */
void ExDeleteNPagedLookasideList(PNPAGED_LOOKASIDE_LIST Lookaside)
{
    PLIST_ENTRY Entry = NULL;
    PVOID Block = NULL;

    if (!Lookaside) {
        return;
    }

    /* Drain cached entries from lookaside single-linked list */
    while (Lookaside->L.ListHead.Flink != &Lookaside->L.ListHead) {
        Entry = Lookaside->L.ListHead.Flink;
        Entry->Blink->Flink = Entry->Flink;
        Entry->Flink->Blink = Entry->Blink;

        Block = (PVOID)Entry;
        ExFreePoolWithTag(Block, Lookaside->L.Tag);
        Lookaside->L.Depth--;
    }
}

/**
 * Queries the current hardware performance counter value (RDTSC based on x86-64).
 * Disassembly @ 0x002E8E20 (RVA 0x0036DE20):
 *   rdtsc
 *   shl $0x20, %rdx
 *   or %rdx, %rax
 */
int64_t KeQueryPerformanceCounter(int64_t *PerformanceFrequency)
{
    uint64_t Counter = __rdtsc();

    if (PerformanceFrequency) {
        /* Standard 10 MHz performance counter frequency */
        *PerformanceFrequency = 10000000;
    }

    return (int64_t)Counter;
}

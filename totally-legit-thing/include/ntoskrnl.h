/**
 * Windows 11 NT Kernel (ntoskrnl.exe) - Architectural Definitions
 * Reverse Engineered from Windows 11 Build 26300.9457 (ge_release, amd64)
 * Target: extracted_kernel/ntoskrnl.exe (SHA-256: e8db142a5a54cf5d...)
 * Entry Point: 0x140B593E0 (File offset: 0x00AD43E0 in section PAGELK)
 */

#ifndef _NTOSKRNL_RECONSTRUCTED_H_
#define _NTOSKRNL_RECONSTRUCTED_H_

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/* Bug Check Codes */
#define HAL_INITIALIZATION_FAILED            0x0000005C
#define PHASE1_INITIALIZATION_FAILED         0x00000031
#define MULTIPROCESSOR_CONFIGURATION_NOT_SUPPORTED 0x0000003E
#define MUST_SUCCEED_POOL_EMPTY              0x00000041
#define PAGE_FAULT_IN_NONPAGED_AREA          0x00000050
#define SYSTEM_THREAD_EXCEPTION_NOT_HANDLED  0x0000007E
#define UNEXPECTED_KERNEL_MODE_TRAP          0x0000007F
#define KERNEL_MODE_EXCEPTION_NOT_HANDLED    0x0000008E
#define BAD_POOL_CALLER                      0x000000C2
#define KERNEL_SECURITY_CHECK_FAILURE        0x00000139

/* Status Codes */
#define STATUS_SUCCESS                       0x00000000
#define STATUS_UNSUCCESSFUL                  0xC0000001
#define STATUS_ACCESS_DENIED                 0xC0000022
#define STATUS_BUFFER_TOO_SMALL              0xC0000023
#define STATUS_INSUFFICIENT_RESOURCES        0xC000009A
#define STATUS_INVALID_PARAMETER             0xC000000D
#define STATUS_NOT_SUPPORTED                 0xC00000BB
#define STATUS_TIMEOUT                       0x00000102
#define STATUS_ACCESS_VIOLATION              0xC0000005
#define STATUS_DATATYPE_MISALIGNMENT         0x80000002
#define STATUS_BREAKPOINT                    0x80000003
#define STATUS_SINGLE_STEP                   0x80000004
#define STATUS_INTEGER_DIVIDE_BY_ZERO        0xC0000094
#define STATUS_ILLEGAL_INSTRUCTION           0xC000001D
#define STATUS_PRIVILEGED_INSTRUCTION        0xC0000096
#define STATUS_IN_PAGE_ERROR                 0xC0000006
#define STATUS_STACK_OVERFLOW                0xC00000FD

/* Object Types & Dispatcher Objects */
#define EventNotificationObject              0
#define EventSynchronizationObject           1
#define MutantObject                         2
#define ProcessObject                        3
#define QueueObject                          4
#define SemaphoreObject                      5
#define ThreadObject                         6
#define NotificationTimer                    8
#define SynchronizationTimer                 9
#define DpcObject                            0x13
#define NotificationEvent                    0
#define SynchronizationEvent                 1

/* Wait Types & Limits */
#define WaitAll                              0
#define WaitAny                              1
#define MAXIMUM_WAIT_OBJECTS                 64
#define THREAD_WAIT_OBJECTS                  3

/* IO Types */
#define IO_TYPE_DEVICE                       0x0003
#define IO_TYPE_DRIVER                       0x0004

/* MDL Flags */
#define MDL_MAPPED_TO_SYSTEM_VA              0x0001
#define MDL_PAGES_LOCKED                     0x0002
#define MDL_SOURCE_IS_NONPAGED_POOL          0x0004
#define MDL_ALLOCATED_FIXED_SIZE             0x0008
#define MDL_PARTIAL                          0x0010
#define MDL_PARTIAL_HAS_BEEN_MAPPED          0x0020
#define MDL_IO_PAGE_READ                     0x0040
#define MDL_WRITE_OPERATION                  0x0080
#define MDL_PARENT_MAPPED_SYSTEM_VA          0x0100
#define MDL_FREE_EXTRA_PTES                  0x0200
#define MDL_DESCRIBES_AWE                    0x0400
#define MDL_IO_SPACE                         0x0800
#define MDL_NETWORK_HEADER                   0x1000
#define MDL_MAPPING_CAN_FAIL                 0x2000
#define MDL_ALLOCATED_MUST_SUCCEED           0x4000
#define MDL_INTERNAL                         0x8000

/* Page Size Constants */
#define PAGE_SIZE                            4096
#define PAGE_SHIFT                           12
#define PAGE_MASK                            0xFFF

/* Page Protection Flags */
#define PAGE_NOACCESS                        0x01
#define PAGE_READONLY                        0x02
#define PAGE_READWRITE                       0x04
#define PAGE_WRITECOPY                       0x08
#define PAGE_EXECUTE                         0x10
#define PAGE_EXECUTE_READ                    0x20
#define PAGE_EXECUTE_READWRITE               0x40
#define PAGE_EXECUTE_WRITECOPY               0x80
#define PAGE_GUARD                           0x100
#define PAGE_NOCACHE                         0x200
#define PAGE_WRITECOMBINE                    0x400

/* Access Modes & Lock Operations */
typedef enum _LOCK_OPERATION {
    IoReadAccess,
    IoWriteAccess,
    IoModifyAccess
} LOCK_OPERATION;

/* Access Rights */
typedef uint32_t ACCESS_MASK;
#define DELETE                           0x00010000L
#define READ_CONTROL                     0x00020000L
#define WRITE_DAC                        0x00040000L
#define WRITE_OWNER                      0x00080000L
#define SYNCHRONIZE                      0x00100000L
#define STANDARD_RIGHTS_REQUIRED         0x000F0000L
#define STANDARD_RIGHTS_ALL              0x001F0000L
#define SPECIFIC_RIGHTS_ALL              0x0000FFFFL
#define ACCESS_SYSTEM_SECURITY           0x01000000L
#define MAXIMUM_ALLOWED                  0x02000000L
#define GENERIC_READ                     0x80000000L
#define GENERIC_WRITE                    0x40000000L
#define GENERIC_EXECUTE                  0x20000000L
#define GENERIC_ALL                      0x10000000L

/* ACE Types */
#define ACCESS_ALLOWED_ACE_TYPE          0x00
#define ACCESS_DENIED_ACE_TYPE           0x01

/* x86-64 Segment Selectors (RPL 3 for user, RPL 0 for kernel) */
#define KGDT64_NULL                          0x0000
#define KGDT64_R0_CODE                       0x0010
#define KGDT64_R0_DATA                       0x0018
#define KGDT64_R3_CMTEB                      0x002B  /* Compatibility Mode TEB (0x28 | 3) */
#define KGDT64_R3_DATA                       0x002B  /* User Data (0x28 | 3) */
#define KGDT64_R3_CODE                       0x0033  /* User 64-bit Code (0x30 | 3) */
#define KGDT64_SYS_TSS                       0x0040  /* Task State Segment (16 bytes) */
#define KGDT64_R3_FS                         0x0053  /* User FS TEB (0x50 | 3) */

/* Model-Specific Registers (MSRs) */
#define MSR_STAR                             0xC0000081
#define MSR_LSTAR                            0xC0000082  /* Long Mode Target Address (KiSystemCall64) */
#define MSR_CSTAR                            0xC0000083
#define MSR_FMASK                            0xC0000084
#define MSR_GS_BASE                          0xC0000101
#define MSR_KERNEL_GS_BASE                   0xC0000102
#define MSR_IA32_S_CET                       0x000006A2  /* Supervisor CET / Shadow Stack */
#define MSR_IA32_INTERRUPT_SSP_TABLE_ADDR    0x000006A8

/* CET Control Flags */
#define CET_SHADOW_STACK_ENABLE              0x00000001
#define CET_WR_SHSTK_ENABLE                  0x00000002

/* Standard IEEE-754 MXCSR exception mask */
#define INITIAL_MXCSR_VALUE                  0x00001F80

/* IRQL Levels on x86-64 (represented via CR8 / TPR) */
#define PASSIVE_LEVEL                        0
#define APC_LEVEL                            1
#define DISPATCH_LEVEL                       2
#define HIGH_LEVEL                           15

/* Basic Windows Kernel Types */
typedef uint32_t NTSTATUS;
typedef uint8_t  KIRQL;
typedef uint8_t  KPROCESSOR_MODE;
typedef void*    PVOID;
typedef void*    HANDLE;
typedef HANDLE*  PHANDLE;
typedef uint32_t ULONG;
typedef int32_t  LONG, *PLONG;
typedef uint64_t ULONG_PTR;
typedef uint64_t SIZE_T;
typedef SIZE_T*  PSIZE_T;
typedef uint64_t PFN_NUMBER, *PPFN_NUMBER;
typedef uint8_t  BOOLEAN;
typedef uint32_t DEVICE_TYPE;
typedef uint64_t KSPIN_LOCK, *PKSPIN_LOCK;

typedef union _LARGE_INTEGER {
    struct {
        uint32_t LowPart;
        int32_t HighPart;
    };
    struct {
        uint32_t LowPart;
        int32_t HighPart;
    } u;
    int64_t QuadPart;
} LARGE_INTEGER, *PLARGE_INTEGER;

typedef struct _LIST_ENTRY {
    struct _LIST_ENTRY *Flink;
    struct _LIST_ENTRY *Blink;
} LIST_ENTRY, *PLIST_ENTRY;

typedef struct _UNICODE_STRING {
    uint16_t Length;
    uint16_t MaximumLength;
    wchar_t *Buffer;
} UNICODE_STRING, *PUNICODE_STRING;

typedef struct _CLIENT_ID {
    HANDLE UniqueProcess;
    HANDLE UniqueThread;
} CLIENT_ID, *PCLIENT_ID;

typedef struct _GENERIC_MAPPING {
    ACCESS_MASK GenericRead;
    ACCESS_MASK GenericWrite;
    ACCESS_MASK GenericExecute;
    ACCESS_MASK GenericAll;
} GENERIC_MAPPING, *PGENERIC_MAPPING;

typedef struct _LUID {
    uint32_t LowPart;
    int32_t HighPart;
} LUID, *PLUID;

typedef struct _LUID_AND_ATTRIBUTES {
    LUID Luid;
    uint32_t Attributes;
} LUID_AND_ATTRIBUTES, *PLUID_AND_ATTRIBUTES;

typedef struct _PRIVILEGE_SET {
    uint32_t PrivilegeCount;
    uint32_t Control;
    LUID_AND_ATTRIBUTES Privilege[1];
} PRIVILEGE_SET, *PPRIVILEGE_SET;

typedef struct _SID_IDENTIFIER_AUTHORITY {
    uint8_t Value[6];
} SID_IDENTIFIER_AUTHORITY, *PSID_IDENTIFIER_AUTHORITY;

typedef struct _SID {
    uint8_t Revision;
    uint8_t SubAuthorityCount;
    SID_IDENTIFIER_AUTHORITY IdentifierAuthority;
    uint32_t SubAuthority[1];
} SID, *PSID, *PISID;

typedef struct _ACL {
    uint8_t AclRevision;
    uint8_t Sbz1;
    uint16_t AclSize;
    uint16_t AceCount;
    uint16_t Sbz2;
} ACL, *PACL;

typedef struct _ACE_HEADER {
    uint8_t AceType;
    uint8_t AceFlags;
    uint16_t AceSize;
} ACE_HEADER, *PACE_HEADER;

typedef struct _ACCESS_ALLOWED_ACE {
    ACE_HEADER Header;
    ACCESS_MASK Mask;
    uint32_t SidStart;
} ACCESS_ALLOWED_ACE, *PACCESS_ALLOWED_ACE;

typedef struct _ACCESS_DENIED_ACE {
    ACE_HEADER Header;
    ACCESS_MASK Mask;
    uint32_t SidStart;
} ACCESS_DENIED_ACE, *PACCESS_DENIED_ACE;

typedef struct _SECURITY_DESCRIPTOR {
    uint8_t Revision;
    uint8_t Sbz1;
    uint16_t Control;
    PSID Owner;
    PSID Group;
    PACL Sacl;
    PACL Dacl;
} SECURITY_DESCRIPTOR, *PSECURITY_DESCRIPTOR;

typedef struct _SECURITY_SUBJECT_CONTEXT {
    PVOID ClientToken;
    uint32_t ImpersonationLevel;
    PVOID PrimaryToken;
    PVOID ProcessAuditId;
} SECURITY_SUBJECT_CONTEXT, *PSECURITY_SUBJECT_CONTEXT;

typedef struct _OBJECT_ATTRIBUTES {
    ULONG Length;
    HANDLE RootDirectory;
    PUNICODE_STRING ObjectName;
    ULONG Attributes;
    PVOID SecurityDescriptor;
    PVOID SecurityQualityOfService;
} OBJECT_ATTRIBUTES, *POBJECT_ATTRIBUTES;

/* Memory Descriptor List (MDL) */
typedef struct _MDL {
    struct _MDL *Next;
    int16_t Size;
    int16_t MdlFlags;
    struct _EPROCESS *Process;
    PVOID MappedSystemVa;
    PVOID StartVa;
    ULONG ByteCount;
    ULONG ByteOffset;
} MDL, *PMDL;

/* Memory Manager Pool Types */
typedef enum _POOL_TYPE {
    NonPagedPool,
    NonPagedPoolExecute = NonPagedPool,
    PagedPool,
    NonPagedPoolMustSucceed = NonPagedPool + 2,
    DontUseThisType,
    NonPagedPoolCacheAligned = NonPagedPool + 4,
    PagedPoolCacheAligned,
    NonPagedPoolCacheAlignedMustS = NonPagedPool + 6,
    MaxPoolType,
    NonPagedPoolSession = 32,
    PagedPoolSession = 33,
    NonPagedPoolMustSucceedSession = 34,
    DontUseThisTypeSession = 35,
    NonPagedPoolCacheAlignedSession = 36,
    PagedPoolCacheAlignedSession = 37,
    NonPagedPoolCacheAlignedMustSSession = 38,
    NonPagedPoolNx = 512,
    NonPagedPoolNxCacheAligned = 516,
    NonPagedPoolSessionNx = 544
} POOL_TYPE;

/* Memory Manager Pool Header */
typedef struct _POOL_HEADER {
    union {
        struct {
            uint16_t PreviousSize : 8;
            uint16_t PoolIndex : 8;
            uint16_t BlockSize : 8;
            uint16_t PoolType : 8;
        };
        uint32_t Ulong1;
    };
    uint32_t PoolTag;
    union {
        struct _EPROCESS *ProcessBilled;
        struct {
            uint16_t AllocatorBackTraceIndex;
            uint16_t PoolTagHash;
        };
    };
} POOL_HEADER, *PPOOL_HEADER;

/* x86-64 Hardware Page Table Entry (MMPTE) */
typedef struct _MMPTE_HARDWARE {
    uint64_t Valid : 1;
    uint64_t Write : 1;
    uint64_t Owner : 1;
    uint64_t WriteThrough : 1;
    uint64_t CacheDisable : 1;
    uint64_t Accessed : 1;
    uint64_t Dirty : 1;
    uint64_t LargePage : 1;
    uint64_t Global : 1;
    uint64_t CopyOnWrite : 1;
    uint64_t Prototype : 1;
    uint64_t WriteCombined : 1;
    uint64_t PageFrameNumber : 36;
    uint64_t ReservedForSoftware : 4;
    uint64_t SoftwareWsIndex : 11;
    uint64_t NoExecute : 1;
} MMPTE_HARDWARE, *PMMPTE_HARDWARE;

typedef union _MMPTE {
    uint64_t Long;
    MMPTE_HARDWARE Hard;
} MMPTE, *PMMPTE;

/* Page Frame Number Database Entry (MMPFN) */
typedef enum _MMLISTS {
    ZeroedPageList = 0,
    FreePageList = 1,
    StandbyPageList = 2,
    ModifiedPageList = 3,
    ModifiedNoWritePageList = 4,
    BadPageList = 5,
    ActiveAndValid = 6,
    TransitionPage = 7
} MMLISTS;

typedef struct _MMPFN {
    union {
        PFN_NUMBER Flink;
        PVOID NextBlink;
        struct {
            uint16_t ReferenceCount;
            uint16_t ReadInProgress : 1;
            uint16_t WriteInProgress : 1;
            uint16_t Modified : 1;
            uint16_t PageLocation : 3;
            uint16_t Priority : 3;
            uint16_t CacheAttribute : 2;
        };
    };
    PMMPTE PteAddress;
    PFN_NUMBER Blink;
    union {
        uint64_t Unused;
        struct {
            uint32_t ShareCount;
            uint32_t Flags;
        };
    };
} MMPFN, *PMMPFN;

/* Virtual Address Descriptor (MMVAD) */
typedef struct _MMVAD_SHORT {
    struct _MMVAD_SHORT *LeftChild;
    struct _MMVAD_SHORT *RightChild;
    struct _MMVAD_SHORT *Parent;
    ULONG_PTR StartingVpn;
    ULONG_PTR EndingVpn;
    uint32_t Flags;
} MMVAD_SHORT, *PMMVAD_SHORT;

typedef struct _MMVAD {
    MMVAD_SHORT Core;
    union {
        uint32_t LongFlags2;
        struct {
            uint32_t FileOffset : 24;
            uint32_t Large : 1;
            uint32_t TrimBehind : 1;
            uint32_t Inherit : 1;
            uint32_t CopyOnWrite : 1;
            uint32_t NoChange : 1;
            uint32_t VadType : 3;
        } VadFlags2;
    } u2;
    PVOID Subsection;
    PMMPTE FirstPrototypePte;
    PMMPTE LastContiguousPte;
    LIST_ENTRY ViewLinks;
    PVOID VadsProcess;
} MMVAD, *PMMVAD;

/* Section Object */
typedef struct _SECTION_OBJECT {
    PVOID StartingVa;
    PVOID EndingVa;
    PVOID LeftChild;
    PVOID RightChild;
    PVOID Segment;
    ULONG_PTR SizeOfSection;
    union {
        ULONG LongFlags;
        struct {
            ULONG BeingDeleted : 1;
            ULONG BeingCreated : 1;
            ULONG BeingPurged : 1;
            ULONG NoModifiedWriting : 1;
            ULONG FailAllIo : 1;
            ULONG Image : 1;
            ULONG Based : 1;
            ULONG File : 1;
            ULONG Networked : 1;
            ULONG NoCache : 1;
            ULONG PhysicalMemory : 1;
            ULONG CopyOnWrite : 1;
            ULONG Reserve : 1;
            ULONG Commit : 1;
        } Flags;
    } u;
} SECTION_OBJECT, *PSECTION_OBJECT;

/* Executive Fast Mutex */
typedef struct _FAST_MUTEX {
    int32_t Count;
    struct _KTHREAD *Owner;
    uint32_t Contention;
    struct _DISPATCHER_HEADER *Event;
    ULONG OldIrql;
} FAST_MUTEX, *PFAST_MUTEX;

/* Executive Push Lock */
typedef struct _EX_PUSH_LOCK {
    union {
        struct {
            ULONG_PTR Locked : 1;
            ULONG_PTR Waiting : 1;
            ULONG_PTR Waking : 1;
            ULONG_PTR MultipleShared : 1;
            ULONG_PTR Shared : sizeof(ULONG_PTR) * 8 - 4;
        };
        ULONG_PTR Value;
        PVOID Ptr;
    };
} EX_PUSH_LOCK, *PEX_PUSH_LOCK;

/* Dispatcher Header (Common to KEVENT, KSEMAPHORE, KTIMER, etc.) */
typedef struct _DISPATCHER_HEADER {
    union {
        struct {
            uint8_t Type;
            uint8_t Signalling;
            uint8_t Size;
            uint8_t Reserved1;
        };
        uint32_t Lock;
    };
    int32_t SignalState;
    LIST_ENTRY WaitListHead;
} DISPATCHER_HEADER;

/* Wait Block used for Thread Waiting on Dispatcher Objects */
struct _KTHREAD;
typedef struct _KWAIT_BLOCK {
    LIST_ENTRY WaitListEntry;
    struct _KTHREAD *Thread;
    PVOID Object;
    struct _KWAIT_BLOCK *NextWaitBlock;
    uint16_t WaitKey;
    uint8_t WaitType;
    uint8_t BlockState;
} KWAIT_BLOCK, *PKWAIT_BLOCK;

/* Dispatcher Event Object */
typedef struct _KEVENT {
    DISPATCHER_HEADER Header;
} KEVENT, *PKEVENT, *PRKEVENT;

/* Dispatcher Semaphore Object */
typedef struct _KSEMAPHORE {
    DISPATCHER_HEADER Header;
    int32_t Limit;
} KSEMAPHORE, *PKSEMAPHORE, *PRKSEMAPHORE;

/* Dispatcher Mutex / Mutant Object */
typedef struct _KMUTANT {
    DISPATCHER_HEADER Header;
    LIST_ENTRY MutantListEntry;
    struct _KTHREAD *OwnerThread;
    union {
        uint8_t MutantFlags;
        struct {
            uint8_t Abandoned : 1;
            uint8_t ApcDisable : 1;
        };
    };
    uint8_t ApcControl;
} KMUTANT, *PKMUTANT, *PRKMUTANT, KMUTEX, *PKMUTEX, *PRKMUTEX;

/* Timer Object */
typedef struct _KTIMER {
    DISPATCHER_HEADER Header;
    uint64_t DueTime;
    LIST_ENTRY TimerListEntry;
    struct _KDPC *Dpc;
    uint16_t Processor;
    uint16_t TimerType;
    uint32_t Period;
} KTIMER, *PKTIMER;

/* Deferred Procedure Call (KDPC) */
struct _KDPC;
typedef void (*PKDEFERRED_ROUTINE)(
    struct _KDPC *Dpc,
    PVOID DeferredContext,
    PVOID SystemArgument1,
    PVOID SystemArgument2
);

typedef struct _KDPC {
    union {
        uint32_t TargetInfoAsUlong;
        struct {
            uint8_t Type;
            uint8_t Importance;
            uint16_t Number;
        };
    };
    LIST_ENTRY DpcListEntry;
    PKDEFERRED_ROUTINE DeferredRoutine;
    PVOID DeferredContext;
    PVOID DpcData;
} KDPC, *PKDPC;

/* Asynchronous Procedure Call (KAPC) */
typedef enum _KAPC_ENVIRONMENT {
    OriginalApcEnvironment,
    AttachedApcEnvironment,
    CurrentApcEnvironment,
    InitApcEnvironment
} KAPC_ENVIRONMENT;

struct _KAPC;
typedef void (*PKNORMAL_ROUTINE)(
    PVOID NormalContext,
    PVOID SystemArgument1,
    PVOID SystemArgument2
);

typedef void (*PKKERNEL_ROUTINE)(
    struct _KAPC *Apc,
    PKNORMAL_ROUTINE *NormalRoutine,
    PVOID *NormalContext,
    PVOID *SystemArgument1,
    PVOID *SystemArgument2
);

typedef void (*PKRUNDOWN_ROUTINE)(
    struct _KAPC *Apc
);

typedef struct _KAPC {
    uint8_t Type;
    uint8_t SpareByte0;
    uint8_t Size;
    uint8_t SpareByte1;
    uint32_t SpareLong0;
    struct _KTHREAD *Thread;
    LIST_ENTRY ApcListEntry;
    PKKERNEL_ROUTINE KernelRoutine;
    PKRUNDOWN_ROUTINE RundownRoutine;
    PKNORMAL_ROUTINE NormalRoutine;
    PVOID NormalContext;
    PVOID SystemArgument1;
    PVOID SystemArgument2;
    char ApcStateIndex;
    char ApcMode;
    BOOLEAN Inserted;
} KAPC, *PKAPC, *PRKAPC;

/* Kernel APC State */
typedef struct _KAPC_STATE {
    LIST_ENTRY ApcListHead[2];
    struct _KPROCESS *Process;
    union {
        uint8_t InProgressFlags;
        struct {
            uint8_t KernelApcInProgress : 1;
            uint8_t SpecialApcInProgress : 1;
        };
    };
    uint8_t KernelApcPending;
    uint8_t UserApcPending;
} KAPC_STATE, *PKAPC_STATE;

/* Kernel Process (KPROCESS) */
typedef struct _KPROCESS {
    DISPATCHER_HEADER Header;
    LIST_ENTRY ProfileListHead;
    ULONG_PTR DirectoryTableBase;
    LIST_ENTRY ThreadListHead;
    uint32_t ProcessQuantum;
    int8_t BasePriority;
    uint8_t QuantumReset;
    uint8_t State;
    uint8_t ThreadSeed[4];
    uint64_t ActiveProcessors;
} KPROCESS, *PKPROCESS;

/* Kernel Thread (KTHREAD) */
typedef struct _KTHREAD {
    DISPATCHER_HEADER Header;
    PVOID SListFaultAddress;
    uint64_t QuantumTarget;
    PVOID InitialStack;
    PVOID StackLimit;
    PVOID KernelStack;
    uint64_t ThreadLock;
    PKWAIT_BLOCK WaitBlockList;
    KWAIT_BLOCK WaitBlock[4];
    KAPC_STATE ApcState;
    int8_t Priority;
    int8_t BasePriority;
    uint8_t State;
    int16_t KernelApcDisable;
    int16_t SpecialApcDisable;
    uint32_t ContextSwitches;
    PVOID Teb;
    PKPROCESS Process;
} KTHREAD, *PKTHREAD;

/* Executive Process (EPROCESS) */
typedef struct _EPROCESS {
    KPROCESS Pcb;
    EX_PUSH_LOCK ProcessLock;
    HANDLE UniqueProcessId;
    LIST_ENTRY ActiveProcessLinks;
    EX_PUSH_LOCK RundownProtect;
    PVOID ObjectTable;
    PVOID Token;
    uint64_t CreateTime;
    uint64_t ExitTime;
} EPROCESS, *PEPROCESS;

/* Executive Thread (ETHREAD) */
typedef struct _ETHREAD {
    KTHREAD Tcb;
    CLIENT_ID Cid;
    LIST_ENTRY ThreadListEntry;
    EX_PUSH_LOCK ThreadRundownProtect;
    PVOID StartAddress;
    PEPROCESS Process;
} ETHREAD, *PETHREAD;

/* Descriptor Register for SGDT / SIDT */
#pragma pack(push, 2)
typedef struct _KDESCRIPTOR_TABLE_REGISTER {
    uint16_t Limit;
    uint64_t Base;
} KDESCRIPTOR_TABLE_REGISTER, *PKDESCRIPTOR_TABLE_REGISTER;
#pragma pack(pop)

/* x86-64 Interrupt Descriptor Table Entry (IDT) */
#pragma pack(push, 1)
typedef struct _KIDTENTRY64 {
    uint16_t OffsetLow;
    uint16_t Selector;
    uint8_t  IstIndex;
    uint8_t  TypeAttr;
    uint16_t OffsetMiddle;
    uint32_t OffsetHigh;
    uint32_t Reserved;
} KIDTENTRY64, *PKIDTENTRY64;
#pragma pack(pop)

/* Interrupt Service Routine & Interrupt Object */
struct _KINTERRUPT;
typedef BOOLEAN (*PKSERVICE_ROUTINE)(
    struct _KINTERRUPT *Interrupt,
    PVOID ServiceContext
);

typedef struct _KINTERRUPT {
    int16_t Type;
    int16_t Size;
    LIST_ENTRY InterruptListEntry;
    PKSERVICE_ROUTINE ServiceRoutine;
    PVOID ServiceContext;
    KSPIN_LOCK SpinLock;
    uint32_t TickCount;
    PKSPIN_LOCK ActualLock;
    PVOID DispatchAddress;
    ULONG Vector;
    KIRQL Irql;
    KIRQL SynchronizeIrql;
    BOOLEAN FloatingSave;
    BOOLEAN Connected;
    uint32_t Number;
    BOOLEAN ShareVector;
    uint32_t Internal;
} KINTERRUPT, *PKINTERRUPT;

/* IoConnectInterruptEx Parameters */
typedef struct _IO_CONNECT_INTERRUPT_PARAMETERS {
    ULONG Version;
    union {
        struct {
            PKINTERRUPT *InterruptObject;
            PKSERVICE_ROUTINE ServiceRoutine;
            PVOID ServiceContext;
            PKSPIN_LOCK SpinLock;
            ULONG Vector;
            KIRQL Irql;
            KIRQL SynchronizeIrql;
            BOOLEAN ShareVector;
            BOOLEAN FloatingSave;
            ULONG ProcessorEnableMask;
        } FullySpecified;
    };
} IO_CONNECT_INTERRUPT_PARAMETERS, *PIO_CONNECT_INTERRUPT_PARAMETERS;

/* x86-64 Processor Special Registers (saved at KiSystemStartup) */
typedef struct _KSPECIAL_REGISTERS {
    uint64_t Cr0;
    uint64_t Cr2;
    uint64_t Cr3;
    uint64_t Cr4;
    uint64_t KernelDr0;
    uint64_t KernelDr1;
    uint64_t KernelDr2;
    uint64_t KernelDr3;
    uint64_t KernelDr6;
    uint64_t KernelDr7;
    KDESCRIPTOR_TABLE_REGISTER Gdtr;
    KDESCRIPTOR_TABLE_REGISTER Idtr;
    uint16_t Tr;
    uint16_t Ldtr;
    uint32_t MxCsr;
    uint64_t MsrGsBase;
    uint64_t MsrKernelGsBase;
    uint64_t MsrCet;
} KSPECIAL_REGISTERS, *PKSPECIAL_REGISTERS;

/* KTRAP_FRAME - Hardware and software register frame during traps/syscalls */
typedef struct _KTRAP_FRAME {
    uint64_t P1Home;
    uint64_t P2Home;
    uint64_t P3Home;
    uint64_t P4Home;
    uint64_t P5;
    char PreviousMode;
    uint8_t PreviousIrql;
    uint8_t FaultIndicator;
    uint8_t ExceptionActive;
    uint32_t MxCsr;
    uint64_t Rax;
    uint64_t Rcx;
    uint64_t Rdx;
    uint64_t R8;
    uint64_t R9;
    uint64_t R10;
    uint64_t R11;
    uint64_t TrapFrame;
    uint64_t Rbp;
    uint64_t Rip;
    uint16_t SegCs;
    uint8_t Fill0;
    uint8_t Logging;
    uint16_t Fill1[2];
    uint32_t EFlags;
    uint32_t Fill2;
    uint64_t Rsp;
    uint16_t SegSs;
    uint16_t Fill3[3];
} KTRAP_FRAME, *PKTRAP_FRAME;

/* Exception Record */
#define EXCEPTION_MAXIMUM_PARAMETERS 15
typedef struct _EXCEPTION_RECORD {
    NTSTATUS ExceptionCode;
    ULONG ExceptionFlags;
    struct _EXCEPTION_RECORD *ExceptionRecord;
    PVOID ExceptionAddress;
    ULONG NumberParameters;
    ULONG_PTR ExceptionInformation[EXCEPTION_MAXIMUM_PARAMETERS];
} EXCEPTION_RECORD, *PEXCEPTION_RECORD;

/* Service Descriptor Table */
typedef struct _KSERVICE_TABLE_DESCRIPTOR {
    uint32_t *ServiceTableBase;
    uint32_t *ServiceCounterTableBase;
    uint64_t NumberOfServices;
    uint8_t *ParamTableBase;
} KSERVICE_TABLE_DESCRIPTOR, *PKSERVICE_TABLE_DESCRIPTOR;

/* Object Manager Definitions */
typedef struct _OBJECT_TYPE_INITIALIZER {
    uint16_t Length;
    uint16_t ObjectTypeFlags;
    uint32_t CaseInsensitive:1;
    uint32_t UnnamedObjectsOnly:1;
    uint32_t UseDefaultObject:1;
    uint32_t SecurityRequired:1;
    uint32_t MaintainHandleCount:1;
    uint32_t MaintainTypeList:1;
    uint32_t SupportsObjectCallbacks:1;
    uint32_t CacheAligned:1;
    ULONG TotalNumberOfObjects;
    ULONG TotalNumberOfHandles;
    ULONG HighWaterNumberOfObjects;
    ULONG HighWaterNumberOfHandles;
    ULONG InvalidAttributes;
    ULONG GenericMapping;
    ULONG ValidAccessMask;
    ULONG RetainAccess;
    uint32_t PoolType;
    ULONG DefaultPagedPoolCharge;
    ULONG DefaultNonPagedPoolCharge;
} OBJECT_TYPE_INITIALIZER, *POBJECT_TYPE_INITIALIZER;

typedef struct _OBJECT_TYPE {
    LIST_ENTRY TypeList;
    UNICODE_STRING Name;
    PVOID DefaultObject;
    uint8_t Index;
    ULONG TotalNumberOfObjects;
    ULONG TotalNumberOfHandles;
    ULONG HighWaterNumberOfObjects;
    ULONG HighWaterNumberOfHandles;
    OBJECT_TYPE_INITIALIZER TypeInfo;
} OBJECT_TYPE, *POBJECT_TYPE;

typedef struct _OBJECT_HEADER {
    int64_t PointerCount;
    union {
        int64_t HandleCount;
        PVOID NextToFree;
    };
    POBJECT_TYPE Type;
    uint8_t NameInfoOffset;
    uint8_t HandleInfoOffset;
    uint8_t QuotaInfoOffset;
    uint8_t Flags;
    PVOID SecurityDescriptor;
    PVOID Body;
} OBJECT_HEADER, *POBJECT_HEADER;

/* I/O Manager Device & Driver Objects */
struct _DRIVER_OBJECT;
typedef struct _DEVICE_OBJECT {
    int16_t Type;
    uint16_t Size;
    int32_t ReferenceCount;
    struct _DRIVER_OBJECT *DriverObject;
    struct _DEVICE_OBJECT *NextDevice;
    struct _DEVICE_OBJECT *AttachedDevice;
    struct _IRP *CurrentIrp;
    uint32_t Flags;
    uint32_t Characteristics;
    PVOID DeviceExtension;
    DEVICE_TYPE DeviceType;
    int8_t StackSize;
} DEVICE_OBJECT, *PDEVICE_OBJECT;

typedef struct _FILE_OBJECT {
    int16_t Type;
    int16_t Size;
    PDEVICE_OBJECT DeviceObject;
    PVOID Vpb;
    PVOID FsContext;
    PVOID FsContext2;
    PVOID SectionObjectPointer;
    PVOID PrivateCacheMap;
    NTSTATUS FinalStatus;
    struct _FILE_OBJECT *RelatedFileObject;
    BOOLEAN LockOperation;
    BOOLEAN DeletePending;
    BOOLEAN ReadAccess;
    BOOLEAN WriteAccess;
    BOOLEAN DeleteAccess;
    BOOLEAN SharedRead;
    BOOLEAN SharedWrite;
    BOOLEAN SharedDelete;
    ULONG Flags;
    UNICODE_STRING FileName;
    LARGE_INTEGER CurrentByteOffset;
    ULONG Waiters;
    ULONG Busy;
    PVOID LastLock;
    KEVENT Lock;
    KEVENT Event;
    PVOID CompletionContext;
    KSPIN_LOCK IrpListLock;
    LIST_ENTRY IrpList;
    PVOID FileObjectExtension;
} FILE_OBJECT, *PFILE_OBJECT;

typedef struct _IO_STATUS_BLOCK {
    union {
        NTSTATUS Status;
        PVOID Pointer;
    };
    ULONG_PTR Information;
} IO_STATUS_BLOCK, *PIO_STATUS_BLOCK;

struct _IO_STACK_LOCATION;
typedef struct _IRP {
    int16_t Type;
    uint16_t Size;
    PMDL MdlAddress;
    uint32_t Flags;
    union {
        struct _IRP *MasterIrp;
        int32_t IrpCount;
        PVOID SystemBuffer;
    } AssociatedIrp;
    LIST_ENTRY ThreadListEntry;
    IO_STATUS_BLOCK IoStatus;
    char RequestorMode;
    BOOLEAN PendingReturned;
    int8_t StackCount;
    int8_t CurrentLocation;
    BOOLEAN Cancel;
    uint8_t CancelIrql;
    PVOID CancelRoutine;
    PVOID UserBuffer;
    union {
        struct {
            PVOID DriverContext[4];
            struct _ETHREAD *Thread;
            char *AuxiliaryBuffer;
            struct {
                LIST_ENTRY ListEntry;
                union {
                    struct _IO_STACK_LOCATION *CurrentStackLocation;
                    ULONG PacketType;
                };
            };
            PVOID OriginalFileObject;
        } Overlay;
    } Tail;
} IRP, *PIRP;

typedef struct _IO_STACK_LOCATION {
    uint8_t MajorFunction;
    uint8_t MinorFunction;
    uint8_t Flags;
    uint8_t Control;
    union {
        struct {
            ULONG OutputBufferLength;
            ULONG InputBufferLength;
            ULONG IoControlCode;
            PVOID Type3InputBuffer;
        } DeviceIoControl;
        struct {
            ULONG Length;
            ULONG Key;
            int64_t ByteOffset;
        } Read;
        struct {
            ULONG Length;
            ULONG Key;
            int64_t ByteOffset;
        } Write;
        PVOID Parameters[4];
    } Parameters;
    PDEVICE_OBJECT DeviceObject;
    PVOID FileObject;
    PVOID CompletionRoutine;
    PVOID Context;
} IO_STACK_LOCATION, *PIO_STACK_LOCATION;

typedef struct _LOOKASIDE_LIST_EX {
    uint32_t Type;
    uint32_t Size;
    uint32_t Tag;
    uint32_t TotalAllocates;
    uint32_t AllocateMisses;
    uint32_t TotalFrees;
    uint32_t FreeMisses;
    POOL_TYPE TypePool;
    uint32_t MaximumDepth;
    uint32_t Depth;
    PVOID Allocate;
    PVOID Free;
    LIST_ENTRY ListHead;
} LOOKASIDE_LIST_EX, *PLOOKASIDE_LIST_EX;

typedef struct _NPAGED_LOOKASIDE_LIST {
    LOOKASIDE_LIST_EX L;
} NPAGED_LOOKASIDE_LIST, *PNPAGED_LOOKASIDE_LIST;

typedef struct _DRIVER_OBJECT {
    int16_t Type;
    int16_t Size;
    PDEVICE_OBJECT DeviceObject;
    uint32_t Flags;
    PVOID DriverStart;
    uint32_t DriverSize;
    PVOID DriverSection;
    PVOID DriverExtension;
    UNICODE_STRING DriverName;
    PVOID DriverInit;
    PVOID DriverStartIo;
    PVOID DriverUnload;
    PVOID MajorFunction[28];
} DRIVER_OBJECT, *PDRIVER_OBJECT;

/* Loader Parameter Extension (Offset 0x88 in LOADER_PARAMETER_BLOCK) */
typedef struct _LOADER_PARAMETER_EXTENSION {
    uint32_t Size;
    uint32_t ProfileAllocations;
    void *MajorVersion;
    uint32_t MinorVersion;
    void *EmulatedConfiguration;
    void *HypervisorSharedData;
    uint32_t ProcessorIndex;                 /* Offset 0x24: 0 = BSP, >0 = AP */
    uint32_t HalReserved[8];
} LOADER_PARAMETER_EXTENSION, *PLOADER_PARAMETER_EXTENSION;

/* Loader Parameter Block (Passed from winload.efi into KiSystemStartup in RCX) */
typedef struct _LOADER_PARAMETER_BLOCK {
    uint64_t OsLoaderMemoryData;             /* 0x00 */
    uint64_t OsLoaderReserved;               /* 0x08 */
    void *LoadOrderListHead;                 /* 0x10 */
    void *MemoryDescriptorListHead;          /* 0x18 */
    void *BootDriverListHead;                /* 0x20 */
    uint64_t KernelStack;                    /* 0x28 */
    uint64_t Prcb;                           /* 0x30 */
    uint64_t Process;                        /* 0x38 */
    uint64_t Thread;                         /* 0x40 */
    uint32_t RegistryLength;                 /* 0x48 */
    void *RegistryBase;                      /* 0x50 */
    void *ConfigurationRoot;                 /* 0x58 */
    char *ArcBootDeviceName;                 /* 0x60 */
    char *ArcHalDeviceName;                  /* 0x68 */
    char *NtBootPathName;                    /* 0x70 */
    char *NtHalPathName;                     /* 0x78 */
    char *LoadOptions;                       /* 0x80 */
    PLOADER_PARAMETER_EXTENSION Extension;   /* 0x88: Extension pointer */
    void *I386;                              /* 0x90 */
    void *FirmwareInformation;               /* 0x98 */
} LOADER_PARAMETER_BLOCK, *PLOADER_PARAMETER_BLOCK;

/* Core Kernel Function Prototypes */
void KiSystemStartup(PLOADER_PARAMETER_BLOCK LoaderBlock);
void KiInitializeProcessorState(PKSPECIAL_REGISTERS SpecialRegisters, PLOADER_PARAMETER_BLOCK LoaderBlock);
void HalInitSystem(uint32_t Phase, PLOADER_PARAMETER_BLOCK LoaderBlock);
void ExpInitializeExecutivePhase(uint32_t Phase, PLOADER_PARAMETER_BLOCK LoaderBlock);
uint32_t Phase1Initialization(PLOADER_PARAMETER_BLOCK LoaderBlock);
void KeBugCheckEx(uint32_t BugCheckCode, uint64_t BugCheckParameter1, uint64_t BugCheckParameter2, uint64_t BugCheckParameter3, uint64_t BugCheckParameter4);

/* System Call Dispatcher & Trap Handlers */
void KiSystemCall64(void);
void KiSystemCall64Shadow(void);
NTSTATUS KiSystemServiceHandler(uint32_t ServiceIndex, uint64_t *Arguments, PKTRAP_FRAME TrapFrame);
void KiDispatchException(
    PEXCEPTION_RECORD ExceptionRecord,
    PKTRAP_FRAME TrapFrame,
    KPROCESSOR_MODE PreviousMode,
    BOOLEAN FirstChance
);
void KiDivideErrorFault(PKTRAP_FRAME TrapFrame);
void KiBreakpointTrap(PKTRAP_FRAME TrapFrame);
void KiInvalidOpcodeFault(PKTRAP_FRAME TrapFrame);
void KiGeneralProtectionFault(PKTRAP_FRAME TrapFrame);
void KiDoubleFaultAbort(PKTRAP_FRAME TrapFrame);

/* Kernel Core Synchronization */
KIRQL KeAcquireSpinLockRaiseToDpc(PKSPIN_LOCK SpinLock);
void KeReleaseSpinLock(PKSPIN_LOCK SpinLock, KIRQL NewIrql);
NTSTATUS KeWaitForSingleObject(PVOID Object, uint32_t WaitReason, KPROCESSOR_MODE WaitMode, BOOLEAN Alertable, int64_t *Timeout);
void ExAcquireFastMutex(PFAST_MUTEX FastMutex);
void ExReleaseFastMutex(PFAST_MUTEX FastMutex);
void ExAcquirePushLockExclusiveEx(PEX_PUSH_LOCK PushLock, ULONG Flags);
void ExReleasePushLockExclusiveEx(PEX_PUSH_LOCK PushLock, ULONG Flags);

/* DPC and Timer */
void KeInitializeDpc(PKDPC Dpc, PKDEFERRED_ROUTINE DeferredRoutine, PVOID DeferredContext);
void KeInitializeTimer(PKTIMER Timer);

/* Memory Manager */
void MmProbeAndLockPages(PMDL MemoryDescriptorList, KPROCESSOR_MODE AccessMode, LOCK_OPERATION Operation);
void MmUnlockPages(PMDL MemoryDescriptorList);
void MmBuildMdlForNonPagedPool(PMDL MemoryDescriptorList);
BOOLEAN MmIsNonPagedSystemAddressValid(PVOID VirtualAddress);
PVOID ExAllocatePoolWithTag(POOL_TYPE PoolType, SIZE_T NumberOfBytes, ULONG Tag);
void ExFreePoolWithTag(PVOID P, ULONG Tag);
NTSTATUS MmAccessFault(ULONG_PTR FaultStatus, PVOID VirtualAddress, KPROCESSOR_MODE PreviousMode, PKTRAP_FRAME TrapInformation);
void KiPageFault(PKTRAP_FRAME TrapFrame);

/* Process and Thread Management */
typedef void (*PKSTART_ROUTINE)(PVOID StartContext);
NTSTATUS PsCreateSystemThread(
    PHANDLE ThreadHandle,
    ACCESS_MASK DesiredAccess,
    POBJECT_ATTRIBUTES ObjectAttributes,
    HANDLE ProcessHandle,
    PCLIENT_ID ClientId,
    PKSTART_ROUTINE StartRoutine,
    PVOID StartContext
);
NTSTATUS PspCreateThread(
    PHANDLE ThreadHandle,
    ACCESS_MASK DesiredAccess,
    POBJECT_ATTRIBUTES ObjectAttributes,
    HANDLE ProcessHandle,
    PEPROCESS ProcessPointer,
    PCLIENT_ID ClientId,
    PKSTART_ROUTINE StartRoutine,
    PVOID StartContext,
    BOOLEAN CreateSuspended
);
NTSTATUS PsTerminateSystemThread(NTSTATUS ExitStatus);
NTSTATUS PsLookupProcessByProcessId(HANDLE ProcessId, PEPROCESS *Process);
NTSTATUS PsLookupThreadByThreadId(HANDLE ThreadId, PETHREAD *Thread);
PEPROCESS PsGetCurrentProcess(void);
PETHREAD PsGetCurrentThread(void);

/* Scheduler & Context Switching */
BOOLEAN KiSwapContext(PKTHREAD OldThread, PKTHREAD NewThread);
PKTHREAD KiSelectNextThread(void);
void KiQuantumEnd(void);
int8_t KeSetPriorityThread(PKTHREAD Thread, int8_t Priority);

/* Configuration Manager (Registry) */
typedef enum _KEY_VALUE_INFORMATION_CLASS {
    KeyValueBasicInformation,
    KeyValueFullInformation,
    KeyValuePartialInformation,
    KeyValueFullInformationAlign64,
    KeyValuePartialInformationAlign64
} KEY_VALUE_INFORMATION_CLASS;

typedef struct _KEY_VALUE_BASIC_INFORMATION {
    ULONG TitleIndex;
    ULONG Type;
    ULONG NameLength;
    wchar_t Name[1];
} KEY_VALUE_BASIC_INFORMATION, *PKEY_VALUE_BASIC_INFORMATION;

typedef struct _KEY_VALUE_PARTIAL_INFORMATION {
    ULONG TitleIndex;
    ULONG Type;
    ULONG DataLength;
    uint8_t Data[1];
} KEY_VALUE_PARTIAL_INFORMATION, *PKEY_VALUE_PARTIAL_INFORMATION;

typedef struct _CM_KEY_BODY {
    uint32_t Type;
    PVOID KeyControlBlock;
    PVOID NotifyBlock;
    HANDLE ProcessId;
    LIST_ENTRY KeyBodyList;
} CM_KEY_BODY, *PCM_KEY_BODY;

typedef NTSTATUS (*PEX_CALLBACK_FUNCTION)(
    PVOID CallbackContext,
    PVOID Argument1,
    PVOID Argument2
);

NTSTATUS CmRegisterCallbackEx(
    PEX_CALLBACK_FUNCTION Function,
    PUNICODE_STRING Altitude,
    PVOID Driver,
    PVOID Context,
    uint64_t *Cookie,
    PVOID Reserved
);
NTSTATUS NtOpenKey(
    PHANDLE KeyHandle,
    ACCESS_MASK DesiredAccess,
    POBJECT_ATTRIBUTES ObjectAttributes
);
NTSTATUS NtQueryValueKey(
    HANDLE KeyHandle,
    PUNICODE_STRING ValueName,
    KEY_VALUE_INFORMATION_CLASS KeyValueInformationClass,
    PVOID KeyValueInformation,
    ULONG Length,
    ULONG *ResultLength
);
NTSTATUS NtSetValueKey(
    HANDLE KeyHandle,
    PUNICODE_STRING ValueName,
    ULONG TitleIndex,
    ULONG Type,
    PVOID Data,
    ULONG DataSize
);

/* Security Reference Monitor */
BOOLEAN SeAccessCheck(
    PSECURITY_DESCRIPTOR SecurityDescriptor,
    PSECURITY_SUBJECT_CONTEXT SubjectSecurityContext,
    BOOLEAN SubjectContextLocked,
    ACCESS_MASK DesiredAccess,
    ACCESS_MASK PreviouslyGrantedAccess,
    PPRIVILEGE_SET *Privileges,
    PGENERIC_MAPPING GenericMapping,
    KPROCESSOR_MODE AccessMode,
    ACCESS_MASK *GrantedAccess,
    NTSTATUS *AccessStatus
);
BOOLEAN SeSinglePrivilegeCheck(LUID PrivilegeValue, KPROCESSOR_MODE PreviousMode);

/* Object Manager */
NTSTATUS ObCreateObject(
    KPROCESSOR_MODE ProbeMode,
    POBJECT_TYPE ObjectType,
    POBJECT_ATTRIBUTES ObjectAttributes,
    KPROCESSOR_MODE AccessMode,
    PVOID ParseContext,
    ULONG ObjectSize,
    ULONG PagedPoolCharge,
    ULONG NonPagedPoolCharge,
    PVOID *Object
);
NTSTATUS ObReferenceObjectByHandle(
    HANDLE Handle,
    ACCESS_MASK DesiredAccess,
    POBJECT_TYPE ObjectType,
    KPROCESSOR_MODE AccessMode,
    PVOID *Object,
    PVOID HandleInformation
);
NTSTATUS ObReferenceObjectByPointer(
    PVOID Object,
    ACCESS_MASK DesiredAccess,
    POBJECT_TYPE ObjectType,
    KPROCESSOR_MODE AccessMode
);
int64_t ObfDereferenceObject(PVOID Object);

/* I/O Manager */
NTSTATUS IoCreateDevice(
    PDRIVER_OBJECT DriverObject,
    ULONG DeviceExtensionSize,
    PUNICODE_STRING DeviceName,
    DEVICE_TYPE DeviceType,
    ULONG DeviceCharacteristics,
    BOOLEAN Exclusive,
    PDEVICE_OBJECT *DeviceObject
);
PIRP IoAllocateIrp(int8_t StackSize, BOOLEAN ChargeQuota);
void IoFreeIrp(PIRP Irp);
NTSTATUS IofCallDriver(PDEVICE_OBJECT DeviceObject, PIRP Irp);
void IofCompleteRequest(PIRP Irp, char PriorityBoost);

/* Security Reference Monitor Tokens */
void SeCaptureSubjectContext(PSECURITY_SUBJECT_CONTEXT SubjectContext);
void SeReleaseSubjectContext(PSECURITY_SUBJECT_CONTEXT SubjectContext);

/* Executive Subsystems & Lookaside Lists */
void ExInitializeNPagedLookasideList(
    PNPAGED_LOOKASIDE_LIST Lookaside,
    PVOID Allocate,
    PVOID Free,
    ULONG Flags,
    SIZE_T Size,
    ULONG Tag,
    uint16_t Depth
);
void ExDeleteNPagedLookasideList(PNPAGED_LOOKASIDE_LIST Lookaside);
int64_t KeQueryPerformanceCounter(int64_t *PerformanceFrequency);

/* Dispatcher Objects & Synchronization */
void KeInitializeEvent(PRKEVENT Event, uint32_t Type, BOOLEAN State);
LONG KeSetEvent(PRKEVENT Event, int32_t Increment, BOOLEAN Wait);
LONG KeResetEvent(PRKEVENT Event);
void KeClearEvent(PRKEVENT Event);
void KeInitializeSemaphore(PRKSEMAPHORE Semaphore, int32_t Count, int32_t Limit);
LONG KeReleaseSemaphore(PRKSEMAPHORE Semaphore, int32_t Increment, int32_t Adjustment, BOOLEAN Wait);
LONG KeReadStateSemaphore(PRKSEMAPHORE Semaphore);
void KeInitializeMutex(PRKMUTEX Mutex, ULONG Level);
LONG KeReleaseMutex(PRKMUTEX Mutex, BOOLEAN Wait);
LONG KeReadStateMutex(PRKMUTEX Mutex);
NTSTATUS KeWaitForMultipleObjects(
    ULONG Count,
    PVOID Object[],
    uint32_t WaitType,
    uint32_t WaitReason,
    KPROCESSOR_MODE WaitMode,
    BOOLEAN Alertable,
    PLARGE_INTEGER Timeout,
    PKWAIT_BLOCK WaitBlockArray
);

/* Asynchronous Procedure Call (APC) Manager */
void KeInitializeApc(
    PRKAPC Apc,
    PKTHREAD Thread,
    KAPC_ENVIRONMENT Environment,
    PKKERNEL_ROUTINE KernelRoutine,
    PKRUNDOWN_ROUTINE RundownRoutine,
    PKNORMAL_ROUTINE NormalRoutine,
    KPROCESSOR_MODE ApcMode,
    PVOID NormalContext
);
BOOLEAN KeInsertQueueApc(
    PRKAPC Apc,
    PVOID SystemArgument1,
    PVOID SystemArgument2,
    int32_t PriorityBoost
);
void KiDeliverApc(KPROCESSOR_MODE PreviousMode, PKTRAP_FRAME TrapFrame);
BOOLEAN KeTestAlertThread(KPROCESSOR_MODE AlertMode);
BOOLEAN KeAreApcsDisabled(void);

/* Interrupt Management & APIC */
NTSTATUS IoConnectInterruptEx(PIO_CONNECT_INTERRUPT_PARAMETERS Parameters);
void IoDisconnectInterruptEx(PIO_CONNECT_INTERRUPT_PARAMETERS Parameters);
void KiInterruptDispatch(PKINTERRUPT Interrupt, PKTRAP_FRAME TrapFrame);
void KiChainedDispatch(PKINTERRUPT Interrupt, PKTRAP_FRAME TrapFrame);

/* Memory Manager Sections & Virtual Address Descriptors */
NTSTATUS MmCreateSection(
    PVOID *SectionObject,
    ACCESS_MASK DesiredAccess,
    POBJECT_ATTRIBUTES ObjectAttributes,
    PLARGE_INTEGER MaximumSize,
    ULONG SectionPageProtection,
    ULONG AllocationAttributes,
    HANDLE FileHandle,
    PVOID FileObject
);
NTSTATUS MmMapViewOfSection(
    PVOID SectionToMap,
    PEPROCESS Process,
    PVOID *BaseAddress,
    ULONG_PTR ZeroBits,
    SIZE_T CommitSize,
    PLARGE_INTEGER SectionOffset,
    PSIZE_T ViewSize,
    uint32_t InheritDisposition,
    ULONG AllocationType,
    ULONG Win32Protect
);
NTSTATUS MmUnmapViewOfSection(PEPROCESS Process, PVOID BaseAddress);

/* Work Queue Subsystem */
typedef enum _WORK_QUEUE_TYPE {
    CriticalWorkQueue,
    DelayedWorkQueue,
    HyperCriticalWorkQueue,
    NormalWorkQueue,
    MaximumWorkQueue
} WORK_QUEUE_TYPE;

typedef void (*PWORKER_THREAD_ROUTINE)(PVOID Parameter);

typedef struct _WORK_QUEUE_ITEM {
    LIST_ENTRY List;
    PWORKER_THREAD_ROUTINE WorkerRoutine;
    PVOID Parameter;
} WORK_QUEUE_ITEM, *PWORK_QUEUE_ITEM;

void ExQueueWorkItem(PWORK_QUEUE_ITEM WorkItem, WORK_QUEUE_TYPE QueueType);

/* Push Lock Operations */
void ExAcquirePushLockExclusiveEx(PEX_PUSH_LOCK PushLock, ULONG Flags);
void ExAcquirePushLockSharedEx(PEX_PUSH_LOCK PushLock, ULONG Flags);
void ExReleasePushLockExclusiveEx(PEX_PUSH_LOCK PushLock, ULONG Flags);
void ExReleasePushLockSharedEx(PEX_PUSH_LOCK PushLock, ULONG Flags);

/* Timer Subsystem */
void KeInitializeTimer(PKTIMER Timer);
void KeInitializeTimerEx(PKTIMER Timer, uint32_t Type);
BOOLEAN KeSetTimer(PKTIMER Timer, LARGE_INTEGER DueTime, struct _KDPC *Dpc);
BOOLEAN KeSetTimerEx(PKTIMER Timer, LARGE_INTEGER DueTime, LONG Period, struct _KDPC *Dpc);
BOOLEAN KeCancelTimer(PKTIMER Timer);
NTSTATUS KeDelayExecutionThread(KPROCESSOR_MODE WaitMode, BOOLEAN Alertable, PLARGE_INTEGER Interval);

/* I/O Request Completion & Cancellation */
typedef void (*PDRIVER_CANCEL)(PDEVICE_OBJECT DeviceObject, PIRP Irp);
void IoCompleteRequest(PIRP Irp, char PriorityBoost);
BOOLEAN IoCancelIrp(PIRP Irp);
PDRIVER_CANCEL IoSetCancelRoutine(PIRP Irp, PDRIVER_CANCEL CancelRoutine);
PDEVICE_OBJECT IoAttachDeviceToDeviceStack(PDEVICE_OBJECT SourceDevice, PDEVICE_OBJECT TargetDevice);

/* Page Table Base Addresses & Translation */
#define PTE_BASE    0xFFFFF68000000000ULL
#define PDE_BASE    0xFFFFF6FB40000000ULL
#define PPE_BASE    0xFFFFF6FB7DA00000ULL
#define PXE_BASE    0xFFFFF6FB7DBED000ULL

PMMPTE MiGetPteAddress(PVOID VirtualAddress);
PMMPTE MiGetPdeAddress(PVOID VirtualAddress);

/* Process Creation / Termination / Job Subsystems */
typedef const UNICODE_STRING *PCUNICODE_STRING;

typedef struct _PS_CREATE_NOTIFY_INFO {
    SIZE_T Size;
    union {
        ULONG Flags;
        struct {
            ULONG FileOpenNameAvailable : 1;
            ULONG IsSubsystemProcess : 1;
            ULONG Reserved : 30;
        };
    };
    HANDLE ParentProcessId;
    CLIENT_ID CreatingThreadId;
    struct _FILE_OBJECT *FileObject;
    PCUNICODE_STRING ImageFileName;
    PCUNICODE_STRING CommandLine;
    NTSTATUS CreationStatus;
} PS_CREATE_NOTIFY_INFO, *PPS_CREATE_NOTIFY_INFO;

typedef void (*PCREATE_PROCESS_NOTIFY_ROUTINE)(HANDLE ParentId, HANDLE ProcessId, BOOLEAN Create);
typedef void (*PCREATE_PROCESS_NOTIFY_ROUTINE_EX)(PEPROCESS Process, HANDLE ProcessId, PPS_CREATE_NOTIFY_INFO CreateInfo);
typedef void (*PCREATE_THREAD_NOTIFY_ROUTINE)(HANDLE ProcessId, HANDLE ThreadId, BOOLEAN Create);

typedef struct _IMAGE_INFO {
    union {
        ULONG Properties;
        struct {
            ULONG ImageAddressingMode : 8;
            ULONG SystemModeImage : 1;
            ULONG ImageMappedToAllPids : 1;
            ULONG ExtendedInfoPresent : 1;
            ULONG MachineTypeMismatch : 1;
            ULONG ImageSignatureLevel : 4;
            ULONG ImageSignatureType : 3;
            ULONG ImagePartialMap : 1;
            ULONG Reserved : 13;
        };
    };
    PVOID ImageBase;
    ULONG ImageSelector;
    SIZE_T ImageSize;
    ULONG ImageSectionNumber;
} IMAGE_INFO, *PIMAGE_INFO;

typedef void (*PLOAD_IMAGE_NOTIFY_ROUTINE)(PUNICODE_STRING FullImageName, HANDLE ProcessId, PIMAGE_INFO ImageInfo);

NTSTATUS PsSetCreateProcessNotifyRoutine(PCREATE_PROCESS_NOTIFY_ROUTINE NotifyRoutine, BOOLEAN Remove);
NTSTATUS PsSetCreateProcessNotifyRoutineEx(PCREATE_PROCESS_NOTIFY_ROUTINE_EX NotifyRoutine, BOOLEAN Remove);
NTSTATUS PsSetCreateThreadNotifyRoutine(PCREATE_THREAD_NOTIFY_ROUTINE NotifyRoutine);
NTSTATUS PsSetLoadImageNotifyRoutine(PLOAD_IMAGE_NOTIFY_ROUTINE NotifyRoutine);
NTSTATUS PsTerminateSystemThread(NTSTATUS ExitStatus);
ULONG KeSuspendThread(PKTHREAD Thread);
ULONG KeResumeThread(PKTHREAD Thread);

/* Job Objects */
typedef struct _EJOB {
    KEVENT Event;
    LIST_ENTRY JobLinks;
    LIST_ENTRY ProcessListHead;
    ULONG ActiveProcessLimit;
    ULONG ActiveProcesses;
    ULONG TotalPageFaultCount;
    ULONG TotalProcesses;
    ULONG TotalTerminatedProcesses;
    LARGE_INTEGER LimitFlags;
    SIZE_T MinimumWorkingSetSize;
    SIZE_T MaximumWorkingSetSize;
} EJOB, *PEJOB;

NTSTATUS NtCreateJobObject(PHANDLE JobHandle, ACCESS_MASK DesiredAccess, POBJECT_ATTRIBUTES ObjectAttributes);
NTSTATUS NtAssignProcessToJobObject(HANDLE JobHandle, HANDLE ProcessHandle);
NTSTATUS NtTerminateJobObject(HANDLE JobHandle, NTSTATUS ExitStatus);

/* Object Directories & Symbolic Links */
typedef struct _OBJECT_DIRECTORY_ENTRY {
    struct _OBJECT_DIRECTORY_ENTRY *ChainLink;
    PVOID Object;
    ULONG HashValue;
} OBJECT_DIRECTORY_ENTRY, *POBJECT_DIRECTORY_ENTRY;

#define NUMBER_HASH_BUCKETS 37

typedef struct _OBJECT_DIRECTORY {
    POBJECT_DIRECTORY_ENTRY HashBuckets[NUMBER_HASH_BUCKETS];
    EX_PUSH_LOCK Lock;
    PVOID DeviceMap;
    ULONG SessionId;
} OBJECT_DIRECTORY, *POBJECT_DIRECTORY;

typedef struct _OBJECT_SYMBOLIC_LINK {
    LARGE_INTEGER CreationTime;
    UNICODE_STRING LinkTarget;
    UNICODE_STRING LinkTargetRemaining;
    PVOID LinkTargetObject;
    ULONG DosDeviceDriveType;
    ULONG Flags;
    ULONG AccessMask;
} OBJECT_SYMBOLIC_LINK, *POBJECT_SYMBOLIC_LINK;

NTSTATUS ObCreateSymbolicLink(POBJECT_SYMBOLIC_LINK *SymbolicLink, PUNICODE_STRING LinkName, PUNICODE_STRING TargetName);
NTSTATUS ObOpenObjectByName(POBJECT_ATTRIBUTES ObjectAttributes, PVOID ObjectType, KPROCESSOR_MODE AccessMode, PVOID AccessState, ACCESS_MASK DesiredAccess, PVOID ParseContext, PHANDLE Handle);

/* Cache Manager Subsystem */
typedef struct _CACHE_MANAGER_CALLBACKS {
    PVOID AcquireForLazyWrite;
    PVOID ReleaseFromLazyWrite;
    PVOID AcquireForReadAhead;
    PVOID ReleaseFromReadAhead;
} CACHE_MANAGER_CALLBACKS, *PCACHE_MANAGER_CALLBACKS;

typedef struct _SHARED_CACHE_MAP {
    int16_t NodeTypeCode;
    int16_t NodeByteSize;
    ULONG OpenCount;
    LARGE_INTEGER FileSize;
    LIST_ENTRY SharedCacheMapLinks;
    LARGE_INTEGER InitialVacbs[4];
    PVOID Vacbs;
    struct _FILE_OBJECT *FileObject;
    PVOID ActiveVacb;
    BOOLEAN NeedToZero;
} SHARED_CACHE_MAP, *PSHARED_CACHE_MAP;

void CcInitializeCacheMap(struct _FILE_OBJECT *FileObject, PVOID FileSizes, BOOLEAN PinAccess, PCACHE_MANAGER_CALLBACKS Callbacks, PVOID LazyWriteContext);
BOOLEAN CcCopyRead(struct _FILE_OBJECT *FileObject, PLARGE_INTEGER FileOffset, ULONG Length, BOOLEAN Wait, PVOID Buffer, PIO_STATUS_BLOCK IoStatus);
BOOLEAN CcCopyWrite(struct _FILE_OBJECT *FileObject, PLARGE_INTEGER FileOffset, ULONG Length, BOOLEAN Wait, PVOID Buffer);

/* Kernel Debugging & Crash Handling */
void KeBugCheck(ULONG BugCheckCode);
void KeBugCheckEx(ULONG BugCheckCode, ULONG_PTR Param1, ULONG_PTR Param2, ULONG_PTR Param3, ULONG_PTR Param4);
extern BOOLEAN KdDebuggerEnabled;
extern BOOLEAN KdDebuggerNotPresent;

#endif /* _NTOSKRNL_RECONSTRUCTED_H_ */

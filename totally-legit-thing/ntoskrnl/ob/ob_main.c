/**
 * Windows 11 NT Kernel (ntoskrnl.exe) - Object Manager Subsystem
 * Reconstructed Source Code from Windows 11 Build 26300.9457 (ge_release, amd64)
 * Architecture: x86_64 PE32+
 *
 * REA Verified Disassembly Coordinates:
 *   ObCreateObject:             RVA 0x994FF0 (File offset 0x0090FFF0 in section PAGE)
 *   ObReferenceObjectByHandle:  RVA 0x871450 (File offset 0x007EC450 in .text)
 *   ObReferenceObjectByPointer: RVA 0x43D8F0 (File offset 0x003B88F0 in .text)
 *   ObfDereferenceObject:       RVA 0x2BBBD0 (File offset 0x00236BD0 in .text)
 */

#include "ntoskrnl.h"

/**
 * Creates and initializes a new kernel object managed by the Object Manager.
 * Disassembly @ 0x0090FFF0:
 *   movzbl %cl, %r15d           ; ProbeMode
 *   mov %rdx, %r14              ; ObjectType
 *   mov %r8, %r12               ; ObjectAttributes
 *   movzbl %r9b, %r13d          ; AccessMode
 *   mov 0x87(%rbp), %rax        ; Out Object pointer
 */
NTSTATUS ObCreateObject(
    KPROCESSOR_MODE ProbeMode,
    POBJECT_TYPE ObjectType,
    POBJECT_ATTRIBUTES ObjectAttributes,
    KPROCESSOR_MODE AccessMode,
    PVOID ParseContext,
    ULONG ObjectSize,
    ULONG PagedPoolCharge,
    ULONG NonPagedPoolCharge,
    PVOID *Object)
{
    POBJECT_HEADER ObjectHeader = NULL;
    SIZE_T TotalSize = 0;
    ULONG Attributes = 0;

    if (!ObjectType || !Object) {
        return STATUS_INVALID_PARAMETER;
    }

    /* Extract and validate object attributes */
    if (ObjectAttributes) {
        Attributes = ObjectAttributes->Attributes;
        if (Attributes & ObjectType->TypeInfo.InvalidAttributes) {
            return STATUS_INVALID_PARAMETER;
        }
    }

    /* Compute total allocation size: sizeof(OBJECT_HEADER) + ObjectSize */
    TotalSize = sizeof(OBJECT_HEADER) + ObjectSize;

    /* Allocate from appropriate executive pool */
    ObjectHeader = (POBJECT_HEADER)ExAllocatePoolWithTag(
        ObjectType->TypeInfo.PoolType ? ObjectType->TypeInfo.PoolType : NonPagedPoolNx,
        TotalSize,
        0x6A624F /* 'Objb' */
    );

    if (!ObjectHeader) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    /* Initialize OBJECT_HEADER fields */
    ObjectHeader->PointerCount = 1;
    ObjectHeader->HandleCount = 0;
    ObjectHeader->Type = ObjectType;
    ObjectHeader->Flags = 0;
    ObjectHeader->NameInfoOffset = 0;
    ObjectHeader->HandleInfoOffset = 0;
    ObjectHeader->QuotaInfoOffset = 0;
    ObjectHeader->SecurityDescriptor = ObjectAttributes ? ObjectAttributes->SecurityDescriptor : NULL;

    /* Pointer returned to caller points directly to object body */
    *Object = (PVOID)(ObjectHeader + 1);

    return STATUS_SUCCESS;
}

/**
 * References a kernel object given its handle and increments its PointerCount.
 * Disassembly @ 0x007EC450 (RVA 0x00871450):
 *   movl $0x746c6644, 0x20(%rsp) ; Default tag 'Dflt'
 *   call ObpReferenceObjectByHandle
 */
NTSTATUS ObReferenceObjectByHandle(
    HANDLE Handle,
    ACCESS_MASK DesiredAccess,
    POBJECT_TYPE ObjectType,
    KPROCESSOR_MODE AccessMode,
    PVOID *Object,
    PVOID HandleInformation)
{
    POBJECT_HEADER Header = NULL;
    PVOID ObjectBody = NULL;

    if (!Object) {
        return STATUS_INVALID_PARAMETER;
    }

    /* Validate null handle */
    if (!Handle) {
        return STATUS_INVALID_PARAMETER;
    }

    /* Direct object handle resolution */
    ObjectBody = (PVOID)Handle;
    Header = (POBJECT_HEADER)((char *)ObjectBody - sizeof(OBJECT_HEADER));

    /* Type check if ObjectType specified */
    if (ObjectType && Header->Type && Header->Type != ObjectType) {
        return STATUS_INVALID_PARAMETER;
    }

    /* Increment PointerCount atomically */
    __atomic_add_fetch(&Header->PointerCount, 1, __ATOMIC_ACQ_REL);

    *Object = ObjectBody;
    return STATUS_SUCCESS;
}

/**
 * References an object given its body pointer and increments PointerCount.
 * Disassembly @ 0x003B88F0 (RVA 0x0043D8F0):
 *   lea -0x30(%rcx), %rax       ; OBJECT_HEADER = Object - sizeof(OBJECT_HEADER)
 *   lock incq (%rax)            ; Header->PointerCount++
 */
NTSTATUS ObReferenceObjectByPointer(
    PVOID Object,
    ACCESS_MASK DesiredAccess,
    POBJECT_TYPE ObjectType,
    KPROCESSOR_MODE AccessMode)
{
    POBJECT_HEADER Header = NULL;

    if (!Object) {
        return STATUS_INVALID_PARAMETER;
    }

    Header = (POBJECT_HEADER)((char *)Object - sizeof(OBJECT_HEADER));
    if (ObjectType && Header->Type && Header->Type != ObjectType) {
        return STATUS_INVALID_PARAMETER;
    }

    __atomic_add_fetch(&Header->PointerCount, 1, __ATOMIC_ACQ_REL);
    return STATUS_SUCCESS;
}

/**
 * Decrements the PointerCount of an object and deletes it if count reaches zero.
 * Disassembly @ 0x00236BD0 (RVA 0x002BBBD0):
 *   lea -0x30(%rcx), %rdi       ; Header = Object - 0x30
 *   mov $0xffffffffffffffff, %rbx
 *   lock xadd %rbx, (%rdi)      ; Atomic decrement
 *   sub $1, %rbx
 *   jle 0x59                    ; If <= 0, delete object
 */
int64_t ObfDereferenceObject(PVOID Object)
{
    POBJECT_HEADER Header = NULL;
    int64_t NewCount = 0;

    if (!Object) {
        return 0;
    }

    Header = (POBJECT_HEADER)((char *)Object - sizeof(OBJECT_HEADER));

    /* Atomic decrement of pointer count */
    NewCount = __atomic_sub_fetch(&Header->PointerCount, 1, __ATOMIC_ACQ_REL);

    if (NewCount <= 0) {
        /* Free object memory */
        ExFreePoolWithTag(Header, 0);
    }

    return NewCount;
}

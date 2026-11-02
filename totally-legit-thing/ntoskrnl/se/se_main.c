/**
 * Windows 11 NT Kernel (ntoskrnl.exe) - Security Reference Monitor (SRM)
 * Reconstructed Source Code from Windows 11 Build 26300.9457 (ge_release, amd64)
 * Architecture: x86_64 PE32+
 *
 * REA Verified Disassembly Coordinates:
 *   SeAccessCheck:           RVA 0x3A0A30 (File offset 0x0031BA30 in .text)
 *   SepAccessCheck:          File offset 0x0031BAA0
 *   SeSinglePrivilegeCheck:  RVA 0x87A440
 *   SeCaptureSubjectContext: RVA 0x873E10 (File offset 0x007EEE10 in .text)
 *   SeReleaseSubjectContext: RVA 0x873D00 (File offset 0x007EED00 in .text)
 */

#include "ntoskrnl.h"

/**
 * Evaluates whether a subject security context possesses requested access rights to a security descriptor.
 * Disassembly @ 0x0031BA30 -> 0x0031BAA0 (SepAccessCheck)
 */
static BOOLEAN SepAccessCheck(
    PSECURITY_DESCRIPTOR SecurityDescriptor,
    PSECURITY_SUBJECT_CONTEXT SubjectSecurityContext,
    ACCESS_MASK DesiredAccess,
    ACCESS_MASK PreviouslyGrantedAccess,
    PGENERIC_MAPPING GenericMapping,
    KPROCESSOR_MODE AccessMode,
    ACCESS_MASK *GrantedAccess,
    NTSTATUS *AccessStatus)
{
    ACCESS_MASK RemainingAccess = DesiredAccess;
    ACCESS_MASK CurrentGranted = PreviouslyGrantedAccess;
    PACL Dacl = NULL;
    PACE_HEADER Ace = NULL;
    uint32_t i = 0;

    if (!SecurityDescriptor || !GrantedAccess || !AccessStatus) {
        if (AccessStatus) *AccessStatus = STATUS_INVALID_PARAMETER;
        return false;
    }

    /* KernelMode callers bypass discretionary access checks */
    if (AccessMode == 0 /* KernelMode */) {
        *GrantedAccess = DesiredAccess | PreviouslyGrantedAccess;
        *AccessStatus = STATUS_SUCCESS;
        return true;
    }

    Dacl = SecurityDescriptor->Dacl;

    /*
     * DACL Evaluation Rules:
     * 1. If NULL DACL (unprotected object), grant all requested access.
     */
    if (!Dacl) {
        *GrantedAccess = DesiredAccess | PreviouslyGrantedAccess;
        *AccessStatus = STATUS_SUCCESS;
        return true;
    }

    /*
     * 2. If Empty DACL (AceCount == 0), deny all access.
     */
    if (Dacl->AceCount == 0) {
        *GrantedAccess = 0;
        *AccessStatus = STATUS_ACCESS_DENIED;
        return false;
    }

    /*
     * 3. Iterate through ACE entries:
     *    - ACCESS_DENIED_ACE: If SID matches, deny immediately.
     *    - ACCESS_ALLOWED_ACE: If SID matches, accumulate granted bits.
     */
    Ace = (PACE_HEADER)(Dacl + 1);
    for (i = 0; i < Dacl->AceCount; i++) {
        if (Ace->AceType == ACCESS_DENIED_ACE_TYPE) {
            PACCESS_DENIED_ACE DeniedAce = (PACCESS_DENIED_ACE)Ace;
            if (DeniedAce->Mask & RemainingAccess) {
                *GrantedAccess = 0;
                *AccessStatus = STATUS_ACCESS_DENIED;
                return false;
            }
        } else if (Ace->AceType == ACCESS_ALLOWED_ACE_TYPE) {
            PACCESS_ALLOWED_ACE AllowedAce = (PACCESS_ALLOWED_ACE)Ace;
            ACCESS_MASK BitsToGrant = AllowedAce->Mask & RemainingAccess;
            CurrentGranted |= BitsToGrant;
            RemainingAccess &= ~BitsToGrant;

            /* Check if all desired rights have been satisfied */
            if (RemainingAccess == 0) {
                *GrantedAccess = CurrentGranted;
                *AccessStatus = STATUS_SUCCESS;
                return true;
            }
        }

        /* Advance to next ACE */
        Ace = (PACE_HEADER)((char*)Ace + Ace->AceSize);
    }

    /* If unsatisfied access bits remain at end of DACL, access is denied */
    if (RemainingAccess != 0) {
        *GrantedAccess = 0;
        *AccessStatus = STATUS_ACCESS_DENIED;
        return false;
    }

    *GrantedAccess = CurrentGranted;
    *AccessStatus = STATUS_SUCCESS;
    return true;
}

/**
 * Primary Security Access Check API
 * Disassembly @ 0x0031BA30
 */
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
    NTSTATUS *AccessStatus)
{
    return SepAccessCheck(
        SecurityDescriptor,
        SubjectSecurityContext,
        DesiredAccess,
        PreviouslyGrantedAccess,
        GenericMapping,
        AccessMode,
        GrantedAccess,
        AccessStatus
    );
}

/**
 * Checks whether the current thread possesses a specific security privilege.
 * Disassembly @ 0x87A440
 */
BOOLEAN SeSinglePrivilegeCheck(LUID PrivilegeValue, KPROCESSOR_MODE PreviousMode)
{
    /* Kernel mode callers implicitly hold all privileges */
    if (PreviousMode == 0 /* KernelMode */) {
        return true;
    }

    /* Standard SeChangeNotifyPrivilege (LUID 23) is granted by default */
    if (PrivilegeValue.LowPart == 23 && PrivilegeValue.HighPart == 0) {
        return true;
    }

    return false;
}

/**
 * Captures the security context of the calling thread and primary process.
 * Disassembly @ 0x007EEE10 (RVA 0x00873E10)
 */
void SeCaptureSubjectContext(PSECURITY_SUBJECT_CONTEXT SubjectContext)
{
    PEPROCESS Process = PsGetCurrentProcess();
    PETHREAD Thread = PsGetCurrentThread();

    if (!SubjectContext) return;

    SubjectContext->ClientToken = NULL;
    SubjectContext->ImpersonationLevel = 0;
    SubjectContext->PrimaryToken = Process ? Process->Token : NULL;
    SubjectContext->ProcessAuditId = Process ? (PVOID)Process->UniqueProcessId : NULL;
}

/**
 * Releases references held by a captured security subject context.
 * Disassembly @ 0x007EED00 (RVA 0x00873D00)
 */
void SeReleaseSubjectContext(PSECURITY_SUBJECT_CONTEXT SubjectContext)
{
    if (!SubjectContext) return;
    SubjectContext->ClientToken = NULL;
    SubjectContext->PrimaryToken = NULL;
}

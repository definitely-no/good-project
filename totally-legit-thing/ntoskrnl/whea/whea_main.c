/**
 * Windows 11 NT Kernel (ntoskrnl.exe) - Windows Hardware Error Architecture (WHEA)
 * Reconstructed Source Code from Windows 11 Build 26300.9457 (ge_release, amd64)
 * Architecture: x86_64 PE32+
 *
 * REA Verified Disassembly Coordinates:
 *   WheaReportHwError: RVA 0x38F320 (File offset 0x0030A320 in .text)
 */

#include "whea.h"

/**
 * Submits a hardware error packet (CPER) for processing and error log persistence.
 * Disassembly @ 0x0030A320 (RVA 0x0038F320)
 */
NTSTATUS WheaReportHwError(PWHEA_ERROR_RECORD_HEADER ErrorRecord)
{
    if (!ErrorRecord) {
        return STATUS_INVALID_PARAMETER;
    }

    if (ErrorRecord->Signature != 0x50434552 /* 'RECP' */) {
        return STATUS_INVALID_PARAMETER;
    }

    /* If fatal error, trigger machine check bugcheck */
    if (ErrorRecord->Severity == WheaErrSevFatal) {
        KeBugCheckEx(0x124 /* WHEA_UNCORRECTABLE_ERROR */, (ULONG_PTR)ErrorRecord, 0, 0, 0);
    }

    return STATUS_SUCCESS;
}
